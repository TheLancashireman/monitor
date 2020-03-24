/*  mon-bcm2835.c - UART on bcm2835 etc. (raspberry pi)
 *
 *  Copyright 2020 David Haworth
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mon-bcm2835.h"


/* bcm2835_uart_init() - initialise the UART
 *
 * Initialize to the selected baud rate, parity and bits.
 * At the moment, baud and parity must be 115200 and none (0).
 * Bits can be 7 or 8
*/
void bcm2835_uart_init(uint32_t baud, uint32_t bits, uint32_t parity)
{
	if ( baud == 115200 &&			/* ToDo: Other bitrates */
		 (bits == 7 || bits == 8) &&
		 (parity == 0 ) )
	{
		bcm2835_enable(BCM2835_AUX_uart);

		bcm2835_uart.cntl = 0;				/* Tx and Rx disabled */
		bcm2835_uart.ier = 0;				/* Interrupts disabled */
		bcm2835_uart.lcr = (bits==7 ? BCM2835_LCR_7bit : BCM2835_LCR_8bit);
		bcm2835_uart.mcr = 0;				/* RTS high (not used) */
		bcm2835_uart.baud = 270;			/* 115200 */

		bcm2835_gpio_pinconfig(14, BCM2835_pinfunc_alt5, BCM2835_pinpull_none);	/* Transmit pin gpio14 */
		bcm2835_gpio_pinconfig(15, BCM2835_pinfunc_alt5, BCM2835_pinpull_none);	/* Receive pin gpio15 */

	    bcm2835_uart.cntl = BCM2835_CNTL_TxEn | BCM2835_CNTL_RxEn;
	}
	else
	{
		bcm2835_disable(BCM2835_AUX_uart);
	}
}

/* bcm2835_gpio_pinconfig() - set up GPIO pin
 *
 * Select input, output or alternative functions
 * Set open, pull-down or pull-up.
*/
void bcm2835_gpio_pinconfig(uint32_t pin, uint32_t fsel, uint32_t pud)
{
	int index, shift;
	uint32_t mask, val;
	volatile int delay;

	index = pin/10;		/* fsel: 3 bits per field, 10 fields per register */
	shift = (pin%10) * 3;
	mask = 0x7 << shift;
	val = fsel << shift;

	bcm2835_gpio.fsel[index] = (bcm2835_gpio.fsel[index] & ~mask) | val;

	index = pin/32;		/* pudclk: 1 bit per field, 32 fields per register */
	shift = (pin%32);
	mask = 0x1 << shift;

	bcm2835_gpio.pud = pud;
	for ( delay = 0; delay < 150; delay++ )
	{
		/* Wait for 150 cycles setup time for the control signal */
	}
	bcm2835_gpio.pudclk[index] |= mask;
	for ( delay = 0; delay < 150; delay++ )
	{ 
		/* Wait for 150 cycles hold time for the control signal */
	}
	bcm2835_gpio.pudclk[index] &= ~mask;
	bcm2835_gpio.pud = 0;
}
