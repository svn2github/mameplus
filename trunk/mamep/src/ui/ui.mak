#####################################################################
# make SUFFIX=32

# use CFLAGSOSDEPEND
$(OBJ)/ui/%.o: src/ui/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@

ifneq ($(USE_IMAGE_MENU),)
$(OBJ)/ui/%.o: src/ui/%.cpp
	@echo Compiling $<...
ifneq ($(MSVC_BUILD),)
	$(CC) -mwindows -c $< -o $@
else
	@g++ -mwindows -c $< -o $@
endif
endif

OBJDIRS += $(OBJ)/ui

# only OS specific output files and rules
TMPOBJS = \
	$(OBJ)/ui/m32util.o \
	$(OBJ)/ui/directinput.o \
	$(OBJ)/ui/dijoystick.o \
	$(OBJ)/ui/directdraw.o \
	$(OBJ)/ui/directories.o \
	$(OBJ)/ui/audit32.o \
	$(OBJ)/ui/columnedit.o \
	$(OBJ)/ui/screenshot.o \
	$(OBJ)/ui/treeview.o \
	$(OBJ)/ui/splitters.o \
	$(OBJ)/ui/bitmask.o \
	$(OBJ)/ui/datamap.o \
	$(OBJ)/ui/dxdecode.o \
	$(OBJ)/ui/picker.o \
	$(OBJ)/ui/properties.o \
	$(OBJ)/ui/tabview.o \
	$(OBJ)/ui/help.o \
	$(OBJ)/ui/history.o \
	$(OBJ)/ui/dialogs.o \
	$(OBJ)/ui/win32ui.o \
	$(OBJ)/ui/options.o \
	$(OBJ)/ui/layout.o \
	$(OBJ)/ui/translate.o

ifneq ($(USE_UI_COLOR_PALETTE),)
    TMPOBJS += $(OBJ)/ui/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
    TMPOBJS += $(OBJ)/ui/imagemenu.o
endif

$(OBJ)/ui/ui.a: $(TMPOBJS)

ifeq ($(MSVC_BUILD),)
    GUIOBJS += $(OBJ)/ui/m32main.o $(OBJ)/ui/ui.a

    # add resource file
    GUIOBJS += $(OBJ)/ui/mame32.res
else
    OSOBJS += $(OBJ)/ui/ui.a

    ifeq ($(NO_DLL),)
        GUIOBJS += $(OBJ)/ui/m32main.o
        OSOBJS += $(OBJ)/ui/win32ui.o

        # add resource file
        GUIOBJS += $(OBJ)/ui/mame32.res
    else
        OSOBJS += $(OBJ)/ui/m32main.o

        # add resource file
        OSOBJS += $(OBJ)/ui/mame32.res
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

RCFLAGS += --include-dir src/ui



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
