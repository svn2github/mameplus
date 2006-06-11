/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

 /***************************************************************************

  win32ui.c

  Win32 GUI code.

  Created 8/12/97 by Christopher Kirmse (ckirmse@ricochet.net)
  Additional code November 1997 by Jeff Miller (miller@aa.net)
  More July 1998 by Mike Haaland (mhaaland@hypertech.com)
  Added Spitters/Property Sheets/Removed Tabs/Added Tree Control in
  Nov/Dec 1998 - Mike Haaland

***************************************************************************/
#define MULTISESSION 0

#ifdef _MSC_VER
#ifndef NONAMELESSUNION
#define NONAMELESSUNION 
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <string.h>
#include <sys/stat.h>
#include <wingdi.h>
#include <time.h>

#include "mame32.h"
#include "driver.h"
#include "osdepend.h"
#include "unzip.h"

#include "resource.h"
#include "resource.hm"

#include "screenshot.h"
#include "M32Util.h"
#include "file.h"
#include "audit32.h"
#include "Directories.h"
#include "Properties.h"
#include "ColumnEdit.h"
#include "picker.h"
#include "tabview.h"
#include "bitmask.h"
#include "TreeView.h"
#include "Splitters.h"
#include "help.h"
#include "history.h"
#include "options.h"
#include "dialogs.h"
#include "windows/input.h"
#include "windows/config.h"
#include "windows/window.h"
#include "PaletteEdit.h"
#include "translate.h"

#include "DirectDraw.h"
#include "DirectInput.h"
#include "DIJoystick.h"     /* For DIJoystick avalibility. */

#ifndef LVS_EX_LABELTIP
#define LVS_EX_LABELTIP         0x00004000 // listview unfolds partly hidden labels if it does not have infotip text
#endif // LVS_EX_LABELTIP

// fix warning: cast does not match function type
#if defined(__GNUC__) && defined(ListView_CreateDragImage)
#undef ListView_CreateDragImage
#endif

#ifndef ListView_CreateDragImage
#define ListView_CreateDragImage(hwnd, i, lpptUpLeft) \
    (HIMAGELIST)(LRESULT)(int)SendMessage((hwnd), LVM_CREATEDRAGIMAGE, (WPARAM)(int)(i), (LPARAM)(LPPOINT)(lpptUpLeft))
#endif // ListView_CreateDragImage

#ifndef TreeView_EditLabel
#define TreeView_EditLabel(w, i) \
    SNDMSG(w,TVM_EDITLABEL,0,(LPARAM)(i))
#endif // TreeView_EditLabel

#ifndef HDF_SORTUP
#define HDF_SORTUP 0x400
#endif // HDF_SORTUP

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN 0x200
#endif // HDF_SORTDOWN

#ifndef LVM_SETBKIMAGEA
#define LVM_SETBKIMAGEA         (LVM_FIRST + 68)
#endif // LVM_SETBKIMAGEA

#ifndef LVM_SETBKIMAGEW
#define LVM_SETBKIMAGEW         (LVM_FIRST + 138)
#endif // LVM_SETBKIMAGEW

#ifndef LVM_GETBKIMAGEA
#define LVM_GETBKIMAGEA         (LVM_FIRST + 69)
#endif // LVM_GETBKIMAGEA

#ifndef LVM_GETBKIMAGEW
#define LVM_GETBKIMAGEW         (LVM_FIRST + 139)
#endif // LVM_GETBKIMAGEW

#ifndef LVBKIMAGE

typedef struct tagLVBKIMAGEA
{
	ULONG ulFlags;
	HBITMAP hbm;
	LPSTR pszImage;
	UINT cchImageMax;
	int xOffsetPercent;
	int yOffsetPercent;
} LVBKIMAGEA, *LPLVBKIMAGEA;

typedef struct tagLVBKIMAGEW
{
	ULONG ulFlags;
	HBITMAP hbm;
	LPWSTR pszImage;
	UINT cchImageMax;
	int xOffsetPercent;
	int yOffsetPercent;
} LVBKIMAGEW, *LPLVBKIMAGEW;

#ifdef UNICODE
#define LVBKIMAGE               LVBKIMAGEW
#define LPLVBKIMAGE             LPLVBKIMAGEW
#define LVM_SETBKIMAGE          LVM_SETBKIMAGEW
#define LVM_GETBKIMAGE          LVM_GETBKIMAGEW
#else
#define LVBKIMAGE               LVBKIMAGEA
#define LPLVBKIMAGE             LPLVBKIMAGEA
#define LVM_SETBKIMAGE          LVM_SETBKIMAGEA
#define LVM_GETBKIMAGE          LVM_GETBKIMAGEA
#endif
#endif

#ifndef LVBKIF_SOURCE_NONE
#define LVBKIF_SOURCE_NONE      0x00000000
#endif // LVBKIF_SOURCE_NONE

#ifndef LVBKIF_SOURCE_HBITMAP
#define LVBKIF_SOURCE_HBITMAP   0x00000001
#endif

#ifndef LVBKIF_SOURCE_URL
#define LVBKIF_SOURCE_URL       0x00000002
#endif // LVBKIF_SOURCE_URL

#ifndef LVBKIF_SOURCE_MASK
#define LVBKIF_SOURCE_MASK      0x00000003
#endif // LVBKIF_SOURCE_MASK

#ifndef LVBKIF_STYLE_NORMAL
#define LVBKIF_STYLE_NORMAL     0x00000000
#endif // LVBKIF_STYLE_NORMAL

#ifndef LVBKIF_STYLE_TILE
#define LVBKIF_STYLE_TILE       0x00000010
#endif // LVBKIF_STYLE_TILE

#ifndef LVBKIF_STYLE_MASK
#define LVBKIF_STYLE_MASK       0x00000010
#endif // LVBKIF_STYLE_MASK

#ifndef ListView_SetBkImageA
#define ListView_SetBkImageA(hwnd, plvbki) \
    (BOOL)SNDMSG((hwnd), LVM_SETBKIMAGEA, 0, (LPARAM)(plvbki))
#endif // ListView_SetBkImageA

#ifndef ListView_GetBkImageA
#define ListView_GetBkImageA(hwnd, plvbki) \
    (BOOL)SNDMSG((hwnd), LVM_GETBKIMAGEA, 0, (LPARAM)(plvbki))
#endif // ListView_GetBkImageA

#define MM_PLAY_GAME (WM_APP + 15000)

#define JOYGUI_MS 100

#define JOYGUI_TIMER 1
#define SCREENSHOT_TIMER 2
#if MULTISESSION
#define GAMEWND_TIMER 3
#endif

/* Max size of a sub-menu */
#define DBU_MIN_WIDTH  292
#define DBU_MIN_HEIGHT 190

int MIN_WIDTH  = DBU_MIN_WIDTH;
int MIN_HEIGHT = DBU_MIN_HEIGHT;

/* Max number of bkground picture files */
#define MAX_BGFILES 100

#ifdef USE_IPS
#define MAX_PATCHES 64
#define MAX_PATCHNAME 64
#endif /* USE_IPS */

#define NO_FOLDER -1
#define STATESAVE_VERSION 1

enum
{
	FILETYPE_INPUT_FILES = 0,
	FILETYPE_SAVESTATE_FILES,
	FILETYPE_WAVE_FILES,
	FILETYPE_MNG_FILES,
	FILETYPE_IMAGE_FILES,
	FILETYPE_LST_FILES,
	FILETYPE_MAX
};

typedef BOOL (WINAPI *common_file_dialog_procW)(LPOPENFILENAMEW lpofn);
typedef BOOL (WINAPI *common_file_dialog_procA)(LPOPENFILENAMEA lpofn);


/***************************************************************************
 externally defined global variables
 ***************************************************************************/
#if 0
extern const char g_szPlayGameString[] = "&Play %s";
extern const char g_szGameCountString[] = "%d games";
extern const char *history_filename;
extern const char *mameinfo_filename;
#endif

/***************************************************************************
    function prototypes
 ***************************************************************************/

static BOOL             Win32UI_init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
static void             Win32UI_exit(void);

static BOOL             PumpMessage(void);
static BOOL             OnIdle(HWND hWnd);
static void             OnSize(HWND hwnd, UINT state, int width, int height);
static long WINAPI      MameWindowProc(HWND hwnd,UINT message,UINT wParam,LONG lParam);

static void             SetView(int menu_id);
static void             ResetListView(void);
static void             UpdateGameList(void);
static void             DestroyIcons(void);
static void             ReloadIcons(void);
static void             PollGUIJoystick(void);
#if 0
static void             PressKey(HWND hwnd, UINT vk);
#endif
static BOOL             MameCommand(HWND hwnd,int id, HWND hwndCtl, UINT codeNotify);
static void             KeyboardKeyDown(int syskey, int vk_code, int special);
static void             KeyboardKeyUp(int syskey, int vk_code, int special);
static void             KeyboardStateClear(void);

static void             UpdateStatusBar(void);
static BOOL             PickerHitTest(HWND hWnd);
static BOOL             TreeViewNotify(NMHDR *nm);

static void             ResetBackground(const char *szFile);
static void             RandomSelectBackground(void);
static void             LoadBackgroundBitmap(void);
#ifndef USE_VIEW_PCBINFO
static void             PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y);
#endif /* USE_VIEW_PCBINFO */

static int CLIB_DECL    DriverDataCompareFunc(const void *arg1,const void *arg2);
static int              GamePicker_Compare(HWND hwndPicker, int index1, int index2, int sort_subitem);

static void             DisableSelection(void);
static void             EnableSelection(int nGame);

static int              GetSelectedPick(void);
static HICON            GetSelectedPickItemIcon(void);
static void             SetRandomPickItem(void);
static void             PickColor(COLORREF *cDefault);

static LPTREEFOLDER     GetSelectedFolder(void);
static HICON            GetSelectedFolderIcon(void);

static LRESULT CALLBACK HistoryWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK PictureWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void             ChangeLanguage(int id);
static void             MamePlayRecordGame(void);
static void             MamePlayBackGame(const char* fname_playback);
static void             MamePlayRecordWave(void);
static void             MamePlayRecordMNG(void);
static void             MameLoadState(void);
static BOOL             CommonFileDialogW(BOOL open_for_write, char *filename, int filetype);
static BOOL             CommonFileDialogA(BOOL open_for_write, char *filename, int filetype);
static BOOL             CommonFileDialog(BOOL open_for_write,char *filename, int filetype);
static void             MamePlayGame(void);
static void             MamePlayGameWithOptions(int nGame);
static INT_PTR CALLBACK LoadProgressDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static int              UpdateLoadProgress(const char* name, const rom_load_data *romdata);
static BOOL             GameCheck(void);
static BOOL             FolderCheck(void);

static void             ToggleScreenShot(void);
static void             AdjustMetrics(void);
static void             EnablePlayOptions(int nIndex, options_type *o);

/* Icon routines */
static DWORD            GetShellLargeIconSize(void);
static DWORD            GetShellSmallIconSize(void);
static void             CreateIcons(void);
static int              GetIconForDriver(int nItem);
static void             AddDriverIcon(int nItem,int default_icon_index);

// Context Menu handlers
static void             UpdateMenu(HMENU hMenu);
static void             InitTreeContextMenu(HMENU hTreeMenu);
static void             ToggleShowFolder(int folder);
static BOOL             HandleTreeContextMenu( HWND hWnd, WPARAM wParam, LPARAM lParam);
static BOOL             HandleScreenShotContextMenu( HWND hWnd, WPARAM wParam, LPARAM lParam);
static void             GamePicker_OnHeaderContextMenu(POINT pt, int nColumn);
static void             GamePicker_OnBodyContextMenu(POINT pt);

static void             InitListView(void);
/* Re/initialize the ListView header columns */
static void             ResetColumnDisplay(BOOL first_time);

static void             CopyToolTipTextW(LPTOOLTIPTEXTW lpttt);
static void             CopyToolTipTextA(LPTOOLTIPTEXTA lpttt);

static void             ProgressBarShow(void);
static void             ProgressBarHide(void);
static void             ResizeProgressBar(void);
static void             ProgressBarStep(void);
static void             ProgressBarStepParam(int iGameIndex, int nGameCount);

static HWND             InitProgressBar(HWND hParent);
static HWND             InitToolbar(HWND hParent);
static HWND             InitStatusBar(HWND hParent);

static LRESULT          Statusbar_MenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam);

static BOOL             NeedScreenShotImage(void);
static BOOL             NeedHistoryText(void);
static void             UpdateHistory(void);


void RemoveCurrentGameCustomFolder(void);
void RemoveGameCustomFolder(int driver_index);

void BeginListViewDrag(NM_LISTVIEW *pnmv);
void MouseMoveListViewDrag(POINTS pt);
void ButtonUpListViewDrag(POINTS p);

void CalculateBestScreenShotRect(HWND hWnd, RECT *pRect, BOOL restrict_height);

BOOL MouseHasBeenMoved(void);
void SwitchFullScreenMode(void);

#ifdef USE_SHOW_SPLASH_SCREEN
static LRESULT CALLBACK BackMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void  CreateBackgroundMain(HINSTANCE hInstance);
static void  DestroyBackgroundMain(void);
#endif /* USE_SHOW_SPLASH_SCREEN */

// Game Window Communication Functions
#if MULTISESSION
BOOL SendMessageToEmulationWindow(UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL SendIconToEmulationWindow(int nGameIndex);
HWND GetGameWindow(void);
#else
void SendMessageToProcess(LPPROCESS_INFORMATION lpProcessInformation,
                                             UINT Msg, WPARAM wParam, LPARAM lParam);
void SendIconToProcess(LPPROCESS_INFORMATION lpProcessInformation, int nGameIndex);
HWND GetGameWindow(LPPROCESS_INFORMATION lpProcessInformation);
#endif

static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam);

static const char *GetLastDir(void);

/***************************************************************************
    External variables
 ***************************************************************************/

/***************************************************************************
    Internal structures
 ***************************************************************************/

/*
 * These next two structs represent how the icon information
 * is stored in an ICO file.
 */
typedef struct
{
	BYTE    bWidth;               /* Width of the image */
	BYTE    bHeight;              /* Height of the image (times 2) */
	BYTE    bColorCount;          /* Number of colors in image (0 if >=8bpp) */
	BYTE    bReserved;            /* Reserved */
	WORD    wPlanes;              /* Color Planes */
	WORD    wBitCount;            /* Bits per pixel */
	DWORD   dwBytesInRes;         /* how many bytes in this resource? */
	DWORD   dwImageOffset;        /* where in the file is this image */
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	UINT            Width, Height, Colors; /* Width, Height and bpp */
	LPBYTE          lpBits;                /* ptr to DIB bits */
	DWORD           dwNumBytes;            /* how many bytes? */
	LPBITMAPINFO    lpbi;                  /* ptr to header */
	LPBYTE          lpXOR;                 /* ptr to XOR image bits */
	LPBYTE          lpAND;                 /* ptr to AND image bits */
} ICONIMAGE, *LPICONIMAGE;

/* Which edges of a control are anchored to the corresponding side of the parent window */
#define RA_LEFT     0x01
#define RA_RIGHT    0x02
#define RA_TOP      0x04
#define RA_BOTTOM   0x08
#define RA_ALL      0x0F

#define RA_END  0
#define RA_ID   1
#define RA_HWND 2

typedef struct
{
	int         type;       /* Either RA_ID or RA_HWND, to indicate which member of u is used; or RA_END
				   to signify last entry */
	union                   /* Can identify a child window by control id or by handle */
	{
		int     id;         /* Window control id */
		HWND    hwnd;       /* Window handle */
	} u;
	BOOL		setfont;	/* Do we set this item's font? */
	int         action;     /* What to do when control is resized */
	void        *subwindow; /* Points to a Resize structure for this subwindow; NULL if none */
} ResizeItem;

typedef struct
{
	RECT        rect;       /* Client rect of window; must be initialized before first resize */
	ResizeItem* items;      /* Array of subitems to be resized */
} Resize;

static void ResizeWindow(HWND hParent, Resize *r);
static void SetAllWindowsFont(HWND hParent, const Resize *r, HFONT hFont, BOOL bRedraw);

/* List view Icon defines */
#define LG_ICONMAP_WIDTH    GetSystemMetrics(SM_CXICON)
#define LG_ICONMAP_HEIGHT   GetSystemMetrics(SM_CYICON)
#define ICONMAP_WIDTH       GetSystemMetrics(SM_CXSMICON)
#define ICONMAP_HEIGHT      GetSystemMetrics(SM_CYSMICON)

/*
typedef struct tagPOPUPSTRING
{
	HMENU hMenu;
	UINT uiString;
} POPUPSTRING;

#define MAX_MENUS 3
*/

#define SPLITTER_WIDTH	4
#define MIN_VIEW_WIDTH	10

// Struct needed for Game Window Communication

typedef struct
{
	LPPROCESS_INFORMATION ProcessInfo;
	HWND hwndFound;
} FINDWINDOWHANDLE;

/***************************************************************************
    Internal variables
 ***************************************************************************/

static HWND   hMain  = NULL;
static HACCEL hAccel = NULL;

static HWND hwndList  = NULL;
static HWND hTreeView = NULL;
static HWND hProgWnd  = NULL;
static HWND hTabCtrl  = NULL;

static BOOL g_bAbortLoading = FALSE; /* doesn't work right */
static BOOL g_bCloseLoading = FALSE;

static HINSTANCE hInst = NULL;

static HFONT hFont = NULL;     /* Font for list view */

static int game_count = 0;

/* global data--know where to send messages */
static BOOL in_emulation;

/* idle work at startup */
static BOOL idle_work;

static int  game_index;
static int  progBarStep;

static BOOL bDoGameCheck = FALSE;

/* Tree control variables */
static BOOL bShowTree      = 1;
static BOOL bShowToolBar   = 1;
static BOOL bShowStatusBar = 1;
static BOOL bShowTabCtrl   = 1;
static BOOL bProgressShown = FALSE;
static BOOL bListReady     = FALSE;

/* use a joystick subsystem in the gui? */
static struct OSDJoystick* g_pJoyGUI = NULL;

/* store current keyboard state (in internal codes) here */
static input_code keyboard_state[ __code_max ]; /* __code_max #defines the number of internal key_codes */

/* search */
static WCHAR g_SearchText[256];

/* table copied from windows/inputs.c */
// table entry indices
#define MAME_KEY		0
#define DI_KEY			1
#define VIRTUAL_KEY		2
#define ASCII_KEY		3

typedef struct
{
	char		name[40];	    // functionality name (optional)
	input_seq	is;				// the input sequence (the keys pressed)
	UINT		func_id;        // the identifier
	input_seq* (*getiniptr)(void);// pointer to function to get the value from .ini file
} GUISequence;

static GUISequence GUISequenceControl[]=
{
	{"gui_key_up",                SEQ_DEF_0,    ID_UI_UP,           Get_ui_key_up },
	{"gui_key_down",              SEQ_DEF_0,    ID_UI_DOWN,         Get_ui_key_down },
	{"gui_key_left",              SEQ_DEF_0,    ID_UI_LEFT,         Get_ui_key_left },
	{"gui_key_right",             SEQ_DEF_0,    ID_UI_RIGHT,        Get_ui_key_right },
	{"gui_key_start",             SEQ_DEF_0,    ID_UI_START,        Get_ui_key_start },
	{"gui_key_pgup",              SEQ_DEF_0,    ID_UI_PGUP,         Get_ui_key_pgup },
	{"gui_key_pgdwn",             SEQ_DEF_0,    ID_UI_PGDOWN,       Get_ui_key_pgdwn },
	{"gui_key_home",              SEQ_DEF_0,    ID_UI_HOME,         Get_ui_key_home },
	{"gui_key_end",               SEQ_DEF_0,    ID_UI_END,          Get_ui_key_end },
	{"gui_key_ss_change",         SEQ_DEF_0,    IDC_SSFRAME,        Get_ui_key_ss_change },
	{"gui_key_history_up",        SEQ_DEF_0,    ID_UI_HISTORY_UP,   Get_ui_key_history_up },
	{"gui_key_history_down",      SEQ_DEF_0,    ID_UI_HISTORY_DOWN, Get_ui_key_history_down },
	
	{"gui_key_context_filters",    SEQ_DEF_0,    ID_CONTEXT_FILTERS,       Get_ui_key_context_filters },
	{"gui_key_select_random",      SEQ_DEF_0,    ID_CONTEXT_SELECT_RANDOM, Get_ui_key_select_random },
	{"gui_key_game_audit",         SEQ_DEF_0,    ID_GAME_AUDIT,            Get_ui_key_game_audit },
	{"gui_key_game_properties",    SEQ_DEF_0,    ID_GAME_PROPERTIES,       Get_ui_key_game_properties },
	{"gui_key_help_contents",      SEQ_DEF_0,    ID_HELP_CONTENTS,         Get_ui_key_help_contents },
	{"gui_key_update_gamelist",    SEQ_DEF_0,    ID_UPDATE_GAMELIST,       Get_ui_key_update_gamelist },
	{"gui_key_view_folders",       SEQ_DEF_0,    ID_VIEW_FOLDERS,          Get_ui_key_view_folders },
	{"gui_key_view_fullscreen",    SEQ_DEF_0,    ID_VIEW_FULLSCREEN,       Get_ui_key_view_fullscreen },
	{"gui_key_view_pagetab",       SEQ_DEF_0,    ID_VIEW_PAGETAB,          Get_ui_key_view_pagetab },
	{"gui_key_view_picture_area",  SEQ_DEF_0,    ID_VIEW_PICTURE_AREA,     Get_ui_key_view_picture_area },
	{"gui_key_view_status",        SEQ_DEF_0,    ID_VIEW_STATUS,           Get_ui_key_view_status },
	{"gui_key_view_toolbars",      SEQ_DEF_0,    ID_VIEW_TOOLBARS,         Get_ui_key_view_toolbars },

	{"gui_key_view_tab_cabinet",     SEQ_DEF_0,  ID_VIEW_TAB_CABINET,       Get_ui_key_view_tab_cabinet },
	{"gui_key_view_tab_cpanel",      SEQ_DEF_0,  ID_VIEW_TAB_CONTROL_PANEL, Get_ui_key_view_tab_cpanel },
	{"gui_key_view_tab_flyer",       SEQ_DEF_0,  ID_VIEW_TAB_FLYER,         Get_ui_key_view_tab_flyer },
	{"gui_key_view_tab_history",     SEQ_DEF_0,  ID_VIEW_TAB_HISTORY,       Get_ui_key_view_tab_history },
#ifdef STORY_DATAFILE
	{"gui_key_view_tab_story",       SEQ_DEF_0,  ID_VIEW_TAB_STORY,         Get_ui_key_view_tab_story },
#endif /* STORY_DATAFILE */
	{"gui_key_view_tab_marquee",     SEQ_DEF_0,  ID_VIEW_TAB_MARQUEE,       Get_ui_key_view_tab_marquee },
	{"gui_key_view_tab_screenshot",  SEQ_DEF_0,  ID_VIEW_TAB_SCREENSHOT,    Get_ui_key_view_tab_screenshot },
	{"gui_key_view_tab_title",       SEQ_DEF_0,  ID_VIEW_TAB_TITLE,         Get_ui_key_view_tab_title },
	{"gui_key_quit",                 SEQ_DEF_0,  ID_FILE_EXIT,              Get_ui_key_quit },
};


#define NUM_GUI_SEQUENCES (sizeof(GUISequenceControl) / sizeof(GUISequenceControl[0]))


static UINT    lastColumnClick   = 0;
static WNDPROC g_lpHistoryWndProc = NULL;
static WNDPROC g_lpPictureFrameWndProc = NULL;
static WNDPROC g_lpPictureWndProc = NULL;

/*
static POPUPSTRING popstr[MAX_MENUS + 1];
*/

/* Tool and Status bar variables */
static HWND hStatusBar = 0;
static HWND hToolBar   = 0;

/* Column Order as Displayed */
static BOOL oldControl = FALSE;
static BOOL xpControl = FALSE;

/* Used to recalculate the main window layout */
static int  bottomMargin;
static int  topMargin;
static int  have_history = FALSE;
static RECT history_rect;

static BOOL have_selection = FALSE;

static HBITMAP hMissing_bitmap;

/* Icon variables */
static HIMAGELIST   hLarge = NULL;
static HIMAGELIST   hSmall = NULL;
static HIMAGELIST   hHeaderImages = NULL;
static int          *icon_index = NULL; /* for custom per-game icons */

static TBBUTTON tbb[] =
{
	{0, ID_VIEW_FOLDERS,     TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 0},
	{1, ID_VIEW_PICTURE_AREA,TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 1},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{2, ID_VIEW_LARGE_ICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 2},
	{3, ID_VIEW_SMALL_ICON,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 3},
	{4, ID_VIEW_LIST_MENU,   TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 4},
	{5, ID_VIEW_DETAIL,      TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 5},
	{6, ID_VIEW_GROUPED,     TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, {0, 0}, 0, 6},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{9, IDC_USE_LIST,        TBSTATE_ENABLED, TBSTYLE_CHECK,      {0, 0}, 0, 9},
	{0, 0,                   TBSTATE_ENABLED, TBSTYLE_SEP,        {0, 0}, 0, 0},
	{7, ID_HELP_ABOUT,       TBSTATE_ENABLED, TBSTYLE_BUTTON,     {0, 0}, 0, 7},
	{8, ID_HELP_CONTENTS,    TBSTATE_ENABLED, TBSTYLE_BUTTON,     {0, 0}, 0, 8}
};

#define NUM_TOOLBUTTONS (sizeof(tbb) / sizeof(tbb[0]))

#define NUM_TOOLTIPS 8 + 1

static char szTbStrings[NUM_TOOLTIPS + 1][30] =
{
	"Toggle Folder List",
	"Toggle Screen Shot",
	"Large Icons",
	"Small Icons",
	"List",
	"Details",
	"Grouped",
	"About",
	"Help",
	"Use Local Language Game List"
};

static int CommandToString[] =
{
	ID_VIEW_FOLDERS,
	ID_VIEW_PICTURE_AREA,
	ID_VIEW_LARGE_ICON,
	ID_VIEW_SMALL_ICON,
	ID_VIEW_LIST_MENU,
	ID_VIEW_DETAIL,
	ID_VIEW_GROUPED,
	ID_HELP_ABOUT,
	ID_HELP_CONTENTS,
	IDC_USE_LIST,
	-1
};

/* How to resize main window */
static ResizeItem main_resize_items[] =
{
	{ RA_HWND, { 0 },            FALSE, RA_LEFT  | RA_RIGHT  | RA_TOP,     NULL },
	{ RA_HWND, { 0 },            FALSE, RA_LEFT  | RA_RIGHT  | RA_BOTTOM,  NULL },
	{ RA_ID,   { IDC_DIVIDER },  FALSE, RA_LEFT  | RA_RIGHT  | RA_TOP,     NULL },
	{ RA_ID,   { IDC_TREE },     TRUE,	RA_LEFT  | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_LIST },     TRUE,	RA_ALL,                            NULL },
	{ RA_ID,   { IDC_SPLITTER }, FALSE,	RA_LEFT  | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SPLITTER2 },FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSFRAME },  FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSPICTURE },FALSE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_HISTORY },  TRUE,	RA_RIGHT | RA_BOTTOM | RA_TOP,     NULL },
	{ RA_ID,   { IDC_SSTAB },    FALSE,	RA_RIGHT | RA_TOP,                 NULL },
	{ RA_END,  { 0 },            FALSE, 0,                                 NULL }
};

static Resize main_resize = { {0, 0, 0, 0}, main_resize_items };

/* our dialog/configured options */
static options_type playing_game_options;

/* last directory for common file dialogs */
static char last_directory[MAX_PATH];

/* system-wide window message sent out with an ATOM of the current game name
   each time it changes */
static UINT g_mame32_message = 0;
static BOOL g_bDoBroadcast   = FALSE;

static BOOL use_gui_romloading = FALSE;

static BOOL g_listview_dragging = FALSE;
HIMAGELIST himl_drag;
int game_dragged; /* which game started the drag */
HTREEITEM prev_drag_drop_target; /* which tree view item we're currently highlighting */

static BOOL g_in_treeview_edit = FALSE;

#ifdef USE_IPS
static char *g_IPSMenuSelectName;
#endif /* USE_IPS */

typedef struct
{
	const char *name;
	int index;
} driver_data_type;
static driver_data_type *sorted_drivers;

static char * g_pRecordName = NULL;
static char * g_pPlayBkName = NULL;
static char * g_pSaveStateName = NULL;
static char * g_pRecordWaveName = NULL;
static char * g_pRecordMNGName = NULL;
static char * override_playback_directory = NULL;
static char * override_savestate_directory = NULL;

static struct
{
	const char *filter;
	const char *title_load;
	const char *title_save;
	const char *(*dir)(void);
	const char *ext;
} cfg_data[FILETYPE_MAX] =
{
	{
		MAMENAME " input files (*.inp,*.zip)\0*.inp;*.zip\0All files (*.*)\0*.*\0",
		"Select a recorded file",
		"Select a file to record",
		GetInpDir,
		"inp"
	},
	{
		MAMENAME " savestate files (*.sta)\0*.sta;\0All files (*.*)\0*.*\0",
		"Select a savestate file",
		NULL,
		GetStateDir,
		"sta"
	},
	{
		"Sounds (*.wav)\0*.wav;\0All files (*.*)\0*.*\0",
		NULL,
		"Select a sound file to record",
		GetLastDir,
		"wav"
	},
	{
		"Videos (*.mng)\0*.mng;\0All files (*.*)\0*.*\0",
		NULL,
		"Select a mng file to record",
		GetLastDir,
		"mng"
	},
	{
		"Image Files (*.png,*.bmp)\0*.png;*.bmp\0",
		"Select a Background Image",
		NULL,
		GetBgDir,
		"png"
	},
	{
		MAME32NAME " game list files (*.lst)\0*.lst;\0All files (*.*)\0*.*\0",
		NULL,
		"Select a game list file to export",
		GetLastDir,
		"lst"
	},
};


/***************************************************************************
    Global variables
 ***************************************************************************/

/* Background Image handles also accessed from TreeView.c */
static HPALETTE         hPALbg   = 0;
static HBITMAP          hBackground  = 0;
static MYBITMAPINFO     bmDesc;

#ifdef USE_SHOW_SPLASH_SCREEN
static HWND             hBackMain = NULL;
static HBITMAP          hSplashBmp = 0;
static HDC              hMemoryDC;
#endif /* USE_SHOW_SPLASH_SCREEN */

/* List view Column text */
const char* column_names[COLUMN_MAX] =
{
	"Description",
	"ROMs",
	"Samples",
	"Name",
	"Type",
	"Trackball",
	"Playcount",
	"Manufacturer",
	"Year",
	"Clone Of",
	"Driver",
	"Play Time"
};

/***************************************************************************
    Message Macros
 ***************************************************************************/

#ifndef StatusBar_GetItemRect
#define StatusBar_GetItemRect(hWnd, iPart, lpRect) \
    SendMessage(hWnd, SB_GETRECT, (WPARAM) iPart, (LPARAM) (LPRECT) lpRect)
#endif

#ifndef ToolBar_CheckButton
#define ToolBar_CheckButton(hWnd, idButton, fCheck) \
    SendMessage(hWnd, TB_CHECKBUTTON, (WPARAM)idButton, (LPARAM)MAKELONG(fCheck, 0))
#endif

/***************************************************************************
    External functions
 ***************************************************************************/

static int GetTotalPathLen(void)
{
	int len = 0;

	len += strlen(GetRomDirs());
	len += strlen(GetSampleDirs());
	len += strlen(GetIniDir());
	len += strlen(GetCfgDir());
	len += strlen(GetNvramDir());
	len += strlen(GetMemcardDir());

	if (override_playback_directory != NULL)
		len += strlen(override_playback_directory);
	else
		len += strlen(GetInpDir());

	len += strlen(GetHiDir());
	len += strlen(GetStateDir());
	len += strlen(GetArtDir());
	len += strlen(GetImgDir());
	len += strlen(GetDiffDir());
	len += strlen(GetCheatFile());
	len += strlen(GetHistoryFile());
#ifdef STORY_DATAFILE
	len += strlen(GetStoryFile());
#endif /* STORY_DATAFILE */
	len += strlen(GetMAMEInfoFile());
	len += strlen(GetHiscoreFile());
	len += strlen(GetCtrlrDir());
#ifdef USE_IPS
	len += strlen(GetPatchDir());
#endif /* USE_IPS */
	len += strlen(GetLangDir());

	if (g_pPlayBkName != NULL)
		len += strlen(g_pPlayBkName);
	if (g_pRecordName != NULL)
		len += strlen(g_pRecordName);

	return len;
}

static void CreateCommandLine(int nGameIndex, char* pCmdLine)
{
	char pModule[_MAX_PATH];
	options_type* pOpts;

	// this command line can grow too long for win9x, so we try to not send
	// some default values

	GetModuleFileNameA(GetModuleHandle(NULL), pModule, _MAX_PATH);

	pOpts = GetGameOptions(nGameIndex);

	sprintf(pCmdLine, "\"%s\" %s -norc", pModule, drivers[nGameIndex]->name);

	sprintf(&pCmdLine[strlen(pCmdLine)], " -rp \"%s\"",            GetRomDirs());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -sp \"%s\"",         GetSampleDirs());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -inipath \"%s\"",			GetIniDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cfg_directory \"%s\"",      GetCfgDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -nvram_directory \"%s\"",    GetNvramDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -memcard_directory \"%s\"",  GetMemcardDir());

	if (override_playback_directory != NULL)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -input_directory \"%s\"",    override_playback_directory);
	}
	else
		sprintf(&pCmdLine[strlen(pCmdLine)], " -input_directory \"%s\"",    GetInpDir());

	sprintf(&pCmdLine[strlen(pCmdLine)], " -hiscore_directory \"%s\"",  GetHiDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -state_directory \"%s\"",    GetStateDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -artwork_directory \"%s\"",  GetArtDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -snapshot_directory \"%s\"", GetImgDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -diff_directory \"%s\"",     GetDiffDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cheat_file \"%s\"",         GetCheatFile());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ctrlr_directory \"%s\"",    GetCtrlrDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -comment_directory \"%s\"",  GetCommentDir());
#ifdef USE_IPS
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ips_directory \"%s\"",      GetPatchDir());
#endif /* USE_IPS */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -lang_directory \"%s\"",     GetLangDir());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -history_file \"%s\"",       GetHistoryFile());
#ifdef STORY_DATAFILE
	sprintf(&pCmdLine[strlen(pCmdLine)], " -story_file \"%s\"",         GetStoryFile());
#endif /* STORY_DATAFILE */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -mameinfo_file \"%s\"",      GetMAMEInfoFile());
	sprintf(&pCmdLine[strlen(pCmdLine)], " -hiscore_file \"%s\"",       GetHiscoreFile());

	/* video */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sautoframeskip",           pOpts->autoframeskip   ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -frameskip %d",              pOpts->frameskip);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%swaitvsync",               pOpts->waitvsync      ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%striplebuffer",            pOpts->triplebuffer ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%swindow",                  pOpts->window     ? "" : "no");
#ifndef NEW_RENDER
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdd",                      pOpts->ddraw       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%shwstretch",               pOpts->hwstretch   ? "" : "no");
#endif
	if (pOpts->screen0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -screen0 %s",                pOpts->screen0); 
	sprintf(&pCmdLine[strlen(pCmdLine)], " -resolution0 %s",            pOpts->resolution0);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -aspect0 %s",                pOpts->aspect0);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sswitchres",               pOpts->switchres       ? "" : "no");
#ifndef NEW_RENDER
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sswitchbpp",               pOpts->switchbpp       ? "" : "no");
#endif
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%smaximize",                pOpts->maximize        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -numscreens %d",            pOpts->numscreens);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssyncrefresh",             pOpts->syncrefresh     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sthrottle",                pOpts->throttle        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -full_screen_gamma %f",      pOpts->full_screen_gamma);
//	sprintf(&pCmdLine[strlen(pCmdLine)], " -frames_to_run %d",          pOpts->frames_to_run);
#ifndef NEW_RENDER
	sprintf(&pCmdLine[strlen(pCmdLine)], " -effect %s",                 pOpts->effect);
#endif
#ifdef USE_SCALE_EFFECTS
	sprintf(&pCmdLine[strlen(pCmdLine)], " -scale_effect %s",           pOpts->scale_effect);
#endif /* USE_SCALE_EFFECTS */

	// d3d
	if (pOpts->direct3d)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -direct3d");
#ifndef NEW_RENDER
		sprintf(&pCmdLine[strlen(pCmdLine)], " -zoom %i", pOpts->zoom);
#endif
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sfilter",pOpts->filter?"":"no");
#ifndef NEW_RENDER
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sd3dtexmanage",    pOpts->d3dtexmanage ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3deffect %s",      pOpts->d3deffect);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dprescale %s",    pOpts->d3dprescale);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sd3deffectrotate", pOpts->d3deffectrotate ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dscan %i",        pOpts->d3dscan);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -d3dfeedback %i",    pOpts->d3dfeedback);
#endif
	}
	else
		sprintf(&pCmdLine[strlen(pCmdLine)], " -nodirect3d");

#ifdef UI_COLOR_DISPLAY
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_blank \"%s\"",         GetUIPaletteString(FONT_COLOR_BLANK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_normal \"%s\"",        GetUIPaletteString(FONT_COLOR_NORMAL));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -font_special \"%s\"",       GetUIPaletteString(FONT_COLOR_SPECIAL));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_background \"%s\"",  GetUIPaletteString(SYSTEM_COLOR_BACKGROUND));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framemedium \"%s\"", GetUIPaletteString(SYSTEM_COLOR_FRAMEMEDIUM));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framelight \"%s\"",  GetUIPaletteString(SYSTEM_COLOR_FRAMELIGHT));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -system_framedark \"%s\"",   GetUIPaletteString(SYSTEM_COLOR_FRAMEDARK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framemedium \"%s\"", GetUIPaletteString(OSDBAR_COLOR_FRAMEMEDIUM));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framelight \"%s\"",  GetUIPaletteString(OSDBAR_COLOR_FRAMELIGHT));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_framedark \"%s\"",   GetUIPaletteString(OSDBAR_COLOR_FRAMEDARK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -osdbar_defaultbar \"%s\"",  GetUIPaletteString(OSDBAR_COLOR_DEFAULTBAR));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_red \"%s\"",         GetUIPaletteString(BUTTON_COLOR_RED));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_yellow \"%s\"",      GetUIPaletteString(BUTTON_COLOR_YELLOW));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_green \"%s\"",       GetUIPaletteString(BUTTON_COLOR_GREEN));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_blue \"%s\"",        GetUIPaletteString(BUTTON_COLOR_BLUE));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_purple \"%s\"",      GetUIPaletteString(BUTTON_COLOR_PURPLE));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_pink \"%s\"",        GetUIPaletteString(BUTTON_COLOR_PINK));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_aqua \"%s\"",        GetUIPaletteString(BUTTON_COLOR_AQUA));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_silver \"%s\"",      GetUIPaletteString(BUTTON_COLOR_SILVER));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_navy \"%s\"",        GetUIPaletteString(BUTTON_COLOR_NAVY));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -button_lime \"%s\"",        GetUIPaletteString(BUTTON_COLOR_LIME));
	sprintf(&pCmdLine[strlen(pCmdLine)], " -cursor \"%s\"",             GetUIPaletteString(CURSOR_COLOR));
#endif /* UI_COLOR_DISPLAY */

	/* input */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%smouse",                   pOpts->mouse       ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sjoystick",                pOpts->joystick    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -a2d_deadzone %f",           pOpts->a2d_deadzone);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssteady",                  pOpts->steadykey       ? "" : "no");

	sprintf(&pCmdLine[strlen(pCmdLine)], " -%slightgun",                pOpts->lightgun        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdual_lightgun",           pOpts->dual_lightgun ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%soffscreen_reload",        pOpts->offscreen_reload ? "" : "no");

#ifdef USE_JOY_MOUSE_MOVE
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sstickpoint",              pOpts->stickpoint  ? "" : "no");
#endif /* USE_JOY_MOUSE_MOVE */
#ifdef JOYSTICK_ID
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid1 %d",                 pOpts->joyid1);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid2 %d",                 pOpts->joyid2);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid3 %d",                 pOpts->joyid3);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid4 %d",                 pOpts->joyid4);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid5 %d",                 pOpts->joyid5);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid6 %d",                 pOpts->joyid6);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid7 %d",                 pOpts->joyid7);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -joyid8 %d",                 pOpts->joyid8);
#endif /* JOYSTICK_ID */

	if (pOpts->ctrlr && strlen(pOpts->ctrlr) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -ctrlr \"%s\"",              pOpts->ctrlr);
	if (strlen(pOpts->digital) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -digital \"%s\"",            pOpts->digital);

	/* controller mapping*/
	if (strlen(pOpts->paddle_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -paddle_device \"%s\"",      pOpts->paddle_device);
	if (strlen(pOpts->adstick_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -adstick_device \"%s\"",     pOpts->adstick_device);
	if (strlen(pOpts->pedal_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -pedal_device \"%s\"",       pOpts->pedal_device);
	if (strlen(pOpts->dial_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -dial_device \"%s\"",        pOpts->dial_device);
	if (strlen(pOpts->trackball_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -trackball_device \"%s\"",   pOpts->trackball_device);
	if (strlen(pOpts->lightgun_device) > 0)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -lightgun_device \"%s\"",    pOpts->lightgun_device);


	/* core video */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -brightness %f",             pOpts->brightness); 
	sprintf(&pCmdLine[strlen(pCmdLine)], " -pause_brightness %f",       pOpts->pause_brightness); 
	//if (pOpts->norotate)
	//	sprintf(&pCmdLine[strlen(pCmdLine)], " -%snorotate",                pOpts->norotate ? "" : "no");
#ifndef NEW_RENDER
	if (pOpts->cleanstretch)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -cleanstretch %s",       pOpts->cleanstretch);
	if (pOpts->keepaspect)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%skeepaspect",              pOpts->keepaspect      ? "" : "no");
	if (pOpts->scanlines)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%sscanlines",               pOpts->scanlines       ? "" : "no");
	if (pOpts->matchrefresh)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%smatchrefresh",            pOpts->matchrefresh    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -refresh %d",                pOpts->refresh);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -gamma %f",                  pOpts->gamma);
#endif
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sror",                     pOpts->ror ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%srol",                     pOpts->rol ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sautoror",                 pOpts->autoror ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sautorol",                 pOpts->autorol ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sflipx",                   pOpts->flipx ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sflipy",                   pOpts->flipy ? "" : "no");

	/* vector */
	if (DriverIsVector(nGameIndex))
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%santialias",               pOpts->antialias       ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -beam %f",                   pOpts->beam);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -flicker %f",                pOpts->flicker);
		sprintf(&pCmdLine[strlen(pCmdLine)], " -intensity %f",              pOpts->intensity);
	}

	/* sound */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -samplerate %d",             pOpts->samplerate);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssamples",                 pOpts->samples     ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssound",                   pOpts->sound    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -volume %d",                 pOpts->volume);
#ifdef USE_VOLUME_AUTO_ADJUST
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%svolume_adjust",           pOpts->volume_adjust ? "" : "no");
#endif /* USE_VOLUME_AUTO_ADJUST */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -audio_latency %i",          pOpts->audio_latency);

	/* misc artwork options */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sartwork",                 pOpts->artwork     ? "" : "no");
	if (pOpts->artwork == TRUE)
	{
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_backdrops",           pOpts->use_backdrops       ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_overlays",            pOpts->use_overlays        ? "" : "no");
		sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_bezels",              pOpts->use_bezels          ? "" : "no");
	}

	/* misc */
//	sprintf(&pCmdLine[strlen(pCmdLine)], " -%svalidate",                pOpts->validate      ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sc",                       pOpts->cheat          ? "" : "no");
	//if (pOpts->mame_debug)
	//	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdebug",                   pOpts->mame_debug     ? "" : "no");
	if (g_pPlayBkName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -pb \"%s\"",                 g_pPlayBkName);
	if (g_pRecordName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -rec \"%s\"",                g_pRecordName);
	if (g_pRecordWaveName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -wavwrite \"%s\"",           g_pRecordWaveName);
	if (g_pRecordMNGName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -mngwrite \"%s\"",           g_pRecordMNGName);
	if (g_pSaveStateName != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -state \"%s\"",              g_pSaveStateName);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%slog",                     pOpts->log        ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%ssleep",                   pOpts->sleep           ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%srdtsc",                   pOpts->rdtsc      ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%skeyboard_leds",           pOpts->keyboard_leds            ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sskip_gameinfo",           pOpts->skip_gameinfo   ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -priority %i",               pOpts->priority);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sautosave",                pOpts->autosave        ? "" : "no");

	if (DriverHasOptionalBIOS(nGameIndex))
		sprintf(&pCmdLine[strlen(pCmdLine)], " -bios %s",                   pOpts->bios);		

#ifdef USE_IPS
	if (pOpts->ips != NULL)
		sprintf(&pCmdLine[strlen(pCmdLine)], " -ips %s",                    pOpts->ips);
#endif /* USE_IPS */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sdisable_second_monitor",  pOpts->disable_second_monitor    ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sconfirm_quit",            pOpts->confirm_quit           ? "" : "no");
#ifdef AUTO_PAUSE_PLAYBACK
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%sauto_pause_playback",     pOpts->auto_pause_playback    ? "" : "no");
#endif /* AUTO_PAUSE_PLAYBACK */
#ifdef TRANS_UI
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_trans_ui",            pOpts->use_trans_ui            ? "" : "no");
	sprintf(&pCmdLine[strlen(pCmdLine)], " -ui_transparency %d",        pOpts->ui_transparency);
#endif /* TRANS_UI */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	sprintf(&pCmdLine[strlen(pCmdLine)], " -m68k_core %d",              pOpts->m68k_core);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */

	/* langcode */
	sprintf(&pCmdLine[strlen(pCmdLine)], " -language %s",               ui_lang_info[GetLangcode()].name);
	sprintf(&pCmdLine[strlen(pCmdLine)], " -%suse_lang_list",           UseLangList()                 ? "" : "no");

//	dprintf("Launching MAME32:");
//	dprintf("%s",pCmdLine);
}

static BOOL WaitWithMessageLoop(HANDLE hEvent)
{
	DWORD dwRet;
	MSG   msg;

	while (1)
	{
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

		if (dwRet == WAIT_OBJECT_0)
			return TRUE;

		if (dwRet != WAIT_OBJECT_0 + 1)
			break;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
				return TRUE;
		}
	}
	return FALSE;
}

static int RunMAME(int nGameIndex)
{
	time_t start, end;
	double elapsedtime;

#if MULTISESSION
	int exit_code = 0;
	int argc = 0;
	const char *argv[256];
	char *pCmdLine = NULL;

#if 0
// load settings from ini files
	char pModule[_MAX_PATH];
	char game_name[500];

	SaveOptions();

	GetModuleFileNameA(GetModuleHandle(NULL), pModule, _MAX_PATH);
	argv[0] = pModule;
	strcpy(game_name,drivers[nGameIndex]->name);
	argv[argc++] = pModule;
	argv[argc++] = drivers[nGameIndex]->name;

	if (override_playback_directory != NULL)
	{
		argv[argc++] = "-input_directory";
		argv[argc++] = override_playback_directory;
	}
	if (g_pPlayBkName != NULL)
	{
		argv[argc++] = "-pb";
		argv[argc++] = g_pPlayBkName;
	}
	if (g_pRecordName != NULL)
	{
		argv[argc++] = "-rec";
		argv[argc++] = g_pRecordName;
	}
	if (g_pRecordWaveName != NULL)
	{
		argv[argc++] = "-wavwrite";
		argv[argc++] = g_pRecordWaveName;
	}
	if (g_pRecordMNGName != NULL)
	{
		argv[argc++] = "-mngwrite";
		argv[argc++] = g_pRecordMNGName;
	}
	if (g_pSaveStateName != NULL)
	{
		argv[argc++] = "-state";
		argv[argc++] = g_pSaveStateName;
	}
#else
// pass settings via cmd-line
	pCmdLine = malloc(GetTotalPathLen() + 2500);
	if (!pCmdLine)
	{
		OutputDebugStringA("Out of memory.");
		return 1;
	}

	CreateCommandLine(nGameIndex, pCmdLine);

	while (argc < 256 - 1)
	{
		while (*pCmdLine == ' ')
			pCmdLine++;

		if (*pCmdLine == '\0')
			break;

		if (*pCmdLine == '\"')
		{
			argv[argc++] = ++pCmdLine;

			for (; *pCmdLine != '\0'; pCmdLine++)
				if (*pCmdLine == '\"')
					break;

			if (*pCmdLine != '\0')
				*pCmdLine++ = '\0';

			continue;
		}

		argv[argc++] = pCmdLine;

		for (; *pCmdLine != '\0'; pCmdLine++)
			if (*pCmdLine == ' ')
				break;

		if (*pCmdLine != '\0')
			*pCmdLine++ = '\0';

		continue;
	}
	*pCmdLine = '\0';
#endif
	argv[argc] = NULL;

	ShowWindow(hMain, SW_HIDE);

	time(&start);
	SetTimer(hMain, GAMEWND_TIMER, 1000/*1s*/, NULL);
	exit_code = main_(argc, (char **)argv);
	time(&end);
	/*This is to make sure this timer is killed, if the Game Window was not found
	Should not happen, but you never know... */
	KillTimer(hMain,GAMEWND_TIMER);
	elapsedtime = end - start;
	if (exit_code == 0)
	{
		// Check the exitcode before incrementing Playtime
		IncrementPlayTime(nGameIndex, elapsedtime);
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
	}

	if (GetHideMouseOnStartup())
	{
		ShowCursor(FALSE);
	}
	else
	{
		// recover windows cursor and our main window
		while (1)
		{
			if (ShowCursor(TRUE) >= 0)
				break;
		}
	}
	ShowWindow(hMain, SW_SHOW);

	// Kludge: Reset the font
	if (hFont != NULL)
		SetAllWindowsFont(hMain, &main_resize, hFont, FALSE);

	if (pCmdLine != NULL)
		free(pCmdLine);

	return exit_code;

#else
	DWORD               dwExitCode = 0;
	STARTUPINFOA        si;
	PROCESS_INFORMATION pi;
	char *pCmdLine;
	HWND hGameWnd = NULL;
	long lGameWndStyle = 0;

	pCmdLine = malloc(GetTotalPathLen() + 2500);
	if (!pCmdLine)
	{
		OutputDebugStringA("Out of memory.");
		return 1;
	}

	CreateCommandLine(nGameIndex, pCmdLine);

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	if (!CreateProcessA(NULL,
	                    pCmdLine,
	                    NULL,		  /* Process handle not inheritable. */
	                    NULL,		  /* Thread handle not inheritable. */
	                    TRUE,		  /* Handle inheritance.  */
	                    0,			  /* Creation flags. */
	                    NULL,		  /* Use parent's environment block.  */
	                    NULL,		  /* Use parent's starting directory.  */
	                    &si,		  /* STARTUPINFO */
	                    &pi))		  /* PROCESS_INFORMATION */
	{
		dprintf("CreateProcess failed.");
		dwExitCode = GetLastError();
	}
	else
	{

		ShowWindow(hMain, SW_HIDE);
		SendIconToProcess(&pi, nGameIndex);
		if( ! GetGameCaption() )
		{
			hGameWnd = GetGameWindow(&pi);
			if( hGameWnd )
			{
				lGameWndStyle = GetWindowLong(hGameWnd, GWL_STYLE);
				lGameWndStyle = lGameWndStyle & (WS_BORDER ^ 0xffffffff);
				SetWindowLong(hGameWnd, GWL_STYLE, lGameWndStyle);
				SetWindowPos(hGameWnd,0,0,0,0,0,SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
			}
		}
		time(&start);

		// Wait until child process exits.
		WaitWithMessageLoop(pi.hProcess);

		GetExitCodeProcess(pi.hProcess, &dwExitCode);

		time(&end);
		elapsedtime = end - start;
		if( dwExitCode == 0 )
		{
			// Check the exitcode before incrementing Playtime
			IncrementPlayTime(nGameIndex, elapsedtime);
			ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		}

		ShowWindow(hMain, SW_SHOW);
		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	free(pCmdLine);

	return dwExitCode;
#endif
}

#undef WinMainInDLL
#ifndef DONT_USE_DLL
#ifdef _MSC_VER
#define WinMainInDLL
#endif /* _MSC_VER */
#endif /* !DONT_USE_DLL */

#ifdef WinMainInDLL
SHAREDOBJ_FUNC(int) WinMain_(HINSTANCE    hInstance,
#else
int WinMain_(HINSTANCE    hInstance,
#endif
                   HINSTANCE    hPrevInstance,
                   LPSTR        lpCmdLine,
                   int          nCmdShow)
{
	dprintf("MAME32 starting");

	options.gui_host = 1;
	use_gui_romloading = TRUE;

	/* set up language for windows */
	assign_msg_catategory(UI_MSG_OSD0, "windows");
	assign_msg_catategory(UI_MSG_OSD1, "ui");

	osd_display_loading_rom_message_ = osd_display_loading_rom_message;

	if (__argc != 1)
	{
		/* Rename main because gcc will use it instead of WinMain even with -mwindows */
//		extern int DECL_SPEC main_(int, char**);
		exit(main_(__argc, __argv));
	}
	if (!Win32UI_init(hInstance, lpCmdLine, nCmdShow))
		return 1;

	// pump message, but quit on WM_QUIT
	while(PumpMessage())
		;

	Win32UI_exit();
	return 0;
}


HWND GetMainWindow(void)
{
	return hMain;
}

HWND GetTreeView(void)
{
	return hTreeView;
}

void GetRealColumnOrder(int order[])
{
	int tmpOrder[COLUMN_MAX];
	int nColumnMax;
	int i;

	nColumnMax = Picker_GetNumColumns(hwndList);

	/* Get the Column Order and save it */
	if (!oldControl)
	{
		ListView_GetColumnOrderArray(hwndList, nColumnMax, tmpOrder);

		for (i = 0; i < nColumnMax; i++)
		{
			order[i] = Picker_GetRealColumnFromViewColumn(hwndList, tmpOrder[i]);
		}
	}
}

/*
 * PURPOSE: Format raw data read from an ICO file to an HICON
 * PARAMS:  PBYTE ptrBuffer  - Raw data from an ICO file
 *          UINT nBufferSize - Size of buffer ptrBuffer
 * RETURNS: HICON - handle to the icon, NULL for failure
 * History: July '95 - Created
 *          March '00- Seriously butchered from MSDN for mine own
 *          purposes, sayeth H0ek.
 */
HICON FormatICOInMemoryToHICON(PBYTE ptrBuffer, UINT nBufferSize)
{
	ICONIMAGE           IconImage;
	LPICONDIRENTRY      lpIDE = NULL;
	UINT                nNumImages;
	UINT                nBufferIndex = 0;
	HICON               hIcon = NULL;
	int                 i;

	/* Is there a WORD? */
	if (nBufferSize < sizeof(WORD))
	{
		return NULL;
	}

	/* Was it 'reserved' ?	 (ie 0) */
	if ((WORD)(ptrBuffer[nBufferIndex]) != 0)
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (nBufferSize - nBufferIndex < sizeof(WORD))
	{
		return NULL;
	}

	/* Was it type 1? */
	if ((WORD)(ptrBuffer[nBufferIndex]) != 1)
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (nBufferSize - nBufferIndex < sizeof(WORD))
	{
		return NULL;
	}

	/* Then that's the number of images in the ICO file */
	nNumImages = (WORD)(ptrBuffer[nBufferIndex]);

	/* Is there at least one icon in the file? */
	if ( nNumImages < 1 )
	{
		return NULL;
	}

	nBufferIndex += sizeof(WORD);

	/* Is there enough space for the icon directory entries? */
	if ((nBufferIndex + nNumImages * sizeof(ICONDIRENTRY)) > nBufferSize)
	{
		return NULL;
	}

	/* Assign icon directory entries from buffer */
	lpIDE = (LPICONDIRENTRY)(&ptrBuffer[nBufferIndex]);
	nBufferIndex += nNumImages * sizeof (ICONDIRENTRY);

	/* Search large icon index to load */
	for (i = 0; i < nNumImages; i++)
	{
		int width;
		int height;

		IconImage.dwNumBytes = lpIDE[i].dwBytesInRes;

		/* Seek to beginning of this image */
		if ( lpIDE[i].dwImageOffset > nBufferSize )
		{
			return NULL;
		}

		nBufferIndex = lpIDE[i].dwImageOffset;

		/* Read it in */
		if ((nBufferIndex + lpIDE[i].dwBytesInRes) > nBufferSize)
		{
			return NULL;
		}

		IconImage.lpBits = &ptrBuffer[nBufferIndex];
		nBufferIndex += lpIDE[i].dwBytesInRes;

		width = (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biWidth;
		height = (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biHeight / 2;

		if (width == GetSystemMetrics(SM_CXCURSOR) && height == GetSystemMetrics(SM_CYCURSOR))
			break;
	}

	/* If not found, use first one */
	if (i == nNumImages)
		i = 0;

	IconImage.dwNumBytes = lpIDE[i].dwBytesInRes;
	nBufferIndex = lpIDE[i].dwImageOffset;

	IconImage.lpBits = &ptrBuffer[nBufferIndex];
	nBufferIndex += lpIDE[i].dwBytesInRes;

	hIcon = CreateIconFromResourceEx(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000,
			(*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biWidth, (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biHeight/2, 0 );

	/* It failed, odds are good we're on NT so try the non-Ex way */
	if (hIcon == NULL)
	{
		/* We would break on NT if we try with a 16bpp image */
		if (((LPBITMAPINFO)IconImage.lpBits)->bmiHeader.biBitCount != 16)
		{
			hIcon = CreateIconFromResource(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000);
		}
	}
	return hIcon;
}

HICON LoadIconFromFile(const char *iconname)
{
	HICON       hIcon = 0;
	struct stat file_stat;
	char        tmpStr[MAX_PATH];
	char        tmpIcoName[MAX_PATH];
	const char* sDirName = GetImgDir();
	PBYTE       bufferPtr;
	UINT        bufferLen;

	sprintf(tmpStr, "%s/%s.ico", GetIconsDir(), iconname);
	if (stat(tmpStr, &file_stat) != 0
	|| (hIcon = ExtractIconA(hInst, tmpStr, 0)) == 0)
	{
		sprintf(tmpStr, "%s/%s.ico", sDirName, iconname);
		if (stat(tmpStr, &file_stat) != 0
		|| (hIcon = ExtractIconA(hInst, tmpStr, 0)) == 0)
		{
			sprintf(tmpStr, "%s/icons.zip", GetIconsDir());
			sprintf(tmpIcoName, "%s.ico", iconname);
			if (stat(tmpStr, &file_stat) == 0)
			{
				SetCorePathList(FILETYPE_SCREENSHOT, GetIconsDir());
				if (load_zipped_file(FILETYPE_SCREENSHOT, 0, "icons.zip", tmpIcoName, &bufferPtr, &bufferLen) == 0)
				{
					hIcon = FormatICOInMemoryToHICON(bufferPtr, bufferLen);
					free(bufferPtr);
				}
			}
		}
	}

	SetCorePathList(FILETYPE_SCREENSHOT, GetImgDir());

	return hIcon;
}

/* Return the number of games currently displayed */
int GetNumGames()
{
	return game_count;
}

LPWSTR GetSearchText(void)
{
	return g_SearchText;
}

/* Sets the treeview and listviews sizes in accordance with their visibility and the splitters */
static void ResizeTreeAndListViews(BOOL bResizeHidden)
{
	int i;
	int nLastWidth = 0;
	int nLastWidth2 = 0;
	int nLeftWindowWidth = 0;
	RECT rect;
	BOOL bVisible;

	/* Size the List Control in the Picker */
	GetClientRect(hMain, &rect);

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;
	if (bShowToolBar)
		rect.top += topMargin;

	/* Tree control */
	ShowWindow(GetDlgItem(hMain, IDC_TREE), bShowTree ? SW_SHOW : SW_HIDE);

	for (i = 0; g_splitterInfo[i].nSplitterWindow; i++)
	{
		bVisible = GetWindowLong(GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow), GWL_STYLE) & WS_VISIBLE ? TRUE : FALSE;
		if (bResizeHidden || bVisible)
		{
			nLeftWindowWidth = nSplitterOffset[i] - SPLITTER_WIDTH/2 - nLastWidth;

			/* special case for the rightmost pane when the screenshot is gone */
			if (!GetShowScreenShot() && !g_splitterInfo[i+1].nSplitterWindow)
				nLeftWindowWidth = rect.right - nLastWidth;

			/* woah?  are we overlapping ourselves? */
			if (nLeftWindowWidth < MIN_VIEW_WIDTH)
			{
				nLastWidth = nLastWidth2;
				nLeftWindowWidth = nSplitterOffset[i] - MIN_VIEW_WIDTH - (SPLITTER_WIDTH*3/2) - nLastWidth;
				i--;
			}

			MoveWindow(GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow), nLastWidth, rect.top + 2,
				nLeftWindowWidth, (rect.bottom - rect.top) - 4 , TRUE);

			MoveWindow(GetDlgItem(hMain, g_splitterInfo[i].nSplitterWindow), nSplitterOffset[i], rect.top + 2,
				SPLITTER_WIDTH, (rect.bottom - rect.top) - 4, TRUE);
		}

		if (bVisible)
		{
			nLastWidth2 = nLastWidth;
			nLastWidth += nLeftWindowWidth + SPLITTER_WIDTH; 
		}
	}
}

/* Adjust the list view and screenshot button based on GetShowScreenShot() */
void UpdateScreenShot(void)
{
	RECT rect;
	int  nWidth;
	RECT fRect;
	POINT p = {0, 0};

	/* first time through can't do this stuff */
	if (hwndList == NULL)
		return;

	/* Size the List Control in the Picker */
	GetClientRect(hMain, &rect);

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;
	if (bShowToolBar)
		rect.top += topMargin;

	if (GetShowScreenShot())
	{
		nWidth = nSplitterOffset[GetSplitterCount() - 1];
		CheckMenuItem(GetMenu(hMain),ID_VIEW_PICTURE_AREA, MF_CHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, MF_CHECKED);
	}
	else
	{
		nWidth = rect.right;
		CheckMenuItem(GetMenu(hMain),ID_VIEW_PICTURE_AREA, MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, MF_UNCHECKED);
	}

	ResizeTreeAndListViews(FALSE);

	FreeScreenShot();

	if (have_selection)
	{
#ifdef USE_IPS
		// load and set image, or empty it if we don't have one
		if (g_IPSMenuSelectName)
			LoadScreenShot(Picker_GetSelectedItem(hwndList), g_IPSMenuSelectName, TAB_IPS);
		else
#endif /* USE_IPS */
			LoadScreenShot(Picker_GetSelectedItem(hwndList), NULL, TabView_GetCurrentTab(hTabCtrl));
	}

	// figure out if we have a history or not, to place our other windows properly
	UpdateHistory();

	// setup the picture area

	if (GetShowScreenShot())
	{
		DWORD dwStyle;
		DWORD dwStyleEx;
		BOOL showing_history;

		ClientToScreen(hMain, &p);
		GetWindowRect(GetDlgItem(hMain, IDC_SSFRAME), &fRect);
		OffsetRect(&fRect, -p.x, -p.y);

		// show history on this tab IFF
		// - we have history for the game
		// - we're on the first tab
		// - we DON'T have a separate history tab
		showing_history = (have_history && NeedHistoryText());
		CalculateBestScreenShotRect(GetDlgItem(hMain, IDC_SSFRAME), &rect,showing_history);

		dwStyle   = GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_STYLE);
		dwStyleEx = GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_EXSTYLE);

		AdjustWindowRectEx(&rect, dwStyle, FALSE, dwStyleEx);
		MoveWindow(GetDlgItem(hMain, IDC_SSPICTURE),
		           fRect.left  + rect.left,
		           fRect.top   + rect.top,
		           rect.right  - rect.left,
		           rect.bottom - rect.top,
		           TRUE);

		ShowWindow(GetDlgItem(hMain,IDC_SSPICTURE), NeedScreenShotImage() ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSFRAME),SW_SHOW);
		ShowWindow(GetDlgItem(hMain,IDC_SSTAB),bShowTabCtrl ? SW_SHOW : SW_HIDE);

		InvalidateRect(GetDlgItem(hMain,IDC_SSPICTURE),NULL,FALSE);
	}
	else
	{
		ShowWindow(GetDlgItem(hMain,IDC_SSPICTURE),SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSFRAME),SW_HIDE);
		ShowWindow(GetDlgItem(hMain,IDC_SSTAB),SW_HIDE);
	}

}

void ResizePickerControls(HWND hWnd)
{
	RECT frameRect;
	RECT rect;
	int  nListWidth, nScreenShotWidth;
	static BOOL firstTime = TRUE;
	int  doSSControls = TRUE;
	int i, nSplitterCount;

	nSplitterCount = GetSplitterCount();

	/* Size the List Control in the Picker */
	GetClientRect(hWnd, &rect);

	/* Calc the display sizes based on g_splitterInfo */
	if (firstTime)
	{
		RECT rWindow;

		for (i = 0; i < nSplitterCount; i++)
			nSplitterOffset[i] = rect.right * g_splitterInfo[i].dPosition;

		GetWindowRect(hStatusBar, &rWindow);
		bottomMargin = rWindow.bottom - rWindow.top;
		GetWindowRect(hToolBar, &rWindow);
		topMargin = rWindow.bottom - rWindow.top;
		/*buttonMargin = (sRect.bottom + 4); */

		firstTime = FALSE;
	}
	else
	{
		doSSControls = GetShowScreenShot();
	}

	if (bShowStatusBar)
		rect.bottom -= bottomMargin;

	if (bShowToolBar)
		rect.top += topMargin;

	MoveWindow(GetDlgItem(hWnd, IDC_DIVIDER), rect.left, rect.top - 4, rect.right, 2, TRUE);

	ResizeTreeAndListViews(TRUE);

	nListWidth = nSplitterOffset[nSplitterCount-1];
	nScreenShotWidth = (rect.right - nListWidth) - 4;

	/* Screen shot Page tab control */
	if (bShowTabCtrl)
	{
		MoveWindow(GetDlgItem(hWnd, IDC_SSTAB), nListWidth + 4, rect.top + 2,
			nScreenShotWidth - 2, rect.top + 20, doSSControls);
		rect.top += 20;
	}

	/* resize the Screen shot frame */
	MoveWindow(GetDlgItem(hWnd, IDC_SSFRAME), nListWidth + 4, rect.top + 2,
		nScreenShotWidth - 2, (rect.bottom - rect.top) - 4, doSSControls);

	/* The screen shot controls */
	GetClientRect(GetDlgItem(hWnd, IDC_SSFRAME), &frameRect);

	/* Text control - game history */
	history_rect.left = nListWidth + 14;
	history_rect.right = nScreenShotWidth - 22;

	history_rect.top = rect.top;
	history_rect.bottom = rect.bottom;

	/* the other screen shot controls will be properly placed in UpdateScreenshot() */
}

static int modify_separator_len(const char *str)
{
	int n;

	switch (*str)
	{
	case ' ':
		if (!strncmp(str, " - ", 3))
			return 3;

		if ((n = modify_separator_len(str + 1)) != 0)
			return n + 1;

		break;

	case ':':
	case '/':
	case ',':
	case '(':
	case ')':
	case '!':
		for (n = 1; str[n]; n++)
			if (str[n] != ' ')
				break;
		return modify_separator_len(str + n) + n;
	}

	return 0;
}

char *ModifyThe(const char *str)
{
	static int  bufno = 0;
	static char buffer[4][255];
	int modified = 0;
	char *ret = (char *)str;
	char *t;

	t = buffer[bufno];

	while (*str)
	{
		char *p = t;

		while (!modify_separator_len(str))
		{
			if ((*p++ = *str++) == '\0')
				break;
		}

		*p = '\0';

		if (strncmp(t, "The ", 4) == 0)
		{
			char temp[255];

			strcpy(temp, t + 4);
			strcat(temp, ", The");

			strcpy(t, temp);
			p++;
			modified = 1;
		}

		t = p + modify_separator_len(str);
		while (p < t)
			*p++ = *str++;
		*p = '\0';
	}

	if (modified)
	{
		ret = buffer[bufno];
		bufno = (bufno + 1) % 4;
	}

	return ret;
}

HBITMAP GetBackgroundBitmap(void)
{
	return hBackground;
}

HPALETTE GetBackgroundPalette(void)
{
	return hPALbg;
}

MYBITMAPINFO * GetBackgroundInfo(void)
{
	return &bmDesc;
}

BOOL GetUseOldControl(void)
{
	return oldControl;
}

BOOL GetUseXPControl(void)
{
	return xpControl;
}

int GetMinimumScreenShotWindowWidth(void)
{
	BITMAP bmp;
	GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);

	return bmp.bmWidth + 6; // 6 is for a little breathing room
}


int GetDriverIndex(const game_driver *driver)
{
	return GetGameNameIndex(driver->name);
}

int GetGameNameIndex(const char *name)
{
	driver_data_type *driver_index_info;
	driver_data_type key;
	key.name = name;

	// uses our sorted array of driver names to get the index in log time
	driver_index_info = bsearch(&key,sorted_drivers,game_count,sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

}

int GetIndexFromSortedIndex(int sorted_index)
{
	return sorted_drivers[sorted_index].index;
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static char **game_readings;

static void build_game_readings(void)
{
	int i;

	if (game_readings)
		free (game_readings);

	game_readings = malloc (sizeof (char *) * game_count);
	if (!game_readings)
		return;

	for (i = 0; i < game_count; i++)
	{
		char *l,*r;
		l = _LST(drivers[i]->description);
		r = _READINGS(drivers[i]->description);

		game_readings[i] = (drivers[i]->description == r) ? l : r;
	}
}

static void ChangeLanguage(int id)
{
	int nGame = Picker_GetSelectedItem(hwndList);
	int i;

	if (id)
		SetLangcode(id - ID_LANGUAGE_ENGLISH_US);

	for (i = 0; i < UI_LANG_MAX; i++)
	{
		UINT cp = ui_lang_info[i].codepage;

		CheckMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, i == GetLangcode() ? MF_CHECKED : MF_UNCHECKED);
		if (OnNT())
			EnableMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, IsValidCodePage(cp) ? MF_ENABLED : MF_GRAYED);
		else
			EnableMenuItem(GetMenu(hMain), i + ID_LANGUAGE_ENGLISH_US, (i == UI_LANG_EN_US || cp == GetACP()) ? MF_ENABLED : MF_GRAYED);
	}

	if (id)
	{
		LOGFONTA logfont;

		if (hFont != NULL)
			DeleteObject(hFont);

		GetTranslatedFont(&logfont);

		SetListFont(&logfont);

		hFont = TranslateCreateFont(&logfont);
	}

	build_game_readings();

	if (id && hToolBar != NULL)
	{
		DestroyWindow(hToolBar);
		hToolBar = InitToolbar(hMain);
		main_resize_items[0].u.hwnd = hToolBar;
		ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_PICTURE_AREA, GetShowScreenShot() ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_LARGE_ICON + Picker_GetViewID(hwndList), MF_CHECKED);
		ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
	}

	TranslateDialog(hMain, 0, TRUE);
	TranslateMenu(GetMenu(hMain), 0);

	DrawMenuBar(hMain);

	TranslateTreeFolders(hTreeView);

	/* Reset the font */
	if (hFont != NULL)
		SetAllWindowsFont(hMain, &main_resize, hFont, FALSE);

	TabView_Reset(hTabCtrl);
	TabView_UpdateSelection(hTabCtrl);
	UpdateHistory();

	ResetColumnDisplay(FALSE);

	Picker_SetSelectedItem(hwndList, nGame);
}

// used for our sorted array of game names
int CLIB_DECL DriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((driver_data_type *)arg1)->name, ((driver_data_type *)arg2)->name );
}

static void ResetBackground(const char *szFile)
{
	char szDestFile[MAX_PATH];

	/* The MAME core load the .png file first, so we only need replace this file */
	sprintf(szDestFile, "%s\\bkground.png", GetBgDir());
	SetFileAttributesA(szDestFile, FILE_ATTRIBUTE_NORMAL);
	CopyFileA(szFile, szDestFile, FALSE);
}

static void RandomSelectBackground(void)
{
	struct _finddata_t c_file;
	long hFile;
	char szFile[MAX_PATH];
	int count=0;
	const char *szDir=GetBgDir();
	char *buf=malloc(_MAX_FNAME * MAX_BGFILES);

	if (buf == NULL)
		return;

	sprintf(szFile, "%s\\*.bmp", szDir);
	hFile = _findfirst(szFile, &c_file);
	if (hFile != -1L)
	{
		int Done = 0;
		while (!Done && count < MAX_BGFILES)
		{
			memcpy(buf + count * _MAX_FNAME, c_file.name, _MAX_FNAME);
			count++;
			Done = _findnext(hFile, &c_file);
		}
		_findclose(hFile);
	}
	sprintf(szFile, "%s\\*.png", szDir);
	hFile = _findfirst(szFile, &c_file);
	if (hFile != -1L)
	{
		int Done = 0;
		while (!Done && count < MAX_BGFILES)
		{
			memcpy(buf + count * _MAX_FNAME, c_file.name, _MAX_FNAME);
			count++;
			Done = _findnext(hFile, &c_file);
		}
		_findclose(hFile);
	}

	if (count)
	{
		srand( (unsigned)time( NULL ) );
		sprintf(szFile, "%s\\%s", szDir, buf + (rand() % count) * _MAX_FNAME);
		ResetBackground(szFile);
	}

	free(buf);
}

void SetMainTitle(void)
{
	char version[50];
	char buffer[100];

	sscanf(build_version,"%s",version);
	sprintf(buffer,"%s Plus! %s",MAME32NAME,version);
	SetWindowText(hMain,_Unicode(buffer));
}

static void TabSelectionChanged(void)
{
#ifdef USE_IPS
	FreeIfAllocated(&g_IPSMenuSelectName);
#endif /* USE_IPS */

	UpdateScreenShot();
}

static BOOL Win32UI_init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	extern int mame_validitychecks(int);
	WNDCLASS wndclass;
	RECT     rect;
	int      i;
	int      nSplitterCount;
	LONG     common_control_version = GetCommonControlVersion();

	srand((unsigned)time(NULL));

	init_resource_tracking();
	begin_resource_tracking();

	// Count the number of games
	game_count = 0;
	while (drivers[game_count] != 0)
		game_count++;

	/* custom per-game icons */
	icon_index = malloc(sizeof(int) * game_count);
	if (!icon_index)
		return FALSE;
	ZeroMemory(icon_index,sizeof(int) * game_count);

	/* sorted list of drivers by name */
	sorted_drivers = (driver_data_type *) malloc(sizeof(driver_data_type) * game_count);
	if (!sorted_drivers)
		return FALSE;
	for (i=0;i<game_count;i++)
	{
		sorted_drivers[i].name = drivers[i]->name;
		sorted_drivers[i].index = i;
	}
	qsort(sorted_drivers,game_count,sizeof(driver_data_type),DriverDataCompareFunc );

	/* initialize cpu information */
	cpuintrf_init();

	/* initialize sound information */
	sndintrf_init();

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = MameWindowProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = DLGWINDOWEXTRA;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAME32_ICON));
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszMenuName  = MAKEINTRESOURCE(IDR_UI_MENU);
	wndclass.lpszClassName = TEXT("MainClass");

	RegisterClass(&wndclass);

	InitCommonControls();

	// Are we using an Old comctl32.dll?
	dprintf("common control version %i.%i",common_control_version >> 16,
	        common_control_version & 0xffff);
			 
	oldControl = (common_control_version < PACKVERSION(4,71));
	xpControl = (common_control_version >= PACKVERSION(6,0));
	if (oldControl)
	{
		char buf[] = MAME32NAME " has detected an old version of comctl32.dll\n\n"
					 "Game Properties, many configuration options and\n"
					 "features are not available without an updated DLL\n\n"
					 "Please install the common control update found at:\n\n"
					 "http://www.microsoft.com/msdownload/ieplatform/ie/comctrlx86.asp\n\n"
					 "Would you like to continue without using the new features?\n";

		if (IDNO == MessageBoxA(0, buf, MAME32NAME " Outdated comctl32.dll Warning", MB_YESNO | MB_ICONWARNING))
			return FALSE;
	}

	dprintf("about to init options");
	OptionsInit();
	dprintf("options loaded");

#ifdef USE_SHOW_SPLASH_SCREEN
	// Display splash screen window
	if (GetDisplaySplashScreen() != FALSE)
		CreateBackgroundMain(hInstance);
#endif /* USE_SHOW_SPLASH_SCREEN */

	/* USE LANGUAGE LIST */
	build_game_readings();

	g_mame32_message = RegisterWindowMessage(TEXT_MAME32NAME);
	g_bDoBroadcast = GetBroadcast();

	HelpInit();

	strcpy(last_directory,GetInpDir());
	hMain = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, NULL);
	if (hMain == NULL)
	{
		dprintf("error creating main dialog, aborting");
		return FALSE;
	}

	SetMainTitle();
	hTabCtrl = GetDlgItem(hMain, IDC_SSTAB);

	{
		struct TabViewOptions opts;

		static struct TabViewCallbacks s_tabviewCallbacks =
		{
			GetShowTabCtrl,			// pfnGetShowTabCtrl
			SetCurrentTab,			// pfnSetCurrentTab
			GetCurrentTab,			// pfnGetCurrentTab
			SetShowTab,				// pfnSetShowTab
			GetShowTab,				// pfnGetShowTab

			GetImageTabShortName,	// pfnGetTabShortName
			GetImageTabLongName,	// pfnGetTabLongName
			TabSelectionChanged		// pfnOnSelectionChanged
		};

		memset(&opts, 0, sizeof(opts));
		opts.pCallbacks = &s_tabviewCallbacks;
		opts.nTabCount = MAX_TAB_TYPES;

		if (!SetupTabView(hTabCtrl, &opts))
			return FALSE;
	}

	/* subclass history window */
	g_lpHistoryWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_WNDPROC, (LONG)HistoryWndProc);

	/* subclass picture frame area */
	g_lpPictureFrameWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_SSFRAME), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_SSFRAME), GWL_WNDPROC, (LONG)PictureFrameWndProc);

	/* subclass picture area */
	g_lpPictureWndProc = (WNDPROC)(LONG)(int)GetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_WNDPROC);
	SetWindowLong(GetDlgItem(hMain, IDC_SSPICTURE), GWL_WNDPROC, (LONG)PictureWndProc);

	/* Load the pic for the default screenshot. */
	hMissing_bitmap = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_ABOUT));

	/* Stash hInstance for later use */
	hInst = hInstance;

	hToolBar   = InitToolbar(hMain);
	hStatusBar = InitStatusBar(hMain);
	hProgWnd   = InitProgressBar(hStatusBar);

	main_resize_items[0].u.hwnd = hToolBar;
	main_resize_items[1].u.hwnd = hStatusBar;

	/* In order to handle 'Large Fonts' as the Windows
	 * default setting, we need to make the dialogs small
	 * enough to fit in our smallest window size with
	 * large fonts, then resize the picker, tab and button
	 * controls to fill the window, no matter which font
	 * is currently set.  This will still look like bad
	 * if the user uses a bigger default font than 125%
	 * (Large Fonts) on the Windows display setting tab.
	 *
	 * NOTE: This has to do with Windows default font size
	 * settings, NOT our picker font size.
	 */

	GetClientRect(hMain, &rect);

	hTreeView = GetDlgItem(hMain, IDC_TREE);
	hwndList  = GetDlgItem(hMain, IDC_LIST);

	//history_filename = mame_strdup(GetHistoryFile());
#ifdef STORY_DATAFILE
	//story_filename = mame_strdup(GetStoryFile());
#endif /* STORY_DATAFILE */
	//mameinfo_filename = mame_strdup(GetMAMEInfoFile());

	if (!InitSplitters())
		return FALSE;

	nSplitterCount = GetSplitterCount();
	for (i = 0; i < nSplitterCount; i++)
	{
		HWND hWnd;
		HWND hWndLeft;
		HWND hWndRight;

		hWnd = GetDlgItem(hMain, g_splitterInfo[i].nSplitterWindow);
		hWndLeft = GetDlgItem(hMain, g_splitterInfo[i].nLeftWindow);
		hWndRight = GetDlgItem(hMain, g_splitterInfo[i].nRightWindow);

		AddSplitter(hWnd, hWndLeft, hWndRight, g_splitterInfo[i].pfnAdjust);
	}

	/* Initial adjustment of controls on the Picker window */
	ResizePickerControls(hMain);

	TabView_UpdateSelection(hTabCtrl);

	bDoGameCheck = GetGameCheck();
	idle_work    = TRUE;
	game_index   = 0;

	bShowTree      = GetShowFolderList();
	bShowToolBar   = GetShowToolBar();
	bShowStatusBar = GetShowStatusBar();
	bShowTabCtrl   = GetShowTabCtrl();

	CheckMenuRadioItem(GetMenu(hMain), ID_VIEW_BYGAME, ID_VIEW_BYPLAYTIME, GetSortColumn(), MF_CHECKED);

	CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
	ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
	ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
	ShowWindow(hStatusBar, (bShowStatusBar) ? SW_SHOW : SW_HIDE);
	CheckMenuItem(GetMenu(hMain), ID_VIEW_PAGETAB, (bShowTabCtrl) ? MF_CHECKED : MF_UNCHECKED);
	ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);
	DragAcceptFiles(hMain, TRUE);

	if (oldControl)
	{
		EnableMenuItem(GetMenu(hMain), ID_CUSTOMIZE_FIELDS,  MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_GAME_PROPERTIES,   MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_FOLDER_SOURCEPROPERTIES, MF_GRAYED);
		EnableMenuItem(GetMenu(hMain), ID_OPTIONS_DEFAULTS,  MF_GRAYED);
	}

	/* Init DirectDraw */
	if (!DirectDraw_Initialize())
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIRECTX), NULL, DirectXDialogProc);
		return FALSE;
	}

	if (GetRandomBackground())
		RandomSelectBackground();

	LoadBackgroundBitmap();

	dprintf("about to init tree");
	InitTree(g_folderData, g_filterList);
	dprintf("did init tree");

	PropertiesInit();

	/* Initialize listview columns */
	InitListView();
	SetFocus(hwndList);

	/* Reset the font */
	{
		LOGFONTA logfont;

		GetListFont(&logfont);
		hFont = TranslateCreateFont(&logfont);
		if (hFont != NULL)
			SetAllWindowsFont(hMain, &main_resize, hFont, FALSE);
	}

	/* Init DirectInput */
	if (!DirectInputInitialize())
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIRECTX), NULL, DirectXDialogProc);
		return FALSE;
	}

	AdjustMetrics();
	UpdateScreenShot();

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_TAB_KEYS));

	/* clear keyboard state */
	KeyboardStateClear();

	for (i = 0; i < NUM_GUI_SEQUENCES; i++)
	{
		input_seq *is1;
		input_seq *is2;
		is1 = &(GUISequenceControl[i].is);
		is2 = GUISequenceControl[i].getiniptr();
		seq_copy(is1, is2);
		//dprintf("seq =%s is: %4i %4i %4i %4i\n",GUISequenceControl[i].name, (*is1)[0], (*is1)[1], (*is1)[2], (*is1)[3]);
	}

	if (GetJoyGUI() == TRUE)
	{
		g_pJoyGUI = &DIJoystick;
		if (g_pJoyGUI->init() != 0)
			g_pJoyGUI = NULL;
		else
			SetTimer(hMain, JOYGUI_TIMER, JOYGUI_MS, NULL);
	}
	else
		g_pJoyGUI = NULL;

	ChangeLanguage(0);

	if (GetHideMouseOnStartup())
	{    
		/*  For some reason the mouse is centered when a game is exited, which of
			course causes a WM_MOUSEMOVE event that shows the mouse. So we center
			it now, before the startup coords are initilized, and that way the mouse
			will still be hidden when exiting from a game (i hope) :)
		*/      
		SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2);
		
		// Then hide it
		ShowCursor(FALSE);
	}

	dprintf("about to show window");

	nCmdShow = GetWindowState();
	if (nCmdShow == SW_HIDE || nCmdShow == SW_MINIMIZE || nCmdShow == SW_SHOWMINIMIZED)
	{
		nCmdShow = SW_RESTORE;
	}

	if (GetRunFullScreen())
	{ 
		LONG lMainStyle;

		// Remove menu
		SetMenu(hMain,NULL); 

		// Frameless dialog (fake fullscreen)
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle & (WS_BORDER ^ 0xffffffff);
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);

		nCmdShow = SW_MAXIMIZE;
	}

#ifdef USE_SHOW_SPLASH_SCREEN
	// Destroy splash screen window
	if (GetDisplaySplashScreen() != FALSE)
		DestroyBackgroundMain();
#endif /* USE_SHOW_SPLASH_SCREEN */

	ShowWindow(hMain, nCmdShow);


	switch (GetViewMode())
	{
	case VIEW_LARGE_ICONS :
		SetView(ID_VIEW_LARGE_ICON);
		break;
	case VIEW_SMALL_ICONS :
		SetView(ID_VIEW_SMALL_ICON);
		break;
	case VIEW_INLIST :
		SetView(ID_VIEW_LIST_MENU);
		break;
	case VIEW_REPORT :
		SetView(ID_VIEW_DETAIL);
		break;
	case VIEW_GROUPED :
	default :
		SetView(ID_VIEW_GROUPED);
		break;
	}

	if (GetCycleScreenshot() > 0)
	{
		SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL); //scale to Seconds
	}

#ifdef MAME_DEBUG
	if (mame_validitychecks(-1))
	{
		MessageBoxA(hMain, MAMENAME " has failed its validity checks.  The GUI will "
			"still work, but emulations will fail to execute", MAMENAME, MB_OK);
	}
#endif // MAME_DEBUG

	return TRUE;
}

static void Win32UI_exit()
{
	DragAcceptFiles(hMain, FALSE);

	if (g_bDoBroadcast == TRUE)
	{
        ATOM a = GlobalAddAtomA("");
		SendMessage(HWND_BROADCAST, g_mame32_message, a, a);
		GlobalDeleteAtom(a);
	}

	if (g_pJoyGUI != NULL)
		g_pJoyGUI->exit();

	/* Free GDI resources */

	if (hMissing_bitmap)
	{
		DeleteObject(hMissing_bitmap);
		hMissing_bitmap = NULL;
	}

	if (hBackground)
	{
		DeleteObject(hBackground);
		hBackground = NULL;
	}
	
	if (hPALbg)
	{
		DeleteObject(hPALbg);
		hPALbg = NULL;
	}
	
	if (hFont)
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
	
	DestroyIcons();

	DestroyAcceleratorTable(hAccel);

	if (icon_index != NULL)
	{
		free(icon_index);
		icon_index = NULL;
	}
	if (sorted_drivers != NULL)
	{
		free(sorted_drivers);
		sorted_drivers = NULL;
	}

	DirectInputClose();
	DirectDraw_Close();

	SetSavedFolderID(GetCurrentFolderID());

	SaveOptions();

	FreeFolders();

	/* DestroyTree(hTreeView); */

	FreeScreenShot();

	OptionsExit();

	HelpExit();

	end_resource_tracking();
	exit_resource_tracking();
}

static long WINAPI MameWindowProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	MINMAXINFO	*mminfo;
#if MULTISESSION
	int nGame;
	HWND hGameWnd;
	long lGameWndStyle;
#endif // MULTISESSION


	int 		i;
	char		szClass[128];
	
#ifdef USE_IPS
	static char patch_name[MAX_PATCHNAME];
#endif /* USE_IPS */

	switch (message)
	{
	case WM_CTLCOLORSTATIC:
		if (hBackground && (HWND)lParam == GetDlgItem(hMain, IDC_HISTORY))
		{
			static HBRUSH hBrush=0;
			HDC hDC=(HDC)wParam;
			LOGBRUSH lb;

			if (hBrush)
				DeleteObject(hBrush);

			if (hBackground)	// Always true?
			{
				lb.lbStyle  = BS_HOLLOW;
				hBrush = CreateBrushIndirect(&lb);
				SetBkMode(hDC, TRANSPARENT);
			}
			else
			{
				hBrush = GetSysColorBrush(COLOR_BTNFACE);
				SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
				SetBkMode(hDC, OPAQUE);
			}
			SetTextColor(hDC, GetListFontColor());
			return (LRESULT) hBrush;
		}
		break;

	case WM_INITDIALOG:
		TranslateDialog(hWnd, lParam, FALSE);

		/* Initialize info for resizing subitems */
		GetClientRect(hWnd, &main_resize.rect);
		return TRUE;

	case WM_SETFOCUS:
		SetFocus(hwndList);
		break;

	case WM_SETTINGCHANGE:
		AdjustMetrics();
		return 0;

	case WM_SIZE:
		OnSize(hWnd, wParam, LOWORD(lParam), HIWORD(wParam));
		return TRUE;

	case WM_MENUSELECT:
#ifdef USE_IPS
		//menu closed, do not UpdateScreenShot() for EditControl scrolling
		if ((int)(HIWORD(wParam)) == 0xFFFF)
		{
			FreeIfAllocated(&g_IPSMenuSelectName);
			dprintf("menusele: clear");
			return 0;
		}

		i = (int)(LOWORD(wParam)) - ID_PLAY_PATCH;
		if (i >= 0 && i < MAX_PATCHES && GetPatchName(patch_name, drivers[Picker_GetSelectedItem(hwndList)]->name, i))
		{
			FreeIfAllocated(&g_IPSMenuSelectName);
			g_IPSMenuSelectName = mame_strdup(patch_name);
			dprintf("menusele: %d %s, updateSS", (int)(LOWORD(wParam)), patch_name);
			UpdateScreenShot();
		}
		else if (g_IPSMenuSelectName)
		{
			FreeIfAllocated(&g_IPSMenuSelectName);
			dprintf("menusele:none, updateSS");
			UpdateScreenShot();
		}
#endif /* USE_IPS */

		return Statusbar_MenuSelect(hWnd, wParam, lParam);

	case MM_PLAY_GAME:
		MamePlayGame();
		return TRUE;

	case WM_INITMENUPOPUP:
		UpdateMenu(GetMenu(hWnd));
		break;

	case WM_CONTEXTMENU:
		if (HandleTreeContextMenu(hWnd, wParam, lParam)
		 || HandleScreenShotContextMenu(hWnd, wParam, lParam))
			return FALSE;
		break;

	case WM_COMMAND:
		return MameCommand(hWnd,(int)(LOWORD(wParam)),(HWND)(lParam),(UINT)HIWORD(wParam));

	case WM_GETMINMAXINFO:
		/* Don't let the window get too small; it can break resizing */
		mminfo = (MINMAXINFO *) lParam;
		mminfo->ptMinTrackSize.x = MIN_WIDTH;
		mminfo->ptMinTrackSize.y = MIN_HEIGHT;
		return 0;

	case WM_TIMER:
		switch (wParam)
		{
		case JOYGUI_TIMER:
			PollGUIJoystick();
			break;
		case SCREENSHOT_TIMER:
			TabView_CalculateNextTab(hTabCtrl);
			UpdateScreenShot();
			TabView_UpdateSelection(hTabCtrl);
			break;
#if MULTISESSION
		case GAMEWND_TIMER:
			nGame = Picker_GetSelectedItem(hwndList);
			if( ! GetGameCaption() )
			{
				hGameWnd = GetGameWindow();
				if( hGameWnd )
				{
					lGameWndStyle = GetWindowLong(hGameWnd, GWL_STYLE);
					lGameWndStyle = lGameWndStyle & (WS_BORDER ^ 0xffffffff);
					SetWindowLong(hGameWnd, GWL_STYLE, lGameWndStyle);
					SetWindowPos(hGameWnd,0,0,0,0,0,SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			if( SendIconToEmulationWindow(nGame)== TRUE);
				KillTimer(hMain, GAMEWND_TIMER);
#endif // MULTISESSION
			break;
		default:
			break;
		}
		return TRUE;

	case WM_CLOSE:
		{
			/* save current item */
			RECT rect;
			AREA area;
			int nItem;
			WINDOWPLACEMENT wndpl;
			UINT state;

			wndpl.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hMain, &wndpl);
			state = wndpl.showCmd;

			/* Restore the window before we attempt to save parameters,
			 * This fixed the lost window on startup problem, among other problems
			 */
			if (state == SW_MINIMIZE || state == SW_SHOWMINIMIZED || state == SW_MAXIMIZE)
			{
				if( wndpl.flags & WPF_RESTORETOMAXIMIZED || state == SW_MAXIMIZE)
					state = SW_MAXIMIZE;
				else
				{
					state = SW_RESTORE;
					ShowWindow(hWnd, SW_RESTORE);
				}
			}
			ShowWindow(hWnd, SW_RESTORE);
			for (i = 0; i < GetSplitterCount(); i++)
				SetSplitterPos(i, nSplitterOffset[i]);
			SetWindowState(state);

			Picker_SaveColumnWidths(hwndList);
#ifdef MESS
			Picker_SaveColumnWidths(GetDlgItem(hMain, IDC_SWLIST));
#endif /* MESS */

			GetWindowRect(hWnd, &rect);
			area.x		= rect.left;
			area.y		= rect.top;
			area.width	= rect.right  - rect.left;
			area.height = rect.bottom - rect.top;
			SetWindowArea(&area);

			/* Save the users current game options and default game */
			nItem = Picker_GetSelectedItem(hwndList);
			SetDefaultGame(drivers[nItem]->name);

			/* hide window to prevent orphan empty rectangles on the taskbar */
			/* ShowWindow(hWnd,SW_HIDE); */
			DestroyWindow( hWnd );

			/* PostQuitMessage(0); */
			break;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;

		/*
		  Check to see if the mouse has been moved by the user since
		  startup. I'd like this checking to be done only in the
		  main WinProc (here), but somehow the WM_MOUSEDOWN messages
		  are eaten up before they reach MameWindowProc. That's why
		  there is one check for each of the subclassed windows too.
    
		  POSSIBLE BUGS:
		  I've included this check in the subclassed windows, but a 
		  mose move in either the title bar, the menu, or the
		  toolbar will not generate a WM_MOUSEOVER message. At least
		  not one that I know how to pick up. A solution could maybe
		  be to subclass those too, but that's too much work :)
		*/

	case WM_MOUSEMOVE:
	{
		if (MouseHasBeenMoved())
			ShowCursor(TRUE);

		if (g_listview_dragging)
			MouseMoveListViewDrag(MAKEPOINTS(lParam));
		else
			/* for splitters */
			OnMouseMove(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;
	}

	case WM_LBUTTONUP:
		if (g_listview_dragging)
			ButtonUpListViewDrag(MAKEPOINTS(lParam));
		else
			/* for splitters */
			OnLButtonUp(hWnd, (UINT)wParam, MAKEPOINTS(lParam));
		break;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wParam;
			char fileName[MAX_PATH];

			if (OnNT())
			{
				WCHAR fileNameW[MAX_PATH];
				DragQueryFileW(hDrop, 0, fileNameW, MAX_PATH);
				strcpy(fileName, _String((LPTSTR)fileNameW));
			} else {
				DragQueryFileA(hDrop, 0, fileName, MAX_PATH);
			}
			DragFinish(hDrop);

			DragAcceptFiles(hMain, FALSE);
			SetForegroundWindow(hMain);
			MamePlayBackGame(fileName);
			DragAcceptFiles(hMain, TRUE);

		}
		break;

	case WM_NOTIFY:
		/* Where is this message intended to go */
		{
			LPNMHDR lpNmHdr = (LPNMHDR)lParam;

			/* Fetch tooltip text */
			if (lpNmHdr->code == TTN_NEEDTEXTW)
			{
				LPTOOLTIPTEXTW lpttt = (LPTOOLTIPTEXTW)lParam;
				CopyToolTipTextW(lpttt);
			}
			if (lpNmHdr->code == TTN_NEEDTEXTA)
			{
				LPTOOLTIPTEXTA lpttt = (LPTOOLTIPTEXTA)lParam;
				CopyToolTipTextA(lpttt);
			}

			if (lpNmHdr->hwndFrom == hTreeView)
				return TreeViewNotify(lpNmHdr);

			GetClassNameA(lpNmHdr->hwndFrom, szClass, sizeof(szClass) / sizeof(szClass[0]));
			if (!strcmp(szClass, "SysListView32"))
				return Picker_HandleNotify(lpNmHdr);	
			if (!strcmp(szClass, "SysTabControl32"))
				return TabView_HandleNotify(lpNmHdr);
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDis = (LPDRAWITEMSTRUCT)lParam;

			GetClassNameA(lpDis->hwndItem, szClass, sizeof(szClass) / sizeof(szClass[0]));
			if (!strcmp(szClass, "SysListView32"))
				Picker_HandleDrawItem(GetDlgItem(hMain, lpDis->CtlID), lpDis);
		}
		break;

	case WM_MEASUREITEM :
	{
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

		// tell the list view that each row (item) should be just taller than our font

		//DefWindowProc(hWnd, message, wParam, lParam);
		//dprintf("default row height calculation gives %u\n",lpmis->itemHeight);

		TEXTMETRIC tm;
		HDC hDC = GetDC(NULL);
		HFONT hFontOld = (HFONT)SelectObject(hDC,hFont);

		GetTextMetrics(hDC,&tm);
		
		lpmis->itemHeight = tm.tmHeight + tm.tmExternalLeading + 1;
		if (lpmis->itemHeight < 17)
			lpmis->itemHeight = 17;
		//dprintf("we would do %u\n",tm.tmHeight + tm.tmExternalLeading + 1);
		SelectObject(hDC,hFontOld);
		ReleaseDC(NULL,hDC);

		return TRUE;
	}
	default:

		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static int HandleKeyboardGUIMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
		case WM_CHAR: /* List-View controls use this message for searching the items "as user types" */
			//MessageBox(NULL,"wm_char message arrived","TitleBox",MB_OK);
			return TRUE;

		case WM_KEYDOWN:
			KeyboardKeyDown(0, wParam, lParam);
			return TRUE;

		case WM_KEYUP:
			KeyboardKeyUp(0, wParam, lParam);
			return TRUE;

		case WM_SYSKEYDOWN:
			KeyboardKeyDown(1, wParam, lParam);
			return TRUE;

		case WM_SYSKEYUP:
			KeyboardKeyUp(1, wParam, lParam);
			return TRUE;
	}

	return FALSE;	/* message not processed */
}

static BOOL PumpMessage()
{
	MSG msg;

	if (!GetMessage(&msg, NULL, 0, 0))
	{
		return FALSE;
	}

	if (IsWindow(hMain))
	{
		BOOL absorbed_key = FALSE;
		if (GetKeyGUI())
			absorbed_key = HandleKeyboardGUIMessage(msg.hwnd, msg.message, 
			                                        msg.wParam, msg.lParam);
		else
			absorbed_key = TranslateAccelerator(hMain, hAccel, &msg);

		if (!absorbed_key)
		{
			if (!IsDialogMessage(hMain, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return TRUE;
}

static BOOL FolderCheck(void)
{
	
	char *pDescription = NULL;
	int nGameIndex = 0;
	int i=0;
	int iStep = 0;
	LV_FINDINFO lvfi;
	int nCount = ListView_GetItemCount(hwndList);
	BOOL changed = FALSE;

	MSG msg;
	for(i=0; i<nCount;i++)
	{
		LV_ITEM lvi;

		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.mask	 = LVIF_PARAM;
		ListView_GetItem(hwndList, &lvi);
		nGameIndex  = lvi.lParam;
		SetRomAuditResults(nGameIndex, UNKNOWN);
		SetSampleAuditResults(nGameIndex, UNKNOWN);
	}
	if( nCount > 0)
		ProgressBarShow();
	else
		return FALSE;
	if( nCount < 100 )
		iStep = 100 / nCount;
	else
		iStep = nCount/100;
	UpdateListView();
	UpdateWindow(hMain);
	for(i=0; i<nCount;i++)
	{
		LV_ITEM lvi;

		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.mask	 = LVIF_PARAM;
		ListView_GetItem(hwndList, &lvi);
		nGameIndex  = lvi.lParam;
		if (GetRomAuditResults(nGameIndex) == UNKNOWN)
		{
			Mame32VerifyRomSet(nGameIndex);
			changed = TRUE;
		}

		if (GetSampleAuditResults(nGameIndex) == UNKNOWN)
		{
			Mame32VerifySampleSet(nGameIndex);
			changed = TRUE;
		}

		lvfi.flags	= LVFI_PARAM;
		lvfi.lParam = nGameIndex;

		i = ListView_FindItem(hwndList, -1, &lvfi);
		if (changed && i != -1);
		{
			ListView_RedrawItems(hwndList, i, i);
			while( PeekMessage( &msg, hwndList, 0, 0, PM_REMOVE ) != 0)
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
			}
		}
		changed = FALSE;
		if ((i % iStep) == 0)
			ProgressBarStepParam(i, nCount);
	}
	ProgressBarHide();
	pDescription = UseLangList() ? _LST(drivers[Picker_GetSelectedItem(hwndList)]->description) : ModifyThe(drivers[Picker_GetSelectedItem(hwndList)]->description);
	SetStatusBarText(0, pDescription);
	UpdateStatusBar();
	return TRUE;
}

static BOOL GameCheck(void)
{
	LV_FINDINFO lvfi;
	int i;
	BOOL changed = FALSE;

	if (game_index == 0)
		ProgressBarShow();

	if (game_index >= game_count)
	{
		bDoGameCheck = FALSE;
		ProgressBarHide();
		ResetWhichGamesInFolders();
		return FALSE;
	}

	if (GetRomAuditResults(game_index) == UNKNOWN)
	{
		Mame32VerifyRomSet(game_index);
		changed = TRUE;
	}

	if (GetSampleAuditResults(game_index) == UNKNOWN)
	{
		Mame32VerifySampleSet(game_index);
		changed = TRUE;
	}

	lvfi.flags	= LVFI_PARAM;
	lvfi.lParam = game_index;

	i = ListView_FindItem(hwndList, -1, &lvfi);
	if (changed && i != -1);
		ListView_RedrawItems(hwndList, i, i);
	if ((game_index % progBarStep) == 0)
		ProgressBarStep();
	game_index++;

	return changed;
}

static BOOL OnIdle(HWND hWnd)
{
	static int bFirstTime = TRUE;
	static int bResetList = TRUE;

	char *pDescription;
	int driver_index;

	if (bFirstTime)
	{
		bResetList = FALSE;
		bFirstTime = FALSE;
	}
	if (bDoGameCheck)
	{
		bResetList |= GameCheck();
		return idle_work;
	}
	// NPW 17-Jun-2003 - Commenting this out because it is redundant
	// and it causes the game to reset back to the original game after an F5 
	// refresh
	//driver_index = GetGameNameIndex(GetDefaultGame());
	//SetSelectedPickItem(driver_index);

	// in case it's not found, get it back
	driver_index = Picker_GetSelectedItem(hwndList);

	pDescription = UseLangList() ? _LST(drivers[driver_index]->description) : ModifyThe(drivers[driver_index]->description);
	SetStatusBarText(0, pDescription);
	if (bResetList || (GetViewMode() == VIEW_LARGE_ICONS))
	{
		ResetWhichGamesInFolders();
		ResetListView();
	}
	idle_work = FALSE;
	UpdateStatusBar();
	bFirstTime = TRUE;

	if (!idle_work)
		PostMessage(GetMainWindow(),WM_COMMAND, MAKEWPARAM(ID_VIEW_LINEUPICONS, TRUE),(LPARAM)NULL);
	return idle_work;
}

static void OnSize(HWND hWnd, UINT nState, int nWidth, int nHeight)
{
	static BOOL firstTime = TRUE;

	if (nState != SIZE_MAXIMIZED && nState != SIZE_RESTORED)
		return;

	ResizeWindow(hWnd, &main_resize);
	ResizeProgressBar();
	if (firstTime == FALSE)
		OnSizeSplitter(hMain);
	//firstTime = FALSE;
	/* Update the splitters structures as appropriate */
	RecalcSplitters();
	if (firstTime == FALSE)
		ResizePickerControls(hMain);
	firstTime = FALSE;
	UpdateScreenShot();
}



static HWND GetResizeItemWindow(HWND hParent, const ResizeItem *ri)
{
	HWND hControl;
	if (ri->type == RA_ID)
		hControl = GetDlgItem(hParent, ri->u.id);
	else
		hControl = ri->u.hwnd;
	return hControl;
}



static void SetAllWindowsFont(HWND hParent, const Resize *r, HFONT hTheFont, BOOL bRedraw)
{
	int i;
	HWND hControl;

	for (i = 0; r->items[i].type != RA_END; i++)
	{
		if (r->items[i].setfont)
		{
			hControl = GetResizeItemWindow(hParent, &r->items[i]);
			SetWindowFont(hControl, hTheFont, bRedraw);
		}
	}

	hControl = GetDlgItem(hwndList, 0);
	if (hControl)
		TranslateControl(hControl);
}



static void ResizeWindow(HWND hParent, Resize *r)
{
	int cmkindex = 0, dx, dy, dx1, dtempx;
	HWND hControl;
	RECT parent_rect, rect;
	ResizeItem *ri;
	POINT p = {0, 0};

	if (hParent == NULL)
		return;

	/* Calculate change in width and height of parent window */
	GetClientRect(hParent, &parent_rect);
	//dx = parent_rect.right - r->rect.right;
	dtempx = parent_rect.right - r->rect.right;
	dy = parent_rect.bottom - r->rect.bottom;
	dx = dtempx/2;
	dx1 = dtempx - dx;
	ClientToScreen(hParent, &p);

	while (r->items[cmkindex].type != RA_END)
	{
		ri = &r->items[cmkindex];
		if (ri->type == RA_ID)
			hControl = GetDlgItem(hParent, ri->u.id);
		else
			hControl = ri->u.hwnd;

		if (hControl == NULL)
		{
			cmkindex++;
			continue;
		}

		/* Get control's rectangle relative to parent */
		GetWindowRect(hControl, &rect);
		OffsetRect(&rect, -p.x, -p.y);

		if (!(ri->action & RA_LEFT))
			rect.left += dx;

		if (!(ri->action & RA_TOP))
			rect.top += dy;

		if (ri->action & RA_RIGHT)
			rect.right += dx;

		if (ri->action & RA_BOTTOM)
			rect.bottom += dy;

		MoveWindow(hControl, rect.left, rect.top,
		           (rect.right - rect.left),
		           (rect.bottom - rect.top), TRUE);

		/* Take care of subcontrols, if appropriate */
		if (ri->subwindow != NULL)
			ResizeWindow(hControl, ri->subwindow);

		cmkindex++;
	}

	/* Record parent window's new location */
	memcpy(&r->rect, &parent_rect, sizeof(RECT));
}

static void ProgressBarShow()
{
	RECT rect;
	int  widths[2] = {150, -1};

	if (game_count < 100)
		progBarStep = 100 / game_count;
	else
		progBarStep = (game_count / 100);

	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)(LPINT)widths);
	SendMessage(hProgWnd, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, game_count));
	SendMessage(hProgWnd, PBM_SETSTEP, (WPARAM)progBarStep, 0);
	SendMessage(hProgWnd, PBM_SETPOS, 0, 0);

	StatusBar_GetItemRect(hStatusBar, 1, &rect);

	MoveWindow(hProgWnd, rect.left, rect.top,
	           rect.right - rect.left,
	           rect.bottom - rect.top, TRUE);

	bProgressShown = TRUE;
}

static void ProgressBarHide()
{
	RECT rect;
	int  widths[4];
	HDC  hDC;
	SIZE size;
	int  numParts = 4;

	if (hProgWnd == NULL)
	{
		return;
	}

	hDC = GetDC(hProgWnd);

	ShowWindow(hProgWnd, SW_HIDE);

	GetTextExtentPoint32A(hDC, "MMX", 3 , &size);
	widths[3] = size.cx;
	GetTextExtentPoint32A(hDC, "MMMM games", 10, &size);
	widths[2] = size.cx;
	//Just specify 24 instead of 30, gives us sufficient space to display the message, and saves some space
	GetTextExtentPoint32(hDC, TEXT("Screen flip support is missing"), 24, &size);
	widths[1] = size.cx;

	ReleaseDC(hProgWnd, hDC);

	widths[0] = -1;
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)1, (LPARAM)(LPINT)widths);
	StatusBar_GetItemRect(hStatusBar, 0, &rect);

	widths[0] = (rect.right - rect.left) - (widths[1] + widths[2] + widths[3]);
	widths[1] += widths[0];
	widths[2] += widths[1];
	widths[3] = -1;

	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)numParts, (LPARAM)(LPINT)widths);
	UpdateStatusBar();

	bProgressShown = FALSE;
}

static void ResizeProgressBar()
{
	if (bProgressShown)
	{
		RECT rect;
		int  widths[2] = {150, -1};

		SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)2, (LPARAM)(LPINT)widths);
		StatusBar_GetItemRect(hStatusBar, 1, &rect);
		MoveWindow(hProgWnd, rect.left, rect.top,
		           rect.right  - rect.left,
		           rect.bottom - rect.top, TRUE);
	}
	else
	{
		ProgressBarHide();
	}
}

static void ProgressBarStepParam(int iGameIndex, int nGameCount)
{
	SetStatusBarTextF(0, _UI("Game search %d%% complete"),
	                  ((iGameIndex + 1) * 100) / nGameCount);
	if (iGameIndex == 0)
		ShowWindow(hProgWnd, SW_SHOW);
	SendMessage(hProgWnd, PBM_STEPIT, 0, 0);
}

static void ProgressBarStep()
{
	ProgressBarStepParam(game_index, game_count);
}

static HWND InitProgressBar(HWND hParent)
{
	RECT rect;

	StatusBar_GetItemRect(hStatusBar, 0, &rect);

	rect.left += 150;

	return CreateWindowEx(WS_EX_STATICEDGE,
			PROGRESS_CLASS,
			TEXT("Progress Bar"),
			WS_CHILD | PBS_SMOOTH,
			rect.left,
			rect.top,
			rect.right	- rect.left,
			rect.bottom - rect.top,
			hParent,
			NULL,
			hInst,
			NULL);
}

static void CopyToolTipTextW(LPTOOLTIPTEXTW lpttt)
{
	int   i;
	int   id = lpttt->hdr.idFrom;
	int   iButton = lpttt->hdr.idFrom;
	static char String[1024];
	BOOL bConverted = FALSE;
	//LPWSTR pDest = lpttt->lpszText;
	/* Map command ID to string index */
	for (i = 0; CommandToString[i] != -1; i++)
	{
		if (CommandToString[i] == id)
		{
			iButton = i;
			bConverted = TRUE;
			break;
		}
	}
	if( bConverted )
	{
		/* Check for valid parameter */
		if (iButton > NUM_TOOLTIPS)
		{
			strcpy(String, _UI("Invalid Button Index"));
		}
		else
		{
			strcpy(String, (id==IDC_USE_LIST && GetLangcode()==UI_LANG_EN_US) ?
			       "Modify 'The'" : _UI(szTbStrings[iButton]));
		}
	}
	else if (iButton <= 2 )
	{
		//Statusbar
		SendMessage(lpttt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 200);
		if (iButton != 1)
			SendMessage(hStatusBar, SB_GETTEXTA, (WPARAM)iButton, (LPARAM)(LPSTR) &String );
		else
			//for first pane we get the Status directly, to get the line breaks
			strcpy(String, _String(GameInfoStatus(Picker_GetSelectedItem(hwndList), FALSE)) );
	}
	else
		strcpy(String, _UI("Invalid Button Index"));

	lstrcpy(lpttt->lpszText, _Unicode(String));
}

static void CopyToolTipTextA(LPTOOLTIPTEXTA lpttt)
{
	int   i;
	int   id = lpttt->hdr.idFrom;
	int   iButton = lpttt->hdr.idFrom;
	static char String[1024];
	BOOL bConverted = FALSE;
	//LPSTR pDest = lpttt->lpszText;
	/* Map command ID to string index */
	for (i = 0; CommandToString[i] != -1; i++)
	{
		if (CommandToString[i] == id)
		{
			iButton = i;
			bConverted = TRUE;
			break;
		}
	}

	if( bConverted )
	{
		/* Check for valid parameter */
		if (iButton > NUM_TOOLTIPS)
		{
			strcpy(String, _UI("Invalid Button Index"));
		}
		else
		{
			strcpy(String, (id==IDC_USE_LIST && GetLangcode()==UI_LANG_EN_US) ?
			       "Modify 'The'" : _UI(szTbStrings[iButton]));
		}
	}
	else if (iButton <= 2 )
	{
		//Statusbar
		SendMessage(lpttt->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 140);
		if (iButton != 1)
			SendMessage(hStatusBar, SB_GETTEXTA, (WPARAM)iButton, (LPARAM)(LPSTR) &String );
		else
			//for first pane we get the Status directly, to get the line breaks
			strcpy(String, _String(GameInfoStatus(Picker_GetSelectedItem(hwndList), FALSE)) );
	}
	else
		strcpy(String, _UI("Invalid Button Index"));

	strcpy(lpttt->lpszText, String);
}

static HWND InitToolbar(HWND hParent)
{
	HWND hToolBar = CreateToolbarEx(hParent,
	                       WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                       CCS_TOP | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
	                       1,
	                       8,
	                       hInst,
	                       IDB_TOOLBAR_US + GetLangcode(),
	                       tbb,
	                       NUM_TOOLBUTTONS,
	                       16,
	                       16,
	                       0,
	                       0,
	                       sizeof(TBBUTTON));

	// create Edit Control
	HWND hEdit= CreateWindowEx( 0L, _Unicode("Edit"), _Unicode(_UI(SEARCH_PROMPT)), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT, 
								260, 1, 200, 20, hParent, (HMENU) ID_TOOLBAR_EDIT, hInst, 0 );
	SetParent (hEdit, hToolBar);

	return hToolBar;
}

static HWND InitStatusBar(HWND hParent)
{
#if 0
	HMENU hMenu = GetMenu(hParent);

	popstr[0].hMenu    = 0;
	popstr[0].uiString = 0;
	popstr[1].hMenu    = hMenu;
	popstr[1].uiString = IDS_UI_FILE;
	popstr[2].hMenu    = GetSubMenu(hMenu, 1);
	popstr[2].uiString = IDS_UI_TOOLBAR;
	popstr[3].hMenu    = 0;
	popstr[3].uiString = 0;
#endif

	return CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                          CCS_BOTTOM | SBARS_SIZEGRIP | SBT_TOOLTIPS,
	                          _Unicode(_UI("Ready")),
	                          hParent,
	                          2);
}


static LRESULT Statusbar_MenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#if 0
	UINT  fuFlags	= (UINT)HIWORD(wParam);
	HMENU hMainMenu = NULL;
	int   iMenu 	= 0;

	/* Handle non-system popup menu descriptions. */
	if (  (fuFlags & MF_POPUP)
	&&	(!(fuFlags & MF_SYSMENU)))
	{
		for (iMenu = 1; iMenu < MAX_MENUS; iMenu++)
		{
			if ((HMENU)lParam == popstr[iMenu].hMenu)
			{
				hMainMenu = (HMENU)lParam;
				break ;
			}
		}
	}

	if (hMainMenu)
	{
		/* Display helpful text in status bar */
		MenuHelp(WM_MENUSELECT, wParam, lParam, hMainMenu, hInst,
				 hStatusBar, (UINT *)&popstr[iMenu]);
	}
	else
	{
		UINT nZero = 0;
		MenuHelp(WM_MENUSELECT, wParam, lParam, NULL, hInst,
				 hStatusBar, &nZero);
	}
#else
	char *p = TranslateMenuHelp((HMENU)lParam, (UINT)LOWORD(wParam), HIWORD(wParam) & MF_POPUP);
	StatusBarSetTextA(hStatusBar, 0, p);
#endif

	return 0;
}

static void UpdateStatusBar()
{
	LPTREEFOLDER lpFolder = GetCurrentFolder();
	int 		 games_shown = 0;
	int 		 i = -1;

	if (!lpFolder)
		return;

	while (1)
	{
		i = FindGame(lpFolder,i+1);
		if (i == -1)
			break;

		if (!GameFiltered(i, lpFolder->m_dwFlags))
			games_shown++;
	}

	/* Show number of games in the current 'View' in the status bar */
	SetStatusBarTextF(2, _UI("%d games"), games_shown);

	i = Picker_GetSelectedItem(hwndList);

	if (games_shown == 0)
		DisableSelection();
	else
		SetStatusBarTextW(1, GameInfoStatus(i, FALSE));
}

static BOOL NeedScreenShotImage(void)
{
#ifdef USE_IPS
	if (g_IPSMenuSelectName)
		return TRUE;
#endif /* USE_IPS */

	if (TabView_GetCurrentTab(hTabCtrl) == TAB_HISTORY && GetShowTab(TAB_HISTORY))
		return FALSE;

#ifdef STORY_DATAFILE
	if (TabView_GetCurrentTab(hTabCtrl) == TAB_STORY && GetShowTab(TAB_STORY))
		return FALSE;
#endif /* STORY_DATAFILE */

	return TRUE;
}

static BOOL NeedHistoryText(void)
{
#ifdef USE_IPS
	if (g_IPSMenuSelectName)
		return TRUE;
#endif /* USE_IPS */

	if (TabView_GetCurrentTab(hTabCtrl) == TAB_HISTORY)
		return TRUE;
	if (GetShowTab(TAB_HISTORY) == FALSE)
	{
		if (TabView_GetCurrentTab(hTabCtrl) == GetHistoryTab())
			return TRUE;
		if (TAB_ALL == GetHistoryTab())
			return TRUE;
	}

#ifdef STORY_DATAFILE
	if (TabView_GetCurrentTab(hTabCtrl) == TAB_STORY)
		return TRUE;
#endif /* STORY_DATAFILE */

	return FALSE;
}

static void UpdateHistory(void)
{
	HDC hDC;
	RECT rect;
	TEXTMETRIC     tm ;
	int nLines, nLineHeight;
	//DWORD dwStyle = GetWindowLong(GetDlgItem(hMain, IDC_HISTORY), GWL_STYLE);
	have_history = FALSE;

	if (GetSelectedPick() >= 0)
	{
		LPCWSTR histText;

#ifdef USE_IPS
		if (g_IPSMenuSelectName)
			histText = GetPatchDesc(drivers[Picker_GetSelectedItem(hwndList)]->name, g_IPSMenuSelectName);
		else
#endif /* USE_IPS */
#ifdef STORY_DATAFILE
			if (TabView_GetCurrentTab(hTabCtrl) == TAB_STORY)
				histText = GetGameStory(Picker_GetSelectedItem(hwndList));
			else
#endif /* STORY_DATAFILE */
				histText = GetGameHistory(Picker_GetSelectedItem(hwndList));

		have_history = (histText && histText[0]) ? TRUE : FALSE;
		Edit_SetText(GetDlgItem(hMain, IDC_HISTORY), histText);
	}

	if (have_history && GetShowScreenShot() && NeedHistoryText())
	{
		RECT sRect;

		sRect.left = history_rect.left;
		sRect.right = history_rect.right;

		if (!NeedScreenShotImage())
		{
			// We're using the new mode, with the history filling the entire tab (almost)
			sRect.top = history_rect.top + 14;
			sRect.bottom = (history_rect.bottom - history_rect.top) - 30;   
		}
		else
		{
			// We're using the original mode, with the history beneath the SS picture
			sRect.top = history_rect.top + 264;
			sRect.bottom = (history_rect.bottom - history_rect.top) - 278;
		}

		MoveWindow(GetDlgItem(hMain, IDC_HISTORY),
			sRect.left, sRect.top,
			sRect.right, sRect.bottom, TRUE);

		Edit_GetRect(GetDlgItem(hMain, IDC_HISTORY), &rect);
		nLines = Edit_GetLineCount(GetDlgItem(hMain, IDC_HISTORY));
		hDC = GetDC(GetDlgItem(hMain, IDC_HISTORY));
		GetTextMetrics (hDC, &tm);
		nLineHeight = tm.tmHeight - tm.tmInternalLeading;
		if ((rect.bottom - rect.top) / nLineHeight < nLines)
		{
			//more than one Page, so show Scrollbar
			SetScrollRange(GetDlgItem(hMain, IDC_HISTORY), SB_VERT, 0, nLines, TRUE); 
		}
		else
		{
			//hide Scrollbar
			SetScrollRange(GetDlgItem(hMain, IDC_HISTORY),SB_VERT, 0, 0, TRUE);

		}
 		ShowWindow(GetDlgItem(hMain, IDC_HISTORY), SW_SHOW);
	}
	else
		ShowWindow(GetDlgItem(hMain, IDC_HISTORY), SW_HIDE);

}


static void DisableSelection()
{
	MENUITEMINFO	mmi;
	HMENU			hMenu = GetMenu(hMain);
	BOOL			prev_have_selection = have_selection;

	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(_UI("&Play"));
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mmi);

	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(_UI("Propert&ies for Driver"));
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FOLDER_SOURCEPROPERTIES, FALSE, &mmi);

	EnableMenuItem(hMenu, ID_FILE_PLAY, 		   MF_GRAYED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,	   MF_GRAYED);
	EnableMenuItem(hMenu, ID_GAME_PROPERTIES,	   MF_GRAYED);
	EnableMenuItem(hMenu, ID_FOLDER_SOURCEPROPERTIES,	   MF_GRAYED);
#ifdef USE_VIEW_PCBINFO
	EnableMenuItem(hMenu, ID_VIEW_PCBINFO,		   MF_GRAYED);
#endif /* USE_VIEW_PCBINFO */

	SetStatusBarText(0, _UI("No Selection"));
	SetStatusBarText(1, "");
	SetStatusBarText(3, "");

	have_selection = FALSE;

	if (prev_have_selection != have_selection)
		UpdateScreenShot();
}

static void EnableSelection(int nGame)
{
	char		buf[200];
	const char *	pText;
	MENUITEMINFO	mmi;
	HMENU		hMenu = GetMenu(hMain);

	
	snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("&Play %s"),
	         ConvertAmpersandString(UseLangList() ?
	                                _LST(drivers[nGame]->description) :
	                                ModifyThe(drivers[nGame]->description)));
	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(buf);
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mmi);

	snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("Propert&ies for %s"),
	         GetDriverFilename(nGame));
	mmi.cbSize         = sizeof(mmi);
	mmi.fMask          = MIIM_TYPE;
	mmi.fType          = MFT_STRING;
	mmi.dwTypeData     = _Unicode(buf);
	mmi.cch            = lstrlen(mmi.dwTypeData);
	SetMenuItemInfo(hMenu, ID_FOLDER_SOURCEPROPERTIES, FALSE, &mmi);

	pText = UseLangList() ?
		_LST(drivers[nGame]->description) :
		ModifyThe(drivers[nGame]->description);
	SetStatusBarText(0, pText);
	/* Add this game's status to the status bar */
	SetStatusBarTextW(1, GameInfoStatus(nGame, FALSE));
	SetStatusBarText(3, "");

	/* If doing updating game status and the game name is NOT pacman.... */

	EnableMenuItem(hMenu, ID_FILE_PLAY, 		   MF_ENABLED);
	EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,	   MF_ENABLED);
#ifdef USE_VIEW_PCBINFO
	EnableMenuItem(hMenu, ID_VIEW_PCBINFO,		   MF_ENABLED);
#endif /* USE_VIEW_PCBINFO */

	if (!oldControl)
	{
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,          MF_ENABLED);
		EnableMenuItem(hMenu, ID_FOLDER_SOURCEPROPERTIES,	   MF_ENABLED);
	}

	if (bProgressShown && bListReady == TRUE)
	{
		SetDefaultGame(drivers[nGame]->name);
	}
	have_selection = TRUE;

	UpdateScreenShot();
}

#ifdef USE_VIEW_PCBINFO
void PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y)
#else /* USE_VIEW_PCBINFO */
static void PaintBackgroundImage(HWND hWnd, HRGN hRgn, int x, int y)
#endif /* USE_VIEW_PCBINFO */
{
	RECT		rcClient;
	HRGN		rgnBitmap;
	HPALETTE	hPAL;
	HDC 		hDC = GetDC(hWnd);
	int 		i, j;
	HDC 		htempDC;
	HBITMAP 	oldBitmap;

	/* x and y are offsets within the background image that should be at 0,0 in hWnd */

	/* So we don't paint over the control's border */
	GetClientRect(hWnd, &rcClient);

	htempDC = CreateCompatibleDC(hDC);
	oldBitmap = SelectObject(htempDC, hBackground);

	if (hRgn == NULL)
	{
		/* create a region of our client area */
		rgnBitmap = CreateRectRgnIndirect(&rcClient);
		SelectClipRgn(hDC, rgnBitmap);
		DeleteObject(rgnBitmap);
	}
	else
	{
		/* use the passed in region */
		SelectClipRgn(hDC, hRgn);
	}

	hPAL = GetBackgroundPalette();
	if (hPAL == NULL)
		hPAL = CreateHalftonePalette(hDC);

	if (GetDeviceCaps(htempDC, RASTERCAPS) & RC_PALETTE && hPAL != NULL)
	{
		SelectPalette(htempDC, hPAL, FALSE);
		RealizePalette(htempDC);
	}

	for (i = rcClient.left-x; i < rcClient.right; i += bmDesc.bmWidth)
		for (j = rcClient.top-y; j < rcClient.bottom; j += bmDesc.bmHeight)
			BitBlt(hDC, i, j, bmDesc.bmWidth, bmDesc.bmHeight, htempDC, 0, 0, SRCCOPY);

	SelectObject(htempDC, oldBitmap);
	DeleteDC(htempDC);

	if (GetBackgroundPalette() == NULL)
	{
		DeleteObject(hPAL);
		hPAL = NULL;
	}

	ReleaseDC(hWnd, hDC);
}

static LPCSTR GetCloneParentName(int nItem)
{
	const game_driver *clone_of = NULL;
	if (DriverIsClone(nItem) == TRUE)
	{
		clone_of = driver_get_clone(drivers[nItem]);
		if( clone_of )
			return  (UseLangList()) ? 
					_LST(clone_of->description) : ModifyThe(clone_of->description);
	}
	return "";
}

static BOOL PickerHitTest(HWND hWnd)
{
	RECT			rect;
	POINTS			p;
	DWORD			res = GetMessagePos();
	LVHITTESTINFO	htInfo;

	ZeroMemory(&htInfo,sizeof(LVHITTESTINFO));
	p = MAKEPOINTS(res);
	GetWindowRect(hWnd, &rect);
	htInfo.pt.x = p.x - rect.left;
	htInfo.pt.y = p.y - rect.top;
	ListView_HitTest(hWnd, &htInfo);

	return (! (htInfo.flags & LVHT_NOWHERE));
}

static BOOL TreeViewNotify(LPNMHDR nm)
{
	switch (nm->code)
	{
	case TVN_SELCHANGEDW :
	case TVN_SELCHANGEDA:
	    {
		HTREEITEM hti = TreeView_GetSelection(hTreeView);
		TVITEM	  tvi;

		tvi.mask  = TVIF_PARAM | TVIF_HANDLE;
		tvi.hItem = hti;

		if (TreeView_GetItem(hTreeView, &tvi))
		{
			SetCurrentFolder((LPTREEFOLDER)tvi.lParam);
			if (bListReady)
			{
				ResetListView();
				UpdateScreenShot();
			}
		}
		return TRUE;
	    }
	case TVN_BEGINLABELEDITW :
	case TVN_BEGINLABELEDITA :
	    {
		TV_DISPINFO *ptvdi = (TV_DISPINFO *)nm;
		LPTREEFOLDER folder = (LPTREEFOLDER)ptvdi->item.lParam;

		if (folder->m_dwFlags & F_CUSTOM)
		{
			// user can edit custom folder names
			g_in_treeview_edit = TRUE;
			return FALSE;
		}
		// user can't edit built in folder names
		return TRUE;
	    }
	case TVN_ENDLABELEDITW :
	case TVN_ENDLABELEDITA :
	    {
		TV_DISPINFO *ptvdi = (TV_DISPINFO *)nm;
		LPTREEFOLDER folder = (LPTREEFOLDER)ptvdi->item.lParam;

		g_in_treeview_edit = FALSE;

		if (ptvdi->item.pszText == NULL || lstrlen(ptvdi->item.pszText) == 0)
			return FALSE;

		return TryRenameCustomFolder(folder,_String(ptvdi->item.pszText));
	    }
	}
	return FALSE;
}



static void GamePicker_OnHeaderContextMenu(POINT pt, int nColumn)
{
	// Right button was clicked on header
	HMENU hMenuLoad;
	HMENU hMenu;

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_HEADER));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_SORT_ASCENDING);
	lastColumnClick = nColumn;
	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hMain,NULL);

	DestroyMenu(hMenuLoad);
}



char* ConvertAmpersandString(const char *s)
{
	/* takes a string and changes any ampersands to double ampersands,
	   for setting text of window controls that don't allow us to disable
	   the ampersand underlining.
	 */
	/* returns a static buffer--use before calling again */

	static char buf[200];
	char *ptr;

	ptr = buf;
	while (*s)
	{
		if (*s == '&')
			*ptr++ = *s;
		*ptr++ = *s++;
	}
	*ptr = 0;

	return buf;
}

static int GUI_seq_pressed(input_code *code)
{
	int j;
	int res = 1;
	int invert = 0;
	int count = 0;

	for(j=0;j<SEQ_MAX;++j)
	{
		switch (code[j])
		{
			case CODE_NONE :
				return res && count;
			case CODE_OR :
				if (res && count)
					return 1;
				res = 1;
				count = 0;
				break;
			case CODE_NOT :
				invert = !invert;
				break;
			default:
				if (res)
				{
					int pressed = keyboard_state[code[j]];
					if ((pressed != 0) == invert)
						res = 0;
				}
				invert = 0;
				++count;
		}
	}
	return res && count;
}

static void check_for_GUI_action(void)
{
	int i;

	for (i = 0; i < NUM_GUI_SEQUENCES; i++)
	{
		input_seq *is = &(GUISequenceControl[i].is);

		if (GUI_seq_pressed(is->code))
		{
			dprintf("seq =%s pressed\n", GUISequenceControl[i].name);
			switch (GUISequenceControl[i].func_id)
			{
			case ID_GAME_AUDIT:
			case ID_GAME_PROPERTIES:
			case ID_CONTEXT_FILTERS:
			case ID_UI_START:
				KeyboardStateClear(); /* beacuse whe won't receive KeyUp mesage when we loose focus */
				break;
			default:
				break;
			}
			SendMessage(hMain, WM_COMMAND, GUISequenceControl[i].func_id, 0);
		}
	}
}

static void KeyboardStateClear(void)
{
	int i;

	for (i = 0; i < __code_max; i++)
		keyboard_state[i] = 0;

	dprintf("keyboard gui state cleared.\n");
}


static void KeyboardKeyDown(int syskey, int vk_code, int special)
{
	int i, found = 0;
	input_code icode = 0;
	int special_code = (special >> 24) & 1;
	int scancode = (special>>16) & 0xff;

	if ((vk_code==VK_MENU) || (vk_code==VK_CONTROL) || (vk_code==VK_SHIFT))
	{
		found = 1;

		/* a hack for right shift - it's better to use Direct X for keyboard input it seems......*/
		if (vk_code==VK_SHIFT)
			if (scancode>0x30) /* on my keyboard left shift scancode is 0x2a, right shift is 0x36 */
				special_code = 1;

		if (special_code) /* right hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_RALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_RCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_RSHIFT;
				break;
			}
		}
		else /* left hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_LALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_LCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_LSHIFT;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < __code_max; i++)
		{
			if ( vk_code == win_key_trans_table[i][VIRTUAL_KEY])
			{
				icode = win_key_trans_table[i][MAME_KEY];
				found = 1;
				break;
			}
		}
	}
	if (!found)
	{
		dprintf("VK_code pressed not found =  %i\n",vk_code);
		//MessageBox(NULL,"keydown message arrived not processed","TitleBox",MB_OK);
		return;
	}
	dprintf("VK_code pressed found =  %i, sys=%i, mame_keycode=%i special=%08x\n", vk_code, syskey, icode, special);
	keyboard_state[icode] = 1;
	check_for_GUI_action();
}

static void KeyboardKeyUp(int syskey, int vk_code, int special)
{
	int i, found = 0;
	input_code icode = 0;
	int special_code = (special >> 24) & 1;
	int scancode = (special>>16) & 0xff;

	if ((vk_code==VK_MENU) || (vk_code==VK_CONTROL) || (vk_code==VK_SHIFT))
	{
		found = 1;

		/* a hack for right shift - it's better to use Direct X for keyboard input it seems......*/
		if (vk_code==VK_SHIFT)
			if (scancode>0x30) /* on my keyboard left shift scancode is 0x2a, right shift is 0x36 */
				special_code = 1;

		if (special_code) /* right hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_RALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_RCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_RSHIFT;
				break;
			}
		}
		else /* left hand keys */
		{
			switch(vk_code)
			{
			case VK_MENU:
				icode = KEYCODE_LALT;
				break;
			case VK_CONTROL:
				icode = KEYCODE_LCONTROL;
				break;
			case VK_SHIFT:
				icode = KEYCODE_LSHIFT;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < __code_max; i++)
		{
			if (vk_code == win_key_trans_table[i][VIRTUAL_KEY])
			{
				icode = win_key_trans_table[i][MAME_KEY];
				found = 1;
				break;
			}
		}
	}
	if (!found)
	{
		dprintf("VK_code released not found =  %i",vk_code);
		//MessageBox(NULL,"keyup message arrived not processed","TitleBox",MB_OK);
		return;
	}
	keyboard_state[icode] = 0;
	dprintf("VK_code released found=  %i, sys=%i, mame_keycode=%i special=%08x\n", vk_code, syskey, icode, special );
	check_for_GUI_action();
}

static void PollGUIJoystick()
{
	// For the exec timer, will keep track of how long the button has been pressed  
	static int exec_counter = 0;

	if (in_emulation)
		return;

	if (g_pJoyGUI == NULL)
		return;

	g_pJoyGUI->poll_joysticks();


	// User pressed UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyUp(0), GetUIJoyUp(1), GetUIJoyUp(2),GetUIJoyUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_UP, 0);
	}

	// User pressed DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyDown(0), GetUIJoyDown(1), GetUIJoyDown(2),GetUIJoyDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_DOWN, 0);
	}

	// User pressed LEFT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyLeft(0), GetUIJoyLeft(1), GetUIJoyLeft(2),GetUIJoyLeft(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_LEFT, 0);
	}

	// User pressed RIGHT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyRight(0), GetUIJoyRight(1), GetUIJoyRight(2),GetUIJoyRight(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_RIGHT, 0);
	}

	// User pressed START GAME
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyStart(0), GetUIJoyStart(1), GetUIJoyStart(2),GetUIJoyStart(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_START, 0);
	}

	// User pressed PAGE UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyPageUp(0), GetUIJoyPageUp(1), GetUIJoyPageUp(2),GetUIJoyPageUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_PGUP, 0);
	}

	// User pressed PAGE DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyPageDown(0), GetUIJoyPageDown(1), GetUIJoyPageDown(2),GetUIJoyPageDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_PGDOWN, 0);
	}

	// User pressed HOME
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHome(0), GetUIJoyHome(1), GetUIJoyHome(2),GetUIJoyHome(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HOME, 0);
	}

	// User pressed END
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyEnd(0), GetUIJoyEnd(1), GetUIJoyEnd(2),GetUIJoyEnd(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_END, 0);
	}

	// User pressed CHANGE SCREENSHOT
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoySSChange(0), GetUIJoySSChange(1), GetUIJoySSChange(2),GetUIJoySSChange(3))))
	{
		SendMessage(hMain, WM_COMMAND, IDC_SSFRAME, 0);
	}

	// User pressed SCROLL HISTORY UP
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHistoryUp(0), GetUIJoyHistoryUp(1), GetUIJoyHistoryUp(2),GetUIJoyHistoryUp(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HISTORY_UP, 0);
	}

	// User pressed SCROLL HISTORY DOWN
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyHistoryDown(0), GetUIJoyHistoryDown(1), GetUIJoyHistoryDown(2),GetUIJoyHistoryDown(3))))
	{
		SendMessage(hMain, WM_COMMAND, ID_UI_HISTORY_DOWN, 0);
	}

	// User pressed EXECUTE COMMANDLINE
	if (g_pJoyGUI->is_joy_pressed(JOYCODE(GetUIJoyExec(0), GetUIJoyExec(1), GetUIJoyExec(2),GetUIJoyExec(3))))
	{
		if (++exec_counter >= GetExecWait()) // Button has been pressed > exec timeout
		{
			PROCESS_INFORMATION pi;
			STARTUPINFOA si;

			// Reset counter
			exec_counter = 0;

			ZeroMemory( &si, sizeof(si) );
			ZeroMemory( &pi, sizeof(pi) );
			si.dwFlags = STARTF_FORCEONFEEDBACK;
			si.cb = sizeof(si);

			CreateProcessA(NULL, GetExecCommand(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

			// We will not wait for the process to finish cause it might be a background task
			// The process won't get closed when MAME32 closes either.

			// But close the handles cause we won't need them anymore. Will not close process.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	else
	{
		// Button has been released within the timeout period, reset the counter
		exec_counter = 0;
	}
}

#if 0
static void PressKey(HWND hwnd, UINT vk)
{
	SendMessage(hwnd, WM_KEYDOWN, vk, 0);
	SendMessage(hwnd, WM_KEYUP,   vk, 0xc0000000);
}
#endif

static void DoSortColumn(int column)
{
	int id;

	SetSortColumn(column);

	for (id = 0; id < COLUMN_MAX; id++)
		CheckMenuItem(GetMenu(hMain), ID_VIEW_BYGAME + id, id == column ? MF_CHECKED : MF_UNCHECKED);
}

static void SetView(int menu_id)
{
	BOOL force_reset = FALSE;

	// first uncheck previous menu item, check new one
	CheckMenuRadioItem(GetMenu(hMain), ID_VIEW_LARGE_ICON, ID_VIEW_GROUPED, menu_id, MF_CHECKED);
	ToolBar_CheckButton(hToolBar, menu_id, MF_CHECKED);

	if (Picker_GetViewID(hwndList) == VIEW_GROUPED || menu_id == ID_VIEW_GROUPED)
	{
		// this changes the sort order, so redo everything
		force_reset = TRUE;
	}

	Picker_SetViewID(hwndList, menu_id - ID_VIEW_LARGE_ICON);

	if (force_reset)
	{
		Picker_Sort(hwndList);
		DoSortColumn(GetSortColumn());
	}
}

static void ResetListView()
{
	int 	i;
	int 	current_game;
	LV_ITEM lvi;
	BOOL	no_selection = FALSE;
	LPTREEFOLDER lpFolder = GetCurrentFolder();

	if (!lpFolder)
	{
		return;
	}

	/* If the last folder was empty, no_selection is TRUE */
	if (have_selection == FALSE)
	{
		no_selection = TRUE;
	}

	current_game = Picker_GetSelectedItem(hwndList);

	SetWindowRedraw(hwndList,FALSE);

	ListView_DeleteAllItems(hwndList);

	// hint to have it allocate it all at once
	ListView_SetItemCount(hwndList,game_count);

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.stateMask = 0;

	i = -1;

	do
	{
		/* Add the games that are in this folder */
		if ((i = FindGame(lpFolder, i + 1)) != -1)
		{
			if (GameFiltered(i, lpFolder->m_dwFlags))
				continue;

			lvi.iItem	 = i;
			lvi.iSubItem = 0;
			lvi.lParam	 = i;
			lvi.pszText  = LPSTR_TEXTCALLBACK;
			lvi.iImage	 = I_IMAGECALLBACK;
			ListView_InsertItem(hwndList, &lvi);
		}
	} while (i != -1);

	Picker_Sort(hwndList);
	DoSortColumn(GetSortColumn());

	if (bListReady)
	{
		/* If last folder was empty, select the first item in this folder */
		if (no_selection)
			Picker_SetSelectedPick(hwndList, 0);
		else
			Picker_SetSelectedItem(hwndList, current_game);
	}

	/*RS Instead of the Arrange Call that was here previously on all Views
		 We now need to set the ViewMode for SmallIcon again,
		 for an explanation why, see SetView*/
	if (GetViewMode() == VIEW_SMALL_ICONS)
		SetView(ID_VIEW_SMALL_ICON);

	SetWindowRedraw(hwndList, TRUE);

	UpdateStatusBar();

}

static int MMO2LST(void)
{
	int i;
	FILE *file;
	char filename[MAX_PATH];
	*filename = 0;

	sprintf(filename, MAME32NAME "%s", ui_lang_info[options.langcode].shortname);
	strcpy(filename, strlower(filename));

	if (CommonFileDialog(TRUE, filename, FILETYPE_LST_FILES))
	{
		file = fopen(filename, "wt");
		if (file == NULL)
		{
			//fprintf(stderr, "error: create file %s\n", filename);
			fclose(file);
			return 2;
		}

	    for (i = 0; drivers[i]; i++)
	    {
		    const char *lst = _LST(drivers[i]->description);
		    const char *readings = _READINGS(drivers[i]->description);
    
		    if (readings == drivers[i]->description)
			    readings = lst;
    
		    fprintf(file, "%s\t%s\t%s\t%s\n",
			    drivers[i]->name, lst, readings, drivers[i]->manufacturer);
	    }
    
	    fclose(file);
	}
	return 0;
}

static void UpdateGameList()
{
	int i;

	for (i = 0; i < game_count; i++)
	{
		SetRomAuditResults(i, UNKNOWN);
		SetSampleAuditResults(i, UNKNOWN);
	}

	idle_work    = TRUE;
	bDoGameCheck = TRUE;
	game_index   = 0;

	ReloadIcons();

	// Let REFRESH also load new background if found
	LoadBackgroundBitmap();
	InvalidateRect(hMain,NULL,TRUE);
	Picker_ResetIdle(hwndList);
}

UINT_PTR CALLBACK CFHookProc(
	HWND hdlg,      // handle to dialog box
	UINT uiMsg,     // message identifier
	WPARAM wParam,  // message parameter
	LPARAM lParam   // message parameter
)
{
	int iIndex, i;
	COLORREF cCombo, cList;
	switch (uiMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessageA(hdlg, cmb4, CB_ADDSTRING, 0, (LPARAM)(const void *)_UI("Custom"));
			iIndex = SendDlgItemMessage(hdlg, cmb4, CB_GETCOUNT, 0, 0);
			cList = GetListFontColor();
			SendDlgItemMessage(hdlg, cmb4, CB_SETITEMDATA,(WPARAM)iIndex-1,(LPARAM)cList );
			for( i = 0; i< iIndex; i++)
			{
				cCombo = SendDlgItemMessage(hdlg, cmb4, CB_GETITEMDATA,(WPARAM)i,0 );
				if( cList == cCombo)
				{
					SendDlgItemMessage(hdlg, cmb4, CB_SETCURSEL,(WPARAM)i,0 );
					break;
				}
			}
			break;
		case WM_COMMAND:
			if( LOWORD(wParam) == cmb4)
			{
				switch (HIWORD(wParam))
				{
					case CBN_SELCHANGE:  // The color ComboBox changed selection
						iIndex = (int)SendDlgItemMessage(hdlg, cmb4,
													  CB_GETCURSEL, 0, 0L);
						if( iIndex == SendDlgItemMessage(hdlg, cmb4, CB_GETCOUNT, 0, 0)-1)
						{
							//Custom color selected
 							cList = GetListFontColor();
 							PickColor(&cList);
							SendDlgItemMessage(hdlg, cmb4, CB_DELETESTRING, iIndex, 0);
							SendDlgItemMessageA(hdlg, cmb4, CB_ADDSTRING, 0, (LPARAM)(const void *)_UI("Custom"));
							SendDlgItemMessage(hdlg, cmb4, CB_SETITEMDATA,(WPARAM)iIndex,(LPARAM)cList);
							SendDlgItemMessage(hdlg, cmb4, CB_SETCURSEL,(WPARAM)iIndex,0 );
							return TRUE;
						}
				}
			}
			break;
	}
	return FALSE;
}

static void PickFont(void)
{
	LOGFONTA font;
	CHOOSEFONTA cf;
	HWND hWnd;

	GetListFont(&font);
	font.lfQuality = DEFAULT_QUALITY;

	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner   = hMain;
	cf.lpLogFont   = &font;
	cf.lpfnHook = &CFHookProc;
	cf.rgbColors   = GetListFontColor();
	cf.Flags	   = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_ENABLEHOOK;
	if (!ChooseFontA(&cf))
		return;

	SetListFont(&font);
	if (hFont != NULL)
		DeleteObject(hFont);
	hFont = TranslateCreateFont(&font);
	if (hFont != NULL)
	{
		COLORREF textColor = cf.rgbColors;

		if (textColor == RGB(255,255,255))
		{
			textColor = RGB(240, 240, 240);
		}

		SetAllWindowsFont(hMain, &main_resize, hFont, TRUE);

		hWnd = GetWindow(hMain, GW_CHILD);
		while(hWnd)
		{
			char szClass[128];
			if (GetClassNameA(hWnd, szClass, sizeof(szClass) / sizeof(szClass[0])))
			{
				if (!strcmp(szClass, "SysListView32"))
				{
					ListView_SetTextColor(hWnd, textColor);
				}
				else if (!strcmp(szClass, "SysTreeView32"))
				{
					TreeView_SetTextColor(hTreeView, textColor);
				}
			}
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}
		SetListFontColor(cf.rgbColors);
		ResetListView();
	}
}

static void PickColor(COLORREF *cDefault)
{
	CHOOSECOLOR cc;
	COLORREF choice_colors[16];
	int i;
	
	for (i=0;i<16;i++)
		choice_colors[i] = GetCustomColor(i);
 
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner   = hMain;
	cc.rgbResult   = *cDefault;
	cc.lpCustColors = choice_colors;
	cc.Flags       = CC_ANYCOLOR | CC_RGBINIT | CC_SOLIDCOLOR;
	if (!ChooseColor(&cc))
		return;
	for (i=0;i<16;i++)
		SetCustomColor(i,choice_colors[i]);
	*cDefault = cc.rgbResult;
}

static void PickCloneColor(void)
{
	COLORREF cClonecolor;
	cClonecolor = GetListCloneColor();
	PickColor( &cClonecolor);
	SetListCloneColor(cClonecolor);
	InvalidateRect(hwndList,NULL,FALSE);
}

static BOOL MameCommand(HWND hwnd,int id, HWND hwndCtl, UINT codeNotify)
{
	int i;

	if ((id >= ID_LANGUAGE_ENGLISH_US) && (id < ID_LANGUAGE_ENGLISH_US + UI_LANG_MAX))
	{
		ChangeLanguage(id);
		return TRUE;
	}

#ifdef USE_IPS
	if ((id >= ID_PLAY_PATCH) && (id < ID_PLAY_PATCH + MAX_PATCHES))
	{
		int  nGame = Picker_GetSelectedItem(hwndList);
		char patch_name[MAX_PATCHNAME];

		if (GetPatchName(patch_name, drivers[nGame]->name, id-ID_PLAY_PATCH))
		{
			options_type* pOpts = GetGameOptions(nGame);
			static char new_opt[MAX_PATCHNAME * MAX_PATCHES];

			new_opt[0] = '\0';

			if (pOpts->ips)
			{
				char *temp = mame_strdup(pOpts->ips);
				char *token = NULL;

				if (temp)
					token = strtok(temp, ",");

				while (token)
				{
					if (!strcmp(patch_name, token))
					{
						dprintf("dup!");
						patch_name[0] = '\0';
					}
					else
					{
						if (new_opt[0] != '\0')
							strcat(new_opt, ",");
						strcat(new_opt, token);
					}

					token = strtok(NULL, ",");
				}

				free(temp);
			}

			if (patch_name[0] != '\0')
			{
				if (new_opt[0] != '\0')
					strcat(new_opt, ",");
				strcat(new_opt, patch_name);
			}

			FreeIfAllocated(&pOpts->ips);
			if (new_opt[0] != '\0')
				pOpts->ips = mame_strdup(new_opt);

			dprintf("%s / %s | %d", patch_name, pOpts->ips, strlen(new_opt));
			SaveGameOptions(nGame);
		}
		return TRUE;
	}
	else if (g_IPSMenuSelectName && id != IDC_HISTORY)
	{
		FreeIfAllocated(&g_IPSMenuSelectName);
		UpdateScreenShot();
	}
#endif /* USE_IPS */

	switch (id)
	{
	case ID_FILE_PLAY:
		MamePlayGame();
		return TRUE;

	case ID_FILE_PLAY_RECORD:
		MamePlayRecordGame();
		return TRUE;

	case ID_FILE_PLAY_BACK:
		MamePlayBackGame(NULL);
		return TRUE;

	case ID_FILE_PLAY_RECORD_WAVE:
		MamePlayRecordWave();
		return TRUE;

	case ID_FILE_PLAY_RECORD_MNG:
		MamePlayRecordMNG();
		return TRUE;

	case ID_FILE_LOADSTATE :
		MameLoadState();
		return TRUE;

	case ID_FILE_AUDIT:
		AuditDialog(hMain);
		ResetWhichGamesInFolders();
		ResetListView();
		SetFocus(hwndList);
		return TRUE;

	case ID_FILE_EXIT:
		PostMessage(hMain, WM_CLOSE, 0, 0);
		return TRUE;

	case ID_VIEW_LARGE_ICON:
		SetView(ID_VIEW_LARGE_ICON);
		return TRUE;

	case ID_VIEW_SMALL_ICON:
		SetView(ID_VIEW_SMALL_ICON);
		ResetListView();
		return TRUE;

	case ID_VIEW_LIST_MENU:
		SetView(ID_VIEW_LIST_MENU);
		return TRUE;

	case ID_VIEW_DETAIL:
		SetView(ID_VIEW_DETAIL);
		return TRUE;

	case ID_VIEW_GROUPED:
		SetView(ID_VIEW_GROUPED);
		return TRUE;

	/* Arrange Icons submenu */
	case ID_VIEW_BYGAME:
	case ID_VIEW_BYROMS:
	case ID_VIEW_BYSAMPLES:
	case ID_VIEW_BYDIRECTORY:
	case ID_VIEW_BYTYPE:
	case ID_VIEW_TRACKBALL:
	case ID_VIEW_BYTIMESPLAYED:
	case ID_VIEW_BYMANUFACTURER:
	case ID_VIEW_BYYEAR:
	case ID_VIEW_BYCLONE:
	case ID_VIEW_BYSRCDRIVERS:
	case ID_VIEW_BYPLAYTIME:
		SetSortReverse(FALSE);
		DoSortColumn(id - ID_VIEW_BYGAME);
		Picker_Sort(hwndList);
		break;

	case ID_VIEW_FOLDERS:
		bShowTree = !bShowTree;
		SetShowFolderList(bShowTree);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_FOLDERS, (bShowTree) ? MF_CHECKED : MF_UNCHECKED);
		UpdateScreenShot();
		break;

	case ID_VIEW_TOOLBARS:
		bShowToolBar = !bShowToolBar;
		SetShowToolBar(bShowToolBar);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_TOOLBARS, (bShowToolBar) ? MF_CHECKED : MF_UNCHECKED);
		ShowWindow(hToolBar, (bShowToolBar) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		break;

	case ID_VIEW_STATUS:
		bShowStatusBar = !bShowStatusBar;
		SetShowStatusBar(bShowStatusBar);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ToolBar_CheckButton(hToolBar, ID_VIEW_STATUS, (bShowStatusBar) ? MF_CHECKED : MF_UNCHECKED);
		ShowWindow(hStatusBar, (bShowStatusBar) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		break;

	case ID_VIEW_PAGETAB:
		bShowTabCtrl = !bShowTabCtrl;
		SetShowTabCtrl(bShowTabCtrl);
		ShowWindow(hTabCtrl, (bShowTabCtrl) ? SW_SHOW : SW_HIDE);
		ResizePickerControls(hMain);
		UpdateScreenShot();
		InvalidateRect(hMain,NULL,TRUE);
		break;

		/*
		  Switches to fullscreen mode. No check mark handeling 
		  for this item cause in fullscreen mode the menu won't
		  be visible anyways.
		*/    
	case ID_VIEW_FULLSCREEN:
		SwitchFullScreenMode();
		break;

	case IDC_USE_LIST:
		SetUseLangList(!UseLangList());
		ToolBar_CheckButton(hToolBar, IDC_USE_LIST, UseLangList() ^ (GetLangcode() == UI_LANG_EN_US) ? MF_CHECKED : MF_UNCHECKED);
		ResetListView();
		UpdateHistory();
		break;

	case ID_TOOLBAR_EDIT:
		{
			WCHAR buf[256];

			Edit_GetText(hwndCtl, buf, sizeof(buf));
			switch (codeNotify)
			{
			case EN_CHANGE:
				//put search routine here first, add a 200ms timer later.
				if ((!_wcsicmp(buf, _Unicode(_UI(SEARCH_PROMPT))) && !_wcsicmp(g_SearchText, _Unicode(""))) ||
				    (!_wcsicmp(g_SearchText, _Unicode(_UI(SEARCH_PROMPT))) && !_wcsicmp(buf, _Unicode(""))))
				{
					wcscpy(g_SearchText, buf);
				}
				else
				{
					wcscpy(g_SearchText, buf);
					ResetListView();
				}
				break;
			case EN_SETFOCUS:
				if (!_wcsicmp(buf, _Unicode(_UI(SEARCH_PROMPT))))
					SetWindowTextW(hwndCtl, _Unicode(""));
				break;
			case EN_KILLFOCUS:
				if (wcslen(buf) == 0)
					SetWindowTextW(hwndCtl, _Unicode(_UI(SEARCH_PROMPT)));
				break;
			}
		}
		break;

	case ID_GAME_AUDIT:
		InitGameAudit(0);
		if (!oldControl)
		{
			InitPropertyPageToPage(hInst, hwnd, Picker_GetSelectedItem(hwndList), GetSelectedPickItemIcon(), AUDIT_PAGE, NULL);
			SaveGameOptions(Picker_GetSelectedItem(hwndList));
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	/* ListView Context Menu */
	case ID_CONTEXT_ADD_CUSTOM:
	{
		int  nResult;

		nResult = DialogBoxParam(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_CUSTOM_FILE),
								 hMain,AddCustomFileDialogProc,Picker_GetSelectedItem(hwndList));
		SetFocus(hwndList);
		break;
	}

	case ID_CONTEXT_REMOVE_CUSTOM:
	{
		RemoveCurrentGameCustomFolder();
		break;
	}

	/* Tree Context Menu */
	case ID_CONTEXT_FILTERS:
		if (DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_FILTERS), hMain, FilterDialogProc) == TRUE)
			ResetListView();
		SetFocus(hwndList);
		return TRUE;

	// ScreenShot Context Menu
	// select current tab
	case ID_VIEW_TAB_SCREENSHOT :
	case ID_VIEW_TAB_FLYER :
	case ID_VIEW_TAB_CABINET :
	case ID_VIEW_TAB_MARQUEE :
	case ID_VIEW_TAB_TITLE :
	case ID_VIEW_TAB_CONTROL_PANEL :
	case ID_VIEW_TAB_HISTORY :
#ifdef STORY_DATAFILE
	case ID_VIEW_TAB_STORY :
		if ((id == ID_VIEW_TAB_HISTORY || id == ID_VIEW_TAB_STORY) && GetShowTab(TAB_HISTORY) == FALSE)
#else /* STORY_DATAFILE */
		if (id == ID_VIEW_TAB_HISTORY && GetShowTab(TAB_HISTORY) == FALSE)
#endif /* STORY_DATAFILE */
			break;

		TabView_SetCurrentTab(hTabCtrl, id - ID_VIEW_TAB_SCREENSHOT);
		UpdateScreenShot();
		TabView_UpdateSelection(hTabCtrl);
		break;

		// toggle tab's existence
	case ID_TOGGLE_TAB_SCREENSHOT :
	case ID_TOGGLE_TAB_FLYER :
	case ID_TOGGLE_TAB_CABINET :
	case ID_TOGGLE_TAB_MARQUEE :
	case ID_TOGGLE_TAB_TITLE :
	case ID_TOGGLE_TAB_CONTROL_PANEL :
	case ID_TOGGLE_TAB_HISTORY :
#ifdef STORY_DATAFILE
	case ID_TOGGLE_TAB_STORY :
#endif /* STORY_DATAFILE */
	{
		int toggle_flag = id - ID_TOGGLE_TAB_SCREENSHOT;

		if (AllowedToSetShowTab(toggle_flag,!GetShowTab(toggle_flag)) == FALSE)
		{
			// attempt to hide the last tab
			// should show error dialog? hide picture area? or ignore?
			break;
		}

		SetShowTab(toggle_flag,!GetShowTab(toggle_flag));

		TabView_Reset(hTabCtrl);

		if (TabView_GetCurrentTab(hTabCtrl) == toggle_flag && GetShowTab(toggle_flag) == FALSE)
		{
			// we're deleting the tab we're on, so go to the next one
			TabView_CalculateNextTab(hTabCtrl);
		}


		// Resize the controls in case we toggled to another history
		// mode (and the history control needs resizing).

		ResizePickerControls(hMain);
		UpdateScreenShot();

		TabView_UpdateSelection(hTabCtrl);

		break;
	}

	/* Header Context Menu */
	case ID_SORT_ASCENDING:
		SetSortReverse(FALSE);
		SetSortColumn(Picker_GetRealColumnFromViewColumn(hwndList, lastColumnClick));
		Picker_Sort(hwndList);
		break;

	case ID_SORT_DESCENDING:
		SetSortReverse(TRUE);
		SetSortColumn(Picker_GetRealColumnFromViewColumn(hwndList, lastColumnClick));
		Picker_Sort(hwndList);
		break;

	case ID_CUSTOMIZE_FIELDS:
		if (DialogBox(GetModuleHandle(NULL),
			MAKEINTRESOURCE(IDD_COLUMNS), hMain, ColumnDialogProc) == TRUE)
			ResetColumnDisplay(FALSE);
		SetFocus(hwndList);
		return TRUE;

	/* View Menu */
	case ID_VIEW_LINEUPICONS:
		if( codeNotify == FALSE)
			ResetListView();
		else
		{
			/*it was sent after a refresh (F5) was done, we only reset the View if "available" is the selected folder
			  as it doesn't affect the others*/
			LPTREEFOLDER folder = GetSelectedFolder();
			if( folder )
			{
				if (folder->m_nFolderId == FOLDER_AVAILABLE )
					ResetListView();

			}
		}
		break;

	//fixme: inconsistent w/ official
	case ID_GAME_PROPERTIES:
		if (!oldControl)
		{
			InitPropertyPage(hInst, hwnd, Picker_GetSelectedItem(hwndList), GetSelectedPickItemIcon(), NULL);
			SaveGameOptions(Picker_GetSelectedItem(hwndList));
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	case ID_FOLDER_SOURCEPROPERTIES:
		if (!oldControl)
		{
			LPTREEFOLDER lpFolder = GetSourceFolder(Picker_GetSelectedItem(hwndList));
			int icon_id = GetTreeViewIconIndex(lpFolder->m_nIconId);
			HICON hIcon = ImageList_GetIcon(NULL, icon_id, ILD_TRANSPARENT);
			const char *name = lpFolder->m_lpTitle;

			if (lpFolder->m_lpOriginalTitle)
				name = lpFolder->m_lpOriginalTitle;

			InitPropertyPage(hInst, hwnd, -2, hIcon, name);
			SaveFolderOptions(name);
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	//fixme: inconsistent w/ official
	case ID_FOLDER_PROPERTIES:
		if (!oldControl)
		{
			LPTREEFOLDER lpFolder = GetSelectedFolder();
			const char *name = lpFolder->m_lpTitle;

			if (lpFolder->m_lpOriginalTitle)
				name = lpFolder->m_lpOriginalTitle;

			InitPropertyPage(hInst, hwnd, -2, GetSelectedFolderIcon(), name);
			SaveFolderOptions(name);
		}
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;
	case ID_FOLDER_AUDIT:
		FolderCheck();
		/* Just in case the toggle MMX on/off */
		UpdateStatusBar();
		break;

	case ID_VIEW_PICTURE_AREA :
		ToggleScreenShot();
		break;

	case ID_UPDATE_GAMELIST:
		UpdateGameList();
		break;

	case ID_OPTIONS_MMO2LST:
		MMO2LST();
		break;

	case ID_OPTIONS_FONT:
		PickFont();
		UpdateHistory();
		return TRUE;

	case ID_OPTIONS_CLONE_COLOR:
		PickCloneColor();
		return TRUE;

	case ID_OPTIONS_DEFAULTS:
		/* Check the return value to see if changes were applied */
		if (!oldControl)
		{
			InitDefaultPropertyPage(hInst, hwnd);
			SaveDefaultOptions();
		}
		SetFocus(hwndList);
		return TRUE;

	case ID_OPTIONS_DIR:
		{
			int  nResult;
			BOOL bUpdateRoms;
			BOOL bUpdateSamples;

			nResult = DialogBox(GetModuleHandle(NULL),
			                    MAKEINTRESOURCE(IDD_DIRECTORIES),
			                    hMain,
			                    DirectoriesDialogProc);

			SaveDefaultOptions();

			bUpdateRoms    = ((nResult & DIRDLG_ROMS)	 == DIRDLG_ROMS)	? TRUE : FALSE;
			bUpdateSamples = ((nResult & DIRDLG_SAMPLES) == DIRDLG_SAMPLES) ? TRUE : FALSE;

			/* update game list */
			if (bUpdateRoms == TRUE || bUpdateSamples == TRUE)
				UpdateGameList();

			SetFocus(hwndList);
		}
		return TRUE;

	case ID_OPTIONS_RESET_DEFAULTS:
		if (DialogBox(GetModuleHandle(NULL),
		              MAKEINTRESOURCE(IDD_RESET), hMain, ResetDialogProc) == TRUE)
		{
			// these may have been changed
			SaveDefaultOptions();
			DestroyWindow(hwnd);
			PostQuitMessage(0);
		}
		return TRUE;

	case ID_OPTIONS_INTERFACE:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INTERFACE_OPTIONS),
		          hMain, InterfaceDialogProc);
		SaveDefaultOptions();

		KillTimer(hMain, SCREENSHOT_TIMER);
		if( GetCycleScreenshot() > 0)
		{
			SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL ); // Scale to seconds
		}

		return TRUE;

#ifdef USE_VIEW_PCBINFO
	case ID_VIEW_PCBINFO:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PCBINFO),
				  hMain, PCBInfoDialogProc);
		SetFocus(hwndList);
		return TRUE;
#endif /* USE_VIEW_PCBINFO */

	case ID_OPTIONS_BG:
		{
			char filename[MAX_PATH];
			*filename = 0;

			if (CommonFileDialog(FALSE, filename, FILETYPE_IMAGE_FILES))
			{
				ResetBackground(filename);
				LoadBackgroundBitmap();
				InvalidateRect(hMain, NULL, TRUE);
				return TRUE;
			}
		}
		break;

#ifdef UI_COLOR_DISPLAY
	case ID_OPTIONS_PALETTE:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PALETTE),
				  hMain, PaletteDialogProc);
		return TRUE;
#endif /* UI_COLOR_DISPLAY */

	case ID_HELP_ABOUT:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT),
				  hMain, AboutDialogProc);
		SetFocus(hwndList);
		return TRUE;

	case IDOK :
		/* cmk -- might need to check more codes here, not sure */
		if (codeNotify != EN_CHANGE && codeNotify != EN_UPDATE)
		{
			/* enter key */
			if (g_in_treeview_edit)
			{
				TreeView_EndEditLabelNow(hTreeView, FALSE);
				return TRUE;
			}
			else 
				if (have_selection)
					MamePlayGame();
		}
		break;

	case IDCANCEL : /* esc key */
		if (g_in_treeview_edit)
			TreeView_EndEditLabelNow(hTreeView, TRUE);
		break;

	case IDC_PLAY_GAME :
		if (have_selection)
			MamePlayGame();
		break;

	case ID_UI_START:
		SetFocus(hwndList);
		MamePlayGame();
		break;

	case ID_UI_UP:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() - 1);
		break;

	case ID_UI_DOWN:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() + 1);
		break;

	case ID_UI_PGUP:
		Picker_SetSelectedPick(hwndList, GetSelectedPick() - ListView_GetCountPerPage(hwndList));
		break;

	case ID_UI_PGDOWN:
		if ( (GetSelectedPick() + ListView_GetCountPerPage(hwndList)) < ListView_GetItemCount(hwndList) )
			Picker_SetSelectedPick(hwndList,  GetSelectedPick() + ListView_GetCountPerPage(hwndList) );
		else
			Picker_SetSelectedPick(hwndList,  ListView_GetItemCount(hwndList)-1 );
		break;

	case ID_UI_HOME:
		Picker_SetSelectedPick(hwndList, 0);
		break;

	case ID_UI_END:
		Picker_SetSelectedPick(hwndList,  ListView_GetItemCount(hwndList)-1 );
		break;
	case ID_UI_LEFT:
		/* hmmmmm..... */
		SendMessage(hwndList,WM_HSCROLL, SB_LINELEFT, 0);
		break;

	case ID_UI_RIGHT:
		/* hmmmmm..... */
		SendMessage(hwndList,WM_HSCROLL, SB_LINERIGHT, 0);
		break;
	case ID_UI_HISTORY_UP:
		/* hmmmmm..... */
		{
			HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);
			SendMessage(hHistory, EM_SCROLL, SB_PAGEUP, 0);
		}
		break;

	case ID_UI_HISTORY_DOWN:
		/* hmmmmm..... */
		{
			HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);
			SendMessage(hHistory, EM_SCROLL, SB_PAGEDOWN, 0);
		}
		break;

	case IDC_SSFRAME:
		TabView_CalculateNextTab(hTabCtrl);
		UpdateScreenShot();
		TabView_UpdateSelection(hTabCtrl);
		break;

	case ID_CONTEXT_SELECT_RANDOM:
		SetRandomPickItem();
		break;

	case ID_CONTEXT_RESET_PLAYTIME:
		ResetPlayTime( Picker_GetSelectedItem(hwndList) );
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		break;

	case ID_CONTEXT_RESET_PLAYCOUNT:
		ResetPlayCount( Picker_GetSelectedItem(hwndList) );
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
		break;

	case ID_CONTEXT_RENAME_CUSTOM :
		TreeView_EditLabel(hTreeView,TreeView_GetSelection(hTreeView));
		break;

	default:
		if (id >= ID_CONTEXT_SHOW_FOLDER_START && id < ID_CONTEXT_SHOW_FOLDER_END)
		{
			ToggleShowFolder(id - ID_CONTEXT_SHOW_FOLDER_START);
			break;
		}
		for (i = 0; g_helpInfo[i].nMenuItem > 0; i++)
		{
			if (g_helpInfo[i].nMenuItem == id)
			{
				if (g_helpInfo[i].bIsHtmlHelp)
					HelpFunction(hMain, g_helpInfo[i].lpFile, HH_DISPLAY_TOPIC, 0);
				else
					DisplayTextFile(hMain, g_helpInfo[i].lpFile);
				return FALSE;
			}
		}
		break;
	}

	return FALSE;
}

static void LoadBackgroundBitmap()
{
	HGLOBAL hDIBbg;
	char*	pFileName = 0;

	if (hBackground)
	{
		DeleteObject(hBackground);
		hBackground = 0;
	}

	if (hPALbg)
	{
		DeleteObject(hPALbg);
		hPALbg = 0;
	}

	/* Pick images based on number of colors avaliable. */
	if (GetDepth(hwndList) <= 8)
	{
		pFileName = (char *)"bkgnd16";
		/*nResource = IDB_BKGROUND16;*/
	}
	else
	{
		pFileName = (char *)"bkground";
		/*nResource = IDB_BKGROUND;*/
	}

	if (LoadDIB(pFileName, &hDIBbg, &hPALbg, BACKGROUND))
	{
		HDC hDC = GetDC(hwndList);
		hBackground = DIBToDDB(hDC, hDIBbg, &bmDesc);
		GlobalFree(hDIBbg);
		ReleaseDC(hwndList, hDC);
	}
}

static void ResetColumnDisplay(BOOL first_time)
{
	int driver_index;

	if (!first_time)
		Picker_ResetColumnDisplay(hwndList);

	ResetListView();

	driver_index = GetGameNameIndex(GetDefaultGame());
	Picker_SetSelectedItem(hwndList, driver_index);
}

int GamePicker_GetItemImage(HWND hwndPicker, int nItem)
{
	return GetIconForDriver(nItem);
}

int GamePicker_GetUseBrokenColor(void)
{
	return !GetUseBrokenIcon();
}

const char *GamePicker_GetItemString(HWND hwndPicker, int nItem, int nColumn)
{
	const char *s = "";

	switch(nColumn)
	{
		case COLUMN_GAMES:
			/* Driver description */
			s = UseLangList() ?
				_LST(drivers[nItem]->description):
				ModifyThe(drivers[nItem]->description);
			break;

		case COLUMN_ROMS:
			/* Has Roms */
			s = GetAuditString(GetRomAuditResults(nItem));
			break;

		case COLUMN_SAMPLES:
			/* Samples */
			if (DriverUsesSamples(nItem))
				s = GetAuditString(GetSampleAuditResults(nItem));
			break;

		case COLUMN_DIRECTORY:
			/* Driver name (directory) */
			s = drivers[nItem]->name;
			break;

		case COLUMN_SRCDRIVERS:
			/* Source drivers */
			s = GetDriverFilename(nItem);
			break;

		case COLUMN_PLAYTIME:
			/* total play time */
			{
				static char buf[100];

				GetTextPlayTime(nItem, buf);
				s = buf;
			}
			break;

		case COLUMN_TYPE:
		{
			machine_config drv;
			expand_machine_driver(drivers[nItem]->drv,&drv);

			/* Vector/Raster */
			if (drv.video_attributes & VIDEO_TYPE_VECTOR)
				s = _UI("Vector");
			else
				s = _UI("Raster");
			break;
		}
		case COLUMN_TRACKBALL:
			/* Trackball */
			if (DriverUsesTrackball(nItem))
				s = _UI("Yes");
			else
				s = _UI("No");
			break;

		case COLUMN_PLAYED:
			/* times played */
			{
				static char buf[100];

				sprintf(buf, "%i", GetPlayCount(nItem));
				s = buf;
			}
			break;

		case COLUMN_MANUFACTURER:
			/* Manufacturer */
			if (UseLangList())
				s = _MANUFACT(drivers[nItem]->manufacturer);
			else
				s = (char *)drivers[nItem]->manufacturer;
			break;

		case COLUMN_YEAR:
			/* Year */
			s = drivers[nItem]->year;
			break;

		case COLUMN_CLONE:
			s = GetCloneParentName(nItem);
			break;
	}
	return s;
}

static void GamePicker_LeavingItem(HWND hwndPicker, int nItem)
{
	// leaving item
	// printf("leaving %s\n",drivers[nItem]->name);
#ifdef MESS
	MessWriteMountedSoftware(nItem);
#endif	
}

static void GamePicker_EnteringItem(HWND hwndPicker, int nItem)
{
	// printf("entering %s\n",drivers[nItem]->name);
	if (g_bDoBroadcast == TRUE)
	{
		ATOM a = GlobalAddAtom(_Unicode(drivers[nItem]->description));
		SendMessage(HWND_BROADCAST, g_mame32_message, a, a);
		GlobalDeleteAtom(a);
	}

	EnableSelection(nItem);
#ifdef MESS
	MessReadMountedSoftware(nItem);
#endif
}

static int GamePicker_FindItemParent(HWND hwndPicker, int nItem)
{
	return DriverParentIndex(nItem);
}

static int GamePicker_CheckItemBroken(HWND hwndPicker, int nItem)
{
	return DriverIsBroken(nItem);
}

/* Initialize the Picker and List controls */
static void InitListView()
{
	LVBKIMAGEA bki;
	char path[MAX_PATH];

	static const struct PickerCallbacks s_gameListCallbacks =
	{
		DoSortColumn,			/* pfnSetSortColumn */
		GetSortColumn,			/* pfnGetSortColumn */
		SetSortReverse,			/* pfnSetSortReverse */
		GetSortReverse,			/* pfnGetSortReverse */
		SetViewMode,			/* pfnSetViewMode */
		GetViewMode,			/* pfnGetViewMode */
		SetColumnWidths,		/* pfnSetColumnWidths */
		GetColumnWidths,		/* pfnGetColumnWidths */
		SetColumnOrder,			/* pfnSetColumnOrder */
		GetColumnOrder,			/* pfnGetColumnOrder */
		SetColumnShown,			/* pfnSetColumnShown */
		GetColumnShown,			/* pfnGetColumnShown */
		GetOffsetClones,		/* pfnGetOffsetChildren */
		GamePicker_GetUseBrokenColor,	/* pfnGetUseBrokenColor */

		GamePicker_Compare,		/* pfnCompare */
		MamePlayGame,			/* pfnDoubleClick */
		GamePicker_GetItemString,	/* pfnGetItemString */
		GamePicker_GetItemImage,	/* pfnGetItemImage */
		GamePicker_LeavingItem,		/* pfnLeavingItem */
		GamePicker_EnteringItem,	/* pfnEnteringItem */
		BeginListViewDrag,		/* pfnBeginListViewDrag */
		GamePicker_FindItemParent,	/* pfnFindItemParent */
		GamePicker_CheckItemBroken,	/* pfnCheckItemBroken */
		OnIdle,				/* pfnIdle */
		GamePicker_OnHeaderContextMenu,	/* pfnOnHeaderContextMenu */
		GamePicker_OnBodyContextMenu	/* pfnOnBodyContextMenu */
	};

	struct PickerOptions opts;

	// subclass the list view
	memset(&opts, 0, sizeof(opts));
	opts.pCallbacks = &s_gameListCallbacks;
	opts.nColumnCount = COLUMN_MAX;
	opts.ppszColumnNames = column_names;
	SetupPicker(hwndList, &opts);

	ListView_SetTextBkColor(hwndList, CLR_NONE);
	ListView_SetBkColor(hwndList, CLR_NONE);
	sprintf(path, "%s\\bkground.png", GetBgDir() );
	bki.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
	bki.pszImage = path;
	if( hBackground )	
		ListView_SetBkImageA(hwndList, &bki);

	CreateIcons();

	ResetColumnDisplay(TRUE);

	// Allow selection to change the default saved game
	bListReady = TRUE;
}

static void AddDriverIcon(int nItem,int default_icon_index)
{
	HICON hIcon = 0;
	const game_driver *clone_of = driver_get_clone(drivers[nItem]);

	/* if already set to rom or clone icon, we've been here before */
	if (icon_index[nItem] == 1 || icon_index[nItem] == 3)
		return;

	hIcon = LoadIconFromFile((char *)drivers[nItem]->name);
	if (hIcon == NULL && clone_of != NULL)
	{
		hIcon = LoadIconFromFile((char *)drivers[nItem]->parent);
		if (hIcon == NULL && driver_get_clone(clone_of) != NULL)
			hIcon = LoadIconFromFile((char *)clone_of->parent);
	}

	if (hIcon != NULL)
	{
		int nIconPos = ImageList_AddIcon(hSmall, hIcon);
		ImageList_AddIcon(hLarge, hIcon);
		if (nIconPos != -1)
			icon_index[nItem] = nIconPos;
	}
	if (icon_index[nItem] == 0)
		icon_index[nItem] = default_icon_index;
}

static void DestroyIcons(void)
{
	if (hSmall != NULL)
	{
		//FIXME: ImageList_Destroy(hSmall);
		hSmall = NULL;
	}

	if (icon_index != NULL)
	{
		int i;
		for (i=0;i<game_count;i++)
			icon_index[i] = 0; // these are indices into hSmall
	}

	if (hLarge != NULL)
	{
		//FIXME: ImageList_Destroy(hLarge);
		hLarge = NULL;
	}

	if (hHeaderImages != NULL)
	{
		//FIXME: ImageList_Destroy(hHeaderImages);
		hHeaderImages = NULL;
	}

}

static void ReloadIcons(void)
{
	HICON hIcon;
	INT i;

	// clear out all the images
	ImageList_Remove(hSmall,-1);
	ImageList_Remove(hLarge,-1);

	if (icon_index != NULL)
	{
		for (i=0;i<game_count;i++)
			icon_index[i] = 0; // these are indices into hSmall
	}

	for (i = 0; g_iconData[i].icon_name; i++)
	{
		hIcon = LoadIconFromFile((char *) g_iconData[i].icon_name);
		if (hIcon == NULL)
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(g_iconData[i].resource));

		ImageList_AddIcon(hSmall, hIcon);
		ImageList_AddIcon(hLarge, hIcon);
	}
}

static DWORD GetShellLargeIconSize(void)
{
	DWORD  dwSize, dwLength = 512, dwType = REG_SZ;
	char   szBuffer[512];
	HKEY   hKey;

	/* Get the Key */
	RegOpenKeyA(HKEY_CURRENT_USER, "Control Panel\\desktop\\WindowMetrics", &hKey);
	/* Save the last size */
	RegQueryValueExA(hKey, "Shell Icon Size", NULL, &dwType, (LPBYTE)szBuffer, &dwLength);
	dwSize = atol(szBuffer);
	if (dwSize < 32)
		dwSize = 32;

	if (dwSize > 48)
		dwSize = 48;

	/* Clean up */
	RegCloseKey(hKey);
	return dwSize;
}

static DWORD GetShellSmallIconSize(void)
{
	DWORD dwSize = ICONMAP_WIDTH;

	if (dwSize < 48)
	{
		if (dwSize < 32)
			dwSize = 16;
		else
			dwSize = 32;
	}
	else
	{
		dwSize = 48;
	}
	return dwSize;
}

// create iconlist for Listview control
static void CreateIcons(void)
{
	DWORD dwLargeIconSize = GetShellLargeIconSize();
	HICON hIcon;
	int icon_count;
	DWORD dwStyle;

	icon_count = 0;
	while(g_iconData[icon_count].icon_name)
		icon_count++;

	// the current window style affects the sizing of the rows when changing
	// between list views, so put it in small icon mode temporarily while we associate
	// our image list
	//
	// using large icon mode instead kills the horizontal scrollbar when doing
	// full refresh, which seems odd (it should recreate the scrollbar when
	// set back to report mode, for example, but it doesn't).

	dwStyle = GetWindowLong(hwndList,GWL_STYLE);
	SetWindowLong(hwndList,GWL_STYLE,(dwStyle & ~LVS_TYPEMASK) | LVS_ICON);

	hSmall = ImageList_Create(GetShellSmallIconSize(),GetShellSmallIconSize(),
	                          ILC_COLORDDB | ILC_MASK, icon_count, 500);
	hLarge = ImageList_Create(dwLargeIconSize, dwLargeIconSize,
	                          ILC_COLORDDB | ILC_MASK, icon_count, 500);

	ReloadIcons();

	// Associate the image lists with the list view control.
	ListView_SetImageList(hwndList, hSmall, LVSIL_SMALL);
	ListView_SetImageList(hwndList, hLarge, LVSIL_NORMAL);

	// restore our view
	SetWindowLong(hwndList,GWL_STYLE,dwStyle);

	// Now set up header specific stuff
	hHeaderImages = ImageList_Create(16,16,ILC_COLORDDB | ILC_MASK,2,2);
	hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HEADER_UP));
	ImageList_AddIcon(hHeaderImages,hIcon);
	hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_HEADER_DOWN));
	ImageList_AddIcon(hHeaderImages,hIcon);

	Picker_SetHeaderImageList(hwndList, hHeaderImages);
}



static int GamePicker_Compare(HWND hwndPicker, int index1, int index2, int sort_subitem)
{
	int value;
	const char *name1 = NULL;
	const char *name2 = NULL;
	int nTemp1, nTemp2;

	machine_config drv1;
	machine_config drv2;
	expand_machine_driver(drivers[index1]->drv, &drv1);
	expand_machine_driver(drivers[index2]->drv, &drv2);

#ifdef DEBUG
	if (strcmp(drivers[index1]->name,"1941") == 0 && strcmp(drivers[index2]->name,"1942") == 0)
	{
		dprintf("comparing 1941, 1942");
	}

	if (strcmp(drivers[index1]->name,"1942") == 0 && strcmp(drivers[index2]->name,"1941") == 0)
	{
		dprintf("comparing 1942, 1941");
	}
#endif

	switch (sort_subitem)
	{
	case COLUMN_GAMES:
		value = 0;

		if (UseLangList())
			value = stricmp(game_readings[index1], game_readings[index2]);

		if (value == 0)
			value = stricmp(ModifyThe(drivers[index1]->description),
			                ModifyThe(drivers[index2]->description));
		break;

	case COLUMN_ROMS:
		nTemp1 = GetRomAuditResults(index1);
		nTemp2 = GetRomAuditResults(index2);

		if (IsAuditResultKnown(nTemp1) == FALSE && IsAuditResultKnown(nTemp2) == FALSE)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (IsAuditResultKnown(nTemp1) == FALSE)
		{
			value = 1;
			break;
		}

		if (IsAuditResultKnown(nTemp2) == FALSE)
		{
			value = -1;
			break;
		}

		// ok, both are known

		if (IsAuditResultYes(nTemp1) && IsAuditResultYes(nTemp2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);
		
		if (IsAuditResultNo(nTemp1) && IsAuditResultNo(nTemp2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (IsAuditResultYes(nTemp1) && IsAuditResultNo(nTemp2))
			value = -1;
		else
			value = 1;
		break;

	case COLUMN_SAMPLES:
		nTemp1 = -1;
		if (DriverUsesSamples(index1))
		{
			int audit_result = GetSampleAuditResults(index1);
			if (IsAuditResultKnown(audit_result))
			{
				if (IsAuditResultYes(audit_result))
					nTemp1 = 1;
				else 
					nTemp1 = 0;
			}
			else
				nTemp1 = 2;
		}

		nTemp2 = -1;
		if (DriverUsesSamples(index2))
		{
			int audit_result = GetSampleAuditResults(index2);
			if (IsAuditResultKnown(audit_result))
			{
				if (IsAuditResultYes(audit_result))
					nTemp2 = 1;
				else 
					nTemp2 = 0;
			}
			else
				nTemp2 = 2;
		}

		if (nTemp1 == nTemp2)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = nTemp2 - nTemp1;
		break;

	case COLUMN_DIRECTORY:
		value = stricmp(drivers[index1]->name, drivers[index2]->name);
		break;

	case COLUMN_SRCDRIVERS:
		if (stricmp(GetDriverFilename(index1), GetDriverFilename(index2)) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(GetDriverFilename(index1), GetDriverFilename(index2));
		break;

	case COLUMN_PLAYTIME:
	   value = GetPlayTime(index1) - GetPlayTime(index2);
	   if (value == 0)
		  return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

	   break;

	case COLUMN_TYPE:
	{
		if ((drv1.video_attributes & VIDEO_TYPE_VECTOR) ==
			(drv2.video_attributes & VIDEO_TYPE_VECTOR))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = (drv1.video_attributes & VIDEO_TYPE_VECTOR) -
				(drv2.video_attributes & VIDEO_TYPE_VECTOR);
		break;
	}
	case COLUMN_TRACKBALL:
		if (DriverUsesTrackball(index1) == DriverUsesTrackball(index2))
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = DriverUsesTrackball(index1) - DriverUsesTrackball(index2);
		break;

	case COLUMN_PLAYED:
		value = GetPlayCount(index1) - GetPlayCount(index2);
		if (value == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		break;

	case COLUMN_MANUFACTURER:
		if (stricmp(drivers[index1]->manufacturer, drivers[index2]->manufacturer) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(drivers[index1]->manufacturer, drivers[index2]->manufacturer);
		break;

	case COLUMN_YEAR:
		if (stricmp(drivers[index1]->year, drivers[index2]->year) == 0)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		value = stricmp(drivers[index1]->year, drivers[index2]->year);
		break;

	case COLUMN_CLONE:
		name1 = GetCloneParentName(index1);
		name2 = GetCloneParentName(index2);

		if (*name1 == '\0')
			name1 = NULL;
		if (*name2 == '\0')
			name2 = NULL;

		if (name1 == name2)
			return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);

		if (name2 == NULL)
			value = -1;
		else if (name1 == NULL)
			value = 1;
		else
			value = stricmp(name1, name2);
		break;

	default :
		return GamePicker_Compare(hwndPicker, index1, index2, COLUMN_GAMES);
	}

#ifdef DEBUG
	if ((strcmp(drivers[index1]->name,"1941") == 0 && strcmp(drivers[index2]->name,"1942") == 0) ||
		(strcmp(drivers[index1]->name,"1942") == 0 && strcmp(drivers[index2]->name,"1941") == 0))
		dprintf("result: %i",value);
#endif

	return value;
}

static int GetSelectedPick()
{
	/* returns index of listview selected item */
	/* This will return -1 if not found */
	return ListView_GetNextItem(hwndList, -1, LVIS_SELECTED | LVIS_FOCUSED);
}

static HICON GetSelectedPickItemIcon()
{
	LV_ITEM lvi;

	lvi.iItem = GetSelectedPick();
	lvi.iSubItem = 0;
	lvi.mask     = LVIF_IMAGE;
	ListView_GetItem(hwndList, &lvi);

	return ImageList_GetIcon(hLarge, lvi.iImage, ILD_TRANSPARENT);
}

static void SetRandomPickItem()
{
	int nListCount;

	nListCount = ListView_GetItemCount(hwndList);

	if (nListCount > 0)
	{
		Picker_SetSelectedPick(hwndList, rand() % nListCount);
	}
}

static const char *GetLastDir(void)
{
	return last_directory;
}

static BOOL CommonFileDialogW(BOOL open_for_write, char *filename, int filetype)
{
	BOOL success;

	OPENFILENAMEW of;
	common_file_dialog_procW cfd;
	WCHAR fn[MAX_PATH];
	WCHAR *p, buf[256];
	const char *s = NULL;
	WCHAR dir[256];
	WCHAR title[256];
	WCHAR ext[256];

	lstrcpy(fn, _Unicode(filename));

	of.lStructSize       = sizeof(of);
	of.hwndOwner         = hMain;
	of.hInstance         = NULL;

	of.lpstrInitialDir   = NULL;
	of.lpstrTitle        = NULL;
	of.lpstrDefExt       = NULL;
	of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (open_for_write)
	{
		cfd = GetSaveFileNameW;

		if (cfg_data[filetype].title_save)
		{
			lstrcpy(title, _Unicode(_UI(cfg_data[filetype].title_save)));
			of.lpstrTitle = title;
		}
	}
	else
	{
		cfd = GetOpenFileNameW;
		of.Flags |= OFN_FILEMUSTEXIST;

		if (cfg_data[filetype].title_load)
		{
			lstrcpy(title, _Unicode(_UI(cfg_data[filetype].title_load)));
			of.lpstrTitle = title;
		}
	}

	if (cfg_data[filetype].dir)
	{
		lstrcpy(dir, _Unicode(cfg_data[filetype].dir()));
		of.lpstrInitialDir = dir;
	}

	if (cfg_data[filetype].ext)
	{
		lstrcpy(ext, _Unicode(_UI(cfg_data[filetype].ext)));
		of.lpstrDefExt = ext;
	}

	s = cfg_data[filetype].filter;
	for (p = buf; *s; s += strlen(s) + 1)
	{
		lstrcpy(p, _Unicode(_UI(s)));
		p += lstrlen(p) + 1;
	}
	*p = '\0';

	of.lpstrFilter       = buf;
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 1;
	of.lpstrFile         = fn;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;

	success = cfd(&of);
	if (success)
	{
		strcpy(filename, _String(fn));
		//dprintf("got filename %s nFileExtension %u\n",filename,_String(of.nFileExtension));
		/*GetDirectory(filename,last_directory,sizeof(last_directory));*/
	}

	return success;
}

static BOOL CommonFileDialogA(BOOL open_for_write, char *filename, int filetype)
{
	BOOL success;

	OPENFILENAMEA of;
	common_file_dialog_procA cfd;
	char fn[MAX_PATH];
	char *p, buf[256];
	const char *s = NULL;
	char dir[256];
	char title[256];
	char ext[256];

	strcpy(fn, filename);

	of.lStructSize       = sizeof(of);
	of.hwndOwner         = hMain;
	of.hInstance         = NULL;

	of.lpstrInitialDir   = NULL;
	of.lpstrTitle        = NULL;
	of.lpstrDefExt       = NULL;
	of.Flags             = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (open_for_write)
	{
		cfd = GetSaveFileNameA;

		if (cfg_data[filetype].title_save)
		{
			strcpy(title, _UI(cfg_data[filetype].title_save));
			of.lpstrTitle = title;
		}
	}
	else
	{
		cfd = GetOpenFileNameA;
		of.Flags |= OFN_FILEMUSTEXIST;

		if (cfg_data[filetype].title_load)
		{
			strcpy(title, _UI(cfg_data[filetype].title_load));
			of.lpstrTitle = title;
		}
	}

	if (cfg_data[filetype].dir)
	{
		strcpy(dir, cfg_data[filetype].dir());
		of.lpstrInitialDir = dir;
	}

	if (cfg_data[filetype].ext)
	{
		strcpy(ext, _UI(cfg_data[filetype].ext));
		of.lpstrDefExt = ext;
	}

	s = cfg_data[filetype].filter;
	for (p = buf; *s; s += strlen(s) + 1)
	{
		strcpy(p, _UI(s));
		p += strlen(p) + 1;
	}
	*p = '\0';

	of.lpstrFilter       = buf;
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 1;
	of.lpstrFile         = fn;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;

	success = cfd(&of);
	if (success)
	{
		strcpy(filename, fn);
		//dprintf("got filename %s nFileExtension %u\n",filename,_String(of.nFileExtension));
		/*GetDirectory(filename,last_directory,sizeof(last_directory));*/
	}

	return success;
}

static BOOL CommonFileDialog(BOOL open_for_write, char *filename, int filetype)
{
	if (OnNT())
		return CommonFileDialogW(open_for_write, filename, filetype);
	else
		return CommonFileDialogA(open_for_write, filename, filetype);
}

void SetStatusBarText(int part_index, const char *message)
{
	StatusBarSetTextA(hStatusBar, part_index, message);
}

void SetStatusBarTextW(int part_index, LPWSTR message)
{
	StatusBarSetTextW(hStatusBar, part_index, message);
}

void SetStatusBarTextF(int part_index, const char *fmt, ...)
{
	char buf[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	SetStatusBarText(part_index, buf);
}

static void MameMessageBox(const char *fmt, ...)
{
	char buf[2048];
	va_list va;

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	MessageBox(GetMainWindow(), _Unicode(buf), TEXT_MAME32NAME, MB_OK | MB_ICONERROR);
	va_end(va);
}

static void MamePlayBackGame(const char *fname_playback)
{
	int nGame = -1;
	char filename[MAX_PATH];

	if (fname_playback)
	{
		strcpy(filename, fname_playback);
	}
	else
	{
		*filename = 0;

		nGame = Picker_GetSelectedItem(hwndList);
		if (nGame != -1)
			strcpy(filename, drivers[nGame]->name);

		if (!CommonFileDialog(FALSE, filename, FILETYPE_INPUT_FILES)) return;
	}
	
	if (*filename)
	{
		mame_file* pPlayBack;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char bare_fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		char path[MAX_PATH];
		char fname[MAX_PATH];

		_splitpath(filename, drive, dir, bare_fname, ext);

		sprintf(path,"%s%s",drive,dir);
		sprintf(fname,"%s",bare_fname); // don't add extension for zip support.
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

		SetCorePathList(FILETYPE_INPUTLOG, path);
		pPlayBack = mame_fopen(fname,NULL,FILETYPE_INPUTLOG,0);
		if (pPlayBack == NULL)
		{
			MameMessageBox(_UI("Could not open '%s' as a valid input file."), filename);
			return;
		}

		// check for game name embedded in .inp header
		if (pPlayBack)
		{
			inp_header inp_header;

			// read playback header
			mame_fread(pPlayBack, &inp_header, sizeof(inp_header));

			if (!isalnum(inp_header.name[0])) // If first byte is not alpha-numeric
				mame_fseek(pPlayBack, 0, SEEK_SET); // old .inp file - no header
			else
			{
				int i;
				for (i = 0; drivers[i] != 0; i++) // find game and play it
				{
					if (strcmp(drivers[i]->name, inp_header.name) == 0)
					{
						nGame = i;
						break;
					}
				}
			}
		}
		mame_fclose(pPlayBack);

		g_pPlayBkName = fname;
		override_playback_directory = path;
		MamePlayGameWithOptions(nGame);
		g_pPlayBkName = NULL;
		override_playback_directory = NULL;
	}
}

static void MameLoadState()
{
	int nGame;
	char filename[MAX_PATH];
	char selected_filename[MAX_PATH];

	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	if (nGame != -1)
	{
		strcpy(filename, drivers[nGame]->name);
		strcpy(selected_filename, drivers[nGame]->name);
	}
	if (CommonFileDialog(FALSE, filename, FILETYPE_SAVESTATE_FILES))
	{
		mame_file* pSaveState;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char ext[_MAX_EXT];

		char path[MAX_PATH];
		char fname[MAX_PATH];
		char bare_fname[_MAX_FNAME];
		char *state_fname;
		int rc;

		_splitpath(filename, drive, dir, bare_fname, ext);

		// parse path
		sprintf(path,"%s%s",drive,dir);
		sprintf(fname,"%s%s",bare_fname,ext);
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

#ifdef MESS
		{
			state_fname = filename;
			return;
		}
#else // !MESS
		{
			char *cPos=0;
			int  iPos=0;
			char romname[MAX_PATH];

			cPos = strchr(bare_fname, '-' );
			iPos = cPos ? cPos - bare_fname : strlen(bare_fname);
			strncpy(romname, bare_fname, iPos );
			romname[iPos] = '\0';
			if( strcmp(selected_filename,romname) != 0 )
			{
				MameMessageBox(_UI("'%s' is not a valid savestate file for game '%s'."), filename, selected_filename);
				return;
			}
			SetCorePathList(FILETYPE_STATE, path);
			state_fname = fname;
		}
#endif // MESS

		pSaveState = mame_fopen(NULL,state_fname,FILETYPE_STATE,0);
		if (pSaveState == NULL)
		{
			MameMessageBox(_UI("Could not open '%s' as a valid savestate file."), filename);
			return;
		}

		// call the MAME core function to check the save state file
		rc = state_save_check_file(pSaveState, selected_filename, TRUE, MameMessageBox);
		mame_fclose(pSaveState);
		if (rc)
			return;

#ifdef MESS
		g_pSaveStateName = state_fname;
#else
		{
			char *cPos;
			cPos = strrchr(bare_fname, '-' );
			cPos = cPos+1;
			if( strlen(cPos) >0)
			{
				g_pSaveStateName = cPos;
				override_savestate_directory = path;
			}
		}
#endif

		MamePlayGameWithOptions(nGame);
		g_pSaveStateName = NULL;
		override_savestate_directory = NULL;
	}
}

static void MamePlayRecordGame()
{
	int  nGame;
	char filename[MAX_PATH];
	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(TRUE, filename, FILETYPE_INPUT_FILES))
	{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char path[MAX_PATH];

		_splitpath(filename, drive, dir, fname, ext);

		sprintf(path,"%s%s",drive,dir);
		if (path[strlen(path)-1] == '\\')
			path[strlen(path)-1] = 0; // take off trailing back slash

		g_pRecordName = fname;
		override_playback_directory = path;
		MamePlayGameWithOptions(nGame);
		g_pRecordName = NULL;
		override_playback_directory = NULL;
	}
}

static void MamePlayGame()
{
	int nGame;

	nGame = Picker_GetSelectedItem(hwndList);

	g_pPlayBkName = NULL;
	g_pRecordName = NULL;

	MamePlayGameWithOptions(nGame);
}

static void MamePlayRecordWave()
{
	int  nGame;
	char filename[MAX_PATH];
	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(TRUE, filename, FILETYPE_WAVE_FILES))
	{
		g_pRecordWaveName = filename;
		MamePlayGameWithOptions(nGame);
		g_pRecordWaveName = NULL;
	}	
}

static void MamePlayRecordMNG()
{
	int  nGame;
	char filename[MAX_PATH];
	*filename = 0;

	nGame = Picker_GetSelectedItem(hwndList);
	strcpy(filename, drivers[nGame]->name);

	if (CommonFileDialog(TRUE, filename, FILETYPE_MNG_FILES))
	{
		g_pRecordMNGName = filename;
		MamePlayGameWithOptions(nGame);
		g_pRecordMNGName = NULL;
	}	
}

static void MamePlayGameWithOptions(int nGame)
{
	memcpy(&playing_game_options, GetGameOptions(nGame), sizeof(options_type));

	/* Deal with options that can be disabled. */
	EnablePlayOptions(nGame, &playing_game_options);

	if (g_pJoyGUI != NULL)
		KillTimer(hMain, JOYGUI_TIMER);
	if (GetCycleScreenshot() > 0)
		KillTimer(hMain, SCREENSHOT_TIMER);

	g_bAbortLoading = FALSE;

	in_emulation = TRUE;

	if (RunMAME(nGame) == 0)
	{
		IncrementPlayCount(nGame);
		ResetWhichGamesInFolders();
		ListView_RedrawItems(hwndList, GetSelectedPick(), GetSelectedPick());
	}
	else
	{
		ShowWindow(hMain, SW_SHOW);
	}

	in_emulation = FALSE;

	// re-sort if sorting on # of times played
	if (GetSortColumn() == COLUMN_PLAYED
	 || GetSortColumn() == COLUMN_PLAYTIME)
		Picker_Sort(hwndList);

	UpdateStatusBar();

	ShowWindow(hMain, SW_SHOW);
	SetFocus(hwndList);

	if (g_pJoyGUI != NULL)
		SetTimer(hMain, JOYGUI_TIMER, JOYGUI_MS, NULL);
	if (GetCycleScreenshot() > 0)
		SetTimer(hMain, SCREENSHOT_TIMER, GetCycleScreenshot()*1000, NULL); //scale to seconds
}

/* Toggle ScreenShot ON/OFF */
static void ToggleScreenShot(void)
{
	BOOL showScreenShot = GetShowScreenShot();

	SetShowScreenShot((showScreenShot) ? FALSE : TRUE);
	UpdateScreenShot();

	/* Redraw list view */
	if (hBackground != NULL && showScreenShot)
		InvalidateRect(hwndList, NULL, FALSE);
}

static void AdjustMetrics(void)
{
	HDC hDC;
	TEXTMETRIC tm;
	int xtraX, xtraY;
	AREA area;
	int  offX, offY;
	int  maxX, maxY;
	COLORREF textColor;
	HWND hWnd;

	/* WM_SETTINGCHANGE also */
	xtraX  = GetSystemMetrics(SM_CXFIXEDFRAME); /* Dialog frame width */
	xtraY  = GetSystemMetrics(SM_CYFIXEDFRAME); /* Dialog frame height */
	xtraY += GetSystemMetrics(SM_CYMENUSIZE);   /* Menu height */
	xtraY += GetSystemMetrics(SM_CYCAPTION);    /* Caption Height */
	maxX   = GetSystemMetrics(SM_CXSCREEN);     /* Screen Width */
	maxY   = GetSystemMetrics(SM_CYSCREEN);     /* Screen Height */

	hDC = GetDC(hMain);
	GetTextMetrics (hDC, &tm);

	/* Convert MIN Width/Height from Dialog Box Units to pixels. */
	MIN_WIDTH  = (int)((tm.tmAveCharWidth / 4.0) * DBU_MIN_WIDTH)  + xtraX;
	MIN_HEIGHT = (int)((tm.tmHeight       / 8.0) * DBU_MIN_HEIGHT) + xtraY;
	ReleaseDC(hMain, hDC);

	if ((textColor = GetListFontColor()) == RGB(255, 255, 255))
		textColor = RGB(240, 240, 240);

	hWnd = GetWindow(hMain, GW_CHILD);
	while(hWnd)
	{
		char szClass[128];

		if (GetClassNameA(hWnd, szClass, sizeof(szClass) / sizeof(szClass[0])))
		{
			if (!strcmp(szClass, "SysListView32"))
			{
				ListView_SetBkColor(hWnd, GetSysColor(COLOR_WINDOW));
				ListView_SetTextColor(hWnd, textColor);
			}
			else if (!strcmp(szClass, "SysTreeView32"))
			{
				TreeView_SetBkColor(hTreeView, GetSysColor(COLOR_WINDOW));
				TreeView_SetTextColor(hTreeView, textColor);
			}
		}
		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
	}

	GetWindowArea(&area);

	offX = area.x + area.width;
	offY = area.y + area.height;

	if (offX > maxX)
	{
		offX = maxX;
		area.x = (offX - area.width > 0) ? (offX - area.width) : 0;
	}
	if (offY > maxY)
	{
		offY = maxY;
		area.y = (offY - area.height > 0) ? (offY - area.height) : 0;
	}

	SetWindowArea(&area);
	SetWindowPos(hMain, 0, area.x, area.y, area.width, area.height, SWP_NOZORDER | SWP_SHOWWINDOW);
}


/* Adjust options - tune them to the currently selected game */
static void EnablePlayOptions(int nIndex, options_type *o)
{
	if (!DIJoystick.Available())
		o->joystick = FALSE;
}

static int FindIconIndex(int nIconResource)
{
	int i;
	for(i = 0; g_iconData[i].icon_name; i++)
	{
		if (g_iconData[i].resource == nIconResource)
			return i;
	}
	return -1;
}

static BOOL UseBrokenIcon(int type)
{
	//if ((GetViewMode() != VIEW_GROUPED) && (GetViewMode() != VIEW_DETAILS))
	//	return TRUE;
	if (type == 4 && !GetUseBrokenIcon())
		return FALSE;
	return TRUE;
}

static int GetIconForDriver(int nItem)
{
	int iconRoms;

	if (DriverUsesRoms(nItem))
	{
		int audit_result = GetRomAuditResults(nItem);
		if (IsAuditResultKnown(audit_result) == FALSE)
			return 2;

		if (IsAuditResultYes(audit_result))
			iconRoms = 1;
		else
			iconRoms = 0;
	}
	else
		iconRoms = 1;

	// iconRoms is now either 0 (no roms), 1 (roms), or 2 (unknown)

	/* these are indices into icon_names, which maps into our image list
	 * also must match IDI_WIN_NOROMS + iconRoms
	 */

	// Show Red-X if the ROMs are present and flagged as NOT WORKING
	if (iconRoms == 1 && DriverIsBroken(nItem))
		iconRoms = FindIconIndex(IDI_WIN_REDX);

	// show clone icon if we have roms and game is working
	if (iconRoms == 1 && DriverIsClone(nItem))
		iconRoms = FindIconIndex(IDI_WIN_CLONE);

	// if we have the roms, then look for a custom per-game icon to override
	if (iconRoms == 1 || iconRoms == 3 || !UseBrokenIcon(iconRoms))
	{
		if (icon_index[nItem] == 0)
			AddDriverIcon(nItem,iconRoms);
		iconRoms = icon_index[nItem];
	}

	return iconRoms;
}

static BOOL HandleTreeContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HMENU hTreeMenu;
	HMENU hMenu;
	TVHITTESTINFO hti;
	POINT pt;

	if ((HWND)wParam != GetDlgItem(hWnd, IDC_TREE))
		return FALSE;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	if (pt.x < 0 && pt.y < 0)
		GetCursorPos(&pt);

	/* select the item that was right clicked or shift-F10'ed */
	hti.pt = pt;
	ScreenToClient(hTreeView,&hti.pt);
	TreeView_HitTest(hTreeView,&hti);
	if ((hti.flags & TVHT_ONITEM) != 0)
		TreeView_SelectItem(hTreeView,hti.hItem);

	hTreeMenu = LoadMenu(hInst,MAKEINTRESOURCE(IDR_CONTEXT_TREE));

	InitTreeContextMenu(hTreeMenu);

	hMenu = GetSubMenu(hTreeMenu, 0);

	TranslateMenu(hMenu, ID_CONTEXT_RENAME_CUSTOM);
	UpdateMenu(hMenu);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hWnd,NULL);

	DestroyMenu(hTreeMenu);

	return TRUE;
}



static void GamePicker_OnBodyContextMenu(POINT pt)
{
	HMENU hMenuLoad;
	HMENU hMenu;
	
	TPMPARAMS tpmp;
	ZeroMemory(&tpmp,sizeof(tpmp));
	tpmp.cbSize = sizeof(tpmp);
	GetWindowRect(GetDlgItem(hMain, IDC_SSFRAME), &tpmp.rcExclude);

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_MENU));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_FILE_PLAY);

	UpdateMenu(hMenu);

#ifdef USE_IPS
	if (have_selection)
	{
		int  nGame = Picker_GetSelectedItem(hwndList);
		options_type* pOpts = GetGameOptions(nGame);
		int  patch_count = HasPatch(drivers[nGame]->name, "*");
		char patch_name[MAX_PATCHNAME];
		char buf[MAX_PATCHNAME * MAX_PATCHES];
		char *token;
		int  i;

		if (patch_count > MAX_PATCHES)
			patch_count = MAX_PATCHES;

		while (patch_count--)
		{
			if (GetPatchName(patch_name, drivers[nGame]->name, patch_count))
			{
				LPWSTR patch_desc = GetPatchDesc(drivers[nGame]->name, patch_name);

				if (patch_desc && patch_desc[0])
					snprintf(buf, ARRAY_LENGTH(buf), "   %s", strtok(_String(patch_desc), "\r\n"));
				else
					snprintf(buf, ARRAY_LENGTH(buf), "   %s", patch_name);

				InsertMenu(hMenu, 1, MF_BYPOSITION, ID_PLAY_PATCH + patch_count, _Unicode(ConvertAmpersandString(buf)));

				if (pOpts->ips != NULL)
				{
					strcpy(buf, pOpts->ips);
					token = strtok(buf, ",");
					for (i = 0; i < MAX_PATCHES && token; i++)
					{
						if (!strcmp(patch_name, token))
						{
							CheckMenuItem(hMenu,ID_PLAY_PATCH + patch_count, MF_BYCOMMAND | MF_CHECKED);
							break;
						}
						token = strtok(NULL, ",");
					}
				}
			}
		}
	}
#endif /* USE_IPS */

	dprintf("%d,%d,%d,%d", tpmp.rcExclude.left,tpmp.rcExclude.right,tpmp.rcExclude.top,tpmp.rcExclude.bottom);
	//the menu should not overlap SSFRAME
	TrackPopupMenuEx(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,hMain,&tpmp);

	DestroyMenu(hMenuLoad);
}



static BOOL HandleScreenShotContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenuLoad;
	HMENU hMenu;
	POINT pt;

	if ((HWND)wParam != GetDlgItem(hWnd, IDC_SSPICTURE) && (HWND)wParam != GetDlgItem(hWnd, IDC_SSFRAME))
		return FALSE;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	if (pt.x < 0 && pt.y < 0)
		GetCursorPos(&pt);

	hMenuLoad = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT_SCREENSHOT));
	hMenu = GetSubMenu(hMenuLoad, 0);
	TranslateMenu(hMenu, ID_VIEW_PAGETAB);

	UpdateMenu(hMenu);

	TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,pt.x,pt.y,0,hWnd,NULL);

	DestroyMenu(hMenuLoad);

	return TRUE;
}

static void UpdateMenu(HMENU hMenu)
{
	char			buf[200];
	MENUITEMINFO	mItem;
	int 			nGame = Picker_GetSelectedItem(hwndList);
	LPTREEFOLDER lpFolder = GetCurrentFolder();
	int i;

	if (have_selection)
	{
		snprintf(buf, sizeof(buf), _UI("&Play %s"),
		         ConvertAmpersandString(UseLangList() ?
		                                _LST(drivers[nGame]->description) :
		                                ModifyThe(drivers[nGame]->description)));

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mItem);

		snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("Propert&ies for %s"),
		         GetDriverFilename(nGame));

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_FOLDER_SOURCEPROPERTIES, FALSE, &mItem);

		EnableMenuItem(hMenu, ID_CONTEXT_SELECT_RANDOM, MF_ENABLED);
	}
	else
	{
		snprintf(buf, sizeof(buf), _UI("&Play %s"), "...");

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_FILE_PLAY, FALSE, &mItem);

		snprintf(buf, sizeof(buf) / sizeof(buf[0]), _UI("Propert&ies for %s"), "...");

		mItem.cbSize     = sizeof(mItem);
		mItem.fMask      = MIIM_TYPE;
		mItem.fType      = MFT_STRING;
		mItem.dwTypeData = _Unicode(buf);
		mItem.cch        = lstrlen(mItem.dwTypeData);

		SetMenuItemInfo(hMenu, ID_FOLDER_SOURCEPROPERTIES, FALSE, &mItem);

		EnableMenuItem(hMenu, ID_FILE_PLAY,             MF_GRAYED);
		EnableMenuItem(hMenu, ID_FILE_PLAY_RECORD,      MF_GRAYED);
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,       MF_GRAYED);
		EnableMenuItem(hMenu, ID_FOLDER_SOURCEPROPERTIES,     MF_GRAYED);
		EnableMenuItem(hMenu, ID_CONTEXT_SELECT_RANDOM, MF_GRAYED);
	}

	if (oldControl)
	{
		EnableMenuItem(hMenu, ID_CUSTOMIZE_FIELDS,  MF_GRAYED);
		EnableMenuItem(hMenu, ID_GAME_PROPERTIES,   MF_GRAYED);
		EnableMenuItem(hMenu, ID_FOLDER_SOURCEPROPERTIES, MF_GRAYED);
		EnableMenuItem(hMenu, ID_OPTIONS_DEFAULTS,  MF_GRAYED);
	}

	if (lpFolder->m_dwFlags & F_CUSTOM)
	{
		EnableMenuItem(hMenu,ID_CONTEXT_REMOVE_CUSTOM,MF_ENABLED);
		EnableMenuItem(hMenu,ID_CONTEXT_RENAME_CUSTOM,MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu,ID_CONTEXT_REMOVE_CUSTOM,MF_GRAYED);
		EnableMenuItem(hMenu,ID_CONTEXT_RENAME_CUSTOM,MF_GRAYED);
	}

	if (lpFolder && (IsSourceFolder(lpFolder) || IsVectorFolder(lpFolder)))
		EnableMenuItem(hMenu,ID_FOLDER_PROPERTIES,MF_ENABLED);
	else
		EnableMenuItem(hMenu,ID_FOLDER_PROPERTIES,MF_GRAYED);

	CheckMenuRadioItem(hMenu, ID_VIEW_TAB_SCREENSHOT, ID_VIEW_TAB_HISTORY,
					   ID_VIEW_TAB_SCREENSHOT + TabView_GetCurrentTab(hTabCtrl), MF_BYCOMMAND);

	// set whether we're showing the tab control or not
	if (bShowTabCtrl)
		CheckMenuItem(hMenu,ID_VIEW_PAGETAB,MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuItem(hMenu,ID_VIEW_PAGETAB,MF_BYCOMMAND | MF_UNCHECKED);


	for (i=0; i < MAX_TAB_TYPES; i++)
	{
		// disable menu items for tabs we're not currently showing
		if (GetShowTab(i))
			EnableMenuItem(hMenu,ID_VIEW_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_ENABLED);
		else
			EnableMenuItem(hMenu,ID_VIEW_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_GRAYED);

		// check toggle menu items 
		if (GetShowTab(i))
			CheckMenuItem(hMenu, ID_TOGGLE_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem(hMenu, ID_TOGGLE_TAB_SCREENSHOT + i,MF_BYCOMMAND | MF_UNCHECKED);
	}

	for (i=0; i < MAX_FOLDERS; i++)
	{
		if (GetShowFolder(i))
			CheckMenuItem(hMenu,ID_CONTEXT_SHOW_FOLDER_START + i,MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem(hMenu,ID_CONTEXT_SHOW_FOLDER_START + i,MF_BYCOMMAND | MF_UNCHECKED);
	}

}

void InitTreeContextMenu(HMENU hTreeMenu)
{
	MENUITEMINFOA mii;
	HMENU hMenu;
	int i;

	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize = sizeof(mii);

	mii.wID = -1;
	mii.fMask = MIIM_SUBMENU | MIIM_ID;

	hMenu = GetSubMenu(hTreeMenu, 0);

	if (GetMenuItemInfoA(hMenu,3,TRUE,&mii) == FALSE)
	{
		dprintf("can't find show folders context menu");
		return;
	}

	if (mii.hSubMenu == NULL)
	{
		dprintf("can't find submenu for show folders context menu");
		return;
	}

	hMenu = mii.hSubMenu;

	for (i=0; g_folderData[i].m_lpTitle != NULL; i++)
	{
		mii.fMask = MIIM_TYPE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.dwTypeData = (char *)g_folderData[i].m_lpTitle;
		mii.cch = strlen(mii.dwTypeData);
		mii.wID = ID_CONTEXT_SHOW_FOLDER_START + g_folderData[i].m_nFolderId;


		// menu in resources has one empty item (needed for the submenu to setup properly)
		// so overwrite this one, append after
		if (i == 0)
			SetMenuItemInfoA(hMenu,ID_CONTEXT_SHOW_FOLDER_START,FALSE,&mii);
		else
			InsertMenuItemA(hMenu,i,FALSE,&mii);
	}

}

void ToggleShowFolder(int folder)
{
	int current_id = GetCurrentFolderID();

	SetWindowRedraw(hwndList,FALSE);

	SetShowFolder(folder,!GetShowFolder(folder));

	ResetTreeViewFolders();
	SelectTreeViewFolder(current_id);

	SetWindowRedraw(hwndList,TRUE);
}

static LRESULT CALLBACK HistoryWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hBackground)
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE:
		{
			if (MouseHasBeenMoved())
				ShowCursor(TRUE);
			break;
		}

		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
		{
			POINT p = { 0, 0 };
			
			/* get base point of background bitmap */
			MapWindowPoints(hWnd,hTreeView,&p,1);
			PaintBackgroundImage(hWnd, NULL, p.x, p.y);
			/* to ensure our parent procedure repaints the whole client area */
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		}
	}
	return CallWindowProc(g_lpHistoryWndProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK PictureFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
    {
		if (MouseHasBeenMoved())
			ShowCursor(TRUE);
		break;
    }

	case WM_NCHITTEST :
	{
		POINT pt;
		RECT  rect;
		HWND hHistory = GetDlgItem(hMain, IDC_HISTORY);

		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		GetWindowRect(hHistory, &rect);
		// check if they clicked on the picture area (leave 6 pixel no man's land
		// by the history window to reduce mistaken clicks)
		// no more no man's land, the Cursor changes when Edit control is left, should be enough feedback
		if (have_history && NeedHistoryText() &&
//		        (rect.top - 6) < pt.y && pt.y < (rect.bottom + 6) )
		        PtInRect( &rect, pt ) )
		{
			return HTTRANSPARENT;
		}
		else
		{
			return HTCLIENT;
		}
		break;
	}

	case WM_CONTEXTMENU:
		if ( HandleScreenShotContextMenu(hWnd, wParam, lParam))
			return FALSE;
		break;
	}

	if (hBackground)
	{
		switch (uMsg)
		{
		case WM_ERASEBKGND :
			return TRUE;
		case WM_PAINT :
		{
			RECT rect,nodraw_rect;
			HRGN region,nodraw_region;
			POINT p = { 0, 0 };

			/* get base point of background bitmap */
			MapWindowPoints(hWnd,hTreeView,&p,1);

			/* get big region */
			GetClientRect(hWnd,&rect);
			region = CreateRectRgnIndirect(&rect);

			if (IsWindowVisible(GetDlgItem(hMain,IDC_HISTORY)))
			{
				/* don't draw over this window */
				GetWindowRect(GetDlgItem(hMain,IDC_HISTORY),&nodraw_rect);
				MapWindowPoints(HWND_DESKTOP,hWnd,(LPPOINT)&nodraw_rect,2);
				nodraw_region = CreateRectRgnIndirect(&nodraw_rect);
				CombineRgn(region,region,nodraw_region,RGN_DIFF);
				DeleteObject(nodraw_region);
			}
			if (IsWindowVisible(GetDlgItem(hMain,IDC_SSPICTURE)))
			{
				/* don't draw over this window */
				GetWindowRect(GetDlgItem(hMain,IDC_SSPICTURE),&nodraw_rect);
				MapWindowPoints(HWND_DESKTOP,hWnd,(LPPOINT)&nodraw_rect,2);
				nodraw_region = CreateRectRgnIndirect(&nodraw_rect);
				CombineRgn(region,region,nodraw_region,RGN_DIFF);
				DeleteObject(nodraw_region);
			}

			PaintBackgroundImage(hWnd,region,p.x,p.y);

			DeleteObject(region);

			/* to ensure our parent procedure repaints the whole client area */
			InvalidateRect(hWnd, NULL, FALSE);

			break;
		}
		}
	}
	return CallWindowProc(g_lpPictureFrameWndProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK PictureWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND :
		return TRUE;
	case WM_PAINT :
	{
		PAINTSTRUCT ps;
		HDC	hdc,hdc_temp;
		RECT rect;
		HBITMAP old_bitmap;

		int width,height;

		RECT rect2;
		HBRUSH hBrush;
		HBRUSH holdBrush;
		HRGN region1, region2;
		int nBordersize;
		nBordersize = GetScreenshotBorderSize();
		hBrush = CreateSolidBrush(GetScreenshotBorderColor());

		hdc = BeginPaint(hWnd,&ps);

		hdc_temp = CreateCompatibleDC(hdc);
		if (ScreenShotLoaded())
		{
			width = GetScreenShotWidth();
			height = GetScreenShotHeight();

			old_bitmap = SelectObject(hdc_temp,GetScreenShotHandle());
		}
		else
		{
			BITMAP bmp;

			GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);
			width = bmp.bmWidth;
			height = bmp.bmHeight;

			old_bitmap = SelectObject(hdc_temp,hMissing_bitmap);
		}

		GetClientRect(hWnd,&rect);

		rect2 = rect;
		//Configurable Borders around images
		rect.bottom -= nBordersize;
		if( rect.bottom < 0)
			rect.bottom = rect2.bottom;
		rect.right -= nBordersize;
		if( rect.right < 0)
			rect.right = rect2.right;
		rect.top += nBordersize;
		if( rect.top > rect.bottom )
			rect.top = rect2.top;
		rect.left += nBordersize;
		if( rect.left > rect.right )
			rect.left = rect2.left;
		region1 = CreateRectRgnIndirect(&rect);
		region2 = CreateRectRgnIndirect(&rect2);
		CombineRgn(region2,region2,region1,RGN_DIFF);
		holdBrush = SelectObject(hdc, hBrush); 

		FillRgn(hdc,region2, hBrush );
		SelectObject(hdc, holdBrush); 
		DeleteObject(hBrush); 

		SetStretchBltMode(hdc,STRETCH_HALFTONE);
		StretchBlt(hdc,nBordersize,nBordersize,rect.right-rect.left,rect.bottom-rect.top,
		           hdc_temp,0,0,width,height,SRCCOPY);
		SelectObject(hdc_temp,old_bitmap);
		DeleteDC(hdc_temp);

		EndPaint(hWnd,&ps);

		return TRUE;
	}
	}

	return CallWindowProc(g_lpPictureWndProc, hWnd, uMsg, wParam, lParam);
}

// replaces function in src/windows/fileio.c:
int osd_display_loading_rom_message(const char *name,rom_load_data *romdata)
{
	int retval;

	if (use_gui_romloading)
	{
		options.gui_host = 1;
		retval = UpdateLoadProgress(name,romdata);
	}
	else
	{
		if (name)
			fprintf(stdout, _WINDOWS("loading %-12s\r"), name);
		else
			fprintf(stdout, "%30s\r", "");
		fflush (stdout);
		retval = 0;
	}

	return retval;
}

static INT_PTR CALLBACK LoadProgressDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG :
	{
		char buf[256];
		
		TranslateDialog(hDlg, lParam, TRUE);
		sprintf(buf, _UI("Loading %s"), options.use_lang_list ?
			_LST(Machine->gamedrv->description):Machine->gamedrv->description);
		SetWindowText(hDlg, _Unicode(buf));
		
		g_bCloseLoading = FALSE;
		g_bAbortLoading = FALSE;

		return 1;
	}

	case WM_CLOSE:
		if (!g_bCloseLoading)
			g_bAbortLoading = TRUE;
		EndDialog(hDlg, 0);
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			g_bAbortLoading = TRUE;
			EndDialog(hDlg, IDCANCEL);
			return 1;
		}
		if (LOWORD(wParam) == IDOK)
		{
			g_bCloseLoading = TRUE;
			EndDialog(hDlg, IDOK);
			return 1;
		}
	}
	return 0;
}

int UpdateLoadProgress(const char* name, const rom_load_data *romdata)
{
	static HWND hWndLoad = 0;
	MSG Msg;

	int current = romdata->romsloaded;
	int total = romdata->romstotal;

	//dprintf("updateloadprogress %s %u %u %08x\n",name,current,total,hWndLoad);

	if (hWndLoad == NULL)
	{
		InitTranslator(options.langcode);
		hWndLoad = CreateDialog(GetModuleHandle(NULL),
		                        MAKEINTRESOURCE(IDD_LOAD_PROGRESS),
		                        hMain,
		                        LoadProgressDialogProc);

		EnableWindow(GetDlgItem(hWndLoad,IDOK),FALSE);
		EnableWindow(GetDlgItem(hWndLoad,IDCANCEL),TRUE);

		ShowWindow(hWndLoad,SW_SHOW);
	}

	SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_STATUS),
				  _Unicode(ConvertToWindowsNewlines(romdata->errorbuf)));

	SendDlgItemMessage(hWndLoad, IDC_LOAD_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, total));
	SendDlgItemMessage(hWndLoad, IDC_LOAD_PROGRESS, PBM_SETPOS, current, 0);

	if (name == NULL)
	{
		// final call to us
		SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_ROMNAME), TEXT(""));
		if (romdata->errors > 0)
		{

			/*
			  Shows the load progress dialog if there is an error while
			  loading the game.
			*/

			ShowWindow(hWndLoad, SW_SHOW);

			EnableWindow(GetDlgItem(hWndLoad,IDOK),TRUE);
			if (romdata->errors != 0)
				EnableWindow(GetDlgItem(hWndLoad,IDCANCEL),FALSE);
			SetFocus(GetDlgItem(hWndLoad,IDOK));
			if (romdata->errors)
				SetWindowText(GetDlgItem(hWndLoad,IDC_ERROR_TEXT),
							  _Unicode(_UI("ERROR: required files are missing, the game cannot be run.")));
			else
				SetWindowText(GetDlgItem(hWndLoad,IDC_ERROR_TEXT),
							  _Unicode(_UI("WARNING: the game might not run correctly.")));
		}
	}
	else
		SetWindowText(GetDlgItem(hWndLoad, IDC_LOAD_ROMNAME), _Unicode(name));

	if (name == NULL && romdata->errors > 0)
	{
		while (GetMessage(&Msg, NULL, 0, 0))
		{
			if (!IsDialogMessage(hWndLoad, &Msg))
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

			// make sure the user clicks-through on an error/warning
			if (g_bCloseLoading || g_bAbortLoading)
				break;
		}

	}

	if (name == NULL)
	{
		DestroyWindow(hWndLoad);
		hWndLoad = NULL;
	}

	// take care of any pending messages
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(hWndLoad, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	// if abort with a warning, gotta exit abruptly
	if (name == NULL && g_bAbortLoading && romdata->errors == 0)
		return 1;

	// if abort along the way, tell 'em
	if (g_bAbortLoading && name != NULL)
	{
		return 1;
	}

	return 0;
}

void RemoveCurrentGameCustomFolder(void)
{
	RemoveGameCustomFolder(Picker_GetSelectedItem(hwndList));
}

void RemoveGameCustomFolder(int driver_index)
{
	int i;
	TREEFOLDER **folders;
	int num_folders;

	GetFolders(&folders,&num_folders);
	
	for (i=0;i<num_folders;i++)
	{
		if (folders[i]->m_dwFlags & F_CUSTOM && folders[i]->m_nFolderId == GetCurrentFolderID())
		{
			int current_pick_index;

			RemoveFromCustomFolder(folders[i],driver_index);

			if (driver_index == Picker_GetSelectedItem(hwndList))
			{
				/* if we just removed the current game,
				  move the current selection so that when we rebuild the listview it
				  leaves the cursor on next or previous one */
			
				current_pick_index = GetSelectedPick();
				Picker_SetSelectedPick(hwndList, GetSelectedPick() + 1);
				if (current_pick_index == GetSelectedPick()) /* we must have deleted the last item */
					Picker_SetSelectedPick(hwndList, GetSelectedPick() - 1);
			}

			ResetListView();
			return;
		}
	}
	MessageBox(GetMainWindow(), _Unicode(_UI("Error searching for custom folder")), TEXT_MAME32NAME, MB_OK | MB_ICONERROR);

}


void BeginListViewDrag(NM_LISTVIEW *pnmv)
{
	LV_ITEM lvi;
	POINT pt;

	lvi.iItem = pnmv->iItem;
	lvi.mask	 = LVIF_PARAM;
	ListView_GetItem(hwndList, &lvi);

	game_dragged = lvi.lParam;

	pt.x = 0;
	pt.y = 0;

	/* Tell the list view control to create an image to use 
	   for dragging. */
	himl_drag = ListView_CreateDragImage(hwndList,pnmv->iItem,&pt);
 
	/* Start the drag operation. */
	ImageList_BeginDrag(himl_drag, 0, 0, 0); 

	pt = pnmv->ptAction;
	ClientToScreen(hwndList,&pt);
	ImageList_DragEnter(GetDesktopWindow(),pt.x,pt.y);

	/* Hide the mouse cursor, and direct mouse input to the 
	   parent window. */
	SetCapture(hMain);

	prev_drag_drop_target = NULL;

	g_listview_dragging = TRUE; 

}

void MouseMoveListViewDrag(POINTS p)
{
	HTREEITEM htiTarget;
	TV_HITTESTINFO tvht;

	POINT pt;
	pt.x = p.x;
	pt.y = p.y;

	ClientToScreen(hMain,&pt);

	ImageList_DragMove(pt.x,pt.y);

	MapWindowPoints(GetDesktopWindow(),hTreeView,&pt,1);

	tvht.pt = pt;
	htiTarget = TreeView_HitTest(hTreeView,&tvht);

	if (htiTarget != prev_drag_drop_target)
	{
		ImageList_DragShowNolock(FALSE);
		if (htiTarget != NULL)
			TreeView_SelectDropTarget(hTreeView,htiTarget);
		else
			TreeView_SelectDropTarget(hTreeView,NULL);
		ImageList_DragShowNolock(TRUE);

		prev_drag_drop_target = htiTarget;
	}
}

void ButtonUpListViewDrag(POINTS p)
{
	POINT pt;
	HTREEITEM htiTarget;
	TV_HITTESTINFO tvht;
	TVITEM tvi;
	
	ReleaseCapture();

	ImageList_DragLeave(hwndList);
	ImageList_EndDrag();
	ImageList_Destroy(himl_drag);

	TreeView_SelectDropTarget(hTreeView,NULL);

	g_listview_dragging = FALSE;

	/* see where the game was dragged */

	pt.x = p.x;
	pt.y = p.y;

	MapWindowPoints(hMain,hTreeView,&pt,1);

	tvht.pt = pt;
	htiTarget = TreeView_HitTest(hTreeView,&tvht);
	if (htiTarget == NULL)
	{
		LVHITTESTINFO lvhtti;
		LPTREEFOLDER folder;
		RECT rcList;

		/* the user dragged a game onto something other than the treeview */
		/* try to remove if we're in a custom folder */

		/* see if it was dragged within the list view; if so, ignore */

		MapWindowPoints(hTreeView,hwndList,&pt,1);
		lvhtti.pt = pt;
		GetWindowRect(hwndList, &rcList);
		ClientToScreen(hwndList, &pt);
		if( PtInRect(&rcList, pt) != 0 )
			return;

		folder = GetCurrentFolder();
		if (folder->m_dwFlags & F_CUSTOM)
		{
			/* dragged out of a custom folder, so let's remove it */
			RemoveCurrentGameCustomFolder();
		}
		return;
	}


	tvi.lParam = 0;
	tvi.mask  = TVIF_PARAM | TVIF_HANDLE;
	tvi.hItem = htiTarget;

	if (TreeView_GetItem(hTreeView, &tvi))
	{
		LPTREEFOLDER folder = (LPTREEFOLDER)tvi.lParam;
		AddToCustomFolder(folder,game_dragged);
	}

}

static LPTREEFOLDER GetSelectedFolder(void)
{
	HTREEITEM htree;
	TVITEM tvi;
	htree = TreeView_GetSelection(hTreeView);
	if(htree != NULL)
	{
		tvi.hItem = htree;
		tvi.mask = TVIF_PARAM;
		TreeView_GetItem(hTreeView,&tvi);
		return (LPTREEFOLDER)tvi.lParam;
	}
	return NULL;
}

static HICON GetSelectedFolderIcon(void)
{
	HTREEITEM htree;
	TVITEM tvi;
	HIMAGELIST hSmall_icon;
	LPTREEFOLDER folder;
	htree = TreeView_GetSelection(hTreeView);

	if (htree != NULL)
	{
		tvi.hItem = htree;
		tvi.mask = TVIF_PARAM;
		TreeView_GetItem(hTreeView,&tvi);
		
		folder = (LPTREEFOLDER)tvi.lParam;
		//hSmall_icon = TreeView_GetImageList(hTreeView,(int)tvi.iImage);
		hSmall_icon = NULL;
		return ImageList_GetIcon(hSmall_icon, tvi.iImage, ILD_TRANSPARENT);
	}
	return NULL;
}

/* Updates all currently displayed Items in the List with the latest Data*/
void UpdateListView(void)
{
	if( (GetViewMode() == VIEW_GROUPED) || (GetViewMode() == VIEW_DETAILS ) )
	    ListView_RedrawItems(hwndList,ListView_GetTopIndex(hwndList),
	                         ListView_GetTopIndex(hwndList)+ ListView_GetCountPerPage(hwndList) );
}

void CalculateBestScreenShotRect(HWND hWnd, RECT *pRect, BOOL restrict_height)
{
	int 	destX, destY;
	int 	destW, destH;
	int	nBorder;
	RECT	rect;
	/* for scaling */		 
	int x, y;
	int rWidth, rHeight;
	double scale;
	BOOL bReduce = FALSE;

	GetClientRect(hWnd, &rect);

	// Scale the bitmap to the frame specified by the passed in hwnd
	if (ScreenShotLoaded())
	{
		x = GetScreenShotWidth();
		y = GetScreenShotHeight();
	}
	else
	{
		BITMAP bmp;
		GetObject(hMissing_bitmap,sizeof(BITMAP),&bmp);

		x = bmp.bmWidth;
		y = bmp.bmHeight;
	}
	rWidth	= (rect.right  - rect.left);
	rHeight = (rect.bottom - rect.top);

	/* Limit the screen shot to max height of 264 */
	if (restrict_height == TRUE && rHeight > 264)
	{
		rect.bottom = rect.top + 264;
		rHeight = 264;
	}

	/* If the bitmap does NOT fit in the screenshot area */
	if (x > rWidth - 10 || y > rHeight - 10)
	{
		rect.right	-= 10;
		rect.bottom -= 10;
		rWidth	-= 10;
		rHeight -= 10;
		bReduce = TRUE;
		/* Try to scale it properly */
		/*	assumes square pixels, doesn't consider aspect ratio */
		if (x > y)
			scale = (double)rWidth / x;
		else
			scale = (double)rHeight / y;

		destW = (int)(x * scale);
		destH = (int)(y * scale);

		/* If it's still too big, scale again */
		if (destW > rWidth || destH > rHeight)
		{
			if (destW > rWidth)
				scale = (double)rWidth	/ destW;
			else
				scale = (double)rHeight / destH;

			destW = (int)(destW * scale);
			destH = (int)(destH * scale);
		}
	}
	else
	{
		if (GetStretchScreenShotLarger())
		{
			rect.right	-= 10;
			rect.bottom -= 10;
			rWidth	-= 10;
			rHeight -= 10;
			bReduce = TRUE;
			// Try to scale it properly
			// assumes square pixels, doesn't consider aspect ratio
			if (x < y)
				scale = (double)rWidth / x;
			else
				scale = (double)rHeight / y;
			
			destW = (int)(x * scale);
			destH = (int)(y * scale);
			
			// If it's too big, scale again
			if (destW > rWidth || destH > rHeight)
			{
				if (destW > rWidth)
					scale = (double)rWidth	/ destW;
				else
					scale = (double)rHeight / destH;
				
				destW = (int)(destW * scale);
				destH = (int)(destH * scale);
			}
		}
		else
		{
			// Use the bitmaps size if it fits
			destW = x;
			destH = y;
		}

	}


	destX = ((rWidth  - destW) / 2);
	destY = ((rHeight - destH) / 2);

	if (bReduce)
	{
		destX += 5;
		destY += 5;
	}
	nBorder = GetScreenshotBorderSize();
	if( destX > nBorder+1)
		pRect->left   = destX - nBorder;
	else
		pRect->left   = 2;
	if( destY > nBorder+1)
		pRect->top    = destY - nBorder;
	else
		pRect->top    = 2;
	if( rWidth >= destX + destW + nBorder)
		pRect->right  = destX + destW + nBorder;
	else
		pRect->right  = rWidth - pRect->left;
	if( rHeight >= destY + destH + nBorder)
		pRect->bottom = destY + destH + nBorder;
	else
		pRect->bottom = rHeight - pRect->top;
}

/*
  Switches to either fullscreen or normal mode, based on the
  current mode.

  POSSIBLE BUGS:
  Removing the menu might cause problems later if some
  function tries to poll info stored in the menu. Don't
  know if you've done that, but this was the only way I
  knew to remove the menu dynamically. 
*/

void SwitchFullScreenMode(void)
{
	LONG lMainStyle;
	
	if (GetRunFullScreen())
	{
		// Return to normal

		// Restore the menu
		SetMenu(hMain, LoadMenu(hInst,MAKEINTRESOURCE(IDR_UI_MENU)));
		TranslateMenu(GetMenu(hMain), 0);

		// Refresh the checkmarks
		CheckMenuItem(GetMenu(hMain), ID_VIEW_FOLDERS, GetShowFolderList() ? MF_CHECKED : MF_UNCHECKED); 
		CheckMenuItem(GetMenu(hMain), ID_VIEW_TOOLBARS, GetShowToolBar() ? MF_CHECKED : MF_UNCHECKED);    
		CheckMenuItem(GetMenu(hMain), ID_VIEW_STATUS, GetShowStatusBar() ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetMenu(hMain), ID_VIEW_PAGETAB, GetShowTabCtrl() ? MF_CHECKED : MF_UNCHECKED);

		// Add frame to dialog again
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle | WS_BORDER;
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);

		// Show the window maximized
		if( GetWindowState() == SW_MAXIMIZE )
/*
		{
			ShowWindow(hMain, SW_NORMAL);
			ShowWindow(hMain, SW_MAXIMIZE);
		}
		else
*/
			ShowWindow(hMain, SW_RESTORE);

		SetRunFullScreen(FALSE);
	}
	else
	{
		// Set to fullscreen

		// Remove menu
		SetMenu(hMain,NULL); 

		// Frameless dialog (fake fullscreen)
		lMainStyle = GetWindowLong(hMain, GWL_STYLE);
		lMainStyle = lMainStyle & (WS_BORDER ^ 0xffffffff);
		SetWindowLong(hMain, GWL_STYLE, lMainStyle);
		if( IsMaximized(hMain) )
		{
			ShowWindow(hMain, SW_NORMAL);
			SetWindowState( SW_MAXIMIZE );
		}
		ShowWindow(hMain, SW_MAXIMIZE);

		SetRunFullScreen(TRUE);
	}
}

/*
  Checks to see if the mouse has been moved since this func
  was first called (which is at startup). The reason for 
  storing the startup coordinates of the mouse is that when
  a window is created it generates WM_MOUSEOVER events, even
  though the user didn't actually move the mouse. So we need
  to know when the WM_MOUSEOVER event is user-triggered.

  POSSIBLE BUGS:
  Gets polled at every WM_MOUSEMOVE so it might cause lag,
  but there's probably another way to code this that's 
  way better?
  
*/

BOOL MouseHasBeenMoved(void)
{
	static int mouse_x = -1;
	static int mouse_y = -1;
	POINT p;

	GetCursorPos(&p);

	if (mouse_x == -1) // First time
	{
		mouse_x = p.x;
		mouse_y = p.y;
	}
	
	return (p.x != mouse_x || p.y != mouse_y);       
}

/*
	The following two functions enable us to send Messages to the Game Window
*/
#if MULTISESSION

BOOL SendIconToEmulationWindow(int nGameIndex)
{
	const game_driver *clone_of = NULL;
	HICON hIcon; 
	hIcon = LoadIconFromFile(drivers[nGameIndex]->name); 
	if( hIcon == NULL ) 
	{ 
		//Check if clone, if so try parent icon first 
		if( DriverIsClone(nGameIndex) ) 
		{ 
			if( ( clone_of = driver_get_clone(drivers[nGameIndex])) != NULL )
				hIcon = LoadIconFromFile(clone_of->name); 
			if( hIcon == NULL) 
			{ 
				hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
			} 
		} 
		else 
		{ 
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
		} 
	} 
	if( SendMessageToEmulationWindow( WM_SETICON, ICON_SMALL, (LPARAM)hIcon ) == FALSE)
		return FALSE;
	if( SendMessageToEmulationWindow( WM_SETICON, ICON_BIG, (LPARAM)hIcon ) == FALSE)
		return FALSE;
	return TRUE;
}

BOOL SendMessageToEmulationWindow(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = NULL;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	if( fwhs.hwndFound )
	{
		SendMessage(fwhs.hwndFound, Msg, wParam, lParam);
		//Fix for loosing Focus, we reset the Focus on our own Main window
		SendMessage(fwhs.hwndFound, WM_KILLFOCUS,(WPARAM) hMain, (LPARAM) NULL);
		return TRUE;
	}
	return FALSE;
}


static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	FINDWINDOWHANDLE * pfwhs = (FINDWINDOWHANDLE * )lParam; 
	DWORD ProcessId, ProcessIdGUI; 
	char buffer[MAX_PATH]; 

	GetWindowThreadProcessId(hwnd, &ProcessId);
	GetWindowThreadProcessId(hMain, &ProcessIdGUI);

	// cmk--I'm not sure I believe this note is necessary
	// note: In order to make sure we have the MainFrame, verify that the title 
	// has Zero-Length. Under Windows 98, sometimes the Client Window ( which doesn't 
	// have a title ) is enumerated before the MainFrame 

	GetWindowText(hwnd, buffer, sizeof(buffer));
	if (ProcessId  == ProcessIdGUI &&
		 strncmp(buffer,MAMENAME,strlen(MAMENAME)) == 0 &&
		 hwnd != hMain) 
	{ 
		pfwhs->hwndFound = hwnd; 
		return FALSE; 
	} 
	else 
	{ 
		// Keep enumerating 
		return TRUE; 
	} 
}

HWND GetGameWindow(void)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = NULL;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	return fwhs.hwndFound;
}


#else

void SendIconToProcess(LPPROCESS_INFORMATION pi, int nGameIndex)
{
	HICON hIcon; 
	const game_driver *clone_of = NULL;
	hIcon = LoadIconFromFile(drivers[nGameIndex]->name); 
	if( hIcon == NULL ) 
	{ 
		//Check if clone, if so try parent icon first 
		if( DriverIsClone(nGameIndex) ) 
		{ 
			if( ( clone_of = driver_get_clone(drivers[nGameIndex])) != NULL )
				hIcon = LoadIconFromFile(clone_of->name); 
			if( hIcon == NULL) 
			{ 
				hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
			} 
		} 
		else 
		{ 
			hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAME32_ICON)); 
		} 
	} 
	WaitForInputIdle( pi->hProcess, INFINITE ); 
	SendMessageToProcess( pi, WM_SETICON, ICON_SMALL, (LPARAM)hIcon ); 
	SendMessageToProcess( pi, WM_SETICON, ICON_BIG, (LPARAM)hIcon ); 
}

void SendMessageToProcess(LPPROCESS_INFORMATION lpProcessInformation, 
                                      UINT Msg, WPARAM wParam, LPARAM lParam)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = lpProcessInformation;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);

	SendMessage(fwhs.hwndFound, Msg, wParam, lParam);
	//Fix for loosing Focus, we reset the Focus on our own Main window
	SendMessage(fwhs.hwndFound, WM_KILLFOCUS,(WPARAM) hMain, (LPARAM) NULL);
}

static BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	FINDWINDOWHANDLE * pfwhs = (FINDWINDOWHANDLE * )lParam; 
	DWORD ProcessId; 
	char buffer[MAX_PATH]; 

	GetWindowThreadProcessId(hwnd, &ProcessId);

	// cmk--I'm not sure I believe this note is necessary
	// note: In order to make sure we have the MainFrame, verify that the title 
	// has Zero-Length. Under Windows 98, sometimes the Client Window ( which doesn't 
	// have a title ) is enumerated before the MainFrame 

	GetWindowTextA(hwnd, buffer, sizeof(buffer));
	if (ProcessId  == pfwhs->ProcessInfo->dwProcessId &&
		 strncmp(buffer,MAMENAME,strlen(MAMENAME)) == 0) 
	{ 
		pfwhs->hwndFound = hwnd; 
		return FALSE; 
	} 
	else 
	{ 
		// Keep enumerating 
		return TRUE; 
	} 
}

HWND GetGameWindow(LPPROCESS_INFORMATION lpProcessInformation)
{
	FINDWINDOWHANDLE fwhs;
	fwhs.ProcessInfo = lpProcessInformation;
	fwhs.hwndFound  = NULL;

	EnumWindows(EnumWindowCallBack, (LPARAM)&fwhs);
	return fwhs.hwndFound;
}

#endif

#ifdef USE_SHOW_SPLASH_SCREEN
static LRESULT CALLBACK BackMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_ERASEBKGND:
		{
			BITMAP Bitmap;
			GetObject(hSplashBmp, sizeof(BITMAP), &Bitmap);
			BitBlt((HDC)wParam, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemoryDC, 0, 0, SRCCOPY);
			break;
		}

		default:
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}

	return FALSE;
}

static void CreateBackgroundMain(HINSTANCE hInstance)
{
	static HDC hSplashDC = 0;

	WNDCLASSEX BackMainClass;

	BackMainClass.cbSize        = sizeof(WNDCLASSEX);
	BackMainClass.style         = 0;
	BackMainClass.lpfnWndProc   = (WNDPROC)BackMainWndProc;
	BackMainClass.cbClsExtra    = 0;
	BackMainClass.cbWndExtra    = 0;
	BackMainClass.hInstance     = hInstance;
	BackMainClass.hIcon         = NULL;
	BackMainClass.hIconSm       = NULL;
	BackMainClass.hCursor       = NULL;
	BackMainClass.hbrBackground = NULL;
	BackMainClass.lpszMenuName  = NULL;
	BackMainClass.lpszClassName = TEXT("BackMainWindowClass");

	if ( RegisterClassEx(&BackMainClass) )
	{
		BITMAP Bitmap;
		RECT DesktopRect;

		GetWindowRect(GetDesktopWindow(), &DesktopRect);
		hSplashBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
		GetObject(hSplashBmp, sizeof(BITMAP), &Bitmap);

		hBackMain = CreateWindowEx(WS_EX_TOOLWINDOW,
					TEXT("BackMainWindowClass"),
					TEXT("Main Backgound windows"),
					WS_POPUP,
					(DesktopRect.right - Bitmap.bmWidth) / 2,
					(DesktopRect.bottom - Bitmap.bmHeight) / 2,
					Bitmap.bmWidth,
					Bitmap.bmHeight,
					NULL,
					NULL,
					hInstance,
					NULL);

		hSplashDC = GetDC(hBackMain);
		hMemoryDC = CreateCompatibleDC(hSplashDC);
		SelectObject(hMemoryDC, (HGDIOBJ)hSplashBmp);

		if (GetDisplaySplashScreen() != FALSE)
			ShowWindow(hBackMain, SW_SHOW);

		UpdateWindow(hBackMain);
	}
}

static void DestroyBackgroundMain(void)
{
	static HDC hSplashDC = 0;

	if ( hBackMain )
	{
		DeleteObject(hSplashBmp);
		ReleaseDC(hBackMain, hSplashDC);
		ReleaseDC(hBackMain, hMemoryDC);
		DestroyWindow(hBackMain);
	}
}
#endif /* USE_SHOW_SPLASH_SCREEN */

/* End of source file */
