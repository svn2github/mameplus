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
#define MAME32NAME "MAME32"
#endif
	
#if !defined(TEXT_MAME32NAME)
#define TEXT_MAME32NAME TEXT("MAME32")
#endif

#if !defined(MAMENAME)
#define MAMENAME "MAME"
#endif

#define UI_MSG_UI	UI_MSG_OSD1
#define UI_MSG_EXTRA	UI_MSG_OSD2

#define _UI(str)	lang_message(UI_MSG_UI, str)
#endif
