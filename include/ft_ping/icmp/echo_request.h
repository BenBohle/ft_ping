#ifndef FT_PING_ICMP_ECHO_REQUEST_H
#define FT_PING_ICMP_ECHO_REQUEST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ft_ping/icmp/echo.h"
#include "ft_ping/platform/socket.h"

typedef struct FtPingEchoRequestParams
{
    const char *target;
    uint16_t identifier;
    uint16_t sequence;
    size_t payload_size;
} FtPingEchoRequestParams;

// ICMP Echo Request (Type 8)
// returnes number of bytes sended, -1 on error.
int ft_ping_send_echo_request(FtPingSocket *socket,
                              const FtPingEchoRequestParams *params,
                              struct sockaddr_in *dest_out,
                              char *error_buffer,
                              size_t error_buffer_size);

#endif /* FT_PING_ICMP_ECHO_REQUEST_H */
