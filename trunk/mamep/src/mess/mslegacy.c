/*********************************************************************

	mslegacy.c

	Defines since removed from MAME, but kept in MESS for legacy
	reasons

*********************************************************************/

#include "driver.h"
#include "uitext.h"
#include "mslegacy.h"


const char *const mess_default_text[] =
{
	"The emulated system is a computer: ",
	"The keyboard emulation may not be 100% accurate.",
	"Keyboard Emulation Status",
	"-------------------------",
	"Mode: PARTIAL Emulation",
	"Mode: FULL Emulation",
	"UI:   Enabled",
	"UI:   Disabled",
	"**Use ScrLock to toggle**",

	"Image Information",
	"File Manager",
	"Tape Control",

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
	"CD-ROM",
	NULL
};



/***************************************************************************
    UI TEXT
***************************************************************************/
/*
const char * ui_getstring (int string_num)
{
	return mess_default_text[string_num];
}
*/
