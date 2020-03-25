/*	mon-stdio.h - monitor console i/o
 *
 *	Copyright 2020 David Haworth
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
 *
 *
 *	This file contains I/O definitions for monitor.
 *
*/

#ifndef mon_stdio_h
#define mon_stdio_h

#include "monitor.h"
#include "mon-bcm2835.h"

extern int m_printf(char *fmt, ...);
extern char *m_gets(char *buf, int max);

static inline char m_readchar(void)
{
	return (char)bcm2835_uart_getc();
}

static inline void m_writechar(char c)
{
	bcm2835_uart_putc((int)c);
}

static inline void m_putc(char c)
{
	if ( c == '\n' )
		bcm2835_uart_putc((int)'\r');
	bcm2835_uart_putc((int)c);
}

#endif
