#ifndef FT_PING_CORE_SESSION_H
#define FT_PING_CORE_SESSION_H

#include "ft_ping/cli/options.h"
#include "ft_ping/platform/socket.h"

typedef struct FtPingStatistics
{
    uint64_t packets_sent;
    uint64_t packets_received;
    double min_rtt_ms;
    double max_rtt_ms;
    double total_rtt_ms;
    double total_rtt_sq_ms;
} FtPingStatistics;

typedef struct FtPingSession
{
    FtPingOptions options;
    FtPingSocket socket;
    FtPingStatistics stats;
    struct sockaddr_in destination;
    char resolved_ip[16];
    uint16_t identifier;
    bool is_ready;
} FtPingSession;

void ft_ping_session_reset(FtPingSession *session);
bool ft_ping_session_prepare(FtPingSession *session, const FtPingOptions *options, char *error_buffer, size_t error_buffer_size);
void ft_ping_session_shutdown(FtPingSession *session);
bool ft_ping_session_tick(FtPingSession *session, double now_ms);

#endif /* FT_PING_CORE_SESSION_H */
