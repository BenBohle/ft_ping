#include "ft_ping/icmp/echo_reply.h"

#include <stdio.h>
#include <string.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <errno.h>
#   include <sys/socket.h>
#endif

static bool ft_ping_reply_matches_request(uint16_t identifier, uint16_t sequence,
                                          uint16_t expected_identifier,
                                          uint16_t expected_sequence)
{
#if defined(FT_PING_PLATFORM_DARWIN)
    /* The kernel rewrites the identifier on SOCK_DGRAM ICMP and only returns our packets. */
    (void)identifier;
    (void)expected_identifier;
    return sequence == expected_sequence;
#else
    return identifier == expected_identifier && sequence == expected_sequence;
#endif
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

static void ft_ping_save_quoted_packet(const FtPingIcmpHeader *icmp, int icmp_len, FtPingEchoReply *reply)
{
    const uint8_t *embedded_ip = (const uint8_t *)icmp + sizeof(*icmp);
    int embedded_bytes = icmp_len - (int)sizeof(*icmp);
    int ip_len;
    const uint8_t *embedded_icmp;

    if (embedded_bytes < 20)
    {
        return;
    }

    ip_len = (embedded_ip[0] & 0x0F) * 4;
    if (ip_len < 20 || embedded_bytes < ip_len + (int)sizeof(FtPingIcmpHeader))
    {
        return;
    }

    reply->has_ip_header = true;
    reply->ip_header_len = (size_t)ip_len;
    if (reply->ip_header_len > sizeof(reply->ip_header))
    {
        reply->ip_header_len = sizeof(reply->ip_header);
    }
    memcpy(reply->ip_header, embedded_ip, reply->ip_header_len);

    embedded_icmp = embedded_ip + ip_len;
    reply->has_quoted_icmp = true;
    reply->quoted_type = embedded_icmp[0];
    reply->quoted_code = embedded_icmp[1];
    reply->quoted_size = (size_t)(embedded_bytes - ip_len);
    reply->quoted_id = (uint16_t)((embedded_icmp[4] << 8) | embedded_icmp[5]);
    reply->quoted_seq = (uint16_t)((embedded_icmp[6] << 8) | embedded_icmp[7]);
}

static bool ft_ping_packet_has_ip_header(const uint8_t *packet, int received, int *ip_header_len)
{
    int header_len;

    if (received < 20 || (packet[0] >> 4) != 4)
    {
        return false;
    }

    header_len = (packet[0] & 0x0F) * 4;
    if (header_len < 20 || received < header_len + (int)sizeof(FtPingIcmpHeader))
    {
        return false;
    }

    if (packet[9] != IPPROTO_ICMP)
    {
        return false;
    }

    *ip_header_len = header_len;
    return true;
}

static bool ft_ping_parse_icmp_packet(const uint8_t *packet,
                                      int received,
                                      const struct sockaddr_in *from,
                                      int ttl_hint,
                                      uint16_t expected_identifier,
                                      uint16_t expected_sequence,
                                      FtPingEchoReply *reply)
{
    int ip_header_len = 0;
    int icmp_offset = 0;
    int icmp_len;
    const FtPingIcmpHeader *icmp;
    bool has_ip_header;

    has_ip_header = ft_ping_packet_has_ip_header(packet, received, &ip_header_len);
    if (has_ip_header)
    {
        icmp_offset = ip_header_len;
        reply->has_ip_header = true;
        reply->ip_header_len = (size_t)ip_header_len;
        if (reply->ip_header_len > sizeof(reply->ip_header))
        {
            reply->ip_header_len = sizeof(reply->ip_header);
        }
        memcpy(reply->ip_header, packet, reply->ip_header_len);
        reply->ttl = packet[8];
    }
    else if (ttl_hint >= 0 && ttl_hint <= 255)
    {
        reply->ttl = (uint8_t)ttl_hint;
    }

    icmp_len = received - icmp_offset;
    if (icmp_len < (int)sizeof(*icmp))
    {
        reply->ignored = true;
        return true;
    }

    icmp = (const FtPingIcmpHeader *)(packet + icmp_offset);
    reply->from = *from;
    reply->type = icmp->type;
    reply->code = icmp->code;
    reply->payload = (size_t)icmp_len;

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

    ft_ping_save_quoted_packet(icmp, icmp_len, reply);
    reply->packet_error = true;
    return true;
}

#if !defined(FT_PING_PLATFORM_WINDOWS)
static int ft_ping_ttl_from_control(struct msghdr *message)
{
#if defined(FT_PING_PLATFORM_DARWIN) && defined(IP_RECVTTL)
    struct cmsghdr *header;

    for (header = CMSG_FIRSTHDR(message); header != NULL; header = CMSG_NXTHDR(message, header))
    {
        int value;

        if (header->cmsg_level != IPPROTO_IP || header->cmsg_type != IP_RECVTTL)
        {
            continue;
        }
        if (header->cmsg_len < CMSG_LEN(sizeof(value)))
        {
            continue;
        }

        memcpy(&value, CMSG_DATA(header), sizeof(value));
        return value;
    }
#else
    (void)message;
#endif

    return -1;
}
#endif

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
    memset(&from, 0, sizeof(from));

#if defined(FT_PING_PLATFORM_WINDOWS)
    {
        int from_len = sizeof(from);
        int received = recvfrom(socket->handle, (char *)packet, sizeof(packet), 0,
                                (struct sockaddr *)&from, &from_len);

        if (received < 0)
        {
            int err = WSAGetLastError();

            if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
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
                                         -1,
                                         expected_identifier,
                                         expected_sequence,
                                         reply);
    }
#else
    {
        union
        {
            char bytes[256];
            struct cmsghdr align;
        } control;
        struct iovec io;
        struct msghdr message;
        ssize_t received;

        memset(&message, 0, sizeof(message));
        memset(&control, 0, sizeof(control));
        io.iov_base = packet;
        io.iov_len = sizeof(packet);
        message.msg_name = &from;
        message.msg_namelen = sizeof(from);
        message.msg_iov = &io;
        message.msg_iovlen = 1;
        message.msg_control = control.bytes;
        message.msg_controllen = sizeof(control.bytes);

        received = recvmsg(socket->fd, &message, 0);
        if (received < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK || errno == ETIMEDOUT)
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
                                         ft_ping_ttl_from_control(&message),
                                         expected_identifier,
                                         expected_sequence,
                                         reply);
    }
#endif
}
