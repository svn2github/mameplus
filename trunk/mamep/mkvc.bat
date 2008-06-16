call env
call "%VS90COMNTOOLS%vsvars32.bat"
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= OPTIMIZE=ng obj/windows/mamep/emu/mconfig.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= OPTIMIZE=ng obj/windows/mamep/emu/cpu/m68000/d68kcpu.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT= NO_FORCEINLINE=1 obj/windows/mamep/emu/cpu/m68000/d68kops.o
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT=
