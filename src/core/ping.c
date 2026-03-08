#include "ft_ping/core/ping.h"
#include "ft_ping/icmp/echo_request.h"
#include "ft_ping/icmp/echo_reply.h"
#include "ft_ping/icmp/echo_utils.h"

#include <string.h>
#include <stdio.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <arpa/inet.h>
#   include <unistd.h>
#endif

bool ft_ping(FtPingSession *session,
             uint16_t sequence,
             FtPingResult *result,
             char *error_buffer,
             size_t error_buffer_size) {
    

    memset(result, 0, sizeof(*result));

    FtPingEchoRequestParams params;
    params.target       = session->options.target;
#if defined(FT_PING_PLATFORM_WINDOWS)
        params.identifier   = (uint16_t)(GetCurrentProcessId() & 0xFFFF);
#else
        params.identifier   = (uint16_t)(getpid() & 0xFFFF);
#endif

    params.sequence     = sequence;
    params.payload_size = session->options.payload_bytes;

    struct sockaddr_in dest;
    double start_time = ft_ping_get_time_ms();

    int sent = ft_ping_send_echo_request(&session->socket, &params, &dest,
                    error_buffer, error_buffer_size);

    if (sent < 0)
        return false;

    session->stats.packets_sent++;

    FtPingEchoReply reply;
    if (!ft_ping_echo_reply(&session->socket, &reply, error_buffer, error_buffer_size))
        return false;

    double end_time = ft_ping_get_time_ms();

    if (reply.timeout)
    {
        result->timeout = true;
        return true;
    }

    if (reply.received)
    {
        result->received = true;
        result->payload  = reply.payload;
        result->ttl      = reply.ttl;
        result->rtt_ms   = end_time - start_time;
        inet_ntop(AF_INET, &reply.from.sin_addr, result->from, sizeof(result->from));

        session->stats.packets_received++;
        session->stats.total_rtt_ms += result->rtt_ms;
        if (result->rtt_ms < session->stats.min_rtt_ms)
            session->stats.min_rtt_ms = result->rtt_ms;
        if (result->rtt_ms > session->stats.max_rtt_ms)
            session->stats.max_rtt_ms = result->rtt_ms;
    }

    return true;
}
