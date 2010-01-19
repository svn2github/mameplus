@echo off

set MINGW_ROOT=..\mingw\mingw64-w64
set PATH=%MINGW_ROOT%\bin;extra\bin;%PATH%

gcc -v

make >compile.log
pause
