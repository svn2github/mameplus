###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################



###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


include config.def

#-------------------------------------------------
# specify core target: mame, mess, etc.
# specify subtarget: mame, mess, tiny, etc.
# build rules will be included from 
# src/$(TARGET)/$(SUBTARGET).mak
#-------------------------------------------------

ifneq ($(HAZEMD),)
    TARGET = mame
    SUBTARGET = hazemd
    USE_DRIVER_SWITCH=
else
    ifneq ($(NEOCPSMAME),)
        TARGET = mame
        SUBTARGET = neocpsmame
    else
        ifeq ($(TARGET),)
        TARGET = mame
    endif
endif
endif

ifeq ($(SUBTARGET),)
SUBTARGET = $(TARGET)
endif



#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build rules will be included from 
# src/osd/$(OSD)/$(OSD).mak
#-------------------------------------------------

ifeq ($(OSD),)
OSD = windows
endif

ifneq ($(NO_DLL),)
    ifneq ($(WINUI),)
        SUFFIX = guinodll
    else
        SUFFIX = nodll
    endif
# no dll version
    DONT_USE_DLL=1
else
# always define WINUI = 1 for mamelib.dll
    WINUI=1
endif



#-------------------------------------------------
# specify OS target, which further differentiates
# the underlying OS; supported values are:
# win32, unix, macosx, os2
#-------------------------------------------------

ifeq ($(TARGETOS),)
ifeq ($(OSD),windows)
TARGETOS = win32
else
TARGETOS = unix
endif
endif



#-------------------------------------------------
# specify program options; see each option below 
# for details
#-------------------------------------------------

# uncomment next line to include the debugger
# DEBUG = 1

# uncomment next line to include the internal profiler
# PROFILER = 1

# uncomment next line to use DRC MIPS3 engine
X86_MIPS3_DRC = 1

# uncomment next line to use DRC PowerPC engine
X86_PPC_DRC = 1

# uncomment next line to use DRC Voodoo rasterizers
# X86_VOODOO_DRC = 1

# uncomment next line to use Assembler 68000 engine
X86_ASM_68000 = 1

# uncomment next line to use Assembler 68010 engine
X86_ASM_68010 = 1

# uncomment next line to use Assembler 68020 engine
# X86_ASM_68020 = 1

# uncomment next line to use DRC 68K engine
X86_M68K_DRC = 1



#-------------------------------------------------
# specify build options; see each option below 
# for details
#-------------------------------------------------

# uncomment one of the next lines to build a target-optimized build
# NATIVE = 1
# ATHLON = 1
# I686 = 1
# P4 = 1
# PM = 1
# AMD64 = 1
# G4 = 1
# G5 = 1
# CELL = 1

# uncomment next line if you are building for a 64-bit target
# PTR64 = 1

# uncomment next line to build expat as part of MAME build
BUILD_EXPAT = 1

# uncomment next line to build zlib as part of MAME build
BUILD_ZLIB = 1

# uncomment next line to include the symbols
# SYMBOLS = 1

# uncomment next line to include profiling information from the compiler
# PROFILE = 1

# uncomment next line to generate a link map for exception handling in windows
# MAP = 1

# specify optimization level or leave commented to use the default
# (default is OPTIMIZE = 3 normally, or OPTIMIZE = 0 with symbols)
# OPTIMIZE = 3


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

# disable DRC cores for 64-bit builds
ifneq ($(PTR64),)
X86_PPC_DRC =
X86_VOODOO_DRC =
X86_ASM_68000 =
X86_ASM_68010 =
X86_ASM_68020 =
X86_M68K_DRC =
endif

# specify a default optimization level if none explicitly stated
ifeq ($(OPTIMIZE),)
ifeq ($(SYMBOLS),)
OPTIMIZE = 3
else
OPTIMIZE = 0
endif
endif

# profiler defaults to on for DEBUG builds
ifneq ($(DEBUG),)
ifneq ($(PROFILER),)
PROFILER = 1
endif
endif



#-------------------------------------------------
# platform-specific definitions
#-------------------------------------------------

# extension for executables
EXE = 

ifeq ($(TARGETOS),win32)
EXE = .exe
endif
ifeq ($(TARGETOS),os2)
EXE = .exe
endif

# compiler, linker and utilities
    AR = @ar
    CC = @gcc
    LD = @gcc
MD = -mkdir$(EXE)
RM = @rm -f



#-------------------------------------------------
# based on the architecture, determine suffixes
# and endianness
#-------------------------------------------------

# by default, don't compile for a specific target CPU
# and assume little-endian (x86)
ARCH = 
ENDIAN = little

# architecture-specific builds get extra options
ifneq ($(NATIVE),)
    SUFFIX = nat
    ARCH = -march=native
endif

# architecture-specific builds get extra options
ifneq ($(ATHLON),)
    SUFFIX = at
    ARCH = -march=athlon
endif

ifneq ($(I686),)
    SUFFIX = pp
    ARCH = -march=pentiumpro
endif

ifneq ($(P4),)
    SUFFIX = p4
    ARCH = -march=pentium4
endif

ifneq ($(AMD64),)
    SUFFIX = 64
    ARCH = -march=athlon64
endif

ifneq ($(PM),)
    SUFFIX = pm
    ARCH = -march=pentium3 -msse2
endif

ifneq ($(G4),)
    SUFFIX = g4
    ARCH = -mcpu=G4
    ENDIAN = big
endif

ifneq ($(G5),)
    SUFFIX = g5
    ARCH = -mcpu=G5
    ENDIAN = big
endif

ifneq ($(CELL),)
    SUFFIX = cbe
    ARCH = 
    ENDIAN = big
endif


#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

# x64 builds append the 'x64' suffix
ifdef PTR64
SUFFIX:=$(SUFFIX)x64
endif

# debug builds append the 'd' suffix
ifdef DEBUG
SUFFIX:=$(SUFFIX)d
endif

# the name is just 'target' if no subtarget; otherwise it is
# the concatenation of the two (e.g., mametiny)
ifeq ($(TARGET),$(SUBTARGET))
    NAME = $(TARGET)
else
    NAME = $(TARGET)$(SUBTARGET)
endif

# fullname is prefix+name+suffix
FULLNAME = $(PREFIX)$(NAME)$(SUFFIX)
FULLGUINAME = $(PREFIX)$(NAME)gui$(SUFFIX)

# add an EXE suffix to get the final emulator name
EMULATORCLI = $(FULLNAME)$(EXE)
EMULATORGUI = $(FULLGUINAME)$(EXE)



#-------------------------------------------------
# source and object locations
#-------------------------------------------------

# all sources are under the src/ directory
SRC = src

# build the targets in different object dirs, so they can co-exist
OBJ = obj/$(OSD)/$(FULLNAME)



#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------

# CR/LF setup: use both on win32/os2, CR only on everything else
DEFS = -DCRLF=2
  
ifeq ($(TARGETOS),win32)
DEFS = -DCRLF=3
endif
ifeq ($(TARGETOS),os2)
DEFS = -DCRLF=3
endif

# map the INLINE to something digestible by GCC
DEFS += -DINLINE="static __inline__"

# define LSB_FIRST if we are a little-endian target
ifeq ($(ENDIAN),little)
DEFS += -DLSB_FIRST
endif

# MAME Plus! specific options
DEFS += -DXML_STATIC -Drestrict=__restrict

# define PTR64 if we are a 64-bit target
ifneq ($(PTR64),)
DEFS += -DPTR64
endif

# define MAME_DEBUG if we are a debugging build
ifneq ($(DEBUG),)
    DEFS += -DMAME_DEBUG
else
    DEFS += -DNDEBUG 
endif

# define MAME_PROFILER if we are a profiling build
ifneq ($(PROFILER),)
DEFS += -DMAME_PROFILER
endif

# define VOODOO_DRC if we are building the DRC Voodoo engine
ifneq ($(X86_VOODOO_DRC),)
DEFS += -DVOODOO_DRC
endif

ifneq ($(USE_SCALE_EFFECTS),)
DEFS += -DUSE_SCALE_EFFECTS
endif

ifneq ($(USE_STORY_DATAFILE),)
    DEFS += -DSTORY_DATAFILE
endif

ifneq ($(USE_TRANS_UI),)
    DEFS += -DTRANS_UI
endif

ifneq ($(USE_INP_CAPTION),)
    DEFS += -DINP_CAPTION
endif

ifneq ($(USE_AUTO_PAUSE_PLAYBACK),)
    DEFS += -DAUTO_PAUSE_PLAYBACK
endif

ifneq ($(USE_UI_COLOR_DISPLAY),)
    DEFS += -DUI_COLOR_DISPLAY
    ifneq ($(USE_CMD_LIST),)
        DEFS += -DCMD_LIST
    endif
    ifneq ($(USE_CUSTOM_BUTTON),)
        DEFS += -DUSE_CUSTOM_BUTTON
    endif
endif

ifneq ($(USE_UI_COLOR_PALETTE),)
    DEFS += -DUI_COLOR_PALETTE
endif

ifneq ($(USE_JOY_EXTRA_BUTTONS),)
    DEFS += -DUSE_JOY_EXTRA_BUTTONS
endif

ifneq ($(USE_NEOGEO_HACKS),)
    DEFS+= -DUSE_NEOGEO_HACKS
endif

ifneq ($(USE_HISCORE),)
DEFS += -DUSE_HISCORE
endif

ifneq ($(SHOW_UNAVAILABLE_FOLDER),)
    DEFS += -DSHOW_UNAVAILABLE_FOLDER
endif

ifneq ($(USE_IPS),)
    DEFS += -DUSE_IPS
endif

ifneq ($(USE_VOLUME_AUTO_ADJUST),)
    DEFS += -DUSE_VOLUME_AUTO_ADJUST
endif

ifneq ($(USE_SHOW_TIME),)
    DEFS += -DUSE_SHOW_TIME
endif

ifneq ($(USE_SHOW_INPUT_LOG),)
    DEFS += -DUSE_SHOW_INPUT_LOG
endif

ifneq ($(USE_DRIVER_SWITCH),)
    DEFS += -DDRIVER_SWITCH
endif

ifneq ($(NEOCPSMAME),)
    DEFS += -DNEOCPSMAME
endif

ifneq ($(HAZEMD),)
    DEFS += -DHAZEMD
endif


#-------------------------------------------------
# compile flags
#-------------------------------------------------

# we compile to C89 standard with GNU extensions
CFLAGS = -std=gnu89

# add -g if we need symbols
ifneq ($(SYMBOLS),)
CFLAGS += -g
endif

# add a basic set of warnings
CFLAGS += \
	-Wall \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wundef \
	-Wno-format-security \
	-Wwrite-strings \
	-Wno-pointer-sign \

# add profiling information for the compiler
ifneq ($(PROFILE),)
CFLAGS += -pg
endif

# this warning is not supported on the os2 compilers
ifneq ($(TARGETOS),os2)
CFLAGS += -Wdeclaration-after-statement
endif

# add the optimization flag
CFLAGS += -O$(OPTIMIZE)

# if we are optimizing, include optimization options
# and make all errors into warnings
ifneq ($(OPTIMIZE),0)
CFLAGS += $(ARCH) -fno-strict-aliasing
endif

# if symbols are on, make sure we have frame pointers
ifneq ($(SYMBOLS),)
CFLAGS += -fno-omit-frame-pointer
endif

ifeq ($(NO_DLL),)
	DEFS += -DWIN32 -DWINNT
	EMULATORDLL = $(FULLNAME)lib.dll
	EMULATORALL = $(EMULATORDLL) $(EMULATORCLI) $(EMULATORGUI)
else
	EMULATORALL = $(EMULATORCLI)
endif

#-------------------------------------------------
# include paths
#-------------------------------------------------

# add core include paths
CFLAGS += \
	-I$(SRC)/$(TARGET) \
	-I$(SRC)/$(TARGET)/includes \
	-I$(OBJ)/$(TARGET)/layout \
	-I$(SRC)/emu \
	-I$(OBJ)/emu \
	-I$(OBJ)/emu/layout \
	-I$(SRC)/lib/util \
	-I$(SRC)/osd \
	-I$(SRC)/osd/$(OSD) \

CFLAGS += \
	-I$(SRC)/mess \
	-I$(SRC)/mess/includes \
	-I$(SRC)/mess/osd/$(OSD)



#-------------------------------------------------
# linking flags
#-------------------------------------------------

# LDFLAGS are used generally; LDFLAGSEMULATOR are additional
# flags only used when linking the core emulator
LDFLAGS = -Wl,--warn-common -Lextra/lib
LDFLAGSEMULATOR =

# add profiling information for the linker
ifneq ($(PROFILE),)
    LDFLAGS += -pg
endif

# strip symbols and other metadata in non-symbols and non profiling builds

ifeq ($(SYMBOLS),)
    ifeq ($(PROFILE),)
        LDFLAGS += -s
    endif
endif

# output a map file (emulator only)
ifneq ($(MAP),)
    MAPFLAGS = -Wl,-Map,$(FULLNAME).map
    MAPDLLFLAGS = -Wl,-Map,$(LIBNAME).map
    MAPCLIFLAGS = -Wl,-Map,$(FULLNAME).map
    MAPGUIFLAGS = -Wl,-Map,$(FULLGUINAME).map
else
    MAPFLAGS =
    MAPDLLFLAGS =
    MAPCLIFLAGS =
    MAPGUIFLAGS =
endif

# any reason why this doesn't work for all cases?
ifeq ($(TARGETOS),macosx)
LDFLAGSEMULATOR += -Xlinker -all_load
endif



#-------------------------------------------------
# define the standard object directory; other
# projects can add their object directories to
# this variable
#-------------------------------------------------

OBJDIRS = $(OBJ)



#-------------------------------------------------
# define standard libarires for CPU and sounds
#-------------------------------------------------

LIBEMU = $(OBJ)/libemu.a
LIBCPU = $(OBJ)/libcpu.a
LIBSOUND = $(OBJ)/libsound.a
LIBUTIL = $(OBJ)/libutil.a
LIBOCORE = $(OBJ)/libocore.a
LIBOSD = $(OBJ)/libosd.a

ifeq ($(HAZEMD),)
    VERSIONOBJ = $(OBJ)/version.o
else
    VERSIONOBJ = $(OBJ)/versionmd.o
endif


#-------------------------------------------------
# either build or link against the included 
# libraries
#-------------------------------------------------

# start with an empty set of libs
LIBS = 

# add expat XML library
ifneq ($(BUILD_EXPAT),)
CFLAGS += -I$(SRC)/lib/expat
EXPAT = $(OBJ)/libexpat.a
else
LIBS += -lexpat
EXPAT =
endif

# add ZLIB compression library
ifneq ($(BUILD_ZLIB),)
CFLAGS += -I$(SRC)/lib/zlib
ZLIB = $(OBJ)/libz.a
else
LIBS += -lz
ZLIB =
endif



#-------------------------------------------------
# 'all' target needs to go here, before the 
# include files which define additional targets
#-------------------------------------------------

all: maketree buildtools emulator



#-------------------------------------------------
# include the various .mak files
#-------------------------------------------------

# include OSD-specific rules first
include $(SRC)/osd/$(OSD)/$(OSD).mak

# then the various core pieces
include $(SRC)/$(TARGET)/$(SUBTARGET).mak
#include $(SRC)/mess/osd/$(OSD)/$(OSD).mak
#include $(SRC)/mess/messcore.mak
include $(SRC)/lib/lib.mak
include $(SRC)/build/build.mak
include $(SRC)/tools/tools.mak
include $(SRC)/emu/emu.mak

# combine the various definitions to one
CDEFS = $(DEFS) $(COREDEFS) $(CPUDEFS) $(SOUNDDEFS) $(ASMDEFS)

ifneq ($(BUILD_EXPAT),)
COREOBJS += $(EXPAT)
endif

ifneq ($(BUILD_ZLIB),)
COREOBJS += $(ZLIB)
endif


#-------------------------------------------------
# primary targets
#-------------------------------------------------

emulator: maketree $(BUILD) $(EMULATORALL)

buildtools: maketree $(BUILD)

tools: maketree $(TOOLS)

maketree: $(sort $(OBJDIRS))

clean:
	@echo Deleting object tree $(OBJ)...
	$(RM) -r $(OBJ)
	@echo Deleting $(EMULATORALL)...
	$(RM) $(EMULATORALL)
	@echo Deleting $(TOOLS)...
	$(RM) $(TOOLS)
ifneq ($(MAP),)
	@echo Deleting $(FULLNAME).map...
	$(RM) $(FULLNAME).map
endif



#-------------------------------------------------
# directory targets
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) -p $@



#-------------------------------------------------
# executable targets and dependencies
#-------------------------------------------------

ifdef MSVC_BUILD
DLLLINK=lib
else
DLLLINK=dll
endif

ifeq ($(NO_DLL),)
$(EMULATORDLL): $(VERSIONOBJ) $(OBJ)/osd/windows/mamelib.o $(DRVLIBS) $(LIBOSD) $(LIBEMU) $(LIBCPU) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE)
# always recompile the version string
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) -shared $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@ $(MAPDLLFLAGS)
endif

ifeq ($(NO_DLL),)
# gui target
$(EMULATORGUI):	$(EMULATORDLL) $(OBJ)/osd/ui/guimain.o $(GUIRESFILE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mwindows $(FULLNAME)lib.$(DLLLINK) $(OBJ)/osd/ui/guimain.o $(GUIRESFILE) $(LIBS) -o $@ $(MAPCLIFLAGS)
endif

# cli target
ifeq ($(NO_DLL),)
    $(EMULATORCLI): $(EMULATORDLL) $(OBJ)/osd/windows/climain.o
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $(FULLNAME)lib.$(DLLLINK) $(OBJ)/osd/windows/climain.o $(LIBS) -o $@ $(MAPCLIFLAGS)
else
	$(EMULATORCLI):	$(VERSIONOBJ) $(DRVLIBS) $(LIBOSD) $(LIBEMU) $(LIBCPU) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $^ $(LIBS) -o $@
endif

#-------------------------------------------------
# generic rules
#-------------------------------------------------

$(OBJ)/mess/%.o: $(SRC)/mess/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@

$(OBJ)/mess/devices/%.o: $(SRC)/mess/devices/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@

$(OBJ)/mess/drivers/%.o: $(SRC)/mess/drivers/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@

$(OBJ)/mess/osd/windows/%.o: $(SRC)/mess/osd/windows/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.pp: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(OBJ)/%.lh: $(SRC)/%.lay $(FILE2STR)
	@echo Converting $<...
	@$(FILE2STR) $< $@ layout_$(basename $(notdir $<))

$(OBJ)/%.fh: $(OBJ)/%.bdc $(FILE2STR)
	@echo Converting $<...
	@$(FILE2STR) $< $@ font_$(basename $(notdir $<)) UINT8

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) -cr $@ $^

ifeq ($(TARGETOS),macosx)
$(OBJ)/%.o: $(SRC)/%.m | $(OSPREBUILD)
	@echo Objective-C compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
endif
