General Instructions
• Your project must be realized in a virtual machine running on Debian (>= 7.0).
• Your virtual machine must have all the necessary software to complete your project.
These softwares must be configured and installed.
• You must be able to use your virtual machine from a cluster computer.
• This project will be corrected by humans only. You’re allowed to organise and name
your files as you see fit, but you must follow the following rules
• You must use C and submit a Makefile
• Your Makefile must compile the project and must contain the usual rules. It must
recompile and re-link the program only if necessary.
• You have to handle errors carefully. In no way can your program quit in an unex-
pected manner (Segmentation fault, bus error, double free, etc).
• You are authorised to use the libc functions to complete this project.
ATTENTION: Program in C, all libC is authorised, using the system
ping or the sources of a standard ping in any way is forbidden.
4
Chapter IV
Mandatory Part
• The executable must be named ft_ping.
• You will take as reference the ping implementation from inetutils-2.0 (ping -V).
• You have to manage the -v -? options.
The -v option here will also allow us to see the results in case of a
problem or an error linked to the packets, which logically shouldn’t
force the program to stop (the modification of the TTL value can help
to force an error).
• You will have to manage a simple IPv4 (address/hostname) as parameters.
• You will have to manage FQDN without doing the DNS resolution in the packet
return
You are allowed to use all the functions of the printf family.
For the smarty pants (or not)... Obviously you are NOT allowed to
call a real ping.
5
Chapter V
Bonus Part
Find below a few ideas of interesting bonuses:
• Additional -f -l -n -w -W -p -r -s -T --ttl --ip-timestamp flags...
the flags -V, –usage, –echo are not considered as bonus
Of course two flags corresponding to the same feature (eg: -t and
–type) are not considered as two bonuses