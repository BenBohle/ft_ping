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
    bool packet_error;      // true = relevant ICMP error for our request
    bool ignored;           // true = unrelated packet, keep waiting
    uint16_t identifier;
    uint16_t sequence;
    size_t payload;
    uint8_t ttl;
    uint8_t type;
    uint8_t code;
    struct sockaddr_in from;
} FtPingEchoReply;

// ICMP Echo Reply (Type 0)
// Returns true on success (also on timeout), false on error.
bool ft_ping_echo_reply(FtPingSocket *socket,
                        uint16_t expected_identifier,
                        uint16_t expected_sequence,
                        FtPingEchoReply *reply,
                        char *error_buffer,
                        size_t error_buffer_size);

#endif /* FT_PING_ICMP_ECHO_REPLY_H */