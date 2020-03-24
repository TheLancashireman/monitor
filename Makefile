#	Makefile - makefile for the monitor/bootloader
#
#	Copyright 2020 David Haworth
#
#	This monitor/loader is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This monitor/loader is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this monitor/loader.  If not, see <http://www.gnu.org/licenses/>.

#	Usage:
#		make [BOARD=pi3-arm64|pi-zero] [GNU_D=</path/to/gcc>] [INSTALL_DIR=</place/to/install/]
#	Alternatively, you can set BOARD GNU_D and INSTALL_DIR as environment variables.
#
#	Targets:
#		clean: removes all object and binary files
#		default: compiles and links
#		install: objcopy the ELF file to a binary (img) file in INSTALL_DIR
#		srec: objcopy the ELF to an S-record file in the bin directory

# Select your hardware here
BOARD	?= pi3-arm64
#BOARD	?= pi-zero

# Select your compiler here
ifeq ($(BOARD), pi3-arm64)

MON_BOARD	?=	MON_PI3_ARM64
GNU_D		?=	/data1/gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-elf

CC			:=	$(GNU_D)/bin/aarch64-elf-gcc
LD			:=	$(GNU_D)/bin/aarch64-elf-ld
OBJCOPY		:=	$(GNU_D)/bin/aarch64-elf-objcopy
LDLIB_D		:=	$(GNU_D)/aarch64-elf/libc/usr/lib/
HIGH_ADDR	?=	0x20000000

ENTRY	?=	mon_reset

BOARD_OBJS	+= $(OBJ_D)/mon-arm64-reset.o
BOARD_OBJS	+= $(OBJ_D)/mon-bcm2835.o

else

MON_BOARD	?=	MON_PI_ZERO
GNU_D		?=	/data1/gnu/gcc-linaro-6.3.1-2017.02-x86_64_arm-eabi

CC			:=	$(GNU_D)/bin/arm-eabi-gcc
LD			:=	$(GNU_D)/bin/arm-eabi-ld
OBJCOPY		:=	$(GNU_D)/bin/arm-eabi-objcopy
LDLIB_D		:=	$(GNU_D)/arm-eabi/libc/usr/lib/
HIGH_ADDR	?=	0x18000000

BOARD_OBJS	+= $(OBJ_D)/mon-arm-reset.o
BOARD_OBJS	+= $(OBJ_D)/mon-bcm2835.o

ENTRY	?=	mon_trap_reset

endif

MON_MAXSIZE	?=	65536

BIN_D	= bin
OBJ_D	= obj

CC_OPT		+=	-D MON_BOARD=$(MON_BOARD)
CC_OPT		+= -I h
CC_OPT		+= -Wall
CC_OPT		+= -fno-common

CC_OPT		+= -O2

LD_OPT		+= -e $(ENTRY)
LD_OPT		+=	-L $(LDLIB_D)
LD_OPT		+=	-lc

# The monitor code
MONITOR_OBJS	+= $(BOARD_OBJS)
MONITOR_OBJS	+= $(OBJ_D)/monitor.o
MONITOR_OBJS	+= $(OBJ_D)/mon-srec.o
MONITOR_OBJS	+= $(OBJ_D)/mon-stdio.o
MONITOR_OBJS	+= $(OBJ_D)/mon-util.o
MONITOR_OBJS	+= $(OBJ_D)/board-start.o

# The loader code
LOADER_OBJS		+= $(BOARD_OBJS)
LOADER_OBJS		+= $(OBJ_D)/loadbin.o
LOADER_OBJS		+= $(OBJ_D)/loadhigh.o
LOADER_OBJS		+= $(OBJ_D)/mon-stdio.o

VPATH		+= 	bin
VPATH 		+=	s
VPATH 		+=	c

.PHONY:		default loader help clean install srec mon mon-low

default:	loader

clean:
	-rm -rf $(OBJ_D) $(BIN_D)

mon:		$(BIN_D) $(OBJ_D) $(BIN_D)/monitor.bin

loader:		$(OBJ_D) $(BIN_D) $(BIN_D)/loader.bin

# Rules for the loader that loads the monitor into high memory
# The loader contains a binary image of the monitor.
$(BIN_D)/loader.bin:		$(BIN_D)/loader.elf
	$(OBJCOPY) $< -O binary $@

$(BIN_D)/loader.elf:		$(LOADER_OBJS) l/ld-low.ldscript
	$(LD) -o $@ -T l/ld-low.ldscript $(LOADER_OBJS) $(LD_LIB) $(LD_OPT)

$(BIN_D)/loadbin.c:		$(BIN_D)/monitor.bin
	echo "const char bin_name[] = \"monitor.bin\";" > $@
	echo "const unsigned char bin_array[] = {"  >> $@
	hexdump -v -e '16/1 "0x%02X, ""\n"""' $(BIN_D)/monitor.bin | sed -e 's/, 0x .*$///' >> $@
	echo "};" >> $@
	echo "const unsigned bin_length = sizeof(bin_array);" >> $@
	echo "const unsigned bin_loadaddr = $(HIGH_ADDR);" >> $@


# Rules for the monitor, linked in high memory
$(BIN_D)/monitor.bin:	$(BIN_D)/monitor.elf
	$(OBJCOPY) $< -O binary $@

$(BIN_D)/monitor.elf:	$(MONITOR_OBJS) l/ld-$(HIGH_ADDR).ldscript
	$(LD) -o $@ -T l/ld-$(HIGH_ADDR).ldscript $(MONITOR_OBJS) $(LD_LIB) $(LD_OPT)

# General rules
$(OBJ_D)/%.o:  %.c
	$(CC) $(CC_OPT) -o $@ -c $<

$(OBJ_D)/%.o:  %.S
	$(CC) $(CC_OPT) -o $@ -c $<

$(BIN_D):
	mkdir -p bin

$(OBJ_D):
	mkdir -p obj

# For testing with an old version of the monitor already installed and running.
srec:		loader
	$(OBJCOPY) bin/loader.elf -O srec --srec-forceS3 /dev/stdout | dos2unix | egrep -v '^S3..........00*..$$' > bin/loader.srec
