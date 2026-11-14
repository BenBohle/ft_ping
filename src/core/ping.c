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
#endif

volatile sig_atomic_t ft_ping_stop = 0;

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

static void ft_ping_format_icmp_error(char *buffer, size_t size, uint8_t type, uint8_t code)
{
    const char *text = "ICMP error";

    if (type == 3)
    {
        if (code == 1)
        {
            text = "Destination Host Unreachable";
        }
        else if (code == 0)
        {
            text = "Destination Net Unreachable";
        }
        else
        {
            text = "Destination Unreachable";
        }
    }
    else if (type == 11)
    {
        if (code == 0)
        {
            text = "Time to live exceeded";
        }
        else
        {
            text = "Time exceeded";
        }
    }
    else if (type == 12)
    {
        text = "Parameter problem";
    }

    snprintf(buffer, size, "%s", text);
}

static void ft_ping_store_packet_error(FtPingResult *result, const FtPingEchoReply *reply)
{
    result->packet_error = true;
    result->ttl = reply->ttl;
    result->payload = reply->payload;
    result->has_ip_header = reply->has_ip_header;
    result->ip_header_len = reply->ip_header_len;
    result->has_quoted_icmp = reply->has_quoted_icmp;
    result->quoted_type = reply->quoted_type;
    result->quoted_code = reply->quoted_code;
    result->quoted_size = reply->quoted_size;
    result->quoted_id = reply->quoted_id;
    result->quoted_seq = reply->quoted_seq;
    if (reply->has_ip_header && reply->ip_header_len > 0)
    {
        memcpy(result->ip_header, reply->ip_header, reply->ip_header_len);
    }
    inet_ntop(AF_INET, &reply->from.sin_addr, result->from, sizeof(result->from));
    ft_ping_format_icmp_error(result->error_text, sizeof(result->error_text), reply->type, reply->code);
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
    session->stats.total_rtt_sq_ms += result->rtt_ms * result->rtt_ms;
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
    double start_time_ms;
    double deadline_ms;
    const uint32_t poll_slice_ms = 200u;

    memset(result, 0, sizeof(*result));

    request.identifier = session->identifier;
    request.sequence = sequence;
    request.payload_size = session->options.payload_bytes;

    start_time_ms = ft_ping_get_time_ms();
    result->sent_at_ms = start_time_ms;
    deadline_ms = start_time_ms + (double)session->options.timeout_ms;

    if (ft_ping_send_echo_request(&session->socket, &request, &session->destination,
                                  error_buffer, error_buffer_size) < 0)
    {
        return false;
    }

    session->stats.packets_sent++;

    for (;;)
    {
        FtPingEchoReply reply;
        uint32_t timeout_ms;

        if (ft_ping_stop)
        {
            result->interrupted = true;
            return true;
        }

        timeout_ms = ft_ping_remaining_timeout_ms(deadline_ms);
        if (timeout_ms == 0u)
        {
            result->timeout = true;
            return true;
        }

        if (timeout_ms > poll_slice_ms)
        {
            timeout_ms = poll_slice_ms;
        }

        if (!ft_ping_socket_set_timeout_ms(&session->socket, timeout_ms, error_buffer, error_buffer_size))
        {
            return false;
        }

        if (!ft_ping_echo_reply(&session->socket, session->identifier, sequence, &reply,
                                error_buffer, error_buffer_size))
        {
            return false;
        }

        if (reply.timeout || reply.ignored)
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
