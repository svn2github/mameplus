#####################################################################
# make SUFFIX=32

# remove main.o from OSDCOREOBJS
ifneq ($(NO_DLL),)
OSDCOREOBJS := $(OSDCOREOBJS:$(OBJ)/osd/$(MAMEOS)/osdmain.o=)
endif

# use CFLAGSOSDEPEND
$(OBJ)/osd/ui/%.o: src/osd/ui/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@

ifneq ($(USE_IMAGE_MENU),)
$(OBJ)/osd/ui/%.o: src/osd/ui/%.cpp
	@echo Compiling $<...
ifneq ($(MSVC_BUILD),)
	$(CC) -mwindows -c $< -o $@
else
	@g++ -mwindows -c $< -o $@
endif
endif

OBJDIRS += $(OBJ)/osd/ui

# only OS specific output files and rules
TMPOBJS = \
	$(OBJ)/osd/ui/m32util.o \
	$(OBJ)/osd/ui/directinput.o \
	$(OBJ)/osd/ui/dijoystick.o \
	$(OBJ)/osd/ui/directdraw.o \
	$(OBJ)/osd/ui/directories.o \
	$(OBJ)/osd/ui/audit32.o \
	$(OBJ)/osd/ui/columnedit.o \
	$(OBJ)/osd/ui/screenshot.o \
	$(OBJ)/osd/ui/treeview.o \
	$(OBJ)/osd/ui/splitters.o \
	$(OBJ)/osd/ui/bitmask.o \
	$(OBJ)/osd/ui/datamap.o \
	$(OBJ)/osd/ui/dxdecode.o \
	$(OBJ)/osd/ui/picker.o \
	$(OBJ)/osd/ui/properties.o \
	$(OBJ)/osd/ui/tabview.o \
	$(OBJ)/osd/ui/help.o \
	$(OBJ)/osd/ui/history.o \
	$(OBJ)/osd/ui/dialogs.o \
	$(OBJ)/osd/ui/win32ui.o \
	$(OBJ)/osd/ui/options.o \
	$(OBJ)/osd/ui/layout.o \
	$(OBJ)/osd/ui/translate.o

ifneq ($(USE_UI_COLOR_PALETTE),)
    TMPOBJS += $(OBJ)/osd/ui/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
    TMPOBJS += $(OBJ)/osd/ui/imagemenu.o
endif

$(OBJ)/osd/ui/ui.a: $(TMPOBJS)

ifeq ($(MSVC_BUILD),)
    GUIOBJS += $(OBJ)/osd/ui/m32main.o $(OBJ)/ui/ui.a

    # add resource file
    GUIOBJS += $(OBJ)/osd/ui/mame32.res
else
    OSOBJS += $(OBJ)/osd/ui/ui.a

    ifeq ($(NO_DLL),)
        GUIOBJS += $(OBJ)/osd/ui/m32main.o
        OSOBJS += $(OBJ)/osd/ui/win32ui.o

        # add resource file
        GUIOBJS += $(OBJ)/osd/ui/mame32.res
    else
        OSOBJS += $(OBJ)/osd/ui/m32main.o

        # add resource file
        OSOBJS += $(OBJ)/osd/ui/mame32.res
    endif
endif



#####################################################################
# compiler

#
# Preprocessor Definitions
#

DEFS += \
	-DWINUI=1 \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-D_WIN32_WINNT=0x0400



#####################################################################
# Resources

RCFLAGS += --include-dir $(SRC)/osd/ui



#####################################################################
# Linker

ifeq ($(MSVC_BUILD),)
    GUILIBS += \
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
    ifneq ($(USE_IMAGE_MENU),)
        GUILIBS += \
		    -lmsimg32 \
		    -lstdc++
    endif
else
    TMPLIBS = \
		-lkernel32 \
		-lshell32 \
		-lshlwapi \
		-lcomctl32 \
		-lcomdlg32 \
		-ladvapi32 \
		-lddraw \
		-ldinput \
		-ldxguid \
		-lhtmlhelp \
		-lunicows

    ifneq ($(NO_DLL),)
        GUILIBS = $(TMPLIBS)
    else
        LIBS += $(TMPLIBS)
    endif
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
