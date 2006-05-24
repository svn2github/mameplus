@echo off

rem --------------------------------------
rem Intel C++ Compiler Configure
rem --------------------------------------

make ICC_BUILD=1 I686=1 ARCHSUFFIX= maketree
make ICC_BUILD=1 I686=1 ARCHSUFFIX= MAXOPT= W_ERROR= NO_FORCEINLINE=1 obj/mamep-icc/cpu/m6809/m6809.o
make ICC_BUILD=1 I686=1 ARCHSUFFIX= MAXOPT= W_ERROR= emulator
