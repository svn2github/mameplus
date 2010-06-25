###########################################################################
#
#   scale.mak
#
#   scale effects makefile
#
#   Scale Effects is an unofficial version based on MAME.
#   Please do not send any reports from this build to the MAME team.
#
###########################################################################


SCALEOBJ = $(WINOBJ)/scale

OBJDIRS += $(SCALEOBJ)



#-------------------------------------------------
# scale effects objects
#-------------------------------------------------

OSDOBJS += \
	$(SCALEOBJ)/scale.o \
	$(SCALEOBJ)/2xpm.o \
	$(SCALEOBJ)/2xsai.o \
	$(SCALEOBJ)/hq2x.o \
	$(SCALEOBJ)/hq3x.o \
	$(SCALEOBJ)/scale2x.o \
	$(SCALEOBJ)/scale3x.o \
	$(SCALEOBJ)/scanline.o \
	$(SCALEOBJ)/snes9x_render.o \
	$(SCALEOBJ)/vba_hq2x.o \

ifndef PTR64
DEFS += -DUSE_MMX_INTERP_SCALE
endif


