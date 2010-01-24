//============================================================
//
//	guimain.c - Win32 GUI entry point
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int __declspec(dllimport) mame_gui_main(HINSTANCE    hInstance,
                   HINSTANCE    hPrevInstance,
                   LPWSTR        lpCmdLine,
                   int          nCmdShow);

int WINAPI wWinMain(HINSTANCE    hInstance,
                   HINSTANCE    hPrevInstance,
                   LPWSTR       lpCmdLine,
                   int          nCmdShow)
{
	return mame_gui_main(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
