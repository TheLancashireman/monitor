/*	mon-arm64-reset.S - ARM64 startup code for monitor
 *
 *	Copyright David Haworth
 *
 *	This file is part of XXXX.
 *
 *	XXXX is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	XXXX is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with XXXX.  If not, see <http://www.gnu.org/licenses/>.
*/

/* mon_reset() - startup code after reset
 *
 * This code is placed at the reset location, wherever that happens to be.
 *
 * On a raspberry pi 3 use
 *	kernel_old=1
 * in config.txt to load the image at 0. Also use
 *	arm_control=0x200				(start in ARM64 mode)
 *	disable_commandline_tags=1		(not sure, but bare-metal images don't have command lines)
 * There might be more modern versions of these config options.
 * 
 * Initialise processor then call core0_start on core 0.
 * Cores 1, 2 and 3 loop forever
*/
	.globl	mon_reset
	.globl	mon_startaddr

	.extern	core0_start
	.extern	core1_start
	.extern	core2_start
	.extern	core3_start

	.extern c0_initialsp
	.extern c1_initialsp
	.extern c2_initialsp
	.extern c3_initialsp

	.section	.reset, "ax"

/* This is where the bootloader jumps to.
 * Insert a few dummy instructions so that, if the bootloader loads at zero, mon_reset is not at zero.
 * This allows other cores to be released at mon_reset.
*/
mon_startaddr:
	nop
	nop
	nop
	nop

/* This is where we start cores 1, 2 and 3 from the monitor. The address has to be non-zero
*/
mon_reset:
	msr		DAIFSet, 0xf	/* Disable interrupts and exceptions */

/* Clear all registers
*/
	mov		x0, xzr
	mov		x1, xzr
	mov		x2, xzr
	mov		x3, xzr
	mov		x4, xzr
	mov		x5, xzr
	mov		x6, xzr
	mov		x7, xzr
	mov		x8, xzr
	mov		x9, xzr
	mov		x10, xzr
	mov		x11, xzr
	mov		x12, xzr
	mov		x13, xzr
	mov		x14, xzr
	mov		x15, xzr
	mov		x16, xzr
	mov		x17, xzr
	mov		x18, xzr
	mov		x19, xzr
	mov		x20, xzr
	mov		x21, xzr
	mov		x22, xzr
	mov		x23, xzr
	mov		x24, xzr
	mov		x25, xzr
	mov		x26, xzr
	mov		x27, xzr
	mov		x28, xzr
	mov		x29, xzr
	mov		x30, xzr

	fmov	d0, xzr
	fmov	d1, xzr
	fmov	d2, xzr
	fmov	d3, xzr
	fmov	d4, xzr
	fmov	d5, xzr
	fmov	d6, xzr
	fmov	d7, xzr
	fmov	d8, xzr
	fmov	d9, xzr
	fmov	d10, xzr
	fmov	d11, xzr
	fmov	d12, xzr
	fmov	d13, xzr
	fmov	d14, xzr
	fmov	d15, xzr
	fmov	d16, xzr
	fmov	d17, xzr
	fmov	d18, xzr
	fmov	d19, xzr
	fmov	d20, xzr
	fmov	d21, xzr
	fmov	d22, xzr
	fmov	d23, xzr
	fmov	d24, xzr
	fmov	d25, xzr
	fmov	d26, xzr
	fmov	d27, xzr
	fmov	d28, xzr
	fmov	d29, xzr
	fmov	d30, xzr
	fmov	d31, xzr

/*	Now find out which core we're on.
*/
	mrs		x5, mpidr_el1
	mov		x6, #0xFF000000
	bic		x5, x5, x6
	cbz		x5, core_zero			/* Core zero: continue startup */

	sub		x6, x5, #1
	cbz		x6, core_one

	sub		x6, x5, #2
	cbz		x6, core_two

	sub		x6, x5, #3
	cbz		x6, core_three

stop:								/* Other cores (if any) wait here forever - the demo doesn't need them */
	wfi
	b		stop

/* Core 0 - go to core0_start
*/
core_zero:
	ldr		x1, c0_sp_addr			/* Load the kernel stack pointer */
	mov		sp, x1
	bl		core0_start
	b		mon_reset				/* if the core start function returns, rinse and repeat. */

/* Cores 1, 2 and 3 - go to corex_start
*/
core_one:
	ldr		x1, c1_sp_addr			/* Load the core 1 stack pointer */
	mov		sp, x1
	bl		core1_start
	b		mon_reset				/* if the core start function returns, rinse and repeat. */

core_two:
	ldr		x1, c2_sp_addr			/* Load the core 2 stack pointer */
	mov		sp, x1
	bl		core2_start
	b		mon_reset				/* if the core start function returns, rinse and repeat. */

core_three:
	ldr		x1, c3_sp_addr			/* Load the core 3 stack pointer */
	mov		sp, x1
	bl		core3_start
	b		mon_reset				/* if the core start function returns, rinse and repeat. */

	.balign	8
c0_sp_addr:
	.quad	c0_initialsp			/* This is the address of where the core 0 stack pointer is held. */
c1_sp_addr:
	.quad	c1_initialsp			/* This is the address of where the core 1 stack pointer is held. */
c2_sp_addr:
	.quad	c2_initialsp			/* This is the address of where the core 2 stack pointer is held. */
c3_sp_addr:
	.quad	c3_initialsp			/* This is the address of where the core 3 stack pointer is held. */
