#include "ft_ping/cli/options.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool ft_ping_read_numeric_argument(char **argv, int argc, int *index, uintmax_t maximum,
                                          unsigned long *value, char *error_buffer, size_t error_buffer_size)
{
    const char *option = argv[*index];
    const char *argument;
    char *end = NULL;
    unsigned long parsed;

    if (*index + 1 >= argc || !argv[*index + 1])
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "option '%s' requires an argument", option);
        return false;
    }

    argument = argv[*index + 1];
    if (argument[0] == '-')
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "invalid argument '%s' for option '%s'", argument, option);
        return false;
    }

    errno = 0;
    parsed = strtoul(argument, &end, 10);
    if (end == argument || *end != '\0' || errno == ERANGE || (uintmax_t)parsed > maximum)
    {
        ft_ping_set_error(error_buffer, error_buffer_size, "invalid argument '%s' for option '%s'", argument, option);
        return false;
    }

    *index += 1;
    *value = parsed;
    return true;
}

void ft_ping_options_set_defaults(FtPingOptions *options)
{
    if (!options)
    {
        return;
    }

    memset(options, 0, sizeof(*options));
    options->ttl = FT_PING_DEFAULT_TTL;
    options->payload_bytes = FT_PING_DEFAULT_PAYLOAD_BYTES;
    options->timeout_ms = FT_PING_DEFAULT_TIMEOUT_MS;
    options->interval_ms = FT_PING_DEFAULT_INTERVAL_MS;
    options->count = 0; // unendlich
}

FtPingCliParseResult ft_ping_cli_parse(int argc, char **argv, FtPingOptions *options, char *error_buffer, size_t error_buffer_size)
{
    FtPingCliParseResult result = { true, false, EXIT_SUCCESS };

    for (int index = 1; index < argc; ++index)
    {
        const char *arg = argv[index];
        if (!arg)
        {
            continue;
        }

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0)
        {
            options->verbose = true;
        }
        else if (strcmp(arg, "-?") == 0 || strcmp(arg, "--help") == 0)
        {
            options->show_help = true;
            result.should_exit = true;
            break;
        }
        else if (strcmp(arg, "--version") == 0)
        {
            options->show_version = true;
            result.should_exit = true;
            break;
        }
        else if (strcmp(arg, "-c") == 0)
        {
            unsigned long value;

            if (!ft_ping_read_numeric_argument(argv, argc, &index, UINT32_MAX, &value, error_buffer, error_buffer_size))
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                break;
            }
            options->count = (uint32_t)value;
        }
        else if (strcmp(arg, "-s") == 0)
        {
            unsigned long value;

            if (!ft_ping_read_numeric_argument(argv, argc, &index, SIZE_MAX, &value, error_buffer, error_buffer_size))
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                break;
            }
            options->payload_bytes = (size_t)value;
        }
        else if (strcmp(arg, "-i") == 0)
        {
            unsigned long value;

            if (!ft_ping_read_numeric_argument(argv, argc, &index, UINT32_MAX, &value, error_buffer, error_buffer_size))
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                break;
            }
            options->interval_ms = (uint32_t)value;
        }
        else if (strcmp(arg, "-t") == 0)
        {
            unsigned long value;

            if (!ft_ping_read_numeric_argument(argv, argc, &index, UINT16_MAX, &value, error_buffer, error_buffer_size))
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                break;
            }
            options->ttl = (uint16_t)value;
        }
        else if (strcmp(arg, "-W") == 0)
        {
            unsigned long value;

            if (!ft_ping_read_numeric_argument(argv, argc, &index, UINT32_MAX, &value, error_buffer, error_buffer_size))
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                break;
            }
            options->timeout_ms = (uint32_t)value;
        }
        else if (strcmp(arg, "--windows") == 0)
        {
            options->windows_output = true;
        }
        else if (arg[0] == '-')
        {
            result.success = false;
            result.exit_code = EXIT_FAILURE;
            ft_ping_set_error(error_buffer, error_buffer_size, "unknown option '%s' provided", arg);
            break;
        }
        else
        {
            if (options->target[0] || strlen(arg) > FT_PING_MAX_HOST_LEN)
            {
                result.success = false;
                result.exit_code = EXIT_FAILURE;
                ft_ping_set_error(error_buffer, error_buffer_size,
                                  options->target[0] ? "multiple target hosts provided"
                                                     : "target host name too long");
                break;
            }
            strncpy(options->target, arg, FT_PING_MAX_HOST_LEN);
            options->target[FT_PING_MAX_HOST_LEN] = '\0';
        }
    }

    if (result.success && !result.should_exit && !options->target[0])
    {
        result.success = false;
        result.exit_code = EXIT_FAILURE;
        ft_ping_set_error(error_buffer, error_buffer_size, "missing target host or IP address");
    }

    return result;
}

void ft_ping_cli_print_usage(FILE *stream)
{
    if (!stream)
    {
        stream = stdout;
    }

    fprintf(stream,
            "Usage: ft_ping [options] <host>\n"
            "  -v, --verbose     Show the id and ICMP error headers\n"
            "  -c <count>        Number of requests (0 = infinite)\n"
            "  -s <bytes>        Payload size in bytes\n"
            "  -i <ms>           Interval in milliseconds\n"
            "  -t <ttl>          Time to live value\n"
            "  -W <ms>           Timeout in milliseconds\n"
            "      --windows     Windows-style text (default is inetutils-2.0)\n"
            "  -?, --help        Show this help\n"
            "      --version     Show version and exit\n");
}

void ft_ping_cli_print_version(FILE *stream)
{
    if (!stream)
    {
        stream = stdout;
    }

    fprintf(stream, "ft_ping version %u.%u.%u\n",
            FT_PING_VERSION_MAJOR,
            FT_PING_VERSION_MINOR,
            FT_PING_VERSION_PATCH);
}

