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
#include "mon-bcm2835.h"
#include "mon-stdio.h"

typedef int (*fp_t)(int);

volatile fp_t core_start_addr[4];

void core0_start(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3)
{
    /* Enable the UART, then initialise it.
    */
    bcm2835_enable(BCM2835_AUX_uart);
    bcm2835_uart_init(115200, 8, 0);

    /* Friendly greeting.
    */
    m_printf("Davros monitor version 0.2\n");
    m_printf("Startup parameters:\n");
    m_printf("  x0 = 0x%08x%08x\n", (uint32_t)(x0>>32), (uint32_t)(x0&0xffffffff));
    m_printf("  x1 = 0x%08x%08x\n", (uint32_t)(x1>>32), (uint32_t)(x1&0xffffffff));
    m_printf("  x2 = 0x%08x%08x\n", (uint32_t)(x2>>32), (uint32_t)(x2&0xffffffff));
    m_printf("  x3 = 0x%08x%08x\n", (uint32_t)(x3>>32), (uint32_t)(x3&0xffffffff));

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
