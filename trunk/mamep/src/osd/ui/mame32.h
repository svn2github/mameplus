/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef MAME32_H
#define MAME32_H

#ifndef DONT_USE_DLL
#ifndef _MSC_VER
#define SHAREDOBJ_IMPORT
#endif /* !_MSC_VER */
#endif /* !DONT_USE_DLL */

#include "osd_so.h"

#undef assert
#ifdef MAME_DEBUG
#include "M32Util.h"
#define assert(x)	do { if (!(x)) { printf("assert: %s:%d: %s", __FILE__, __LINE__, #x); dprintf("assert: %s:%d: %s", __FILE__, __LINE__, #x); exit(-1); } } while (0)
#else
#define assert(x)
#endif

#undef strdup
#undef stricmp
#undef strnicmp

#include "screenshot.h"
#include "win32ui.h"

#if !defined(MAME32NAME)
#ifndef HAZEMD
#define MAME32NAME "MAME32"
#else
#define MAME32NAME "HazeMD"
#endif
#endif
	
#if !defined(TEXT_MAME32NAME)
#ifndef HAZEMD
#define TEXT_MAME32NAME TEXT("MAME32")
#else
#define TEXT_MAME32NAME TEXT("HazeMD")
#endif
#endif

#if !defined(MAMENAME)
#define MAMENAME "MAME"
#endif

#define UI_MSG_UI	UI_MSG_OSD1
#define UI_MSG_EXTRA	UI_MSG_OSD2

#undef _
#undef _LST
#undef _READINGS
#undef _MANUFACT
#undef _WINDOWS
#undef _UI

#if 0
#define _(str)		mb_lang_message(UI_MSG_MAME, str)
#define _LST(str)	mb_lang_message(UI_MSG_LIST, str)
#define _READINGS(str)	mb_lang_message(UI_MSG_READINGS, str)
#define _MANUFACT(str)	mb_lang_message(UI_MSG_MANUFACTURE, str)
#define _WINDOWS(str)	mb_lang_message(UI_MSG_OSD0, str)
#define _UI(str)	mb_lang_message(UI_MSG_UI, str)
#endif

#define _W(str)		w_lang_message(UI_MSG_MAME, str)
#define _LSTW(str)	w_lang_message(UI_MSG_LIST, str)
#define _READINGSW(str)	w_lang_message(UI_MSG_READINGS, str)
#define _MANUFACTW(str)	w_lang_message(UI_MSG_MANUFACTURE, str)
#define _WINDOWSW(str)	w_lang_message(UI_MSG_OSD0, str)
#define _UIW(str)	w_lang_message(UI_MSG_UI, str)
#endif


// It seems that functions lstr...() don't work properly on win9x.
// Anyway we'd better to use functions wcs...() instead of them.
#undef lstrcpy
#define lstrcpy		!use_wcscpy_win9x_doesnt_work_properly!
#undef lstrcpyn
#define lstrcpyn	!use_wcsncpy_win9x_doesnt_work_properly!
#undef lstrcmp
#define lstrcmp		!use_wcscmp_win9x_doesnt_work_properly!
#undef lstrcmpi
#define lstrcmpi	!use_wcsicmp_win9x_doesnt_work_properly!

#include <wchar.h>
#define _wfindfirst	!win9x_doesnt_has_it!
#define _wfindnext	!win9x_doesnt_has_it!
#define _wmkdir 	!win9x_doesnt_has_it!
#define _wstat  	!win9x_doesnt_has_it!
#define _wunlink	!win9x_doesnt_has_it!
#define _wfopen 	!win9x_doesnt_has_it!
#define _wgetcwd	!win9x_doesnt_has_it!
