TARGET =  mamepguix
INCLUDEPATH += quazip/ zlib/
FORMS += mamepguimain.ui options.ui directories.ui about.ui
TRANSLATIONS    = lang/mamepgui_zh_CN.ts lang/mamepgui_zh_TW.ts lang/mamepgui_ja_JP.ts
SOURCES += mamepguimain.cpp dialogs.cpp audit.cpp gamelist.cpp procmgr.cpp mameopt.cpp utils.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS += mamepguimain.h dialogs.h audit.h gamelist.h procmgr.h mameopt.h utils.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = mamepguimain.qrc
QT += xml
#static qt works with windows version
win32 {
RC_FILE = mamepgui.rc
QTPLUGIN     +=  qico qjpeg
LIBS           = -lqico -lqjpeg
}