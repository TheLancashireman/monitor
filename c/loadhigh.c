/*	loadhigh.c - a stub program to boot another program high in memory.
 *
 *	Copyright 2018 David Haworth
 *
 *	This file is part of davros.
 *
 *	davros is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	davros is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with davros.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "monitor.h"
#include "mon-stdio.h"

extern const char bin_name[];
extern const unsigned char bin_array[];
extern const unsigned bin_length;
extern const unsigned bin_loadaddr;

typedef void (*fp_t)(void);

volatile fp_t core_start_addr[4];

/* loadhigh contains a copy of the binary of a different program that was
 * linked at a high address in RAM.
 *
 * loadhigh moves the program to its rightful place and then starts it.
 *
 * Assumptions:
 *	1. The program also contains a copy of the reset code that catches all cores and puts three of them
 *	   to sleep until it needs them.
 *	2. The load address of the program is also the entry point.
*/

static inline void release_core(int c, uint32_t entry)
{
#if 0
	m_printf("Release core %d at 0x%08x, rel_addr = 0x%08x\n", c, entry, (uint32_t)(uint64_t)&core_start_addr[c]);
#endif
	core_start_addr[c] = (fp_t)(uint64_t)entry;
}

static inline void wait_return(void)
{
	int c;
	for (;;)
	{
		c = m_readchar();
		if ( c == '\r' || c == '\n' )
			return;
	}
}

void core0_start(void)
{
	/* Enable the UART, then initialise it.
	*/
	bcm2835_enable(BCM2835_AUX_uart);
	bcm2835_uart_init(115200, 8, 0);

	/* Friendly greeting.
	*/
	m_printf("Loadhigh version 0.3!\n");

	/* Copy the data.
	*/
	m_printf("Copying %s to 0x%08x\n", bin_name, bin_loadaddr);
	unsigned char *d = (unsigned char *)(uint64_t)bin_loadaddr;
	for ( unsigned i = 0; i < bin_length; i++ )
	{
		d[i] = bin_array[i];
	}
	m_printf("Copied %u bytes to 0x%08x\n", bin_length, bin_loadaddr);

#if 0
	m_printf("Press RETURN to start each core in turn\n");
#endif

	/* Start the other cores.
	*/
#if 0
	wait_return();
#endif
	m_printf("Starting core 1\n");
	release_core(1, bin_loadaddr);
#if 0
	wait_return();
#endif
	m_printf("Starting core 2\n");
	release_core(2, bin_loadaddr);
#if 0
	wait_return();
#endif
	m_printf("Starting core 3\n");
	release_core(3, bin_loadaddr);
#if 0
	wait_return();
#endif
	m_printf("Starting core 0\n");
	fp_t x = (fp_t)(uint64_t)bin_loadaddr;
	x();
	m_printf("Oops! Loaded program returned!\n");
	for (;;) {}
}

void core_start(int c)
{
	core_start_addr[c] = NULL;

	for (;;)
	{
		if ( core_start_addr[c] != NULL )
		{
			core_start_addr[c]();
			m_printf("Core %d: start function returned\n", c);
		}
	}
}

void core1_start(void)
{
	core_start(1);
}

void core2_start(void)
{
	core_start(2);
}

void core3_start(void)
{
	core_start(3);
}
