# nasm for Windows (but not cygwin) has a "w" at the end
ifndef COMPILESYSTEM_CYGWIN
ASM = @nasmw
endif

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

# only Windows specific output files and rules
# the first two targets generate the prefix.h header
# note this requires that OSOBJS be the first target
#
OSOBJS = $(OBJ)/windows/winmain.o $(OBJ)/windows/fileio.o $(OBJ)/windows/config.o \
	 $(OBJ)/windows/ticker.o $(OBJ)/windows/fronthlp.o $(OBJ)/windows/video.o \
	 $(OBJ)/windows/input.o $(OBJ)/windows/sound.o $(OBJ)/windows/blit.o \
	 $(OBJ)/windows/snprintf.o $(OBJ)/windows/rc.o $(OBJ)/windows/misc.o \
	 $(OBJ)/windows/window.o $(OBJ)/windows/wind3d.o $(OBJ)/windows/wind3dfx.o \
	 $(OBJ)/windows/winddraw.o \
	 $(OBJ)/windows/asmblit.o $(OBJ)/windows/asmtile.o

# extra targets and rules for the scale effects
ifneq ($(USE_SCALE_EFFECTS),)
CFLAGS += -DUSE_SCALE_EFFECTS
OSOBJS += $(OBJ)/windows/scale.o

OBJDIRS += $(OBJ)/windows/scale
OSOBJS += $(OBJ)/windows/scale/superscale.o $(OBJ)/windows/scale/eagle_fm.o $(OBJ)/windows/scale/2xsaimmx.o

ifneq ($(USE_MMX_INTERP_SCALE),)
DEFS += -DUSE_MMX_INTERP_SCALE
OSOBJS += $(OBJ)/windows/scale/hlq_mmx.o
else
OSOBJS += $(OBJ)/windows/scale/hlq.o
endif

$(OBJ)/windows/scale/superscale.o: src/windows/scale/superscale.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(OBJ)/windows/scale/eagle_fm.o: src/windows/scale/eagle_fm.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(OBJ)/windows/scale/2xsaimmx.o: src/windows/scale/2xsaimmx.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(OBJ)/windows/scale/hlq.o: src/windows/scale/hlq.c
    ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -Wno-unused-variable -mno-mmx -UINTERP_MMX -c $< -o $@
    else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
    endif

$(OBJ)/windows/scale/hlq_mmx.o: src/windows/scale/hlq.c
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
CLIOBJS = $(OBJ)/windows/climain.o

# add resource file for dll
ifeq ($DONT_USE_DLL,)
# add resource file if no UI
ifeq ($(WINUI),)
OSOBJS  += $(OBJ)/windows/mame.res
endif
endif

# add resource file
CLIOBJS += $(OBJ)/windows/mame.res

ifdef NEW_DEBUGGER
OSOBJS += $(OBJ)/windows/debugwin.o
endif

# enable guard pages on all memory allocations in the debug build
#ifdef DEBUG
#DEFS += -DMALLOC_DEBUG
#OSDBGOBJS = $(OBJ)/windows/winalloc.o
#OSDBGLDFLAGS = -Wl,--allow-multiple-definition
#else
OSDBGOBJS =
OSDBGLDFLAGS =
#endif

# video blitting functions
$(OBJ)/windows/asmblit.o: src/windows/asmblit.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

# tilemap blitting functions
$(OBJ)/windows/asmtile.o: src/windows/asmtile.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

# add our prefix files to the mix (we need -Wno-strict-aliasing for DirectX)
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

# due to quirks of using /bin/sh, we need to explicitly specify the current path
CURPATH = ./

# if building with a UI, set the C flags and include the ui.mak
ifneq ($(WINUI),)
CFLAGS+= -DWINUI=1
include src/ui/ui.mak
endif

# if we are not using x86drc.o, we should be
ifndef X86_MIPS3_DRC
COREOBJS += $(OBJ)/x86drc.o
endif

#####################################################################
# Resources
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
