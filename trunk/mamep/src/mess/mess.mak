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
	$(MESSOBJ)/capcom.a \
	$(MESSOBJ)/funtech.a \
	$(MESSOBJ)/nec.a \
	$(MESSOBJ)/nintendo.a \
	$(MESSOBJ)/sega.a \
	$(MESSOBJ)/snk.a \

ifneq ($(MAMEMESS),)
DRVLIBS += \
	$(MESSOBJ)/mame.a
endif

#-------------------------------------------------
# the following files are MAME components and
# shared across a number of drivers
#-------------------------------------------------

$(MESSOBJ)/mame.a: \
	$(MAME_VIDEO)/tia.o			\
	$(MAME_MACHINE)/atari.o		\
	$(MAME_VIDEO)/atari.o		\
	$(MAME_VIDEO)/antic.o		\
	$(MAME_DRIVERS)/cps1.o	\
	$(MAME_VIDEO)/cps1.o	\

#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MESSOBJ)/ascii.a:                     \
	$(MESS_DRIVERS)/msx.o		\
	$(MESS_MACHINE)/msx.o		\
	$(MESS_MACHINE)/msx_slot.o	\

$(MESSOBJ)/atari.a:				\
	$(MESS_MACHINE)/ataricrt.o	\
	$(MESS_MACHINE)/atarifdc.o	\
	$(MESS_DRIVERS)/atari400.o	\
	$(MESS_MACHINE)/a7800.o		\
	$(MESS_DRIVERS)/a7800.o		\
	$(MESS_VIDEO)/a7800.o		\
	$(MESS_DRIVERS)/a2600.o		\
	$(MESS_MACHINE)/vcsctrl.o	\
	$(MESS_MACHINE)/vcs_joy.o	\
	$(MESS_MACHINE)/vcs_lightpen.o	\
	$(MESS_MACHINE)/vcs_paddles.o	\
	$(MESS_MACHINE)/vcs_joybooster.o	\
	$(MESS_MACHINE)/vcs_wheel.o	\
	$(MESS_MACHINE)/vcs_keypad.o	\

$(MESSOBJ)/bandai.a:			\
	$(MESS_DRIVERS)/wswan.o		\
	$(MESS_MACHINE)/wswan.o		\
	$(MESS_VIDEO)/wswan.o		\
	$(MESS_AUDIO)/wswan.o		\

$(MESSOBJ)/funtech.a:			\
	$(MESS_DRIVERS)/supracan.o	\

$(MESSOBJ)/nec.a:				\
	$(MESS_MACHINE)/pce.o		\
	$(MESS_DRIVERS)/pce.o		\

$(MESSOBJ)/nintendo.a:			\
	$(MESS_MACHINE)/nes_mmc.o	\
	$(MESS_VIDEO)/nes.o			\
	$(MESS_MACHINE)/nes.o		\
	$(MESS_DRIVERS)/nes.o		\
	$(MESS_MACHINE)/snescart.o	\
	$(MESS_DRIVERS)/snes.o		\
	$(MESS_AUDIO)/gb.o			\
	$(MESS_VIDEO)/gb.o			\
	$(MESS_MACHINE)/gb.o		\
	$(MESS_DRIVERS)/gb.o		\
	$(MESS_DRIVERS)/gba.o		\
	$(MESS_VIDEO)/gba.o		\

$(MESSOBJ)/sega.a:				\
	$(MAME_MACHINE)/md_cart.o	\
	$(MESS_DRIVERS)/megadriv.o  \
	$(MESS_MACHINE)/sms.o	\
	$(MESS_DRIVERS)/sms.o	\

$(MESSOBJ)/snk.a:				\
	$(MESS_DRIVERS)/ngp.o		\
	$(MESS_VIDEO)/k1ge.o		\





#-------------------------------------------------
# miscellaneous dependencies
#-------------------------------------------------

$(MAME_MACHINE)/snes.o: $(MAMESRC)/machine/snesobc1.c \
				$(MAMESRC)/machine/snescx4.c \
				$(MAMESRC)/machine/cx4ops.c \
				$(MAMESRC)/machine/cx4oam.c \
				$(MAMESRC)/machine/cx4fn.c \
				$(MAMESRC)/machine/cx4data.c \
				$(MAMESRC)/machine/snesrtc.c \
				$(MAMESRC)/machine/snessdd1.c \
				$(MAMESRC)/machine/snes7110.c \
				$(MAMESRC)/machine/snesbsx.c

$(MESS_VIDEO)/gba.o:		$(MESSSRC)/video/gbamode0.c \
				$(MESSSRC)/video/gbamode1.c \
				$(MESSSRC)/video/gbamode2.c \
				$(MESSSRC)/video/gbam345.c

$(MESS_MACHINE)/nes_mmc.o:	$(MESSSRC)/machine/nes_ines.c \
				$(MESSSRC)/machine/nes_pcb.c \
				$(MESSSRC)/machine/nes_unif.c \

#-------------------------------------------------
# layout dependencies
#-------------------------------------------------

$(MESS_DRIVERS)/sms.o:		$(MESS_LAYOUT)/sms1.lh
$(MESS_DRIVERS)/wswan.o:	$(MESS_LAYOUT)/wswan.lh


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

