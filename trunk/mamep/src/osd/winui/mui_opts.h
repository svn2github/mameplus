/***************************************************************************

  M.A.M.E.UI  -  Multiple Arcade Machine Emulator with User Interface
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse,
  Copyright (C) 2003-2007 Chris Kirmse and the MAME32/MAMEUI team.

  This file is part of MAMEUI, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef MUI_OPTS_H
#define MUI_OPTS_H

#include "osdcomm.h"
#include "options.h"
#include "emu.h" /* for input_seq definition */
#include <video.h> /* for MAX_SCREENS Definition*/
#include "winmain.h"

#ifdef MESS
#include "optionsms.h"
#endif

#define MAX_SYSTEM_BIOS		16
#define MAX_SYSTEM_BIOS_ENTRY	32
#define BIOS_DEFAULT		"default"

//mamep: DATAFILE
#define MUIOPTION_HISTORY_FILE					"history_file"
#define MUIOPTION_MAMEINFO_FILE					"mameinfo_file"
#ifdef STORY_DATAFILE
#define MUIOPTION_STORY_FILE					"story_file"
#endif /* STORY_DATAFILE */

// Various levels of ini's we can edit.
typedef enum {
	OPTIONS_GLOBAL = 0,
	OPTIONS_HORIZONTAL,
	OPTIONS_VERTICAL,
	OPTIONS_VECTOR,
	OPTIONS_SOURCE,
	OPTIONS_PARENT,
	OPTIONS_GAME,
	OPTIONS_MAX
} OPTIONS_TYPE;


enum 
{
	COLUMN_GAMES = 0,
	COLUMN_ORIENTATION,
	COLUMN_ROMS,
	COLUMN_SAMPLES,
	COLUMN_DIRECTORY,
	COLUMN_TYPE,
	COLUMN_TRACKBALL,
	COLUMN_PLAYED,
	COLUMN_MANUFACTURER,
	COLUMN_YEAR,
	COLUMN_CLONE,
	COLUMN_SRCDRIVERS,
	COLUMN_PLAYTIME,
	COLUMN_MAX
};


// can't be the same as the VerifyRomSet() results, listed in audit.h
enum
{
	UNKNOWN	= -1
};

/* Default input */
/*
enum 
{
	INPUT_LAYOUT_STD,
	INPUT_LAYOUT_HR,
	INPUT_LAYOUT_HRSE
};

// clean stretch types
enum
{
	// these must match array of strings clean_stretch_long_name in options.c
	CLEAN_STRETCH_NONE = 0,
	CLEAN_STRETCH_AUTO = 1,

	MAX_CLEAN_STRETCH = 5,
};
*/

#define FOLDER_OPTIONS	-2
#define GLOBAL_OPTIONS	-1

/*
// d3d effect types
enum
{
	// these must match array of strings d3d_effects_long_name in options.c
	D3D_EFFECT_NONE = 0,
	D3D_EFFECT_AUTO = 1,

	MAX_D3D_EFFECTS = 17,
};

// d3d prescale types
enum
{
	D3D_PRESCALE_NONE = 0,
	D3D_PRESCALE_AUTO = 1,
	MAX_D3D_PRESCALE = 10,
};
*/

typedef struct
{
	int x, y, width, height;
} AREA;

/*
typedef struct 
{
	char* screen;
	char* aspect;
	char* resolution;
	char* view;
} ScreenParams;
*/



// List of artwork types to display in the screen shot area
enum
{
	// these must match array of strings image_tabs_long_name in options.c
	// if you add new Tabs, be sure to also add them to the ComboBox init in dialogs.c
	TAB_SCREENSHOT = 0,
	TAB_FLYER,
	TAB_CABINET,
	TAB_MARQUEE,
	TAB_TITLE,
	TAB_CONTROL_PANEL,
	TAB_PCB,
	TAB_HISTORY,
#ifdef STORY_DATAFILE
	TAB_STORY,
#endif /* STORY_DATAFILE */

	MAX_TAB_TYPES,
	BACKGROUND,
	TAB_ALL,
	TAB_NONE
#ifdef USE_IPS
	,TAB_IPS
#endif /* USE_IPS */
};
// Because we have added the Options after MAX_TAB_TYPES, we have to subtract 3 here
// (that's how many options we have after MAX_TAB_TYPES)
#define TAB_SUBTRACT 3

class winui_options : public core_options
{
public:
	// construction/destruction
	winui_options();

private:
	static const options_entry s_option_entries[];
};

BOOL OptionsInit(void);
void OptionsExit(void);

#define OPTIONS_TYPE_GLOBAL		-1
#define OPTIONS_TYPE_FOLDER		-2

void load_options(windows_options &opts, OPTIONS_TYPE opt_type, int game_num);
void save_options(OPTIONS_TYPE opt_type, windows_options &opts, int game_num);

//void AddOptions(winui_options *opts, const options_entry *entrylist, BOOL is_global);
void CreateGameOptions(windows_options &opts, int driver_index);

winui_options & MameUISettings(void);
windows_options & MameUIGlobal(void);

//void LoadFolderFlags(void);
//const char* GetFolderNameByID(UINT nID);
DWORD LoadFolderFlags(const char *path);
void SaveFolderFlags(const char *path, DWORD flags);

void SaveOptions(void);

void ResetGUI(void);
void ResetGameDefaults(void);
void ResetAllGameOptions(void);

const WCHAR *GetImageTabLongName(int tab_index);
const char *GetImageTabShortName(int tab_index);

void SetViewMode(int val);
int  GetViewMode(void);

void SetGameCheck(BOOL game_check);
BOOL GetGameCheck(void);

void SetVersionCheck(BOOL version_check);
BOOL GetVersionCheck(void);

void SetJoyGUI(BOOL use_joygui);
BOOL GetJoyGUI(void);

void SetKeyGUI(BOOL use_keygui);
BOOL GetKeyGUI(void);

void SetCycleScreenshot(int cycle_screenshot);
int GetCycleScreenshot(void);

void SetStretchScreenShotLarger(BOOL stretch);
BOOL GetStretchScreenShotLarger(void);

void SetScreenshotBorderSize(int size);
int GetScreenshotBorderSize(void);

void SetScreenshotBorderColor(COLORREF uColor);
COLORREF GetScreenshotBorderColor(void);

void SetFilterInherit(BOOL inherit);
BOOL GetFilterInherit(void);

void SetOffsetClones(BOOL offset);
BOOL GetOffsetClones(void);

void SetBroadcast(BOOL broadcast);
BOOL GetBroadcast(void);

void SetRandomBackground(BOOL random_bg);
BOOL GetRandomBackground(void);

void SetSavedFolderID(UINT val);
UINT GetSavedFolderID(void);

void SetShowScreenShot(BOOL val);
BOOL GetShowScreenShot(void);

void SetShowFolderList(BOOL val);
BOOL GetShowFolderList(void);

BOOL GetShowFolder(int folder);
void SetShowFolder(int folder,BOOL show);

void SetShowStatusBar(BOOL val);
BOOL GetShowStatusBar(void);

void SetShowToolBar(BOOL val);
BOOL GetShowToolBar(void);

void SetShowTabCtrl(BOOL val);
BOOL GetShowTabCtrl(void);

void SetCurrentTab(const char *shortname);
const char *GetCurrentTab(void);

void SetDefaultGame(const char *name);
const char *GetDefaultGame(void);

void SetWindowArea(const AREA *area);
void GetWindowArea(AREA *area);

void SetWindowState(UINT state);
UINT GetWindowState(void);

void SetColumnWidths(int widths[]);
void GetColumnWidths(int widths[]);

void SetColumnOrder(int order[]);
void GetColumnOrder(int order[]);

void SetColumnShown(int shown[]);
void GetColumnShown(int shown[]);

void SetSplitterPos(int splitterId, int pos);
int  GetSplitterPos(int splitterId);

void SetCustomColor(int iIndex, COLORREF uColor);
COLORREF GetCustomColor(int iIndex);

void SetListFont(const LOGFONTW *font);
void GetListFont(LOGFONTW *font);

//DWORD GetFolderFlags(int folder_index);

void SetListFontColor(COLORREF uColor);
COLORREF GetListFontColor(void);

void SetListCloneColor(COLORREF uColor);
COLORREF GetListCloneColor(void);

int GetHistoryTab(void);
void SetHistoryTab(int tab,BOOL show);

int GetShowTab(int tab);
void SetShowTab(int tab,BOOL show);
BOOL AllowedToSetShowTab(int tab,BOOL show);

void SetSortColumn(int column);
int  GetSortColumn(void);

void SetSortReverse(BOOL reverse);
BOOL GetSortReverse(void);

//const char* GetLanguage(void);
//void SetLanguage(const char* lang);

const WCHAR *GetRomDirs(void);
void SetRomDirs(const WCHAR *paths);

const WCHAR *GetSampleDirs(void);
void  SetSampleDirs(const WCHAR *paths);

const WCHAR *GetIniDir(void);
void SetIniDir(const WCHAR *path);

const WCHAR *GetCfgDir(void);
void SetCfgDir(const WCHAR *path);

const WCHAR *GetNvramDir(void);
void SetNvramDir(const WCHAR *path);

const WCHAR *GetInpDir(void);
void SetInpDir(const WCHAR *path);

const WCHAR *GetImgDir(void);
void SetImgDir(const WCHAR *path);

const WCHAR *GetStateDir(void);
void SetStateDir(const WCHAR *path);

const WCHAR *GetArtDir(void);
void SetArtDir(const WCHAR *path);

const WCHAR *GetMemcardDir(void);
void SetMemcardDir(const WCHAR *path);

const WCHAR *GetFlyerDir(void);
void SetFlyerDir(const WCHAR *path);

const WCHAR *GetCabinetDir(void);
void SetCabinetDir(const WCHAR *path);

const WCHAR *GetMarqueeDir(void);
void SetMarqueeDir(const WCHAR *path);

const WCHAR *GetTitlesDir(void);
void SetTitlesDir(const WCHAR *path);

const WCHAR *GetControlPanelDir(void);
void SetControlPanelDir(const WCHAR *path);

const WCHAR * GetPcbDir(void);
void SetPcbDir(const WCHAR *path);

const WCHAR *GetDiffDir(void);
void SetDiffDir(const WCHAR *path);

const WCHAR *GetIconsDir(void);
void SetIconsDir(const WCHAR *path);

const WCHAR *GetBgDir(void);
void SetBgDir(const WCHAR *path);

const WCHAR *GetCtrlrDir(void);
void SetCtrlrDir(const WCHAR *path);

const WCHAR *GetCommentDir(void);
void SetCommentDir(const WCHAR *path);

const WCHAR *GetFolderDir(void);
void SetFolderDir(const WCHAR *path);

const WCHAR* GetFontDir(void);
void  SetFontDir(const WCHAR* paths);

const WCHAR* GetCrosshairDir(void);
void  SetCrosshairDir(const WCHAR* paths);

const WCHAR *GetCheatDir(void);
void SetCheatDir(const WCHAR *path);

const WCHAR *GetHistoryFileName(void);
void SetHistoryFileName(const WCHAR *path);

const WCHAR *GetMAMEInfoFileName(void);
void SetMAMEInfoFileName(const WCHAR *path);

const char* GetSnapName(void);
void SetSnapName(const char* pattern);

void ResetGameOptions(int driver_index);

int GetRomAuditResults(int driver_index);
void SetRomAuditResults(int driver_index, int audit_results);

int GetSampleAuditResults(int driver_index);
void SetSampleAuditResults(int driver_index, int audit_results);

void IncrementPlayCount(int driver_index);
int GetPlayCount(int driver_index);
void ResetPlayCount(int driver_index);

void IncrementPlayTime(int driver_index,int playtime);
int GetPlayTime(int driver_index);
void GetTextPlayTime(int driver_index, WCHAR *buf);
void ResetPlayTime(int driver_index);

const char *GetVersionString(void);

void SaveDefaultOptions(void);

BOOL IsGlobalOption(const char *option_name);



// Keyboard control of ui
input_seq* Get_ui_key_up(void);
input_seq* Get_ui_key_down(void);
input_seq* Get_ui_key_left(void);
input_seq* Get_ui_key_right(void);
input_seq* Get_ui_key_start(void);
input_seq* Get_ui_key_pgup(void);
input_seq* Get_ui_key_pgdwn(void);
input_seq* Get_ui_key_home(void);
input_seq* Get_ui_key_end(void);
input_seq* Get_ui_key_ss_change(void);
input_seq* Get_ui_key_history_up(void);
input_seq* Get_ui_key_history_down(void);

input_seq* Get_ui_key_context_filters(void);
input_seq* Get_ui_key_select_random(void);
input_seq* Get_ui_key_game_audit(void);
input_seq* Get_ui_key_game_properties(void);
input_seq* Get_ui_key_help_contents(void);
input_seq* Get_ui_key_update_gamelist(void);
input_seq* Get_ui_key_view_folders(void);
input_seq* Get_ui_key_view_fullscreen(void);
input_seq* Get_ui_key_view_pagetab(void);
input_seq* Get_ui_key_view_picture_area(void);
input_seq* Get_ui_key_view_status(void);
input_seq* Get_ui_key_view_toolbars(void);

input_seq* Get_ui_key_view_tab_cabinet(void);
input_seq* Get_ui_key_view_tab_cpanel(void);
input_seq* Get_ui_key_view_tab_flyer(void);
input_seq* Get_ui_key_view_tab_history(void);
#ifdef STORY_DATAFILE
input_seq *Get_ui_key_view_tab_story(void);
#endif /* STORY_DATAFILE */
input_seq* Get_ui_key_view_tab_marquee(void);
input_seq* Get_ui_key_view_tab_screenshot(void);
input_seq* Get_ui_key_view_tab_title(void);
input_seq* Get_ui_key_view_tab_pcb(void);
input_seq* Get_ui_key_quit(void);


int GetUIJoyUp(int joycodeIndex);
void SetUIJoyUp(int joycodeIndex, int val);

int GetUIJoyDown(int joycodeIndex);
void SetUIJoyDown(int joycodeIndex, int val);

int GetUIJoyLeft(int joycodeIndex);
void SetUIJoyLeft(int joycodeIndex, int val);

int GetUIJoyRight(int joycodeIndex);
void SetUIJoyRight(int joycodeIndex, int val);

int GetUIJoyStart(int joycodeIndex);
void SetUIJoyStart(int joycodeIndex, int val);

int GetUIJoyPageUp(int joycodeIndex);
void SetUIJoyPageUp(int joycodeIndex, int val);

int GetUIJoyPageDown(int joycodeIndex);
void SetUIJoyPageDown(int joycodeIndex, int val);

int GetUIJoyHome(int joycodeIndex);
void SetUIJoyHome(int joycodeIndex, int val);

int GetUIJoyEnd(int joycodeIndex);
void SetUIJoyEnd(int joycodeIndex, int val);

int GetUIJoySSChange(int joycodeIndex);
void SetUIJoySSChange(int joycodeIndex, int val);

int GetUIJoyHistoryUp(int joycodeIndex);
void SetUIJoyHistoryUp(int joycodeIndex, int val);

int GetUIJoyHistoryDown(int joycodeIndex);
void SetUIJoyHistoryDown(int joycodeIndex, int val);

int GetUIJoyExec(int joycodeIndex);
void SetUIJoyExec(int joycodeIndex, int val);

WCHAR *GetExecCommand(void);
void SetExecCommand(WCHAR* cmd);

int GetExecWait(void);
void SetExecWait(int wait);

BOOL GetHideMouseOnStartup(void);
void SetHideMouseOnStartup(BOOL hide);

BOOL GetRunFullScreen(void);
void SetRunFullScreen(BOOL fullScreen);

void ColumnEncodeStringWithCount(const int *value, char *str, int count);
void ColumnDecodeStringWithCount(const char* str, int *value, int count);


/***************************************************************************
    MAME Plus! specific code
 ***************************************************************************/

#ifdef UNICODE
WCHAR *options_get_wstring(winui_options &opts, const char *name);
void options_set_wstring(winui_options &opts, const char *name, const WCHAR *value, int priority);
WCHAR *options_get_wstring(windows_options &opts, const char *name);
void options_set_wstring(windows_options &opts, const char *name, const WCHAR *value, int priority);
#endif /* UNICODE */

#ifdef STORY_DATAFILE
const WCHAR *GetStoryFileName(void);
void SetStoryFileName(const WCHAR *path);
#endif /* STORY_DATAFILE */

#ifdef USE_VIEW_PCBINFO
const WCHAR *GetPcbInfoDir(void);
void SetPcbInfoDir(const WCHAR *path);
#endif /* USE_VIEW_PCBINFO */

int GetLangcode(void);
void SetLangcode(int langcode);

BOOL UseLangList(void);
void SetUseLangList(BOOL is_use);

const WCHAR *GetLanguageDir(void);
void SetLanguageDir(const WCHAR *path);

const WCHAR *GetHiDir(void);
void SetHiDir(const WCHAR *path);

#ifdef USE_HISCORE
const WCHAR *GetHiscoreFile(void);
void SetHiscoreFile(const WCHAR *);
#endif /* USE_HISCORE */

#ifdef USE_IPS
const WCHAR *GetIPSDir(void);
void SetIPSDir(const WCHAR *path);
#endif /* USE_IPS */

#ifdef UI_COLOR_DISPLAY
const char *GetUIPaletteString(int n);
void SetUIPaletteString(int n, const char *s);
#endif /* UI_COLOR_DISPLAY */

BOOL FolderHasVector(const WCHAR *name);

void SetListBrokenColor(COLORREF uColor);
COLORREF GetListBrokenColor(void);

void SetUseBrokenIcon(BOOL use_broken_icon);
BOOL GetUseBrokenIcon(void);

#ifdef USE_SHOW_SPLASH_SCREEN
BOOL GetDisplaySplashScreen(void);
void SetDisplaySplashScreen(BOOL val);
#endif /* USE_SHOW_SPLASH_SCREEN */

#ifdef TREE_SHEET
BOOL GetShowTreeSheet(void);
void SetShowTreeSheet(BOOL val);
#endif /* TREE_SHEET */

#endif

