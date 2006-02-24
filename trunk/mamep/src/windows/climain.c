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
 #if 1 // #ifndef WINUI
	STARTUPINFO startup_info = { sizeof(STARTUPINFO) };
	GetStartupInfo(&startup_info);

	// try to determine if MAME was simply double-clicked
	if (argc <= 1 &&
		startup_info.dwFlags &&
		!(startup_info.dwFlags & STARTF_USESTDHANDLES))
	{
		char message_text[1024] = "";
		int button;
		FILE* fp;

  #ifndef MESS
		sprintf(message_text, APPLONGNAME " v%s - Multiple Arcade Machine Emulator\n"
							  "Copyright (C) 1997-2006 by Nicola Salmoria and the MAME Team\n"
							  "\n"
							  APPLONGNAME " is a console application, you should launch it from a command prompt.\n"
							  "\n"
							  "Usage:\t" APPNAME " gamename [options]\n"
							  "\n"
							  "\t" APPNAME " -showusage\t\tfor a brief list of options\n"
							  "\t" APPNAME " -showconfig\t\tfor a list of configuration options\n"
							  "\t" APPNAME " -createconfig\tto create a mame.ini\n"
							  "\n"
							  "Please consult the documentation for more information.\n"
							  "\n"
							  "Would you like to open the documentation now?"
							  , build_version);
  #else
		sprintf(message_text, APPLONGNAME " is a console application, you should launch it from a command prompt.\n"
							  "\n"
							  "Please consult the documentation for more information.\n"
							  "\n"
							  "Would you like to open the documentation now?");
  #endif

		// pop up a messagebox with some information
		button = MessageBox(NULL, message_text, APPLONGNAME " usage information...", MB_YESNO | MB_ICONASTERISK);

		if (button == IDYES)
		{
			// check if windows.txt exists
			fp = fopen(helpfile, "r");
			if (fp) {
				fclose(fp);

				// if so, open it with the default application
				ShellExecute(NULL, "open", helpfile, NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				// if not, inform the user
				MessageBox(NULL, "Couldn't find the documentation.", "Error...", MB_OK | MB_ICONERROR);
			}
		}
		return 1;
	}
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

