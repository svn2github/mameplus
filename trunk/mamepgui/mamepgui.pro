include(common_settings.pri)

DESTDIR = bin
TARGET = mamepgui
QT += xml

#INCLUDE
INCLUDEPATH += quazip lzma include include/zlib include/SDL include/SDL/$${OSDIR}

#LIBS
LIBS += -L./lib/$${OSDIR}
LIBS += -lquazip -llzma

build_static {
  QTPLUGIN += qico qjpeg
  DEFINES += USE_STATIC
}

build_sdl {
  macx: LIBS += -framework Cocoa -framework IOKit -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework OpenGL -framework ForceFeedback
  LIBS += -lSDL
  DEFINES += USE_SDL
}

win32: RC_FILE = mamepgui.rc

macx {
DEFINES += _LZMA_UINT32_IS_ULONG
ICON = mamepgui.icns
}

HEADERS += \
	mamepgui_types.h \
	mamepgui_main.h \
	dialogs.h \
	audit.h \
	gamelist.h \
	mameopt.h \
	utils.h \
	ips.h \
	m1.h \

SOURCES += \
	mamepgui_types.cpp \
	mamepgui_main.cpp \
	dialogs.cpp \
	audit.cpp \
	gamelist.cpp \
	mameopt.cpp \
	utils.cpp \
	ips.cpp \
	m1.cpp \

FORMS += \
	mamepgui_main.ui \
	playoptions.ui \
	options.ui \
	csvcfg.ui \
	directories.ui \
	about.ui \
	cmd.ui \
	ips.ui \
	m1.ui

TRANSLATIONS = \
	lang/mamepgui_zh_CN.ts \
	lang/mamepgui_zh_TW.ts \
	lang/mamepgui_ja_JP.ts \
	lang/mamepgui_es_ES.ts \
	lang/mamepgui_fr_FR.ts \
	lang/mamepgui_hu_HU.ts \
	lang/mamepgui_ko_KR.ts \
	lang/mamepgui_pt_BR.ts \
	lang/mamepgui_ru_RU.ts

RESOURCES = mamepgui_main.qrc
