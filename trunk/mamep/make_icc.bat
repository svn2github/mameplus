@echo off

rem --------------------------------------
rem Intel C++ Compiler Configure
rem --------------------------------------

make ICC_BUILD=1 I686=1 ARCHSUFFIX= maketree obj/windows/mamep-icc/osd/windows/vconv.exe
make ICC_BUILD=1 I686=1 ARCHSUFFIX= MAXOPT= W_ERROR= NO_FORCEINLINE=1 obj/windows/mamep-icc/emu/cpu/m6809/m6809.o
make ICC_BUILD=1 I686=1 ARCHSUFFIX= MAXOPT= W_ERROR= emulator
