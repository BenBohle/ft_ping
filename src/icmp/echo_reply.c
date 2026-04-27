#include "ft_ping/icmp/echo_reply.h"

#include <stdio.h>
#include <string.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#include "ft_ping/platform/windows_support.h"
#else
#include <sys/socket.h>
#include <errno.h>
#endif

static bool ft_ping_reply_matches_request(uint16_t identifier, uint16_t sequence,
   uint16_t expected_identifier,
     uint16_t expected_sequence)
{
    return identifier == expected_identifier && sequence == expected_sequence;
}

static bool ft_ping_parse_embedded_request(const FtPingIcmpHeader *icmp,
                                           int icmp_len,
                                           uint16_t *identifier,
                                           uint16_t *sequence)
{
    const uint8_t *embedded_ip = (const uint8_t *)icmp + sizeof(*icmp);
    int embedded_bytes = icmp_len - (int)sizeof(*icmp);
    int embedded_ip_header_len;
    const FtPingIcmpHeader *embedded_icmp;

    if (embedded_bytes < 20)
    {
        return false;
    }

    embedded_ip_header_len = (embedded_ip[0] & 0x0F) * 4;
    if (embedded_ip_header_len < 20 || embedded_bytes < embedded_ip_header_len + (int)sizeof(*embedded_icmp))
    {
        return false;
    }

    embedded_icmp = (const FtPingIcmpHeader *)(embedded_ip + embedded_ip_header_len);
    if (embedded_icmp->type != 8 || embedded_icmp->code != 0)
    {
        return false;
    }

    *identifier = ntohs(embedded_icmp->identifier);
    *sequence = ntohs(embedded_icmp->sequence);
    return true;
}

static bool ft_ping_parse_icmp_packet(const uint8_t *packet,
                                      int received,
                                      const struct sockaddr_in *from,
                                      uint16_t expected_identifier,
                                      uint16_t expected_sequence,
                                      FtPingEchoReply *reply,
                                      char *error_buffer,
                                      size_t error_buffer_size)
{
    int ip_header_len = (packet[0] & 0x0F) * 4;
    int icmp_len = received - ip_header_len;
    const FtPingIcmpHeader *icmp;

    if (ip_header_len < 20 || icmp_len < (int)sizeof(*icmp))
    {
        snprintf(error_buffer, error_buffer_size, "Packet too short (%d bytes)", icmp_len);
        return false;
    }

    icmp = (const FtPingIcmpHeader *)(packet + ip_header_len);
    reply->from = *from;
    reply->ttl = packet[8];
    reply->type = icmp->type;
    reply->code = icmp->code;

    if (icmp->type == 0 && icmp->code == 0)
    {
        reply->identifier = ntohs(icmp->identifier);
        reply->sequence = ntohs(icmp->sequence);
        if (!ft_ping_reply_matches_request(reply->identifier,
                                           reply->sequence,
                                           expected_identifier,
                                           expected_sequence))
        {
            reply->ignored = true;
            return true;
        }

        reply->received = true;
        reply->payload = (size_t)icmp_len;
        return true;
    }

    if (!ft_ping_parse_embedded_request(icmp, icmp_len, &reply->identifier, &reply->sequence) ||
        !ft_ping_reply_matches_request(reply->identifier,
                                       reply->sequence,
                                       expected_identifier,
                                       expected_sequence))
    {
        reply->ignored = true;
        return true;
    }

    reply->packet_error = true;
    return true;
}

bool ft_ping_echo_reply(FtPingSocket *socket,
                        uint16_t expected_identifier,
                        uint16_t expected_sequence,
                        FtPingEchoReply *reply,
                        char *error_buffer,
                        size_t error_buffer_size)
{
    uint8_t packet[65535];
    struct sockaddr_in from;

    memset(reply, 0, sizeof(*reply));

#if defined(FT_PING_PLATFORM_WINDOWS)
    {
        int from_len = sizeof(from);
        int received = recvfrom(socket->handle, (char *)packet, sizeof(packet), 0,
                                (struct sockaddr *)&from, &from_len);

        if (received < 0)
        {
            int err = WSAGetLastError();

            if (err == WSAETIMEDOUT)
            {
                reply->timeout = true;
                return true;
            }

            snprintf(error_buffer, error_buffer_size, "Receive failed (WSA %d)", err);
            return false;
        }

        return ft_ping_parse_icmp_packet(packet,
                                         received,
                                         &from,
                                         expected_identifier,
                                         expected_sequence,
                                         reply,
                                         error_buffer,
                                         error_buffer_size);
    }
#else
    {
        socklen_t from_len = sizeof(from);
        ssize_t received = recvfrom(socket->fd, packet, sizeof(packet), 0,
                                    (struct sockaddr *)&from, &from_len);

        if (received < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                reply->timeout = true;
                return true;
            }

            snprintf(error_buffer, error_buffer_size, "Receive failed: %s", strerror(errno));
            return false;
        }

        return ft_ping_parse_icmp_packet(packet,
                                         (int)received,
                                         &from,
                                         expected_identifier,
                                         expected_sequence,
                                         reply,
                                         error_buffer,
                                         error_buffer_size);
    }
#endif
}
