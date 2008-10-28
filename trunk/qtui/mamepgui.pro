INCLUDEPATH += quazip include/zlib include/SDL-1.3
win32 {
INCLUDEPATH += include/SDL-1.3/Win32
}
unix {
INCLUDEPATH += include/SDL-1.3/Linux
}

win32 {
LIBS += -Llib/Win32 -lSDLmain-1.3
RC_FILE = mamepgui.rc
}
unix {
LIBS += -Llib/Linux
}
LIBS += -lqico -lqjpeg -lSDL-1.3

FORMS += mamepguimain.ui options.ui directories.ui about.ui
TRANSLATIONS    = lang/mamepgui_zh_CN.ts lang/mamepgui_zh_TW.ts lang/mamepgui_ja_JP.ts lang/mamepgui_pt_BR.ts
SOURCES += mamepguimain.cpp dialogs.cpp audit.cpp gamelist.cpp mameopt.cpp utils.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS += mamepguimain.h dialogs.h audit.h gamelist.h mameopt.h utils.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = mamepguimain.qrc

TARGET =  mamepgui
QT += xml
QTPLUGIN +=  qico qjpeg
