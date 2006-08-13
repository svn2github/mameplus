@echo off

rem --------------------------------------
rem MinGW Compiler Configure
rem --------------------------------------

set PATH=\MinGW\bin;\binutils\bin;extra\bin;%PATH%

rem --------------------------------------
rem HtmlHelp Path Configure
rem --------------------------------------

set C_INCLUDE_PATH=extra\include
set LIBRARY_PATH=extra\lib

make USE_NEOGEO_DECRYPTED=1
