#ifndef FT_PING_ICMP_ECHO_UTILS_H
#define FT_PING_ICMP_ECHO_UTILS_H

#include <stdint.h>
#include <stddef.h>

uint16_t ft_ping_icmp_checksum(const void *data, size_t len);
void ft_ping_icmp_generate_payload(uint8_t *payload, size_t payload_size);
double ft_ping_get_time_ms(void);

#endif /* FT_PING_ICMP_ECHO_UTILS_H */
