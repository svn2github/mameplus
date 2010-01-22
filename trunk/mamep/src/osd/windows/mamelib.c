//============================================================
//
//  mamelib.c - Entry points for mamelib.dll
//
//============================================================

// We want to use the main() in src/windows/main.c, but we have
// to export the entry point with dllexport.  So we'll do a trick
// to wrap it
#define MAMELIB
#undef main
#undef wmain
#define main mame_main
#define wmain mame_main
#include "windows/main.c"
#undef main
#undef wmain

//============================================================
//  mame_cli_main - main entry proc for CLI MAME
//============================================================

int __declspec(dllexport) mame_cli_main(int argc, char **argv)
{
	return mame_main(argc, (TCHAR **)argv);
}



//============================================================
//  mame_gui_main - main entry proc for GUI MAME
//============================================================

int __declspec(dllexport) mame_gui_main(
	HINSTANCE    hInstance,
	HINSTANCE    hPrevInstance,
	LPTSTR       lpCmdLine,
	int          nCmdShow)
{
	extern int MameUIMain(HINSTANCE, LPTSTR, int);
	return MameUIMain(hInstance, lpCmdLine, nCmdShow);
}
