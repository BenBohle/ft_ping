#include "ft_ping/icmp/echo_reply.h"

#include <string.h>
#include <stdio.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <sys/socket.h>
#   include <errno.h>
#endif

bool ft_ping_echo_reply(FtPingSocket *socket,
                        FtPingEchoReply *reply,
                        char *error_buffer,
                        size_t error_buffer_size)
{
    memset(reply, 0, sizeof(*reply));

    uint8_t recv_buffer[65535];
    struct sockaddr_in from;

#if defined(FT_PING_PLATFORM_WINDOWS)
    int from_len = sizeof(from);
    int received = recvfrom(socket->handle, (char *)recv_buffer, sizeof(recv_buffer), 0,
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
#else
    socklen_t from_len = sizeof(from);
    ssize_t received = recvfrom(socket->fd, recv_buffer, sizeof(recv_buffer), 0,
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
#endif

    // IP-Header-Länge auslesen byte 0 und untere 4 bits * 4
    int ip_header_len = (recv_buffer[0] & 0x0F) * 4;
    int icmp_len = (int)received - ip_header_len;

    if (icmp_len < (int)sizeof(FtPingIcmpHeader))
    {
        snprintf(error_buffer, error_buffer_size, "Packet too short (%d bytes)", icmp_len);
        return false;
    }

    FtPingIcmpHeader *icmp = (FtPingIcmpHeader *)(recv_buffer + ip_header_len);

    // error werfen oder skippen einfach?
    if (icmp->type != 0 || icmp->code != 0)
    {
        snprintf(error_buffer, error_buffer_size, "Not an Echo Reply (type=%u code=%u)",
            icmp->type, icmp->code);
        return false;
    }

    reply->received = true;
    reply->timeout  = false;
    reply->identifier = ntohs(icmp->identifier);
    reply->sequence = ntohs(icmp->sequence);
    reply->payload = (size_t)icmp_len;
    reply->ttl = recv_buffer[8];
    reply->from = from;

    return true;
}
