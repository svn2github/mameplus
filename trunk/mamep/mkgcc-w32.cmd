@echo off

set MINGW_ROOT=..\mingw\mingw64-w32
set PATH=%MINGW_ROOT%\bin;extra\bin;%PATH%

gcc -v

make ARCHOPTS=-march=pentiumpro >compile.log
pause
