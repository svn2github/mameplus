call env

set PSDK_DIR=%ProgramFiles%\Microsoft SDKs\Windows\v6.0A
set PATH=%PSDK_DIR%\bin\;%PATH%

set INCLUDE=extravc\include\;%PSDK_DIR%\Include\
set LIB=extravc\lib\;extravc\lib\x64;%PSDK_DIR%\Lib\

call "D:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\amd64\vcvarsamd64.bat"

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 maketree obj/windows/mamep/osd/windows/vconv.exe
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 NO_FORCEINLINE=1 obj/windows/mamep/emu/cpu/m6809/m6809.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 NO_FORCEINLINE=1 obj/windows/mamep/emu/cpu/mips/mips3drc.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 NO_FORCEINLINE=1 obj/windows/mamep/osd/windows/scale/hlq.o

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 OPTIMIZE=ng obj/windows/mamep/emu/mconfig.o

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 OPTIMIZE=ng obj/windows/mamep/emu/cpu/m68000/d68kcpu.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 NO_FORCEINLINE=1 obj/windows/mamep/emu/cpu/m68000/d68kops.o

mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= PTR64=1 NO_DLL=1 SUFFIX=
pause