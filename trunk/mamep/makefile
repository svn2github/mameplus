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

TARGET = mame



#-------------------------------------------------
# specify operating system: windows, msdos, etc.
# build rules will be includes from $(MAMEOS)/$(MAMEOS).mak
#-------------------------------------------------

MAMEOS = windows



# select compiler
# USE_GCC = 1
# USE_VC = 1
# INTEL = 1
# if compiler is not selected, GCC is used as the default.
ifndef USE_VC
    ifndef USE_GCC
        USE_GCC = 1
    endif
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

ifdef USE_VC
# uncomment one of the next lines to use Whole Program Optimization
# USE_IPO = 1
endif

# uncomment next line to include the debugger
# DEBUG = 1

# uncomment next line to use the new multiwindow debugger
NEW_DEBUGGER = 1

# uncomment next line to use the new rendering system
# NEW_RENDER = 1

# uncomment next line to use Assembler 68000 engine
# X86_ASM_68000 = 1

# uncomment next line to use Assembler 68020 engine
# X86_ASM_68020 = 1

# uncomment next line to use DRC 68K engine
# X86_M68K_DRC = 1

# uncomment next line to use DRC MIPS3 engine
X86_MIPS3_DRC = 1

# uncomment next line to use DRC PowerPC engine
X86_PPC_DRC = 1

# uncomment next line to use DRC Voodoo rasterizers
# X86_VOODOO_DRC = 1



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


# uncomment next line to use cygwin compiler
# COMPILESYSTEM_CYGWIN	= 1

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
# platform-specific definitions
#-------------------------------------------------

# extension for executables
EXE = .exe

# CPU core include paths
VPATH=src $(wildcard src/cpu/*)

# compiler, linker and utilities
ifdef USE_GCC
    ifndef USE_XGCC
        AR = @ar
        CC = @gcc
        XCC = @gcc
        LD = @gcc
        DLLWRAP = @dllwrap
    else
        AR = @i686-pc-mingw32-ar
        CC = @i686-pc-mingw32-gcc
        XCC = @i686-pc-mingw32-gcc
        LD = @i686-pc-mingw32-gcc
        DLLWRAP = @i686-pc-mingw32-dllwrap
    endif
else
    ifdef INTEL
        AR = @xilib
        CC = @icl
        LD = @xilink
    else
        AR = @lib
        CC = @cl
        LD = @link
    endif
endif

ASM = @nasm
ASMFLAGS = -f coff
MD = -mkdir.exe
RM = @rm -f

ifdef USE_GCC
    WINDOWS_PROGRAM = -mwindows
    CONSOLE_PROGRAM = -mconsole
else
    WINDOWS_PROGRAM = -subsystem:windows
    CONSOLE_PROGRAM = -subsystem:console
endif

ifdef I686
    P6OPT = ppro
else
    P6OPT = notppro
endif



#-------------------------------------------------
# form the name of the executable
#-------------------------------------------------

ifeq ($(MAMEOS),msdos)
    PREFIX = d
else
    PREFIX =
endif
ifdef X86_VOODOO_DRC
DEFS += -DVOODOO_DRC
endif

ifdef USE_GCC
    COMPILER_SUFFIX =
    XEXTRA_SUFFIX = $(EXTRA_SUFFIX)

    # by default, compile for Pentium target and add no suffix
    ARCHSUFFIX =
    ARCH = -march=pentium

    ifdef AMD64
        ARCHSUFFIX = 64
        ARCH = -march=athlon64
    endif

    ifdef ATHLON
        ARCHSUFFIX = at
        ARCH = -march=athlon -m3dnow
    endif

    ifdef ATHLONXP
        ARCHSUFFIX = ax
        ARCH = -march=athlon-xp -m3dnow -msse
    endif

    ifdef I686
        ARCHSUFFIX = pp
        ARCH = -march=i686 -mmmx
    endif

    ifdef P4
        ARCHSUFFIX = p4
        ARCH = -march=pentium4 -msse2
    endif

    ifdef PM
        ARCHSUFFIX = pm
        ARCH = -march=pentium3 -msse2
    endif
else
    ifdef INTEL
        COMPILER_SUFFIX = -icc
    else
        COMPILER_SUFFIX = -vc
    endif

    XEXTRA_SUFFIX = $(EXTRA_SUFFIX)

    # by default, compile for Pentium target and add no suffix
    ARCHSUFFIX =
    ARCH = -G5

    ifdef I686
        ARCHSUFFIX = pp
        ARCH = -G6
    endif

    ifdef P4
        ARCHSUFFIX = p4
        ARCH = -G7
        ifdef INTEL
            ARCH += -QxN
        else
            ARCH += -arch:SSE2
        endif
    endif

    ifdef PM
        ARCHSUFFIX = pm
        ARCH = -G6
        ifdef INTEL
            ARCH += -QxB
        else
            ARCH += -arch:SSE2
        endif
    endif
endif

NAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)$(COMPILER_SUFFIX)
ifeq ($(NO_DLL),)
    LIBNAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)lib$(COMPILER_SUFFIX)
    GUINAME = $(PREFIX)$(TARGET)$(SUFFIX)$(ARCHSUFFIX)$(XEXTRA_SUFFIX)gui$(COMPILER_SUFFIX)
endif

# debug builds just get the 'd' suffix and nothing more
ifdef DEBUG
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
    EMULATORLIB = $(LIBNAME).lib
    EMULATORDLL = $(LIBNAME).dll
    EMULATORCLI = $(NAME)$(EXE)
    EMULATORGUI = $(GUINAME)$(EXE)
    EMULATOR    = $(EMULATORDLL) $(EMULATORCLI) $(EMULATORGUI)
endif



#-------------------------------------------------
# compile-time definitions
#-------------------------------------------------

ifdef USE_GCC
    DEFS = -DX86_ASM -DLSB_FIRST -DINLINE="static __inline__" -Dasm=__asm__ -DCRLF=3 -DXML_STATIC -Drestrict=__restrict
else
    DEFS = -DLSB_FIRST=1 -DINLINE='static __forceinline' -Dinline=__inline -D__inline__=__inline -DCRLF=3 -DXML_STATIC
	ifndef INTEL
		DEFS += -Drestrict=
	endif
endif

ifdef DEBUG
DEFS += -DMAME_DEBUG
endif

ifdef NEW_DEBUGGER
DEFS += -DNEW_DEBUGGER
endif

ifdef NEW_RENDER
DEFS += -DNEW_RENDER
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

ifneq ($(USE_JOY_EXTRA_BUTTONS),)
    DEFS += -DUSE_JOY_EXTRA_BUTTONS
endif

ifneq ($(USE_NEOGEO_HACKS),)
    DEFS+= -DUSE_NEOGEO_HACKS
endif

ifneq ($(SHOW_UNAVAILABLE_FOLDER),)
    DEFS += -DSHOW_UNAVAILABLE_FOLDER
endif

ifneq ($(USE_IPS),)
    DEFS += -DUSE_IPS
endif

ifdef USE_VOLUME_AUTO_ADJUST
    DEFS += -DUSE_VOLUME_AUTO_ADJUST
endif

ifdef X86_M68K_DRC
    DEFS += -DX86_M68K_DRC
endif

ifneq ($(USE_SHOW_TIME),)
    DEFS += -DUSE_SHOW_TIME
endif

ifneq ($(USE_SHOW_INPUT_LOG),)
    DEFS += -DUSE_SHOW_INPUT_LOG
endif



#-------------------------------------------------
# compile and linking flags
#-------------------------------------------------

ifdef USE_GCC
    CFLAGS = -std=gnu89 -Isrc -Isrc/includes -Isrc/zlib -Iextra/include -Isrc/$(MAMEOS)

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
			-Wwrite-strings \
			-Wdeclaration-after-statement
		#	-Wformat-security
    endif

    ifdef I686
    # If you have a trouble in I686 build, try to remove a comment.
    #    CFLAGS += -fno-builtin -fno-omit-frame-pointer 
    endif

    # extra options needed *only* for the osd files
    CFLAGSOSDEPEND = $(CFLAGS)

else
    CFLAGS = -Isrc -Isrc/includes -Isrc/zlib -Isrc/$(MAMEOS) \
             -W3 -nologo

    ifdef INTEL
		CFLAGS += -Qc99 -Qrestrict
    endif

    ifneq ($(W_ERROR),)
        CFLAGS += -WX
    endif

    ifneq ($(SYMBOLS),)
        CFLAGS += -Od -RTC1 -MLd -ZI -Zi -GS
    else
        ifneq ($(USE_IPO),)
            ifdef INTEL
                CFLAGS += -Qipo -Qipo_obj
            else
                CFLAGS += -GL
            endif
        endif

        ifdef INTEL
            CFLAGS += -O3 -Qip -Qvec_report0
        else
            CFLAGS += -O2
        endif

        CFLAGS += -Og -Ob2 -Oi -Ot -Oy -GA -Gy -GF
        CFLAGS += -DNDEBUG -ML $(ARCH)
    endif
endif

ifdef USE_GCC
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
else
    ARFLAGS = -nologo

    LDFLAGS += -machine:x86 -nologo -opt:noref

    ifneq ($(SYMBOLS),)
        LDFLAGS += -debug:full -incremental -nodefaultlib:libc
    else
        LDFLAGS += -release -incremental:no

        ifneq ($(USE_IPO),)
            ifndef INTEL
                ARFLAGS += -LTCG
                LDFLAGS += -LTCG
            endif
        endif
    endif

    ifneq ($(MAP),)
        MAPFLAGS = -map
        MAPDLLFLAGS = -map
        MAPCLIFLAGS = -map
        MAPGUIFLAGS = -map
    else
        MAPFLAGS =
        MAPDLLFLAGS =
        MAPCLIFLAGS =
        MAPGUIFLAGS =
    endif
endif

ifdef COMPILESYSTEM_CYGWIN
CFLAGS	+= -mno-cygwin
LDFLAGS	+= -mno-cygwin
endif



#-------------------------------------------------
# define the dependency search paths
#-------------------------------------------------

VPATH = src $(wildcard src/cpu/*)



#-------------------------------------------------
# define the standard object directories
#-------------------------------------------------


OBJDIRS = obj $(OBJ) $(OBJ)/cpu $(OBJ)/sound $(OBJ)/$(MAMEOS) \
	$(OBJ)/drivers $(OBJ)/machine $(OBJ)/vidhrdw $(OBJ)/sndhrdw $(OBJ)/debug

ifdef MESS
OBJDIRS += $(OBJ)/mess $(OBJ)/mess/systems $(OBJ)/mess/machine \
	$(OBJ)/mess/vidhrdw $(OBJ)/mess/sndhrdw $(OBJ)/mess/tools
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
ifdef BUILD_EXPAT
CFLAGS += -Isrc/expat
OBJDIRS += $(OBJ)/expat
EXPAT = $(OBJ)/libexpat.a
#COREOBJS += $(EXPAT)
else
LIBS += -lexpat
EXPAT =
endif

# add ZLIB compression library
ifdef BUILD_ZLIB
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

all:	maketree emulator extrafiles



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

ifdef BUILD_EXPAT
COREOBJS += $(EXPAT)
endif

ifdef BUILD_ZLIB
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

emulator:	maketree $(EMULATOR)

extrafiles:	$(TOOLS)

maketree: $(sort $(OBJDIRS))

clean:
	@echo Deleting object tree $(OBJ)...
	$(RM) -r $(OBJ)
	@echo Deleting $(EMULATOR)...
	$(RM) $(EMULATOR)
	@echo Deleting $(TOOLS)...
	$(RM) $(TOOLS)

check: $(EMULATOR) xml2info$(EXE)
	./$(EMULATOR) -listxml > $(NAME).xml
	./xml2info < $(NAME).xml > $(NAME).lst
	./xmllint --valid --noout $(NAME).xml



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
ifdef USE_GCC
	$(CC) $(CDEFS) $(CFLAGS) -c src/version.c -o $(OBJ)/version.o
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -c src/version.c -Fo$(OBJ)/version.o
endif
	@echo Linking $@...

ifneq ($(NO_DLL),)

    ifdef USE_GCC
        ifneq ($(WINUI),)
			$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(WINDOWS_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -o $@ $(MAPFLAGS)
        else
			$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -o $@ $(MAPFLAGS)
        endif
    else
        ifneq ($(WINUI),)
			$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(WINDOWS_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -out:$(EMULATOR) $(MAPFLAGS)
        else
			$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) -out:$(EMULATOR) $(MAPFLAGS)
        endif
    endif

    ifneq ($(UPX),)
		upx -9 $(EMULATOR)
    endif

else
	$(RM) $@
    ifdef USE_GCC
		$(DLLWRAP) --image-base=0x10080000 --dllname=$@ --driver-name=gcc \
			$(LDFLAGS) $(OSDBGLDFLAGS) $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) $(MAPDLLFLAGS)
    else
		$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) -dll -out:$@ $(OBJS) $(COREOBJS) $(OSOBJS) $(LIBS) $(CPULIB) $(SOUNDLIB) $(DRVLIBS) $(OSDBGOBJS) $(MAPDLLFLAGS)
    endif
    ifneq ($(UPX),)
		upx -9 $@
    endif

# gui target
    $(EMULATORGUI): $(EMULATORDLL) $(GUIOBJS)
		@echo Linking $@...
    ifdef USE_GCC
		$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $(EMULATORDLL) $^ -o $@ $(GUILIBS) $(MAPGUIFLAGS)
    else
		$(LD) $(LDFLAGS) $(WINDOWS_PROGRAM) $(EMULATORLIB) $(GUIOBJS) -out:$@ $(GUILIBS) $(LIBS) $(MAPGUIFLAGS)
    endif
    ifneq ($(UPX),)
		upx -9 $@
    endif

# cli target
    $(EMULATORCLI):	$(EMULATORDLL) $(CLIOBJS)
		@echo Linking $@...
    ifdef USE_GCC
		$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $(EMULATORDLL) $^ -o $@ $(CLILIBS) $(MAPCLIFLAGS)
    else
		$(LD) $(LDFLAGS) $(CONSOLE_PROGRAM) $(EMULATORLIB) $(CLIOBJS) -out:$@ $(CLILIBS) $(MAPCLIFLAGS)
    endif
    ifneq ($(UPX),)
		upx -9 $@
    endif

endif

romcmp$(EXE): $(OBJ)/romcmp.o $(OBJ)/unzip.o $(OBJ)/ui_lang.o $(VCOBJS) $(ZLIB) $(OSDBGOBJS)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -o $@
    else
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -out:$@
    endif

chdman$(EXE): $(OBJ)/chdman.o $(OBJ)/chd.o $(OBJ)/chdcd.o $(OBJ)/cdrom.o $(OBJ)/md5.o $(OBJ)/sha1.o $(OBJ)/version.o $(ZLIB) $(OSTOOLOBJS) $(OSDBGOBJS)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -o $@
    else
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -out:$@
    endif

xml2info$(EXE): $(OBJ)/xml2info.o $(EXPAT) $(ZLIB) $(OSDBGOBJS)
	@echo Linking $@...
    ifdef USE_GCC
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -o $@
    else
	$(LD) $(LDFLAGS) $(OSDBGLDFLAGS) $(CONSOLE_PROGRAM) $^ $(LIBS) -out:$@
    endif




#-------------------------------------------------
# library targets and dependencies
#-------------------------------------------------

$(CPULIB): $(CPUOBJS)

ifdef DEBUG
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
ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGSOSDEPEND) -c $< -o $@
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
endif

$(OBJ)/%.o: src/%.c
ifdef USE_GCC
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
else
	@echo -n Compiling\040
	$(CC) $(CDEFS) $(CFLAGS) -Fo$@ -c $<
endif

$(OBJ)/%.pp: src/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: src/%.c
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
ifdef USE_GCC
	$(AR) cr $@ $^
else
	$(AR) $(ARFLAGS) -out:$@ $^
endif
