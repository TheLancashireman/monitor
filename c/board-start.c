/*	board-start.c - startup function for monitor and bootloader
 *
 *	Copyright David Haworth
 *
 *	This file is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2, or (at your option)
 *	any later version.
 *
 *	It is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; see the file COPYING.  If not, write to
 *	the Free Software Foundation, 59 Temple Place - Suite 330,
 *	Boston, MA 02111-1307, USA.
*/
#include "monitor.h"
#include "mon-stdio.h"

extern uint64_t mon_startaddr, bss_start, bss_end, null_addr;

typedef int (*fp_t)(int);

volatile fp_t core_start_addr[4];

static void print_release_address(int c)
{
	uint64_t a = (uint64_t)&core_start_addr[c];
	uint32_t ah = (uint32_t)((a >> 32) & 0xffffffff);
	uint32_t al = (uint32_t)(a & 0xffffffff);
	m_printf("Release address for core %d : 0x%08x%08x\n", c, ah, al);
}

void core0_start(void)
{
	uint64_t *p;

    /* Enable the UART, then initialise it.
    */
    bcm2835_enable(BCM2835_AUX_uart);
    bcm2835_uart_init(115200, 8, 0);

    /* Friendly greeting.
    */
    m_printf("Davros monitor version 0.4\n");
    m_printf("... clearing bss\n");
	p = &bss_start;
	while ( p < &bss_end )
	{
		*p++ = 0;
	}
	if ( &mon_startaddr != &null_addr )
	{
    	m_printf("... clearing low memory\n");
		p = &null_addr;
		while ( p < &mon_startaddr )
		{
			*p++ = 0;
		}
	}

	print_release_address(1);
	print_release_address(2);
	print_release_address(3);

	monitor("mon > ");
}

void core_start(int c)
{
	core_start_addr[c] = NULL;

	for (;;)
	{
		if ( core_start_addr[c] != NULL )
		{
			int r = core_start_addr[c](c);
			m_printf("Core %d: start function returned %d\n", c, r);

			core_start_addr[c] = NULL;
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
