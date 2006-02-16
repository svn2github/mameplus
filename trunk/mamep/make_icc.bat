@echo off

rem --------------------------------------
rem Intel C++ Compiler Configure
rem --------------------------------------

make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 maketree
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 CFLAGS="/Og /Ob2 /Oi /Ot /Oy -Isrc/debug -Isrc/vc -Isrc -Isrc/windows /D NDEBUG /D WIN32 /GF /W3 /nologo /c /FI vcmame.h /Oa " obj/mamep-icc/cpu/m6809/m6809.o
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 USE_IPO= W_ERROR= emulator
