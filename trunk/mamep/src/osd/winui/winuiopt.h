/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef OPTIONS_H
#define OPTIONS_H

#include "osd_cpu.h"
#include "input.h" /* for input_seq definition */

//#ifdef MESS
#include "optionsms.h"
//#endif

#define MAX_SYSTEM_BIOS		16
#define MAX_SYSTEM_BIOS_ENTRY	16
#define BIOS_DEFAULT		"default"
#define MAX_GAMEDESC 256

#define GUIOPTION_LIST_MODE	"list_mode"
#define GUIOPTION_LIST_FONT	"list_font"
#define GUIOPTION_LIST_FONTFACE	"list_fontface"
#define GUIOPTION_FOLDER_FLAG	"folder_flag"


enum 
{
	COLUMN_GAMES = 0,
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


// can't be the same as the audit_verify_roms() results, listed in audit.h
enum
{
	UNKNOWN	= -1
};

enum
{
	SPLITTER_LEFT = 0,
	SPLITTER_RIGHT,
	SPLITTER_MAX
};

typedef struct
{
	int x, y, width, height;
} AREA;

typedef struct
{
	char *seq_string;	/* KEYCODE_LALT KEYCODE_A, etc... */
	input_seq is;		/* sequence definition in MAME's internal keycodes */
} KeySeq;

typedef struct
{
// CORE CONFIGURATION OPTIONS
#ifdef DRIVER_SWITCH
	char*	driver_config;
#endif /* DRIVER_SWITCH */

// CORE STATE/PLAYBACK OPTIONS
	WCHAR*	state;
	BOOL	autosave;
	WCHAR*	playback;
	WCHAR*	record;
	WCHAR*	mngwrite;
	WCHAR*	wavwrite;

// CORE PERFORMANCE OPTIONS
	BOOL	autoframeskip;
	int	frameskip;
	int	seconds_to_run;
	BOOL	throttle;
	BOOL	sleep;
	float	speed;
	BOOL	refreshspeed;

// CORE ROTATION OPTIONS
	BOOL	rotate;
	BOOL	ror;
	BOOL	rol;
	BOOL	autoror;
	BOOL	autorol;
	BOOL	flipx;
	BOOL	flipy;

// CORE ARTWORK OPTIONS
	BOOL	artwork_crop;
	BOOL	use_backdrops;
	BOOL	use_overlays;
	BOOL	use_bezels;

// CORE SCREEN OPTIONS
	float	brightness;
	float	contrast;      /* "1.0", 0.5, 2.0 */
	float	gamma;         /* "1.0", 0.5, 3.0 */
	float	pause_brightness;
#ifdef USE_SCALE_EFFECTS
	char*	scale_effect;
#endif /* USE_SCALE_EFFECTS */

// CORE VECTOR OPTIONS
	BOOL	antialias;
	float	beam;
	float	flicker;

// CORE SOUND OPTIONS
	BOOL	sound;
	int	samplerate;
	BOOL	samples;
	int	volume;
#ifdef USE_VOLUME_AUTO_ADJUST
	BOOL	volume_adjust;
#endif /* USE_VOLUME_AUTO_ADJUST */

// CORE INPUT OPTIONS
	char*	ctrlr;
	BOOL	mouse;
	BOOL	joystick;
	BOOL	lightgun;
	BOOL	multikeyboard;
	BOOL	multimouse;
	BOOL	steadykey;
	BOOL	offscreen_reload;
	char*	joystick_map;
	float	joy_deadzone;
	float	joy_saturation;

// CORE INPUT AUTOMATIC ENABLE OPTIONS
	char*	paddle_device;
	char*	adstick_device;
	char*	pedal_device;
	char*	dial_device;
	char*	trackball_device;
	char*	lightgun_device;
	char*	positional_device;
	char*	mouse_device;

// CORE DEBUGGING OPTIONS
	BOOL	log;
	BOOL	verbose;

// CORE MISC OPTIONS
	char*	bios;
	BOOL	cheat;
	BOOL	skip_gameinfo;
#ifdef USE_IPS
	WCHAR*	ips;
#endif /* USE_IPS */
	BOOL	confirm_quit;
#ifdef AUTO_PAUSE_PLAYBACK
	BOOL	auto_pause_playback;
#endif /* AUTO_PAUSE_PLAYBACK */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	int	m68k_core;
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef TRANS_UI
	BOOL	use_trans_ui;
	int	ui_transparency;
#endif /* TRANS_UI */

// WINDOWS DEBUGGING OPTIONS
	BOOL	oslog;

// WINDOWS PERFORMANCE OPTIONS
	int	priority;
	BOOL	multithreading;

// WINDOWS VIDEO OPTIONS
	char*	video;
	int	numscreens;
	BOOL	window;
	BOOL	maximize;
	BOOL	keepaspect;
	int	prescale;
	char*	effect;
	BOOL	waitvsync;
	BOOL	syncrefresh;

// DIRECTDRAW-SPECIFIC OPTIONS
	BOOL	hwstretch;

// DIRECT3D-SPECIFIC OPTIONS
	int	d3dversion;
	BOOL	filter;

// PER-WINDOW VIDEO OPTIONS
	char*	screen;
	char*	aspect;
	char*	resolution;
	char*	view;
	char*	screens[4];
	char*	aspects[4];
	char*	resolutions[4];
	char*	views[4];

// FULL SCREEN OPTIONS
	BOOL	triplebuffer;
	BOOL	switchres;
	float	full_screen_brightness;
	float	full_screen_contrast;
	float	full_screen_gamma;

// WINDOWS SOUND OPTIONS
	int	audio_latency;

// INPUT DEVICE OPTIONS
	BOOL	dual_lightgun;
#ifdef JOYSTICK_ID
	int	joyid1;
	int	joyid2;
	int	joyid3;
	int	joyid4;
	int	joyid5;
	int	joyid6;
	int	joyid7;
	int	joyid8;
#endif /* JOYSTICK_ID */

// MESS SPECIFIC OPTIONS
	char*	ramsize;
	BOOL	writeconfig;
	BOOL	skip_warnings;

// WINDOWS MESS SPECIFIC OPTIONS
	BOOL	newui;
	BOOL	natural;
} options_type;


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


/*----------------------------------------*/
void OptionsInit(void);
void OptionsExit(void);

core_options *get_core_options(void);
void set_core_input_directory(const WCHAR *dir);
void set_core_state_directory(const WCHAR *dir);
void set_core_input_directory(const WCHAR *dir);
void set_core_snapshot_directory(const WCHAR *dir);
void set_core_localized_directory(const WCHAR *dir);
void set_core_state(const WCHAR *name);
void set_core_playback(const WCHAR *name);
void set_core_record(const WCHAR *name);
void set_core_mngwrite(const WCHAR *filename);
void set_core_wavwrite(const WCHAR *filename);
void set_core_history_filename(const WCHAR *filename);
#ifdef STORY_DATAFILE
void set_core_story_filename(const WCHAR *filename);
#endif /* STORY_DATAFILE */
void set_core_mameinfo_filename(const WCHAR *filename);
void set_core_bios(const char *bios); 

#ifdef UNICODE
WCHAR *OptionsGetCommandLine(int driver_index, void (*override_callback)(void));
#endif

void FreeGameOptions(options_type *o);
void CopyGameOptions(const options_type *source, options_type *dest);

BOOL FolderHasVector(const WCHAR *name);
options_type* GetFolderOptions(const WCHAR *name);
options_type* GetDefaultOptions(void);
options_type* GetVectorOptions(void);
options_type* GetSourceOptions(int driver_index);
options_type* GetParentOptions(int driver_index);
options_type* GetGameOptions(int driver_index);

BOOL GetGameUsesDefaults(int driver_index);
void SetGameUsesDefaults(int driver_index, BOOL use_defaults);
BOOL GetFolderUsesDefaults(const WCHAR *name);
void SetFolderUsesDefaults(const WCHAR *name, BOOL use_defaults);

const WCHAR *GetUnifiedFolder(int driver_index);
int GetUnifiedDriver(const WCHAR *name);

int GetSystemBiosDriver(int bios_index);
const char *GetDefaultBios(int bios_index);
void SetDefaultBios(int bios_index, const char *value);

void SaveOptions(void);
void SaveDefaultOptions(void);
void SaveFolderOptions(const WCHAR *name);
void SaveGameOptions(int driver_index);

void ResetGUI(void);
void ResetGameDefaults(void);
void ResetAllGameOptions(void);
void ResetGameOptions(int driver_index);


/*----------------------------------------*/
char * GetVersionString(void);

const WCHAR * GetImageTabLongName(int tab_index);
const char * GetImageTabShortName(int tab_index);

void SetViewMode(int val);
int  GetViewMode(void);

void SetGameCheck(BOOL game_check);
BOOL GetGameCheck(void);

void SetJoyGUI(BOOL joygui);
BOOL GetJoyGUI(void);

void SetKeyGUI(BOOL keygui);
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

BOOL GetShowFolder(int folder);
void SetShowFolder(int folder, BOOL show);


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

void SetWindowArea(AREA *area);
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

void SetListFont(LOGFONTW *font);
void GetListFont(LOGFONTW *font);

void SaveFolderFlags(const char *path, DWORD flags);
DWORD LoadFolderFlags(const char *path);

void SetUseBrokenIcon(BOOL use_broken_icon);
BOOL GetUseBrokenIcon(void);

void SetListFontColor(COLORREF uColor);
COLORREF GetListFontColor(void);

void SetListCloneColor(COLORREF uColor);
COLORREF GetListCloneColor(void);

void SetListBrokenColor(COLORREF uColor);
COLORREF GetListBrokenColor(void);

int GetHistoryTab(void);
void SetHistoryTab(int tab, BOOL show);

int GetShowTab(int tab);
void SetShowTab(int tab, BOOL show);
BOOL AllowedToSetShowTab(int tab, BOOL show);

void SetSortColumn(int column);
int  GetSortColumn(void);

void SetSortReverse(BOOL reverse);
BOOL GetSortReverse(void);

#ifdef USE_SHOW_SPLASH_SCREEN
void SetDisplaySplashScreen(BOOL val);
BOOL GetDisplaySplashScreen(void);
#endif /* USE_SHOW_SPLASH_SCREEN */

#ifdef TREE_SHEET
void SetShowTreeSheet(BOOL val);
BOOL GetShowTreeSheet(void);
#endif /* TREE_SHEET */

int GetRomAuditResults(int driver_index);
void SetRomAuditResults(int driver_index, int audit_results);

int GetSampleAuditResults(int driver_index);
void SetSampleAuditResults(int driver_index, int audit_results);

void IncrementPlayCount(int driver_index);
int GetPlayCount(int driver_index);
void ResetPlayCount(int driver_index);

void IncrementPlayTime(int driver_index, int playtime);
int GetPlayTime(int driver_index);
void GetTextPlayTime(int driver_index, WCHAR *buf);
void ResetPlayTime(int driver_index);

WCHAR* GetExecCommand(void);
void SetExecCommand(WCHAR* cmd);

int GetExecWait(void);
void SetExecWait(int wait);

BOOL GetHideMouseOnStartup(void);
void SetHideMouseOnStartup(BOOL hide);

BOOL GetRunFullScreen(void);
void SetRunFullScreen(BOOL fullScreen);


/*----------------------------------------*/
const WCHAR* GetRomDirs(void);
void SetRomDirs(const WCHAR* paths);

const WCHAR* GetSampleDirs(void);
void  SetSampleDirs(const WCHAR* paths);

const WCHAR* GetIniDir(void);
void  SetIniDir(const WCHAR* path);

const WCHAR* GetCfgDir(void);
void SetCfgDir(const WCHAR* path);

const WCHAR* GetNvramDir(void);
void SetNvramDir(const WCHAR* path);

const WCHAR* GetMemcardDir(void);
void SetMemcardDir(const WCHAR* path);

const WCHAR* GetInpDir(void);
void SetInpDir(const WCHAR* path);

const WCHAR* GetHiDir(void);
void SetHiDir(const WCHAR* path);

const WCHAR* GetStateDir(void);
void SetStateDir(const WCHAR* path);

const WCHAR* GetArtDir(void);
void SetArtDir(const WCHAR* path);

const WCHAR* GetImgDirs(void);
void SetImgDirs(const WCHAR* path);

const WCHAR* GetDiffDir(void);
void SetDiffDir(const WCHAR* path);

const WCHAR* GetCtrlrDir(void);
void SetCtrlrDir(const WCHAR* path);

const WCHAR* GetTranslationDir(void);
void SetTranslationDir(const WCHAR* path);

const WCHAR* GetCommentDir(void);
void SetCommentDir(const WCHAR* path);

#ifdef USE_IPS
const WCHAR* GetPatchDir(void);
void SetPatchDir(const WCHAR* path);
#endif /* USE_IPS */

const WCHAR* GetLocalizedDir(void);
void SetLocalizedDir(const WCHAR* path);

const WCHAR* GetCheatFile(void);
void SetCheatFile(const WCHAR*);

const WCHAR* GetHistoryFile(void);
void SetHistoryFile(const WCHAR*);

#ifdef STORY_DATAFILE
const WCHAR* GetStoryFile(void);
void SetStoryFile(const WCHAR*);

#endif /* STORY_DATAFILE */
const WCHAR* GetMAMEInfoFile(void);
void SetMAMEInfoFile(const WCHAR*);

#ifdef USE_HISCORE
const WCHAR* GetHiscoreFile(void);
void SetHiscoreFile(const WCHAR*);
#endif /* USE_HISCORE */

#ifdef UI_COLOR_PALETTE
const char *GetUIPaletteString(int n);
void SetUIPaletteString(int n, const char *s);
#endif /* UI_COLOR_PALETTE */

#ifdef IMAGE_MENU
int GetImageMenuStyle(void);
void SetImageMenuStyle(int style);
#endif /* IMAGE_MENU */

int GetLangcode(void);
void SetLangcode(int langcode);

BOOL UseLangList(void);
void SetUseLangList(BOOL is_use);


/*----------------------------------------*/
const WCHAR* GetFlyerDirs(void);
void SetFlyerDirs(const WCHAR* path);

const WCHAR* GetCabinetDirs(void);
void SetCabinetDirs(const WCHAR* path);

const WCHAR* GetMarqueeDirs(void);
void SetMarqueeDirs(const WCHAR* path);

const WCHAR* GetTitlesDirs(void);
void SetTitlesDirs(const WCHAR* path);

const WCHAR* GetControlPanelDirs(void);
void SetControlPanelDirs(const WCHAR* path);

const WCHAR* GetIconsDirs(void);
void SetIconsDirs(const WCHAR* path);

const WCHAR* GetBgDir(void);
void SetBgDir(const WCHAR* path);

const WCHAR* GetFolderDir(void);
void SetFolderDir(const WCHAR* path);

#ifdef USE_VIEW_PCBINFO
const WCHAR* GetPcbinfoDir(void);
void SetPcbinfoDir(const WCHAR* path);
#endif /* USE_VIEW_PCBINFO */


/*----------------------------------------*/
// Keyboard control of ui
input_seq *Get_ui_key_up(void);
input_seq *Get_ui_key_down(void);
input_seq *Get_ui_key_left(void);
input_seq *Get_ui_key_right(void);
input_seq *Get_ui_key_start(void);
input_seq *Get_ui_key_pgup(void);
input_seq *Get_ui_key_pgdwn(void);
input_seq *Get_ui_key_home(void);
input_seq *Get_ui_key_end(void);
input_seq *Get_ui_key_ss_change(void);
input_seq *Get_ui_key_history_up(void);
input_seq *Get_ui_key_history_down(void);

input_seq *Get_ui_key_context_filters(void);
input_seq *Get_ui_key_select_random(void);
input_seq *Get_ui_key_game_audit(void);
input_seq *Get_ui_key_game_properties(void);
input_seq *Get_ui_key_help_contents(void);
input_seq *Get_ui_key_update_gamelist(void);
input_seq *Get_ui_key_view_folders(void);
input_seq *Get_ui_key_view_fullscreen(void);
input_seq *Get_ui_key_view_pagetab(void);
input_seq *Get_ui_key_view_picture_area(void);
input_seq *Get_ui_key_view_status(void);
input_seq *Get_ui_key_view_toolbars(void);

input_seq *Get_ui_key_view_tab_cabinet(void);
input_seq *Get_ui_key_view_tab_cpanel(void);
input_seq *Get_ui_key_view_tab_flyer(void);
input_seq *Get_ui_key_view_tab_history(void);
#ifdef STORY_DATAFILE
input_seq *Get_ui_key_view_tab_story(void);
#endif /* STORY_DATAFILE */
input_seq *Get_ui_key_view_tab_marquee(void);
input_seq *Get_ui_key_view_tab_screenshot(void);
input_seq *Get_ui_key_view_tab_title(void);
input_seq *Get_ui_key_quit(void);


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

#endif
