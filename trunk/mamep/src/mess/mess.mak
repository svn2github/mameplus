###########################################################################
#
#   mess.mak
#
#   MESS target makefile
#
###########################################################################



###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################



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
	$(MESSOBJ)/shared.a \

#-------------------------------------------------
# the following files are general components and
# shared across a number of drivers
#-------------------------------------------------

$(MESSOBJ)/shared.a: \
	$(MESS_FORMATS)/imd_dsk.o	\
	$(MESS_FORMATS)/td0_dsk.o	\
	$(MESS_FORMATS)/cqm_dsk.o	\
	$(MESS_FORMATS)/dsk_dsk.o	\
	$(MESS_FORMATS)/d88_dsk.o	\
	$(MESS_FORMATS)/fdi_dsk.o	\
	$(MESS_FORMATS)/basicdsk.o	\
	$(MESS_MACHINE)/ctronics.o	\
	$(MESS_MACHINE)/wd17xx.o	\
	$(MESS_FORMATS)/msx_dsk.o	\



#-------------------------------------------------
# manufacturer-specific groupings for drivers
#-------------------------------------------------

$(MESSOBJ)/ascii.a:				\
	$(MESS_FORMATS)/fmsx_cas.o	\
	$(MESS_DRIVERS)/msx.o		\
	$(MESS_MACHINE)/msx_slot.o	\
	$(MESS_MACHINE)/msx.o		\

$(MESSOBJ)/atari.a:				\
	$(MAME_VIDEO)/tia.o			\
	$(MAME_MACHINE)/atari.o		\
	$(MAME_VIDEO)/atari.o		\
	$(MAME_VIDEO)/antic.o		\
	$(MAME_VIDEO)/gtia.o		\
	$(MESS_MACHINE)/ataricrt.o	\
	$(MESS_MACHINE)/atarifdc.o	\
	$(MESS_DRIVERS)/atari.o		\
	$(MESS_MACHINE)/a7800.o		\
	$(MESS_DRIVERS)/a7800.o		\
	$(MESS_VIDEO)/a7800.o		\
	$(MESS_DRIVERS)/a2600.o		\
	$(MESS_FORMATS)/a26_cas.o	\

$(MESSOBJ)/bandai.a:			\
	$(MESS_DRIVERS)/wswan.o		\
	$(MESS_MACHINE)/wswan.o		\
	$(MESS_VIDEO)/wswan.o		\
	$(MESS_AUDIO)/wswan.o		\

$(MESSOBJ)/capcom.a:			\
	$(MESS_DRIVERS)/cpschngr.o	\
	$(MESS_VIDEO)/cpschngr.o	\
	$(MAME_MACHINE)/kabuki.o	\

$(MESSOBJ)/funtech.a:			\
	$(MESS_DRIVERS)/supracan.o	\

$(MESSOBJ)/nec.a:				\
	$(MAME_VIDEO)/vdc.o			\
	$(MESS_MACHINE)/pce.o		\
	$(MESS_DRIVERS)/pce.o		\

$(MESSOBJ)/nintendo.a:			\
	$(MESS_AUDIO)/gb.o			\
	$(MESS_VIDEO)/gb.o			\
	$(MESS_MACHINE)/gb.o		\
	$(MESS_DRIVERS)/gb.o		\
	$(MESS_MACHINE)/nes_mmc.o	\
	$(MAME_VIDEO)/ppu2c0x.o		\
	$(MESS_VIDEO)/nes.o			\
	$(MESS_FORMATS)/nes_dsk.o	\
	$(MESS_MACHINE)/nes.o		\
	$(MESS_DRIVERS)/nes.o		\
	$(MAME_AUDIO)/snes_snd.o	\
	$(MAME_MACHINE)/snes.o		\
	$(MAME_VIDEO)/snes.o		\
	$(MESS_MACHINE)/snescart.o	\
	$(MESS_DRIVERS)/snes.o		\
	$(MESS_DRIVERS)/gba.o		\
	$(MESS_VIDEO)/gba.o		\

$(MESSOBJ)/sega.a:				\
	$(MESS_VIDEO)/smsvdp.o		\
	$(MESS_MACHINE)/sms.o		\
	$(MESS_DRIVERS)/sms.o		\
	$(MAME_DRIVERS)/megadriv.o  \
	$(MAME_MACHINE)/megadriv.o  \
	$(MAME_MACHINE)/md_cart.o	\

$(MESSOBJ)/snk.a:				\
	$(MESS_DRIVERS)/ngp.o		\
	$(MESS_VIDEO)/k1ge.o		\




#-------------------------------------------------
# miscellaneous dependencies
#-------------------------------------------------

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


#-------------------------------------------------
# MESS special OSD rules
#-------------------------------------------------

include $(MESSSRC)/osd/$(OSD)/$(OSD).mak

#-------------------------------------------------
# mamep: driver list dependencies
#-------------------------------------------------

#FXIXME
$(MESSOBJ)/%.lst:	$(MESSSRC)/%.lst
	@echo Generating $@...
	@echo #include "$<" > $@.h
	$(CC) $(CDEFS) $(INCPATH) -I. -E $@.h -o $@

