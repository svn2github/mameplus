TEMPLATE =  app
TARGET =  mamepgui
INCLUDEPATH += quazip/ D:/Qt/4.3.3/src/3rdparty/zlib/
FORMS += qmc2main.ui options.ui untitled.ui
TRANSLATIONS    = lang/mamepgui_zh_CN.ts
win32 {
RC_FILE = mamepgui.rc
}
SOURCES += qmc2main.cpp options.cpp gamelist.cpp procmgr.cpp mameopt.cpp utils.cpp qticoimageformat.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS += qmc2main.h options.h gamelist.h procmgr.h mameopt.h utils.h qtendian.h qticohandler.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = qmc2main.qrc
QT += xml
