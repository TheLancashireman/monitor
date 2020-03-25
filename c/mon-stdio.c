/*	mon-stdio.c - stdio for monitor
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
 *	This file contains a limited stdio for the monitor
 *
 *  Requires architecture-dependent functions or macros:
 *
 *		char m_readchar(void) - returns next character from input
 *			(serial port or whatever). Waits until available.
 *
 *		void m_writechar(char c) - writes character c to output (serial port
 *			or whatever). Waits until space available in output buffer.
 *
*/
#include "monitor.h"
#include "mon-stdio.h"
#include <stdarg.h>

static int isreturn(char c)
{
	return ( c == '\n' || c == '\r' );
}

static int m_xprintf(const char *fmt, va_list ap);

extern memaddr_t GetSP(void);

char *m_gets(char *buf, int max)
{
	char c;
	char *p = buf;
	int i = 0;

	*p = '\0';
	while ( !isreturn(c = m_readchar()) )
	{
		if ( c == BS ||
			 c == DEL )
		{
			if ( i > 0 )
			{
				i--;
				p--;
				*p = '\0';
				m_writechar(BS);
				m_writechar(' ');
				m_writechar(BS);
			}
			else
				m_writechar(BEL);
		}
		else
		if ( i < max && c >= ' ' && c < 0x7f )
		{
			m_writechar(c);
			*p++ = c;
			*p = '\0';
			i++;
		}
		else
			m_writechar(BEL);
	}
	*p = '\0';
	m_writechar('\r');
	m_writechar('\n');
	return(buf);
}

int m_printf(char *fmt, ...)
{
	int n;
	va_list ap;
	va_start(ap,fmt);

	n = m_xprintf(fmt, ap);

	va_end(ap);

	return(n);
}

/* We need to be able to print 64-bit numbers in decimal and hex.
 * 31 characters should be enough for that ;-)
*/
#define XPRINT_MAXSTR 31

static char *prt10(unsigned long val, char *str);
static char *prt16(unsigned long val, char *str, char *hexdgt);

static int m_xprintf
(	const char *fmt,
	va_list ap
)
{
	char ch, fill, string[XPRINT_MAXSTR+1], *str;
	int fmin, fmax, len, sign, ljust, longarg;
	int i, leading;
	int nprinted;
	long num;

	nprinted = 0;

	while ( (ch = *fmt++) != '\0' )
	{
		if ( ch == '%' )
		{
			ch = *fmt++;

			if ( ch == '%' )
			{
				m_writechar(ch);
				nprinted++;
			}
			else
			{
				sign = ljust = longarg = 0;
				fmin = 0;
				fmax = 255;
				fill = ' ';

				if ( ch == '-' )
				{
					ljust = 1;
					ch = *fmt++;
				}
				if ( ch == '0' )
				{
					fill = '0';
					ch = *fmt++;
				}
				if ( ch == '*' )
				{
					fmin = va_arg(ap,int);
					ch = *fmt++;
				}
				else
				while ( m_isdigit(ch) )
				{
					fmin = fmin * 10 + (ch - '0');
					ch = *fmt++;
				}
				if ( ch == '.' )
				{
					ch = *fmt++;
					if ( ch == '*' )
					{
						fmax = va_arg(ap,int);
						ch = *fmt++;
					}
					else
					{
						fmax = 0;
						while ( m_isdigit(ch) )
						{
							fmax = fmax * 10 + (ch - '0');
							ch = *fmt++;
						}
					}
				}
				if ( ch == 'l' )
				{
					longarg = 1;
					ch = *fmt++;
				}
				str = string;

				switch (ch)
				{
				case '\0':
					m_writechar('%');
					nprinted++;
					return(nprinted);
					break;

				case 'c':
					if ( longarg )
						string[0] = (char) va_arg(ap, long);
					else
						string[0] = (char) va_arg(ap, int);
					string[1] = '\0';
					break;

				case 's':
					str = va_arg(ap, char*);
					break;

				case 'd':
				case 'u':
				case 'x':
				case 'X':
					if ( longarg )
						num = va_arg(ap, long);
					else
						num = va_arg(ap, int);
					if ( ch == 'd' && num < 0 )
					{
						sign = 1;
						num = -num;
					}
					if ( !longarg )
						num = (long)(int)num;
					if ( ch == 'd' || ch == 'u' )
						str = prt10(num, str);
					else
						str = prt16(num, str, ch=='x' ? "abcdef" : "ABCDEF");
					break;

				default:
					str = "***";
					break;
				}
				leading = 0;
				len = m_strlen(str);
				if ( len > fmax )
					len = fmax;
				if ( len < fmin )
					leading = fmin - len - sign;
				nprinted += len + leading + sign;
				if ( sign && ( fill == '0' ) )
				{
					sign = 0;
					m_writechar('-');
				}
				if ( !ljust )
				{
					for ( i=0; i<leading; i++ )
						m_writechar(fill);
				}
				if ( sign )
					m_writechar('-');
				for ( i=0; i<len; i++ )
				{
					m_putc(*str++);
				}
				if ( ljust )
				{
					for ( i=0; i<leading; i++ )
						m_writechar(fill);
				}
			}
		}
		else
		{
			m_putc(ch);
			nprinted++;
		}
	}
	return(nprinted);
}

static char *prt10(unsigned long val, char *str)
{
	unsigned long tmp;

	str += XPRINT_MAXSTR;
	*str = '\0';
	do
	{
		tmp = val / 10L;
		*(--str) = (val - tmp*10L) + '0';
		val = tmp;
	} while ( val != 0 );
	return(str);
}

static char *prt16(unsigned long val, char *str, char *hexdgt)
{
	unsigned tmp;

	str += XPRINT_MAXSTR;
	*str = '\0';
	do
	{
		tmp = ((unsigned)val) & 0xF;
		if ( tmp < 10 )
			tmp += '0';
		else
			tmp = hexdgt[tmp-10];
		*(--str) = tmp;
		val = val >> 4;
	} while ( val != 0 );
	return(str);
}
