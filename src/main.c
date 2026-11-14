#include "ft_ping.h"
#include "ft_ping/output/print.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <signal.h>
#endif

static void ft_ping_on_signal(int signum)
{
    (void)signum;
    ft_ping_stop = 1;
}

static void ft_ping_install_stop_handler(void)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    signal(SIGINT, ft_ping_on_signal);
#else
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = ft_ping_on_signal;
    sigaction(SIGINT, &action, NULL);
#endif
}

static void ft_ping_sleep_ms(uint32_t duration_ms)
{
    if (duration_ms == 0 || ft_ping_stop)
    {
        return;
    }

#if defined(FT_PING_PLATFORM_WINDOWS)
    while (duration_ms > 0 && !ft_ping_stop)
    {
        DWORD slice = duration_ms > 200u ? 200u : duration_ms;
        Sleep(slice);
        duration_ms -= slice;
    }
#else
    struct timespec remaining;

    remaining.tv_sec = (time_t)(duration_ms / 1000u);
    remaining.tv_nsec = (long)(duration_ms % 1000u) * 1000000L;
    while (!ft_ping_stop && nanosleep(&remaining, &remaining) < 0)
    {
        if (errno != EINTR)
        {
            break;
        }
    }
#endif
}

static void ft_ping_wait_interval(const FtPingSession *session, double sent_at_ms)
{
    double elapsed_ms;

    if (ft_ping_stop)
    {
        return;
    }

    elapsed_ms = ft_ping_get_time_ms() - sent_at_ms;
    if (elapsed_ms < (double)session->options.interval_ms)
    {
        ft_ping_sleep_ms((uint32_t)((double)session->options.interval_ms - elapsed_ms));
    }
}

int main(int argc, char **argv)
{
    FtPingOptions options;
    FtPingCliParseResult parse_result;
    FtPingSession session;
    char error_buffer[256];
    uint16_t sequence;
    int status;

    if (!platform_system_init())
    {
        fprintf(stderr, "Failed to initialize platform backend (%s)\n", platform_system_name());
        return EXIT_FAILURE;
    }

    ft_ping_log_init_colors();
    ft_ping_options_set_defaults(&options);
    error_buffer[0] = '\0';
    parse_result = ft_ping_cli_parse(argc, argv, &options, error_buffer, sizeof(error_buffer));

    if (!parse_result.success)
    {
        if (error_buffer[0] != '\0')
        {
            fprintf(stderr, "ft_ping: %s\n", error_buffer);
        }
        ft_ping_cli_print_usage(stderr);
        platform_system_shutdown();
        return parse_result.exit_code;
    }

    if (options.show_help)
    {
        ft_ping_cli_print_usage(stdout);
        platform_system_shutdown();
        return EXIT_SUCCESS;
    }

    if (options.show_version)
    {
        ft_ping_cli_print_version(stdout);
        platform_system_shutdown();
        return EXIT_SUCCESS;
    }

    ft_ping_install_stop_handler();

    if (!ft_ping_session_prepare(&session, &options, error_buffer, sizeof(error_buffer)))
    {
        fprintf(stderr, "ft_ping: %s\n", error_buffer[0] ? error_buffer : "failed to start session");
        platform_system_shutdown();
        return EXIT_FAILURE;
    }

    if (!ft_ping_stop)
    {
        ft_ping_print_header(&session);
    }

    sequence = 0;
    while (!ft_ping_stop)
    {
        FtPingResult ping_result;

        error_buffer[0] = '\0';
        if (!ft_ping(&session, sequence, &ping_result, error_buffer, sizeof(error_buffer)))
        {
            fprintf(stderr, "ft_ping: %s\n", error_buffer[0] ? error_buffer : "ICMP failed");
            break;
        }

        ft_ping_print_result(&session, sequence, &ping_result);
        if (ping_result.interrupted)
        {
            break;
        }

        if (options.count > 0 && session.stats.packets_sent >= options.count)
        {
            break;
        }

        ft_ping_wait_interval(&session, ping_result.sent_at_ms);
        if (ft_ping_stop)
        {
            break;
        }

        sequence++;
    }

    ft_ping_print_stats(&session);
    status = session.stats.packets_received > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    ft_ping_session_shutdown(&session);
    platform_system_shutdown();
    return status;
}
