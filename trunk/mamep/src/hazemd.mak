###########################################################################
#
#   mame.mak
#
#   MAME target makefile
#
#   Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


#-------------------------------------------------
# mamedriv.c is MAME-specific and contains the
# list of drivers
#-------------------------------------------------

COREOBJS += $(OBJ)/hazemddriv.o \
	    $(OBJ)/hazesmsdriv.o


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
# this is the list of driver libaries that
# comprise MAME
#-------------------------------------------------

DRVLIBS = \
	$(OBJ)/misc.a \

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

$(OBJ)/misc.a: \
	$(OBJ)/drivers/megadriv.o \
	$(OBJ)/drivers/sms.o \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

