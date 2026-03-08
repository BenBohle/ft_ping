#ifndef FT_PING_CORE_PING_H
#define FT_PING_CORE_PING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ft_ping/core/session.h"

typedef struct FtPingResult
{
    bool received;
    bool timeout;
    size_t payload;
    uint8_t ttl;
    double rtt_ms;      // Round-Trip-Time in ms
    char from[46];
} FtPingResult;

// Sends Echo Request, receive Echo Reply, tracks RTT.
// returns true on success & on timeout, false on error.
bool ft_ping(FtPingSession *session,
             uint16_t sequence,
             FtPingResult *result,
             char *error_buffer,
             size_t error_buffer_size);

#endif
