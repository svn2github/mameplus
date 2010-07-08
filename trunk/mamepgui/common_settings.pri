win32:OSDIR      = win32
macx:OSDIR       = macx
unix:!macx:OSDIR = linux

CONFIG(debug, debug|release){
	OBJECTS_DIR       = obj/debug
}else{
	OBJECTS_DIR       = obj/release
	DEFINES          *= QT_NO_DEBUG_OUTPUT
}

MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

CONFIG += build_static
CONFIG += build_sdl
macx {
CONFIG += x86 ppc
}
