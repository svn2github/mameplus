###########################################################################
#
#   scale.mak
#
#   scale effects makefile
#
#   This is an unofficial version based on MAME.
#   Please do not send any reports from this build to the MAME team.
#
###########################################################################


SCALEOBJ = $(WINOBJ)/scale

OBJDIRS += $(SCALEOBJ)



#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------

# scale-specific defines
DEFS += -Drestrict=__restrict

# define the x64 MMX
ifndef PTR64
DEFS += -DUSE_MMX_INTERP_SCALE
endif



#-------------------------------------------------
# scale effects framework code
#-------------------------------------------------

OSDOBJS += $(SCALEOBJ)/scale.o



#-------------------------------------------------
# scale effects objects
#-------------------------------------------------

OSDOBJS += \
	$(SCALEOBJ)/2xpm.o \
	$(SCALEOBJ)/2xsai.o \
	$(SCALEOBJ)/hq2x.o \
	$(SCALEOBJ)/hq3x.o \
	$(SCALEOBJ)/scale2x.o \
	$(SCALEOBJ)/scale3x.o \
	$(SCALEOBJ)/scanline.o \
	$(SCALEOBJ)/snes9x_render.o \
	$(SCALEOBJ)/vba_hq2x.o \


