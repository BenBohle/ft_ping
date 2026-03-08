#ifndef FT_PING_ICMP_ECHO_REPLY_H
#define FT_PING_ICMP_ECHO_REPLY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ft_ping/icmp/echo.h"
#include "ft_ping/platform/socket.h"

// Result of a received Echo Reply (Type 0)
typedef struct FtPingEchoReply
{
    bool received;          // true = valid Echo Reply received
    bool timeout;           // true = timeout, no reply
    uint16_t identifier;
    uint16_t sequence;
    size_t payload;
    uint8_t ttl;
    struct sockaddr_in from;
} FtPingEchoReply;

// ICMP Echo Reply (Type 0)
// Returns true on success (also on timeout), false on error.
bool ft_ping_echo_reply(FtPingSocket *socket,
                        FtPingEchoReply *reply,
                        char *error_buffer,
                        size_t error_buffer_size);

#endif /* FT_PING_ICMP_ECHO_REPLY_H */