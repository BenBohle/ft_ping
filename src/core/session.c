#include "ft_ping/core/session.h"

#include <string.h>

#include "ft_ping/utils/logger.h"

void ft_ping_session_reset(FtPingSession *session)
{
    // Todo: checken ob ma noch mehr brauchen beim reseten oder das inline schreiben können
    memset(session, 0, sizeof(*session));
}

bool ft_ping_session_prepare(FtPingSession *session, const FtPingOptions *options, char *error_buffer, size_t error_buffer_size)
{
    ft_ping_session_reset(session);
    session->options = *options;

    FtPingSocketConfig socket_config = {
        .ttl = options->ttl,
        .payload_bytes = options->payload_bytes,
        .timeout_ms = options->timeout_ms,
    };

    if (!ft_ping_socket_open(&session->socket, &socket_config, options->target, error_buffer, error_buffer_size))
    {
        ft_ping_log_message(FT_PING_LOG_LEVEL_ERROR, "failed to prepare raw socket on %s", FT_PING_OS_NAME);
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
