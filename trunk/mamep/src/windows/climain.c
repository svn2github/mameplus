#include <windows.h>
#include <stdio.h>

#ifdef DONT_USE_DLL
#else /* DONT_USE_DLL */
#define SHAREDOBJ_IMPORT
#endif /* DONT_USE_DLL */

#include "osd_so.h"

#ifndef MESS
static const char helpfile[] = "docs\\windows.txt";
#else
static const char helpfile[] = "mess.chm";
#endif


//============================================================
//	osd_display_loading_rom_message
//============================================================

// called while loading ROMs. It is called a last time with name == 0 to signal
// that the ROM loading process is finished.
// return non-zero to abort loading
#ifndef DONT_USE_DLL
#ifndef _MSC_VER
int osd_display_loading_rom_message(const char *name,rom_load_data *romdata)
{
	if (name)
		fprintf(stdout, _WINDOWS("loading %-12s\r"), name);
	else
		fprintf(stdout, "%30s\r", "");
	fflush(stdout);

	return 0;
}

int main(int argc, char *argv[])
{
#if 1 // move from windows/winmain.c
#ifndef WINUI
	// check for double-clicky starts
	if (check_for_double_click_start(argc) != 0)
		return 1;
#endif
#endif

	osd_display_loading_rom_message_ = osd_display_loading_rom_message;

	return main_(argc, argv);
}
#else /* _MSC_VER */
int main(int argc, char *argv[])
{
	return main_(argc, argv);
}
#endif /* _MSC_VER */
#endif /* !DONT_USE_DLL */

