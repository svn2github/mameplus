include(../common_settings.pri)

DESTDIR = ../lib/$${OSDIR}
TARGET = lzma
TEMPLATE = lib

HEADERS += \
	7zBuf.h \
	7zCrc.h \
	7zFile.h \
	7zVersion.h \
	Bcj2.h \
	Bra.h \
	CpuArch.h \
	LzmaDec.h \
	Lzma2Dec.h \
	Types.h \
	7zAlloc.h \
	7zDecode.h \
	7zExtract.h \
	7zHeader.h \
	7zIn.h \
	7zItem.h

SOURCES += \
	7zBuf.c \
	7zBuf2.c \
	7zCrc.c \
	7zFile.c \
	7zStream.c \
	Bcj2.c \
	Bra.c \
	Bra86.c \
	LzmaDec.c \
	Lzma2Dec.c \
	7zAlloc.c \
	7zDecode.c \
	7zExtract.c \
	7zHeader.c \
	7zIn.c \
	7zItem.c
