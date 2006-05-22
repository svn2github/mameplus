@echo off

rem --------------------------------------
rem Intel C++ Compiler Configure
rem --------------------------------------

make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 maketree
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 USE_IPO= W_ERROR= NO_FORCEINLINE=1 obj/mamep-icc/cpu/m6809/m6809.o
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 USE_IPO= W_ERROR= emulator
