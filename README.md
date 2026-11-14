# ft_ping

[Deutsch](README.de.md)

`ft_ping` is ping, written for Linux, macOS, and Windows. One codebase, one command, three platforms — the same reachability check wherever you work.

## Features

- IPv4 ICMP Echo (type 8 request, type 0 reply) with identifier, sequence number, and checksum.
- Hostnames and numeric IPv4 addresses. Names are resolved once with `getaddrinfo`; reply lines show the address, not a second reverse lookup.
- Round-trip time in milliseconds, plus minimum, average, maximum, and standard deviation.
- ICMP errors such as time exceeded and destination unreachable. With `-v`, the quoted IP header and the embedded ICMP header are printed as well.
- Graceful stop on Ctrl+C. Statistics are printed before the process exits.
- Two output dialects from the same session: inetutils (default) and Windows (`--windows`).
- A small platform layer for sockets, timers, and process setup, so the ping loop stays the same on every OS.

## Requirements

- CMake 3.21 or newer
- A C11 compiler (GCC, Clang, or MSVC)
- `make`, if you use the Makefile wrapper

ICMP needs a privileged socket on some systems:

| Platform | Socket | Privilege |
| --- | --- | --- |
| Linux | Raw ICMP (`SOCK_RAW`) | Root, or the `CAP_NET_RAW` capability |
| Windows | Raw ICMP via Winsock | An elevated prompt |
| macOS | ICMP datagram (`SOCK_DGRAM`) | The kernel owns the checksum and the identifier. A normal user session is enough on current macOS. |

## Build

The Makefile configures CMake and copies the binary to `./ft_ping` (or `ft_ping.exe` on Windows).

```sh
make
./ft_ping -?
```

CMake directly:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/ft_ping -?
```

On Windows the Makefile selects the Ninja generator. CMake, Ninja, and a C compiler must be on `PATH`.

| Target | Effect |
| --- | --- |
| `make` / `make all` | Release build, binary copied next to the Makefile |
| `make debug` | Same, with `CMAKE_BUILD_TYPE=Debug` |
| `make run` | Build, then start `ft_ping` with no arguments (prints usage) |
| `make clean` | Clean the CMake build |
| `make fclean` | Remove `build/` and the copied binary |
| `make re` | `fclean`, then `all` |

Warnings are on by default (`-Wall -Wextra -Wpedantic`, or `/W4` with MSVC). To treat them as errors:

```sh
cmake -S . -B build -DFT_PING_WARNINGS_AS_ERRORS=ON
```

### Docker

The image is a Debian Bookworm toolchain. It also installs `inetutils-ping`, so the reference tool is available next to `ft_ping`. The source tree is mounted at runtime; the image does not copy it in.

```sh
docker build -t ft_ping-dev .
docker run --rm -it \
  --cap-add NET_RAW --cap-add NET_ADMIN \
  -v "$PWD":/ft_ping \
  ft_ping-dev
make
./ft_ping -c 2 127.0.0.1
```

`NET_RAW` is required inside the container because Linux uses a raw ICMP socket.

## Usage

```text
ft_ping [options] <host>
```

```sh
ft_ping 127.0.0.1
ft_ping -c 4 1.1.1.1
ft_ping -v -t 1 example.com
ft_ping -s 1200 -i 500 -W 2000 8.8.8.8
ft_ping --windows -c 4 8.8.8.8
```

`-i` and `-W` take milliseconds. inetutils uses seconds for those two options; the values here are intentional and match the usage text.

Without `-c`, `ft_ping` keeps sending until Ctrl+C. The exit status is `0` when at least one Echo Reply arrived. It is non-zero when argument parsing fails, the session cannot start, or no reply was received.

### Options

| Option | Default | Meaning |
| --- | --- | --- |
| `-v`, `--verbose` | off | Print the ICMP identifier in the header, and dump the quoted IP and ICMP headers on errors |
| `-c <count>` | `0` | Stop after this many requests. `0` means no limit |
| `-s <bytes>` | `56` | ICMP payload size |
| `-i <ms>` | `1000` | Gap between requests, in milliseconds, measured from send time |
| `-t <ttl>` | `64` | IPv4 time to live |
| `-W <ms>` | `1000` | How long to wait for one reply, in milliseconds |
| `--windows` | off | Windows-style report instead of inetutils |
| `-?`, `--help` | | Print usage and exit |
| `--version` | | Print `ft_ping version 0.1.0` and exit |

One host per invocation. A second target, an unknown flag, or a missing value is an error and the usage text is printed to stderr.

### Output

Default (inetutils 2.0):

```text
PING example.com (93.184.216.34): 56 data bytes
64 bytes from 93.184.216.34: icmp_seq=0 ttl=56 time=12.340 ms
--- example.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 12.340/12.340/12.340/0.000 ms
```

With `-v`, the header also includes `id 0x.... = <pid>`. On ICMP errors the line names the condition (`Time to live exceeded`, `Destination Host Unreachable`, and similar), then prints an IP header dump and the quoted ICMP header.

`--windows`:

```text
PING example.com: 56 data bytes
64 bytes from 93.184.216.34: icmp_seq=0 ttl=56 time=12.34 ms

Ping statistics for example.com:
    Packets: Sent = 1, Received = 1, Lost = 0
    (0% loss),
Approximate round trip times in milli-seconds:
    Minimum = 12.34ms, Maximum = 12.34ms, Average = 12.34ms
```

Timed-out probes are silent in the inetutils layout and printed as `Request timeout for icmp_seq N` in the Windows layout. Both layouts count them as loss.

## How it works

Each probe follows the same path:

1. Resolve the host to an IPv4 address.
2. Open the platform ICMP socket and set TTL and receive timeout.
3. Build an Echo Request. The identifier is the process id. The sequence starts at 0. The checksum is computed in user space on Linux and Windows; on macOS the kernel requires a zero checksum and fills it in.
4. Send the packet, wait for a matching Echo Reply or a related ICMP error, and record the round trip.
5. Sleep until the interval has elapsed since the send, then repeat.
6. On count, timeout of the loop, or SIGINT, print the summary and close the socket.

Linux and Windows match replies by identifier and sequence. macOS rewrites the identifier on `SOCK_DGRAM` ICMP, so replies are matched by sequence only.

## Layout

```text
include/ft_ping/     Public headers (CLI, session, ICMP, output, platform)
src/main.c           Argument dispatch, send loop, signals
src/cli/             Option parsing and usage text
src/core/            Name resolution, session, one probe
src/icmp/            Echo request, echo reply, checksum, payload
src/output/          inetutils and Windows printers
src/platform/        Sockets and process setup per OS
src/utils/           Error strings and logging
```

The ping loop links a static library (`ft_ping_lib`). Platform differences stay behind that library: Winsock on Windows (`ws2_32`, `iphlpapi`), Darwin ICMP datagrams, and Linux raw sockets.

## Scope

`ft_ping` covers the inetutils-style probe loop used for everyday reachability checks: count, size, interval, TTL, per-reply timeout, verbose ICMP errors, and a final RTT summary.

It is IPv4 only, accepts one destination, and implements the options listed above. The text is modeled on inetutils 2.0 and on Windows `ping.exe`; it is a readable match for those tools, not a byte-for-byte copy of every banner and spacing detail.
