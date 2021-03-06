# $Id: Makefile,v 1.4 2001/05/03 13:43:42 fma Exp $

#######################################
# Base dir of your m68k gcc toolchain #
#######################################

BASEDIR = $(NEODEV)
AS = as
LD = gcc
CC = gcc
AR = ar
OBJC = objcopy
BIN2O = bin2elf
GFXCC = gfxcc
FIXCNV = fixcnv

#######################################
# Path to libraries and include files #
#######################################

INCDIR = $(BASEDIR)/m68k/include
LIBDIR = $(BASEDIR)/m68k/lib
TMPDIR = $(BASEDIR)/tmp

###################################
# Output: {cart, cd} *lower case* #
###################################
#OUTPUT = cart
OUTPUT = cd

############################
# Settings for cart output #
############################
ROMSIZE = 0x80000
PADBYTE = 0xFF

##############################
# Object Files and Libraries #
##############################

OBJS = $(TMPDIR)/crt0_$(OUTPUT).o $(TMPDIR)/main.o $(TMPDIR)/palettes.o $(TMPDIR)/maps.o
LIBS = -lDATlib -lprocess -lc -lgcc -lmath

#####################
# Compilation Flags #
#####################

ASFLAGS = -m68000 --register-prefix-optional
LDFLAGS = -Wl,-T$(BASEDIR)/src/system/neo$(OUTPUT).x -Xlinker -Map=output.map 
CCFLAGS = -m68000 -O3 -Wall -fomit-frame-pointer -ffast-math -fno-builtin -nostartfiles -nodefaultlibs -D__$(OUTPUT)__
ARFLAGS = cr
DEBUG = -g

##################
# FIX Definition #
##################
FIXFILES = gfx/fix_font.bmp

##############
# Make rules #
##############

ifeq ($(OUTPUT),cart)
dev_p1.rom : test.o
	$(OBJC) --gap-fill=$(PADBYTE) --pad-to=$(ROMSIZE) -R .data -O binary $< $@
#	$(OBJC) --gap-fill=$(PADBYTE) -R .data -O binary $< $@
#	romwak /w char.bin 202-c1.bin 202-c2.bin 
	copy 202-.c1 202-c1.bin
	copy 202-.c2 202-c2.bin
	romwak /p 202-c1.bin 202-c1.bin 1024 255 
	romwak /p 202-c2.bin 202-c2.bin 1024 255 
	romwak /f dev_p1.rom 202-p1.p1
	copy 202-p1.p1 c:\emu\mame\roms\puzzledp\202-p1.p1 /y
	copy 202-c1.bin c:\emu\mame\roms\puzzledp\202-c1.c1 /y
	copy 202-c2.bin c:\emu\mame\roms\puzzledp\202-c2.c2 /y
	
else
test.prg : test.o
	$(OBJC) -O binary $< $@
	copy test.cd PB_CHR.SPR
	copy test.fix PB_FIX.FIX
	copy test.prg PB_PROG.PRG
	copy PB_CHR.SPR output\cd\ /y
	copy PB_FIX.FIX output\cd\ /y
	copy PB_PROG.PRG output\cd\ /y


endif

#test.o : test.fix $(OBJS)
test.o : test.fix $(OBJS)
	$(LD) -L$(LIBDIR) $(CCFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(TMPDIR)/%.o: %.c
	$(CC) -I$(INCDIR) $(CCFLAGS) -c $< -o $@

$(TMPDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(TMPDIR)/*.*
	rm -f palettes.pal
	rm -f test.o
	rm -f test.prg
	rm -f test.fix
	rm -f test.spr
	rm -f dev_p1.rom

convert:
	..\..\bin\buildchar chardata.xml
	..\..\bin\charsplit char.bin -cd test

cdiso:
	del test.iso
	..\..\bin\mkisofs -o test.iso .\output\cd
	copy test.iso .\output\image\image.iso

cdzip:
	del .\output\emu\demo.zip
	del demo.zip
	'C:\Program Files\7-Zip\7z.exe' a demo.zip .\output\cd
	copy demo.zip .\output\emu

runr:
	..\..\..\raine\raine32 output\emu\demo.zip

run:
	c:\emu\mame\mame neocdz -cdrom output\image\image.cue -window -skip_gameinfo -waitvsync

############
# FIX Rule #
############

test.fix : $(FIXFILES)
	$(FIXCNV) $(FIXFILES) -o $@ -pal fix.pal
