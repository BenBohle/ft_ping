# Ping Optionen – Übersicht, Wirkung und Funktionsweise

Diese Datei ist eine praxisnahe Referenz für `ping`-Optionen.

Wichtig: Es gibt **nicht die eine** `ping`-Implementierung. Optionen unterscheiden sich je nach Plattform und Tool:
- Linux: meist `iputils ping`
- GNU/inetutils: eigene `ping`-Variante (für `ft_ping` relevant, Referenz: inetutils 2.0)
- macOS/BSD: BSD-`ping`
- Windows: `ping.exe` mit anderer Syntax

Wenn du exakt reproduzierbar arbeiten willst, prüfe lokal immer:
- Linux/macOS: `ping --help` oder `man ping`
- Windows: `ping /?`

---

## 1) Grundprinzip: Wie `ping` funktioniert

`ping` sendet ICMP Echo Request Pakete (Typ 8 bei IPv4) an ein Ziel und erwartet ICMP Echo Reply (Typ 0). Daraus entstehen:
- **Erreichbarkeit** (kommt eine Antwort zurück?)
- **RTT/Latenz** (Round-Trip-Time)
- **Packet Loss** (Paketverlust)

Typischer Ablauf pro Paket:
1. Zeitstempel beim Senden
2. Echo Request mit Identifier + Sequence Number
3. Echo Reply empfangen
4. RTT = Empfangszeit - Sendezeit

Viele Optionen steuern genau diese Punkte: **wie oft**, **wie groß**, **wie schnell**, **über welche Route/TTL**, **welche Ausgabe**.

---

## 2) Häufige Optionen (plattformübergreifend ähnlich)

> Namen/Flags sind ähnlich, aber nicht immer identisch.

### Ziel, Anzahl, Intervall, Timeout

- `destination` (Host/IP)
  - **Macht:** Zieladresse setzen.
  - **Wie:** Hostname wird in IP aufgelöst (DNS), dann ICMP an diese IP gesendet.

- `-c <count>` (Linux/macOS/inetutils)
  - **Macht:** Anzahl Requests begrenzen.
  - **Wie:** Sendeschleife stoppt nach `<count>` Sequenzen.

- `-i <interval>`
  - **Macht:** Abstand zwischen zwei Requests.
  - **Wie:** Scheduler/Timer wartet `<interval>` Sekunden.

- Timeout insgesamt (`-w <deadline>` oder windows-spezifisch anders)
  - **Macht:** Gesamtdauer der Ping-Session begrenzen.
  - **Wie:** Nach Deadline wird Ausgabe/Statistik abgeschlossen, auch wenn nicht alle Antworten kamen.

- Timeout pro Antwort (`-W <timeout>` bzw. Windows `-w <ms>`)
  - **Macht:** Wartezeit für ein einzelnes Reply.
  - **Wie:** Receive-Timeout auf Socket-Ebene.

### Paketinhalt und Größe

- `-s <size>`
  - **Macht:** Nutzdatenlänge (Payload) pro ICMP Request ändern.
  - **Wie:** Größere Pakete testen MTU, Fragmentierung und Verhalten unter Last.

- `-p <pattern>`
  - **Macht:** Payload-Muster setzen.
  - **Wie:** Statt Standarddaten wird ein Byte-Muster wiederholt; nützlich bei Leitungs-/NIC-Fehlersuche.

### Routing/Netzwerkverhalten

- `-t <ttl>` (Linux/macOS häufig), Windows `-i <TTL>`
  - **Macht:** Time-To-Live setzen.
  - **Wie:** Jedes Routing-Hop dekrementiert TTL; bei 0 kommt meist ICMP Time Exceeded.

- `-r`
  - **Macht:** Direktes Routing ohne normale Routing-Tabellen bevorzugen (plattformabhängig).
  - **Wie:** Socket/Kernel-Route-Flags beeinflussen Wegwahl.

- `-I <interface|source>` / `-S <srcaddr>` (abhängig von OS)
  - **Macht:** Quell-IP oder Interface festlegen.
  - **Wie:** Bind auf Interface/Source-Adresse vor dem Senden.

### Ausgabe/Diagnose

- `-v`
  - **Macht:** Verbose-Ausgabe.
  - **Wie:** Zusätzliche ICMP-/Fehlerinformationen werden nicht still verworfen.

- `-q`
  - **Macht:** Quiet-Modus.
  - **Wie:** Pro-Paket-Ausgabe reduziert, Fokus auf Endstatistik.

---

## 3) Linux (iputils) – typische Optionen

> Je nach Distribution/Version leicht abweichend.

- `-4`, `-6`: IPv4/IPv6 erzwingen.
- `-a`: Akustisches Signal (Bell) bei Reply.
- `-A`: Adaptive Ping-Rate (interaktiv je nach RTT).
- `-B`: Source-Address nicht wechseln lassen.
- `-c <count>`: Anzahl Requests.
- `-D`: Zeitstempel vor jeder Ausgabezeile.
- `-f`: Flood Ping (sehr hohe Sende-Rate, privilegiert).
- `-i <interval>`: Intervall in Sekunden.
- `-I <iface|addr>`: Interface/Source binden.
- `-l <preload>`: Anzahl sofort gesendeter Pakete am Start.
- `-m <mark>`: Firewall-Mark (SO_MARK) setzen.
- `-M <do|dont|want>`: PMTU-Discovery/Fragmentierungsverhalten steuern.
- `-n`: Keine Namensauflösung (nur IPs anzeigen).
- `-O`: Outstanding replies melden (fehlende Antworten markieren).
- `-p <pattern>`: Payload-Muster.
- `-Q <tos>`: TOS/DSCP setzen.
- `-q`: Quiet.
- `-s <size>`: Payload-Größe.
- `-t <ttl>`: TTL setzen.
- `-U`: User-to-user Latenz vollständig messen (inkl. DNS-Auflösung).
- `-v`: Verbose.
- `-w <deadline>`: Gesamt-Deadline.
- `-W <timeout>`: Timeout pro Antwort.

Funktionsidee hinter Spezialflags:
- `-f`/`-A` testen Last- oder Reaktionsverhalten bei hoher Packetrate.
- `-M` prüft MTU/Fragmentierung explizit.
- `-Q` und `-t` helfen bei QoS- und Routing-Diagnose.

---

## 4) GNU inetutils (Referenz für `ft_ping`) – relevante Optionen

Für dein Projekt ist das besonders wichtig, weil die Aufgabenstellung `inetutils-2.0` nennt.

Häufig relevante Flags (je nach Build):
- `-c <count>`: Anzahl.
- `-i <interval>`: Intervall.
- `-n`: Keine DNS-Auflösung in der Ausgabe.
- `-q`: Quiet.
- `-s <size>`: Payload-Größe.
- `-t <ttl>` oder ähnliche TTL-Optionen (implementierungsabhängig).
- `-v`: Verbose (in deinem Subject explizit Pflicht).
- `-w <deadline>`, `-W <timeout>`: Zeitsteuerung (falls vorhanden).
- `-?` / `--help`: Hilfe (in deinem Subject explizit erwähnt).

Hinweis für `ft_ping`:
- Minimum laut Subject: `-v` und `-?` plus einfache IPv4-Adresse/Hostname.
- Bonus-Ideen im Subject (`-f -l -n -w -W -p -r -s -T --ttl --ip-timestamp ...`) orientieren sich an realen Ping-Varianten.

---

## 5) macOS / BSD `ping` – typische Optionen

- `-A`: Audible bei fehlendem Reply (auf manchen BSD-Varianten anders).
- `-a`: Audible bei jedem Reply (BSD-abhängig).
- `-c <count>`: Anzahl.
- `-D`: Setzt "Don't Fragment" (plattformabhängig).
- `-i <wait>`: Intervall.
- `-n`: Keine DNS-Auflösung.
- `-o`: Nach erster erfolgreicher Antwort beenden.
- `-p <pattern>`: Muster.
- `-q`: Quiet.
- `-s <size>`: Payload-Größe.
- `-t <ttl>`: TTL.
- `-v`: Verbose.
- `-W <waittime>`: Wartezeit (Semantik je nach BSD-Version).

BSD/macOS ist POSIX-nah zu Linux, aber nicht identisch bei Option-Details und Default-Werten.

---

## 6) Windows `ping.exe` – typische Optionen

Windows nutzt andere Schalter (Slash/Minus gemischt je nach Version):

- `-t`
  - **Macht:** Ping läuft kontinuierlich bis Abbruch (`Ctrl+C`).

- `-a`
  - **Macht:** Reverse-Name-Lookup der Ziel-IP.

- `-n <count>`
  - **Macht:** Anzahl Echo Requests.

- `-l <size>`
  - **Macht:** Sendepuffergröße (Payload) in Bytes.

- `-f`
  - **Macht:** "Don't Fragment" für IPv4 setzen.

- `-i <TTL>`
  - **Macht:** TTL setzen.

- `-v <TOS>`
  - **Macht:** Type of Service (IPv4, eingeschränkt modern nutzbar).

- `-r <count>`
  - **Macht:** Record Route für bestimmte Anzahl Hops (IPv4, selten praktikabel).

- `-s <count>`
  - **Macht:** Timestamps für Hops (IPv4).

- `-j <host-list>`, `-k <host-list>`
  - **Macht:** Loose/Strict Source Route (historisch, oft gefiltert/deaktiviert).

- `-w <timeout>`
  - **Macht:** Timeout pro Reply in Millisekunden.

- `/R`, `/S`, `/4`, `/6`
  - **Macht:** IPv6-spezifische Diagnosen bzw. IP-Version erzwingen (versionsabhängig).

---

## 7) Welche Optionen sind für `ft_ping` am wichtigsten?

Für ein solides Lern-/Projekt-`ping` priorisieren:
1. `-v` (Pflicht laut Subject)
2. `-?`/`--help` (Pflicht laut Subject)
3. `-c`, `-i`, `-s`, `-n`, `-w`, `-W`
4. `-t`/`--ttl`, optional `-p`

Damit deckst du die wichtigsten Diagnosefälle ab:
- Erreichbarkeit
- Stabilität unter Zeitdruck
- Paketgröße/MTU-Effekte
- DNS vs. reine IP-Ausgabe
- Basis-Routing-Verhalten über TTL

---

## 8) Praktische Beispiele

- 5 Pakete senden:
  - `ping -c 5 8.8.8.8`

- Ohne DNS-Ausgabe:
  - `ping -n 1.1.1.1`

- Größere Payload testen:
  - `ping -s 1200 1.1.1.1`

- Kurzes Timeout pro Antwort:
  - Linux/macOS oft: `ping -W 1 -c 4 8.8.8.8`
  - Windows: `ping -n 4 -w 1000 8.8.8.8`

---

## 9) Kurzfazit

- `ping`-Optionen sind stark implementierungsabhängig.
- Linux, macOS/BSD und Windows überlappen in der Idee, aber nicht immer in Flag-Namen oder Semantik.
- Für `ft_ping` orientierst du dich am besten an `inetutils-2.0` + Subject-Pflichtflags.
