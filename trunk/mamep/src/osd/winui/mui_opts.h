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
#include "osd_cpu.h"
#include "options.h"
#include "inputseq.h" /* for input_seq definition */
#include <video.h> /* for MAX_SCREENS Definition*/

//#ifdef MESS
#include "optionsms.h"
//#endif

#define MAX_SYSTEM_BIOS		16
#define MAX_SYSTEM_BIOS_ENTRY	16
#define BIOS_DEFAULT		"default"

// Various levels of ini's we can edit.
typedef enum {
	OPTIONS_GLOBAL = 0,
	OPTIONS_VECTOR,
	OPTIONS_SOURCE,
	OPTIONS_PARENT,
	OPTIONS_GAME,
	OPTIONS_MAX
} OPTIONS_TYPE;


enum 
{
	COLUMN_GAMES = 0,
	//COLUMN_ORIENTATION,
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

#define FOLDER_OPTIONS	-2
#define GLOBAL_OPTIONS	-1

typedef struct
{
	int x, y, width, height;
} AREA;


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
	TAB_HISTORY,
#ifdef STORY_DATAFILE
	TAB_STORY,
#endif /* STORY_DATAFILE */

	MAX_TAB_TYPES,
	BACKGROUND,
	TAB_ALL,
#ifdef USE_IPS
	TAB_NONE,
	TAB_IPS
#else /* USE_IPS */
	TAB_NONE
#endif /* USE_IPS */
};
// Because we have added the Options after MAX_TAB_TYPES, we have to subtract 3 here
// (that's how many options we have after MAX_TAB_TYPES)
#define TAB_SUBTRACT 3

BOOL OptionsInit(void);
void OptionsExit(void);

#define OPTIONS_TYPE_GLOBAL		-1
#define OPTIONS_TYPE_FOLDER		-2

core_options *load_options(OPTIONS_TYPE opt_type, int game_num);
void save_options(OPTIONS_TYPE opt_type, core_options *opts, int game_num);

void AddOptions(core_options *opts, const options_entry *entrylist, BOOL is_global);
core_options *CreateGameOptions(int driver_index);

core_options * MameUISettings(void);
core_options * MameUIGlobal(void);

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

void SetGameCaption(BOOL caption);
BOOL GetGameCaption(void);

void SetBroadcast(BOOL broadcast);
BOOL GetBroadcast(void);

void SetRandomBackground(BOOL random_bg);
BOOL GetRandomBackground(void);

void SetSavedFolderPath(const char *path);
const char *GetSavedFolderPath(void);

void SetShowScreenShot(BOOL val);
BOOL GetShowScreenShot(void);

void SetShowFolderList(BOOL val);
BOOL GetShowFolderList(void);

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

#ifdef USE_VIEW_PCBINFO
const WCHAR *GetPcbDir(void);
void SetPcbDir(const WCHAR *path);
#endif /* USE_VIEW_PCBINFO */

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

const WCHAR *GetCheatFileName(void);
void SetCheatFileName(const WCHAR *path);

const WCHAR *GetHistoryFileName(void);
void SetHistoryFileName(const WCHAR *path);

const WCHAR *GetMAMEInfoFileName(void);
void SetMAMEInfoFileName(const WCHAR *path);

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
void SetExecCommand(WCHAR *cmd);

int GetExecWait(void);
void SetExecWait(int wait);

BOOL GetHideMouseOnStartup(void);
void SetHideMouseOnStartup(BOOL hide);

BOOL GetRunFullScreen(void);
void SetRunFullScreen(BOOL fullScreen);

void ColumnEncodeStringWithCount(const int *value, char *str, int count);
void ColumnDecodeStringWithCount(const char* str, int *value, int count);


/*----------------------------------------*/
// MAME Plus! specific code

#ifdef UNICODE
WCHAR *options_get_wstring(core_options *opts, const char *name);
void options_set_wstring(core_options *opts, const char *name, const WCHAR *value, int priority);
WCHAR *OptionsGetCommandLine(int driver_index, void (*override_callback)(core_options *opts, void *param), void *param);
#endif

core_options *get_core_options(void);

void set_core_snapshot_directory(const WCHAR *dir);
void set_core_input_directory(const WCHAR *dir);
void set_core_state_directory(const WCHAR *dir);
void set_core_state(const WCHAR *name);
void set_core_playback(const WCHAR *name);
void set_core_record(const WCHAR *name);
void set_core_mngwrite(const WCHAR *filename);
void set_core_wavwrite(const WCHAR *filename);
void set_core_localized_directory(const WCHAR *dir);

void set_core_history_filename(const WCHAR *filename);
#ifdef STORY_DATAFILE
void set_core_story_filename(const WCHAR *filename);
#endif /* STORY_DATAFILE */
void set_core_mameinfo_filename(const WCHAR *filename);

void set_core_bios(const char *bios); 

int GetLangcode(void);
void SetLangcode(int langcode);

const WCHAR *GetLocalizedDir(void);
void SetLocalizedDir(const WCHAR *path);

const WCHAR *GetTranslationDir(void);
void SetTranslationDir(const WCHAR *path);

const WCHAR *GetHiDir(void);
void SetHiDir(const WCHAR *path);

const WCHAR *GetHistoryFile(void);
void SetHistoryFile(const WCHAR *);

#ifdef STORY_DATAFILE
const WCHAR *GetStoryFile(void);
void SetStoryFile(const WCHAR *);

#endif /* STORY_DATAFILE */
const WCHAR *GetMAMEInfoFile(void);
void SetMAMEInfoFile(const WCHAR *);

#ifdef USE_HISCORE
const WCHAR *GetHiscoreFile(void);
void SetHiscoreFile(const WCHAR *);
#endif /* USE_HISCORE */

int GetSystemBiosDriver(int bios_index);

#ifdef UI_COLOR_PALETTE
const char *GetUIPaletteString(int n);
void SetUIPaletteString(int n, const char *s);
#endif /* UI_COLOR_PALETTE */

BOOL UseLangList(void);
void SetUseLangList(BOOL is_use);

BOOL GetShowFolder(int folder);
void SetShowFolder(int folder,BOOL show);

BOOL FolderHasVector(const WCHAR *name);

void SaveFolderFlags(const char *path, DWORD flags);
DWORD LoadFolderFlags(const char *path);

void SetListBrokenColor(COLORREF uColor);
COLORREF GetListBrokenColor(void);

void SetUseBrokenIcon(BOOL use_broken_icon);
BOOL GetUseBrokenIcon(void);

#ifdef IMAGE_MENU
int GetImageMenuStyle(void);
void SetImageMenuStyle(int style);
#endif /* IMAGE_MENU */

#ifdef USE_IPS
const WCHAR *GetPatchDir(void);
void SetPatchDir(const WCHAR *path);
#endif /* USE_IPS */

#ifdef USE_SHOW_SPLASH_SCREEN
void SetDisplaySplashScreen(BOOL val);
BOOL GetDisplaySplashScreen(void);
#endif /* USE_SHOW_SPLASH_SCREEN */

#ifdef TREE_SHEET
void SetShowTreeSheet(BOOL val);
BOOL GetShowTreeSheet(void);
#endif /* TREE_SHEET */

//mamep: fixme
core_options *options_get_mess_option(int driver_index);

#endif
