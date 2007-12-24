call env
call "%VS90COMNTOOLS%vsvars32.bat"
mingw32-make MSVC_BUILD=1 PREFIX= MAXOPT=
