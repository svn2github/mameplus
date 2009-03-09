call env

set PSDK_DIR=%ProgramFiles%\Microsoft SDKs\Windows\v6.0A
set PATH=%PSDK_DIR%\bin\;%PATH%

set INCLUDE=extravc\include\;%PSDK_DIR%\Include\
set LIB=extravc\lib\;extravc\lib\x86;%PSDK_DIR%\Lib\

call "%VS90COMNTOOLS%vsvars32.bat"

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= maketree obj/windows/mamep/osd/windows/vconv.exe
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= NO_FORCEINLINE=1 obj/windows/mamep/emu/cpu/m6809/m6809.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= NO_FORCEINLINE=1 obj/windows/mamep/osd/windows/scale/snes9x_render.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= OPTIMIZE=ng obj/windows/mamep/emu/mconfig.o

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT=
