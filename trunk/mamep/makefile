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

ifndef SUBTARGET
SUBTARGET = $(TARGET)
endif



#-------------------------------------------------
# specify OSD layer: windows, sdl, etc.
# build rules will be included from
# src/osd/$(OSD)/$(OSD).mak
#-------------------------------------------------

ifndef OSD
ifeq ($(OS),Windows_NT)
OSD = windows
TARGETOS = win32
else
OSD = sdl
endif
endif

ifndef CROSS_BUILD_OSD
CROSS_BUILD_OSD = $(OSD)
endif



#-------------------------------------------------
# specify OS target, which further differentiates
# the underlying OS; supported values are:
# win32, unix, macosx, os2
#-------------------------------------------------

ifndef TARGETOS

ifeq ($(OS),Windows_NT)
TARGETOS = win32
else

ifneq ($(CROSS_BUILD),1)

ifneq ($(OS2_SHELL),)
TARGETOS = os2
else

UNAME = $(shell uname -mps)

ifeq ($(firstword $(filter Linux,$(UNAME))),Linux)
TARGETOS = linux
endif
ifeq ($(firstword $(filter Solaris,$(UNAME))),Solaris)
TARGETOS = solaris
endif
ifeq ($(firstword $(filter FreeBSD,$(UNAME))),FreeBSD)
TARGETOS = freebsd
endif
ifeq ($(firstword $(filter GNU/kFreeBSD,$(UNAME))),GNU/kFreeBSD)
TARGETOS = freebsd
endif
ifeq ($(firstword $(filter NetBSD,$(UNAME))),NetBSD)
TARGETOS = netbsd
endif
ifeq ($(firstword $(filter OpenBSD,$(UNAME))),OpenBSD)
TARGETOS = openbsd
endif
ifeq ($(firstword $(filter Darwin,$(UNAME))),Darwin)
TARGETOS = macosx
endif
ifeq ($(firstword $(filter Haiku,$(UNAME))),Haiku)
TARGETOS = haiku
endif

ifndef TARGETOS
$(error Unable to detect TARGETOS from uname -a: $(UNAME))
endif

# Autodetect PTR64
ifndef PTR64
ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
PTR64 = 1
endif
ifeq ($(firstword $(filter amd64,$(UNAME))),amd64)
PTR64 = 1
endif
ifeq ($(firstword $(filter ppc64,$(UNAME))),ppc64)
PTR64 = 1
endif
endif

# Autodetect BIGENDIAN
# MacOSX
ifndef BIGENDIAN
ifneq (,$(findstring Power,$(UNAME)))
BIGENDIAN=1
endif
# Linux
ifneq (,$(findstring ppc,$(UNAME)))
BIGENDIAN=1
endif
endif # BIGENDIAN

endif # OS/2
endif # CROSS_BUILD
endif # Windows_NT

endif # TARGET_OS


ifeq ($(TARGETOS),win32)

# Autodetect PTR64
ifndef PTR64
WIN_TEST_GCC := $(shell gcc --version)
ifeq ($(findstring x86_64,$(WIN_TEST_GCC)),x86_64)
	PTR64=1
endif
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
#   ARCHOPTS = -march=pentiumpro  # optimize for I686
#   ARCHOPTS = -march=core2       # optimize for Core 2
#   ARCHOPTS = -march=native      # optimize for local machine (auto detect)
#   ARCHOPTS = -mcpu=G4           # optimize for G4
# note that we leave this commented by default so that you can
# configure this in your environment and never have to think about it
# ARCHOPTS =



#-------------------------------------------------
# specify program options; see each option below
# for details
#-------------------------------------------------

# uncomment next line to build a debug version
# DEBUG = 1

# uncomment next line to disable some debug-related hotspots/slowdowns (e.g. for profiling)
# FASTDEBUG = 1

# uncomment next line to include the internal profiler
# PROFILER = 1

# uncomment the force the universal DRC to always use the C backend
# you may need to do this if your target architecture does not have
# a native backend
# FORCE_DRC_C_BACKEND = 1

# uncomment next line to build using unix-style libsdl on Mac OS X
# (vs. the native framework port).  Normal users should not enable this.
# MACOSX_USE_LIBSDL = 1

# uncomment and specify path to cppcheck executable to perform
# static code analysis during compilation
# CPPCHECK =



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

# uncomment next line to build libflac as part of MAME build
BUILD_FLAC = 1

# uncomment next line to build jpeglib as part of MAME build
BUILD_JPEGLIB = 1

# uncomment next line to build libsqlite3 as part of MAME/MESS build
BUILD_SQLITE3 = 1

# uncomment next line to build PortMidi as part of MAME/MESS build
BUILD_MIDILIB = 1

# uncomment next line to include the symbols
# SYMBOLS = 1

# specify symbols level or leave commented to use the default
# (default is SYMLEVEL = 2 normally; use 1 if you only need backtrace)
# SYMLEVEL = 2

# uncomment next line to dump the symbols to a .sym file
# DUMPSYM = 1

# uncomment next line to include profiling information from the compiler
# PROFILE = 1

# uncomment next line to generate a link map for exception handling in windows
# MAP = 1

# uncomment next line to generate verbose build information
# VERBOSE = 1

# uncomment next line to generate deprecation warnings during compilation
# DEPRECATED = 1

# specify the sanitizer to use or leave empty to use none
# SANITIZE =

# uncomment next line to enable LTO (link-time optimizations)
# LTO = 1

# uncomment next line to disable networking
# DONT_USE_NETWORK = 1

# uncomment to enable SSE2 optimized code and SSE2 code generation (implicitly enabled by 64-bit compilers)
# SSE2 = 1

# uncomment to enable OpenMP optimized code
# OPENMP = 1

# specify optimization level or leave commented to use the default
# (default is OPTIMIZE = 3 normally, or OPTIMIZE = 0 with symbols)
# OPTIMIZE = 3


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

# enable symbols as it is useless without them
ifdef SANITIZE
SYMBOLS = 1
endif

# profiler defaults to on for DEBUG builds
ifdef DEBUG
ifndef PROFILER
PROFILER = 1
endif
endif

# allow gprof profiling as well, which overrides the internal PROFILER
# also enable symbols as it is useless without them
ifdef PROFILE
PROFILER =
SYMBOLS = 1
ifndef SYMLEVEL
SYMLEVEL = 1
endif
endif

# specify a default optimization level if none explicitly stated
ifndef OPTIMIZE
ifndef SYMBOLS
OPTIMIZE = 3
else
OPTIMIZE = 0
endif
endif

# set the symbols level
ifdef SYMBOLS
ifndef SYMLEVEL
SYMLEVEL = 2
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

# extension for build tools
BUILD_EXE = 

ifeq ($(OS),Windows_NT)
BUILD_EXE = .exe
endif
ifneq ($(OS2_SHELL),)
BUILD_EXE = .exe
endif

# compiler, linker and utilities
ifneq ($(TARGETOS),emscripten)
AR = @ar
CC = @gcc
LD = @g++
endif
MD = -mkdir$(BUILD_EXE)
RM = @rm -f
OBJDUMP = @objdump
PYTHON = @python


#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

# reset all internal prefixes/suffixes
PREFIXSDL =
SUFFIX64 =
SUFFIXDEBUG =
SUFFIXPROFILE =

# Windows SDL builds get an SDL prefix
ifeq ($(OSD),sdl)
ifeq ($(TARGETOS),win32)
PREFIXSDL = sdl
endif
endif

ifeq ($(OSD),osdmini)
PREFIXSDL = mini
endif

# 64-bit builds get a '64' suffix
ifeq ($(PTR64),1)
SUFFIX64 = 64
endif

# debug builds just get the 'd' suffix and nothing more
ifdef DEBUG
SUFFIXDEBUG = d
endif

# gprof builds get an addition 'p' suffix
ifdef PROFILE
SUFFIXPROFILE = p
endif

# the name is just 'target' if no subtarget; otherwise it is
# the concatenation of the two (e.g., mametiny)
ifeq ($(TARGET),$(SUBTARGET))
NAME = $(TARGET)$(EXTRA_SUFFIX)
else
NAME = $(TARGET)$(EXTRA_SUFFIX)$(SUBTARGET)
endif

# fullname is prefix+name+suffix+suffix64+suffixdebug
FULLNAME ?= $(PREFIX)$(PREFIXSDL)$(NAME)$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXPROFILE)
ifneq ($(WINUI),)
ifeq ($(TARGET),$(SUBTARGET))
MAMEUINAME = $(TARGET)$(EXTRA_SUFFIX)ui
else
MAMEUINAME = $(TARGET)$(EXTRA_SUFFIX)$(SUBTARGET)ui
endif
MAMEUIEXE = $(PREFIX)$(PREFIXSDL)$(MAMEUINAME)$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXPROFILE)$(EXE)
BUILD += $(MAMEUIEXE)
endif

# add an EXE suffix to get the final emulator name
EMULATOR = $(FULLNAME)$(EXE)



#-------------------------------------------------
# source and object locations
#-------------------------------------------------

# all sources are under the src/ directory
SRC = src

# build the targets in different object dirs, so they can co-exist
OBJ = obj/$(OSD)/$(FULLNAME)

ifeq ($(CROSS_BUILD),1)
ifndef NATIVE_OBJ
NATIVE_OBJ = OBJ
endif # NATIVE_OBJ
endif # CROSS_BUILD


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
DEFS += -DINLINE="static inline"
endif

# define LSB_FIRST if we are a little-endian target
ifndef BIGENDIAN
DEFS += -DLSB_FIRST
endif

# define PTR64 if we are a 64-bit target
ifeq ($(PTR64),1)
DEFS += -DPTR64
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

# define USE_NETWORK if networking is enabled (not OS/2 and hasn't been disabled)
ifneq ($(TARGETOS),os2)
ifndef DONT_USE_NETWORK
DEFS += -DUSE_NETWORK
endif
endif

# need to ensure FLAC functions are statically linked
ifeq ($(BUILD_FLAC),1)
DEFS += -DFLAC__NO_DLL
endif

# define USE_SYSTEM_JPEGLIB if library shipped with MAME is not used
ifneq ($(BUILD_JPEGLIB),1)
DEFS += -DUSE_SYSTEM_JPEGLIB
endif

ifdef FASTDEBUG
DEFS += -DMAME_DEBUG_FAST
endif

ifneq ($(USE_UI_COLOR_DISPLAY),)
DEFS += -DUI_COLOR_DISPLAY
endif

ifneq ($(USE_TRANS_UI),)
DEFS += -DTRANS_UI
endif

ifneq ($(USE_JOYSTICK_ID),)
DEFS += -DJOYSTICK_ID
endif

ifneq ($(USE_AUTOFIRE),)
DEFS += -DUSE_AUTOFIRE
endif

ifneq ($(USE_CUSTOM_BUTTON),)
DEFS += -DUSE_CUSTOM_BUTTON
endif

ifneq ($(USE_INP_CAPTION),)
DEFS += -DINP_CAPTION
endif

ifneq ($(USE_SHOW_TIME),)
DEFS += -DUSE_SHOW_TIME
endif

ifneq ($(USE_SHOW_INPUT_LOG),)
DEFS += -DUSE_SHOW_INPUT_LOG
endif

ifneq ($(USE_PLAYBACK_END_PAUSE),)
DEFS += -DPLAYBACK_END_PAUSE
endif

ifneq ($(USE_AUDIO_SYNC),)
DEFS += -DUSE_AUDIO_SYNC
endif

ifneq ($(USE_IPS),)
DEFS += -DUSE_IPS
endif

ifeq ($(SUBTARGET),tiny)
DEFS += -DTINY_BUILD
endif

ifneq ($(USE_DRIVER_SWITCH),)
DEFS += -DDRIVER_SWITCH
endif

ifneq ($(USE_CMD_LIST),)
DEFS += -DCMD_LIST
endif

ifneq ($(USE_HISCORE),)
DEFS += -DUSE_HISCORE
endif

#-------------------------------------------------
# compile flags
# CCOMFLAGS are common flags
# CONLYFLAGS are flags only used when compiling for C
# CPPONLYFLAGS are flags only used when compiling for C++
# COBJFLAGS are flags only used when compiling for Objective-C(++)
#-------------------------------------------------

# start with empties for everything
CCOMFLAGS =
CONLYFLAGS =
COBJFLAGS =
CPPONLYFLAGS =

# CFLAGS is defined based on C or C++ targets
# (remember, expansion only happens when used, so doing it here is ok)
CFLAGS = $(CCOMFLAGS) $(CPPONLYFLAGS)

# we compile C-only to C89 standard with GNU extensions
# we compile C++ code to C++98 standard with GNU extensions
CONLYFLAGS += -std=gnu89
CPPONLYFLAGS += -x c++ -std=gnu++98
COBJFLAGS += -x objective-c++

# this speeds it up a bit by piping between the preprocessor/compiler/assembler
CCOMFLAGS += -pipe

# add -g if we need symbols, and ensure we have frame pointers
ifdef SYMBOLS
CCOMFLAGS += -g$(SYMLEVEL) -fno-omit-frame-pointer -fno-optimize-sibling-calls
endif

# we need to disable some additional implicit optimizations for profiling
ifdef PROFILE
CCOMFLAGS += -mno-omit-leaf-frame-pointer
endif

# add -v if we need verbose build information
ifdef VERBOSE
CCOMFLAGS += -v
endif

# only show deprecation warnings when enabled
ifndef DEPRECATED
CCOMFLAGS += \
	-Wno-deprecated-declarations
endif

# add profiling information for the compiler
ifdef PROFILE
CCOMFLAGS += -pg
endif

# add the optimization flag
CCOMFLAGS += -O$(OPTIMIZE)

# add the error warning flag
ifndef NOWERROR
CCOMFLAGS += -Werror
endif

# if we are optimizing, include optimization options
ifneq ($(OPTIMIZE),0)
CCOMFLAGS += -fno-strict-aliasing $(ARCHOPTS)
ifdef LTO
CCOMFLAGS += -flto
endif
endif

ifdef SSE2
CCOMFLAGS += -msse2
endif

ifdef OPENMP
CCOMFLAGS += -fopenmp
else
CCOMFLAGS += -Wno-unknown-pragmas
endif

# add a basic set of warnings
CCOMFLAGS += \
	-Wall \
	-Wcast-align \
	-Wundef \
	-Wno-format-security \
	-Wno-unused-but-set-variable \
	-Wno-int-to-pointer-cast \
	-Wwrite-strings \
	-Wno-sign-compare \
	-Wno-conversion

# warnings only applicable to C compiles
CONLYFLAGS += \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wstrict-prototypes

# warnings only applicable to OBJ-C compiles
COBJFLAGS += \
	-Wpointer-arith 

# warnings only applicable to C++ compiles
CPPONLYFLAGS += \
	-Woverloaded-virtual

include $(SRC)/build/cc_detection.mak

ifdef SANITIZE
CCOMFLAGS += -fsanitize=$(SANITIZE)
ifneq (,$(findstring thread,$(SANITIZE)))
CCOMFLAGS += -fPIE
endif
ifneq (,$(findstring memory,$(SANITIZE)))
ifneq (,$(findstring clang,$(CC)))
CCOMFLAGS += -fsanitize-memory-track-origins -fPIE
endif
endif
ifneq (,$(findstring undefined,$(SANITIZE)))
ifneq (,$(findstring clang,$(CC)))
# TODO: check if linker is clang++
# produces a lot of messages - disable it for now
CCOMFLAGS += -fno-sanitize=alignment
# these are false positives because of the way our delegates work
CCOMFLAGS += -fno-sanitize=function
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=shift
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/hd6309.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=object-size
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c, src/emu/cpu/m6809/konami.c, src/emu/cpu/m6809/hd6309.c, src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=vptr
# clang takes forever to compile src/emu/video/psx.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=null
# clang takes forever to compile src/emu/cpu/tms57002/tms57002.c when this isn't disabled
CCOMFLAGS += -fno-sanitize=signed-integer-overflow
endif
endif
endif

#-------------------------------------------------
# include paths
#-------------------------------------------------

# add core include paths
INCPATH += \
	-I$(SRC)/$(TARGET) \
	-I$(OBJ)/$(TARGET)/layout \
	-I$(SRC)/emu \
	-I$(OBJ)/emu \
	-I$(OBJ)/emu/layout \
	-I$(SRC)/lib/util \
	-I$(SRC)/lib \
	-I$(SRC)/osd \
	-I$(SRC)/osd/$(OSD) \



#-------------------------------------------------
# archiving flags
#-------------------------------------------------
# Default to something reasonable for all platforms
ARFLAGS = -cr
# Deal with macosx brain damage if COMMAND_MODE is in
# the luser's environment:
ifeq ($(TARGETOS),macosx)
ifeq ($(COMMAND_MODE),"legacy")
ARFLAGS = -crs
endif
endif
ifeq ($(TARGETOS),emscripten)
ARFLAGS = cr
endif


#-------------------------------------------------
# linking flags
#-------------------------------------------------

# LDFLAGS are used generally; LDFLAGSEMULATOR are additional
# flags only used when linking the core emulator
LDFLAGS =
ifneq ($(TARGETOS),macosx)
ifneq ($(TARGETOS),os2)
ifneq ($(TARGETOS),solaris)
LDFLAGS = -Wl,--warn-common
endif
endif
endif
LDFLAGSEMULATOR =

# add profiling information for the linker
ifdef PROFILE
LDFLAGS += -pg
endif

# strip symbols and other metadata in non-symbols and non profiling builds
ifndef SYMBOLS
ifneq ($(TARGETOS),macosx)
LDFLAGS += -s
endif
endif

ifneq ($(OPTIMIZE),0)
ifdef LTO
LDFLAGS += -flto
endif
endif

# output a map file (emulator only)
ifdef MAP
LDFLAGSEMULATOR += -Wl,-Map,$(FULLNAME).map
endif

ifdef SANITIZE
LDFLAGS += -fsanitize=$(SANITIZE)
ifneq (,$(findstring thread,$(SANITIZE)))
LDFLAGS += -pie
endif
ifneq (,$(findstring memory,$(SANITIZE)))
LDFLAGS += -pie
endif
endif


#-------------------------------------------------
# define the standard object directory; other
# projects can add their object directories to
# this variable
#-------------------------------------------------

OBJDIRS = $(OBJ)


#-------------------------------------------------
# define standard libraries for CPU and sounds
#-------------------------------------------------

LIBEMU = $(OBJ)/libemu.a
LIBOPTIONAL = $(OBJ)/liboptional.a
LIBBUS = $(OBJ)/libbus.a
LIBDASM = $(OBJ)/libdasm.a
LIBUTIL = $(OBJ)/libutil.a
LIBOCORE = $(OBJ)/libocore.a
LIBOSD = $(OBJ)/libosd.a

VERSIONOBJ = $(OBJ)/version.o
EMUINFOOBJ = $(OBJ)/$(TARGET)/$(TARGET).o
DRIVLISTSRC = $(OBJ)/$(TARGET)/drivlist.c
DRIVLISTOBJ = $(OBJ)/$(TARGET)/drivlist.o



#-------------------------------------------------
# either build or link against the included
# libraries
#-------------------------------------------------

# start with an empty set of libs
LIBS = 

# add expat XML library
ifeq ($(BUILD_EXPAT),1)
INCPATH += -I$(SRC)/lib/expat
EXPAT = $(OBJ)/libexpat.a
else
LIBS += -lexpat
EXPAT =
endif

# add ZLIB compression library
ifeq ($(BUILD_ZLIB),1)
INCPATH += -I$(SRC)/lib/zlib
ZLIB = $(OBJ)/libz.a
else
LIBS += -lz
ZLIB =
endif

# add flac library
ifeq ($(BUILD_FLAC),1)
INCPATH += -I$(SRC)/lib/util
FLAC_LIB = $(OBJ)/libflac.a
# $(OBJ)/libflac++.a
else
LIBS += -lFLAC
FLAC_LIB =
endif

# add jpeglib image library
ifeq ($(BUILD_JPEGLIB),1)
INCPATH += -I$(SRC)/lib/libjpeg
JPEG_LIB = $(OBJ)/libjpeg.a
else
LIBS += -ljpeg
JPEG_LIB =
endif

# add SoftFloat floating point emulation library
SOFTFLOAT = $(OBJ)/libsoftfloat.a

# add formats emulation library
FORMATS_LIB = $(OBJ)/libformats.a

# add LUA library
LUA_LIB = $(OBJ)/liblua.a

# add web library
WEB_LIB = $(OBJ)/libweb.a

# add SQLite3 library
ifeq ($(BUILD_SQLITE3),1)
SQLITE3_LIB = $(OBJ)/libsqlite3.a
else
LIBS += -lsqlite3
SQLITE3_LIB =
endif

# add PortMidi MIDI library
ifeq ($(BUILD_MIDILIB),1)
INCPATH += -I$(SRC)/lib/portmidi
MIDI_LIB = $(OBJ)/libportmidi.a
else
LIBS += -lportmidi
MIDI_LIB =
endif

ifneq (,$(findstring clang,$(CC)))
LIBS += -lstdc++ -lpthread
endif

#-------------------------------------------------
# 'default' target needs to go here, before the
# include files which define additional targets
#-------------------------------------------------

default: maketree buildtools emulator

all: default tools

7Z_LIB = $(OBJ)/lib7z.a

#-------------------------------------------------
# defines needed by multiple make files
#-------------------------------------------------

BUILDSRC = $(SRC)/build
BUILDOBJ = $(OBJ)/build
BUILDOUT = $(BUILDOBJ)

ifdef NATIVE_OBJ
BUILDOUT = $(NATIVE_OBJ)/build
endif # NATIVE_OBJ


#-------------------------------------------------
# include the various .mak files
#-------------------------------------------------

# include OSD-specific rules first
include $(SRC)/osd/$(OSD)/$(OSD).mak

# then the various core pieces
include $(SRC)/build/build.mak
include $(SRC)/$(TARGET)/$(SUBTARGET).mak
-include $(SRC)/$(TARGET)/osd/$(OSD)/$(OSD).mak
ifneq ($(MAMEMESS),)
include $(SRC)/mess/mess.mak
endif
include $(SRC)/emu/emu.mak
include $(SRC)/lib/lib.mak
-include $(SRC)/osd/$(CROSS_BUILD_OSD)/build.mak
include $(SRC)/tools/tools.mak
include $(SRC)/regtests/regtests.mak

# combine the various definitions to one
CCOMFLAGS += $(INCPATH)
CDEFS = $(DEFS)

# TODO: -x c++ should not be hard-coded
CPPCHECKFLAGS = $(CDEFS) $(INCPATH) -x c++ --enable=style

#-------------------------------------------------
# sanity check OSD additions
#-------------------------------------------------

ifeq (,$(findstring -DOSD_,$(CDEFS)))
$(error $(OSD).mak should have defined -DOSD_)
endif

#-------------------------------------------------
# primary targets
#-------------------------------------------------

emulator: maketree $(BUILD) $(EMULATOR)

buildtools: maketree $(BUILD)

tools: maketree $(TOOLS)

maketree: $(sort $(OBJDIRS))

clean: $(OSDCLEAN)
	@echo Deleting object tree $(OBJ)...
	$(RM) -r $(OBJ)
	@echo Deleting $(EMULATOR)...
	$(RM) $(EMULATOR)
	@echo Deleting $(TOOLS)...
	$(RM) $(TOOLS)
	@echo Deleting dependencies...
	$(RM) depend_emu.mak
	$(RM) depend_mame.mak
	$(RM) depend_mess.mak
	$(RM) depend_ume.mak
ifdef MAP
	@echo Deleting $(FULLNAME).map...
	$(RM) $(FULLNAME).map
endif
ifdef SYMBOLS
	@echo Deleting $(FULLNAME).sym...
	$(RM) $(FULLNAME).sym
endif

checkautodetect:
	@echo TARGETOS=$(TARGETOS)
	@echo PTR64=$(PTR64)
	@echo BIGENDIAN=$(BIGENDIAN)
	@echo UNAME="$(UNAME)"

tests: $(REGTESTS)

mak: maketree $(MAKEMAK_TARGET)
	@echo Rebuilding $(SUBTARGET).mak...
	$(MAKEMAK) $(SRC)/targets/$(SUBTARGET).lst -I. -I$(SRC)/emu -I$(SRC)/mame -I$(SRC)/mame/layout -I$(SRC)/mess -I$(SRC)/mess/layout $(SRC) > $(SUBTARGET).mak
	$(MAKEMAK) $(SRC)/targets/$(SUBTARGET).lst > $(SUBTARGET).lst

#-------------------------------------------------
# directory targets
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) -p $@



#-------------------------------------------------
# executable targets and dependencies
#-------------------------------------------------

ifndef EXECUTABLE_DEFINED

# always recompile the version string
$(VERSIONOBJ): $(EMUINFOOBJ) $(DRVLIBS) $(LIBOSD) $(LIBBUS) $(LIBOPTIONAL) $(LIBEMU) $(LIBUTIL) $(EXPAT) $(ZLIB) $(SOFTFLOAT) $(JPEG_LIB) $(FLAC_LIB) $(7Z_LIB) $(LUA_LIB) $(SQLITE3_LIB) $(FORMATS_LIB) $(LIBOCORE) $(MIDI_LIB) $(RESFILE)

$(EMULATOR): $(EMUINFOOBJ) $(DRIVLISTOBJ) $(DRVLIBS) $(LIBOSD) $(LIBBUS) $(LIBOPTIONAL) $(LIBEMU) $(LIBDASM) $(LIBUTIL) $(EXPAT) $(SOFTFLOAT) $(JPEG_LIB) $(FLAC_LIB) $(7Z_LIB) $(FORMATS_LIB) $(LUA_LIB) $(SQLITE3_LIB) $(WEB_LIB) $(ZLIB) $(LIBOCORE) $(MIDI_LIB) $(RESFILE) $(CLIRESFILE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
ifeq ($(TARGETOS),emscripten)
# Emscripten's linker seems to be stricter about the ordering of .a files
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $(VERSIONOBJ) -Wl,--start-group $^ -Wl,--end-group $(LIBS) -o $@
else
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mconsole $(VERSIONOBJ) $^ $(LIBS) -o $@
endif

ifneq ($(WINUI),)
$(MAMEUIEXE): $(EMUINFOOBJ) $(DRIVLISTOBJ) $(DRVLIBS) $(LIBOSD) $(LIBBUS) $(LIBOPTIONAL) $(LIBEMU) $(LIBDASM) $(LIBUTIL) $(EXPAT) $(SOFTFLOAT) $(JPEG_LIB) $(FLAC_LIB) $(7Z_LIB) $(FORMATS_LIB) $(LUA_LIB) $(SQLITE3_LIB) $(WEB_LIB) $(ZLIB) $(LIBOCORE_NOMAIN) $(MIDI_LIB) $(RESFILE) $(GUIRESFILE)
	$(CC) $(CDEFS) $(CFLAGS) -c $(SRC)/version.c -o $(VERSIONOBJ)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) -mwindows $(VERSIONOBJ) $^ $(LIBS) -o $@
endif

ifeq ($(TARGETOS),win32)
ifdef SYMBOLS
ifndef MSVC_BUILD
	$(OBJDUMP) --section=.text --line-numbers --syms --demangle $@ >$(FULLNAME).sym
endif
endif
endif

endif



#-------------------------------------------------
# generic rules
#-------------------------------------------------

ifneq ($(MAMEMESS),)
$(OBJ)/mess/%.o: $(SRC)/mess/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/mess/devices/%.o: $(SRC)/mess/devices/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/mess/drivers/%.o: $(SRC)/mess/drivers/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/mess/osd/windows/%.o: $(SRC)/mess/osd/windows/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) -DMESS $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif
endif

$(OBJ)/%.o: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/%.o: $(OBJ)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/%.pp: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/%.s: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(OBJ)/%.lh: $(SRC)/%.lay $(SRC)/build/file2str.py
	@echo Converting $<...
	@$(PYTHON) $(SRC)/build/file2str.py $< $@ layout_$(basename $(notdir $<))

$(OBJ)/%.fh: $(OBJ)/%.bdc $(SRC)/build/file2str.py
	@echo Converting $<...
	@$(PYTHON) $(SRC)/build/file2str.py $< $@ font_$(basename $(notdir $<)) UINT8

$(DRIVLISTOBJ): $(DRIVLISTSRC)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

$(DRIVLISTSRC): $(DRVLIST) $(MAKELIST_TARGET)
	@echo Building driver list $<...
	@$(MAKELIST) $^ >$@

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $^

ifeq ($(TARGETOS),macosx)
$(OBJ)/%.o: $(SRC)/%.m | $(OSPREBUILD)
	@echo Objective-C compiling $<...
	$(CC) $(CDEFS) $(COBJFLAGS) $(CCOMFLAGS) -c $< -o $@
endif



#-------------------------------------------------
# embedded font
#-------------------------------------------------

$(EMUOBJ)/uismall11.bdc: $(PNG2BDC_TARGET) \
		$(SRC)/emu/font/uismall.png \
		$(SRC)/emu/font/cp1250.png
	@echo Generating $@...
	@$^ $@

$(EMUOBJ)/uismall14.bdc: $(PNG2BDC_TARGET) \
		$(SRC)/emu/font/cp1252.png \
		$(SRC)/emu/font/cp932.png \
		$(SRC)/emu/font/cp932hw.png \
		$(SRC)/emu/font/cp936.png \
		$(SRC)/emu/font/cp949.png \
		$(SRC)/emu/font/cp950.png
	@echo Generating $@...
	@$^ $@

$(EMUOBJ)/uicmd11.bdc: $(PNG2BDC_TARGET) $(SRC)/emu/font/cmd11.png
	@echo Generating $@...
	@$^ $@

$(EMUOBJ)/uicmd14.bdc: $(PNG2BDC_TARGET) $(SRC)/emu/font/cmd14.png
	@echo Generating $@...
	@$^ $@



#-------------------------------------------------
# optional dependencies file
#-------------------------------------------------

-include depend_emu.mak
-include depend_$(TARGET).mak
