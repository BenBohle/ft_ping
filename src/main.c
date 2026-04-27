#include "ft_ping.h"
#include "ft_ping/core/ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <float.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <unistd.h>
#   include <sys/time.h>
#endif

static volatile bool g_running = true;

static void signal_handler(int signum)
{
    (void)signum;
    g_running = false;
}

int main(int argc, char **argv)
{
    if (!platform_system_init())
    {
        fprintf(stderr, "Failed to initialize platform backend (%s)\n", platform_system_name());
        return EXIT_FAILURE;
    }

    ft_ping_log_init_colors();

    FtPingOptions options;
    ft_ping_options_set_defaults(&options);

    char error_buffer[256] = { 0 };
    FtPingCliParseResult parse_result = ft_ping_cli_parse(argc, argv, &options, error_buffer, sizeof(error_buffer));

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

    signal(SIGINT, signal_handler);

    FtPingSession session;
    if (!ft_ping_session_prepare(&session, &options, error_buffer, sizeof(error_buffer)))
    {
        fprintf(stderr, "ft_ping: %s\n", error_buffer[0] ? error_buffer : "failed to start session");
        platform_system_shutdown();
        return EXIT_FAILURE;
    }

    printf("PING %s: %zu data bytes\n", options.target, options.payload_bytes);

    session.stats.min_rtt_ms = DBL_MAX;
    uint32_t seq = 0;

    while (g_running)
    {
        seq++;

        FtPingResult ping_result;
        error_buffer[0] = '\0';

        if (!ft_ping(&session, (uint16_t)seq, &ping_result, error_buffer, sizeof(error_buffer)))
        {
            fprintf(stderr, "ft_ping: %s\n", error_buffer[0] ? error_buffer : "ICMP failed");
            break;
        }

        if (ping_result.timeout)
        {
            printf("Request timeout for icmp_seq %u\n", seq);
        }
        else if (ping_result.packet_error)
        {
            if (options.verbose)
            {
                printf("From %s icmp_seq=%u %s\n",
                       ping_result.from,
                       seq,
                       ping_result.error_text);
            }
        }
        else if (ping_result.received)
        {
            printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.2f ms\n",
                   ping_result.payload,
                   ping_result.from,
                   seq,
                   ping_result.ttl,
                   ping_result.rtt_ms);
        }

        if (options.count > 0 && seq >= options.count)
        {
            g_running = false;
        }

        if (g_running)
        {
#if defined(FT_PING_PLATFORM_WINDOWS)
            Sleep(options.interval_ms);
#else
            usleep(options.interval_ms * 1000);
#endif
        }
    }

    uint64_t sent = session.stats.packets_sent;
    uint64_t recv = session.stats.packets_received;

    printf("\nPing statistics for %s:\n", options.target);
    printf("    Packets: Sent = %llu, Received = %llu, Lost = %llu\n",
           (unsigned long long)sent,
           (unsigned long long)recv,
           (unsigned long long)(sent - recv));

    if (sent > 0)
    {
        uint64_t loss_percent = ((sent - recv) * 100) / sent;
        printf("    (%llu%% loss),\n", (unsigned long long)loss_percent);
    }

    if (recv > 0)
    {
        double avg_rtt = session.stats.total_rtt_ms / recv;
        printf("Approximate round trip times in milli-seconds:\n");
        printf("    Minimum = %.2fms, Maximum = %.2fms, Average = %.2fms\n",
               session.stats.min_rtt_ms, session.stats.max_rtt_ms, avg_rtt);
    }

    ft_ping_session_shutdown(&session);
    platform_system_shutdown();
    return EXIT_SUCCESS;
}
