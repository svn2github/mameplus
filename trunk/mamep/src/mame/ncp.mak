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
CPUS += DSP16A

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
SOUNDS += YMZ770


#-------------------------------------------------
# specify available machine cores
#-------------------------------------------------

MACHINES += EEPROMDEV
MACHINES += INTELFLASH
MACHINES += WD33C93
MACHINES += TIMEKPR
MACHINES += SCSI
MACHINES += V3021
MACHINES += PD4990A_OLD
MACHINES += UPD1990A
MACHINES += Z80CTC
MACHINES += I8255

#-------------------------------------------------
# specify available bus cores
#-------------------------------------------------

BUSES += SCSI
BUSES += NEOGEO

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
	$(DRIVERS)/kenseim.o \
	$(DRIVERS)/cps2.o \
	$(DRIVERS)/cps3.o $(AUDIO)/cps3.o \
	$(DRIVERS)/fcrash.o \
	$(MACHINE)/cps2crpt.o \
	$(MACHINE)/kabuki.o \

$(MAMEOBJ)/igs.a: \
	$(DRIVERS)/pgm.o $(VIDEO)/pgm.o \
	$(DRIVERS)/pgm2.o \
	$(MACHINE)/igs036crypt.o \
	$(MACHINE)/pgmcrypt.o \
	$(MACHINE)/pgmprot_orlegend.o \
	$(MACHINE)/pgmprot_igs027a_type1.o \
	$(MACHINE)/pgmprot_igs027a_type2.o \
	$(MACHINE)/pgmprot_igs027a_type3.o \
	$(MACHINE)/pgmprot_igs025_igs012.o \
	$(MACHINE)/pgmprot_igs025_igs022.o \
	$(MACHINE)/pgmprot_igs025_igs028.o \
	$(MACHINE)/igs025.o \
	$(MACHINE)/igs022.o \
	$(MACHINE)/igs028.o \

$(MAMEOBJ)/neogeo.a: \
	$(DRIVERS)/neogeo.o $(VIDEO)/neogeo.o \
	$(DRIVERS)/neogeo_noslot.o \
	$(VIDEO)/neogeo_spr.o \
	$(MACHINE)/neoboot.o \
	$(MACHINE)/neocrypt.o \
	$(MACHINE)/neoprot.o \
	$(MACHINE)/ng_memcard.o \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(DRIVERS)/neogeo.o:	$(LAYOUT)/neogeo.lh

$(DRIVERS)/cps3.o:	$(LAYOUT)/sfiii2.lh

$(DRIVERS)/kenseim.o:   $(LAYOUT)/kenseim.lh

#-------------------------------------------------
# mamep: driver list dependencies
#-------------------------------------------------

#FIXME
$(MAMEOBJ)/%.lst:	$(MAMESRC)/%.lst
	@echo Generating $@...
	@echo #include "$<" > $@.h
	$(CC) $(CDEFS) $(INCPATH) -I. -E $@.h -o $@

