# Name: Makefile
# Project: PowerSwitch
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
# This Revision: $Id: Makefile 277 2007-03-20 10:53:33Z cs $

DEVICE = atmega88
#AVRDUDE = avrdude -c stk500v2 -P avrdoper -p $(DEVICE)
#AVRDUDE = avrdude -p atmega88 -P /dev/parport0 -c dapa
AVRDUDE = avrdude -p atmega88 -c usbasp

# Choose your favorite programmer and interface above.
# attiny2313 fuses: -U lfuse:w:0xcf:m -U hfuse:w:0xdd:m
# atmega 88 fuses: -U lfuse:w:0xff:m -U hfuse:w:0xde:m

# dreamcast stick:
#CFLAGS = -DNO_SELECT -DICANTREAD
# Namco stick
CFLAGS = -DINVERTTRIGGERS
# HRAP
#CFLAGS = -DNOPSX

COMPILE = avr-gcc -Wall -Os $(CFLAGS) -Iusbdrv -I. -DF_CPU=16000000 -mmcu=$(DEVICE) #-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o ctrlr.o psx.o

# symbolic targets:
all:	ctrlr.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:ctrlr.hex:i

flash_new: all
	echo Make sure crystal is in.
	$(AVRDUDE) -U lfuse:w:0xff:m -U hfuse:w:0xde:m -U flash:w:ctrlr.hex:i


# Fuse low byte:
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal @ 12 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
#
# Fuse high byte:
# 0xdb = 1 1 0 1   1 0 1 1
#        ^ ^ ^ ^   \-+-/ ^
#        | | | |     |   +---- RSTDISBL (disable external reset -> enabled)
#        | | | |     +-------- BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        +-------------------- DWEN (debug wire enable)
fuse_tiny2313:	# only needed for attiny2313
	$(AVRDUDE) -U hfuse:w:0xdb:m -U lfuse:w:0xef:m


clean:
	rm -f ctrlr.hex ctrlr.lst ctrlr.obj ctrlr.cof ctrlr.list ctrlr.map ctrlr.eep.hex ctrlr.bin *.o usbdrv/*.o ctrlr.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
ctrlr.bin:	$(OBJECTS)
	$(COMPILE) -o ctrlr.bin $(OBJECTS)

ctrlr.hex:	ctrlr.bin
	rm -f ctrlr.hex ctrlr.eep.hex
	avr-objcopy -j .text -j .data -O ihex ctrlr.bin ctrlr.hex
#	./checksize ctrlr.bin
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	ctrlr.bin
	avr-objdump -d ctrlr.bin

cpp:
	$(COMPILE) -E ctrlr.c
