include(../common_settings.pri)

DESTDIR = ../lib/$${OSDIR}
TARGET = quazip
TEMPLATE = lib

INCLUDEPATH += ../include/zlib

HEADERS += \
	ioapi.h \
	zip.h \
	unzip.h \
	quazip.h \
	quazipfile.h \
	quazipfileinfo.h

SOURCES += \
	ioapi.c \
	zip.c \
	unzip.c \
	quazip.cpp \
	quazipfile.cpp
