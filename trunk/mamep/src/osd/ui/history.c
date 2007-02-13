/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/
 
/***************************************************************************

  history.c

    history functions.

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <stdio.h>

#include "MAME32.h"
#include <driver.h>
#include "m32util.h"
#include "bitmask.h"
#include "options.h"
#include "translate.h"

#include "history.h"
#include "datafile.h"

/* Game history variables */
#define MAX_HISTORY_LEN     (400 * 1024)


static WCHAR  historyBuf[MAX_HISTORY_LEN];


/**************************************************************
 * functions
 **************************************************************/

// Load indexes from history.dat if found
LPCWSTR GetGameHistory(int driver_index)
{
	char dataBuf[MAX_HISTORY_LEN];
	char *p;

	historyBuf[0] = '\0';

	options_set_string(OPTION_LOCALIZED_DIRECTORY, GetLocalizedDir());
	options_set_string(OPTION_HISTORY_FILE, GetHistoryFile());
#ifdef STORY_DATAFILE
	options_set_string(OPTION_STORY_FILE, GetStoryFile());
#endif /* STORY_DATAFILE */
	options_set_string(OPTION_MAMEINFO_FILE, GetMAMEInfoFile());

	*dataBuf = 0;
	if (load_driver_history(drivers[driver_index], dataBuf, sizeof(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		lstrcat(historyBuf, _Unicode(p));
	}

#ifdef STORY_DATAFILE
	if (!GetShowTab(TAB_STORY))
	{
		*dataBuf = 0;
		if (load_driver_story(drivers[driver_index], dataBuf, sizeof(dataBuf)) == 0)
		{
			p = ConvertToWindowsNewlines(dataBuf);
			lstrcat(historyBuf, _Unicode(p));
		}
	}
#endif /* STORY_DATAFILE */

	*dataBuf = 0;
	if (load_driver_mameinfo(drivers[driver_index], dataBuf, sizeof(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		lstrcat(historyBuf, _Unicode(p));
	}

	*dataBuf = 0;
	if (load_driver_drivinfo(drivers[driver_index], dataBuf, sizeof(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		lstrcat(historyBuf, _Unicode(p));
	}

	return historyBuf;
}

#ifdef STORY_DATAFILE
LPCWSTR GetGameStory(int driver_index)
{
	char dataBuf[MAX_HISTORY_LEN];
	char *p;

	historyBuf[0] = '\0';

	options_set_string(OPTION_LOCALIZED_DIRECTORY, GetLocalizedDir());
#ifdef STORY_DATAFILE
	options_set_string(OPTION_STORY_FILE, GetStoryFile());
#endif /* STORY_DATAFILE */

	*dataBuf = 0;
	if (load_driver_story(drivers[driver_index], dataBuf, sizeof(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		lstrcat(historyBuf, _Unicode(p));
	}

	return historyBuf;
}
#endif /* STORY_DATAFILE */
