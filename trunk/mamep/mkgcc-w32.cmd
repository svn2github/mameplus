@echo off

set MINGW_ROOT=..\mingw\mingw64-w32
set PATH=%MINGW_ROOT%\bin;%PATH%

gcc -v

make >compile.log
pause
