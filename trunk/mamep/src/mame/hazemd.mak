###########################################################################
#
#   mame.mak
#
#   MAME target makefile
#
#   Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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



#-------------------------------------------------
# specify available CPU cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

CPUS += Z80
CPUS += M68000
CPUS += SH2



#-------------------------------------------------
# specify available sound cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

SOUNDS += YM2612
SOUNDS += YM3438
SOUNDS += SN76496
SOUNDS += UPD7759
SOUNDS += CDDA



#-------------------------------------------------
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIBS = \
	$(MAMEOBJ)/mamemddriv.o

DRVLIBS += \
	$(MAMEOBJ)/misc.a \



#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

#-------------------------------------------------
# remaining drivers
#-------------------------------------------------

$(MAMEOBJ)/misc.a: \
	$(DRIVERS)/megadriv.o \
	$(DRIVERS)/md_games.o \
	$(DRIVERS)/md_arcad.o \
	$(DRIVERS)/md_gamul.o \
	$(DRIVERS)/md_gam32.o \
	$(DRIVERS)/ssf2md.o \
	$(DRIVERS)/sms.o \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

