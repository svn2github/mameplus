###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
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

ifndef TARGET
TARGET = mame
endif

ifdef NEOCPSMAME
SUBTARGET = neocpsmame
endif

ifndef SUBTARGET
SUBTARGET = $(TARGET)
endif



#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build rules will be included from 
# src/osd/$(OSD)/$(OSD).mak
#-------------------------------------------------

ifndef OSD
OSD = windows
endif

ifndef CROSS_BUILD_OSD
CROSS_BUILD_OSD = $(OSD)
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

ifndef TARGETOS
ifeq ($(OSD),windows)
TARGETOS = win32
else
TARGETOS = unix
endif
endif



#-------------------------------------------------
# configure name of final executable
#-------------------------------------------------

# uncomment and specify prefix to be added to the name
# PREFIX =

# uncomment and specify suffix to be added to the name
# SUFFIX =



#-------------------------------------------------
# specify architecture-specific optimizations
#-------------------------------------------------

# uncomment and specify architecture-specific optimizations here
# some examples:
#   optimize for I686:   ARCHOPTS = -march=pentiumpro
#   optimize for Core 2: ARCHOPTS = -march=pentium-m -msse3
#   optimize for G4:     ARCHOPTS = -mcpu=G4
# note that we leave this commented by default so that you can
# configure this in your environment and never have to think about it
# ARCHOPTS =



#-------------------------------------------------
# specify program options; see each option below 
# for details
#-------------------------------------------------

# uncomment next line to build a debug version
# DEBUG = 1

# uncomment next line to include the internal profiler
# PROFILER = 1

# uncomment the force the universal DRC to always use the C backend
# you may need to do this if your target architecture does not have
# a native backend
# FORCE_DRC_C_BACKEND = 1

# uncomment next line to use DRC 68K engine
X86_M68K_DRC = 1



#-------------------------------------------------
# specify build options; see each option below 
# for details
#-------------------------------------------------

# uncomment next line if you are building for a 64-bit target
# PTR64 = 1

# uncomment next line if you are building for a big-endian target
# BIGENDIAN = 1

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

# uncomment next line to generate verbose build information
# VERBOSE = 1

# specify optimization level or leave commented to use the default
# (default is OPTIMIZE = 3 normally, or OPTIMIZE = 0 with symbols)
# OPTIMIZE = 3


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

# specify a default optimization level if none explicitly stated
ifndef OPTIMIZE
ifndef SYMBOLS
OPTIMIZE = 3
else
OPTIMIZE = 0
endif
endif

# profiler defaults to on for DEBUG builds
ifdef DEBUG
ifndef PROFILER
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

ifndef BUILD_EXE
BUILD_EXE = $(EXE)
endif

# compiler, linker and utilities
    AR = @ar
    CC = @gcc
    LD = @gcc
MD = -mkdir$(EXE)
RM = @rm -f



#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

# debug builds just get the 'd' suffix and nothing more
ifdef DEBUG
DEBUGSUFFIX = d
endif

# the name is just 'target' if no subtarget; otherwise it is
# the concatenation of the two (e.g., mametiny)
ifeq ($(TARGET),$(SUBTARGET))
    NAME = $(TARGET)$(EXTRA_SUFFIX)
else
    NAME = $(TARGET)$(SUBTARGET)$(EXTRA_SUFFIX)
endif

# fullname is prefix+name+suffix+debugsuffix
FULLNAME = $(PREFIX)$(NAME)$(SUFFIX)$(DEBUGSUFFIX)

ifeq ($(NO_DLL),)
DEFS += -DWIN32 -DWINNT

EMULATORCLI = $(FULLNAME)$(EXE)
EMULATORDLL = $(FULLNAME)lib.dll
EMULATORGUI = $(FULLNAME)gui$(EXE)
EMULATORALL = $(EMULATORDLL) $(EMULATORCLI) $(EMULATORGUI)
else
# add an EXE suffix to get the final emulator name
EMULATOR = $(FULLNAME)$(EXE)
EMULATORALL = $(EMULATOR)
endif


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

ifdef MSVC_BUILD
    ifdef NO_FORCEINLINE
        DEFS += -DINLINE="static __inline"
    else
        DEFS += -DINLINE="static __forceinline"
    endif
else
# map the INLINE to something digestible by GCC
    DEFS += -DINLINE="static __inline__"
endif

# define LSB_FIRST if we are a little-endian target
ifndef BIGENDIAN
DEFS += -DLSB_FIRST
endif

# MAME Plus! specific options
DEFS += -DXML_STATIC -Drestrict=__restrict

# define PTR64 if we are a 64-bit target
ifdef PTR64
DEFS += -DPTR64
# disable DRC cores for 64-bit builds
X86_M68K_DRC =
endif

# define MAME_DEBUG if we are a debugging build
ifdef DEBUG
    DEFS += -DMAME_DEBUG
else
    DEFS += -DNDEBUG 
endif

# define MAME_PROFILER if we are a profiling build
ifdef PROFILER
DEFS += -DMAME_PROFILER
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

ifneq ($(MAMEMESS),)
    DEFS += -DMAMEMESS
endif


#-------------------------------------------------
# compile flags
#-------------------------------------------------

# we compile to C89 standard with GNU extensions
CFLAGS = -std=gnu89

# this speeds it up a bit by piping between the preprocessor/compiler/assembler
CFLAGS += -pipe

# add -g if we need symbols
ifdef SYMBOLS
CFLAGS += -g
endif

# add -v if we need verbose build information
ifdef VERBOSE
CFLAGS += -v
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
ifdef PROFILE
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
CFLAGS += -Wno-error $(ARCHOPTS) -fno-strict-aliasing
endif

# if symbols are on, make sure we have frame pointers
ifdef SYMBOLS
CFLAGS += -fno-omit-frame-pointer
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



#-------------------------------------------------
# linking flags
#-------------------------------------------------

# LDFLAGS are used generally; LDFLAGSEMULATOR are additional
# flags only used when linking the core emulator
LDFLAGS = -Wl,--warn-common -Lextra/lib
LDFLAGSEMULATOR =

# add profiling information for the linker
ifdef PROFILE
LDFLAGS += -pg
endif

# strip symbols and other metadata in non-symbols and non profiling builds
ifndef SYMBOLS
ifndef PROFILE
LDFLAGS += -s
endif
endif

# output a map file (emulator only)
ifneq ($(MAP),)
    ifeq ($(NO_DLL),)
        MAPCLIFLAGS = -Wl,-Map,$(FULLNAME).map
        MAPDLLFLAGS = -Wl,-Map,$(FULLNAME)lib.map
        MAPGUIFLAGS = -Wl,-Map,$(FULLNAME)gui.map
    else
        MAPFLAGS = -Wl,-Map,$(FULLNAME).map
    endif
else
    MAPFLAGS =
    MAPCLIFLAGS =
    MAPDLLFLAGS =
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
LIBOCORE_NOMAIN = $(OBJ)/libocore_nomain.a
LIBOSD = $(OBJ)/libosd.a

VERSIONOBJ = $(OBJ)/version.o



#-------------------------------------------------
# either build or link against the included 
# libraries
#-------------------------------------------------

# start with an empty set of libs
LIBS = 

# add expat XML library
ifdef BUILD_EXPAT
CFLAGS += -I$(SRC)/lib/expat
EXPAT = $(OBJ)/libexpat.a
else
LIBS += -lexpat
EXPAT =
endif

# add ZLIB compression library
ifdef BUILD_ZLIB
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
# defines needed by multiple make files 
#-------------------------------------------------

BUILDSRC = $(SRC)/build
BUILDOBJ = $(OBJ)/build
BUILDOUT = $(BUILDOBJ)

#-------------------------------------------------
# include the various .mak files
#-------------------------------------------------

# mamep: must stay before MAME OSD
ifdef MAMEMESS
# include MESS core defines
include $(SRC)/mess/messcore.mak
endif

# include OSD-specific rules first
include $(SRC)/osd/$(OSD)/$(OSD).mak

ifdef MAMEMESS
include $(SRC)/mess/osd/$(OSD)/$(OSD).mak
endif

# then the various core pieces
include $(SRC)/$(TARGET)/$(SUBTARGET).mak
ifndef NEOCPSMAME
include $(SRC)/mess/mess.mak
endif
include $(SRC)/lib/lib.mak
include $(SRC)/build/build.mak
-include $(SRC)/osd/$(CROSS_BUILD_OSD)/build.mak
include $(SRC)/tools/tools.mak
# mamep: must stay at the end for png2bdc
include $(SRC)/emu/emu.mak

# combine the various definitions to one
CDEFS = $(DEFS) $(COREDEFS) $(CPUDEFS) $(SOUNDDEFS) $(ASMDEFS)



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
ifdef MAP
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

ifndef EXECUTABLE_DEFINED

ifeq ($(NO_DLL),)

$(EMULATORDLL): $(VERSIONOBJ) $(OBJ)/osd/windows/mamelib.o $(DRVLIBS) $(LIBOSD) $(MESSLIBOSD) $(LIBEMU) $(LIBCPU) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE)
# always recompile the version string
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) -shared $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@ $(MAPDLLFLAGS)

# gui target
$(EMULATORGUI):	$(EMULATORDLL) $(OBJ)/osd/winui/guimain.o $(GUIRESFILE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mwindows $(FULLNAME)lib.$(DLLLINK) $(OBJ)/osd/winui/guimain.o $(GUIRESFILE) $(LIBS) -o $@ $(MAPGUIFLAGS)

# cli target
$(EMULATORCLI):	$(EMULATORDLL) $(OBJ)/osd/windows/climain.o $(CLIRESFILE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $(FULLNAME)lib.$(DLLLINK) $(OBJ)/osd/windows/climain.o $(CLIRESFILE) $(LIBS) -o $@ $(MAPCLIFLAGS)
else
  ifdef WINUI
  # gui target
$(EMULATOR):	$(OBJ)/osd/winui/mui_main.o $(VERSIONOBJ) $(DRVLIBS) $(LIBOSD) $(GUIRESFILE) $(MESSLIBOSD) $(LIBEMU) $(LIBCPU) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE_NOMAIN)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mwindows $^ $(LIBS) -o $@ $(MAPFLAGS)
  else
# cli target
$(EMULATOR):	$(VERSIONOBJ) $(DRVLIBS) $(LIBOSD) $(CLIRESFILE) $(MESSLIBOSD) $(LIBEMU) $(LIBCPU) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $^ $(LIBS) -o $@ $(MAPFLAGS)
  endif
  endif
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
