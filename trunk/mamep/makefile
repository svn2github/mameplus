###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################



###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


include config.def

#-------------------------------------------------
# specify core target: mame, mess, tiny, etc.
# build rules will be included from $(TARGET).mak
#-------------------------------------------------

ifneq ($(HAZEMD),)
TARGET = hazemd
USE_DRIVER_SWITCH=
else
ifneq ($(NEOCPSMAME),)
TARGET = neocpsmame
else
ifeq ($(TARGET),)
TARGET = mame
endif
endif
endif


#-------------------------------------------------
# specify operating system: windows, msdos, etc.
# build rules will be includes from $(MAMEOS)/$(MAMEOS).mak
#-------------------------------------------------

ifeq ($(MAMEOS),)
MAMEOS = windows
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
# specify program options; see each option below 
# for details
#-------------------------------------------------

# uncomment next line to include the debugger
# DEBUG = 1

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
# ATHLON = 1
# I686 = 1
# P4 = 1
# PM = 1
# AMD64 = 1

# uncomment next line if you are building for a 64-bit target
# PTR64 = 1

# uncomment next line to build expat as part of MAME build
BUILD_EXPAT = 1

# uncomment next line to build zlib as part of MAME build
BUILD_ZLIB = 1

# uncomment next line to include the symbols
# SYMBOLS = 1

# uncomment next line to generate a link map for exception handling in windows
# MAP = 1


###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

# disable DRC cores for 64-bit builds
ifneq ($(PTR64),)
X86_MIPS3_DRC =
X86_PPC_DRC =
X86_VOODOO_DRC =
X86_ASM_68000 =
X86_ASM_68010 =
X86_ASM_68020 =
X86_M68K_DRC =
endif



#-------------------------------------------------
# platform-specific definitions
#-------------------------------------------------

# extension for executables
EXE = .exe

# compiler, linker and utilities
ifneq ($(USE_XGCC),)
    AR = @i686-pc-mingw32-ar
    CC = @i686-pc-mingw32-gcc
    XCC = @i686-pc-mingw32-gcc
    LD = @i686-pc-mingw32-gcc
else
    AR = @ar
    CC = @gcc
    XCC = @gcc
    LD = @gcc
endif
MD = -mkdir.exe
RM = @rm -f

WINDOWS_PROGRAM = -mwindows
CONSOLE_PROGRAM = -mconsole

ifneq ($(I686),)
    P6OPT = ppro
else
    P6OPT = notppro
endif



#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

ifeq ($(MAMEOS),msdos)
    PREFIX = d
endif

COMPILER_SUFFIX =
XEXTRA_SUFFIX = $(EXTRA_SUFFIX)

# by default, compile for Pentium target and add no suffix
ARCHSUFFIX =
ARCH = -march=pentium

ifneq ($(ATHLON),)
    ARCHSUFFIX = at
    ARCH = -march=athlon -m3dnow
endif

ifneq ($(ATHLONXP),)
    ARCHSUFFIX = ax
    ARCH = -march=athlon-xp -m3dnow -msse
endif

ifneq ($(I686),)
    ARCHSUFFIX = pp
    ARCH = -march=i686 -mmmx
endif

ifneq ($(P4),)
    ARCHSUFFIX = p4
    ARCH = -march=pentium4 -msse2
endif

ifneq ($(AMD64),)
    ARCHSUFFIX = 64
    ARCH = -march=athlon64
endif

ifneq ($(PM),)
    ARCHSUFFIX = pm
    ARCH = -march=pentiumm
endif

NAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)$(COMPILER_SUFFIX)
ifeq ($(NO_DLL),)
    LIBNAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)lib$(COMPILER_SUFFIX)
    GUINAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)gui$(COMPILER_SUFFIX)
endif

# debug builds just get the 'd' suffix and nothing more
ifneq ($(DEBUG),)
    NAME = $(PREFIX)$(TARGET)$(SUFFIX)$(XEXTRA_SUFFIX)d
    ifeq ($(NO_DLL),)
        LIBNAME = $(PREFIX)$(TARGET)$(SUFFIX)$(XEXTRA_SUFFIX)libd
        GUINAME = $(PREFIX)$(TARGET)$(SUFFIX)$(XEXTRA_SUFFIX)guid
    endif
endif


# build the targets in different object dirs, since mess changes
# some structures and thus they can't be linked against each other.
OBJ = obj/$(NAME)

ifneq ($(NO_DLL),)
    EMULATOR = $(NAME)$(EXE)
else
    EMULATORDLL = $(LIBNAME).dll
    EMULATORCLI = $(NAME)$(EXE)
    EMULATORGUI = $(GUINAME)$(EXE)
    EMULATOR    = $(EMULATORDLL) $(EMULATORCLI) $(EMULATORGUI)
endif



#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------

DEFS = -DX86_ASM -DLSB_FIRST -DINLINE="static __inline__" -Dasm=__asm__ -DCRLF=3 -DXML_STATIC -Drestrict=__restrict

ifneq ($(PTR64),)
DEFS += -DPTR64
endif

ifneq ($(DEBUG),)
DEFS += -DMAME_DEBUG
endif

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

ifneq ($(USE_NEOGEO_DEPRECATED),)
    DEFS+= -DUSE_NEOGEO_DEPRECATED
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

# Support Stick-type Pointing Device by miko2u@hotmail.com
ifneq ($(USE_JOY_MOUSE_MOVE),)
DEFS += -DUSE_JOY_MOUSE_MOVE
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
# compile and linking flags
#-------------------------------------------------

CFLAGS = -std=gnu89 -Isrc -Isrc/includes -Isrc/zlib -Isrc/$(MAMEOS) -I$(OBJ)/layout

ifneq ($(W_ERROR),)
    CFLAGS += -Werror
else
    CFLAGS += -Wno-error
endif

ifneq ($(SYMBOLS),)
    CFLAGS += -O0 -Wall -Wno-unused -g
else
    CFLAGS += -DNDEBUG \
	$(ARCH) -O3 -fno-strict-aliasing \
	-Wall \
	-Wno-sign-compare \
	-Wno-unused-functions \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wcast-align \
	-Wstrict-prototypes \
	-Wundef \
#	-Wformat-security \
	-Wwrite-strings \
	-Wdeclaration-after-statement
endif

ifneq ($(I686),)
# If you have a trouble in I686 build, try to remove a comment.
#    CFLAGS += -fno-builtin -fno-omit-frame-pointer 
endif

# extra options needed *only* for the osd files
CFLAGSOSDEPEND = $(CFLAGS)

LDFLAGS = -Lextra/lib

ifeq ($(SYMBOLS),)
    LDFLAGS += -s
endif

ifneq ($(MAP),)
    MAPFLAGS = -Wl,-Map,$(NAME).map
    MAPDLLFLAGS = -Wl,-Map,$(LIBNAME).map
    MAPCLIFLAGS = -Wl,-Map,$(NAME).map
    MAPGUIFLAGS = -Wl,-Map,$(GUINAME).map
else
    MAPFLAGS =
    MAPDLLFLAGS =
    MAPCLIFLAGS =
    MAPGUIFLAGS =
endif



#-------------------------------------------------
# define the dependency search paths
#-------------------------------------------------

VPATH = src $(wildcard src/cpu/*)



#-------------------------------------------------
# define the standard object directories
#-------------------------------------------------

OBJDIRS = \
	obj \
	$(OBJ) \
	$(OBJ)/cpu \
	$(OBJ)/sound \
	$(OBJ)/debug \
	$(OBJ)/drivers \
	$(OBJ)/layout \
	$(OBJ)/machine \
	$(OBJ)/sndhrdw \
	$(OBJ)/vidhrdw \
	$(OBJ)/$(MAMEOS) \

ifneq ($(MESS),)
OBJDIRS += 
	$(OBJ)/mess \
	$(OBJ)/mess/systems \
	$(OBJ)/mess/machine \
	$(OBJ)/mess/sndhrdw \
	$(OBJ)/mess/vidhrdw \
	$(OBJ)/mess/tools
endif



#-------------------------------------------------
# define standard libarires for CPU and sounds
#-------------------------------------------------

CPULIB = $(OBJ)/libcpu.a

SOUNDLIB = $(OBJ)/libsound.a



#-------------------------------------------------
# either build or link against the included 
# libraries
#-------------------------------------------------

# start with an empty set of libs
LIBS = 

# add expat XML library
ifneq ($(BUILD_EXPAT),)
CFLAGS += -Isrc/expat
OBJDIRS += $(OBJ)/expat
EXPAT = $(OBJ)/libexpat.a
#COREOBJS += $(EXPAT)
else
LIBS += -lexpat
EXPAT =
endif

# add ZLIB compression library
ifneq ($(BUILD_ZLIB),)
CFLAGS += -Isrc/zlib
OBJDIRS += $(OBJ)/zlib
ZLIB = $(OBJ)/libz.a
#COREOBJS += $(ZLIB)
else
LIBS += -lz
ZLIB =
endif



#-------------------------------------------------
# 'all' target needs to go here, before the 
# include files which define additional targets
#-------------------------------------------------

all: maketree emulator extra



#-------------------------------------------------
# include the various .mak files
#-------------------------------------------------

# include OS-specific rules first
include src/$(MAMEOS)/$(MAMEOS).mak

# then the various core pieces
include src/core.mak
include src/$(TARGET).mak
include src/cpu/cpu.mak
include src/sound/sound.mak

# combine the various definitions to one
CDEFS = $(DEFS) $(COREDEFS) $(CPUDEFS) $(SOUNDDEFS) $(ASMDEFS)

ifneq ($(BUILD_EXPAT),)
COREOBJS += $(EXPAT)
endif

ifneq ($(BUILD_ZLIB),)
COREOBJS += $(ZLIB)
endif

ifneq ($(NO_DLL),)
# do not use dllimport
    CDEFS += -DDONT_USE_DLL

    ifneq ($(WINUI),)
        OSOBJS += $(GUIOBJS)
        LIBS += $(GUILIBS)
    else
        OSOBJS += $(CLIOBJS)
        LIBS += $(CLILIBS)
    endif
endif


#-------------------------------------------------
# primary targets
#-------------------------------------------------

emulator: maketree $(EMULATOR)

extra: $(TOOLS)

maketree: $(sort $(OBJDIRS)) $(OSPREBUILD)

clean:
	@echo Deleting object tree $(OBJ)...
	$(RM) -r $(OBJ)
	@echo Deleting $(EMULATOR)...
	$(RM) $(EMULATOR)
	@echo Deleting $(TOOLS)...
	$(RM) $(TOOLS)



#-------------------------------------------------
# directory targets
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) $@



#-------------------------------------------------
# executable targets and dependencies
#-------------------------------------------------

ifneq ($(NO_DLL),)
    $(EMULATOR): $(COREOBJS) $(OSOBJS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS)
else
    $(EMULATORDLL): $(COREOBJS) $(OSOBJS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS)
endif

# always recompile the version string
ifneq ($(HAZEMD),)
	$(CC) $(CDEFS) $(CFLAGS) -c src/versionmd.c -o $(OBJ)/versionmd.o
else
	$(CC) $(CDEFS) $(CFLAGS) -c src/version.c -o $(OBJ)/version.o
endif
	@echo Linking $@...

ifneq ($(NO_DLL),)
    ifneq ($(WINUI),)
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(WINDOWS_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -o $@ $(MAPFLAGS)
    else
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -o $@ $(MAPFLAGS)
    endif

    ifneq ($(UPX),)
	upx -9 $(EMULATOR)
    endif

else
    # build DLL
	$(RM) $@
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) -shared -o $@ $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) $(MAPDLLFLAGS)
    ifneq ($(UPX),)
	upx -9 $@
    endif

    # gui target
    $(EMULATORGUI): $(EMULATORDLL) $(GUIOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $^ -o $@ $(GUILIBS) $(MAPGUIFLAGS)
    ifneq ($(UPX),)
	upx -9 $@
    endif

    # cli target
    $(EMULATORCLI): $(EMULATORDLL) $(CLIOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $^ -o $@ $(CLILIBS) $(MAPCLIFLAGS)
    ifneq ($(UPX),)
	upx -9 $@
    endif
endif

file2str$(EXE): $(OBJ)/file2str.o $(OSDBGOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@

romcmp$(EXE): $(OBJ)/romcmp.o $(OBJ)/unzip.o $(OBJ)/mamecore.o $(VCOBJS) $(ZLIB) $(OSDBGOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@

chdman$(EXE): $(OBJ)/chdman.o $(OBJ)/chd.o $(OBJ)/chdcd.o $(OBJ)/cdrom.o $(OBJ)/md5.o $(OBJ)/sha1.o $(OBJ)/version.o $(ZLIB) $(OSTOOLOBJS) $(OSDBGOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@

jedutil$(EXE): $(OBJ)/jedutil.o $(OBJ)/jedparse.o $(OSDBGOBJS)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# library targets and dependencies
#-------------------------------------------------

$(CPULIB): $(CPUOBJS)

ifneq ($(DEBUG),)
$(CPULIB): $(DBGOBJS)
endif

$(SOUNDLIB): $(SOUNDOBJS)

$(OBJ)/libexpat.a: $(OBJ)/expat/xmlparse.o $(OBJ)/expat/xmlrole.o $(OBJ)/expat/xmltok.o

$(OBJ)/libz.a: $(OBJ)/zlib/adler32.o $(OBJ)/zlib/compress.o $(OBJ)/zlib/crc32.o $(OBJ)/zlib/deflate.o \
				$(OBJ)/zlib/gzio.o $(OBJ)/zlib/inffast.o $(OBJ)/zlib/inflate.o $(OBJ)/zlib/infback.o \
				$(OBJ)/zlib/inftrees.o $(OBJ)/zlib/trees.o $(OBJ)/zlib/uncompr.o $(OBJ)/zlib/zutil.o



#-------------------------------------------------
# generic rules
#-------------------------------------------------

$(OBJ)/$(MAMEOS)/%.o: src/$(MAMEOS)/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@

$(OBJ)/%.o: src/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.pp: src/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: src/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(OBJ)/%.lh: src/%.lay file2str$(EXE)
	@echo Converting $<...
	@file2str$(EXE) $< $@ layout_$(basename $(notdir $<))

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) -cr $@ $^

%$(EXE):
	@echo Linking $@...
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -o $@
