###########################################################################
#
#   windows.mak
#
#   Windows-specific makefile
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
# uncomment next line to enable a build using Intel tools
# ICC_BUILD = 1

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
# object and source roots
#-------------------------------------------------

WINSRC = $(SRC)/osd/$(OSD)
WINOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(WINOBJ)



#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

ifeq ($(USE_XGCC),)
    RC = @windres --use-temp-file
else
    RC = @i686-pc-mingw32-windres
endif
RCDEFS += -DNDEBUG -D_WIN32_IE=0x0400
RCFLAGS += -O coff --include-dir src/osd



#-------------------------------------------------
# overrides for the CYGWIN compiler
#-------------------------------------------------

ifdef CYGWIN_BUILD
CFLAGS += -mno-cygwin
LDFLAGS	+= -mno-cygwin
endif



#-------------------------------------------------
# overrides for the MSVC compiler
#-------------------------------------------------

ifneq ($(ICC_BUILD),)
    COMPILER_SUFFIX = -icc
    VCONVDEFS = -DICC_BUILD
    MSVC_BUILD = 1
else
    ifneq ($(MSVC_BUILD),)
        COMPILER_SUFFIX = -vc
        VCONVDEFS =
    else
        CFLAGS += -Iextra/include
    endif
endif

ifneq ($(MSVC_BUILD),)
# replace the various compilers with vconv.exe prefixes
CC = @$(OBJ)/vconv.exe gcc -I.
LD = @$(OBJ)/vconv.exe ld /profile
AR = @$(OBJ)/vconv.exe ar
RC = @$(OBJ)/vconv.exe windres

# make sure we use the multithreaded runtime
CC += /MT

# turn on link-time codegen if the MAXOPT flag is also set
ifneq ($(MAXOPT),)
    ifneq ($(ICC_BUILD),)
        CC += /Qipo /Qipo_obj
    else
        CC += /GL
        LD += /LTCG
    endif
endif

# /Og enable global optimization
# /Ob<n> inline expansion (level 2)
# /Oi enable intrinsic functions
# /Ot favor code speed
# /Oy enable frame pointer omission
# /GA optimize for Windows Application
# /Gy separate functions for linker
# /GF enable read-only string pooling
CC += /Og /Ob2 /Oi /Ot /Oy /GA /Gy /GF

ifdef PTR64
CC += /wd4267 /Wp64
endif

# filter X86_ASM define
DEFS := $(filter-out -DX86_ASM,$(DEFS))

# add some VC++-specific defines
DEFS += -D_CRT_SECURE_NO_DEPRECATE -DXML_STATIC -Dinline=__inline -D__inline__=__inline -Dsnprintf=_snprintf -Dvsnprintf=_vsnprintf

# make msvcprep into a pre-build step
OSPREBUILD = $(OBJ)/vconv.exe

$(OBJ)/vconv.exe: $(WINOBJ)/vconv.o
	@echo Linking $@...
ifdef PTR64
	@link.exe /nologo $^ version.lib bufferoverflowu.lib /out:$@
else
	@link.exe /nologo $^ version.lib /out:$@
endif

$(WINOBJ)/vconv.o: $(WINSRC)/vconv.c
	@echo Compiling $<...
	@cl.exe /nologo /O1 -D_CRT_SECURE_NO_DEPRECATE -c $< /Fo$@

else
# overwrite optimze option for Pentium M
    ifneq ($(PM),)
        ARCH = -march=pentium3 -msse2
    endif
endif



#-------------------------------------------------
# nasm for Windows (but not cygwin) has a "w"
# at the end
#-------------------------------------------------

ASM = @nasm
ASMFLAGS = -f coff

ifeq ($(COMPILESYSTEM_CYGWIN),)
ASM = @nasmw
endif



#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# debug build: enable guard pages on all memory allocations
ifneq ($(DEBUG),)
ifeq ($(WINUI),)
DEFS += -DMALLOC_DEBUG
LDFLAGS += -Wl,--allow-multiple-definition
endif
endif

ifdef UNICODE
DEFS += -DUNICODE -D_UNICODE
endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

# add our prefix files to the mix
CFLAGS += -mwindows -include $(WINSRC)/winprefix.h

ifneq ($(NO_FORCEINLINE),)
DEFS += -DNO_FORCEINLINE
endif

ifneq ($(WIN95_MULTIMON),)
CFLAGS += -DWIN95_MULTIMON
endif

# add the windows libaries
// mamep: -lunicows MUST be in the first place
LIBS += -lunicows -luser32 -lgdi32 -lddraw -ldsound -ldinput -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi
CLILIBS =

ifdef PTR64
LIBS += -lbufferoverflowu
endif



DEFS += -DMAMENAME=APPNAME

#DEFS+= -DNONAMELESSUNION
DEFS+= -DDIRECTSOUND_VERSION=0x0300
DEFS+= -DDIRECTDRAW_VERSION=0x0300
DEFS+= -DCLIB_DECL=__cdecl
DEFS+= -DDECL_SPEC=

ifneq ($(USE_JOY_MOUSE_MOVE),)
DEFS+= -DDIRECTINPUT_VERSION=0x0700
else
DEFS+= -DDIRECTINPUT_VERSION=0x0500
endif

ifneq ($(USE_JOYSTICK_ID),)
DEFS += -DJOYSTICK_ID
endif


#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(WINOBJ)/main.o	\
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

# remove main.o from OSDCOREOBJS
ifneq ($(WINUI),)
ifneq ($(NO_DLL),)
OSDCOREOBJS := $(OSDCOREOBJS:$(WINOBJ)/main.o=)
OSDMAIN = $(WINOBJ)/main.o
endif
endif

$(LIBOCORE): $(OSDCOREOBJS)



#-------------------------------------------------
# Windows-specific objects
#-------------------------------------------------

OSDOBJS = \
	$(WINOBJ)/d3d8intf.o \
	$(WINOBJ)/d3d9intf.o \
	$(WINOBJ)/drawd3d.o \
	$(WINOBJ)/drawdd.o \
	$(WINOBJ)/drawgdi.o \
	$(WINOBJ)/drawnone.o \
	$(WINOBJ)/input.o \
	$(WINOBJ)/output.o \
	$(WINOBJ)/sound.o \
	$(WINOBJ)/video.o \
	$(WINOBJ)/window.o \
	$(WINOBJ)/winmain.o

# extra dependencies
$(WINOBJ)/drawdd.o : 	$(SRC)/emu/rendersw.c
$(WINOBJ)/drawgdi.o :	$(SRC)/emu/rendersw.c

# extra targets and rules for the scale effects
ifneq ($(USE_SCALE_EFFECTS),)
OSDOBJS += $(WINOBJ)/scale.o

OBJDIRS += $(WINOBJ)/scale
OSDOBJS += $(WINOBJ)/scale/superscale.o $(WINOBJ)/scale/eagle.o $(WINOBJ)/scale/2xsaimmx.o
OSDOBJS += $(WINOBJ)/scale/scale2x.o $(WINOBJ)/scale/scale3x.o

ifneq ($(USE_MMX_INTERP_SCALE),)
DEFS += -DUSE_MMX_INTERP_SCALE
OSDOBJS += $(WINOBJ)/scale/hlq_mmx.o
else
OSDOBJS += $(WINOBJ)/scale/hlq.o
endif

ifneq ($(USE_4X_SCALE),)
DEFS += -DUSE_4X_SCALE
endif

$(WINOBJ)/scale/superscale.o: $(WINSRC)/scale/superscale.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(WINOBJ)/scale/eagle.o: $(WINSRC)/scale/eagle.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<
$(WINOBJ)/scale/2xsaimmx.o: $(WINSRC)/scale/2xsaimmx.asm
	@echo Assembling $<...
	$(ASM) -o $@ $(ASMFLAGS) $(subst -D,-d,$(ASMDEFS)) $<

$(WINOBJ)/scale/hlq.o: $(WINSRC)/scale/hlq.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -Wno-strict-aliasing -Wno-unused-variable -mno-mmx -UINTERP_MMX -c $< -o $@

$(WINOBJ)/scale/hlq_mmx.o: $(WINSRC)/scale/hlq.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -Wno-strict-aliasing -Wno-unused-variable -mmmx -DINTERP_MMX -c $< -o $@
endif

OSDOBJS += $(VCOBJS)
CLIOBJS = $(WINOBJ)/climain.o

# add debug-specific files
ifneq ($(DEBUG),)
OSDOBJS += \
	$(WINOBJ)/debugwin.o
endif

# add resource file for dll
ifeq ($DONT_USE_DLL,)
# non-UI builds need a stub resource file
ifeq ($(WINUI),)
OSDOBJS += $(WINOBJ)/mame.res
endif
endif

# add resource file
CLIOBJS += $(WINOBJ)/mame.res



#-------------------------------------------------
# if building with a UI, set the C flags and
# include the ui.mak
#-------------------------------------------------

ifneq ($(WINUI),)
CFLAGS += -DWINUI=1
include $(WINSRC)/../ui/ui.mak
endif

$(LIBOSD): $(OSDOBJS)



#-------------------------------------------------
# rule for making the ledutil sample
#-------------------------------------------------

ledutil$(EXE): $(WINOBJ)/ledutil.o $(OSDMAIN) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) -mwindows $(OSDBGLDFLAGS) $^ $(LIBS) -o $@

TOOLS += ledutil$(EXE)

# if we are not using x86drc.o, we should be
ifeq ($(X86_MIPS3_DRC),)
COREOBJS += $(OBJ)/x86drc.o
endif


#-------------------------------------------------
# generic rule for the resource compiler
#-------------------------------------------------

$(OBJ)/%.res: $(SRC)/%.rc | $(OSPREBUILD)
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) -o $@ -i $<
