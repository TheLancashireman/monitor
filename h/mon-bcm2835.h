/*  mon-bcm2835.h - raspberry pi suppoort for monitor
 *
 *  Copyright 2020 David Haworth
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef mon_bcm2835_h
#define mon_bcm2835_h	1

#include "monitor.h"

#ifndef BCM2835_PBASE
#error	"No definition of BCM2835_PBASE in the board headers. Please fix!"
#endif

/* BCM2835 auxiliary peripherals (as used on Raspberry Pi CPUs).
 *
 * Two control/status registers for mini-uart and SPI peripherals.
*/
typedef struct bcm2835_aux_s bcm2835_aux_t;

struct bcm2835_aux_s
{
	reg32_t irq;			/* 0x00	interrupt pending flags	(read-only) */
	reg32_t enable;		/* 0x04	peripheral enable */
};

#define bcm2835_aux	((bcm2835_aux_t *)(BCM2835_PBASE+0x215000))[0]

static inline void bcm2835_enable(uint32_t per)
{
	bcm2835_aux.enable |= per;
}

static inline void bcm2835_disable(uint32_t per)
{
	bcm2835_aux.enable &= ~per;
}

/* Masks for both aux registers.
*/
#define BCM2835_AUX_uart	0x01
#define BCM2835_AUX_spi1	0x02
#define BCM2835_AUX_spi2	0x04

/* BCM2835 GPIO
 *
 * There are 54 GPIO channels, most of which have at least one more function
 * on top of the standard digital I/O.
 *
 * resn[] fields are reserved (unused) locations in the memory map.
 *
 * For the fields that have 1 bit per pin:
 *		xxx[0] 31..0 represents gpio 31..0
 *		xxx[1] 21..0 represents gpio 53..32
 *      (actually, in the doc this is reversed: bits 31..0 are gpio 0..31 and 21..0 are gpio 32..53)
 *
 * The function select registers have 3 bits per pin (10 pins per register)
 *		E.g. in register 0, pin 0 is bits 0..2 ---> pin 9 is bits 27..29. Bits 30..31 not used.
 *		For each pin: 000 = input, 001 = output, other values are alternate functions.
 *
 * pud register: lower 2 bits:
 *		00 = pull up/down disabled
 *		01 = pull down enabled
 *		10 = pull up enabled
 *		11 reserved
 *
 * pudclk registers ---> looks complicated. Might be easier to use external Rs :-)
*/

/* Function select - yes, they really are in that order!
*/
#define BCM2835_pinfunc_input	0
#define BCM2835_pinfunc_output	1
#define BCM2835_pinfunc_alt0	4
#define BCM2835_pinfunc_alt1	5
#define BCM2835_pinfunc_alt2	6
#define BCM2835_pinfunc_alt3	7
#define BCM2835_pinfunc_alt4	3
#define BCM2835_pinfunc_alt5	2

/* Pull up/down
*/
#define BCM2835_pinpull_none	0
#define BCM2835_pinpull_down	1
#define BCM2835_pinpull_up		2

typedef struct bcm2835_gpio_s bcm2835_gpio_t;

struct bcm2835_gpio_s
{
	reg32_t fsel[6];	/* 3 bits per pin ==> 10 pins per register. Bits 30..31 not used. */
	reg32_t res1[1];
	reg32_t set[2];		/* write 1 to set (32 pins per register) */
	reg32_t res2[1];
	reg32_t clr[2];		/* write 1 to clear (32 pins per register) */
	reg32_t res3[1];
	reg32_t lev[2];		/* pin level registers */
	reg32_t res4[1];
	reg32_t eds[2];		/* event detect registers */
	reg32_t res5[1];
	reg32_t ren[2];		/* rising edge detect enable registers */
	reg32_t res6[1];
	reg32_t fen[2];		/* falling edge detect enable registers */
	reg32_t res7[1];
	reg32_t hen[2];		/* high detect enable registers */
	reg32_t res8[1];
	reg32_t len[2];		/* low detect enable registers */
	reg32_t res9[1];
	reg32_t aren[2];	/* async rising edge detect enable registers */
	reg32_t res10[1];
	reg32_t afen[2];	/* async falling edge detect enable registers */
	reg32_t res11[1];
	reg32_t pud;		/* Pull up/down enable */
	reg32_t pudclk[2];	/* Pull up/down enable clock */
};

#define bcm2835_gpio	((bcm2835_gpio_t *)(BCM2835_PBASE+0x200000))[0]

extern void bcm2835_gpio_pinconfig(uint32_t pin, uint32_t fsel, uint32_t pud);

static inline void bcm2835_gpio_pin_set(uint32_t pin)
{
	bcm2835_gpio.set[pin/32] = 1 << (pin%32);
}

static inline void bcm2835_gpio_pin_clear(uint32_t pin)
{
	bcm2835_gpio.clr[pin/32] = 1 << (pin%32);
}

static inline void bcm2835_gpio_pin_set_group(uint64_t group)
{
	bcm2835_gpio.set[0] = group & 0xffffffff;
	bcm2835_gpio.set[1] = group >> 32;
}

static inline void bcm2835_gpio_pin_clear_group(uint64_t group)
{
	bcm2835_gpio.clr[0] = group & 0xffffffff;
	bcm2835_gpio.clr[1] = group >> 32;
}

/* BCM2835 mini uart (as used on Raspberry Pi CPUs).
 *
 * From the Broadcom documentation (BCM2835-ARM-Peripherals.pdf):
 * "The implemented UART is not a 16650 compatible UART However as far as possible the
 *  first 8 control and status registers are laid out like a 16550 UART. All 16550 register bits
 *  which are not supported can be written but will be ignored and read back as 0. All control
 *  bits for simple UART receive/transmit operations are available."
 *
 * The device is represented here using 32-bit words for the registers.
 * Although the first 7 registers are only 8-bits wide, using the ARM as a little-endian
 * processor (and considering that the padding bytes are read as zero) this
 * should produce an efficient representation.
 *
 * The device must be enabled (see bcm2835 AUX header) and its pins set to the
 * correct mode (see bcm2835 GPIO header) before using the device.
 *
 * Setting the DLAB bit in lcr enables alternative functions for ios and iir (baud high and low)
 * However, it's easier to use the baud register. Therefore no define for the DLAB bit
 *
 * The baud register: lower 16 bits contains baud divider
*/
typedef struct bcm2835_uart_s bcm2835_uart_t;

struct bcm2835_uart_s
{
	reg32_t io;			/* 0x00	i/o data (tx/rx)	*/
	reg32_t ier;		/* 0x04	interrupt enable	*/
	reg32_t iir;		/* 0x08	interrupt identify	*/
	reg32_t lcr;		/* 0x0c	line control		*/
	reg32_t mcr;		/* 0x10	modem control		*/
	reg32_t lsr;		/* 0x14	line status			*/
	reg32_t msr;		/* 0x18	modem status		*/
	reg32_t scratch;	/* 0x1c	scratch				*/
	reg32_t cntl;		/* 0x20	extra control		*/
	reg32_t stat;		/* 0x24	extra status		*/
	reg32_t baud;		/* 0x28	baudrate			*/
};

#define bcm2835_uart	((bcm2835_uart_t *)(BCM2835_PBASE+0x215040))[0]

extern void bcm2835_uart_init(uint32_t baud, uint32_t bits, uint32_t parity);

#define BCM2835_IER_TxInt		0x02
#define BCM2835_IER_RxInt		0x01

#define BCM2835_IIR_Iid			0x06	/* Apparently both bits set isn't possible */
#define BCM2835_IIR_Iid_Rx		0x04
#define BCM2835_IIR_Iid_Tx		0x02
#define BCM2835_IIR_NotPend		0x01	/* Clear when interrupt pending */

#define BCM2835_LCR_Break		0x40	/* 1 = Tx pulled low. Hold for 12 bit times for a break */
#define BCM2835_LCR_WordLen		0x03	/* Only 7-bit and 8-bit supported. BCM document is incorrect about bit. */
#define BCM2835_LCR_7bit		0x02
#define BCM2835_LCR_8bit		0x03

#define BCM2835_MCR_RTS			0x02

#define BCM2835_LSR_TxIdle		0x40
#define BCM2835_LSR_TxEmpty		0x20	/* Can accept at least one char */
#define BCM2835_LSR_RxOver		0x02	/* Rx overrun (cleared by reading */
#define BCM2835_LSR_RxReady		0x01	/* Rx data available */

#define BCM2835_MSR_CTS			0x20	/* 1 = CTS low */

										/* cntl has various hardware flow control bits in 7..2, not defined here */
#define BCM2835_CNTL_TxEn		0x02	/* 1 = receiver enabled */
#define BCM2835_CNTL_RxEn		0x01	/* 1 = transmitter enabled */

#define BCM2835_STAT_TxNchars	0x0f000000		/* No of chars in Tx fifo (0..8) */
#define BCM2835_STAT_RxNchars	0x000f0000		/* No of chars in Tx fifo (0..8) */
#define BCM2835_STAT_TxDone		0x00000200
#define BCM2835_STAT_TxEmpty	0x00000100
#define BCM2835_STAT_CTS		0x00000080
#define BCM2835_STAT_RTS		0x00000040
#define BCM2835_STAT_TxFull		0x00000020
#define BCM2835_STAT_RxOver		0x00000010
#define BCM2835_STAT_TxIdle		0x00000008
#define BCM2835_STAT_RxIdle		0x00000004
#define BCM2835_STAT_TxSpace	0x00000002
#define BCM2835_STAT_RxChar		0x00000001		/* Receiver fifo contains 1 or more characters */

static inline int bcm2835_uart_istx(void)
{
	return ( (bcm2835_uart.stat & BCM2835_STAT_TxSpace) != 0 );
}

static inline int bcm2835_uart_isrx(void)
{
	return ( (bcm2835_uart.stat & BCM2835_STAT_RxNchars) != 0 );
}

static inline int bcm2835_uart_putc(int c)
{
	while ( !bcm2835_uart_istx() )
	{
		/* Wait till there's room */
	}
	bcm2835_uart.io = c;
	return 1;
}

static inline int bcm2835_uart_getc(void)
{
	while ( !bcm2835_uart_isrx() )
	{
		/* Wait till there's a character */
	}
	return (int)bcm2835_uart.io;
}

#endif
