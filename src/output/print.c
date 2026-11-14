#include "ft_ping/output/print.h"

#include <math.h>
#include <stdio.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <arpa/inet.h>
#endif

static void ft_ping_print_ip_header(const uint8_t *ip, size_t length)
{
    char source[16];
    char destination[16];
    unsigned int version;
    unsigned int header_words;
    uint16_t total_length;
    uint16_t id;
    uint16_t offset;
    unsigned int flags;
    size_t index;

    if (!ip || length < 20)
    {
        return;
    }

    version = ip[0] >> 4;
    header_words = ip[0] & 0x0Fu;
    total_length = (uint16_t)((ip[2] << 8) | ip[3]);
    id = (uint16_t)((ip[4] << 8) | ip[5]);
    offset = (uint16_t)((ip[6] << 8) | ip[7]);
    flags = (offset & 0xe000u) >> 13;

    if (!inet_ntop(AF_INET, ip + 12, source, sizeof(source)))
    {
        source[0] = '\0';
    }
    if (!inet_ntop(AF_INET, ip + 16, destination, sizeof(destination)))
    {
        destination[0] = '\0';
    }

    printf("IP Hdr Dump:\n ");
    for (index = 0; index + 1 < length; index += 2)
    {
        if (index > 0)
        {
            printf(" ");
        }
        printf("%02x%02x", ip[index], ip[index + 1]);
    }
    printf("\n");
    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
    printf(" %x  %x  %02x %04x %04x   %x %04x  %02x  %02x %04x %s  %s\n",
           version,
           header_words,
           ip[1],
           total_length,
           id,
           flags,
           offset & 0x1fffu,
           ip[8],
           ip[9],
           (unsigned)((ip[10] << 8) | ip[11]),
           source,
           destination);
}

static void ft_ping_print_error(const FtPingSession *session, uint16_t sequence, const FtPingResult *result)
{
    if (session->options.windows_output)
    {
        printf("From %s icmp_seq=%u %s\n", result->from, sequence, result->error_text);
    }
    else
    {
        printf("%zu bytes from %s: %s\n", result->payload, result->from, result->error_text);
    }

    if (session->options.windows_output || !session->options.verbose || !result->has_ip_header)
    {
        return;
    }

    ft_ping_print_ip_header(result->ip_header, result->ip_header_len);
    if (result->has_quoted_icmp)
    {
        printf("ICMP: type %u, code %u, size %zu, id 0x%04x, seq 0x%04x\n",
               result->quoted_type,
               result->quoted_code,
               result->quoted_size,
               result->quoted_id,
               result->quoted_seq);
    }
}

static void ft_ping_print_header_inetutils(const FtPingSession *session)
{
    printf("PING %s (%s): %zu data bytes",
           session->options.target,
           session->resolved_ip,
           session->options.payload_bytes);
    if (session->options.verbose)
    {
        printf(", id 0x%04x = %u", session->identifier, session->identifier);
    }
    printf("\n");
}

static void ft_ping_print_result_inetutils(const FtPingSession *session, uint16_t sequence, const FtPingResult *result)
{
    if (result->timeout)
    {
        return;
    }

    if (result->packet_error)
    {
        ft_ping_print_error(session, sequence, result);
        return;
    }

    if (result->received)
    {
        printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
               result->payload,
               result->from,
               sequence,
               result->ttl,
               result->rtt_ms);
    }
}

static void ft_ping_print_stats_inetutils(const FtPingSession *session)
{
    uint64_t sent = session->stats.packets_sent;
    uint64_t received = session->stats.packets_received;

    printf("--- %s ping statistics ---\n", session->options.target);
    printf("%llu packets transmitted, ", (unsigned long long)sent);
    printf("%llu packets received, ", (unsigned long long)received);
    if (sent > 0)
    {
        unsigned long long lost = received < sent ? sent - received : 0;

        printf("%d%% packet loss", (int)((lost * 100ull) / sent));
    }
    printf("\n");

    if (received > 0)
    {
        double average = session->stats.total_rtt_ms / (double)received;
        double variance = session->stats.total_rtt_sq_ms / (double)received - average * average;

        if (variance < 0.0)
        {
            variance = 0.0;
        }

        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
               session->stats.min_rtt_ms,
               average,
               session->stats.max_rtt_ms,
               sqrt(variance));
    }
}

static void ft_ping_print_header_windows(const FtPingSession *session)
{
    printf("PING %s: %zu data bytes\n", session->options.target, session->options.payload_bytes);
}

static void ft_ping_print_result_windows(const FtPingSession *session, uint16_t sequence, const FtPingResult *result)
{
    if (result->timeout)
    {
        printf("Request timeout for icmp_seq %u\n", sequence);
        return;
    }

    if (result->packet_error)
    {
        ft_ping_print_error(session, sequence, result);
        return;
    }

    if (result->received)
    {
        printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.2f ms\n",
               result->payload,
               result->from,
               sequence,
               result->ttl,
               result->rtt_ms);
    }
}

static void ft_ping_print_stats_windows(const FtPingSession *session)
{
    uint64_t sent = session->stats.packets_sent;
    uint64_t received = session->stats.packets_received;
    uint64_t lost = sent > received ? sent - received : 0;

    printf("\nPing statistics for %s:\n", session->options.target);
    printf("    Packets: Sent = %llu, Received = %llu, Lost = %llu\n",
           (unsigned long long)sent,
           (unsigned long long)received,
           (unsigned long long)lost);

    if (sent > 0)
    {
        printf("    (%llu%% loss),\n", (unsigned long long)((lost * 100ull) / sent));
    }

    if (received > 0)
    {
        double average = session->stats.total_rtt_ms / (double)received;
        printf("Approximate round trip times in milli-seconds:\n");
        printf("    Minimum = %.2fms, Maximum = %.2fms, Average = %.2fms\n",
               session->stats.min_rtt_ms,
               session->stats.max_rtt_ms,
               average);
    }
}

void ft_ping_print_header(const FtPingSession *session)
{
    if (session->options.windows_output)
    {
        ft_ping_print_header_windows(session);
    }
    else
    {
        ft_ping_print_header_inetutils(session);
    }
    fflush(stdout);
}

void ft_ping_print_result(const FtPingSession *session, uint16_t sequence, const FtPingResult *result)
{
    if (!session || !result || result->interrupted)
    {
        return;
    }

    if (session->options.windows_output)
    {
        ft_ping_print_result_windows(session, sequence, result);
    }
    else
    {
        ft_ping_print_result_inetutils(session, sequence, result);
    }
    fflush(stdout);
}

void ft_ping_print_stats(const FtPingSession *session)
{
    if (session->options.windows_output)
    {
        ft_ping_print_stats_windows(session);
    }
    else
    {
        ft_ping_print_stats_inetutils(session);
    }
    fflush(stdout);
}
