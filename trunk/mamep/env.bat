@echo off
set PSDK_DIR=%ProgramFiles%\Microsoft SDKs\Windows\v6.0A
set MINGW_ROOT=D:\wgcc421
set PATH=%MINGW_ROOT%\bin;extra\bin;%PSDK_DIR%\bin\

set C_INCLUDE_PATH=extra\include
set LIBRARY_PATH=extra\lib
set INCLUDE=extravc\include\;%PSDK_DIR%\Include\
set LIB=extravc\lib\;%PSDK_DIR%\Lib\

gcc -v
