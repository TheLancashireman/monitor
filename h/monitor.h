/*	monitor.h - monitor definitions
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
 *	This file contains definitions for monitor.
 *
*/

#ifndef monitor_h
#define monitor_h

#define MON_PI3_ARM64	1
#define MON_PI3_ZERO	2
#define MON_LINUXTEST	99

#ifndef MON_BOARD
#define MON_BOARD 		MON_PI3_ARM64
#endif

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef volatile uint32_t reg32_t;

#if MON_BOARD == MON_PI3_ARM64

#define MON_64BIT	1
typedef unsigned long uint64_t;
typedef uint64_t memaddr_t;
typedef uint64_t maxword_t;

#define BCM2835_PBASE	0x3f000000

#elif MON_BOARD == MON_PI3_ZERO

#define MON_64BIT	0
typedef uint32_t memaddr_t;
typedef uint32_t maxword_t;

#define BCM2835_PBASE	0x20000000

#else
#error "Unknown/unsupported MON_BOARD"
#endif

typedef void (*pokefunc_t)(memaddr_t a, uint8_t b);
typedef void (*vfuncv_t)(void);

#define NULL	0
#define MAXLINE	1024

#define SREC_EOF		1
#define SREC_BADTYP		(-1)
#define SREC_BADLEN		(-2)
#define SREC_NONHEX		(-3)
#define SREC_BADCK		(-4)

#define	peek8(a)		(*(uint8_t *)(a))
#define	peek16(a)		(*(uint16_t *)(a))
#define	peek32(a)		(*(uint32_t *)(a))
#define poke8(a, v)		(*(uint8_t *)(a) = (v))
#define poke16(a, v)	(*(uint16_t *)(a) = (v))
#define poke32(a, v)	(*(uint32_t *)(a) = (v))

#if MON_64BIT
#define	peek64(a)		(*(uint64_t *)(a))
#define poke64(a, v)	(*(uint64_t *)(a) = (v))
#endif

#define go(a)			((*(vfuncv_t)(a))())

extern void monitor(char *prompt);
extern int process_s_record(char *line, pokefunc_t _poke);
extern int char2hex(char c);
extern maxword_t gethex(char **pp, int max);


/* Names for ASCII control codes
*/
#define NUL		0x00
#define SOH		0x01
#define STX		0x02
#define ETX		0x03
#define EOT		0x04
#define ENQ		0x05
#define ACK		0x06
#define BEL		0x07
#define BS		0x08
#define HT		0x09
#define LF		0x0a
#define VT		0x0b
#define FF		0x0c
#define CR		0x0d
#define SO		0x0e
#define SI		0x0f
#define DLE		0x10
#define DC1		0x11
#define DC2		0x12
#define DC3		0x13
#define DC4		0x14
#define NAK		0x15
#define SYN		0x16
#define ETB		0x17
#define CAN		0x18
#define EM		0x19
#define SUB		0x1A
#define ESC		0x1B
#define FS		0x1C
#define GS		0x1D
#define RS		0x1E
#define US		0x1F
#define SPACE	0x20
#define DEL		0x7f

#if 0 /* builtin */
static inline char toupper(char c)
{
	return (((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c));
}

static inline char char tolower(char c)
{
	return (((c) >= 'A' && (c) <= 'Z') ? ((c) + 0x20) : (c));
}
#endif

static inline int m_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

static inline int m_isxdigit(int c)
{
	return ( (c >= '0' && c <= '9') ||
			 (c >= 'a' && c <= 'f') || 
			 (c >= 'A' && c <= 'F') );
}
	
static inline int m_isspace(int c)
{
	return ( c == ' ' ||
			 c == '\f' ||
			 c == '\n' ||
			 c == '\r' ||
			 c == '\t' ||
			 c == '\v' );
}

static inline int m_isprint(int c)
{
	return (c >= ' ' && c < 0x7f);
}

static inline int m_strlen(const char *str)
{
	const char *s = str;
	int len = 0;
	while ( *s != '\0' )
	{
		len++;
		s++;
	}
	return len;
}

static inline char *m_skipspaces(char *p)
{
	while ( m_isspace(*p) )
		p++;
	return(p);
}


#endif
