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


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

# add ui specific src/objs
UISRC = $(SRC)/osd/winui
UIOBJ = $(OBJ)/osd/winui

OBJDIRS += $(UIOBJ)

CFLAGS += -I $(UISRC)

#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

UI_RC = @windres --use-temp-file

UI_RCDEFS = -DNDEBUG -D_WIN32_IE=0x0501

# include UISRC direcotry
UI_RCFLAGS = -O coff --include-dir $(UISRC) --include-dir $(UIOBJ)



#-------------------------------------------------
# preprocessor definitions
#-------------------------------------------------

DEFS += \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0501



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

LIBS += \
	-lkernel32 \
	-lshell32 \
	-lshlwapi \
	-lcomctl32 \
	-lcomdlg32 \
	-ladvapi32 \
	-lddraw \
	-ldinput \
	-ldxguid
ifeq ($(MSVC_BUILD),)
	ifneq ($(USE_IMAGE_MENU),)
	LIBS += \
		-lmsimg32 \
		-lstdc++
	endif
else
	LIBS += -lhtmlhelp
endif



#-------------------------------------------------
# OSD Windows library
#-------------------------------------------------

# add UI objs
UIOBJS += \
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
	$(UIOBJ)/datafile.o \
	$(UIOBJ)/dirwatch.o \
	$(UIOBJ)/winui.o \
	$(UIOBJ)/helpids.o \
	$(UIOBJ)/translate.o \


ifneq ($(USE_CUSTOM_UI_COLOR),)
UIOBJS += $(UIOBJ)/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
UIOBJS += $(UIOBJ)/imagemenu.o
endif

# add our UI resources
GUIRESFILE = $(UIOBJ)/mameui.res

$(LIBOSD): $(UIOBJS)




#-------------------------------------------------
# rules for creating helpids.c 
#-------------------------------------------------

$(UISRC)/helpids.c : $(UIOBJ)/mkhelp$(EXE) $(UISRC)/resource.h $(UISRC)/resource.hm $(UISRC)/mameui.rc
	@"$(UIOBJ)/mkhelp$(EXE)" $(UISRC)/mameui.rc >$@

# rule to build the generator
$(UIOBJ)/mkhelp$(EXE): $(UIOBJ)/mkhelp.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# rule for making the verinfo tool
#-------------------------------------------------

#VERINFO = $(UIOBJ)/verinfo$(EXE)
#
#$(VERINFO): $(UIOBJ)/verinfo.o $(LIBOCORE)
#	@echo Linking $@...
#	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
#
#BUILD += $(VERINFO)



#-------------------------------------------------
# Specific rele to compile verinfo util.
#-------------------------------------------------

#$(UIOBJ)/verinfo.o: $(SRC)/build/verinfo.c | $(OSPREBUILD)
#	@echo Compiling $<...
#	$(CC) $(CDEFS) -DWINUI=1 $(CFLAGS) -c $< -o $@



#-------------------------------------------------
# generic rule for the resource compiler for UI
#-------------------------------------------------

$(GUIRESFILE): $(UISRC)/mameui.rc $(UIOBJ)/mamevers.rc
	@echo Compiling mameui resources $<...
	$(UI_RC) $(UI_RCDEFS) $(UI_RCFLAGS) -o $@ -i $<



#-------------------------------------------------
# rules for resource file
#-------------------------------------------------

$(UIOBJ)/mamevers.rc: $(OBJ)/build/verinfo$(EXE) $(SRC)/version.c
	@echo Emitting $@...
	@"$(OBJ)/build/verinfo$(EXE)" $(SRC)/version.c > $@



#-------------------------------------------------
# Specific rele to compile USE_IMAGE_MENU.
#-------------------------------------------------

ifneq ($(USE_IMAGE_MENU),)
    $(UIOBJ)/%.o: $(UISRC)/%.cpp
	    @echo Compiling $<...
    ifneq ($(MSVC_BUILD),)
	    $(CC) -mwindows -c $< -o $@
    else
	    @g++ -mwindows -c $< -o $@
    endif
endif



#-------------------------------------------------
# CORE functions
# only definitions UI_RCDEFS for mameui.rc
#-------------------------------------------------

ifneq ($(USE_CUSTOM_UI_COLOR),)
UI_RCDEFS += -DUI_COLOR_PALETTE
endif

ifneq ($(USE_AUTO_PAUSE_PLAYBACK),)
UI_RCDEFS += -DAUTO_PAUSE_PLAYBACK
endif

ifneq ($(USE_SCALE_EFFECTS),)
UI_RCDEFS += -DUSE_SCALE_EFFECTS
endif

ifneq ($(USE_TRANS_UI),)
UI_RCDEFS += -DTRANS_UI
endif

ifneq ($(USE_JOYSTICK_ID),)
UI_RCDEFS += -DJOYSTICK_ID
endif

ifneq ($(USE_VOLUME_AUTO_ADJUST),)
UI_RCDEFS += -DUSE_VOLUME_AUTO_ADJUST
endif

ifneq ($(USE_DRIVER_SWITCH),)
UI_RCDEFS += -DDRIVER_SWITCH
endif



#-------------------------------------------------
# MAMEUI-specific functions
#-------------------------------------------------

ifneq ($(USE_MISC_FOLDER),)
DEFS += -DMISC_FOLDER
UI_RCDEFS += -DMISC_FOLDER
endif

ifneq ($(USE_SHOW_SPLASH_SCREEN),)
DEFS += -DUSE_SHOW_SPLASH_SCREEN
UI_RCDEFS += -DUSE_SHOW_SPLASH_SCREEN
endif

ifneq ($(USE_STORY_DATAFILE),)
DEFS += -DSTORY_DATAFILE
UI_RCDEFS += -DSTORY_DATAFILE
endif

ifneq ($(USE_VIEW_PCBINFO),)
DEFS += -DUSE_VIEW_PCBINFO
UI_RCDEFS += -DUSE_VIEW_PCBINFO
endif

ifneq ($(USE_IMAGE_MENU),)
DEFS += -DIMAGE_MENU
UI_RCDEFS += -DIMAGE_MENU
endif

ifneq ($(USE_TREE_SHEET),)
DEFS += -DTREE_SHEET
UI_RCDEFS += -DTREE_SHEET
endif

ifneq ($(SHOW_UNAVAILABLE_FOLDER),)
DEFS += -DSHOW_UNAVAILABLE_FOLDER
endif




#####  End windui.mak ##############################################

