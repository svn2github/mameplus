###########################################################################
#
#   windows.mak
#
#   Windows-specific makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
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
# overrides
#-------------------------------------------------

# turn on unicode for all 64-bit builds regardless
ifndef UNICODE
ifdef PTR64
UNICODE = 1
endif
endif



#-------------------------------------------------
# object and source roots
#-------------------------------------------------

WINSRC = $(SRC)/osd/$(OSD)
WINOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(WINOBJ)



#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

RC = @windres --use-temp-file

RCDEFS = -DNDEBUG -D_WIN32_IE=0x0501

RCFLAGS = -O coff -I $(WINOBJ)
ifneq ($(NO_DLL),)
ifneq ($(WINUI),)
else
RCFLAGS += -I $(WINSRC)
endif
else
RCFLAGS += -I $(WINSRC)
endif



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

    VCONV = $(WINOBJ)/vconv$(EXE)

    # append a 'v' prefix if nothing specified
    ifndef PREFIX
    PREFIX = v
    endif
    
    # replace the various compilers with vconv.exe prefixes
    CC = @$(VCONV) gcc -I.
    LD = @$(VCONV) ld /profile
    AR = @$(VCONV) ar
    RC = @$(VCONV) windres
    
    # make sure we use the multithreaded runtime
    ifdef DEBUG
    CC += /MTd
    else
    CC += /MT
    endif
    
    # turn on link-time codegen if the MAXOPT flag is also set
    ifneq ($(MAXOPT),)
        ifneq ($(ICC_BUILD),)
            CC += /Qipo /Qipo_obj
        else
            CC += /GL
            LD += /LTCG
        endif
    endif
    
    # /O2 (Maximize Speed) equals /Og /Oi /Ot /Oy /Ob2 /Gs /GF /Gy
    # /GA optimize for Windows Application
    CC += /GA
    
    ifdef PTR64
    CC += /wd4267
    endif
    
    # filter X86_ASM define
    DEFS := $(filter-out -DX86_ASM,$(DEFS))
    
    # explicitly set the entry point for UNICODE builds
    ifdef UNICODE
    LD += /ENTRY:wmainCRTStartup
    endif

    # add some VC++-specific defines
    DEFS += -D_CRT_SECURE_NO_DEPRECATE -DXML_STATIC -Dinline=__inline -D__inline__=__inline -Dsnprintf=_snprintf
    
    # make msvcprep into a pre-build step
    # OSPREBUILD = $(VCONV)
    
    # add VCONV to the build tools
    BUILD += $(VCONV)
    
    $(VCONV): $(WINOBJ)/vconv.o
	    @echo Linking $@...
	    @link.exe /nologo $^ version.lib /out:$@
    
    $(WINOBJ)/vconv.o: $(WINSRC)/vconv.c
	    @echo Compiling $<...
	    @cl.exe /nologo /O1 -D_CRT_SECURE_NO_DEPRECATE $(VCONVDEFS) -c $< /Fo$@
endif


#-------------------------------------------------
# nasm for Windows (but not cygwin) has a "w"
# at the end
#-------------------------------------------------

ASM = @nasmw
ASMFLAGS = -f coff -Wno-orphan-labels


#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# define the x64 ABI to be Windows
DEFS += -DX64_WINDOWS_ABI

# map all instances of "main" to "utf8_main"
DEFS += -Dmain=utf8_main

# debug build: enable guard pages on all memory allocations
ifneq ($(DEBUG),)
# mamep: disable malloc debug
#DEFS += -DMALLOC_DEBUG
#LDFLAGS += -Wl,--allow-multiple-definition
endif

# enable UNICODE flags for unicode builds
ifdef UNICODE
DEFS += -DUNICODE -D_UNICODE
endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

# add our prefix files to the mix
CFLAGS += -include $(WINSRC)/winprefix.h

ifneq ($(NO_FORCEINLINE),)
DEFS += -DNO_FORCEINLINE
endif

ifneq ($(WIN95_MULTIMON),)
CFLAGS += -DWIN95_MULTIMON
endif

# add the windows libaries
# mamep: -lunicows MUST be in the first place
LIBS += -lunicows -luser32 -lgdi32 -lddraw -ldsound -ldinput -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi

ifdef PTR64
ifdef MSVC_BUILD
LIBS += -lbufferoverflowu
else
DEFS += -D_COM_interface=struct
endif
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



#-------------------------------------------------
# OSD Windows library
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
OSDOBJS += $(WINOBJ)/scale/superscale.obj $(WINOBJ)/scale/eagle.obj $(WINOBJ)/scale/2xsaimmx.obj
ifneq ($(ASM_HQ),)
DEFS += -DASM_HQ
OSDOBJS += $(WINOBJ)/scale/hq2x16.obj $(WINOBJ)/scale/hq3x16.obj
endif
OSDOBJS += $(WINOBJ)/scale/scale2x.o $(WINOBJ)/scale/scale3x.o $(WINOBJ)/scale/2xpm.o

ifneq ($(USE_MMX_INTERP_SCALE),)
DEFS += -DUSE_MMX_INTERP_SCALE
OSDOBJS += $(WINOBJ)/scale/hlq_mmx.o
else
OSDOBJS += $(WINOBJ)/scale/hlq.o
endif

ifneq ($(USE_4X_SCALE),)
DEFS += -DUSE_4X_SCALE
endif

$(WINOBJ)/scale/%.obj: $(WINSRC)/scale/%.asm
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
ifneq ($(DEBUGGER),)
OSDOBJS += \
	$(WINOBJ)/debugwin.o
endif

# add a stub resource file
CLIRESFILE = $(WINOBJ)/mame.res
VERSIONRES = $(WINOBJ)/version.res



#-------------------------------------------------
# if building with a UI, include the ui.mak
#-------------------------------------------------

ifneq ($(WINUI),)
include $(SRC)/osd/winui/winui.mak
endif



#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)



#-------------------------------------------------
# rule for making the ledutil sample
#-------------------------------------------------

LEDUTIL = ledutil$(EXE)
TOOLS += $(LEDUTIL)

LEDUTILOBJS = \
	$(WINOBJ)/ledutil.o

$(LEDUTIL): $(LEDUTILOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

# if we are not using x86drc.o, we should
ifeq ($(X86_MIPS3_DRC),)
COREOBJS += $(OBJ)/x86drc.o
endif


#-------------------------------------------------
# generic rule for the resource compiler
#-------------------------------------------------

$(WINOBJ)/%.res: $(WINSRC)/%.rc | $(OSPREBUILD)
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) -o $@ -i $<



#-------------------------------------------------
# rules for resource file
#-------------------------------------------------

$(WINOBJ)/mame.res: $(WINSRC)/mame.rc $(WINOBJ)/mamevers.rc
$(WINOBJ)/version.res: $(WINOBJ)/mamevers.rc

$(WINOBJ)/mamevers.rc: $(BUILDOUT)/verinfo$(BUILD_EXE) $(SRC)/version.c
	@echo Emitting $@...
	@$(BUILDOUT)/verinfo$(BUILD_EXE) -b windows $(SRC)/version.c > $@


