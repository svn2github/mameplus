###########################################################################
#
#   neocpsmame.mak
#
#   NEOGEO & CPS1,2 driver-specific makefile
#	Use make TARGET=NEOCPSMAME to build
#
#   Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


#-------------------------------------------------
# driver.c is MAME-specific and contains the
# list of drivers
#-------------------------------------------------

COREOBJS += $(OBJ)/mamedriv.o



#-------------------------------------------------
# specify available CPU cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

CPUS += Z80
CPUS += M68000



#-------------------------------------------------
# specify available sound cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

SOUNDS += YM2151
SOUNDS += OKIM6295
SOUNDS += QSOUND
SOUNDS += YM2610
SOUNDS += YM2610B



#-------------------------------------------------
# this is the list of driver libaries that
# comprise MAME
#-------------------------------------------------

DRVLIBS = \
	$(OBJ)/capcom.a \
	$(OBJ)/neogeo.a \
	$(OBJ)/shared.a \



#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

$(OBJ)/shared.a: \
	$(OBJ)/machine/pd4990a.o \


#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(OBJ)/capcom.a: \
	$(OBJ)/drivers/cps1.o $(OBJ)/vidhrdw/cps1.o \
	$(OBJ)/drivers/cps2.o \
	$(OBJ)/machine/kabuki.o \

$(OBJ)/neogeo.a: \
	$(OBJ)/drivers/neogeo.o $(OBJ)/machine/neogeo.o $(OBJ)/vidhrdw/neogeo.o \
	$(OBJ)/machine/neoboot.o \
	$(OBJ)/machine/neocrypt.o \
	$(OBJ)/machine/neoprot.o \

