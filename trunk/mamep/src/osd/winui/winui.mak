#####################################################################
# make SUFFIX=32
#####################################################################

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

WINUISRC = $(SRC)/osd/winui
WINUIOBJ = $(OBJ)/osd/winui

OBJDIRS += $(WINUIOBJ)

MESS_WINSRC = $(SRC)/mess/osd/windows
MESS_WINOBJ = $(OBJ)/mess/osd/windows
MESS_WINUISRC = $(SRC)/mess/osd/winui
MESS_WINUIOBJ = $(OBJ)/mess/osd/winui
OBJDIRS += $(MESS_WINUIOBJ)
CFLAGS += -I$(WINUISRC) -I$(MESS_WINUISRC)

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
	$(WINUIOBJ)/mui_util.o \
	$(WINUIOBJ)/directinput.o \
	$(WINUIOBJ)/dijoystick.o \
	$(WINUIOBJ)/directdraw.o \
	$(WINUIOBJ)/directories.o \
	$(WINUIOBJ)/mui_audit.o \
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
	$(WINUIOBJ)/winui.o \
	$(WINUIOBJ)/mui_opts.o \
	$(WINUIOBJ)/layout.o \
	$(WINUIOBJ)/translate.o \
	$(WINUIOBJ)/helpids.o \
	$(MESS_WINUIOBJ)/optionsms.o \


ifneq ($(USE_UI_COLOR_PALETTE),)
    WINUIOBJS += $(WINUIOBJ)/paletteedit.o
endif

ifneq ($(USE_IMAGE_MENU),)
    WINUIOBJS += $(WINUIOBJ)/imagemenu.o
endif

$(LIBOSD): $(WINUIOBJS)

$(WINUIOBJ)/mameui.res: $(WINUISRC)/mameui.rc $(WINUIOBJ)/mameversui.rc

$(WINUIOBJ)/winuiopt.o: $(WINUISRC)/optdef.h $(WINUISRC)/opthndlr.h $(WINUISRC)/opthndlr.c


#####################################################################
# compiler

#
# Preprocessor Definitions
#

DEFS += \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500



#####################################################################
# Resources

VERINFO32 = $(WINUIOBJ)/verinfo32$(EXE)

$(WINUIOBJ)/verinfo32.o: $(SRC)/build/verinfo.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DWINUI=1 $(CFLAGS) -c $< -o $@

$(VERINFO32): $(WINUIOBJ)/verinfo32.o $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

BUILD += $(VERINFO32)


UI_RC = @windres --use-temp-file

UI_RCDEFS = -DNDEBUG -D_WIN32_IE=0x0400

UI_RCFLAGS = -O coff --include-dir $(WINUISRC) --include-dir $(WINUIOBJ) --include-dir $(MESS_WINUISRC)

$(WINUIOBJ)/%.res: $(WINUISRC)/%.rc
	@echo Compiling mame32 resources $<...
	$(UI_RC) $(UI_RCDEFS) $(UI_RCFLAGS) -o $@ -i $<

$(WINUIOBJ)/mameversui.rc: $(VERINFO32) $(SRC)/version.c
	@echo Emitting $@...
	@$(VERINFO32) $(SRC)/version.c > $@

ifeq ($(NO_DLL),)
    GUIRESFILE = $(WINUIOBJ)/mameui.res
else
    UI_RCFLAGS += --include-dir $(MESS_WINSRC)
    $(MESS_WINUIOBJ)/messgui.res: $(MESS_WINSRC)/mess.rc $(WINUISRC)/mameui.rc $(WINUIOBJ)/mameversui.rc
    GUIRESFILE = $(MESS_WINUIOBJ)/messgui.res
endif

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
UI_RCDEFS += -DUI_COLOR_DISPLAY
endif

ifneq ($(USE_UI_COLOR_PALETTE),)
UI_RCDEFS += -DUI_COLOR_PALETTE
endif

ifneq ($(USE_IPS),)
UI_RCDEFS += -DUSE_IPS
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

ifneq ($(USE_STORY_DATAFILE),)
UI_RCDEFS += -DSTORY_DATAFILE
endif

ifneq ($(USE_JOYSTICK_ID),)
UI_RCDEFS += -DJOYSTICK_ID
endif

ifneq ($(USE_JOY_MOUSE_MOVE),)
UI_RCDEFS += -DUSE_JOY_MOUSE_MOVE
endif

ifneq ($(USE_VOLUME_AUTO_ADJUST),)
UI_RCDEFS += -DUSE_VOLUME_AUTO_ADJUST
endif

ifneq ($(USE_DRIVER_SWITCH),)
UI_RCDEFS += -DDRIVER_SWITCH
endif

ifneq ($(NEOCPSMAME),)
UI_RCDEFS += -DNEOCPSMAME
endif

#####################################################################
# Windows UI specific options

ifneq ($(USE_MISC_FOLDER),)
DEFS += -DMISC_FOLDER
UI_RCDEFS += -DMISC_FOLDER
endif

ifneq ($(USE_SHOW_SPLASH_SCREEN),)
DEFS += -DUSE_SHOW_SPLASH_SCREEN
UI_RCDEFS += -DUSE_SHOW_SPLASH_SCREEN
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
