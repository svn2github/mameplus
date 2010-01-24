/***************************************************************************

  M.A.M.E.UI  -  Multiple Arcade Machine Emulator with User Interface
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse,
  Copyright (C) 2003-2007 Chris Kirmse and the MAME32/MAMEUI team.

  This file is part of MAMEUI, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/
 
/***************************************************************************

  history.c

    history functions.

***************************************************************************/
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

// MAME/MAMEUI headers
#include "emu.h"
#include "mui_util.h"
#include "datafile.h"
#include "history.h"
#include "mui_opts.h"
#include "translate.h"

/* Game history variables */
#define MAX_HISTORY_LEN     (1024 * 1024)


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

	GetLanguageDir();
	GetHistoryFileName();
	GetMAMEInfoFileName();
#ifdef STORY_DATAFILE
	GetStoryFileName();
#endif /* STORY_DATAFILE */

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

	GetLanguageDir();
	GetStoryFileName();

	*dataBuf = 0;
	if (load_driver_story(drivers[driver_index], dataBuf, ARRAY_LENGTH(dataBuf)) == 0)
	{
		p = ConvertToWindowsNewlines(dataBuf);
		wcscat(historyBuf, _UTF8Unicode(p));
	}

	return historyBuf;
}
#endif /* STORY_DATAFILE */
