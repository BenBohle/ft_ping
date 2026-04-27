#include "ft_ping/platform/socket.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ft_ping/utils/logger.h"

void ft_ping_socket_init(FtPingSocket *sock)
{

#if defined(FT_PING_PLATFORM_WINDOWS)
    sock->handle = INVALID_SOCKET;
#else
    sock->fd = -1;
#endif
    sock->is_open = false;
}

static bool ft_ping_socket_apply_ttl(FtPingSocket *sock, const FtPingSocketConfig *config, char *error_buffer, size_t error_buffer_size)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    DWORD ttl = config->ttl;
    if (setsockopt(sock->handle, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "failed to set TTL (error %d)", WSAGetLastError());
        return false;
    }
#else
    int ttl = (int)config->ttl;
    if (setsockopt(sock->fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "failed to set TTL (%s)", strerror(errno));
        return false;
    }
#endif

    return true;
}

static bool ft_ping_socket_apply_timeout(FtPingSocket *sock, const FtPingSocketConfig *config, char *error_buffer, size_t error_buffer_size)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    DWORD timeout = config->timeout_ms;
    if (setsockopt(sock->handle, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "failed to set timeout (error %d)", WSAGetLastError());
        return false;
    }
#else
    struct timeval tv;
    tv.tv_sec = (long)(config->timeout_ms / 1000u);
    tv.tv_usec = (long)((config->timeout_ms % 1000u) * 1000u);
    if (setsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "failed to set timeout (%s)", strerror(errno));
        return false;
    }
#endif

    return true;
}

bool ft_ping_socket_set_timeout_ms(FtPingSocket *sock, uint32_t timeout_ms, char *error_buffer, size_t error_buffer_size)
{
    FtPingSocketConfig config = { 0 };

    if (!sock || !ft_ping_socket_is_open(sock))
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "socket is not open");
        return false;
    }

    config.timeout_ms = timeout_ms;
    return ft_ping_socket_apply_timeout(sock, &config, error_buffer, error_buffer_size);
}

bool ft_ping_socket_open(FtPingSocket *sock, const FtPingSocketConfig *config, const char *target, char *error_buffer, size_t error_buffer_size)
{
    if (!sock)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "internal error: socket pointer is null");
        return false;
    }

    ft_ping_socket_close(sock);

#if defined(FT_PING_PLATFORM_WINDOWS)
    SOCKET handle = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, 0);
    if (handle == INVALID_SOCKET)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "WSASocket failed (%d)", WSAGetLastError());
        return false;
    }
    sock->handle = handle;
#else
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "socket error (%s)", strerror(errno));
        return false;
    }
    sock->fd = fd;
#endif

    sock->is_open = true;

    if (config)
    {
        if (!ft_ping_socket_apply_ttl(sock, config, error_buffer, error_buffer_size) ||
            !ft_ping_socket_apply_timeout(sock, config, error_buffer, error_buffer_size))
        {
            ft_ping_socket_close(sock);
            return false;
        }
    }

    if (target && target[0] != '\0')
    {
        ft_ping_log_message(FT_PING_LOG_LEVEL_DEBUG, "socket prepared for %s", target);
    }

    return true;
}

void ft_ping_socket_close(FtPingSocket *sock)
{

#if defined(FT_PING_PLATFORM_WINDOWS)
    if (sock->handle != INVALID_SOCKET)
    {
        closesocket(sock->handle);
        sock->handle = INVALID_SOCKET;
    }
#else
    if (sock->fd >= 0)
    {
        close(sock->fd);
        sock->fd = -1;
    }
#endif
    sock->is_open = false;
}

bool ft_ping_socket_is_open(const FtPingSocket *sock)
{
    return sock && sock->is_open;
}


