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

#undef assert
#ifdef MAME_DEBUG
#include "mui_util.h"
#define assert(x)	do { if (!(x)) { printf("assert: %s:%d: %s", __FILE__, __LINE__, #x); dprintf("assert: %s:%d: %s", __FILE__, __LINE__, #x); exit(-1); } } while (0)
#else
#define assert(x)
#endif

#include "winui.h"

#if !defined(MAMEUINAME)
#define MAMEUINAME "MAMEUI"
#endif
	
#if !defined(TEXT_MAMEUINAME)
#define TEXT_MAMEUINAME TEXT("MAMEUI")
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


#ifdef _MSC_VER
	#define wcscmpi _wcsicmp
	#define snprintf _snprintf
	#define snwprintf _snwprintf

	// for VC2005
	#if _MSC_VER >= 1400
		#undef strdup
		#undef stricmp
		#define wcsdup _wcsdup
		#define wcsicmp _wcsicmp
		#define strdup _strdup
		#define stricmp _stricmp
		#define strlwr _strlwr
		#define itoa _itoa
	#endif
#endif


// It seems that functions lstr...() don't work properly on win9x.
// Anyway we'd better to use functions wcs...() instead of them.
/* NO Win9x anymore
#undef lstrcpy
#define lstrcpy		!use_wcscpy_win9x_doesnt_work_properly!
#undef lstrcpyn
#define lstrcpyn	!use_wcsncpy_win9x_doesnt_work_properly!
#undef lstrcmp
#define lstrcmp		!use_wcscmp_win9x_doesnt_work_properly!
#undef lstrcmpi
#define lstrcmpi	!use_wcsicmp_win9x_doesnt_work_properly!
#undef lstrlen
#define lstrlen		!use_wcslen_win9x_doesnt_work_properly!
#undef lstrcat
#define lstrcat		!use_wcscat_win9x_doesnt_work_properly!

#include <wchar.h>
#undef _wfindfirst
#define _wfindfirst	!win9x_doesnt_has_it!
#undef _wfindnext
#define _wfindnext	!win9x_doesnt_has_it!
#undef _wmkdir
#define _wmkdir 	!win9x_doesnt_has_it!
#undef _wstat
#define _wstat  	!win9x_doesnt_has_it!
#undef _wunlink
#define _wunlink	!win9x_doesnt_has_it!
#undef _wfopen
#define _wfopen 	!win9x_doesnt_has_it!
#undef _wgetcwd
#define _wgetcwd	!win9x_doesnt_has_it!
*/
