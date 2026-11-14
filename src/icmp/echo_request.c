#include "ft_ping/icmp/echo_request.h"
#include "ft_ping/icmp/echo_utils.h"

#include <stdio.h>
#include <string.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <errno.h>
#   include <sys/socket.h>
#endif

int ft_ping_send_echo_request(FtPingSocket *socket,
                              const FtPingEchoRequestParams *params,
                              const struct sockaddr_in *destination,
                              char *error_buffer,
                              size_t error_buffer_size)
{
    size_t packet_size = sizeof(FtPingIcmpHeader) + params->payload_size;
    uint8_t packet[65535];
    FtPingIcmpHeader *icmp;

    if (!destination)
    {
        snprintf(error_buffer, error_buffer_size, "missing destination");
        return -1;
    }

    if (packet_size > sizeof(packet))
    {
        snprintf(error_buffer, error_buffer_size, "Packet too large (%zu bytes)", packet_size);
        return -1;
    }

    memset(packet, 0, packet_size);

    icmp = (FtPingIcmpHeader *)packet;
    icmp->type = 8;
    icmp->code = 0;
    icmp->identifier = htons(params->identifier);
    icmp->sequence = htons(params->sequence);
    ft_ping_icmp_generate_payload(packet + sizeof(FtPingIcmpHeader), params->payload_size);

#if defined(FT_PING_PLATFORM_DARWIN)
    /* SOCK_DGRAM ICMP: the kernel fills the checksum and rejects a non-zero value. */
    icmp->checksum = 0;
#else
    icmp->checksum = 0;
    icmp->checksum = ft_ping_icmp_checksum(packet, packet_size);
#endif

#if defined(FT_PING_PLATFORM_WINDOWS)
    {
        int sent = sendto(socket->handle, (const char *)packet, (int)packet_size, 0,
                          (const struct sockaddr *)destination, sizeof(*destination));
        if (sent < 0)
        {
            snprintf(error_buffer, error_buffer_size, "Send failed (WSA %d)", WSAGetLastError());
            return -1;
        }
        return sent;
    }
#else
    {
        ssize_t sent = sendto(socket->fd, packet, packet_size, 0,
                              (const struct sockaddr *)destination, sizeof(*destination));
        if (sent < 0)
        {
            snprintf(error_buffer, error_buffer_size, "Send failed: %s", strerror(errno));
            return -1;
        }
        return (int)sent;
    }
#endif
}
