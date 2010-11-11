win32:OSDIR      = win32
macx:OSDIR       = macx
unix:!macx:OSDIR = linux

CONFIG(debug, debug|release){
	OBJECTS_DIR = tmp/debug
}else{
	OBJECTS_DIR = tmp/release
	CONFIG += build_static
	CONFIG += build_sdl
	DEFINES *= QT_NO_DEBUG_OUTPUT
}

build_static {
	QTPLUGIN += qico qjpeg
	DEFINES += USE_STATIC
}

MOC_DIR = tmp
RCC_DIR = tmp
UI_DIR = tmp

macx {
CONFIG += x86 ppc
}
