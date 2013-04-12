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
	@echo Rebuilding depend_$(TARGET).mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... src/$(TARGET) > depend_$(TARGET).mak

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
	$(MAME_VIDEO)/snes.o        \
	$(MAME_MACHINE)/megadriv.o  \
	$(MAME_MACHINE)/megacd.o    \
	$(MAME_MACHINE)/megacdcd.o  \
	$(MAME_MACHINE)/mega32x.o   \
	$(MAME_MACHINE)/megavdp.o   \
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
	$(MESS_MACHINE)/msx_slot.o  \

$(MESSOBJ)/atari.a:             \
	$(MESS_MACHINE)/ataricrt.o  \
	$(MESS_MACHINE)/atarifdc.o  \
	$(MESS_DRIVERS)/atari400.o  \
	$(MESS_MACHINE)/a7800.o     \
	$(MESS_DRIVERS)/a7800.o     \
	$(MESS_VIDEO)/a7800.o       \
	$(MESS_DRIVERS)/a2600.o     \
	$(MESS_MACHINE)/vcs_joy.o   \
	$(MESS_MACHINE)/vcs_joybooster.o\
	$(MESS_MACHINE)/vcs_keypad.o    \
	$(MESS_MACHINE)/vcs_lightpen.o  \
	$(MESS_MACHINE)/vcs_paddles.o   \
	$(MESS_MACHINE)/vcs_wheel.o \
	$(MESS_MACHINE)/vcsctrl.o   \

$(MESSOBJ)/bandai.a:            \
	$(MESS_DRIVERS)/wswan.o     \
	$(MESS_MACHINE)/wswan.o     \
	$(MESS_VIDEO)/wswan.o       \
	$(MESS_AUDIO)/wswan.o       \

$(MESSOBJ)/funtech.a:           \
	$(MESS_DRIVERS)/supracan.o  \

$(MESSOBJ)/nec.a:               \
	$(MESS_MACHINE)/pce.o       \
	$(MESS_DRIVERS)/pce.o       \

$(MESSOBJ)/nintendo.a:          \
	$(MESS_MACHINE)/nes_mmc.o   \
	$(MESS_MACHINE)/nes_slot.o  \
	$(MESS_VIDEO)/nes.o         \
	$(MESS_MACHINE)/nes.o       \
	$(MESS_DRIVERS)/nes.o       \
	$(MESS_MACHINE)/snescx4.o   \
	$(MESS_MACHINE)/sns_slot.o  \
	$(MESS_MACHINE)/sns_rom.o   \
	$(MESS_MACHINE)/sns_rom21.o \
	$(MESS_MACHINE)/sns_bsx.o   \
	$(MESS_MACHINE)/sns_sdd1.o  \
	$(MESS_MACHINE)/sns_sfx.o   \
	$(MESS_MACHINE)/sns_spc7110.o \
	$(MESS_MACHINE)/sns_sufami.o\
	$(MESS_MACHINE)/sns_upd.o   \
	$(MESS_MACHINE)/sns_event.o  \
	$(MESS_DRIVERS)/snes.o      \
	$(MESS_AUDIO)/gb.o          \
	$(MESS_VIDEO)/gb.o          \
	$(MESS_MACHINE)/gb.o        \
	$(MESS_MACHINE)/gb_slot.o   \
	$(MESS_MACHINE)/gb_rom.o    \
	$(MESS_MACHINE)/gb_mbc.o    \
	$(MESS_DRIVERS)/gb.o        \
	$(MESS_DRIVERS)/gba.o       \
	$(MESS_VIDEO)/gba.o         \

$(MESSOBJ)/sega.a:              \
	$(MESS_MACHINE)/md_slot.o   \
	$(MESS_MACHINE)/md_rom.o    \
	$(MESS_MACHINE)/md_sk.o     \
	$(MESS_MACHINE)/md_eeprom.o \
	$(MESS_MACHINE)/md_jcart.o  \
	$(MESS_MACHINE)/md_stm95.o  \
	$(MESS_MACHINE)/md_svp.o    \
	$(MESS_DRIVERS)/megadriv.o  \
	$(MESS_MACHINE)/sat_slot.o  \
	$(MESS_MACHINE)/sat_rom.o   \
	$(MESS_MACHINE)/sat_dram.o  \
	$(MESS_MACHINE)/sat_bram.o  \
	$(MESS_DRIVERS)/saturn.o    \
	$(MESS_MACHINE)/sms.o       \
	$(MESS_DRIVERS)/sms.o       \

$(MESSOBJ)/snk.a:               \
	$(MESS_DRIVERS)/ng_aes.o    \
	$(MAME_MACHINE)/neocrypt.o  \
	$(MAME_MACHINE)/neoprot.o   \
	$(MAME_MACHINE)/neoboot.o   \
	$(MAME_DRIVERS)/neogeo.o    \
	$(MESS_DRIVERS)/ngp.o       \
	$(MESS_VIDEO)/k1ge.o        \

$(MESSOBJ)/sony.a:              \
	$(MESS_DRIVERS)/psx.o       \
	$(MESS_MACHINE)/psxcport.o  \
	$(MESS_MACHINE)/psxcd.o     \
	$(MESS_MACHINE)/psxcard.o   \
	$(MESS_MACHINE)/psxanalog.o \
	$(MESS_DRIVERS)/pockstat.o  \




#-------------------------------------------------
# miscellaneous dependencies
#-------------------------------------------------

$(MESS_MACHINE)/snescx4.o: $(MESSSRC)/machine/cx4ops.c \
				$(MESSSRC)/machine/cx4oam.c \
				$(MESSSRC)/machine/cx4fn.c \
				$(MESSSRC)/machine/cx4data.c \

$(MESS_VIDEO)/gba.o:        $(MESSSRC)/video/gbamode0.c \
				$(MESSSRC)/video/gbamode1.c \
				$(MESSSRC)/video/gbamode2.c \
				$(MESSSRC)/video/gbam345.c

$(MESS_MACHINE)/nes_mmc.o:  $(MESSSRC)/machine/nes_ines.c \
				$(MESSSRC)/machine/nes_pcb.c \
				$(MESSSRC)/machine/nes_unif.c \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

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

