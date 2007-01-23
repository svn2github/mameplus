/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/

#ifndef DONT_USE_DLL
#define SHAREDOBJ_IMPORT
#endif /* DONT_USE_DLL */

#include "osd_so.h"

#undef WinMainInDLL
#ifndef DONT_USE_DLL
#ifdef _MSC_VER
#define WinMainInDLL
#endif /* _MSC_VER */
#endif /* !DONT_USE_DLL */

// import the main() from MAME, but rename it so we can call it indirectly
#define main main_
#include "windows/main.c"
#undef main

#ifdef WinMainInDLL
SHAREDOBJ_FUNC(int) WinMain_(HINSTANCE    hInstance,
#else
extern int WinMain_(HINSTANCE    hInstance,
#endif
                   HINSTANCE    hPrevInstance,
                   LPSTR        lpCmdLine,
                   int          nCmdShow);

int WINAPI WinMain(HINSTANCE    hInstance,
                   HINSTANCE    hPrevInstance,
                   LPSTR        lpCmdLine,
                   int          nCmdShow)
{
	return WinMain_(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
