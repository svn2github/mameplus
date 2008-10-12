###########################################################################
#
#   neocpspgm.mak
#
#   Small driver-specific example makefile
#	Use make SUBTARGET=neocpspgm to build
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

MAMESRC = $(SRC)/mame
MAMEOBJ = $(OBJ)/mame

AUDIO = $(MAMEOBJ)/audio
DRIVERS = $(MAMEOBJ)/drivers
LAYOUT = $(MAMEOBJ)/layout
MACHINE = $(MAMEOBJ)/machine
VIDEO = $(MAMEOBJ)/video

OBJDIRS += \
	$(AUDIO) \
	$(DRIVERS) \
	$(LAYOUT) \
	$(MACHINE) \
	$(VIDEO) \

DEFS += -DNEOCPSPGM



#-------------------------------------------------
# specify available CPU cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

CPUS += Z80
CPUS += M68000
CPUS += SH2
CPUS += ARM7
CPUS += PIC16C57



#-------------------------------------------------
# specify available sound cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

SOUNDS += CUSTOM
SOUNDS += YM2203
SOUNDS += YM2151
SOUNDS += OKIM6295
SOUNDS += MSM5205
SOUNDS += QSOUND
SOUNDS += YM2610
SOUNDS += YM2610B
SOUNDS += CDDA
SOUNDS += ICS2115



#-------------------------------------------------
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS = \
	$(MAMEOBJ)/mamedriv.o \

ifneq ($(USE_DRIVER_SWITCH),)
DRVLIBS += $(MAMEOBJ)/mameplusdriv.o \
            $(MAMEOBJ)/mamehbdriv.o \
            $(MAMEOBJ)/mamedecrypteddriv.o
endif

DRVLIBS += \
	$(MAMEOBJ)/capcom.a \
	$(MAMEOBJ)/neogeo.a \
	$(MAMEOBJ)/igs.a \



#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MAMEOBJ)/capcom.a: \
	$(DRIVERS)/cps1.o $(VIDEO)/cps1.o \
	$(DRIVERS)/cps2.o \
	$(DRIVERS)/cps3.o $(AUDIO)/cps3.o \
	$(DRIVERS)/fcrash.o \
	$(MACHINE)/cps2crpt.o \
	$(MACHINE)/kabuki.o \

$(MAMEOBJ)/neogeo.a: \
	$(DRIVERS)/neogeo.o $(VIDEO)/neogeo.o \
	$(MACHINE)/neoboot.o \
	$(MACHINE)/neocrypt.o \
	$(MACHINE)/neoprot.o \

$(MAMEOBJ)/igs.a: \
	$(DRIVERS)/pgm.o $(VIDEO)/pgm.o \
	$(MACHINE)/pgmcrypt.o \
	$(MACHINE)/pgmprot.o \
	$(MACHINE)/pgmy2ks.o \



#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/neogeo.o:	$(LAYOUT)/neogeo.lh

$(DRIVERS)/cps3.o:	$(LAYOUT)/cps3.lh



#-------------------------------------------------
# misc dependencies
#-------------------------------------------------

$(DRIVERS)/neogeo.o:	$(MAMESRC)/drivers/neodrvr.c
