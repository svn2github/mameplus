###########################################################################
#
#   tiny.mak
#
#   Small driver-specific example makefile
#	Use make SUBTARGET=tiny to build
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit  http://mamedev.org for licensing and usage restrictions.
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
# Specify all the CPU cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

CPUS += Z80
CPUS += M6502
CPUS += I386
CPUS += M6800
CPUS += M6805
CPUS += M6809
CPUS += ARM7
CPUS += M680X0
CPUS += V30MZ
CPUS += MCS48
CPUS += PIC16C5X
CPUS += H6280
CPUS += LR35902
CPUS += SUPERFX
CPUS += SSP1601
CPUS += SH2
CPUS += UPD7725
CPUS += G65816
CPUS += SPC700
CPUS += TLCS900



#-------------------------------------------------
# Specify all the sound cores necessary for the
# drivers referenced in tiny.c.
#-------------------------------------------------

SOUNDS += SAMPLES
SOUNDS += DAC
SOUNDS += SPEAKER
SOUNDS += DISCRETE
SOUNDS += AY8910
SOUNDS += YM2151
SOUNDS += YM2413
SOUNDS += YM2612
SOUNDS += OKIM6295
SOUNDS += QSOUND
SOUNDS += MSM5205
SOUNDS += SN76496
SOUNDS += POKEY
SOUNDS += TIA
SOUNDS += NES
SOUNDS += CDDA
SOUNDS += WAVE
SOUNDS += K051649
SOUNDS += HC55516
SOUNDS += ASTROCADE
SOUNDS += C6280
SOUNDS += RF5C68
SOUNDS += T6W28
ifneq ($(WINUI),)
SOUNDS += VLM5030
endif



#-------------------------------------------------
# This is the list of files that are necessary
# for building all of the drivers referenced
# in tiny.c
#-------------------------------------------------

DRVLIST += \
	$(MAMEOBJ)/mame.lst \
	$(MAMEOBJ)/mameplus.lst \
	$(MAMEOBJ)/mamehb.lst \
	$(MAMEOBJ)/mamedecrypted.lst \

DRVLIBS += \
	$(EMUDRIVERS)/emudummy.o \
	$(MACHINE)/ticket.o \
	$(DRIVERS)/astrocde.o $(VIDEO)/astrocde.o \
	$(DRIVERS)/gridlee.o $(AUDIO)/gridlee.o $(VIDEO)/gridlee.o \
	$(DRIVERS)/williams.o $(MACHINE)/williams.o $(AUDIO)/williams.o $(VIDEO)/williams.o \
	$(AUDIO)/gorf.o \
	$(AUDIO)/wow.o \

DRVLIBS += \
	$(VIDEO)/gtia.o		\
	$(MACHINE)/kabuki.o	\
	$(MACHINE)/megadriv.o	\
	$(AUDIO)/snes_snd.o	\
	$(MACHINE)/snes.o	\
	$(VIDEO)/snes.o		\
	$(MACHINE)/pcecommn.o $(VIDEO)/vdc.o \
	$(VIDEO)/ppu2c0x.o 	\
	$(DRIVERS)/maxaflex.o	\
	$(OBJ)/lib/util/opresolv.o	\


#-------------------------------------------------
# the following files are MAME components and
# shared across a number of drivers
#-------------------------------------------------

$(MAMEOBJ)/share.a: \
	$(VIDEO)/gtia.o		\
	$(MACHINE)/kabuki.o	\
	$(MACHINE)/megadriv.o	\
	$(AUDIO)/snes_snd.o	\
	$(MACHINE)/snes.o	\
	$(VIDEO)/snes.o		\
	$(MACHINE)/pcecommn.o $(VIDEO)/vdc.o \
	$(VIDEO)/ppu2c0x.o 	\
	$(DRIVERS)/maxaflex.o	\



#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/astrocde.o:	$(LAYOUT)/gorf.lh \
						$(LAYOUT)/tenpindx.lh

$(DRIVERS)/maxaflex.o:	$(LAYOUT)/maxaflex.lh

#-------------------------------------------------
# misc dependencies
#-------------------------------------------------

$(MACHINE)/snes.o:  	$(MAMESRC)/machine/snesobc1.c \
			$(MAMESRC)/machine/snescx4.c \
			$(MAMESRC)/machine/cx4ops.c \
			$(MAMESRC)/machine/cx4oam.c \
			$(MAMESRC)/machine/cx4fn.c \
			$(MAMESRC)/machine/cx4data.c \
			$(MAMESRC)/machine/snesrtc.c \
			$(MAMESRC)/machine/snessdd1.c \
			$(MAMESRC)/machine/snes7110.c \
			$(MAMESRC)/machine/snesbsx.c
$(MACHINE)/nes_mmc.o:	$(MAMESRC)/machine/nes_ines.c \
			$(MAMESRC)/machine/nes_pcb.c \
			$(MAMESRC)/machine/nes_unif.c



#-------------------------------------------------
# mamep: driver list dependencies
#-------------------------------------------------

#FXIXME
$(MAMEOBJ)/mame.lst:	$(MAMESRC)/mess.lst
	@echo Generating $@...
	@echo #include "$<" > $@.h
	@echo # > $(MAMEOBJ)/mameplus.lst
	@echo # > $(MAMEOBJ)/mamehb.lst
	@echo # > $(MAMEOBJ)/mamedecrypted.lst
	$(CC) $(CDEFS) $(INCPATH) -I. -E $@.h -o $@

