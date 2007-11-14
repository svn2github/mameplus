#####################################################################
# make SUFFIX=32
#####################################################################

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

WINUISRC = $(SRC)/osd/winui
WINUIOBJ = $(OBJ)/osd/winui

OBJDIRS += $(WINUIOBJ)

ifneq ($(USE_IMAGE_MENU),)
    $(WINUIOBJ)/%.o: $(WINUISRC)/%.cpp
	    @echo Compiling $<...
    ifneq ($(MSVC_BUILD),)
	    $(CC) -mwindows -c $< -o $@
    else
	    @g++ -mwindows -c $< -o $@
    endif
endif

#-------------------------------------------------
# Windows UI object files
#-------------------------------------------------

WINUIOBJS += \
	$(WINUIOBJ)/m32util.o \
	$(WINUIOBJ)/directinput.o \
	$(WINUIOBJ)/dijoystick.o \
	$(WINUIOBJ)/directdraw.o \
	$(WINUIOBJ)/directories.o \
	$(WINUIOBJ)/audit32.o \
	$(WINUIOBJ)/columnedit.o \
	$(WINUIOBJ)/screenshot.o \
	$(WINUIOBJ)/treeview.o \
	$(WINUIOBJ)/splitters.o \
	$(WINUIOBJ)/bitmask.o \
	$(WINUIOBJ)/datamap.o \
	$(WINUIOBJ)/dxdecode.o \
	$(WINUIOBJ)/picker.o \
	$(WINUIOBJ)/properties.o \
	$(WINUIOBJ)/tabview.o \
	$(WINUIOBJ)/help.o \
	$(WINUIOBJ)/history.o \
	$(WINUIOBJ)/dialogs.o \
	$(WINUIOBJ)/win32ui.o \
	$(WINUIOBJ)/winuiopt.o \
	$(WINUIOBJ)/layout.o \
	$(WINUIOBJ)/translate.o

ifneq ($(USE_UI_COLOR_PALETTE),)
    WINUIOBJS += $(WINUIOBJ)/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
    WINUIOBJS += $(WINUIOBJ)/imagemenu.o
endif

$(LIBOSD): $(WINUIOBJS)

GUIRESFILE = $(WINUIOBJ)/mame32.res

$(WINUIOBJ)/mame32.res: $(WINUISRC)/mame32.rc

$(WINUIOBJ)/winuiopt.o: $(WINUISRC)/optdef.h $(WINUISRC)/opthndlr.c


#####################################################################
# compiler

#
# Preprocessor Definitions
#

DEFS += \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-D_WIN32_WINNT=0x0400



#####################################################################
# Resources

UI_RC = @windres --use-temp-file

UI_RCDEFS = -DNDEBUG -D_WIN32_IE=0x0400

UI_RCFLAGS = -O coff --include-dir $(WINUISRC)

$(WINUIOBJ)/%.res: $(WINUISRC)/%.rc
	@echo Compiling mame32 resources $<...
	$(UI_RC) $(UI_RCDEFS) $(UI_RCFLAGS) -o $@ -i $<

#####################################################################
# Linker

    LIBS += \
		-lkernel32 \
		-lshell32 \
		-lshlwapi \
		-lcomctl32 \
		-lcomdlg32 \
		-ladvapi32 \
		-lddraw \
		-ldinput \
		-ldxguid \
		-lunicows
ifeq ($(MSVC_BUILD),)
    ifneq ($(USE_IMAGE_MENU),)
    LIBS += \
		-lmsimg32 \
		-lstdc++
    endif
else
    LIBS += -lhtmlhelp
endif

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

ifneq ($(HAZEMD),)
RCDEFS += -DHAZEMD
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
