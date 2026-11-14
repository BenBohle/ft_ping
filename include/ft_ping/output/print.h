#ifndef FT_PING_OUTPUT_PRINT_H
#define FT_PING_OUTPUT_PRINT_H

#include "ft_ping/core/ping.h"

void ft_ping_print_header(const FtPingSession *session);
void ft_ping_print_result(const FtPingSession *session, uint16_t sequence, const FtPingResult *result);
void ft_ping_print_stats(const FtPingSession *session);

#endif /* FT_PING_OUTPUT_PRINT_H */
