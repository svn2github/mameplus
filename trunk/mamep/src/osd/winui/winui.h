/***************************************************************************

  M.A.M.E.UI  -  Multiple Arcade Machine Emulator with User Interface
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse,
  Copyright (C) 2003-2007 Chris Kirmse and the MAME32/MAMEUI team.

  This file is part of MAMEUI, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef WINUI_H
#define WINUI_H

#define WIN32_LEAN_AND_MEAN
#include <commctrl.h>
#include <commdlg.h>
#include <driver.h>
#include "pool.h"
#include "screenshot.h"

#define SEARCH_PROMPT "type a keyword"

enum
{
	TAB_PICKER = 0,
	TAB_DISPLAY,
	TAB_MISC,
	NUM_TABS
};

typedef struct
{
	INT resource;
	const char *icon_name;
} ICONDATA;

struct _driverw
{
	WCHAR *name;
	WCHAR *description;
	WCHAR *modify_the;
	WCHAR *manufacturer;
	WCHAR *year;
	WCHAR *source_file;
};

/* in winui.c */
extern struct _driverw **driversw;


HWND GetMainWindow(void);
HWND GetTreeView(void);
#ifdef HIMAGELIST
HIMAGELIST GetLargeImageList(void);
HIMAGELIST GetSmallImageList(void);
#endif
int GetNumGames(void);
void GetRealColumnOrder(int order[]);
HICON LoadIconFromFile(const char *iconname);
void UpdateScreenShot(void);
void ResizePickerControls(HWND hWnd);
void MamePlayGame(void);
int FindIconIndex(int nIconResource);
int FindIconIndexByName(const char *icon_name);
LPWSTR GetSearchText(void);
#ifdef USE_VIEW_PCBINFO
void PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y);
#endif /* USE_VIEW_PCBINFO */

void UpdateListView(void);

// Convert Ampersand so it can display in a static control
LPWSTR ConvertAmpersandString(LPCWSTR s);

// globalized for painting tree control
HBITMAP GetBackgroundBitmap(void);
HPALETTE GetBackgroundPalette(void);
MYBITMAPINFO * GetBackgroundInfo(void);
BOOL GetUseOldControl(void);
BOOL GetUseXPControl(void);

int GetMinimumScreenShotWindowWidth(void);

// we maintain an array of drivers sorted by name, useful all around
int GetDriverIndex(const game_driver *driver);
int GetParentIndex(const game_driver *driver);
int GetParentRomSetIndex(const game_driver *driver);
int GetGameNameIndex(const char *name);
int GetIndexFromSortedIndex(int sorted_index);

// sets text in part of the status bar on the main window
void SetStatusBarText(int part_index, const WCHAR *message);
void SetStatusBarTextF(int part_index, const WCHAR *fmt, ...);

int MameUIMain(HINSTANCE	hInstance,
                   LPSTR    lpCmdLine,
                   int      nCmdShow);

BOOL MouseHasBeenMoved(void);
#endif
