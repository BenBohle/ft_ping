#include "ft_ping/core/ping.h"
#include "ft_ping/icmp/echo_request.h"
#include "ft_ping/icmp/echo_reply.h"
#include "ft_ping/icmp/echo_utils.h"

#include <stdio.h>
#include <string.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <arpa/inet.h>
#   include <unistd.h>
#endif

static uint16_t ft_ping_request_identifier(void)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    return (uint16_t)(GetCurrentProcessId() & 0xFFFF);
#else
    return (uint16_t)(getpid() & 0xFFFF);
#endif
}

static uint32_t ft_ping_remaining_timeout_ms(double deadline_ms)
{
    double remaining_ms = deadline_ms - ft_ping_get_time_ms();

    if (remaining_ms <= 0.0)
    {
        return 0u;
    }

    if (remaining_ms < 1.0)
    {
        return 1u;
    }

    return (uint32_t)remaining_ms;
}

static const char *ft_ping_icmp_error_text(uint8_t type, uint8_t code)
{
    if (type == 3)
    {
        switch (code)
        {
            case 0: return "Destination Net Unreachable";
            case 1: return "Destination Host Unreachable";
            case 2: return "Destination Protocol Unreachable";
            case 3: return "Destination Port Unreachable";
            case 4: return "Fragmentation needed and DF set";
            case 5: return "Source Route Failed";
            default: return "Destination Unreachable";
        }
    }

    if (type == 11)
    {
        switch (code)
        {
            case 0: return "Time to live exceeded";
            case 1: return "Frag reassembly time exceeded";
            default: return "Time exceeded";
        }
    }

    if (type == 12)
    {
        switch (code)
        {
            case 0: return "Parameter problem";
            case 1: return "Missing required option";
            case 2: return "Bad length";
            default: return "Parameter problem";
        }
    }

    return "ICMP error";
}

static void ft_ping_store_packet_error(FtPingResult *result, const FtPingEchoReply *reply)
{
    result->packet_error = true;
    result->ttl = reply->ttl;
    inet_ntop(AF_INET, &reply->from.sin_addr, result->from, sizeof(result->from));
    snprintf(result->error_text, sizeof(result->error_text), "%s", ft_ping_icmp_error_text(reply->type, reply->code));
}

static void ft_ping_store_echo_reply(FtPingSession *session,
                                     FtPingResult *result,
                                     const FtPingEchoReply *reply,
                                     double start_time_ms)
{
    double end_time_ms = ft_ping_get_time_ms();

    result->received = true;
    result->payload = reply->payload;
    result->ttl = reply->ttl;
    result->rtt_ms = end_time_ms - start_time_ms;
    inet_ntop(AF_INET, &reply->from.sin_addr, result->from, sizeof(result->from));

    session->stats.packets_received++;
    session->stats.total_rtt_ms += result->rtt_ms;
    if (result->rtt_ms < session->stats.min_rtt_ms)
    {
        session->stats.min_rtt_ms = result->rtt_ms;
    }
    if (result->rtt_ms > session->stats.max_rtt_ms)
    {
        session->stats.max_rtt_ms = result->rtt_ms;
    }
}

bool ft_ping(FtPingSession *session,
             uint16_t sequence,
             FtPingResult *result,
             char *error_buffer,
             size_t error_buffer_size)
{
    FtPingEchoRequestParams request;
    struct sockaddr_in destination;
    uint16_t identifier = ft_ping_request_identifier();
    double start_time_ms;
    double deadline_ms;

    memset(result, 0, sizeof(*result));

    request.target = session->options.target;
    request.identifier = identifier;
    request.sequence = sequence;
    request.payload_size = session->options.payload_bytes;

    start_time_ms = ft_ping_get_time_ms();
    deadline_ms = start_time_ms + (double)session->options.timeout_ms;

    if (ft_ping_send_echo_request(&session->socket, &request, &destination, error_buffer, error_buffer_size) < 0)
    {
        return false;
    }

    session->stats.packets_sent++;

    for (;;)
    {
        FtPingEchoReply reply;
        uint32_t timeout_ms = ft_ping_remaining_timeout_ms(deadline_ms);

        if (timeout_ms == 0u)
        {
            result->timeout = true;
            return true;
        }

        if (!ft_ping_socket_set_timeout_ms(&session->socket, timeout_ms, error_buffer, error_buffer_size))
        {
            return false;
        }

        if (!ft_ping_echo_reply(&session->socket, identifier, sequence, &reply, error_buffer, error_buffer_size))
        {
            return false;
        }

        if (reply.timeout)
        {
            result->timeout = true;
            return true;
        }

        if (reply.ignored)
        {
            continue;
        }

        if (reply.packet_error)
        {
            ft_ping_store_packet_error(result, &reply);
            return true;
        }

        if (reply.received)
        {
            ft_ping_store_echo_reply(session, result, &reply, start_time_ms);
            return true;
        }
    }
}
