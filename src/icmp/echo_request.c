#   define _POSIX_C_SOURCE 200112L

#include "ft_ping/icmp/echo_request.h"
#include "ft_ping/icmp/echo_utils.h"

#include <string.h>
#include <stdio.h>

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <sys/socket.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <arpa/inet.h>
#   include <errno.h>
#endif

int ft_ping_send_echo_request(FtPingSocket *socket,
                              const FtPingEchoRequestParams *params,
                              struct sockaddr_in *dest_out,
                              char *error_buffer,
                              size_t error_buffer_size)
{
    // zieladresse auflösen
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    struct addrinfo *addr_info;
    if (getaddrinfo(params->target, NULL, &hints, &addr_info) != 0)
    {
        snprintf(error_buffer, error_buffer_size, "Cannot resolve %s", params->target);
        return -1;
    }

    *dest_out = *(struct sockaddr_in *)addr_info->ai_addr;
    freeaddrinfo(addr_info);

    // check hier hin oder in parsing?
    size_t packet_size = sizeof(FtPingIcmpHeader) + params->payload_size;
    if (packet_size > 65535)
    {
        snprintf(error_buffer, error_buffer_size, "Packet too large (%zu bytes)", packet_size);
        return -1;
    }

    // maybe wie in inetutils nur einmal beim start und ann wiederverwenden? (payload buffer + header buffer)
    uint8_t packet[65535];
    memset(packet, 0, packet_size);

    FtPingIcmpHeader *icmp = (FtPingIcmpHeader *)packet;
    icmp->type     = 8;   // Echo Request
    icmp->code     = 0;
    icmp->identifier       = htons(params->identifier);
    icmp->sequence = htons(params->sequence);

    ft_ping_icmp_generate_payload((uint8_t *)packet + sizeof(FtPingIcmpHeader), params->payload_size);

    icmp->checksum = 0;
    icmp->checksum = ft_ping_icmp_checksum(packet, (size_t)packet_size);

    // Senden
#if defined(FT_PING_PLATFORM_WINDOWS)
    int sent = sendto(socket->handle, (const char *)packet, (int)packet_size, 0,
                      (struct sockaddr *)dest_out, sizeof(*dest_out));
    if (sent < 0)
    {
        snprintf(error_buffer, error_buffer_size, "Send failed (WSA %d)", WSAGetLastError());
        return -1;
    }
#else
    ssize_t sent = sendto(socket->fd, packet, (size_t)packet_size, 0,
                          (struct sockaddr *)dest_out, sizeof(*dest_out));
    if (sent < 0)
    {
        snprintf(error_buffer, error_buffer_size, "Send failed: %s", strerror(errno));
        return -1;
    }
#endif

    return (int)sent;
}
