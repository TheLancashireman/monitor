/*	monitor.c - monitor and boot loader
 *
 *	Copyright 2001 David Haworth
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
 *	This file contains the main program of the boot monitor and loader.
 *
 *  Commands (not case sensitive):
 *		Sn....	- Type n S-Record 
 *		Ba		- display value of byte at location a
 *		Ha		- display value of 16-bit word at location a
 *		Wa		- display value of 32-bit word at location a
 *		Qa		- display value of 32-bit word at location a
 *		Ba=v	- set byte at location a to v
 *		Ha=v	- set 16-bit word at location a to v
 *		Wa=v	- set 32-bit word at location a to v
 *		Qa=v	- set 64-bit word at location a to v
 *		Da,l,s	- dump l words memory starting at a. Word size is s.
 *		Ma,s	- modify memory starting at a. Word size is s.  [not implemented]
 *		Zs,e	- clear (write zero to) all memory locations a, where s <= a < e
 *		Ga		- call subroutine at address a on all cores
 *		Ga,c	- call subroutine at address a on core c (0 <= c <= 3)
 *		?		- print help text
 *
 *  Requires architecture-dependent functions or macros:
 *
 *		char readchar(void) - returns next character from input
 *			(serial port or whatever). Waits until available.
 *
 *		void writechar(char c) - writes character c to output (serial port
 *			or whatever). Waits until space available in output buffer.
 *
 *		uint8_t peek8(memaddr_t a) - returns the byte at address a.
 *		uint16_t peek16(memaddr_t a) - returns the 16-bit word at address a.
 *		uint32_t peek32(memaddr_t a) - returns the 32-bit word at address a.
 *		uint64_t peek64(memaddr_t a) - returns the 64-bit word at address a.
 *
 *		void poke8(memaddr_t a, uint8_t v) - set the byte of memory at address a to v.
 *		void poke16(memaddr_t a, uint16_t v) - set the 16-bit word of memory at address a to v.
 *		void poke32(memaddr_t a, uint32_t v) - set the 32-bit word of memory at address a to v.
 *		void poke64(memaddr_t a, uint64_t v) - set the 64-bit word of memory at address a to v.
 *
 *		void go(memaddr_t a) - calls subroutine at a
 *		void release(int c, memaddr_t a) - release a core (1..3) at address a
 *
 *		ADDRSIZE - size of an address, in bits.
 *
*/
#include "monitor.h"
#include "mon-stdio.h"

/*	Messages etc. */
const char what[]		= "What?";
const char how[]		= "How?";
const char sorry[]		= "Sorry :-(";

/*	Local functions */
static void word_op(int s, char *p);
static void dump_op(char *p);
static void mod_op(char *p);
static void go_op(char *p);
static void zero_op(char *p);
static void help(void);

#ifdef poke8	/* If poke8 is a macro, we can't pass it to process_s_record */
static void mypoke(memaddr_t a, uint8_t b)
{
	poke8(a, b);
}
#else
#define mypoke poke8
#endif

char line[MAXLINE+2];

void monitor(char *prompt)
{
	char *p;

	help();

	for (;;)
	{
		m_printf("%s", prompt);
		m_gets(line, MAXLINE);
		p = m_skipspaces(line);
		switch ( *p )
		{
		case '\0':
			/* Nothing */
			break;

		case 's':
		case 'S':
			switch ( process_s_record(p, mypoke) )
			{
			case 0:		/* OK - no message */
				break;

			case SREC_EOF:
				m_printf("End of S-record file\n");
				break;

			case SREC_BADTYP:
			case SREC_BADLEN:
			case SREC_NONHEX:
			case SREC_BADCK:
				m_printf("Bad S-record: \"%s\"\n", line);
				break;

			}
			break;

		case 'b':
		case 'B':
			word_op(1, p+1);
			break;

		case 'h':
		case 'H':
			word_op(2, p+1);
			break;

		case 'w':
		case 'W':
			word_op(4, p+1);
			break;

		case 'q':
		case 'Q':
			word_op(8, p+1);
			break;

		case 'd':
		case 'D':
			dump_op(p+1);
			break;

		case 'm':
		case 'M':
			mod_op(p+1);
			break;

		case 'g':
		case 'G':
			go_op(p+1);
			break;

		case 'z':
		case 'Z':
			zero_op(p+1);
			break;

		case '?':
			help();
			break;

		default:
			m_printf("%s\n", what);
			break;
		}
	}
}

static void help(void)
{
	m_printf("    Sn....  - Type n S-Record\n");
	m_printf("    Ba      - display value of byte at location a\n");
	m_printf("    Ha      - display value of 16-bit word at location a\n");
	m_printf("    Wa      - display value of 32-bit word at location a\n");
	m_printf("    Qa      - display value of 64-bit word at location a\n");
	m_printf("    Ba=v    - set byte at location a to v\n");
	m_printf("    Ha=v    - set 16-bit word at location a to v\n");
	m_printf("    Wa=v    - set 32-bit word at location a to v\n");
	m_printf("    Qa=v    - set 64-bit word at location a to v\n");
	m_printf("    Da,l,s  - dump l words memory starting at a. Word size is s.\n");
#if 0
	m_printf("    Ma,s    - modify memory starting at a. Word size is s.\n");
#endif
	m_printf("    Ga      - call subroutine at address a on all cores\n");
	m_printf("    Ga,c    - call subroutine at address a on core c\n");
	m_printf("    Zs,e    - zero memory all memory locations a, where s <= a < e\n");
	m_printf("    ?       - show this help text\n");
}

static void word_op(int s, char *p)
{
	memaddr_t a;
	uint64_t v;

	p = m_skipspaces(p);
	a = gethex(&p, sizeof(memaddr_t)*2);
	if ( p == NULL )
	{
		m_printf("%s\n", how);
		return;
	}

	if ( (a & (s-1)) != 0 )
	{
		/* Misaligned
		*/
		m_printf("%s\n", how);
		return;
	}

	p = m_skipspaces(p);
	if ( *p == '\0' )
	{
		switch ( s )
		{
		case 1:
			m_printf("%08x = %02x\n", a, peek8(a) & 0xff);
			break;
		case 2:
			m_printf("%08x = %04x\n", a, peek16(a) & 0xffff);
			break;
		case 4:
			m_printf("%08x = %08x\n", a, peek32(a) & 0xffffffff);
			break;
		case 8:
			m_printf("%08x = %016x\n", a, peek64(a));
			break;
		}
	}
	else
	if ( *p == '=' )
	{
		p = m_skipspaces(p+1);
		v = gethex(&p, s*2);
		if ( p == NULL ||
			 *(p = m_skipspaces(p)) != '\0' )
		{
			m_printf("%s\n", how);
		}
		else
		{
			switch ( s )
			{
			case 1:
				poke8(a, v);
				break;
			case 2:
				poke16(a, v);
				break;
			case 4:
				poke32(a, v);
				break;
			case 8:
				poke64(a, v);
				break;
			}
		}
	}
}

void dump_op(char *p)
{
	memaddr_t a, ca;
	int l = 16;
	int s = 1;
	int i;
	int c;

	p = m_skipspaces(p);
	a = gethex(&p, sizeof(memaddr_t)*2);

	if ( p == NULL )
	{
		m_printf("%s\n", how);
		return;
	}

	p = m_skipspaces(p);
	if ( *p == ',' )
	{
		p = m_skipspaces(p+1);
		l = gethex(&p, 4);
		if ( p == NULL )
		{
			m_printf("%s\n", how);
			return;
		}
		p = m_skipspaces(p);
		if ( *p == ',' )
		{
			p = m_skipspaces(p+1);
			s = gethex(&p, 1);
			if ( p == NULL )
			{
				m_printf("%s\n", how);
				return;
			}
			p = m_skipspaces(p);
		}
	}

	if ( *p != '\0' )
	{
		m_printf("%s\n", how);
		return;
	}

	if ( !(s == 1 || s == 2 || s == 4 || s == 8) )
	{
		m_printf("%s\n", sorry);
		return;
	}

	if ( (a & (s-1)) != 0 )
	{
		/* Misaligned
		*/
		m_printf("%s\n", sorry);
		return;
	}

	while ( l > 0 )
	{
		m_printf("%08x", a);
		i = 16;
		ca = a;
		while ( i > 0 && l > 0 )
		{
			if ( s == 1 && i == 8 )
				m_printf(" -");
			switch ( s )
			{
			case 1:
				m_printf(" %02x", peek8(a));
				break;
			case 2:
				m_printf(" %04x", peek16(a));
				break;
			case 4:
				m_printf(" %08x", peek32(a));
				break;
			case 8:
				m_printf(" %016lx", peek64(a));
				break;
			}
			a += s;
			i -= s;
			l -= s;
		}
		if ( s == 1 )
		{
			m_printf("   ");
			while ( ca < a )
			{
				c = peek8(ca++);
				if ( c <= 0x20 || c >= 0x7f )
					c = '.';
				m_printf("%c", c);
			}
		}
		m_printf("\n");
	}
}

void mod_op(char *p)
{
	/* FIXME - to do */
	m_printf("Not implemented\n");
}

void go_op(char *p)
{
	memaddr_t a;
	int c = -1;

	p = m_skipspaces(p);
	a = gethex(&p, sizeof(memaddr_t)*2);

	if ( p == NULL )
	{
		m_printf("%s\n", how);
		return;
	}

	if ( *p == ',' )
	{
		p = m_skipspaces(p+1);
		c = gethex(&p, 1);
		if ( p == NULL )
		{
			m_printf("%s\n", how);
			return;
		}
		if ( *p != '\0' )
		{
			m_printf("%s\n", how);
			return;
		}
		if ( c < 0 || c > 3 )
		{
			m_printf("%s\n", sorry);
			return;
		}

		if ( c ==  0 )
			go(a);
		else
			release(c, a);
	}
	else
	{
		release(1, a);
		release(2, a);
		release(3, a);
		go(a);
	}
}

void zero_op(char *p)
{
	memaddr_t s, e=0, l;

	p = m_skipspaces(p);
	s = gethex(&p, sizeof(memaddr_t)*2);

	if ( p == NULL )
	{
		m_printf("%s\n", how);
		return;
	}

	p = m_skipspaces(p);
	if ( *p == ',' )
	{
		p = m_skipspaces(p+1);
		e = gethex(&p, sizeof(memaddr_t)*2);
		if ( p == NULL )
		{
			m_printf("%s\n", how);
			return;
		}
		p = m_skipspaces(p);
		if ( p == NULL )
		{
			m_printf("%s\n", how);
			return;
		}
	}

	if ( *p != '\0' )
	{
		m_printf("%s\n", how);
		return;
	}

	if ( s > e )
	{
		m_printf("%s\n", how);
		return;
	}

	l = e & ~0x7; /* The limit for 64-bit words */

	if ( s < l )
	{
		while ( (s & 0x07) != 0 )
		{
			poke8(s, 0);
			s++;
		}

		/* s is now aligned to 64-bit word */

		while ( s < l )
		{
			poke64(s, 0);
			s += 8;
		}
	}

	while ( s < e )
	{
		poke8(s, 0);
		s++;
	}
}
