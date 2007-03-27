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
#include "winuiopt.h"
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

	set_core_localized_directory(GetLocalizedDir());
	set_core_history_filename(GetHistoryFile());
#ifdef STORY_DATAFILE
	set_core_story_filename(GetStoryFile());
#endif /* STORY_DATAFILE */
	set_core_mameinfo_filename(GetMAMEInfoFile());

	*dataBuf = 0;
	if (load_driver_history(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		wcscat(historyBuf, _UTF8Unicode(p));
	}

#ifdef STORY_DATAFILE
	if (!GetShowTab(TAB_STORY))
	{
		*dataBuf = 0;
		if (load_driver_story(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
		{
			p = ConvertToWindowsNewlines(dataBuf);
			wcscat(historyBuf, _UTF8Unicode(p));
		}
	}
#endif /* STORY_DATAFILE */

	*dataBuf = 0;
	if (load_driver_mameinfo(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		wcscat(historyBuf, _UTF8Unicode(p));
	}

	*dataBuf = 0;
	if (load_driver_drivinfo(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		wcscat(historyBuf, _UTF8Unicode(p));
	}

	return historyBuf;
}

#ifdef STORY_DATAFILE
LPCWSTR GetGameStory(int driver_index)
{
	char dataBuf[MAX_HISTORY_LEN];
	char *p;

	historyBuf[0] = '\0';

	set_core_localized_directory(GetLocalizedDir());
	set_core_story_filename(GetStoryFile());

	*dataBuf = 0;
	if (load_driver_story(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		wcscat(historyBuf, _UTF8Unicode(p));
	}

	return historyBuf;
}
#endif /* STORY_DATAFILE */
