# A serial monitor/boot loader.

"make loader" to build the monitor and then link it to loadhigh.

Copy bin/moni-load.bin to your SD card and boot it (change config.txt).

When started in this way, monitor uses addresses 0x20000000 upwards. Cores 1, 2 and 3 are spinning in this range.

Commands (not case sensitive):
* Sn....  - an S-Record of type n
* Ba      - display value of byte at location a
* Ha      - display value of 16-bit word at location a
* Wa      - display value of 32-bit word at location a
* Qa      - display value of 32-bit word at location a
* Ba=v    - set byte at location a to v
* Ha=v    - set 16-bit word at location a to v
* Wa=v    - set 32-bit word at location a to v
* Qa=v    - set 64-bit word at location a to v
* Da,l,s  - dump l words memory starting at a. Word size is s.
* Ma,s    - modify memory starting at a. Word size is s.  [not implemented]
* Zs,e    - clear (write zero to) all memory locations a, where s <= a < e
* Ga      - call subroutine at address a on all cores
* Ga,c    - call subroutine at address a on core c (0 <= c <= 3)
* I       - print some info about no of s-records etc.
* E       - turn character echo and prompt back on
* ?       - print help text

Note:

# The S0 record turns off the prompt and character echo to allow download to proceed faster. S7/8/9 turn it
back on again. If the transfer gets interrupted or the s-rec file has no terminator record, use the E command.
# Cores 1,2 and 3 can also be released by poking a non-zero address to the appropriate
release location, which is printed at startup.  This causes a function call to the poked address, so
if the function returns, the core goes back to the spinning loop.
# There is no co-ordination for uart between monitor and loaded program, so output gets garbled.
