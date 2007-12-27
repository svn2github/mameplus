//============================================================
//
//	menu.h - Win32 MESS menu handling
//
//============================================================

#ifndef MENU_H
#define MENU_H

#include <windows.h>



//============================================================
//	PROTOTYPES
//============================================================

int win_setup_menus(HMODULE module, HMENU menu_bar);
LRESULT CALLBACK win_mess_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
//void osd_toggle_menubar(int); //mamep: moved to osdepend.h



#endif /* MENU_H */
