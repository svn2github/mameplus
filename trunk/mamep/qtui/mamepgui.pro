TEMPLATE =  app
TARGET =  mamepgui
INCLUDEPATH += quazip/ D:/Qt/4.3.3/src/3rdparty/zlib/
FORMS += mamepguimain.ui options.ui untitled.ui
TRANSLATIONS    = lang/mamepgui_zh_CN.ts
win32 {
RC_FILE = mamepgui.rc
}
SOURCES += mamepguimain.cpp options.cpp gamelist.cpp procmgr.cpp mameopt.cpp utils.cpp qticoimageformat.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS += mamepguimain.h options.h gamelist.h procmgr.h mameopt.h utils.h qtendian.h qticohandler.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = mamepguimain.qrc
QT += xml
