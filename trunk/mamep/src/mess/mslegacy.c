/*********************************************************************

	mslegacy.c

	Defines since removed from MAME, but kept in MESS for legacy
	reasons

*********************************************************************/

#include "driver.h"
#include "mslegacy.h"


static const char *const mess_default_text[] =
{
	"OK",
	"Off",
	"On",
	"Dip Switches",
	"Driver Configuration",
	"Analog Controls",
	"Digital Speed",
	"Autocenter Speed",
	"Reverse",
	"Sensitivity",

	"Keyboard Emulation Status",
	"-------------------------",
	"Mode: PARTIAL Emulation",
	"Mode: FULL Emulation",
	"UI:   Enabled",
	"UI:   Disabled",
	"**Use ScrLock to toggle**",

	"No Tape Image loaded",
	"recording",
	"playing",
	"(recording)",
	"(playing)",
	"stopped",
	"Pause/Stop",
	"Record",
	"Play",
	"Rewind",
	"Fast Forward",
	"Mount...",
	"Create...",
	"Unmount",
	"[empty slot]",
	"Input Devices",
	"Quit Fileselector",
	"File Specification",	/* IMPORTANT: be careful to ensure that the following */
	"Cartridge",		/* device list matches the order found in device.h    */
	"Floppy Disk",		/* and is ALWAYS placed after "File Specification"    */
	"Hard Disk",
	"Cylinder",
	"Cassette",
	"Punched Card",
	"Punched Tape",
	"Printer",
	"Serial Port",
	"Parallel Port",
	"Snapshot",
	"Quickload",
	"Memory Card",
	"CD-ROM"
};



//mamep
static const char *const *const default_text[] =
{
	mess_default_text,
	NULL
};


static const char **trans_text;


int uistring_init (void)
{
	int i, j, str;
	int string_count;

	/* count the total amount of strings */
	string_count = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			string_count++;
	}

	/* allocate the translated text array, and set defaults */
	trans_text = auto_malloc(sizeof(const char *) * string_count);

	/* copy in references to all of the strings */
	str = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			trans_text[str++] = _(default_text[i][j]);
	}

	/* indicate success */
	return 0;
}



/***************************************************************************
    UI TEXT
***************************************************************************/

const char * ui_getstring (int string_num)
{
	if (trans_text)
		return trans_text[string_num];
	else 
	return mess_default_text[string_num];
}

