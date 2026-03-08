#ifndef FT_PING_PLATFORM_WINDOWS_SUPPORT_H
#define FT_PING_PLATFORM_WINDOWS_SUPPORT_H

#include "ft_ping/config.h"

// WIN32_LEAN_AND_MEAN excludes rarely-used services for faster compilation
// NOMINMAX prevents Windows headers from defining min/max macros that can interfere with std::min/std::max
// winsock has to be included before windows.h, sonst gibts Fehler wegen winsock2.h
#if defined(FT_PING_PLATFORM_WINDOWS)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   include <windows.h>
#endif

#endif /* FT_PING_PLATFORM_WINDOWS_SUPPORT_H */
