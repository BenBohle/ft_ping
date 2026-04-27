#ifndef FT_PING_PLATFORM_SOCKET_H
#define FT_PING_PLATFORM_SOCKET_H

#include "ft_ping/config.h"

#if defined(FT_PING_PLATFORM_WINDOWS)
#   include "ft_ping/platform/windows_support.h"
#   include <mstcpip.h>
#   include <iphlpapi.h>
#else
#   include <unistd.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <netinet/ip.h>
#   include <netinet/ip_icmp.h>
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <sys/types.h>
#endif

typedef struct FtPingSocket
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    SOCKET handle;
#else
    int fd;
#endif
    bool is_open;
} FtPingSocket;

typedef struct FtPingSocketConfig
{
    uint16_t ttl;
    size_t payload_bytes;
    uint32_t timeout_ms;
} FtPingSocketConfig;

void ft_ping_socket_init(FtPingSocket *sock);
bool ft_ping_socket_open(FtPingSocket *sock, const FtPingSocketConfig *config, const char *target, char *error_buffer, size_t error_buffer_size);
bool ft_ping_socket_set_timeout_ms(FtPingSocket *sock, uint32_t timeout_ms, char *error_buffer, size_t error_buffer_size);
void ft_ping_socket_close(FtPingSocket *sock);
bool ft_ping_socket_is_open(const FtPingSocket *sock);

#endif /* FT_PING_PLATFORM_SOCKET_H */
