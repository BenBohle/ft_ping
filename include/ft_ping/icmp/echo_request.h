#ifndef FT_PING_ICMP_ECHO_REQUEST_H
#define FT_PING_ICMP_ECHO_REQUEST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ft_ping/icmp/echo.h"
#include "ft_ping/platform/socket.h"

typedef struct FtPingEchoRequestParams
{
    uint16_t identifier;
    uint16_t sequence;
    size_t payload_size;
} FtPingEchoRequestParams;

// ICMP Echo Request (Type 8). destination is already resolved.
// Returns the number of bytes sent, or -1 on error.
int ft_ping_send_echo_request(FtPingSocket *socket,
                              const FtPingEchoRequestParams *params,
                              const struct sockaddr_in *destination,
                              char *error_buffer,
                              size_t error_buffer_size);

#endif /* FT_PING_ICMP_ECHO_REQUEST_H */
