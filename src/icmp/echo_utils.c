#include "ft_ping/icmp/echo_utils.h"
#include "ft_ping/config.h"

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#else
#   include <sys/time.h>
#endif

double ft_ping_get_time_ms(void)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    return (double)GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#endif
}

uint16_t ft_ping_icmp_checksum(const void *data, size_t len)
{
    uint16_t *buf = (uint16_t *)data;
    uint32_t sum = 0;
    
    // Alle 16-bit Wörter addieren
    for (size_t i = 0; i < len / 2; i++)
    {
        sum += buf[i];
    }
    
    // Falls ungerade Länge, letztes Byte dazu
    if (len % 2)
    {
        sum += ((uint8_t *)data)[len - 1];
    }
    
    // Carry-Bits zurückfalten
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return (uint16_t)~sum;  // One's Complement
}

void ft_ping_icmp_generate_payload(uint8_t *payload, size_t payload_size)
{
    const char pattern[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    size_t pattern_len = sizeof(pattern) - 1;

     for (size_t i = 0; i < payload_size; i++) {
        payload[i] = (uint8_t) pattern[i % pattern_len];
    }
}
