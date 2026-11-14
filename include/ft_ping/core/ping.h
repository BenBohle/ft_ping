#ifndef FT_PING_CORE_PING_H
#define FT_PING_CORE_PING_H

#include <signal.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ft_ping/core/session.h"

typedef struct FtPingResult
{
    bool received;
    bool timeout;
    bool packet_error;
    bool interrupted;
    bool has_ip_header;
    bool has_quoted_icmp;
    size_t payload;
    size_t ip_header_len;
    size_t quoted_size;
    uint16_t quoted_id;
    uint16_t quoted_seq;
    uint8_t ttl;
    uint8_t quoted_type;
    uint8_t quoted_code;
    uint8_t ip_header[FT_PING_IP_HEADER_MAX];
    double rtt_ms;
    double sent_at_ms;
    char from[46];
    char error_text[128];
} FtPingResult;

extern volatile sig_atomic_t ft_ping_stop;

// Sends Echo Request, receive Echo Reply, tracks RTT.
// returns true on success & on timeout, false on error.
bool ft_ping(FtPingSession *session,
             uint16_t sequence,
             FtPingResult *result,
             char *error_buffer,
             size_t error_buffer_size);

#endif
