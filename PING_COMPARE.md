# windows
``ping google.com``

Ping wird ausgeführt für google.com [172.217.16.174] mit 32 Bytes Daten:
Antwort von 172.217.16.174: Bytes=32 Zeit=17ms TTL=119
Antwort von 172.217.16.174: Bytes=32 Zeit=17ms TTL=119
Antwort von 172.217.16.174: Bytes=32 Zeit=17ms TTL=119
Antwort von 172.217.16.174: Bytes=32 Zeit=17ms TTL=119

Ping-Statistik für 172.217.16.174:
    Pakete: Gesendet = 4, Empfangen = 4, Verloren = 0
    (0% Verlust),
Ca. Zeitangaben in Millisek.:
    Minimum = 17ms, Maximum = 17ms, Mittelwert = 17ms




Syntax: ping [-t] [-a] [-n count] [-l size] [-f] [-i TTL] [-v TOS]
             [-r count] [-s count] [[-j host-list] | [-k host-list]]
             [-w timeout] [-R] [-S srcaddr] [-c compartment] [-p]
             [-4] [-6] Zielname

Optionen:
    -t             Pingt den angegebenen Host bis zur Beendigung des Vorgangs.
                   Drücken Sie STRG+UNTBR, um die Statistik anzuzeigen und
                   den Vorgang fortzusetzen.
                   Drücken Sie STRG+C, um den Vorgang abzubrechen.
    -a             Löst Adressen zu Hostnamen auf.
    -n count       Die Anzahl der zu sendenden Echoanforderungen.
    -l size        Die Größe des Sendepuffers.
    -f             Legt das Kennzeichen für "Nicht fragmentieren" im Paket
                   fest (nur IPv4).
    -i TTL         Die Lebensdauer.
    -v TOS         Der Diensttyp (nur IPv4. Diese Einstellung ist veraltet
                   und hat keine Auswirkungen auf das Diensttypfeld im IP-
                   Header).
    -r count       Datensatzroute für Anzahl von Hops (nur IPv4).
    -s count       Zeitstempel für Anzahl von Hops (nur IPv4).
    -j host-list   "Loose Source Route" gemäß Hostliste (nur IPv4).
    -k host-list   "Strict Source Route" gemäß Hostliste (nur IPv4).
    -w timeout     Zeitlimit in Millisekunden für die einzelnen Antworten.
    -R             Verwendet den Routingheader, um auch die umgekehrte Route
                   zu testen (nur IPv6).
                   Dieser Routingheader wird nicht mehr verwendet
                   (siehe RFC 5095). Bei manchen Systemen werden
                   Echoanforderungen verworfen, wenn
                   dieser Header verwendet wird.
    -S srcaddr     Die zu verwendende Quelladresse.
    -c compartment Routingdepotbezeichner
    -p             Pingt die Adresse eines Anbieters für die
                   Hyper-V-Netzwerkvirtualisierung.
    -4             Erzwingt die Verwendung von IPv4.
    -6             Erzwingt die Verwendung von IPv6.
