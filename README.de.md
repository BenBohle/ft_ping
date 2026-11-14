# ft_ping

[English](README.md)

`ft_ping` ist Ping, geschrieben für Linux, macOS und Windows. Eine Codebasis, ein Befehl, drei Plattformen — dieselbe Erreichbarkeitsprüfung, egal wo du arbeitest.

## Funktionen

- IPv4-ICMP-Echo (Request Typ 8, Reply Typ 0) mit Identifier, Sequenznummer und Prüfsumme.
- Hostnamen und numerische IPv4-Adressen. Der Name wird einmal über `getaddrinfo` aufgelöst; in den Antwortzeilen steht die Adresse, ohne einen zweiten Reverse-Lookup.
- Laufzeit in Millisekunden, dazu Minimum, Mittelwert, Maximum und Standardabweichung.
- ICMP-Fehler wie „time exceeded“ und „destination unreachable“. Mit `-v` werden zusätzlich der zitierte IP-Header und der eingebettete ICMP-Header ausgegeben.
- Sauberes Beenden mit Strg+C. Die Statistik erscheint, bevor der Prozess endet.
- Zwei Ausgabeformen aus derselben Sitzung: inetutils (Standard) und Windows (`--windows`).
- Eine schmale Plattformschicht für Sockets, Zeit und Prozessstart, damit die Ping-Schleife auf jedem System gleich bleibt.

## Voraussetzungen

- CMake 3.21 oder neuer
- Ein C11-Compiler (GCC, Clang oder MSVC)
- `make`, wenn der Makefile-Wrapper genutzt wird

ICMP braucht auf manchen Systemen einen privilegierten Socket:

| Plattform | Socket | Recht |
| --- | --- | --- |
| Linux | Raw ICMP (`SOCK_RAW`) | Root oder die Capability `CAP_NET_RAW` |
| Windows | Raw ICMP über Winsock | Eine Eingabeaufforderung mit erhöhten Rechten |
| macOS | ICMP-Datagramm (`SOCK_DGRAM`) | Der Kernel besitzt Prüfsumme und Identifier. Auf aktuellem macOS reicht eine normale Benutzersitzung. |

## Bauen

Das Makefile konfiguriert CMake und kopiert die Binary nach `./ft_ping` (unter Windows `ft_ping.exe`).

```sh
make
./ft_ping -?
```

Direkt mit CMake:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/ft_ping -?
```

Unter Windows wählt das Makefile den Ninja-Generator. CMake, Ninja und ein C-Compiler müssen im `PATH` liegen.

| Ziel | Wirkung |
| --- | --- |
| `make` / `make all` | Release-Build, Binary neben dem Makefile |
| `make debug` | Dasselbe mit `CMAKE_BUILD_TYPE=Debug` |
| `make run` | Bauen und `ft_ping` ohne Argumente starten (gibt die Hilfe aus) |
| `make clean` | CMake-Build aufräumen |
| `make fclean` | `build/` und die kopierte Binary entfernen |
| `make re` | `fclean`, danach `all` |

Warnungen sind standardmäßig an (`-Wall -Wextra -Wpedantic`, mit MSVC `/W4`). Als Fehler behandeln:

```sh
cmake -S . -B build -DFT_PING_WARNINGS_AS_ERRORS=ON
```

### Docker

Das Image ist eine Debian-Bookworm-Toolchain. Zusätzlich ist `inetutils-ping` installiert, damit das Referenzprogramm neben `ft_ping` liegt. Der Quellbaum wird zur Laufzeit eingehängt; das Image kopiert ihn nicht hinein.

```sh
docker build -t ft_ping-dev .
docker run --rm -it \
  --cap-add NET_RAW --cap-add NET_ADMIN \
  -v "$PWD":/ft_ping \
  ft_ping-dev
make
./ft_ping -c 2 127.0.0.1
```

`NET_RAW` ist im Container nötig, weil Linux einen Raw-ICMP-Socket verwendet.

## Verwendung

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

`-i` und `-W` erwarten Millisekunden. Bei inetutils stehen diese beiden Optionen in Sekunden; die Einheit hier ist bewusst so und entspricht dem Hilfetext.

Ohne `-c` sendet `ft_ping` weiter, bis Strg+C kommt. Der Exit-Status ist `0`, wenn mindestens eine Echo-Antwort angekommen ist. Er ist ungleich null, wenn das Parsen scheitert, die Sitzung nicht startet oder keine Antwort eintrifft.

### Optionen

| Option | Standard | Bedeutung |
| --- | --- | --- |
| `-v`, `--verbose` | aus | ICMP-Identifier in der Kopfzeile, bei Fehlern Dump von zitiertem IP- und ICMP-Header |
| `-c <count>` | `0` | Nach so vielen Requests stoppen. `0` heißt ohne Limit |
| `-s <bytes>` | `56` | Größe der ICMP-Nutzdaten |
| `-i <ms>` | `1000` | Abstand zwischen Requests in Millisekunden, gemessen ab dem Sendezeitpunkt |
| `-t <ttl>` | `64` | IPv4 Time to Live |
| `-W <ms>` | `1000` | Wartezeit auf eine Antwort, in Millisekunden |
| `--windows` | aus | Bericht im Windows-Layout statt inetutils |
| `-?`, `--help` | | Hilfe ausgeben und beenden |
| `--version` | | `ft_ping version 0.1.0` ausgeben und beenden |

Ein Ziel pro Aufruf. Ein zweites Ziel, ein unbekanntes Flag oder ein fehlender Wert ist ein Fehler; die Hilfe geht dann nach stderr.

### Ausgabe

Standard (inetutils 2.0):

```text
PING example.com (93.184.216.34): 56 data bytes
64 bytes from 93.184.216.34: icmp_seq=0 ttl=56 time=12.340 ms
--- example.com ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max/stddev = 12.340/12.340/12.340/0.000 ms
```

Mit `-v` enthält die Kopfzeile zusätzlich `id 0x.... = <pid>`. Bei ICMP-Fehlern nennt die Zeile den Grund (`Time to live exceeded`, `Destination Host Unreachable` und Ähnliches) und gibt danach einen IP-Header-Dump sowie den zitierten ICMP-Header aus.

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

Zeitüberschreitungen bleiben im inetutils-Layout still und erscheinen im Windows-Layout als `Request timeout for icmp_seq N`. In beiden Layouts zählen sie als Verlust.

## Ablauf

Jeder Versuch läuft gleich:

1. Den Host in eine IPv4-Adresse auflösen.
2. Den plattformeigenen ICMP-Socket öffnen und TTL sowie Empfangs-Timeout setzen.
3. Einen Echo-Request bauen. Der Identifier ist die Prozess-ID. Die Sequenz beginnt bei 0. Die Prüfsumme rechnet das Programm unter Linux und Windows selbst; unter macOS verlangt der Kernel eine Prüfsumme von null und füllt sie aus.
4. Das Paket senden, auf eine passende Echo-Antwort oder einen zugehörigen ICMP-Fehler warten und die Laufzeit festhalten.
5. Warten, bis seit dem Senden das Intervall vergangen ist, dann wiederholen.
6. Bei erreichter Anzahl, Ende der Schleife oder SIGINT die Zusammenfassung ausgeben und den Socket schließen.

Linux und Windows ordnen Antworten über Identifier und Sequenz zu. macOS schreibt den Identifier bei `SOCK_DGRAM`-ICMP um, deshalb gilt dort nur die Sequenz.

## Aufbau

```text
include/ft_ping/     Öffentliche Header (CLI, Sitzung, ICMP, Ausgabe, Plattform)
src/main.c           Argumente, Sendeschleife, Signale
src/cli/             Optionen und Hilfetext
src/core/            Namensauflösung, Sitzung, ein Versuch
src/icmp/            Echo-Request, Echo-Reply, Prüfsumme, Nutzdaten
src/output/          Drucker für inetutils und Windows
src/platform/        Sockets und Prozessstart je Betriebssystem
src/utils/           Fehlertexte und Logging
```

Die Ping-Schleife linkt eine statische Bibliothek (`ft_ping_lib`). Plattformunterschiede bleiben dahinter: Winsock unter Windows (`ws2_32`, `iphlpapi`), Darwin-ICMP-Datagramme und Linux-Raw-Sockets.

## Umfang

`ft_ping` deckt die inetutils-artige Probeschleife für alltägliche Erreichbarkeitsprüfungen ab: Anzahl, Größe, Intervall, TTL, Timeout pro Antwort, ausführliche ICMP-Fehler und eine RTT-Zusammenfassung am Ende.

Es spricht nur IPv4, nimmt ein Ziel entgegen und kennt die Optionen in der Tabelle oben. Der Text ist an inetutils 2.0 und an Windows `ping.exe` angelehnt. Er liegt nah an diesen Programmen, ist aber keine zeichengenaue Kopie jeder Kopfzeile und jedes Abstands.
