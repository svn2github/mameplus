TEMPLATE     =  app
TARGET       =  qmc2
INCLUDEPATH  += quazip/ D:/Qt/4.3.3/src/3rdparty/zlib/
FORMS        += qmc2main.ui
SOURCES      += qmc2main.cpp gamelist.cpp procmgr.cpp qticoimageformat.cpp quazip/ioapi.c quazip/zip.c quazip/unzip.c quazip/quazip.cpp quazip/quazipfile.cpp
HEADERS      += qmc2main.h gamelist.h procmgr.h qtendian.h qticohandler.h quazip/ioapi.h quazip/zip.h quazip/unzip.h quazip/quazip.h quazip/quazipfile.h quazip/quazipfileinfo.h
RESOURCES   = qmc2main.qrc
QT           += xml
