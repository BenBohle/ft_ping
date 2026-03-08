#ifndef FT_PING_UTILS_LOGGER_H
#define FT_PING_UTILS_LOGGER_H

#include <stdarg.h>
#include <stdio.h>

#include "ft_ping/config.h"

/* ANSI color codes */
#define FT_PING_COLOR_RESET   "\033[0m"
#define FT_PING_COLOR_RED     "\033[31m"      /* Error */
#define FT_PING_COLOR_YELLOW  "\033[33m"      /* Warning */
#define FT_PING_COLOR_GREEN   "\033[32m"      /* Info */
#define FT_PING_COLOR_BLUE    "\033[34m"      /* Debug */
#define FT_PING_COLOR_GRAY    "\033[2m"       /* Dim/Faded - "unimportant" Infos */

typedef enum FtPingLogLevel
{
    FT_PING_LOG_LEVEL_INFO,
    FT_PING_LOG_LEVEL_WARN,
    FT_PING_LOG_LEVEL_ERROR,
    FT_PING_LOG_LEVEL_DEBUG
} FtPingLogLevel;

void ft_ping_log_set_debug(bool enabled);
void ft_ping_log_init_colors(void);
void ft_ping_log_message(FtPingLogLevel level, const char *format, ...);
void ft_ping_log_message_v(FtPingLogLevel level, const char *format, va_list args);
void ft_ping_set_error(char *buffer, size_t size, const char *format, ...);

#endif /* FT_PING_UTILS_LOGGER_H */

