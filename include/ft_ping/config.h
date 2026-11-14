#ifndef FT_PING_CONFIG_H
#define FT_PING_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef FT_PING_VERSION_MAJOR
#   define FT_PING_VERSION_MAJOR 0
#endif
#ifndef FT_PING_VERSION_MINOR
#   define FT_PING_VERSION_MINOR 1
#endif
#ifndef FT_PING_VERSION_PATCH
#   define FT_PING_VERSION_PATCH 0
#endif

#if !defined(FT_PING_PLATFORM_WINDOWS) && !defined(FT_PING_PLATFORM_DARWIN) && !defined(FT_PING_PLATFORM_LINUX)
#   if defined(_WIN32) || defined(_WIN64)
#       define FT_PING_PLATFORM_WINDOWS
#   elif defined(__APPLE__)
#       define FT_PING_PLATFORM_DARWIN
#   else
#       define FT_PING_PLATFORM_LINUX
#   endif
#endif

#if defined(FT_PING_PLATFORM_WINDOWS)
#   define FT_PING_OS_NAME "Windows"
#elif defined(FT_PING_PLATFORM_DARWIN)
#   define FT_PING_OS_NAME "macOS"
#else
#   define FT_PING_OS_NAME "Linux"
#endif

#define FT_PING_DEFAULT_PAYLOAD_BYTES 56u
#define FT_PING_DEFAULT_TTL 64u
#define FT_PING_DEFAULT_TIMEOUT_MS 1000u
#define FT_PING_DEFAULT_INTERVAL_MS 1000u
#define FT_PING_MAX_HOST_LEN 255u
#define FT_PING_IP_HEADER_MAX 60u

#define FT_PING_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FT_PING_UNUSED(value) ((void)(value))

#endif /* FT_PING_CONFIG_H */
