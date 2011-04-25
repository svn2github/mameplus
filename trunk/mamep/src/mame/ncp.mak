###########################################################################
#
#   ncp.mak
#
#   NeoGeo, CPS1/2/3, PGM driver-specific example makefile
#   Use make SUBTARGET=ncp to build
#
#   This is an unofficial version based on MAME.
#   Please do not send any reports from this build to the MAME team.
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

DEFS += -DNCP



#-------------------------------------------------
# specify available CPU cores; some of these are
# only for MESS, but are included so that they get
# updated with any MAME core changes
#-------------------------------------------------

CPUS += Z80
CPUS += I386
CPUS += MCS48
CPUS += M680X0
CPUS += ARM7
CPUS += SH2
CPUS += PIC16C5X

#-------------------------------------------------
# specify available sound cores; some of these are
# only for MESS and so aren't included
#-------------------------------------------------

SOUNDS += YM2151
SOUNDS += YM2203
SOUNDS += YM2608
SOUNDS += YM2610
ifneq ($(WINUI),)
SOUNDS += VLM5030
endif
SOUNDS += MSM5205
SOUNDS += OKIM6295
SOUNDS += QSOUND
SOUNDS += CDDA
SOUNDS += ICS2115

#-------------------------------------------------
# this is the list of driver libraries that
# comprise MAME plus mamedriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIST += \
	$(MAMEOBJ)/mame.lst \
	$(MAMEOBJ)/mameplus.lst \
	$(MAMEOBJ)/mamehb.lst \
	$(MAMEOBJ)/mamedecrypted.lst \

DRVLIBS = \
	$(EMUDRIVERS)/emudummy.o \

DRVLIBS += \
	$(MAMEOBJ)/capcom.a \
	$(MAMEOBJ)/igs.a \
	$(MAMEOBJ)/neogeo.a \



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

$(MAMEOBJ)/igs.a: \
	$(DRIVERS)/pgm.o $(VIDEO)/pgm.o \
	$(MACHINE)/pgmcrypt.o \
	$(MACHINE)/pgmprot.o \

$(MAMEOBJ)/neogeo.a: \
	$(DRIVERS)/neogeo.o $(VIDEO)/neogeo.o \
	$(MACHINE)/neoboot.o \
	$(MACHINE)/neocrypt.o \
	$(MACHINE)/neoprot.o \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/neogeo.o:	$(LAYOUT)/neogeo.lh

$(DRIVERS)/cps3.o:	$(LAYOUT)/cps3.lh

$(MAMEOBJ)/mamedriv.o:	$(LAYOUT)/pinball.lh



#-------------------------------------------------
# misc dependencies
#-------------------------------------------------

$(DRIVERS)/neogeo.o:	$(MAMESRC)/drivers/neodrvr.c

#-------------------------------------------------
# mamep: driver list dependencies
#-------------------------------------------------

#FXIXME
$(MAMEOBJ)/%.lst:	$(MAMESRC)/%.lst
	@echo Generating $@...
	@echo #include "$<" > $@.h
	$(CC) $(CDEFS) $(INCPATH) -I. -E $@.h -o $@

