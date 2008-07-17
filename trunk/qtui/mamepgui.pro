TEMPLATE =  app
TARGET =  mamepguix
INCLUDEPATH += quazip/ D:/Qt/$$QT_VERSION/src/3rdparty/zlib/
FORMS += mamepguimain.ui options.ui about.ui untitled.ui
TRANSLATIONS    = lang/mamepgui_zh_CN.ts
win32 {
RC_FILE = mamepgui.rc
}
SOURCES += mamepguimain.cpp dialogs.cpp audit.cpp gamelist.cpp procmgr.cpp mameopt.cpp utils.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS += mamepguimain.h dialogs.h audit.h gamelist.h procmgr.h mameopt.h utils.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = mamepguimain.qrc
QT += xml
QTPLUGIN     +=  qico qmng
LIBS           = -lqico -lqmng
