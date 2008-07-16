@echo off
set MINGW_ROOT=..\MinGW
set PATH=%MINGW_ROOT%\bin;extra\bin;%PATH%

set C_INCLUDE_PATH=extra\include
set LIBRARY_PATH=extra\lib

gcc -v
