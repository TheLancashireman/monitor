# A serial monitor/boot loader.

"make loader" to build the monitor and then link it to loadhigh.

Copy bin/moni-load.bin to your SD card and boot it (change config.txt).

When started in this way, monitor uses addresses 0x20000000 upwards. Cores 1, 2 and 3 are spinning in this range.

Commands (not case sensitive):
	Sn....	- Type n S-Record 
	Ba		- display value of byte at location a
	Ha		- display value of 16-bit word at location a
	Wa		- display value of 32-bit word at location a
	Qa		- display value of 64-bit word at location a
	Ba=v	- set byte at location a to v
	Ha=v	- set 16-bit word at location a to v
	Wa=v	- set 32-bit word at location a to v
	Qa=v	- set 64-bit word at location a to v
	Da,l,s	- dump l words memory starting at a. Word size is s.
[	Ma,s	- modify memory starting at a. Word size is s.    --- not implemented]
	Ga		- call subroutine at address a

Note: cores 1,2 and 3 can be released by poking a non-zero 32-bit address to
20006000, 20004000 and 20002000. This causes a function call to the poked address, so
if the function returns, the core goes back to the spinning loop.
No co-ordination for uart between monitor and loaded program, so output gets garbled.
