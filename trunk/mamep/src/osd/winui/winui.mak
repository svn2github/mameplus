###########################################################################
#
#   winui.mak
#
#   winui (MameUI) makefile
#
#   Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------

# uncomment next line to enable a build using Microsoft tools
# MSVC_BUILD = 1

# uncomment next line to use cygwin compiler
# CYGWIN_BUILD = 1

# uncomment next line to enable multi-monitor stubs on Windows 95/NT
# you will need to find multimon.h and put it into your include
# path in order to make this work
# WIN95_MULTIMON = 1

# uncomment next line to enable a Unicode build
# UNICODE = 1



###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# append "ui" to the emulator name 
#-------------------------------------------------


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

RC = @windres --use-temp-file

RCDEFS = -DNDEBUG -D_WIN32_IE=0x0400

#-------------------------------------------------
# overrides for the CYGWIN compiler
#-------------------------------------------------



#-------------------------------------------------
# overrides for the MSVC compiler
#-------------------------------------------------


# turn on link-time codegen if the MAXOPT flag is also set
ifdef MAXOPT
CC += /GL
LD += /LTCG
endif

ifdef PTR64
CC += /wd4267
endif

# explicitly set the entry point for UNICODE builds
ifdef UNICODE
LD += /ENTRY:wmainCRTStartup
endif

# add some VC++-specific defines
DEFS += -D_CRT_SECURE_NO_DEPRECATE -DXML_STATIC -D__inline__=__inline -Dsnprintf=_snprintf

DEFS += \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500

# make msvcprep into a pre-build step
# OSPREBUILD = $(VCONV)

# add VCONV to the build tools
BUILD += $(VCONV)





#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# define the x64 ABI to be Windows
DEFS += -DX64_WINDOWS_ABI

# debug build: enable guard pages on all memory allocations
ifdef DEBUG
#DEFS += -DMALLOC_DEBUG
LDFLAGS += -Wl,--allow-multiple-definition
endif

ifdef UNICODE
DEFS += -DUNICODE -D_UNICODE
endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

ifdef WIN95_MULTIMON
CFLAGS += -DWIN95_MULTIMON
endif

# add the windows libaries, 3 additional libs at the end for UI
LIBS += \
	-lddraw \
	-ldinput \
	-ldxguid \
	-ladvapi32 \
	-lcomctl32 \
	-lshlwapi \

# add -mwindows for UI
LDFLAGSEMULATOR += \
	-mwindows \
	-lkernel32 \
	-lshell32 \
	-lcomdlg32 \

ifeq ($(MSVC_BUILD),)
    ifneq ($(USE_IMAGE_MENU),)
    LIBS += \
		-lmsimg32 \
		-lstdc++
    endif
else
    LIBS += -lhtmlhelp
endif

ifdef PTR64
LIBS += -lbufferoverflowu
endif



#-------------------------------------------------
# OSD core library
#-------------------------------------------------
# still not sure what to do about main.

OSDCOREOBJS = \
	$(WINOBJ)/strconv.o	\
	$(WINOBJ)/windir.o \
	$(WINOBJ)/winfile.o \
	$(WINOBJ)/winmisc.o \
	$(WINOBJ)/winsync.o \
	$(WINOBJ)/wintime.o \
	$(WINOBJ)/winutf8.o \
	$(WINOBJ)/winutil.o \
	$(WINOBJ)/winwork.o \

# if malloc debugging is enabled, include the necessary code
ifneq ($(findstring MALLOC_DEBUG,$(DEFS)),)
OSDCOREOBJS += \
	$(WINOBJ)/winalloc.o
endif

$(LIBOCORE): $(OSDCOREOBJS)



#-------------------------------------------------
# OSD Windows library
#-------------------------------------------------


# add UI objs
OSDOBJS += \
	$(UIOBJ)/mui_util.o \
	$(UIOBJ)/directinput.o \
	$(UIOBJ)/dijoystick.o \
	$(UIOBJ)/directdraw.o \
	$(UIOBJ)/directories.o \
	$(UIOBJ)/mui_audit.o \
	$(UIOBJ)/columnedit.o \
	$(UIOBJ)/screenshot.o \
	$(UIOBJ)/treeview.o \
	$(UIOBJ)/splitters.o \
	$(UIOBJ)/bitmask.o \
	$(UIOBJ)/datamap.o \
	$(UIOBJ)/dxdecode.o \
	$(UIOBJ)/picker.o \
	$(UIOBJ)/properties.o \
	$(UIOBJ)/tabview.o \
	$(UIOBJ)/help.o \
	$(UIOBJ)/history.o \
	$(UIOBJ)/dialogs.o \
	$(UIOBJ)/mui_opts.o \
	$(UIOBJ)/layout.o \
	$(UIOBJ)/winui.o \
	$(UIOBJ)/helpids.o \
	$(UIOBJ)/translate.o \


# extra dependencies
$(WINOBJ)/drawdd.o : 	$(SRC)/emu/rendersw.c
$(WINOBJ)/drawgdi.o :	$(SRC)/emu/rendersw.c

# add debug-specific files

OSDOBJS += \
	$(WINOBJ)/debugwin.o

$(WINOBJ)/winmain.o : $(WINSRC)/winmain.c
	@echo Compiling $<...
	$(CC) $(CDEFS) -Dmain=utf8_main $(CFLAGS) -c $< -o $@

# add our UI resources
ifeq ($(NO_DLL),)
    RESFILE += $(UIOBJ)/mameui.res
else
    UI_RCFLAGS += --include-dir $(MESS_WINSRC)
    $(UIOBJ)/mameui.res: $(MESS_WINSRC)/mess.rc $(WINUISRC)/mameui.rc $(UIOBJ)/mamevers.rc
    RESFILE +=  $(UIOBJ)/mameui.res
endif

# The : is important! It prevents the dependency above from including mui_main.o in its target!


#-------------------------------------------------
# rule for making the ledutil sample
#
# Don't build for an MSVC_BUILD
#-------------------------------------------------

ledutil$(EXE): $(WINOBJ)/ledutil.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

TOOLS += ledutil$(EXE)



#-------------------------------------------------
# rules for creating helpids.c 
#-------------------------------------------------

$(UISRC)/helpids.c : $(UIOBJ)/mkhelp$(EXE) $(UISRC)/resource.h $(UISRC)/resource.hm $(UISRC)/mameui.rc
	$(UIOBJ)/mkhelp$(EXE) $(UISRC)/mameui.rc >$@

# rule to build the generator
$(UIOBJ)/mkhelp$(EXE): $(UIOBJ)/mkhelp.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# rule for making the verinfo tool
#-------------------------------------------------

#VERINFO = $(UIOBJ)/verinfo$(EXE)

#$(VERINFO): $(UIOBJ)/verinfo.o $(LIBOCORE)
#	@echo Linking $@...
#	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

#BUILD += $(VERINFO)

#####  End windui.mak ##############################################




WINUISRC = $(SRC)/osd/winui
UIOBJ = $(OBJ)/osd/winui

OBJDIRS += $(UIOBJ)

MESS_WINSRC = $(SRC)/mess/osd/windows
MESS_WINOBJ = $(OBJ)/mess/osd/windows
MESS_WINUISRC = $(SRC)/mess/osd/winui
MESS_WINUIOBJ = $(OBJ)/mess/osd/winui
OBJDIRS += $(MESS_WINUIOBJ)
CFLAGS += -I$(WINUISRC) -I$(MESS_WINUISRC)

ifneq ($(USE_IMAGE_MENU),)
    $(UIOBJ)/%.o: $(WINUISRC)/%.cpp
	    @echo Compiling $<...
endif

#-------------------------------------------------
# Windows UI object files
#-------------------------------------------------


ifdef MAMEMESS
    OSDOBJS += $(MESS_WINUIOBJ)/optionsms.o
endif
ifneq ($(USE_UI_COLOR_PALETTE),)
    OSDOBJS += $(UIOBJ)/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
    OSDOBJS += $(UIOBJ)/imagemenu.o
endif

$(LIBOSD): $(OSDOBJS)

$(UIOBJ)/mameui.res: $(WINUISRC)/mameui.rc $(UIOBJ)/mamevers.rc

$(UIOBJ)/winuiopt.o: $(WINUISRC)/optdef.h $(WINUISRC)/opthndlr.h $(WINUISRC)/opthndlr.c


#####################################################################
# Resources

VERINFO32 = $(UIOBJ)/verinfo32$(EXE)

$(UIOBJ)/verinfo32.o: $(SRC)/build/verinfo.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DWINUI=1 $(CFLAGS) -c $< -o $@

$(VERINFO32): $(UIOBJ)/verinfo32.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

BUILD += $(VERINFO32)



UI_RCFLAGS = -O coff --include-dir $(WINUISRC) --include-dir $(UIOBJ) --include-dir $(MESS_WINUISRC)

$(UIOBJ)/%.res: $(WINUISRC)/%.rc
	@echo Compiling mameui resources $<...
	$(RC) $(RCDEFS) $(UI_RCFLAGS) -o $@ -i $<

$(UIOBJ)/mamevers.rc: $(VERINFO32) $(SRC)/version.c
	@echo Emitting $@...
	@$(VERINFO32) $(SRC)/version.c > $@


#####################################################################
# Mame CORE with Windows UI options

ifneq ($(USE_UI_COLOR_DISPLAY),)
RCDEFS += -DUI_COLOR_DISPLAY
endif

ifneq ($(USE_UI_COLOR_PALETTE),)
RCDEFS += -DUI_COLOR_PALETTE
endif

ifneq ($(USE_IPS),)
RCDEFS += -DUSE_IPS
endif

ifneq ($(USE_AUTO_PAUSE_PLAYBACK),)
RCDEFS += -DAUTO_PAUSE_PLAYBACK
endif

ifneq ($(USE_SCALE_EFFECTS),)
RCDEFS += -DUSE_SCALE_EFFECTS
endif

ifneq ($(USE_TRANS_UI),)
RCDEFS += -DTRANS_UI
endif

ifneq ($(USE_STORY_DATAFILE),)
RCDEFS += -DSTORY_DATAFILE
endif

ifneq ($(USE_JOYSTICK_ID),)
RCDEFS += -DJOYSTICK_ID
endif

ifneq ($(USE_JOY_MOUSE_MOVE),)
RCDEFS += -DUSE_JOY_MOUSE_MOVE
endif

ifneq ($(USE_VOLUME_AUTO_ADJUST),)
RCDEFS += -DUSE_VOLUME_AUTO_ADJUST
endif

ifneq ($(USE_DRIVER_SWITCH),)
RCDEFS += -DDRIVER_SWITCH
endif

ifneq ($(NEOCPSMAME),)
RCDEFS += -DNEOCPSMAME
endif

#####################################################################
# Windows UI specific options

ifneq ($(USE_MISC_FOLDER),)
DEFS += -DMISC_FOLDER
RCDEFS += -DMISC_FOLDER
endif

ifneq ($(USE_SHOW_SPLASH_SCREEN),)
DEFS += -DUSE_SHOW_SPLASH_SCREEN
RCDEFS += -DUSE_SHOW_SPLASH_SCREEN
endif

ifneq ($(USE_VIEW_PCBINFO),)
DEFS += -DUSE_VIEW_PCBINFO
RCDEFS += -DUSE_VIEW_PCBINFO
endif

ifneq ($(USE_IMAGE_MENU),)
DEFS += -DIMAGE_MENU
RCDEFS += -DIMAGE_MENU
endif

ifneq ($(USE_TREE_SHEET),)
DEFS += -DTREE_SHEET
RCDEFS += -DTREE_SHEET
endif
