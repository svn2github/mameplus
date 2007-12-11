/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/
 
 /***************************************************************************

  options.c

  Stores global options and per-game options;

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <malloc.h>
#include <math.h>
#include <direct.h>
#include <ctype.h>
#include <io.h>

#include "MAME32.h"	// include this first
#include "driver.h"
#include "winmain.h"
#include "options.h"
#include "emuopts.h"
#include "bitmask.h"
#include "picker.h"
#include "dijoystick.h"
#include "treeview.h"
#include "m32util.h"
#include "splitters.h"
#include "DirectDraw.h"
#include "winuiopt.h"
#include "strconv.h"
#include "translate.h"
#include "directories.h"
#ifdef IMAGE_MENU
#include "imagemenu.h"
#endif /* IMAGE_MENU */

extern DWORD create_path_recursive(const TCHAR *path);

/***************************************************************************
    Internal structures
 ***************************************************************************/

typedef struct
{
	char *name;
	DWORD flags;
} f_flag_entry;

typedef struct
{
	f_flag_entry *entry;
	int           num;
} f_flag;


typedef struct
{
	core_options *dynamic_opt;

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

typedef struct
{
	char*    folder_current;
	BOOL     view;
	BOOL     show_folderlist;
	LPBITS   folder_hide;
	f_flag   folder_flag;
	BOOL     show_toolbar;
	BOOL     show_statusbar;
	BOOL     show_screenshot;
	BOOL     show_screenshottab;
	int      show_tab_flags;
#ifdef STORY_DATAFILE
	int      datafile_tab;
#else /* STORY_DATAFILE */
	int      history_tab;
#endif /* STORY_DATAFILE */
	char     *current_tab;
	BOOL     game_check;        /* Startup GameCheck */
	BOOL     joygui;
	BOOL     keygui;
	BOOL     broadcast;
	BOOL     random_bg;
	int      cycle_screenshot;
	BOOL     stretch_screenshot_larger;
	int      screenshot_bordersize;
	COLORREF screenshot_bordercolor;
	BOOL     inherit_filter;
	BOOL     offset_clones;
	BOOL	 game_caption;

	char     *default_game;
	int      column_widths[COLUMN_MAX];
	int      column_order[COLUMN_MAX];
	int      column_shown[COLUMN_MAX];
	int      sort_column;
	BOOL     sort_reverse;
#ifdef IMAGE_MENU
	int      imagemenu_style;
#endif /* IMAGE_MENU */
	int      window_x;
	int      window_y;
	int      window_width;
	int      window_height;
	UINT     window_state;
	int      splitters[4];		/* NPW 5-Feb-2003 - I don't like hard coding this, but I don't have a choice */
	COLORREF custom_color[16]; /* This is how many custom colors can be shown on the standard ColorPicker */
	BOOL     use_broken_icon;
	LOGFONTW list_logfont;
	COLORREF font_color;
	COLORREF clone_color;
	COLORREF broken_color;

	// Keyboard control of ui
	KeySeq   ui_key_up;
	KeySeq   ui_key_down;
	KeySeq   ui_key_left;
	KeySeq   ui_key_right;
	KeySeq   ui_key_start;
	KeySeq   ui_key_pgup;
	KeySeq   ui_key_pgdwn;
	KeySeq   ui_key_home;
	KeySeq   ui_key_end;
	KeySeq   ui_key_ss_change;
	KeySeq   ui_key_history_up;
	KeySeq   ui_key_history_down;

	KeySeq   ui_key_context_filters;	/* CTRL F */
	KeySeq   ui_key_select_random;		/* CTRL R */
	KeySeq   ui_key_game_audit;		/* ALT A */
	KeySeq   ui_key_game_properties;	/* ALT VK_RETURN */
	KeySeq   ui_key_help_contents;		/* VK_F1 */
	KeySeq   ui_key_update_gamelist;	/* VK_F5 */
	KeySeq   ui_key_view_folders;		/* ALT D */
	KeySeq   ui_key_view_fullscreen;	/* VK_F11 */
	KeySeq   ui_key_view_pagetab;		/* ALT B */
	KeySeq   ui_key_view_picture_area;	/* ALT P */
	KeySeq   ui_key_view_status;		/* ALT S */
	KeySeq   ui_key_view_toolbars;		/* ALT T */

	KeySeq   ui_key_view_tab_cabinet;	/* ALT 3 */
	KeySeq   ui_key_view_tab_cpanel;	/* ALT 6 */
	KeySeq   ui_key_view_tab_flyer;		/* ALT 2 */
	KeySeq   ui_key_view_tab_history;	/* ALT 7 */
#ifdef STORY_DATAFILE
	KeySeq   ui_key_view_tab_story;		/* ALT 8 */
#endif /* STORY_DATAFILE */
	KeySeq   ui_key_view_tab_marquee;	/* ALT 4 */
	KeySeq   ui_key_view_tab_screenshot;	/* ALT 1 */
	KeySeq   ui_key_view_tab_title;		/* ALT 5 */
	KeySeq   ui_key_quit;			/* ALT Q */

	// Joystick control of ui
	// array of 4 is joystick index, stick or button, etc.
	int      ui_joy_up[4];
	int      ui_joy_down[4];
	int      ui_joy_left[4];
	int      ui_joy_right[4];
	int      ui_joy_start[4];
	int      ui_joy_pgup[4];
	int      ui_joy_pgdwn[4];
	int      ui_joy_home[4];
	int      ui_joy_end[4];
	int      ui_joy_ss_change[4];
	int      ui_joy_history_up[4];
	int      ui_joy_history_down[4];
	int      ui_joy_exec[4];

	WCHAR*   exec_command;  // Command line to execute on ui_joy_exec   
	int      exec_wait;     // How long to wait before executing
	BOOL     hide_mouse;    // Should mouse cursor be hidden on startup?
	BOOL     full_screen;   // Should we fake fullscreen?

#ifdef USE_SHOW_SPLASH_SCREEN
	BOOL     display_splash_screen;
#endif /* USE_SHOW_SPLASH_SCREEN */

#ifdef TREE_SHEET
	BOOL     show_tree_sheet;
#endif /* TREE_SHEET */

	// PATH AND DIRECTORY OPTIONS
	WCHAR*	flyer_directory;
	WCHAR*	cabinet_directory;
	WCHAR*	marquee_directory;
	WCHAR*	title_directory;
	WCHAR*	cpanel_directory;
	WCHAR*	icon_directory;
	WCHAR*	bkground_directory;
	WCHAR*	folder_directory;
#ifdef USE_VIEW_PCBINFO
	WCHAR*	pcbinfo_directory;
#endif /* USE_VIEW_PCBINFO */

// CORE SEARCH PATH OPTIONS
	WCHAR*	rompath;
	WCHAR*	hashpath;
	WCHAR*	samplepath;
	WCHAR*	artpath;
	WCHAR*	ctrlrpath;
	WCHAR*	inipath;
	WCHAR*	fontpath;
	WCHAR*	translation_directory;
	WCHAR*	localized_directory;
#ifdef USE_IPS
	WCHAR*	ips_directory;
#endif /* USE_IPS */

// CORE OUTPUT DIRECTORY OPTIONS
	WCHAR*	cfg_directory;
	WCHAR*	nvram_directory;
	WCHAR*	memcard_directory;
	WCHAR*	input_directory;
	WCHAR*	state_directory;
	WCHAR*	snapshot_directory;
	WCHAR*	diff_directory;
	WCHAR*	comment_directory;
#ifdef USE_HISCORE
	WCHAR*	hiscore_directory;
#endif /* USE_HISCORE */

// CORE FILENAME OPTIONS
	WCHAR*	cheat_file;
	WCHAR*	history_file;
#ifdef STORY_DATAFILE
	WCHAR*	story_file;
#endif /* STORY_DATAFILE */
	WCHAR*	mameinfo_file;
#ifdef CMD_LIST
	WCHAR*	command_file;
#endif /* CMD_LIST */
#ifdef USE_HISCORE
	WCHAR*	hiscore_file;
#endif /* USE_HISCORE */

// CORE PALETTE OPTIONS
#ifdef UI_COLOR_DISPLAY
	char*    font_blank;
	char*    font_normal;
	char*    font_special;
	char*    system_background;
	char*    button_red;
	char*    button_yellow;
	char*    button_green;
	char*    button_blue;
	char*    button_purple;
	char*    button_pink;
	char*    button_aqua;
	char*    button_silver;
	char*    button_navy;
	char*    button_lime;
	char*    cursor;
#endif /* UI_COLOR_DISPLAY */

// CORE LANGUAGE OPTIONS
	int      langcode;
	BOOL     use_lang_list;
} settings_type; /* global settings for the UI only */

struct _backup
{
	settings_type settings;
	options_type  global;
};

// per-game data we store, not to pass to mame, but for our own use.
typedef struct
{
	int play_count;
	int play_time;
	int rom_audit_results;
	int samples_audit_results;
	BOOL options_loaded; // whether or not we've loaded the game options yet
	BOOL use_default; // whether or not we should just use default options

} driver_variables_type;

typedef struct
{
	const WCHAR *name;
	options_type *option;
	driver_variables_type *variable;
	BOOL need_vector_config;
} alt_options_type;

struct _joycodes
{
	const char *name;
	int value;
};


/***************************************************************************
    Internal function prototypes
 ***************************************************************************/

static void set_core_rom_directory(const WCHAR *dir);
static void set_core_sample_directory(const WCHAR *dir);
static void set_core_image_directory(const WCHAR *dir);
//#ifdef MESS
static void set_core_hash_directory(const WCHAR *dir);
//#endif
static void set_core_translation_directory(const WCHAR *dir);

static int   regist_alt_option(const WCHAR *name);
static int   bsearch_alt_option(const WCHAR *name);
static void  build_default_bios(void);
static void  build_alt_options(void);

static void  generate_default_ctrlr(void);
static void  generate_default_dirs(void);

static core_options *options_create_entry_cli(void);
static core_options *options_create_entry_winui(void);

static void  options_free_string_core(settings_type *s);
static void  options_free_string_driver(options_type *p);
static void  options_free_string_winui(settings_type *p);

static void  options_duplicate_settings(const settings_type *source, settings_type *dest);
static void  options_duplicate_driver(const options_type *source, options_type *dest);

static BOOL  options_compare_driver(const options_type *p1, const options_type *p2);

static int   options_load_default_config(void);
static int   options_load_driver_config(int driver_index);
static int   options_load_alt_config(alt_options_type *alt_option);
static int   options_load_winui_config(void);

static int   options_save_default_config(void);
static int   options_save_driver_config(int driver_index);
static int   options_save_alt_config(alt_options_type *alt_option);
static int   options_save_winui_config(void);

static void  validate_driver_option(options_type *opt);

static void  options_get_core(core_options *opt, settings_type *p);
static void  options_get_driver(core_options *opt, options_type *p);
static void  options_get_winui(settings_type *p);

static void  options_set_core(core_options *opts, const settings_type *p);
static void  options_set_driver(core_options *opts, const options_type *p);
static void  options_set_winui(const settings_type *p);

static options_type *update_driver_use_default(int driver_index);
static options_type *update_alt_use_default(alt_options_type *alt_option);

static void  CopySettings(const settings_type *source, settings_type *dest);
static void  FreeSettings(settings_type *p);

#if 0
static void  SaveGlobalOptions(void);
#endif
static void  SaveAltOptions(alt_options_type *alt_option);
static void  LoadOptions(void);
static void  LoadGameOptions(int driver_index);
static void  LoadAltOptions(alt_options_type *alt_option);

static BOOL  IsOptionEqual(options_type *o1, options_type *o2);

static void  set_folder_flag(f_flag *flag, const char *folderPath, DWORD dwFlags);
static void  free_folder_flag(f_flag *flag);


void FreeGameOptions(options_type *o);
void CopyGameOptions(const options_type *source, options_type *dest);

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


/***************************************************************************
    Internal defines
 ***************************************************************************/

#define FRAMESKIP_LEVELS	12
#define MAX_WINDOWS		4

#define ALLOC_FOLDERFLAG	8
#define ALLOC_FOLDERS		100

#define CSV_ARRAY_MAX		256

#define TEXT_WINUI_INI TEXT(MAME32NAME) TEXT("ui.ini")
#define TEXT_MAME_INI TEXT(MAMENAME) TEXT(".ini")


/***************************************************************************
    Internal variables
 ***************************************************************************/

static core_options *options_cli;	// base core and driver options
static core_options *options_winui;	// only GUI related
static core_options *options_ref;	// temp to diff options

static settings_type settings;

static struct _backup backup;

static options_type global; // Global 'default' options
static options_type *driver_options;  // Array of Game specific options
static driver_variables_type *driver_variables;  // Array of driver specific extra data

static int  num_alt_options = 0;
static int alt_options_len = 700;
alt_options_type *alt_options;  // Array of Folder specific options

// default bios setting
static int default_bios[MAX_SYSTEM_BIOS];

/* Global UI options */
static int  num_drivers = 0;

// Screen shot Page tab control text
// these must match the order of the options flags in options.h
// (TAB_...)
const WCHAR* image_tabs_long_name[MAX_TAB_TYPES] =
{
	TEXT("Snapshot"),
	TEXT("Flyer"),
	TEXT("Cabinet"),
	TEXT("Marquee"),
	TEXT("Title"),
	TEXT("Control Panel"),
#ifdef STORY_DATAFILE
	TEXT("History"),
	TEXT("Story")
#else /* STORY_DATAFILE */
	TEXT("History")
#endif /* STORY_DATAFILE */
};

const char* image_tabs_short_name[MAX_TAB_TYPES] =
{
	"snapshot",
	"flyer",
	"cabinet",
	"marquee",
	"title",
	"cpanel",
#ifdef STORY_DATAFILE
	"history",
	"story"
#else /* STORY_DATAFILE */
	"history"
#endif /* STORY_DATAFILE */
};

static const char *view_modes[VIEW_MAX] = 
{
	"Large Icons",
	"Small Icons",
	"List",
	"Details",
	"Grouped"
};


#define DEFINE_JOYCODE_START(name)	static struct _joycodes name[] = {
#define DEFINE_JOYCODE(name)	{ #name, name },
#define DEFINE_JOYCODE_END	{ NULL, 0 } };

DEFINE_JOYCODE_START(joycode_axis)
	DEFINE_JOYCODE(JOYCODE_STICK_BTN)
	DEFINE_JOYCODE(JOYCODE_STICK_AXIS)
	DEFINE_JOYCODE(JOYCODE_STICK_POV)
DEFINE_JOYCODE_END

DEFINE_JOYCODE_START(joycode_dir)
	DEFINE_JOYCODE(JOYCODE_DIR_BTN)
	DEFINE_JOYCODE(JOYCODE_DIR_NEG)
	DEFINE_JOYCODE(JOYCODE_DIR_POS)
DEFINE_JOYCODE_END


#ifdef UI_COLOR_PALETTE
struct ui_palette_assign
{
	int code;
	char **data;
};

static struct ui_palette_assign ui_palette_tbl[] =
{
	{ FONT_COLOR_BLANK,  &settings.font_blank },
	{ FONT_COLOR_NORMAL,  &settings.font_normal },
	{ FONT_COLOR_SPECIAL,  &settings.font_special },
	{ SYSTEM_COLOR_BACKGROUND,  &settings.system_background },
	{ BUTTON_COLOR_RED,  &settings.button_red },
	{ BUTTON_COLOR_YELLOW,  &settings.button_yellow },
	{ BUTTON_COLOR_GREEN,  &settings.button_green },
	{ BUTTON_COLOR_BLUE,  &settings.button_blue },
	{ BUTTON_COLOR_PURPLE,  &settings.button_purple },
	{ BUTTON_COLOR_PINK,  &settings.button_pink },
	{ BUTTON_COLOR_AQUA,  &settings.button_aqua },
	{ BUTTON_COLOR_SILVER,  &settings.button_silver },
	{ BUTTON_COLOR_NAVY,  &settings.button_navy },
	{ BUTTON_COLOR_LIME,  &settings.button_lime },
	{ CURSOR_COLOR,  &settings.cursor },
	{ MAX_COLORTABLE, NULL }
};
#endif /* UI_COLOR_PALETTE */


static WCHAR reload_config_msg[] =
	TEXT_MAME32NAME
	TEXT(" has changed *.ini file directory.\n\n")
	TEXT("Would you like to migrate old configurations to the new directory?");


/***************************************************************************
    Wrapper functions
 ***************************************************************************/

core_options *CreateGameOptions(int driver_index)
{
	core_options *opts = options_create_entry_cli();

	if (driver_index >= 0)
		MessSetupGameOptions(opts, driver_index);

	return opts;
}

core_options *load_options(OPTIONS_TYPE opt_type, int game_num)
{
	core_options *opts;
	options_type *o;

	switch (opt_type)
	{
	case OPTIONS_GLOBAL:
		opts = CreateGameOptions(OPTIONS_TYPE_GLOBAL);
		o = GetDefaultOptions();
		break;

	case OPTIONS_VECTOR:
		opts = CreateGameOptions(OPTIONS_TYPE_FOLDER);
		o = GetVectorOptions();
		break;

	case OPTIONS_SOURCE:
		opts = CreateGameOptions(OPTIONS_TYPE_FOLDER);
		o = GetSourceOptions(game_num);
		break;

	case OPTIONS_PARENT:
		opts = CreateGameOptions(game_num);
		o = GetParentOptions(game_num);
		break;

	case OPTIONS_GAME:
		opts = CreateGameOptions(game_num);
		o = GetGameOptions(game_num);
		break;

	default:
		dprintf("load_options(): unknown opt_type: %d", opt_type);
		exit(1);
	}

	if (o->dynamic_opt)
		options_copy(opts, o->dynamic_opt);

	options_set_core(opts, &settings);
	options_set_driver(opts, o);

	return opts;
}

void save_options(OPTIONS_TYPE opt_type, core_options *opts, int game_num)
{
	int alt_index = -1;
	options_type *o;

	switch (opt_type)
	{
	case OPTIONS_GLOBAL:
		o = GetDefaultOptions();
		break;

	case OPTIONS_VECTOR:
		alt_index = bsearch_alt_option(TEXT("Vector"));
		o = alt_options[alt_index].option;
		break;

	case OPTIONS_SOURCE:
		alt_index = bsearch_alt_option(GetDriverFilename(game_num));
		o = alt_options[alt_index].option;
		break;

	case OPTIONS_PARENT:
		o = GetParentOptions(game_num);
		break;

	case OPTIONS_GAME:
		o = GetGameOptions(game_num);
		break;

	default:
		dprintf("save_options(): unknown opt_type: %d", opt_type);
		exit(1);
	}

	if (opt_type != OPTIONS_GLOBAL && opts == NULL)
		opts = load_options(opt_type - 1, game_num);

	options_get_driver(opts, o);

	if (o->dynamic_opt)
		options_copy(o->dynamic_opt, opts);

	if (alt_index != -1)
		update_alt_use_default(&alt_options[alt_index]);
	else if (opt_type != OPTIONS_GLOBAL)
		update_driver_use_default(game_num);
}

/***************************************************************************
    External functions  
 ***************************************************************************/

void OptionsInit()
{
	driver_variables_type default_variables;
	int i;

	memset(&settings, 0, sizeof (settings));
	memset(&global, 0, sizeof (global));
	memset(&backup, 0, sizeof (backup));

	num_drivers = GetNumGames();

	options_cli = options_create_entry_cli();
	options_get_core(options_cli, &settings);
	options_get_driver(options_cli, &global);
	lang_set_langcode(options_cli, UI_LANG_EN_US);

	options_ref = options_create_entry_cli();

	default_variables.play_count  = 0;
	default_variables.play_time = 0;
	default_variables.rom_audit_results = UNKNOWN;
	default_variables.samples_audit_results = UNKNOWN;
	default_variables.options_loaded = FALSE;
	default_variables.use_default = TRUE;

	/* This allocation should be checked */
	driver_options = (options_type *)malloc(num_drivers * sizeof (*driver_options));
	driver_variables = (driver_variables_type *)malloc(num_drivers * sizeof (*driver_variables));

	memset(driver_options, 0, num_drivers * sizeof (*driver_options));
	for (i = 0; i < num_drivers; i++)
		driver_variables[i] = default_variables;

	build_alt_options();
	build_default_bios();

#ifdef HANDLE_MESS_OPTIONS
	for (i = 0; i < num_drivers; i++)
	{
		if (DriverIsConsole(i))
			driver_options[i].dynamic_opt = CreateGameOptions(i);
	}
#endif /* HANDLE_MESS_OPTIONS */

	options_winui = options_create_entry_winui();
	options_get_winui(&settings);

	// Create Backup
	CopySettings(&settings, &backup.settings);
	CopyGameOptions(&global, &backup.global);

	LoadOptions();

	// apply default font if needed
	if (settings.list_logfont.lfFaceName[0] == '\0')
		GetTranslatedFont(&settings.list_logfont);

	// have our mame core (file code) know about our rom path
	// this leaks a little, but the win32 file core writes to this string
	set_core_rom_directory(settings.rompath);
	set_core_image_directory(settings.rompath);
	set_core_sample_directory(settings.samplepath);
//#ifdef MESS
	set_core_hash_directory(settings.hashpath);
//#endif

	generate_default_dirs();
	generate_default_ctrlr();
}

void OptionsExit(void)
{
	int i;

	for (i = 0; i < num_drivers; i++)
		FreeGameOptions(&driver_options[i]);

	for (i = 0; i < num_alt_options; i++)
		FreeGameOptions(alt_options[i].option);

	for (i = 0; i < num_drivers; i++)
		if (driver_options[i].dynamic_opt)
			options_free(driver_options[i].dynamic_opt);

	free(driver_options);
	free(driver_variables);
	free(alt_options);

	FreeGameOptions(&global);
	FreeGameOptions(&backup.global);

	FreeSettings(&settings);
	FreeSettings(&backup.settings);

	options_free(options_cli);
	options_free(options_winui);
	options_free(options_ref);
}


/* -- */
core_options *get_core_options(void)
{
	return options_cli;
}

core_options *get_winui_options(void)
{
	return options_winui;
}

void set_core_input_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, OPTION_INPUT_DIRECTORY, dir, OPTION_PRIORITY_CMDLINE);
}

void set_core_state_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, OPTION_STATE_DIRECTORY, dir, OPTION_PRIORITY_CMDLINE);
}

void set_core_snapshot_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, OPTION_SNAPSHOT_DIRECTORY, dir, OPTION_PRIORITY_CMDLINE);
}

void set_core_localized_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, OPTION_LOCALIZED_DIRECTORY, dir, OPTION_PRIORITY_CMDLINE);
}

void set_core_state(const WCHAR *name)
{
	options_set_wstring(options_cli, OPTION_STATE, name, OPTION_PRIORITY_CMDLINE);
}

void set_core_playback(const WCHAR *name)
{
	options_set_wstring(options_cli, OPTION_PLAYBACK, name, OPTION_PRIORITY_CMDLINE);
}

void set_core_record(const WCHAR *name)
{
	options_set_wstring(options_cli, OPTION_RECORD, name, OPTION_PRIORITY_CMDLINE);
}

void set_core_mngwrite(const WCHAR *filename)
{
	options_set_wstring(options_cli, OPTION_MNGWRITE, filename, OPTION_PRIORITY_CMDLINE);
}

void set_core_wavwrite(const WCHAR *filename)
{
	options_set_wstring(options_cli, OPTION_WAVWRITE, filename, OPTION_PRIORITY_CMDLINE);
}

void set_core_history_filename(const WCHAR *filename)
{
	options_set_wstring(options_cli, OPTION_HISTORY_FILE, filename, OPTION_PRIORITY_CMDLINE);
}

#ifdef STORY_DATAFILE
void set_core_story_filename(const WCHAR *filename)
{
	options_set_wstring(options_cli, OPTION_STORY_FILE, filename, OPTION_PRIORITY_CMDLINE);
}
#endif /* STORY_DATAFILE */

void set_core_mameinfo_filename(const WCHAR *filename)
{
	options_set_wstring(options_cli, OPTION_MAMEINFO_FILE, filename, OPTION_PRIORITY_CMDLINE);
}

void set_core_bios(const char *bios)
{
	options_set_string(options_cli, OPTION_BIOS, bios, OPTION_PRIORITY_CMDLINE);
}


/* -- */
static void set_core_rom_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, SEARCHPATH_ROM, dir, OPTION_PRIORITY_CMDLINE);
}

static void set_core_image_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, SEARCHPATH_IMAGE, dir, OPTION_PRIORITY_CMDLINE);
}

static void set_core_sample_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, SEARCHPATH_SAMPLE, dir, OPTION_PRIORITY_CMDLINE);
}

//#ifdef MESS
static void set_core_hash_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, SEARCHPATH_HASH, dir, OPTION_PRIORITY_CMDLINE);
}
//#endif

static void set_core_translation_directory(const WCHAR *dir)
{
	options_set_wstring(options_cli, SEARCHPATH_TRANSLATION, dir, OPTION_PRIORITY_CMDLINE);
}


/* -- */
static void CopySettings(const settings_type *source, settings_type *dest)
{
	options_duplicate_settings(source, dest);
}

static void FreeSettings(settings_type *p)
{
	options_free_string_winui(p);
	options_free_string_core(p);
}

void CopyGameOptions(const options_type *source, options_type *dest)
{
	options_duplicate_driver(source, dest);
}

void FreeGameOptions(options_type *o)
{
	options_free_string_driver(o);
}

static BOOL IsOptionEqual(options_type *o1, options_type *o2)
{
	options_type opt1, opt2;

	validate_driver_option(o1);
	validate_driver_option(o2);

	opt1 = *o1;
	opt2 = *o2;

	if (options_compare_driver(&opt1, &opt2))
		return FALSE;

	return TRUE;
}

BOOL GetGameUsesDefaults(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	return driver_variables[driver_index].use_default;
}

BOOL GetFolderUsesDefaults(const WCHAR *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return alt_options[alt_index].variable->use_default;
}

void SetGameUsesDefaults(int driver_index, BOOL use_defaults)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	driver_variables[driver_index].use_default = use_defaults;
}

void SetFolderUsesDefaults(const WCHAR *name, BOOL use_defaults)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	alt_options[alt_index].variable->use_default = use_defaults;
}

static options_type * GetAltOptions(alt_options_type *alt_option)
{
	if (alt_option->variable->use_default)
	{
		options_type *opt = GetDefaultOptions();
		char *bios = NULL;

#ifdef USE_IPS
		// HACK: DO NOT INHERIT IPS CONFIGURATION
		WCHAR *ips = alt_option->option->ips;

		alt_option->option->ips = NULL;
#endif /* USE_IPS */

		// if bios has been loaded, save it
		if (alt_option->option->bios)
		{
			bios = alt_option->option->bios;
			alt_option->option->bios = NULL;
		}

		// try vector.ini
		if (alt_option->need_vector_config)
			opt = GetVectorOptions();

		// free strings what will be never used now
		FreeGameOptions(alt_option->option);

		CopyGameOptions(opt, alt_option->option);

		// DO NOT OVERRIDE bios by default setting
		if (bios)
			alt_option->option->bios = bios;

#ifdef USE_IPS
		alt_option->option->ips = ips;
#endif /* USE_IPS */
	}

	if (alt_option->variable->options_loaded == FALSE)
		LoadAltOptions(alt_option);

	return alt_option->option;
}

BOOL FolderHasVector(const WCHAR *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return alt_options[alt_index].need_vector_config;
}

options_type * GetFolderOptions(const WCHAR *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return GetAltOptions(&alt_options[alt_index]);
}

options_type * GetDefaultOptions(void)
{
	return &global;
}

options_type* GetVectorOptions(void)
{
	return GetFolderOptions(TEXT("Vector"));
}

options_type* GetSourceOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	return GetFolderOptions(GetDriverFilename(driver_index));
}

options_type* GetParentOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	if (DriverIsClone(driver_index))
		return GetGameOptions(DriverParentIndex(driver_index));

	if (!DriverIsBios(driver_index) && DriverHasOptionalBios(driver_index))
		return GetGameOptions(DriverBiosIndex(driver_index));

	return GetSourceOptions(driver_index);
}

options_type * GetGameOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	if (driver_variables[driver_index].use_default)
	{
		options_type *opt = GetParentOptions(driver_index);
		char *bios = NULL;

#ifdef USE_IPS
		// HACK: DO NOT INHERIT IPS CONFIGURATION
		WCHAR *ips = driver_options[driver_index].ips;

		driver_options[driver_index].ips = NULL;
#endif /* USE_IPS */

		// save default bios setting if driver is bios
		if (DriverIsBios(driver_index) && driver_options[driver_index].bios)
		{
			bios = driver_options[driver_index].bios;
			driver_options[driver_index].bios = NULL;
		}

		// DO NOT OVERRIDE if driver name is same as parent
		if (opt != &driver_options[driver_index])
		{
			// free strings what will be never used now
			FreeGameOptions(&driver_options[driver_index]);

			CopyGameOptions(opt, &driver_options[driver_index]);
		}

		// DO NOT OVERRIDE bios by default setting
		if (bios)
			driver_options[driver_index].bios = bios;

#ifdef USE_IPS
		driver_options[driver_index].ips = ips;
#endif /* USE_IPS */
	}

	if (driver_variables[driver_index].options_loaded == FALSE)
		LoadGameOptions(driver_index);

	return &driver_options[driver_index];
}

int GetSystemBiosDriver(int bios_index)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	return default_bios[bios_index];
}

const char *GetDefaultBios(int bios_index)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	if (default_bios[bios_index] != -1)
	{
		options_type *opt = GetGameOptions(default_bios[bios_index]);

		return opt->bios;
	}

	return NULL;
}

void SetDefaultBios(int bios_index, const char *value)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	if (default_bios[bios_index] != -1)
	{
		options_type *opt = GetGameOptions(default_bios[bios_index]);

		FreeIfAllocated(&opt->bios);
		opt->bios = mame_strdup(value);
	}
}

void ResetGUI(void)
{
	FreeSettings(&settings);
	CopySettings(&backup.settings, &settings);
	SetLangcode(settings.langcode);
	SetUseLangList(UseLangList());
}

int GetLangcode(void)
{
	return settings.langcode;
}

void SetLangcode(int langcode)
{
	settings.langcode = langcode;

	/* apply to emulator core for datafile.c */
	lang_set_langcode(get_core_options(), langcode);

	/* apply for osd core functions */
	set_osdcore_acp(ui_lang_info[langcode].codepage);

	InitTranslator(langcode);
}

BOOL UseLangList(void)
{
	return settings.use_lang_list;
}

void SetUseLangList(BOOL is_use)
{
	settings.use_lang_list = is_use;

	/* apply to emulator core for datafile.c */
	lang_message_enable(UI_MSG_LIST, is_use);
	lang_message_enable(UI_MSG_MANUFACTURE, is_use);
}

const WCHAR * GetImageTabLongName(int tab_index)
{
	return image_tabs_long_name[tab_index];
}

const char * GetImageTabShortName(int tab_index)
{
	return image_tabs_short_name[tab_index];
}

#ifdef UI_COLOR_PALETTE
const char *GetUIPaletteString(int n)
{
	int i;

	for (i = 0; ui_palette_tbl[i].data; i++)
		if (ui_palette_tbl[i].code == n)
			return *ui_palette_tbl[i].data;

	return NULL;
}

void SetUIPaletteString(int n, const char *s)
{
	int i;

	for (i = 0; ui_palette_tbl[i].data; i++)
		if (ui_palette_tbl[i].code == n)
		{
			FreeIfAllocated(ui_palette_tbl[i].data);
			*ui_palette_tbl[i].data = mame_strdup(s);
		}
}
#endif /* UI_COLOR_PALETTE */

void SetViewMode(int val)
{
	settings.view = val;
}

int GetViewMode(void)
{
	return settings.view;
}

void SetGameCheck(BOOL game_check)
{
	settings.game_check = game_check;
}

BOOL GetGameCheck(void)
{
	return settings.game_check;
}

void SetJoyGUI(BOOL joygui)
{
	settings.joygui = joygui;
}

BOOL GetJoyGUI(void)
{
	return settings.joygui;
}

void SetKeyGUI(BOOL keygui)
{
	settings.keygui = keygui;
}

BOOL GetKeyGUI(void)
{
	return settings.keygui;
}

void SetCycleScreenshot(int cycle_screenshot)
{
	settings.cycle_screenshot = cycle_screenshot;
}

int GetCycleScreenshot(void)
{
	return settings.cycle_screenshot;
}

void SetStretchScreenShotLarger(BOOL stretch)
{
	settings.stretch_screenshot_larger = stretch;
}

BOOL GetStretchScreenShotLarger(void)
{
	return settings.stretch_screenshot_larger;
}

void SetScreenshotBorderSize(int size)
{
	settings.screenshot_bordersize = size;
}

int GetScreenshotBorderSize(void)
{
	return settings.screenshot_bordersize;
}

void SetScreenshotBorderColor(COLORREF uColor)
{
	if (settings.screenshot_bordercolor == GetSysColor(COLOR_3DFACE))
		settings.screenshot_bordercolor = (COLORREF)-1;
	else
		settings.screenshot_bordercolor = uColor;
}

COLORREF GetScreenshotBorderColor(void)
{
	if (settings.screenshot_bordercolor == (COLORREF)-1)
		return (GetSysColor(COLOR_3DFACE));

	return settings.screenshot_bordercolor;
}

void SetFilterInherit(BOOL inherit)
{
	settings.inherit_filter = inherit;
}

BOOL GetFilterInherit(void)
{
	return settings.inherit_filter;
}

void SetOffsetClones(BOOL offset)
{
	settings.offset_clones = offset;
}

BOOL GetOffsetClones(void)
{
	return settings.offset_clones;
}

void SetGameCaption(BOOL caption)
{
	settings.game_caption = caption;
}

BOOL GetGameCaption(void)
{
	return settings.game_caption;
}

void SetBroadcast(BOOL broadcast)
{
	settings.broadcast = broadcast;
}

BOOL GetBroadcast(void)
{
	return settings.broadcast;
}

void SetRandomBackground(BOOL random_bg)
{
	settings.random_bg = random_bg;
}

BOOL GetRandomBackground(void)
{
	return settings.random_bg;
}

void SetSavedFolderPath(const char *path)
{
	FreeIfAllocated(&settings.folder_current);
	settings.folder_current = mame_strdup(path);
}

const char *GetSavedFolderPath(void)
{
	return settings.folder_current;
}

void SetShowScreenShot(BOOL val)
{
	settings.show_screenshot = val;
}

BOOL GetShowScreenShot(void)
{
	return settings.show_screenshot;
}

void SetShowFolderList(BOOL val)
{
	settings.show_folderlist = val;
}

BOOL GetShowFolderList(void)
{
	return settings.show_folderlist;
}

BOOL GetShowFolder(int folder)
{
	if (settings.folder_hide == NULL)
		return TRUE;

	return !TestBit(settings.folder_hide, folder);
}

void SetShowFolder(int folder, BOOL show)
{
	if (!show)
	{
		if (settings.folder_hide == NULL)
		{
			settings.folder_hide = NewBits(MAX_FOLDERS);
			SetAllBits(settings.folder_hide, FALSE);
		}

		SetBit(settings.folder_hide, folder);
	}
	else
	{
		if (settings.folder_hide)
			ClearBit(settings.folder_hide, folder);
	}
}

void SetShowStatusBar(BOOL val)
{
	settings.show_statusbar = val;
}

BOOL GetShowStatusBar(void)
{
	return settings.show_statusbar;
}

void SetShowTabCtrl(BOOL val)
{
	settings.show_screenshottab = val;
}

BOOL GetShowTabCtrl(void)
{
	return settings.show_screenshottab;
}

void SetShowToolBar(BOOL val)
{
	settings.show_toolbar = val;
}

BOOL GetShowToolBar(void)
{
	return settings.show_toolbar;
}

void SetCurrentTab(const char *shortname)
{
	FreeIfAllocated(&settings.current_tab);
	if (shortname != NULL)
		settings.current_tab = mame_strdup(shortname);
}

const char *GetCurrentTab(void)
{
	return settings.current_tab;
}

void SetDefaultGame(const char *name)
{
	FreeIfAllocated(&settings.default_game);

	if (name != NULL)
		settings.default_game = mame_strdup(name);
}

const char *GetDefaultGame(void)
{
	return settings.default_game;
}

void SetWindowArea(AREA *area)
{
	settings.window_x = area->x;
	settings.window_y = area->y;
	settings.window_width = area->width;
	settings.window_height = area->height;
}

void GetWindowArea(AREA *area)
{
	area->x = settings.window_x;
	area->y = settings.window_y;
	area->width = settings.window_width;
	area->height = settings.window_height;
}

void SetWindowState(UINT state)
{
	settings.window_state = state;
}

UINT GetWindowState(void)
{
	return settings.window_state;
}

void SetCustomColor(int iIndex, COLORREF uColor)
{
	settings.custom_color[iIndex] = uColor;
}

COLORREF GetCustomColor(int iIndex)
{
	if (settings.custom_color[iIndex] == (COLORREF)-1)
		return (COLORREF)RGB(0,0,0);

	return settings.custom_color[iIndex];
}

void SetUseBrokenIcon(BOOL use_broken_icon)
{
	settings.use_broken_icon = use_broken_icon;
}

BOOL GetUseBrokenIcon(void)
{
	return settings.use_broken_icon;
}

void SetListFont(LOGFONTW *font)
{
	settings.list_logfont = *font;
}

void GetListFont(LOGFONTW *font)
{
	*font = settings.list_logfont;
}

void SetListFontColor(COLORREF uColor)
{
	if (settings.font_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.font_color = (COLORREF)-1;
	else
		settings.font_color = uColor;
}

COLORREF GetListFontColor(void)
{
	if (settings.font_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.font_color;
}

void SetListCloneColor(COLORREF uColor)
{
	if (settings.clone_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.clone_color = (COLORREF)-1;
	else
		settings.clone_color = uColor;
}

COLORREF GetListCloneColor(void)
{
	if (settings.clone_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.clone_color;

}

void SetListBrokenColor(COLORREF uColor)
{
	if (settings.broken_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.broken_color = (COLORREF)-1;
	else
		settings.broken_color = uColor;
}

COLORREF GetListBrokenColor(void)
{
	if (settings.broken_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.broken_color;

}

int GetShowTab(int tab)
{
	return (settings.show_tab_flags & (1 << tab)) != 0;
}

void SetShowTab(int tab, BOOL show)
{
	if (show)
		settings.show_tab_flags |= 1 << tab;
	else
		settings.show_tab_flags &= ~(1 << tab);
}

// don't delete the last one
BOOL AllowedToSetShowTab(int tab, BOOL show)
{
	int show_tab_flags = settings.show_tab_flags;

	if (show == TRUE)
		return TRUE;

	show_tab_flags &= ~(1 << tab);
	return show_tab_flags != 0;
}

int GetHistoryTab(void)
{
#ifdef STORY_DATAFILE
	return settings.datafile_tab;
#else /* STORY_DATAFILE */
	return settings.history_tab;
#endif /* STORY_DATAFILE */
}

void SetHistoryTab(int tab, BOOL show)
{
#ifdef STORY_DATAFILE
	if (show)
		settings.datafile_tab = tab;
	else
		settings.datafile_tab = TAB_NONE;
#else /* STORY_DATAFILE */
	if (show)
		settings.history_tab = tab;
	else
		settings.history_tab = TAB_NONE;
#endif /* STORY_DATAFILE */
}

void SetColumnWidths(int width[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		settings.column_widths[i] = width[i];
}

void GetColumnWidths(int width[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		width[i] = settings.column_widths[i];
}

void SetSplitterPos(int splitterId, int pos)
{
	if (splitterId < GetSplitterCount())
		settings.splitters[splitterId] = pos;
}

int  GetSplitterPos(int splitterId)
{
	if (splitterId < GetSplitterCount())
		return settings.splitters[splitterId];

	return -1; /* Error */
}

void SetColumnOrder(int order[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		settings.column_order[i] = order[i];
}

void GetColumnOrder(int order[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		order[i] = settings.column_order[i];
}

void SetColumnShown(int shown[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		settings.column_shown[i] = shown[i];
}

void GetColumnShown(int shown[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		shown[i] = settings.column_shown[i];
}

void SetSortColumn(int column)
{
	settings.sort_column = column;
}

int GetSortColumn(void)
{
	return settings.sort_column;
}

void SetSortReverse(BOOL reverse)
{
	settings.sort_reverse = reverse;
}

BOOL GetSortReverse(void)
{
	return settings.sort_reverse;
}

#ifdef IMAGE_MENU
void SetImageMenuStyle(int style)
{
	settings.imagemenu_style = style;
}

int GetImageMenuStyle(void)
{
	return settings.imagemenu_style;
}
#endif /* IMAGE_MENU */

#ifdef USE_SHOW_SPLASH_SCREEN
void SetDisplaySplashScreen (BOOL val)
{
	settings.display_splash_screen = val;
}

BOOL GetDisplaySplashScreen (void)
{
	return settings.display_splash_screen;
}
#endif /* USE_SHOW_SPLASH_SCREEN */

#ifdef TREE_SHEET
void SetShowTreeSheet(BOOL val)
{
	settings.show_tree_sheet = val;
}

BOOL GetShowTreeSheet(void)
{
	return settings.show_tree_sheet;
}
#endif /* TREE_SHEET */

const WCHAR* GetRomDirs(void)
{
	return settings.rompath;
}

void SetRomDirs(const WCHAR* paths)
{
	FreeIfAllocatedW(&settings.rompath);

	if (paths != NULL)
	{
		settings.rompath = wcsdup(paths);

		// have our mame core (file code) know about it
		// this leaks a little, but the win32 file core writes to this string
		set_core_rom_directory(settings.rompath);
		set_core_image_directory(settings.rompath);
	}
}

const WCHAR* GetSampleDirs(void)
{
	return settings.samplepath;
}

void SetSampleDirs(const WCHAR* paths)
{
	FreeIfAllocatedW(&settings.samplepath);

	if (paths != NULL)
	{
		settings.samplepath = wcsdup(paths);
		
		// have our mame core (file code) know about it
		// this leaks a little, but the win32 file core writes to this string
		set_core_sample_directory(settings.samplepath);
	}
}

//#ifdef MESS
const WCHAR* GetHashDirs(void)
{
	return settings.hashpath;
}

void SetHashDirs(const WCHAR* dir)
{
	FreeIfAllocatedW(&settings.hashpath);

	if (dir != NULL)
		settings.hashpath = wcsdup(dir);
}
//#endif

const WCHAR* GetIniDir(void)
{
	return settings.inipath;
}

void SetIniDir(const WCHAR* path)
{
	if (!wcscmp(path, settings.inipath))
		return;

	FreeIfAllocatedW(&settings.inipath);

	if (path != NULL)
		settings.inipath = wcsdup(path);

	if (MessageBox(0, reload_config_msg, TEXT("Reload configurations"), MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		int i;

		for (i = 0 ; i < num_drivers; i++)
			LoadGameOptions(i);
	}

	create_path_recursive(path);
}

const WCHAR* GetCtrlrDir(void)
{
	return settings.ctrlrpath;
}

void SetCtrlrDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.ctrlrpath);

	if (path != NULL)
		settings.ctrlrpath = wcsdup(path);

	generate_default_ctrlr();
}

const WCHAR* GetCfgDir(void)
{
	return settings.cfg_directory;
}

void SetCfgDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.cfg_directory);

	if (path != NULL)
		settings.cfg_directory = wcsdup(path);
}

#ifdef USE_HISCORE
const WCHAR* GetHiDir(void)
{
	return settings.hiscore_directory;
}

void SetHiDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.hiscore_directory);

	if (path != NULL)
		settings.hiscore_directory = wcsdup(path);
}
#endif /* USE_HISCORE */

const WCHAR* GetNvramDir(void)
{
	return settings.nvram_directory;
}

void SetNvramDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.nvram_directory);

	if (path != NULL)
		settings.nvram_directory = wcsdup(path);
}

const WCHAR* GetInpDir(void)
{
	return settings.input_directory;
}

void SetInpDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.input_directory);

	if (path != NULL)
		settings.input_directory = wcsdup(path);
}

const WCHAR* GetImgDirs(void)
{
	return settings.snapshot_directory;
}

void SetImgDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.snapshot_directory);

	if (path != NULL)
		settings.snapshot_directory = wcsdup(path);
}

const WCHAR* GetStateDir(void)
{
	return settings.state_directory;
}

void SetStateDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.state_directory);

	if (path != NULL)
		settings.state_directory = wcsdup(path);
}

const WCHAR* GetArtDir(void)
{
	return settings.artpath;
}

void SetArtDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.artpath);

	if (path != NULL)
		settings.artpath = wcsdup(path);
}

const WCHAR* GetMemcardDir(void)
{
	return settings.memcard_directory;
}

void SetMemcardDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.memcard_directory);

	if (path != NULL)
		settings.memcard_directory = wcsdup(path);
}

const WCHAR* GetFlyerDirs(void)
{
	return settings.flyer_directory;
}

void SetFlyerDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.flyer_directory);

	if (path != NULL)
		settings.flyer_directory = wcsdup(path);
}

const WCHAR* GetCabinetDirs(void)
{
	return settings.cabinet_directory;
}

void SetCabinetDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.cabinet_directory);

	if (path != NULL)
		settings.cabinet_directory = wcsdup(path);
}

const WCHAR* GetMarqueeDirs(void)
{
	return settings.marquee_directory;
}

void SetMarqueeDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.marquee_directory);

	if (path != NULL)
		settings.marquee_directory = wcsdup(path);
}

const WCHAR* GetTitlesDirs(void)
{
	return settings.title_directory;
}

void SetTitlesDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.title_directory);

	if (path != NULL)
		settings.title_directory = wcsdup(path);
}

const WCHAR* GetControlPanelDirs(void)
{
	return settings.cpanel_directory;
}

void SetControlPanelDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.cpanel_directory);
	if (path != NULL)
		settings.cpanel_directory = wcsdup(path);
}

const WCHAR* GetDiffDir(void)
{
	return settings.diff_directory;
}

void SetDiffDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.diff_directory);

	if (path != NULL)
		settings.diff_directory = wcsdup(path);
}

const WCHAR* GetTranslationDir(void)
{
	return settings.translation_directory;
}

void SetTranslationDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.translation_directory);

	if (path != NULL)
		settings.translation_directory = wcsdup(path);
}

const WCHAR* GetCommentDir(void)
{
	return settings.comment_directory;
}

void SetCommentDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.comment_directory);

	if (path != NULL)
		settings.comment_directory = wcsdup(path);
}

#ifdef USE_IPS
const WCHAR* GetPatchDir(void)
{
	return settings.ips_directory;
}

void SetPatchDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.ips_directory);

	if (path != NULL)
		settings.ips_directory = wcsdup(path);
}
#endif /* USE_IPS */

const WCHAR* GetLocalizedDir(void)
{
	return settings.localized_directory;
}

void SetLocalizedDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.localized_directory);

	if (path != NULL)
		settings.localized_directory = wcsdup(path);
}

const WCHAR* GetIconsDirs(void)
{
	return settings.icon_directory;
}

void SetIconsDirs(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.icon_directory);

	if (path != NULL)
		settings.icon_directory = wcsdup(path);
}

const WCHAR* GetBgDir(void)
{
	return settings.bkground_directory;
}

void SetBgDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.bkground_directory);

	if (path != NULL)
		settings.bkground_directory = wcsdup(path);
}

const WCHAR* GetFolderDir(void)
{
	return settings.folder_directory;
}

void SetFolderDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.folder_directory);

	if (path != NULL)
		settings.folder_directory = wcsdup(path);
}

const WCHAR* GetCheatFile(void)
{
	return settings.cheat_file;
}

void SetCheatFile(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.cheat_file);

	if (path != NULL)
		settings.cheat_file = wcsdup(path);
}

const WCHAR* GetHistoryFile(void)
{
	return settings.history_file;
}

void SetHistoryFile(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.history_file);

	if (path != NULL)
		settings.history_file = wcsdup(path);
}

#ifdef STORY_DATAFILE
const WCHAR* GetStoryFile(void)
{
	return settings.story_file;
}

void SetStoryFile(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.story_file);

	if (path != NULL)
		settings.story_file = wcsdup(path);
}
#endif /* STORY_DATAFILE */

#ifdef USE_VIEW_PCBINFO
const WCHAR* GetPcbinfoDir(void)
{
	return settings.pcbinfo_directory;
}

void SetPcbinfoDir(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.pcbinfo_directory);

	if (path != NULL)
		settings.pcbinfo_directory = wcsdup(path);
}
#endif /* USE_VIEW_PCBINFO */

const WCHAR* GetMAMEInfoFile(void)
{
	return settings.mameinfo_file;
}

void SetMAMEInfoFile(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.mameinfo_file);

	if (path != NULL)
		settings.mameinfo_file = wcsdup(path);
}

#ifdef USE_HISCORE
const WCHAR* GetHiscoreFile(void)
{
	return settings.hiscore_file;
}

void SetHiscoreFile(const WCHAR* path)
{
	FreeIfAllocatedW(&settings.hiscore_file);

	if (path != NULL)
		settings.hiscore_file = wcsdup(path);
}
#endif /* USE_HISCORE */

void ResetGameOptions(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	// make sure it's all loaded up.
	GetGameOptions(driver_index);

	if (!driver_variables[driver_index].use_default)
	{
		FreeGameOptions(&driver_options[driver_index]);
		driver_variables[driver_index].use_default = TRUE;
		
		// this will delete the custom file
		SaveGameOptions(driver_index);
	}
}

static void ResetAltOptions(alt_options_type *alt_option)
{
	// make sure it's all loaded up.
	GetAltOptions(alt_option);

	if (!alt_option->variable->use_default)
	{
		FreeGameOptions(alt_option->option);
		alt_option->variable->use_default = TRUE;

		// this will delete the custom file
		SaveAltOptions(alt_option);
	}
}

void ResetGameDefaults(void)
{
	FreeGameOptions(&global);
	CopyGameOptions(&backup.global, &global);
}

void ResetAllGameOptions(void)
{
	int i;

	for (i = 0; i < num_drivers; i++)
		ResetGameOptions(i);

	for (i = 0; i < num_alt_options; i++)
		ResetAltOptions(&alt_options[i]);
}

int GetRomAuditResults(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	return driver_variables[driver_index].rom_audit_results;
}

void SetRomAuditResults(int driver_index, int audit_results)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	driver_variables[driver_index].rom_audit_results = audit_results;
}

int GetSampleAuditResults(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	return driver_variables[driver_index].samples_audit_results;
}

void SetSampleAuditResults(int driver_index, int audit_results)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	driver_variables[driver_index].samples_audit_results = audit_results;
}

void IncrementPlayCount(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	driver_variables[driver_index].play_count++;

	// maybe should do this
	//SavePlayCount(driver_index);
}

int GetPlayCount(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	return driver_variables[driver_index].play_count;
}

void ResetPlayCount(int driver_index)
{
	int i = 0;
	assert(driver_index < num_drivers);
	if ( driver_index < 0 )
	{
		//All drivers
		for ( i= 0; i< num_drivers; i++ )
			driver_variables[i].play_count = 0;
	}
	else
	{
		driver_variables[driver_index].play_count = 0;
	}
}

void ResetPlayTime(int driver_index)
{
	int i = 0;
	assert(driver_index < num_drivers);
	if ( driver_index < 0 )
	{
		//All drivers
		for ( i= 0; i< num_drivers; i++ )
			driver_variables[i].play_time = 0;
	}
	else
	{
		driver_variables[driver_index].play_time = 0;
	}
}

int GetPlayTime(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_drivers);

	return driver_variables[driver_index].play_time;
}

void IncrementPlayTime(int driver_index, int playtime)
{
	assert(0 <= driver_index && driver_index < num_drivers);
	driver_variables[driver_index].play_time += playtime;
}

void GetTextPlayTime(int driver_index, WCHAR *buf)
{
	int hour, minute, second;
	int temp = driver_variables[driver_index].play_time;

	assert(0 <= driver_index && driver_index < num_drivers);

	hour = temp / 3600;
	temp = temp - 3600*hour;
	minute = temp / 60; //Calc Minutes
	second = temp - 60*minute;

	if (hour == 0)
		swprintf(buf, TEXT("%d:%02d"), minute, second );
	else
		swprintf(buf, TEXT("%d:%02d:%02d"), hour, minute, second );
}


input_seq *Get_ui_key_up(void)
{
	return &settings.ui_key_up.is;
}
input_seq *Get_ui_key_down(void)
{
	return &settings.ui_key_down.is;
}
input_seq *Get_ui_key_left(void)
{
	return &settings.ui_key_left.is;
}
input_seq *Get_ui_key_right(void)
{
	return &settings.ui_key_right.is;
}
input_seq *Get_ui_key_start(void)
{
	return &settings.ui_key_start.is;
}
input_seq *Get_ui_key_pgup(void)
{
	return &settings.ui_key_pgup.is;
}
input_seq *Get_ui_key_pgdwn(void)
{
	return &settings.ui_key_pgdwn.is;
}
input_seq *Get_ui_key_home(void)
{
	return &settings.ui_key_home.is;
}
input_seq *Get_ui_key_end(void)
{
	return &settings.ui_key_end.is;
}
input_seq *Get_ui_key_ss_change(void)
{
	return &settings.ui_key_ss_change.is;
}
input_seq *Get_ui_key_history_up(void)
{
	return &settings.ui_key_history_up.is;
}
input_seq *Get_ui_key_history_down(void)
{
	return &settings.ui_key_history_down.is;
}


input_seq *Get_ui_key_context_filters(void)
{
	return &settings.ui_key_context_filters.is;
}
input_seq *Get_ui_key_select_random(void)
{
	return &settings.ui_key_select_random.is;
}
input_seq *Get_ui_key_game_audit(void)
{
	return &settings.ui_key_game_audit.is;
}
input_seq *Get_ui_key_game_properties(void)
{
	return &settings.ui_key_game_properties.is;
}
input_seq *Get_ui_key_help_contents(void)
{
	return &settings.ui_key_help_contents.is;
}
input_seq *Get_ui_key_update_gamelist(void)
{
	return &settings.ui_key_update_gamelist.is;
}
input_seq *Get_ui_key_view_folders(void)
{
	return &settings.ui_key_view_folders.is;
}
input_seq *Get_ui_key_view_fullscreen(void)
{
	return &settings.ui_key_view_fullscreen.is;
}
input_seq *Get_ui_key_view_pagetab(void)
{
	return &settings.ui_key_view_pagetab.is;
}
input_seq *Get_ui_key_view_picture_area(void)
{
	return &settings.ui_key_view_picture_area.is;
}
input_seq *Get_ui_key_view_status(void)
{
	return &settings.ui_key_view_status.is;
}
input_seq *Get_ui_key_view_toolbars(void)
{
	return &settings.ui_key_view_toolbars.is;
}

input_seq *Get_ui_key_view_tab_cabinet(void)
{
	return &settings.ui_key_view_tab_cabinet.is;
}
input_seq *Get_ui_key_view_tab_cpanel(void)
{
	return &settings.ui_key_view_tab_cpanel.is;
}
input_seq *Get_ui_key_view_tab_flyer(void)
{
	return &settings.ui_key_view_tab_flyer.is;
}
input_seq *Get_ui_key_view_tab_history(void)
{
	return &settings.ui_key_view_tab_history.is;
}
#ifdef STORY_DATAFILE
input_seq *Get_ui_key_view_tab_story(void)
{
	return &settings.ui_key_view_tab_story.is;
}
#endif /* STORY_DATAFILE */
input_seq *Get_ui_key_view_tab_marquee(void)
{
	return &settings.ui_key_view_tab_marquee.is;
}
input_seq *Get_ui_key_view_tab_screenshot(void)
{
	return &settings.ui_key_view_tab_screenshot.is;
}
input_seq *Get_ui_key_view_tab_title(void)
{
	return &settings.ui_key_view_tab_title.is;
}
input_seq *Get_ui_key_quit(void)
{
	return &settings.ui_key_quit.is;
}


int GetUIJoyUp(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);
	
	return settings.ui_joy_up[joycodeIndex];
}

void SetUIJoyUp(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_up[joycodeIndex] = val;
}

int GetUIJoyDown(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_down[joycodeIndex];
}

void SetUIJoyDown(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_down[joycodeIndex] = val;
}

int GetUIJoyLeft(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_left[joycodeIndex];
}

void SetUIJoyLeft(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_left[joycodeIndex] = val;
}

int GetUIJoyRight(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_right[joycodeIndex];
}

void SetUIJoyRight(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_right[joycodeIndex] = val;
}

int GetUIJoyStart(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_start[joycodeIndex];
}

void SetUIJoyStart(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_start[joycodeIndex] = val;
}

int GetUIJoyPageUp(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_pgup[joycodeIndex];
}

void SetUIJoyPageUp(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_pgup[joycodeIndex] = val;
}

int GetUIJoyPageDown(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_pgdwn[joycodeIndex];
}

void SetUIJoyPageDown(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_pgdwn[joycodeIndex] = val;
}

int GetUIJoyHome(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_home[joycodeIndex];
}

void SetUIJoyHome(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_home[joycodeIndex] = val;
}

int GetUIJoyEnd(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_end[joycodeIndex];
}

void SetUIJoyEnd(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_end[joycodeIndex] = val;
}

int GetUIJoySSChange(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_ss_change[joycodeIndex];
}

void SetUIJoySSChange(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_ss_change[joycodeIndex] = val;
}

int GetUIJoyHistoryUp(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_history_up[joycodeIndex];
}

void SetUIJoyHistoryUp(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);
  
	settings.ui_joy_history_up[joycodeIndex] = val;
}

int GetUIJoyHistoryDown(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_history_down[joycodeIndex];
}

void SetUIJoyHistoryDown(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_history_down[joycodeIndex] = val;
}

void SetUIJoyExec(int joycodeIndex, int val)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	settings.ui_joy_exec[joycodeIndex] = val;
}

int GetUIJoyExec(int joycodeIndex)
{
	assert(0 <= joycodeIndex && joycodeIndex < 4);

	return settings.ui_joy_exec[joycodeIndex];
}

WCHAR* GetExecCommand(void)
{
	static WCHAR empty = '\0';

	if (settings.exec_command)
		return settings.exec_command;

	return &empty;
}

void SetExecCommand(WCHAR* cmd)
{
	FreeIfAllocatedW(&settings.exec_command);

	if (cmd != NULL && *cmd)
		settings.exec_command = wcsdup(cmd);
}

int GetExecWait(void)
{
	return settings.exec_wait;
}

void SetExecWait(int wait)
{
	settings.exec_wait = wait;
}
 
BOOL GetHideMouseOnStartup(void)
{
	return settings.hide_mouse;
}

void SetHideMouseOnStartup(BOOL hide)
{
	settings.hide_mouse = hide;
}

BOOL GetRunFullScreen(void)
{
	return settings.full_screen;
}

void SetRunFullScreen(BOOL fullScreen)
{
	settings.full_screen = fullScreen;
}

char* GetVersionString(void)
{
	return build_version;
}

void SaveFolderFlags(const char *path, DWORD flags)
{
	set_folder_flag(&settings.folder_flag, path, flags);
}

DWORD LoadFolderFlags(const char *path)
{
	int i;

	if (settings.folder_flag.entry == NULL)
		return 0;

	for (i = 0; i < settings.folder_flag.num; i++)
		if (settings.folder_flag.entry[i].name
		 && strcmp(settings.folder_flag.entry[i].name, path) == 0)
			return settings.folder_flag.entry[i].flags;

	return 0;
}

void SaveGameOptions(int driver_index)
{
	int i;

	assert (0 <= driver_index && driver_index < num_drivers);

	options_save_driver_config(driver_index);

	for (i = 0; i < num_drivers; i++)
		if (DriverParentIndex(i) == driver_index)
		{
			driver_variables[i].use_default = TRUE;
			driver_variables[i].options_loaded = FALSE;
		}
}

static void InvalidateGameOptionsInDriver(const WCHAR *name)
{
	int i;

	for (i = 0; i < num_drivers; i++)
		if (wcscmp(GetDriverFilename(i), name) == 0)
		{
			driver_variables[i].use_default = TRUE;
			driver_variables[i].options_loaded = FALSE;
		}
}

static void SaveAltOptions(alt_options_type *alt_option)
{
	options_save_alt_config(alt_option);

	if (alt_option->option == GetVectorOptions())
	{
		int i;

		for (i = 0; i < num_alt_options; i++)
			if (alt_options[i].need_vector_config)
			{
				alt_options[i].variable->use_default = TRUE;
				alt_options[i].variable->options_loaded = FALSE;
				InvalidateGameOptionsInDriver(alt_options[i].name);
			}
	}

	InvalidateGameOptionsInDriver(alt_option->name);
}

void SaveFolderOptions(const WCHAR *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	SaveAltOptions(&alt_options[alt_index]);
}

void SaveDefaultOptions(void)
{
	int i;

	options_save_default_config();

	for (i = 0; i < num_alt_options; i++)
	{
		alt_options[i].variable->use_default = TRUE;
		alt_options[i].variable->options_loaded = FALSE;
	}

	for (i = 0; i < num_drivers; i++)
	{
		driver_variables[i].use_default = TRUE;
		driver_variables[i].options_loaded = FALSE;
	}

	/* default option has bios tab. so save default bios */
	for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		if (default_bios[i] != -1)
		{
			char *bios = mame_strdup(driver_options[default_bios[i]].bios);
			GetGameOptions(default_bios[i]);

			FreeIfAllocated(&driver_options[default_bios[i]].bios);
			driver_options[default_bios[i]].bios = bios;

			SaveGameOptions(default_bios[i]);
		}
}

void SaveOptions(void)
{
	int i;

	options_save_winui_config();
	options_save_default_config();

	for (i = 0; i < num_drivers; i++)
		options_save_driver_config(i);

	for (i = 0; i < num_alt_options; i++)
		options_save_alt_config(&alt_options[i]);
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static int regist_alt_option(const WCHAR *name)
{
	int s = 0;
	int n = num_alt_options;

	while (n > 0)
	{
		int n2 = n / 2;
		int result;

		if (name == alt_options[s + n2].name)
			return -1;

		result = wcscmp(name, alt_options[s + n2].name);
		if (!result)
		{
			alt_options[s + n2].name = name;
			return -1;
		}

		if (result < 0)
			n = n2;
		else
		{
			s += n2 + 1;
			n -= n2 + 1;
		}
	}

	// not found, add it.
	if (num_alt_options == alt_options_len)
	{
		alt_options_len += ALLOC_FOLDERS;
		alt_options = (alt_options_type *)realloc(alt_options, alt_options_len * sizeof (*alt_options));

		if (!alt_options)
			exit(0);
	}

	for (n = num_alt_options++; n > s; n--)
		alt_options[n].name = alt_options[n - 1].name;

	alt_options[s].name = name;

	return s;
}

static int bsearch_alt_option(const WCHAR *name)
{
	int s = 0;
	int n = num_alt_options;

	while (n > 0)
	{
		int n2 = n / 2;
		int result;

		result = wcscmp(name, alt_options[s + n2].name);
		if (!result)
			return s + n2;

		if (result < 0)
			n = n2;
		else
		{
			s += n2 + 1;
			n -= n2 + 1;
		}
	}

	return -1;
}

static void build_default_bios(void)
{
	const game_driver *last_skip = NULL;
	int i;

	for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		default_bios[i] = -1;

	for (i = 0; i < num_drivers; i++)
	{
		if (DriverHasOptionalBios(i))
		{
			int driver_index = i;
			int n;

			while (!DriverIsBios(driver_index))
			{
				int parent_index = GetParentIndex(drivers[driver_index]);

				if (parent_index < 0)
					break;

				driver_index = parent_index;
			}

			if (last_skip == drivers[driver_index])
				continue;

			for (n = 0; n < MAX_SYSTEM_BIOS; n++)
			{
				if (default_bios[n] == -1)
				{
					const rom_entry *rom;
					int count = 1;

					for (rom = drivers[driver_index]->rom; !ROMENTRY_ISEND(rom); rom++)
						if (ROMENTRY_ISSYSTEM_BIOS(rom))
							if (count < ROM_GETBIOSFLAGS(rom))
								count = ROM_GETBIOSFLAGS(rom);

					if (count == 1)
					{
						dprintf("BIOS skip: %s [%s]", drivers[driver_index]->description, drivers[driver_index]->name);
						last_skip = drivers[driver_index];
						break;
					}
					if (n >= MAX_SYSTEM_BIOS)
					{
						dprintf("BIOS skip: %d in %s [%s]", count, drivers[driver_index]->description, drivers[driver_index]->name);
						last_skip = drivers[driver_index];
						break;
					}
					else
						dprintf("BIOS %d: %d in %s [%s]", n, count, drivers[driver_index]->description, drivers[driver_index]->name);

					default_bios[n] = driver_index;
					break;
				}
				else if (default_bios[n] == driver_index)
					break;
			}
		}
	}
}

static void build_alt_options(void)
{
	options_type *pOpts;
	driver_variables_type *pVars;
	int i;

	alt_options = (alt_options_type *)malloc(alt_options_len * sizeof (*alt_options));
	num_alt_options = 0;

	if (!alt_options)
		exit(0);

	regist_alt_option(TEXT("Vector"));

	for (i = 0; i < num_drivers; i++)
		regist_alt_option(GetDriverFilename(i));

	pOpts = (options_type *)malloc(num_alt_options * sizeof (*pOpts));
	pVars = (driver_variables_type *)malloc(num_alt_options * sizeof (*pVars));

	if (!pOpts || !pVars)
		exit(0);

	memset(pOpts, 0, num_alt_options * sizeof (*pOpts));
	memset(pVars, 0, num_alt_options * sizeof (*pVars));

	for (i = 0; i < num_alt_options; i++)
	{
		alt_options[i].option = &pOpts[i];
		alt_options[i].variable = &pVars[i];
		alt_options[i].variable->options_loaded = FALSE;
		alt_options[i].variable->use_default = TRUE;
		alt_options[i].need_vector_config = FALSE;
	}

	for (i = 0; i < num_drivers; i++)
		if (DriverIsVector(i))
		{
			int n = bsearch_alt_option(GetDriverFilename(i));
				alt_options[n].need_vector_config = TRUE;
		}
}

static const WCHAR *get_base_config_directory(void)
{
	WCHAR full[_MAX_PATH];
	WCHAR dir[_MAX_DIR];
	static WCHAR path[_MAX_PATH];

	GetModuleFileNameW(GetModuleHandle(NULL), full, _MAX_PATH);
	_wsplitpath(full, path, dir, NULL, NULL);
	wcscat(path, dir);

	if (path[wcslen(path) - 1] == *PATH_SEPARATOR)
		path[wcslen(path) - 1] = '\0';

	return path;
}

static void generate_default_dirs(void)
{
	static const WCHAR* (*GetDirsFunc[])(void) =
	{
		GetRomDirs,
		GetSampleDirs,
		GetArtDir,
		//fixme: fontpath
		GetTranslationDir,
		GetLocalizedDir,
		GetPatchDir,
		GetImgDirs,
		GetFlyerDirs,
		GetCabinetDirs,
		GetMarqueeDirs,
		GetTitlesDirs,
		GetControlPanelDirs,
		GetIconsDirs,
		GetBgDir,
		GetPcbinfoDir,
		NULL
	};
	
	int i;
	for (i = 0; GetDirsFunc[i]; i++)
	{
		WCHAR *paths = wcsdup(GetDirsFunc[i]());
		{
			WCHAR *p;

			for (p = wcstok(paths, TEXT(";")); p; p =wcstok(NULL, TEXT(";")))
				create_path_recursive(p);
		}
		free(paths);
	};
}

static void generate_default_ctrlr(void)
{
	static const char *default_ctrlr = 
		"<mameconfig version=\"10\">\n"
		"\t<system name=\"default\">\n"
		"\t\t<!--\n"
		"\t\t\tStandard input customization file\n"
		"\t\t\t(dummy file for GUI)\n"
		"\t\t-->\n"
		"\t</system>\n"
		"</mameconfig>\n";
	const WCHAR *ctrlrpath = GetCtrlrDir();
	const char *ctrlr = backup.global.ctrlr;
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	BOOL DoCreate;

	wcscpy(fname, ctrlrpath);
	wcscat(fname, TEXT(PATH_SEPARATOR));
	wcscat(fname, _Unicode(ctrlr));
	wcscat(fname, TEXT(".cfg"));

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ, &file);
	if (filerr == FILERR_NONE)
		mame_fclose(file);

	DoCreate = (filerr != FILERR_NONE);

	dprintf("I %shave ctrlr %s", DoCreate ? "don't " : "", ctrlr);

	if (DoCreate)
	{
		create_path_recursive(ctrlrpath);

		filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
		if (filerr == FILERR_NONE)
		{
			mame_fputs(file, default_ctrlr);
			mame_fclose(file);
		}
	}

	free(stemp);
}

static options_type *update_driver_use_default(int driver_index)
{
	options_type *opt = GetParentOptions(driver_index);
#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	WCHAR *ips;
#endif /* USE_IPS */

	if (opt == &driver_options[driver_index])
		return NULL;

#ifdef USE_IPS
	ips = driver_options[driver_index].ips;
	driver_options[driver_index].ips = NULL;
#endif /* USE_IPS */

	driver_variables[driver_index].use_default = IsOptionEqual(&driver_options[driver_index], opt);

#ifdef USE_IPS
	if (driver_variables[driver_index].use_default && ips)
		dprintf("%s: use_default with ips", drivers[driver_index]->name);

	driver_options[driver_index].ips = ips;
#endif /* USE_IPS */

	return opt;
}

static options_type *update_alt_use_default(alt_options_type *alt_option)
{
	options_type *opt = GetDefaultOptions();
	char *bios;
#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	WCHAR *ips;
#endif /* USE_IPS */

	// try vector.ini
	if (alt_option->need_vector_config)
		opt = GetVectorOptions();

	bios = alt_option->option->bios;
	alt_option->option->bios = global.bios;

#ifdef USE_IPS
	ips = alt_option->option->ips;
	alt_option->option->ips = NULL;
#endif /* USE_IPS */

	alt_option->variable->use_default = IsOptionEqual(alt_option->option, opt);

#ifdef USE_IPS
	if (alt_option->variable->use_default && ips)
		dwprintf(TEXT("%s: use_default with ips"), alt_option->name);

	alt_option->option->ips = ips;
#endif /* USE_IPS */

	alt_option->option->bios = bios;

	return opt;
}

static void validate_resolution(char **p)
{
	if (strcmp(*p, "0x0@0") == 0)
	{
		FreeIfAllocated(p);
		*p = mame_strdup("auto");
	}
}

static void validate_driver_option(options_type *opt)
{
	validate_resolution(&opt->resolution);
	validate_resolution(&opt->resolutions[0]);
	validate_resolution(&opt->resolutions[1]);
	validate_resolution(&opt->resolutions[2]);
	validate_resolution(&opt->resolutions[3]);

	//if (DirectDraw_GetNumDisplays() < 2)
	//	FreeIfAllocated(&opt->screen);
}

static void set_folder_flag(f_flag *flag, const char *path, DWORD dwFlags)
{
	int i;

	if (flag->entry == NULL)
	{
		flag->entry = malloc(ALLOC_FOLDERFLAG * sizeof (*flag->entry));
		if (!flag->entry)
		{
			dprintf("error: malloc failed in set_folder_flag\n");
			return;
		}

		flag->num = ALLOC_FOLDERFLAG;
		memset(flag->entry, 0, flag->num * sizeof (*flag->entry));
	}

	for (i = 0; i < flag->num; i++)
		if (flag->entry[i].name && strcmp(flag->entry[i].name, path) == 0)
		{
			if (dwFlags == 0)
			{
				free(flag->entry[i].name);
				flag->entry[i].name = NULL;
			}
			else
				flag->entry[i].flags = dwFlags;

			return;
		}

	if (dwFlags == 0)
		return;

	for (i = 0; i < flag->num; i++)
		if (flag->entry[i].name == NULL)
			break;

	if (i == flag->num)
	{
		f_flag_entry *tmp;

		tmp = realloc(flag->entry, (flag->num + ALLOC_FOLDERFLAG) * sizeof (*tmp));
		if (!tmp)
		{
			dprintf("error: realloc failed in set_folder_flag\n");
			return;
		}

		flag->entry = tmp;
		memset(tmp + flag->num, 0, ALLOC_FOLDERFLAG * sizeof (*tmp));
		flag->num += ALLOC_FOLDERFLAG;
	}

	flag->entry[i].name = mame_strdup(path);
	flag->entry[i].flags = dwFlags;
}

static void free_folder_flag(f_flag *flag)
{
	int i;

	for (i = 0; i < flag->num; i++)
		FreeIfAllocated(&flag->entry[i].name);

	if (flag->entry)
		free(flag->entry);
	flag->entry = NULL;
	flag->num = 0;
}

static void LoadGameOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_drivers);

	options_load_driver_config(driver_index);
}

static void LoadAltOptions(alt_options_type *alt_option)
{
	options_load_alt_config(alt_option);
}

static void LoadDefaultOptions(void)
{
	int i;

	options_load_default_config();

	for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		if (default_bios[i] != -1)
			GetGameOptions(default_bios[i]);
}

static void LoadOptions(void)
{
	LoadDefaultOptions();
	options_load_winui_config();

	set_core_translation_directory(settings.translation_directory);
	SetLangcode(settings.langcode);
	SetUseLangList(UseLangList());
}

//============================================================

const options_entry winui_opts[] =
{
	{ NULL,                          NULL,                         OPTION_HEADER,     "PATH AND DIRECTORY OPTIONS"},
	{ "flyer_directory",             "flyers",                     0,                 "directory for flyers"},
	{ "cabinet_directory",           "cabinets",                   0,                 "directory for cabinets"},
	{ "marquee_directory",           "marquees",                   0,                 "directory for marquees"},
	{ "title_directory",             "titles",                     0,                 "directory for titles"},
	{ "cpanel_directory",            "cpanel",                     0,                 "directory for control panel"},
	{ "icon_directory",              "icons",                      0,                 "directory for icons"},
	{ "bkground_directory",          "bkground",                   0,                 "directory for bkground"},
	{ "folder_directory",            "folders",                    0,                 "directory for folders-ini"},
#ifdef USE_VIEW_PCBINFO
	{ "pcbinfo_directory",           "pcb",                        0,                 "directory for pcb info"},
#endif /* USE_VIEW_PCBINFO */

	{ NULL,                          NULL,                         OPTION_HEADER,     "INTERFACE OPTIONS"},
	{ "game_check",                  "1",                          OPTION_BOOLEAN,    "search for new games"},
	{ "joygui",                      "0",                          OPTION_BOOLEAN,    "allow game selection by a joystick"},
	{ "keygui",                      "0",                          OPTION_BOOLEAN,    "allow game selection by a keyboard"},
	{ "broadcast",                   "0",                          OPTION_BOOLEAN,    "broadcast selected game to all windows"},
	{ "random_bg",                   "1",                          OPTION_BOOLEAN,    "random select background image"},
	{ "cycle_screenshot",            "0",                          0,                 "cycle screen shot image"},
	{ "stretch_screenshot_larger",   "0",                          OPTION_BOOLEAN,    "stretch screenshot larger"},
	{ "screenshot_bordersize",       "11",                         0,                 "screen shot border size"},
	{ "screenshot_bordercolor",      "-1",                         0,                 "screen shot border color"},
	{ "inherit_filter",              "0",                          OPTION_BOOLEAN,    "inheritable filters"},
	{ "offset_clones",               "1",                          OPTION_BOOLEAN,    "no offset for clones missing parent in view"},
	{ "game_caption",                "1",                          OPTION_BOOLEAN,    "show game caption"},
#ifdef USE_SHOW_SPLASH_SCREEN
	{ "display_splash_screen",       "1",                          OPTION_BOOLEAN,    "display splash screen on start"},
#endif /* USE_SHOW_SPLASH_SCREEN */
#ifdef TREE_SHEET
	{ "show_tree_sheet",             "1",                          OPTION_BOOLEAN,    "use tree sheet style"},
#endif /* TREE_SHEET */

	{ NULL,                          NULL,                         OPTION_HEADER,     "GENERAL OPTIONS"},
#ifdef MESS
	{ "default_system",              "nes",                        0,                 "last selected system name"},
#else
	{ "default_game",                "puckman",                    0,                 "last selected game name"},
#endif
	{ "show_toolbar",                "1",                          OPTION_BOOLEAN,    "show tool bar"},
	{ "show_statusbar",              "1",                          OPTION_BOOLEAN,    "show status bar"},
	{ "show_folderlist",             "1",                          OPTION_BOOLEAN,    "show folder list"},
	{ "show_screenshot",             "1",                          OPTION_BOOLEAN,    "show image picture"},
	{ "show_screenshottab",          "1",                          OPTION_BOOLEAN,    "show tab control"},
	{ "show_tab_flags",              "63",                         0,                 "show tab control flags"},
	{ "current_tab",                 "snapshot",                   0,                 "current image picture"},
#ifdef STORY_DATAFILE
	// TAB_ALL = 10
	{ "datafile_tab",                "10",                         0,                 "where to show history on tab"},
#else /* STORY_DATAFILE */
	// TAB_ALL = 9
	{ "history_tab",                 "9",                          0,                 "where to show history on tab"},
#endif /* STORY_DATAFILE */
	{ "exec_command",                NULL,                         0,                 "execute command line"},
	{ "exec_wait",                   "0",                          0,                 "execute wait"},
	{ "hide_mouse",                  "0",                          OPTION_BOOLEAN,    "hide mouse"},
	{ "full_screen",                 "0",                          OPTION_BOOLEAN,    "full screen"},

	{ NULL,                          NULL,                         OPTION_HEADER,     "WINDOW POSITION OPTIONS"},
	{ "window_x",                    "0",                          0,                 "window left position"},
	{ "window_y",                    "0",                          0,                 "window top position"},
	{ "window_width",                "640",                        0,                 "window width"},
	{ "window_height",               "400",                        0,                 "window height"},
	{ "window_state",                "1",                          0,                 "window state"},

	{ NULL,                          NULL,                         OPTION_HEADER,     "LISTVIEW OPTIONS"},
	{ "list_mode",                   "Grouped",                    0,                 "view mode"},
	{ "splitters",                   "150,300",                    0,                 "splitter position"},
	{ "column_widths",               "186,68,84,84,64,88,74,108,60,144,84,60", 0,     "column width settings"},
	{ "column_order",                "0,2,3,4,5,6,7,8,9,10,11,1",  0,                 "column order settings"},
	{ "column_shown",                "1,0,1,1,1,1,1,1,1,1,1,1",    0,                 "show or hide column settings"},
	{ "sort_column",                 "0",                          0,                 "sort column"},
#ifdef IMAGE_MENU
	{ "imagemenu_style",             "0",                          0,                 "current menu style"},
#endif /* IMAGE_MENU */
	{ "sort_reverse",                "0",                          OPTION_BOOLEAN,    "sort descending"},
	{ "folder_current",              "/",                          0,                 "last selected folder id"},
	{ "use_broken_icon",             "1",                          OPTION_BOOLEAN,    "use broken icon for not working games"},

	{ NULL,                          NULL,                         OPTION_HEADER,     "LIST FONT OPTIONS"},
	{ "list_font",                   "-8,0,0,0,400,0,0,0,0,0,0,0,0", 0,               "game list font size"},
	{ "list_fontface",               "",                           0,                 "game list font face"},
	{ "font_color",                  "-1",                         0,                 "game list font color"},
	{ "clone_color",                 "8421504",                    0,                 "clone game list font color"},
	{ "broken_color",                "202",                        0,                 "broken game list font color"},
	{ "custom_color",                "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", 0,            "custom colors"},

	{ NULL,                          NULL,                         OPTION_HEADER,     "FOLDER LIST HIDE OPTIONS"},
	{ "folder_hide",                 NULL,                         0,                 "hide selected item in folder list"},
	{ "folder_flag",                 NULL,                         0,                 "folder list filters settings" },

	{ NULL,                          NULL,                         OPTION_HEADER,     "GUI JOYSTICK OPTIONS"},
	{ "ui_joy_up",                   "1,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_NEG", 0,     "joystick to up"},
	{ "ui_joy_down",                 "1,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_POS", 0,     "joystick to down"},
	{ "ui_joy_left",                 "1,JOYCODE_STICK_AXIS,1,JOYCODE_DIR_NEG", 0,     "joystick to left"},
	{ "ui_joy_right",                "1,JOYCODE_STICK_AXIS,1,JOYCODE_DIR_POS", 0,     "joystick to right"},
	{ "ui_joy_start",                "1,JOYCODE_STICK_BTN,1,JOYCODE_DIR_BTN", 0,      "joystick to start game"},
	{ "ui_joy_pgup",                 "2,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_NEG", 0,     "joystick to page-up"},
	{ "ui_joy_pgdwn",                "2,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_POS", 0,     "joystick to page-down"},
	{ "ui_joy_home",                 NULL,                         0,                 "joystick to home"},
	{ "ui_joy_end",                  NULL,                         0,                 "joystick to end"},
	{ "ui_joy_ss_change",            "2,JOYCODE_STICK_BTN,3,JOYCODE_DIR_BTN", 0,      "joystick to change picture"},
	{ "ui_joy_history_up",           "2,JOYCODE_STICK_BTN,4,JOYCODE_DIR_BTN", 0,      "joystick to scroll history up"},
	{ "ui_joy_history_down",         "2,JOYCODE_STICK_BTN,1,JOYCODE_DIR_BTN", 0,      "joystick to scroll history down"},
	{ "ui_joy_exec",                 NULL,                         0,                 "joystick execute commandline"},

	{ NULL,                          NULL,                         OPTION_HEADER,     "GUI KEYBOARD OPTIONS"},
	{ "ui_key_up",                   "KEYCODE_UP",                 0,                 "keyboard to up"},
	{ "ui_key_down",                 "KEYCODE_DOWN",               0,                 "keyboard to down"},
	{ "ui_key_left",                 "KEYCODE_LEFT",               0,                 "keyboard to left"},
	{ "ui_key_right",                "KEYCODE_RIGHT",              0,                 "keyboard to right"},
	{ "ui_key_start",                "KEYCODE_ENTER NOT KEYCODE_LALT", 0,             "keyboard to start game"},
	{ "ui_key_pgup",                 "KEYCODE_PGUP",               0,                 "keyboard to page-up"},
	{ "ui_key_pgdwn",                "KEYCODE_PGDN",               0,                 "keyboard to page-down"},
	{ "ui_key_home",                 "KEYCODE_HOME",               0,                 "keyboard to home"},
	{ "ui_key_end",                  "KEYCODE_END",                0,                 "keyboard to end"},
	{ "ui_key_ss_change",            "KEYCODE_LALT KEYCODE_0",     0,                 "keyboard to change picture"},
	{ "ui_key_history_up",           "KEYCODE_INSERT",             0,                 "keyboard to history up"},
	{ "ui_key_history_down",         "KEYCODE_DEL",                0,                 "keyboard to history down"},

	{ "ui_key_context_filters",      "KEYCODE_LCONTROL KEYCODE_F", 0,                 "keyboard to context filters"},
	{ "ui_key_select_random",        "KEYCODE_LCONTROL KEYCODE_R", 0,                 "keyboard to select random"},
	{ "ui_key_game_audit",           "KEYCODE_LALT KEYCODE_A",     0,                 "keyboard to game audit"},
	{ "ui_key_game_properties",      "KEYCODE_LALT KEYCODE_ENTER", 0,                 "keyboard to game properties"},
	{ "ui_key_help_contents",        "KEYCODE_F1",                 0,                 "keyboard to help contents"},
	{ "ui_key_update_gamelist",      "KEYCODE_F5",                 0,                 "keyboard to update game list"},
	{ "ui_key_view_folders",         "KEYCODE_LALT KEYCODE_D",     0,                 "keyboard to view folders"},
	{ "ui_key_view_fullscreen",      "KEYCODE_F11",                0,                 "keyboard to full screen"},
	{ "ui_key_view_pagetab",         "KEYCODE_LALT KEYCODE_B",     0,                 "keyboard to view page tab"},
	{ "ui_key_view_picture_area",    "KEYCODE_LALT KEYCODE_P",     0,                 "keyboard to view picture area"},
	{ "ui_key_view_status",          "KEYCODE_LALT KEYCODE_S",     0,                 "keyboard to view status"},
	{ "ui_key_view_toolbars",        "KEYCODE_LALT KEYCODE_T",     0,                 "keyboard to view toolbars"},

	{ "ui_key_view_tab_cabinet",     "KEYCODE_LALT KEYCODE_3",     0,                 "keyboard to view tab cabinet"},
	{ "ui_key_view_tab_cpanel",      "KEYCODE_LALT KEYCODE_6",     0,                 "keyboard to view tab control panel"},
	{ "ui_key_view_tab_flyer",       "KEYCODE_LALT KEYCODE_2",     0,                 "keyboard to view tab flyer"},
	{ "ui_key_view_tab_history",     "KEYCODE_LALT KEYCODE_7",     0,                 "keyboard to view tab history"},
#ifdef STORY_DATAFILE
	{ "ui_key_view_tab_story",       "KEYCODE_LALT KEYCODE_8",     0,                 "keyboard to view tab story"},
#endif /* STORY_DATAFILE */
	{ "ui_key_view_tab_marquee",     "KEYCODE_LALT KEYCODE_4",     0,                 "keyboard to view tab marquee"},
	{ "ui_key_view_tab_screenshot",  "KEYCODE_LALT KEYCODE_1",     0,                 "keyboard to view tab screen shot"},
	{ "ui_key_view_tab_title",       "KEYCODE_LALT KEYCODE_5",     0,                 "keyboard to view tab title"},
	{ "ui_key_quit",                 "KEYCODE_LALT KEYCODE_Q",     0,                 "keyboard to quit application"},

	{ NULL }
};

static const char *driver_flag_names[] =
{
	"_playcount",
	"_play_time",
	"_rom_audit",
	"_samples_audit"
};

static char driver_flag_unknown[3];

static options_entry driver_flag_opts[] =
{
	{ NULL, NULL,                OPTION_HEADER, NULL },
	{ NULL, "0",                 0,             NULL },
	{ NULL, "0",                 0,             NULL },
	{ NULL, driver_flag_unknown, 0,             NULL },
	{ NULL, driver_flag_unknown, 0,             NULL },
	{ NULL }
};

static void options_add_driver_flag_opts(core_options *opts)
{
	int i, j;

	sprintf(driver_flag_unknown, "%d", UNKNOWN);

	for (i = 0; i < num_drivers; i++)
	{
		char buf[256];
		char *p = buf;

		driver_flag_opts[0].description = drivers[i]->description;

		for (j = 0; j < ARRAY_LENGTH(driver_flag_names); j++)
		{
			if (!driver_flag_names[j])
				continue;

			driver_flag_opts[j + 1].name = p;
			strcpy(p, drivers[i]->name);
			p += strlen(p);
			strcat(p, driver_flag_names[j]);
			p += strlen(p) + 1;
		}

		options_add_entries(opts, driver_flag_opts);

#ifdef HANDLE_MESS_OPTIONS
		if (DriverIsConsole(i))
			MessSetupGameVariables(opts, i);
#endif /* HANDLE_MESS_OPTIONS */
	}
}

static void options_get_driver_flag_opts(core_options *opts)
{
	int i, j;

	for (i = 0; i < num_drivers; i++)
	{
		int *flags[] =
		{
			&driver_variables[i].play_count,
			&driver_variables[i].play_time,
			&driver_variables[i].rom_audit_results,
			&driver_variables[i].samples_audit_results
		};
		char buf[256];
		char *p;

		strcpy(buf, drivers[i]->name);
		p = buf + strlen(buf);

		for (j = 0; j < ARRAY_LENGTH(driver_flag_names); j++)
		{
			strcpy(p, driver_flag_names[j]);
			*flags[j] = options_get_int(opts, buf);
		}
	}
}

static void options_set_driver_flag_opts(core_options *opts)
{
	int i, j;

	for (i = 0; i < num_drivers; i++)
	{
		int flags[] =
		{
			driver_variables[i].play_count,
			driver_variables[i].play_time,
			driver_variables[i].rom_audit_results,
			driver_variables[i].samples_audit_results
		};
		char buf[256];
		char *p;

		strcpy(buf, drivers[i]->name);
		p = buf + strlen(buf);

		for (j = 0; j < ARRAY_LENGTH(driver_flag_names); j++)
		{
			strcpy(p, driver_flag_names[j]);
			options_set_int(opts, buf, flags[j], OPTION_PRIORITY_CMDLINE);
		}
	}
}


//============================================================

static void debug_printf(const char *s)
{
	dwprintf(TEXT("%s"), _UTF8Unicode(s));
}

static void memory_error(const char *message)
{
	debug_printf(message);
	exit(1);
}

static core_options *options_create_entry_cli(void)
{
	core_options *opt;

	/* create cli options */
	opt = mame_options_init(mame_win_options);

	options_set_output_callback(opt, OPTMSG_INFO, debug_printf);
	options_set_output_callback(opt, OPTMSG_WARNING, debug_printf);
	options_set_output_callback(opt, OPTMSG_ERROR, debug_printf);

	return opt;
}

static core_options *options_create_entry_winui(void)
{
	core_options *opt;

	/* create winui options */
	opt = options_create(memory_error);

	/* set up output callbacks */
	options_set_output_callback(opt, OPTMSG_INFO, debug_printf);
	options_set_output_callback(opt, OPTMSG_WARNING, debug_printf);
	options_set_output_callback(opt, OPTMSG_ERROR, debug_printf);

	options_add_entries(opt, winui_opts);
#ifdef HANDLE_MESS_OPTIONS
	MessSetupSettings(opt);
#endif /* HANDLE_MESS_OPTIONS */

	options_add_driver_flag_opts(opt);

	return opt;
}

static int options_load_default_config(void)
{
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	int retval = 0;

	wcscpy(fname, get_base_config_directory());
	wcscat(fname, TEXT(PATH_SEPARATOR) TEXT_MAME_INI);

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return 0;

	options_set_core(options_cli, &backup.settings);
	options_set_driver(options_cli, &backup.global);
	retval = options_parse_ini_file(options_cli, mame_core_file(file), OPTION_PRIORITY_CMDLINE);
	options_get_core(options_cli, &settings);
	options_get_driver(options_cli, &global);

	mame_fclose(file);

	return retval;
}

static int options_load_driver_config(int driver_index)
{
	core_options *opt = driver_options[driver_index].dynamic_opt;
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	int retval;

	driver_variables[driver_index].options_loaded = TRUE;
	driver_variables[driver_index].use_default = TRUE;

	wcscpy(fname, settings.inipath);
	wcscat(fname, TEXT(PATH_SEPARATOR));
	wcscat(fname, driversw[driver_index]->name);
	wcscat(fname, TEXT(".ini"));

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return 0;

	if (!opt)
		opt = options_cli;

	options_set_driver(opt, &driver_options[driver_index]);
	retval = options_parse_ini_file(opt, mame_core_file(file), OPTION_PRIORITY_CMDLINE);
	options_get_driver(opt, &driver_options[driver_index]);

	update_driver_use_default(driver_index);

	mame_fclose(file);

	return retval;
}

static int options_load_alt_config(alt_options_type *alt_option)
{
	core_options *opt = alt_option->option->dynamic_opt;
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	int len;
	int retval;

	alt_option->variable->options_loaded = TRUE;
	alt_option->variable->use_default = TRUE;

	wcscpy(fname, settings.inipath);
	if (wcscmpi(alt_option->name, TEXT("Vector")) != 0)
	{
		wcscat(fname, TEXT(PATH_SEPARATOR));
		wcscat(fname, TEXT("source"));
	}
	wcscat(fname, TEXT(PATH_SEPARATOR));
	wcscat(fname, alt_option->name);
	wcscat(fname, TEXT(".ini"));
	len = wcslen(fname);
	if (len > 6 && fname[len - 6] == '.' && fname[len - 5] == 'c')
		wcscpy(fname + len - 6, TEXT(".ini"));

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return 0;

	if (!opt)
		opt = options_cli;

	options_set_driver(opt, alt_option->option);
	retval = options_parse_ini_file(opt, mame_core_file(file), OPTION_PRIORITY_CMDLINE);
	options_get_driver(opt, alt_option->option);

	update_alt_use_default(alt_option);

	mame_fclose(file);

	return retval;
}

static int options_load_winui_config(void)
{
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	int retval;

	wcscpy(fname, settings.inipath);
	wcscat(fname, TEXT(PATH_SEPARATOR) TEXT_WINUI_INI);

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return 0;

	options_set_winui(&backup.settings);
	retval = options_parse_ini_file(options_winui, mame_core_file(file), OPTION_PRIORITY_CMDLINE);
	options_get_winui(&settings);
	options_get_driver_flag_opts(options_winui);

	mame_fclose(file);

	return retval;
}


static int options_save_default_config(void)
{
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;

	validate_driver_option(&global);

	wcscpy(fname, get_base_config_directory());
	wcscat(fname, TEXT(PATH_SEPARATOR) TEXT_MAME_INI);

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return -1;

	options_set_core(options_cli, &settings);
	options_set_driver(options_cli, &global);
	options_output_ini_file(options_cli, mame_core_file(file));

	mame_fclose(file);

	return 0;
}

static int options_save_driver_config(int driver_index)
{
	core_options *opt = driver_options[driver_index].dynamic_opt;
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;
	options_type *parent;

	if (driver_variables[driver_index].options_loaded == FALSE)
		return 0;

	parent = update_driver_use_default(driver_index);
	if (parent == NULL)
		return 0;

	wcscpy(fname, settings.inipath);
	wcscat(fname, TEXT(PATH_SEPARATOR));
	wcscat(fname, strlower(driversw[driver_index]->name));
	wcscat(fname, TEXT(".ini"));

#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	if (!driver_options[driver_index].ips)
#endif /* USE_IPS */
	{
		if (!opt && driver_variables[driver_index].use_default)
		{
			DeleteFileW(fname);
			return 0;
		}
	}

	create_path_recursive(settings.inipath);

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return -1;

	if (!opt)
		opt = options_cli;

	options_set_core(options_ref, &settings);
	options_set_driver(options_ref, parent);
	options_set_core(opt, &settings);
	options_set_driver(opt, &driver_options[driver_index]);
	options_output_diff_ini_file(opt, options_ref, mame_core_file(file));

	mame_fclose(file);

	return 0;
}

static int options_save_alt_config(alt_options_type *alt_option)
{
	core_options *opt = alt_option->option->dynamic_opt;
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	WCHAR inipath[MAX_PATH];
	char *stemp;
	options_type *parent;
	int len;

	if (alt_option->variable->options_loaded == FALSE)
		return 0;

	parent = update_alt_use_default(alt_option);

	wcscpy(fname, settings.inipath);
	if (wcscmpi(alt_option->name, TEXT("Vector")) != 0)
	{
		wcscat(fname, TEXT(PATH_SEPARATOR));
		wcscat(fname, TEXT("source"));
	}
	wcscpy(inipath, fname);
	wcscat(fname, TEXT(PATH_SEPARATOR));
	wcscat(fname, strlower(alt_option->name));
	wcscat(fname, TEXT(".ini"));

	len = wcslen(fname);
	if (len > 6 && fname[len - 6] == '.' && fname[len - 5] == 'c')
		wcscpy(fname + len - 6, TEXT(".ini"));

#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	if (!alt_option->option->ips)
#endif /* USE_IPS */
	{
		if (!opt && alt_option->variable->use_default)
		{
			DeleteFileW(fname);
			return 0;
		}
	}

	create_path_recursive(inipath);

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return -1;

	if (!opt)
		opt = options_cli;

	options_set_core(options_ref, &settings);
	options_set_driver(options_ref, parent);
	options_set_core(opt, &settings);
	options_set_driver(opt, alt_option->option);
	options_output_diff_ini_file(opt, options_ref, mame_core_file(file));

	mame_fclose(file);

	return 0;
}

static int options_save_winui_config(void)
{
	mame_file *file;
	file_error filerr;
	WCHAR fname[MAX_PATH];
	char *stemp;

	create_path_recursive(settings.inipath);

	wcscpy(fname, settings.inipath);
	wcscat(fname, strlower(TEXT(PATH_SEPARATOR) TEXT_WINUI_INI));

	stemp = utf8_from_wstring(fname);
	filerr = mame_fopen_options(get_core_options(), SEARCHPATH_RAW, stemp, OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file);
	free(stemp);

	if (filerr != FILERR_NONE)
		return -1;

	options_set_winui(&settings);
	options_set_driver_flag_opts(options_winui);
	options_output_ini_file(options_winui, mame_core_file(file));

	mame_fclose(file);

	return 0;
}

WCHAR *OptionsGetCommandLine(int driver_index, void (*override_callback)(void *param), void *param)
{
	core_options *opt = driver_options[driver_index].dynamic_opt;
	options_type *o;
	WCHAR pModule[_MAX_PATH];
	WCHAR *result;
	WCHAR *wo;
	char *p;
	int pModuleLen;
	int len;

	if (OnNT())
		GetModuleFileNameW(GetModuleHandle(NULL), pModule, _MAX_PATH);
	else
	{
		char pModuleA[_MAX_PATH];

		GetModuleFileNameA(GetModuleHandle(NULL), pModuleA, _MAX_PATH);
		wcscpy(pModule, _Unicode(pModuleA));
	}

	o = GetGameOptions(driver_index);

	if (!opt)
		opt = options_cli;

	options_set_core(options_ref, &backup.settings);
	options_set_driver(options_ref, &backup.global);
	options_set_core(opt, &settings);
	options_set_driver(opt, o);

	if (override_callback)
	{
		core_options *save = options_cli;

		options_cli = opt;
		override_callback(param);
		options_cli = save;
	}

	len = options_output_diff_command_line(opt, options_ref, NULL);
	p = malloc(len + 1);
	options_output_diff_command_line(opt, options_ref, p);
	wo = wstring_from_utf8(p);
	free(p);

	len = wcslen(wo);

	pModuleLen = wcslen(pModule) + 10 + strlen(drivers[driver_index]->name);
	result = malloc((pModuleLen + len + 1) * sizeof (*result));
	wsprintf(result, TEXT("\"%s\" %s -norc "), pModule, driversw[driver_index]->name);

	if (len != 0)
		wcscat(result, wo);
	else
		result[pModuleLen - 1] = '\0';

	free(wo);

	return result;
}


//============================================================

static void options_duplicate_core(const settings_type *source, settings_type *dest);
static void options_duplicate_winui(const settings_type *source, settings_type *dest);

static void options_duplicate_settings(const settings_type *source, settings_type *dest)
{
	options_duplicate_winui(source, dest);
	options_duplicate_core(source, dest);
}


//============================================================

#include "opthndlr.c"


//============================================================

#undef START_OPT_FUNC_CORE
#undef END_OPT_FUNC_CORE
#undef START_OPT_FUNC_DRIVER
#undef END_OPT_FUNC_DRIVER
#undef START_OPT_FUNC_WINUI
#undef END_OPT_FUNC_WINUI
#undef DEFINE_OPT
#undef DEFINE_OPT_CSV
#undef DEFINE_OPT_STRUCT
#undef DEFINE_OPT_ARRAY
#undef DEFINE_OPT_N

#define START_OPT_FUNC_CORE	static void options_get_core(core_options *opts, settings_type *p) {
#define END_OPT_FUNC_CORE	}
#define START_OPT_FUNC_DRIVER	static void options_get_driver(core_options *opts, options_type *p) {
#define END_OPT_FUNC_DRIVER	}
#define START_OPT_FUNC_WINUI	static void options_get_winui(settings_type *p) { \
					core_options *opts = options_winui;
#define END_OPT_FUNC_WINUI	}
#define DEFINE_OPT(type,name)		_options_get_##type(opts, (&p->name), #name);
#define DEFINE_OPT_CSV(type,name)	_options_get_csv_##type(opts, (p->name), ARRAY_LENGTH(p->name), #name);
#define DEFINE_OPT_STRUCT		DEFINE_OPT
#define DEFINE_OPT_ARRAY(type,name)	_options_get_##type(opts, (p->name), #name);
#define DEFINE_OPT_N(type,name,n)	_options_get_##type(opts, &(p->name##s[n]), #name#n);
#include "optdef.h"


#undef START_OPT_FUNC_CORE
#undef END_OPT_FUNC_CORE
#undef START_OPT_FUNC_DRIVER
#undef END_OPT_FUNC_DRIVER
#undef START_OPT_FUNC_WINUI
#undef END_OPT_FUNC_WINUI
#undef DEFINE_OPT
#undef DEFINE_OPT_CSV
#undef DEFINE_OPT_STRUCT
#undef DEFINE_OPT_ARRAY
#undef DEFINE_OPT_N

#define START_OPT_FUNC_CORE	static void options_set_core(core_options *opts, const settings_type *p) {
#define END_OPT_FUNC_CORE	}
#define START_OPT_FUNC_DRIVER	static void options_set_driver(core_options *opts, const options_type *p) {
#define END_OPT_FUNC_DRIVER	}
#define START_OPT_FUNC_WINUI	static void options_set_winui(const settings_type *p) { \
					core_options *opts = options_winui;
#define END_OPT_FUNC_WINUI	}
#define DEFINE_OPT(type,name)		options_set_##type(opts, #name, (p->name), OPTION_PRIORITY_CMDLINE);
#define DEFINE_OPT_CSV(type,name)	options_set_csv_##type(opts, #name, (p->name), ARRAY_LENGTH(p->name), OPTION_PRIORITY_CMDLINE);
#define DEFINE_OPT_STRUCT(type,name)	options_set_##type(opts, #name, (&p->name), OPTION_PRIORITY_CMDLINE);
#define DEFINE_OPT_ARRAY(type,name)	options_set_##type(opts, #name, (p->name), OPTION_PRIORITY_CMDLINE);
#define DEFINE_OPT_N(type,name,n)	options_set_##type(opts, #name#n, (p->name##s[n]), OPTION_PRIORITY_CMDLINE);
#include "optdef.h"


#undef START_OPT_FUNC_CORE
#undef END_OPT_FUNC_CORE
#undef START_OPT_FUNC_DRIVER
#undef END_OPT_FUNC_DRIVER
#undef START_OPT_FUNC_WINUI
#undef END_OPT_FUNC_WINUI
#undef DEFINE_OPT
#undef DEFINE_OPT_CSV
#undef DEFINE_OPT_STRUCT
#undef DEFINE_OPT_ARRAY
#undef DEFINE_OPT_N

#define START_OPT_FUNC_CORE	static void options_free_string_core(settings_type *p) {
#define END_OPT_FUNC_CORE	}
#define START_OPT_FUNC_DRIVER	static void options_free_string_driver(options_type *p) {
#define END_OPT_FUNC_DRIVER	}
#define START_OPT_FUNC_WINUI	static void options_free_string_winui(settings_type *p) {
#define END_OPT_FUNC_WINUI	}
#define DEFINE_OPT(type,name)		options_free_##type(&p->name);
#define DEFINE_OPT_CSV(type,name)	options_free_csv_##type((p->name), ARRAY_LENGTH(p->name));
#define DEFINE_OPT_STRUCT		DEFINE_OPT
#define DEFINE_OPT_ARRAY(type,name)	options_free_##type(p->name);
#define DEFINE_OPT_N(type,name,n)	options_free_##type(&(p->name##s[n]));
#include "optdef.h"


#undef START_OPT_FUNC_CORE
#undef END_OPT_FUNC_CORE
#undef START_OPT_FUNC_DRIVER
#undef END_OPT_FUNC_DRIVER
#undef START_OPT_FUNC_WINUI
#undef END_OPT_FUNC_WINUI
#undef DEFINE_OPT
#undef DEFINE_OPT_CSV
#undef DEFINE_OPT_STRUCT
#undef DEFINE_OPT_ARRAY
#undef DEFINE_OPT_N

#define START_OPT_FUNC_CORE	static void options_duplicate_core(const settings_type *source, settings_type *dest) {
#define END_OPT_FUNC_CORE	}
#define START_OPT_FUNC_DRIVER	static void options_duplicate_driver(const options_type *source, options_type *dest) {
#define END_OPT_FUNC_DRIVER	}
#define START_OPT_FUNC_WINUI	static void options_duplicate_winui(const settings_type *source, settings_type *dest) {
#define END_OPT_FUNC_WINUI	}
#define DEFINE_OPT(type,name)		options_copy_##type((source->name), (&dest->name));
#define DEFINE_OPT_CSV(type,name)	options_copy_csv_##type((source->name), (dest->name), ARRAY_LENGTH(dest->name));
#define DEFINE_OPT_STRUCT(type,name)	options_copy_##type((&source->name), (&dest->name));
#define DEFINE_OPT_ARRAY(type,name)	options_copy_##type((source->name), (dest->name));
#define DEFINE_OPT_N(type,name,n)	options_copy_##type((source->name##s[n]), &(dest->name##s[n]));
#include "optdef.h"


#undef START_OPT_FUNC_CORE
#undef END_OPT_FUNC_CORE
#undef START_OPT_FUNC_DRIVER
#undef END_OPT_FUNC_DRIVER
#undef START_OPT_FUNC_WINUI
#undef END_OPT_FUNC_WINUI
#undef DEFINE_OPT
#undef DEFINE_OPT_CSV
#undef DEFINE_OPT_STRUCT
#undef DEFINE_OPT_ARRAY
#undef DEFINE_OPT_N

#define START_OPT_FUNC_DRIVER	static BOOL options_compare_driver(const options_type *p1, const options_type *p2) {
#define END_OPT_FUNC_DRIVER	return 0; \
				}
#define DEFINE_OPT(type,name)		_options_compare_##type((p1->name), (p2->name));
#define DEFINE_OPT_N(type,name,n)	_options_compare_##type((p1->name##s[n]), (p2->name##s[n]));
#include "optdef.h"

/* End of winuiopt.c */
