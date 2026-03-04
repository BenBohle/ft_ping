#include "ft_ping/utils/logger.h"

#include <time.h>

static bool g_log_debug_enabled = false;
static bool g_colors_enabled = true;

static const char *ft_ping_log_level_label(FtPingLogLevel level)
{
    switch (level)
    {
        case FT_PING_LOG_LEVEL_INFO: return "info";
        case FT_PING_LOG_LEVEL_WARN: return "warn";
        case FT_PING_LOG_LEVEL_ERROR: return "error";
        case FT_PING_LOG_LEVEL_DEBUG: return "debug";
        default: return "log";
    }
}


void ft_ping_set_error(char *buffer, size_t size, const char *format, ...)
{
    if (!buffer || size == 0 || !format)
        return;

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);
}

const char *ft_ping_log_level_color(FtPingLogLevel level)
{
    if (!g_colors_enabled)
        return "";

    switch (level)
    {
        case FT_PING_LOG_LEVEL_INFO: return FT_PING_COLOR_GREEN;
        case FT_PING_LOG_LEVEL_WARN: return FT_PING_COLOR_YELLOW;
        case FT_PING_LOG_LEVEL_ERROR: return FT_PING_COLOR_RED;
        case FT_PING_LOG_LEVEL_DEBUG: return FT_PING_COLOR_BLUE;
        default: return "";
    }
}

static FILE *ft_ping_log_level_stream(FtPingLogLevel level)
{
    if (level == FT_PING_LOG_LEVEL_ERROR)
    {
        return stderr;
    }

    return stdout;
}

void ft_ping_log_set_debug(bool enabled)
{
    g_log_debug_enabled = enabled;
}

void ft_ping_log_init_colors(void)
{
#if defined(FT_PING_PLATFORM_WINDOWS)
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;
    
    /* Try to enable Virtual Terminal Processing on Windows 10+ */
    if (GetConsoleMode(stdout_handle, &mode))
    {
        SetConsoleMode(stdout_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    if (GetConsoleMode(stderr_handle, &mode))
    {
        SetConsoleMode(stderr_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

void ft_ping_log_message_v(FtPingLogLevel level, const char *format, va_list args)
{
    if (level == FT_PING_LOG_LEVEL_DEBUG && !g_log_debug_enabled)
    {
        return;
    }

    FILE *stream = ft_ping_log_level_stream(level);

    const time_t raw_time = time(NULL);
    struct tm time_info;
#if defined(FT_PING_PLATFORM_WINDOWS)
    localtime_s(&time_info, &raw_time);
#else
    localtime_r(&raw_time, &time_info);
#endif
    char time_buffer[32];
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &time_info);

    fprintf(stream, "[%s %s %s] %s[%s]%s ", FT_PING_COLOR_GRAY, time_buffer, FT_PING_COLOR_RESET, ft_ping_log_level_color(level), ft_ping_log_level_label(level), g_colors_enabled ? FT_PING_COLOR_RESET : "");
    vfprintf(stream, format, args);
    fputc('\n', stream);
}

void ft_ping_log_message(FtPingLogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    ft_ping_log_message_v(level, format, args);
    va_end(args);
}

