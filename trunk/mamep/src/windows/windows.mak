###########################################################################
#
#   windows.mak
#
#   Windows-specific makefile
#
#   Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


#-------------------------------------------------
# nasm for Windows (but not cygwin) has a "w"
# at the end
#-------------------------------------------------

ifndef COMPILESYSTEM_CYGWIN
ASM = @nasmw
endif



#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific flags and libararies
#-------------------------------------------------

# add our prefix files to the mix
ifdef USE_GCC
  CFLAGS += -mwindows -include src/$(MAMEOS)/winprefix.h
  CFLAGSOSDEPEND += -Wno-strict-aliasing
  CFLAGS += -include src/$(MAMEOS)/winprefix.h
else
  CFLAGS += /FI"windows/winprefix.h"
endif

# add the windows libaries
ifdef USE_GCC
LIBS += -lunicows -luser32 -lgdi32 -lddraw -ldsound -ldinput -ldxguid -lwinmm
else
LIBS += unicows.lib user32.lib gdi32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib winmm.lib advapi32.lib
endif
CLILIBS =



DEFS += -DMAMENAME=APPNAME

ifeq ($(USE_GCC),)
    DEFS += -DINVALID_FILE_ATTRIBUTES=\(\(DWORD\)-1\)
    DEFS += -DINVALID_SET_FILE_POINTER=\(\(DWORD\)-1\)
    DEFS += -DWIN32
    DEFS += -DZEXPORT= -DZEXTERN= 
endif

DEFS+= -DNONAMELESSUNION
DEFS+= -DDIRECTSOUND_VERSION=0x0300
DEFS+= -DDIRECTDRAW_VERSION=0x0300
DEFS+= -DCLIB_DECL=__cdecl
DEFS+= -DDECL_SPEC=

# Support Stick-type Pointing Device by miko2u@hotmail.com
ifneq ($(USE_JOY_MOUSE_MOVE),)
DEFS+= -DDIRECTINPUT_VERSION=0x0700
else
DEFS+= -DDIRECTINPUT_VERSION=0x0500
endif

ifneq ($(USE_JOYSTICK_ID),)
DEFS += -DJOYSTICK_ID
endif

# Support Stick-type Pointing Device by miko2u@hotmail.com
ifneq ($(USE_JOY_MOUSE_MOVE),)
DEFS += -DUSE_JOY_MOUSE_MOVE
endif

# enable the Scale2x, Eagle and 2xSaI scale effects
# USE_SCALE_EFFECTS = 1

#-------------------------------------------------
# Windows-specific objects
#-------------------------------------------------

OSOBJS = \
	$(OBJ)/$(MAMEOS)/asmblit.o \
	$(OBJ)/$(MAMEOS)/asmtile.o \
	$(OBJ)/$(MAMEOS)/blit.o \
	$(OBJ)/$(MAMEOS)/config.o \
	$(OBJ)/$(MAMEOS)/fileio.o \
	$(OBJ)/$(MAMEOS)/fronthlp.o \
	$(OBJ)/$(MAMEOS)/input.o \
	$(OBJ)/$(MAMEOS)/misc.o \
	$(OBJ)/$(MAMEOS)/rc.o \
	$(OBJ)/$(MAMEOS)/sound.o \
	$(OBJ)/$(MAMEOS)/ticker.o \
	$(OBJ)/$(MAMEOS)/video.o \
	$(OBJ)/$(MAMEOS)/window.o \
	$(OBJ)/$(MAMEOS)/wind3d.o \
	$(OBJ)/$(MAMEOS)/wind3dfx.o \
	$(OBJ)/$(MAMEOS)/winddraw.o \
	$(OBJ)/$(MAMEOS)/winmain.o \

# extra targets and rules for the scale effects
ifneq ($(USE_SCALE_EFFECTS),)
CFLAGS += -DUSE_SCALE_EFFECTS
OSOBJS += $(OBJ)/$(MAMEOS)/scale.o

OBJDIRS += $(OBJ)/$(MAMEOS)/scale
OSOBJS += $(OBJ)/$(MAMEOS)/scale/superscale.o $(OBJ)/$(MAMEOS)/scale/eagle_fm.o $(OBJ)/$(MAMEOS)/scale/2xsaimmx.o

ifneq ($(USE_MMX_INTERP_SCALE),)
DEFS += -DUSE_MMX_INTERP_SCALE
OSOBJS += $(OBJ)/$(MAMEOS)/scale/hlq_mmx.o
else
OSOBJS += $(OBJ)/$(MAMEOS)/scale/hlq.o
endif

$(OBJ)/$(MAMEOS)/scale/superscale.o: src/$(MAMEOS)/scale/superscale.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(OBJ)/$(MAMEOS)/scale/eagle_fm.o: src/$(MAMEOS)/scale/eagle_fm.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(OBJ)/$(MAMEOS)/scale/2xsaimmx.o: src/$(MAMEOS)/scale/2xsaimmx.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/$(MAMEOS)/scale/hlq.o: src/$(MAMEOS)/scale/hlq.c
    ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -Wno-unused-variable -mno-mmx -UINTERP_MMX -c $< -o $@
    else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
    endif

$(OBJ)/$(MAMEOS)/scale/hlq_mmx.o: src/$(MAMEOS)/scale/hlq.c
    ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -Wno-unused-variable -mmmx -DINTERP_MMX -c $< -o $@
    else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
    endif
endif

ifdef USE_GCC
VCOBJS =
else
CFLAGS += -Isrc/vc
OBJDIRS += $(OBJ)/vc
VCOBJS = $(OBJ)/vc/dirent.o
endif

OSOBJS += $(VCOBJS)
CLIOBJS = $(OBJ)/$(MAMEOS)/climain.o

# add debug-specific files
ifdef DEBUG
OSOBJS += \
	$(OBJ)/$(MAMEOS)/debugwin.o
endif

# add resource file for dll
ifeq ($DONT_USE_DLL,)
# non-UI builds need a stub resource file
ifeq ($(WINUI),)
OSOBJS += $(OBJ)/$(MAMEOS)/mame.res
endif
endif

# add resource file
CLIOBJS += $(OBJ)/$(MAMEOS)/mame.res



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

OSDBGOBJS =
OSDBGLDFLAGS =

# debug build: enable guard pages on all memory allocations
ifdef DEBUG
DEFS += -DMALLOC_DEBUG
OSDBGOBJS += $(OBJ)/$(MAMEOS)/winalloc.o
OSDBGLDFLAGS += -Wl,--allow-multiple-definition
endif



#-------------------------------------------------
# rules for assembly targets
#-------------------------------------------------

# video blitting functions
$(OBJ)/$(MAMEOS)/asmblit.o: src/$(MAMEOS)/asmblit.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

# tilemap blitting functions
$(OBJ)/$(MAMEOS)/asmtile.o: src/$(MAMEOS)/asmtile.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<




#-------------------------------------------------
# if building with a UI, set the C flags and
# include the ui.mak
#-------------------------------------------------

ifneq ($(WINUI),)
CFLAGS += -DWINUI=1
include src/ui/ui.mak
endif

# if we are not using x86drc.o, we should be
ifndef X86_MIPS3_DRC
COREOBJS += $(OBJ)/x86drc.o
endif



#-------------------------------------------------
# generic rule for the resource compiler
#-------------------------------------------------

ifdef USE_GCC
    ifndef USE_XGCC
        RC = @windres --use-temp-file
    else
        RC = @i686-pc-mingw32-windres
    endif
    RCDEFS += -DNDEBUG -D_WIN32_IE=0x0400
    RCFLAGS += -O coff --include-dir src
else
    RC = @rc
    RCDEFS += -D_WIN32_IE=0x0400
    RCFLAGS += -Isrc
endif

$(OBJ)/%.res: src/%.rc
	@echo Compiling resources $<...
ifdef USE_GCC
	$(RC) $(RCDEFS) $(RCFLAGS) -o $@ -i $<
else
	$(RC) $(RCDEFS) $(RCFLAGS) -Fo$@ $<
endif
