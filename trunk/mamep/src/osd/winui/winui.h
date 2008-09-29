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

#if !defined(MAMEUINAME)
#define MAMEUINAME "MAMEUI"
#endif

#ifndef MESS
#ifdef PTR64
#define TEXT_MAMEUINAME	TEXT("MAMEUI")
#else
#define TEXT_MAMEUINAME	TEXT("MAMEUI")
#endif
#if !defined(MAMENAME)
#define MAMENAME	"MAME"
#endif
#else
#define MAMEUINAME	"MESSUI"
#define MAMENAME	"MESS"
#endif

#define SEARCH_PROMPT ""

enum
{
	TAB_PICKER = 0,
	TAB_DISPLAY,
	TAB_MISC,
	NUM_TABS
};

enum
{
	FILETYPE_INPUT_FILES = 1,
	FILETYPE_SAVESTATE_FILES = 2,
	FILETYPE_WAVE_FILES = 3,
	FILETYPE_AVI_FILES = 4,
	FILETYPE_MNG_FILES = 5,
	//mamep: export gamelist
	FILETYPE_GAMELIST_FILES = 6,
#if 0 //mamep: use standard combobox
	FILETYPE_EFFECT_FILES = 6,
#endif
	FILETYPE_JOYMAP_FILES = 7,
	FILETYPE_DEBUGSCRIPT_FILES = 8,
	FILETYPE_CHEAT_FILE = 9,
	FILETYPE_HISTORY_FILE = 10,
	FILETYPE_MAMEINFO_FILE = 11
#ifdef STORY_DATAFILE
	,FILETYPE_STORY_FILE = 12
#endif /* STORY_DATAFILE */
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

extern TCHAR last_directory[MAX_PATH];

typedef BOOL (WINAPI *common_file_dialog_proc)(LPOPENFILENAME lpofn);
BOOL CommonFileDialog(common_file_dialog_proc cfd,WCHAR *filename, int filetype);

HWND GetMainWindow(void);
HWND GetTreeView(void);
HIMAGELIST GetLargeImageList(void);
HIMAGELIST GetSmallImageList(void);
int GetNumOptionFolders(void);
void SetNumOptionFolders(int count);
void GetRealColumnOrder(int order[]);
HICON LoadIconFromFile(const char *iconname);
void UpdateScreenShot(void);
void ResizePickerControls(HWND hWnd);
void MamePlayGame(void);
int FindIconIndex(int nIconResource);
int FindIconIndexByName(const char *icon_name);
int GetSelectedPick(void);
object_pool *GetMameUIMemoryPool(void);
int GetNumGames(void);
#ifdef USE_VIEW_PCBINFO
void PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y);
#endif /* USE_VIEW_PCBINFO */

void UpdateListView(void);

// Move The in "The Title (notes)" to "Title, The (notes)"
char * ModifyThe(const char *str);

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

LPWSTR GetSearchText(void);

//mamep
#define UI_MSG_UI	UI_MSG_OSD1
#define UI_MSG_EXTRA	UI_MSG_OSD2

#undef _
#undef _LST
#undef _READINGS
#undef _MANUFACT
#undef _WINDOWS
#undef _UI

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
	#endif // for VC2005
#endif // _MSC_VER
