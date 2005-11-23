make USE_VC=1 I686=1 ARCHSUFFIX= maketree
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 CFLAGS="/Og /Ob2 /Oi /Ot /Oy -Isrc/debug -Isrc/vc /I "src" /I "src\windows" /D "NDEBUG" /D "WIN32" /GF /W3 /nologo /c /FI "vcmame.h"  /Oa " obj/mameip/cpu/m6809/m6809.o
make USE_VC=1 I686=1 ARCHSUFFIX= INTEL=1 USE_IPO= W_ERROR=
