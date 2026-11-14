#ifndef FT_PING_CLI_OPTIONS_H
#define FT_PING_CLI_OPTIONS_H

#include <stdio.h>

#include "ft_ping/config.h"
#include "ft_ping/utils/logger.h"

typedef struct FtPingOptions
{
    char target[FT_PING_MAX_HOST_LEN + 1];
    bool verbose;
    bool windows_output;
    bool show_help;
    bool show_version;
    uint16_t ttl;
    uint32_t count;
    size_t payload_bytes;
    uint32_t timeout_ms;
    uint32_t interval_ms;
} FtPingOptions;

typedef struct FtPingCliParseResult
{
    bool success;
    bool should_exit;
    int exit_code;
} FtPingCliParseResult;

void ft_ping_options_set_defaults(FtPingOptions *options);
FtPingCliParseResult ft_ping_cli_parse(int argc, char **argv, FtPingOptions *options, char *error_buffer, size_t error_buffer_size);

void ft_ping_cli_print_usage(FILE *stream);
void ft_ping_cli_print_version(FILE *stream);

#endif /* FT_PING_CLI_OPTIONS_H */
