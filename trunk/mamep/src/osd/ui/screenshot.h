/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

typedef struct _mybitmapinfo
{
	int bmWidth;
	int bmHeight;
	int bmColors;
} MYBITMAPINFO, *LPMYBITMAPINFO;

#ifdef MESS
extern BOOL LoadScreenShotEx(int nGame, LPCSTR lpSoftwareName, int nType);
#else /* !MESS */
#ifdef USE_IPS
extern BOOL LoadScreenShot(int nGame, const char *lpIPSName, int nType);
#else /* USE_IPS */
extern BOOL LoadScreenShot(int nGame, int nType);
#endif /* USE_IPS */
#endif /* MESS */

extern HANDLE GetScreenShotHandle(void);
extern int GetScreenShotWidth(void);
extern int GetScreenShotHeight(void);

extern void FreeScreenShot(void);
extern BOOL ScreenShotLoaded(void);

extern BOOL LoadDIB(const WCHAR *filename, HGLOBAL *phDIB, HPALETTE *pPal, BOOL flyer);
extern HBITMAP DIBToDDB(HDC hDC, HANDLE hDIB, LPMYBITMAPINFO desc);

#endif
