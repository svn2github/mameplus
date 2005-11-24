#####################################################################
# make SUFFIX=32

ifdef USE_GCC
# use CFLAGSOSDEPEND
$(OBJ)/ui/%.o: src/ui/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@
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
	$(OBJ)/ui/win32ui.o \
	$(OBJ)/ui/picker.o \
	$(OBJ)/ui/properties.o \
        $(OBJ)/ui/tabview.o \
	$(OBJ)/ui/options.o \
	$(OBJ)/ui/help.o \
	$(OBJ)/ui/layout.o \
	$(OBJ)/ui/history.o \
	$(OBJ)/ui/dialogs.o \
	$(OBJ)/ui/translate.o

ifneq ($(USE_UI_COLOR_DISPLAY),)
    TMPOBJS += $(OBJ)/ui/paletteedit.o
endif

$(OBJ)/ui/ui.a: $(TMPOBJS)

ifdef USE_GCC
    GUIOBJS += $(OBJ)/ui/m32main.o $(OBJ)/ui/ui.a

    # add resource file
    GUIOBJS += $(OBJ)/ui/mame32.res
else
    OSOBJS += $(OBJ)/ui/ui.a

    ifeq ($(NO_DLL),)
        GUIOBJS += $(OBJ)/ui/m32main.o

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
	-DMAME32NAME=APPNAME\"32\" \
	-DTEXT_MAME32NAME=TEXT\(APPNAME\)TEXT\(\"32\"\) \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-D_WIN32_WINNT=0x0400 \
	-UWINNT \
	-DPATH_SEPARATOR=\'/\'

ifeq ($(USE_GCC),)
    DEFS += -DLVS_EX_LABELTIP=0x00004000
endif



#####################################################################
# Resources

ifdef USE_GCC
    RCFLAGS += --include-dir src/ui
else
    RCFLAGS += -Isrc/ui
endif


#####################################################################
# Linker

ifdef USE_GCC
    GUILIBS += \
		-lkernel32 \
		-lshell32 \
		-lcomctl32 \
		-lcomdlg32 \
		-ladvapi32 \
		-lddraw \
		-ldinput \
		-ldxguid \
		-lhtmlhelp \
		-lunicows
else
    TMPLIBS = \
		kernel32.lib \
		shell32.lib \
		comctl32.lib \
		comdlg32.lib \
		advapi32.lib \
		ddraw.lib \
		dinput.lib \
		dxguid.lib \
		htmlhelp.lib \
		unicows.lib

    ifneq ($(NO_DLL),)
        GUILIBS = $(TMPLIBS)
    else
        LIBS += $(TMPLIBS)
    endif
endif

#####################################################################

ifneq ($(USE_MISC_FOLDER),)
DEFS += -DMISC_FOLDER
endif

ifneq ($(USE_UI_COLOR_DISPLAY),)
RCDEFS += -DUI_COLOR_DISPLAY
endif

ifneq ($(ROM_PATCH),)
RCDEFS += -DROM_PATCH
endif

ifneq ($(USE_SCALE_EFFECTS),0)
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

# Support Stick-type Pointing Device by miko2u@hotmail.com
ifneq ($(USE_JOY_MOUSE_MOVE),)
RCDEFS += -DUSE_JOY_MOUSE_MOVE
endif

ifneq ($(USE_VOLUME_AUTO_ADJUST),)
RCDEFS += -DUSE_VOLUME_AUTO_ADJUST
endif
