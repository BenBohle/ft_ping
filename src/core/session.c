#include "ft_ping/core/session.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <unistd.h>
#endif

#include "ft_ping/utils/logger.h"

static bool ft_ping_resolve_target(FtPingSession *session, char *error_buffer, size_t error_buffer_size)
{
    struct addrinfo hints;
    struct addrinfo *info = NULL;
    struct addrinfo *entry;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;

    status = getaddrinfo(session->options.target, NULL, &hints, &info);
    if (status != 0)
    {
        snprintf(error_buffer, error_buffer_size, "unknown host %s (%s)",
                 session->options.target, gai_strerror(status));
        return false;
    }

    entry = info;
    while (entry && entry->ai_family != AF_INET)
    {
        entry = entry->ai_next;
    }

    if (!entry || !entry->ai_addr || entry->ai_addrlen < sizeof(struct sockaddr_in))
    {
        freeaddrinfo(info);
        snprintf(error_buffer, error_buffer_size, "unknown host %s", session->options.target);
        return false;
    }

    memcpy(&session->destination, entry->ai_addr, sizeof(session->destination));
    freeaddrinfo(info);

    if (!inet_ntop(AF_INET, &session->destination.sin_addr,
                   session->resolved_ip, sizeof(session->resolved_ip)))
    {
        snprintf(error_buffer, error_buffer_size, "unknown host %s", session->options.target);
        return false;
    }

    return true;
}

static uint16_t ft_ping_process_identifier(void)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    return (uint16_t)(GetCurrentProcessId() & 0xFFFF);
#else
    return (uint16_t)(getpid() & 0xFFFF);
#endif
}

void ft_ping_session_reset(FtPingSession *session)
{
    memset(session, 0, sizeof(*session));
    ft_ping_socket_init(&session->socket);
    session->stats.min_rtt_ms = DBL_MAX;
}

bool ft_ping_session_prepare(FtPingSession *session, const FtPingOptions *options, char *error_buffer, size_t error_buffer_size)
{
    FtPingSocketConfig socket_config;

    ft_ping_session_reset(session);
    session->options = *options;
    session->identifier = ft_ping_process_identifier();

    if (!ft_ping_resolve_target(session, error_buffer, error_buffer_size))
    {
        return false;
    }

    socket_config.ttl = options->ttl;
    socket_config.payload_bytes = options->payload_bytes;
    socket_config.timeout_ms = options->timeout_ms;

    if (!ft_ping_socket_open(&session->socket, &socket_config, options->target, error_buffer, error_buffer_size))
    {
        return false;
    }

    session->is_ready = true;
    return true;
}

void ft_ping_session_shutdown(FtPingSession *session)
{
    if (!session)
    {
        return;
    }

    ft_ping_socket_close(&session->socket);
    session->is_ready = false;
}

bool ft_ping_session_tick(FtPingSession *session, double now_ms)
{
    if (!session->is_ready)
    {
        return false;
    }

    FT_PING_UNUSED(now_ms);
    ft_ping_log_message(FT_PING_LOG_LEVEL_DEBUG, "tick placeholder for host %s", session->options.target);
    return true;
}
