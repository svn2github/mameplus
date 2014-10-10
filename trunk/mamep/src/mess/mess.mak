###########################################################################
#
#   mess.mak
#
#   MESS target makefile
#
###########################################################################

ifeq ($(TARGET),mess)
# In order to keep dependencies reasonable, we exclude objects in the base of
# $(SRC)/emu, as well as all the OSD objects and anything in the $(OBJ) tree
depend: maketree $(MAKEDEP_TARGET)
	@echo Rebuilding depend_emu.mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... $(SRC)/emu > depend_emu.mak
	@echo Rebuilding depend_$(TARGET).mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... $(SRC)/$(TARGET) > depend_$(TARGET).mak

INCPATH += \
	-I$(SRC)/$(TARGET) \
	-I$(OBJ)/$(TARGET)/layout \
	-I$(SRC)/emu \
	-I$(OBJ)/emu \
	-I$(OBJ)/emu/layout \
	-I$(SRC)/lib/util \
	-I$(SRC)/osd \
	-I$(SRC)/osd/$(OSD) \

endif

# include MESS core defines
include $(SRC)/mess/messcore.mak


#-------------------------------------------------
# specify available sound cores; some of these are
# only for MAME and so aren't included
#-------------------------------------------------

SOUNDS += VRC6

#-------------------------------------------------
# specify available machine cores
#-------------------------------------------------

MACHINES += YM2148

#-------------------------------------------------
# specify available bus cores
#-------------------------------------------------

BUSES += A7800
BUSES += A800
BUSES += GAMEBOY
BUSES += GBA
BUSES += GENERIC
BUSES += MIDI
BUSES += MEGADRIVE
BUSES += MSX_SLOT
BUSES += NEOGEO
BUSES += NES
BUSES += PCE
BUSES += SATURN
BUSES += SEGA8
BUSES += SMS_CTRL
BUSES += SMS_EXP
BUSES += SNES
BUSES += VCS
BUSES += VCS_CTRL
BUSES += WSWAN

#-------------------------------------------------
# this is the list of driver libraries that
# comprise MESS plus messdriv.o which contains
# the list of drivers
#-------------------------------------------------

DRVLIST += \
	$(MESSOBJ)/mess.lst \

DRVLIBS += \
	$(MESSOBJ)/ascii.a \
	$(MESSOBJ)/atari.a \
	$(MESSOBJ)/bandai.a \
	$(MESSOBJ)/funtech.a \
	$(MESSOBJ)/nec.a \
	$(MESSOBJ)/nintendo.a \
	$(MESSOBJ)/sega.a \
	$(MESSOBJ)/snk.a \
	$(MESSOBJ)/sony.a \

ifneq ($(MAMEMESS),)
DRVLIBS += \
	$(MESSOBJ)/mame.a
endif

#-------------------------------------------------
# the following files are MAME components and
# shared across a number of drivers
#-------------------------------------------------

$(MESSOBJ)/mame.a: \
	$(MAME_VIDEO)/tia.o         \
	$(MAME_MACHINE)/atari.o     \
	$(MAME_VIDEO)/atari.o       \
	$(MAME_VIDEO)/antic.o       \
	$(MAME_AUDIO)/snes_snd.o    \
	$(MAME_MACHINE)/snes.o      \
	$(MAME_MACHINE)/megadriv.o  \
	$(MAME_VIDEO)/neogeo.o      \
	$(MAME_MACHINE)/neoprot.o   \
	$(MAME_MACHINE)/neocrypt.o  \
	$(MAME_DRIVERS)/cps1.o      \
	$(MAME_VIDEO)/cps1.o        \

#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MESSOBJ)/ascii.a:             \
	$(MESS_DRIVERS)/msx.o       \
	$(MESS_MACHINE)/msx.o       \
	$(MESS_MACHINE)/msx_switched.o \
	$(MESS_MACHINE)/msx_matsushita.o \
	$(MESS_MACHINE)/msx_s1985.o \
	$(MESS_MACHINE)/msx_systemflags.o \

$(MESSOBJ)/atari.a:             \
	$(MESS_DRIVERS)/a2600.o     \
	$(MESS_DRIVERS)/a7800.o $(MESS_VIDEO)/maria.o \
	$(MESS_DRIVERS)/atari400.o $(MESS_MACHINE)/atarifdc.o \

$(MESSOBJ)/bandai.a:            \
	$(MESS_DRIVERS)/wswan.o $(MESS_AUDIO)/wswan.o $(MESS_MACHINE)/wswan.o $(MESS_VIDEO)/wswan.o \

$(MESSOBJ)/funtech.a:           \
	$(MESS_DRIVERS)/supracan.o  \

$(MESSOBJ)/nec.a:               \
	$(MESS_MACHINE)/pce.o       \
	$(MESS_MACHINE)/pce_cd.o    \
	$(MESS_DRIVERS)/pce.o       \

$(MESSOBJ)/nintendo.a:          \
	$(MESS_VIDEO)/nes.o         \
	$(MESS_MACHINE)/nes.o       \
	$(MESS_DRIVERS)/nes.o       \
	$(MESS_MACHINE)/snescx4.o   \
	$(MESS_DRIVERS)/snes.o      \
	$(MESS_AUDIO)/gb.o          \
	$(MESS_VIDEO)/gb_lcd.o      \
	$(MESS_MACHINE)/gb.o        \
	$(MESS_DRIVERS)/gb.o        \
	$(MESS_DRIVERS)/gba.o       \
	$(MESS_VIDEO)/gba.o         \

$(MESSOBJ)/sega.a:              \
	$(MESS_DRIVERS)/megadriv.o  \
	$(MESS_MACHINE)/megacd.o    \
	$(MESS_MACHINE)/megacdcd.o  \
	$(MESS_MACHINE)/mega32x.o   \
	$(MESS_DRIVERS)/segapico.o  \
	$(MESS_DRIVERS)/saturn.o    \
	$(MESS_MACHINE)/sms.o       \
	$(MESS_DRIVERS)/sms.o       \

$(MESSOBJ)/snk.a:               \
	$(MESS_DRIVERS)/ng_aes.o    \
	$(MAME_MACHINE)/neocrypt.o  \
	$(MAME_MACHINE)/neoprot.o   \
	$(MAME_MACHINE)/neoboot.o   \
	$(MAME_VIDEO)/neogeo_spr.o  \
	$(MAME_DRIVERS)/neogeo.o    \
	$(MESS_DRIVERS)/ngp.o       \
	$(MESS_VIDEO)/k1ge.o        \

$(MESSOBJ)/sony.a:              \
	$(MESS_DRIVERS)/psx.o       \
	$(MESS_MACHINE)/psxcport.o  \
	$(MESS_MACHINE)/psxcd.o     \
	$(MESS_MACHINE)/psxcard.o   \
	$(MESS_MACHINE)/psxanalog.o \
	$(MESS_MACHINE)/psxmultitap.o \
	$(MESS_DRIVERS)/pockstat.o  \




#-------------------------------------------------
# miscellaneous dependencies
#-------------------------------------------------

$(MESS_MACHINE)/snescx4.o: $(MESSSRC)/machine/cx4ops.inc \
				$(MESSSRC)/machine/cx4oam.inc \
				$(MESSSRC)/machine/cx4fn.inc \
				$(MESSSRC)/machine/cx4data.inc \

$(MESS_MACHINE)/nes_slot.o:  $(MESSSRC)/machine/nes_ines.inc \
				$(MESSSRC)/machine/nes_pcb.inc \
				$(MESSSRC)/machine/nes_unif.inc \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(MESS_MACHINE)/megacd.o:   $(MESS_LAYOUT)/megacd.lh
$(MESS_DRIVERS)/sms.o:      $(MESS_LAYOUT)/sms1.lh
$(MESS_DRIVERS)/wswan.o:    $(MESS_LAYOUT)/wswan.lh

#-------------------------------------------------
# MESS special OSD rules
#-------------------------------------------------

include $(MESSSRC)/osd/$(OSD)/$(OSD).mak

#-------------------------------------------------
# mamep: driver list dependencies
#-------------------------------------------------

#FIXME
$(MESSOBJ)/%.lst:	$(MESSSRC)/%.lst
	@echo Generating $@...
	@echo #include "$<" > $@.h
	$(CC) $(CDEFS) $(INCPATH) -I. -E $@.h -o $@

