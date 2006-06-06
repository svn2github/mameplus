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

#include "MAME32.h"	// include this first
#include "screenshot.h"
#include "bitmask.h"
#include "driver.h"
#include "inptport.h"
#include "m32util.h"
#include "resource.h"
#include "TreeView.h"
#include "file.h"
#include "splitters.h"
#include "DirectDraw.h"
#include "dijoystick.h"
#include "audit.h"
#include "options.h"
#include "picker.h"
#include "io.h"
#include "translate.h"
#include "rc.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif


/***************************************************************************
    Internal structures
 ***************************************************************************/

// defined in src/windows/rc.c
struct rc_struct
{
	struct rc_option *option;
	int option_size;
	char **arg;
	int arg_size;
	int args_registered;
};

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
	int alt_index; // index for alt_option if driver is unified

} game_variables_type;

typedef struct
{
	const char *name;
	options_type *option;
	game_variables_type *variable;
	BOOL has_bios;
	BOOL need_vector_config;
	int driver_index; // index for driver if driver is unified
} alt_options_type;

struct _default_bios
{
	const game_driver *drv;
	alt_options_type *alt_option;
};

typedef struct
{
	char  name[80];
	DWORD flags;
} folder_flags_type;

struct _joycodes
{
	const char *name;
	int value;
};


/***************************************************************************
    Internal function prototypes
 ***************************************************************************/

static int   regist_alt_option(const char *name);
static int   bsearch_alt_option(const char *name);
static void  build_default_bios(void);
static void  build_alt_options(void);
static void  unify_alt_options(void);

static int   initialize_rc_winui_config(void);
static int   rc_load_winui_config(void);
static int   rc_save_winui_config(void);

static int   rc_load_default_config(void);
static int   rc_save_default_config(void);
static int   rc_load_game_config(int driver_index);
static int   rc_save_game_config(int driver_index);
static int   rc_load_alt_config(alt_options_type *alt_option);
static int   rc_save_alt_config(alt_options_type *alt_option);

static int   rc_write_folder_flags(mame_file *file);

static void  validate_game_option(options_type *opt);

static void  rc_duplicate_strings(struct rc_option *option);
static void  rc_free_strings(struct rc_option *option);

static void  CopySettings(const settings_type *source, settings_type *dest);
static void  FreeSettings(settings_type *p);

static void  SaveGlobalOptions(void);
static void  SaveAltOptions(alt_options_type *alt_option);
static void  LoadOptions(void);
static void  LoadGameOptions(int driver_index);
static void  LoadAltOptions(alt_options_type *alt_option);

static BOOL  IsOptionEqual(options_type *o1, options_type *o2);

#if 0
static void  ResetD3DEffect(void);
static void  SortD3DEffectByOverrides(void);

static int   D3DEffectDecode(struct rc_option *option, const char *arg, int priority);
static int   D3DFeedbackDecode(struct rc_option *option, const char *arg, int priority);
static int   D3DScanlinesDecode(struct rc_option *option, const char *arg, int priority);
static int   D3DPrescaleDecode(struct rc_option *option, const char *arg, int priority);

static int   CleanStretchDecodeString(struct rc_option *option, const char *arg, int priority);

static int   LedmodeDecodeString(struct rc_option *option, const char *arg, int priority);

#ifdef UI_COLOR_DISPLAY
static int   PaletteDecodeString(struct rc_option *option, const char *arg, int priority);
#endif /* UI_COLOR_DISPLAY */

#endif

static void  LanguageEncodeString(void);
static int   LanguageDecodeString(void);

static void  JoyInfoEncodeString(void);
static int   JoyInfoDecodeString(struct rc_option *option, const char *arg, int priority);

static void  KeySeqEncodeString(void);
static int   KeySeqDecodeString(struct rc_option *option, const char *arg, int priority);

static char  *ColumnEncodeString(int *data);
static int   ColumnDecodeString(struct rc_option *option, const char *str, int* data, int priority);

static void  ColumnOrderEncodeString(void);
static int   ColumnOrderDecodeString(struct rc_option *option, const char *arg, int priority);

static void  ColumnShownEncodeString(void);
static int   ColumnShownDecodeString(struct rc_option *option, const char *arg, int priority);

static void  ColumnEncodeWidths(void);
static int   ColumnDecodeWidths(struct rc_option *option, const char *arg, int priority);

static void  CusColorEncodeString(void);
static int   CusColorDecodeString(struct rc_option *option, const char *arg, int priority);

static void  SplitterEncodeString(void);
static int   SplitterDecodeString(struct rc_option *option, const char *arg, int priority);

static void  ListEncodeString(void);
static int   ListDecodeString(struct rc_option *option, const char *arg, int priority);

static void  FontEncodeString(void);
static int   FontDecodeString(struct rc_option *option, const char *arg, int priority);

static void  FontfaceEncodeString(void);
static int   FontfaceDecodeString(struct rc_option *option, const char *arg, int priority);

static void  SaveFolderFlags(const char *folderName, DWORD dwFlags);

static int   FolderFlagDecodeString(struct rc_option *option, const char *arg, int priority);

static void  HideFolderEncodeString(void);
static int   HideFolderDecodeString(struct rc_option *option, const char *arg, int priority);



/***************************************************************************
    Internal defines
 ***************************************************************************/

#define FOLDERFLAG_OPT		"folder_flag"
#define ALLOC_FOLDERFLAG	8
#define ALLOC_FOLDERS		100

#define WINUI_INI MAME32NAME "ui.ini"
#define MAME_INI MAMENAME ".ini"


/***************************************************************************
    Internal variables
 ***************************************************************************/

static settings_type settings;

static struct _backup backup;

static options_type gOpts;  // Used in conjunction with regGameOpts

static options_type global; // Global 'default' options
static options_type *game_options;  // Array of Game specific options
static game_variables_type *game_variables;  // Array of game specific extra data

static int  num_alt_options = 0;
static int alt_options_len = 700;
alt_options_type *alt_options;  // Array of Folder specific options

// default bios setting
static struct _default_bios default_bios[MAX_SYSTEM_BIOS];

static int num_folder_flags = 0;
static folder_flags_type *folder_flags;

/* Global UI options */
static int  num_games = 0;
static BOOL bResetGUI      = FALSE;

// Screen shot Page tab control text
// these must match the order of the options flags in options.h
// (TAB_...)
const char* image_tabs_long_name[MAX_TAB_TYPES] =
{
	"Snapshot",
	"Flyer",
	"Cabinet",
	"Marquee",
	"Title",
	"Control Panel",
#ifdef STORY_DATAFILE
	"History",
	"Story"
#else /* STORY_DATAFILE */
	"History"
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

static struct _joycodes joycode_axis[] =
{
	{ "JOYCODE_STICK_BTN",  JOYCODE_STICK_BTN },
	{ "JOYCODE_STICK_AXIS", JOYCODE_STICK_AXIS },
	{ "JOYCODE_STICK_POV",  JOYCODE_STICK_POV },
	{ NULL, 0 }
};

static struct _joycodes joycode_dir[] =
{
	{ "JOYCODE_DIR_BTN", JOYCODE_DIR_BTN },
	{ "JOYCODE_DIR_NEG", JOYCODE_DIR_NEG },
	{ "JOYCODE_DIR_POS", JOYCODE_DIR_POS },
	{ NULL, 0 }
};

static char reload_config_msg[] =
MAME32NAME " has changed *.ini file directory.\n\n\
Would you like to migrate old configurations to the new directory?";


//============================================================
//	rc options
//============================================================

static struct rc_struct *rc_core;
static struct rc_struct *rc_game;
static struct rc_struct **rc_winui;

/* temporary for rc, it is need to save settings */
static struct
{
	/* WINUI */
	char *save_version;

	char *view;
	char *list_font;
	char *list_fontface;
	char *custom_color;
	char *splitter;
	char *column_width;
	char *column_order;
	char *column_shown;

	char *ui_joy_up;
	char *ui_joy_down;
	char *ui_joy_left;
	char *ui_joy_right;
	char *ui_joy_start;
	char *ui_joy_pgup;
	char *ui_joy_pgdwn;
	char *ui_joy_home;
	char *ui_joy_end;
	char *ui_joy_ss_change;
	char *ui_joy_history_up;
	char *ui_joy_history_down;
	char *ui_joy_exec;

	char *ui_key_up;
	char *ui_key_down;
	char *ui_key_left;
	char *ui_key_right;
	char *ui_key_start;
	char *ui_key_pgup;
	char *ui_key_pgdwn;
	char *ui_key_home;
	char *ui_key_end;
	char *ui_key_ss_change;
	char *ui_key_history_up;
	char *ui_key_history_down;

	char *ui_key_context_filters;
	char *ui_key_select_random;
	char *ui_key_game_audit;
	char *ui_key_game_properties;
	char *ui_key_help_contents;
	char *ui_key_update_gamelist;
	char *ui_key_view_folders;
	char *ui_key_view_fullscreen;
	char *ui_key_view_pagetab;
	char *ui_key_view_picture_area;
	char *ui_key_view_status;
	char *ui_key_view_toolbars;

	char *ui_key_view_tab_cabinet;
	char *ui_key_view_tab_cpanel;
	char *ui_key_view_tab_flyer;
	char *ui_key_view_tab_history;
#ifdef STORY_DATAFILE
	char *ui_key_view_tab_story;
#endif /* STORY_DATAFILE */
	char *ui_key_view_tab_marquee;
	char *ui_key_view_tab_screenshot;
	char *ui_key_view_tab_title;
	char *ui_key_quit;

	char *ui_hide_folder;
} rc_dummy_args;

static struct rc_option rc_game_opts[] =
{
	{ "CORE VIDEO OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "rotate", NULL, rc_bool, &gOpts.rotate, "1", 0, 0, NULL, "rotate the game screen according to the game's orientation needs it" },
	{ "ror", NULL, rc_bool, &gOpts.ror, "0", 0, 0, NULL, "rotate screen clockwise 90 degrees" },
	{ "rol", NULL, rc_bool, &gOpts.rol, "0", 0, 0, NULL, "rotate screen counterclockwise 90 degrees" },
	{ "autoror", NULL, rc_bool, &gOpts.autoror, "0", 0, 0, NULL, "automatically rotate screen clockwise 90 degrees if vertical" },
	{ "autorol", NULL, rc_bool, &gOpts.autorol, "0", 0, 0, NULL, "automatically rotate screen counterclockwise 90 degrees if vertical" },
	{ "flipx", NULL, rc_bool, &gOpts.flipx, "0", 0, 0, NULL, "flip screen left-right" },
	{ "flipy", NULL, rc_bool, &gOpts.flipy, "0", 0, 0, NULL, "flip screen upside-down" },
	{ "brightness", NULL, rc_float, &gOpts.brightness, "1.0", 0.5, 2.0, NULL, "brightness correction" },
	{ "pause_brightness", NULL, rc_float, &gOpts.pause_brightness, "1.0", 0.5, 2.0, NULL, "additional pause brightness" },
#ifdef USE_SCALE_EFFECTS
	{ "scale_effect", NULL, rc_string, &gOpts.scale_effect, "none", 0, 0, NULL, "SaI scale effect" },
#endif /* USE_SCALE_EFFECTS */

	{ "CORE VECTOR OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "antialias", NULL, rc_bool, &gOpts.antialias, "1", 0, 0, NULL, "use antialiasing when drawing vectors" },
	{ "beam", NULL, rc_float, &gOpts.beam, "1.0", 0.1, 16.0, NULL, "set vector beam width" },
	{ "flicker", NULL, rc_float, &gOpts.flicker, "1.0", 0.0, 100.0, NULL, "set vector flicker effect" },
	{ "intensity", NULL, rc_float, &gOpts.intensity, "1.0", 0.5, 3.0, NULL, "set vector intensity" },

	{ "CORE SOUND OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "sound", NULL, rc_bool, &gOpts.sound, "1", 0, 0, NULL, "enable sound output" },
	{ "samplerate", NULL, rc_int, &gOpts.samplerate, "48000", 5000, 50000, NULL, "set sound output sample rate" },
	{ "samples", NULL, rc_bool, &gOpts.samples, "1", 0, 0, NULL, "enable the use of external samples if available" },
	{ "volume", NULL, rc_int, &gOpts.volume, "0", -32, 0, NULL, "sound volume in decibels (-32 min, 0 max)" },
#ifdef USE_VOLUME_AUTO_ADJUST
	{ "volume_adjust", NULL, rc_bool, &gOpts.volume_adjust, "0", 0, 0, NULL, "enable/disable volume auto adjust" },
#endif /* USE_VOLUME_AUTO_ADJUST */
	{ "audio_latency", NULL, rc_int, &gOpts.audio_latency, "1", 1, 4, NULL, "set audio latency (increase to reduce glitches)" },
	{ "wavwrite", NULL, rc_string, &gOpts.wavwrite, NULL, 0, 0, NULL, "save sound in wav file" },

	{ "CORE MISC OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "bios", NULL, rc_string, &gOpts.bios, "default", 0, 0, NULL, "select the system BIOS to use" },
	{ "cheat", NULL, rc_bool, &gOpts.cheat, "0", 0, 0, NULL, "enable cheat subsystem" },
	{ "skip_gameinfo", NULL, rc_bool, &gOpts.skip_gameinfo, "0", 0, 0, NULL, "skip displaying the information screen at startup" },
	{ "artwork", NULL, rc_bool, &gOpts.artwork, "1", 0, 0, NULL, "enable external artwork, if available" },
	{ "use_backdrops", NULL, rc_bool, &gOpts.use_backdrops, "1", 0, 0, NULL, "enable backdrops if artwork is enabled and available" },
	{ "use_overlays", NULL, rc_bool, &gOpts.use_overlays, "1", 0, 0, NULL, "enable overlays if artwork is enabled and available" },
	{ "use_bezels", NULL, rc_bool, &gOpts.use_bezels, "1", 0, 0, NULL, "enable bezels if artwork is enabled and available" },
#ifdef USE_IPS
	{ "ips", NULL, rc_string, &gOpts.ips, NULL, 0, 0, NULL, "ips datfile name" },
#endif /* USE_IPS */
	{ "disable_second_monitor", NULL, rc_bool, &gOpts.disable_second_monitor, "1", 0, 0, NULL, "" },
	{ "confirm_quit", NULL, rc_bool, &gOpts.confirm_quit, "1", 0, 0, NULL, "quit game with confirmation" },
#ifdef AUTO_PAUSE_PLAYBACK
	{ "auto_pause_playback", NULL, rc_bool, &gOpts.auto_pause_playback, "0", 0, 0, NULL, "automatic pause when playback is finished" },
#endif /* AUTO_PAUSE_PLAYBACK */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	{ "m68k_core", NULL, rc_int, &gOpts.m68k_core, "0", 0, 2, NULL, "change m68k core (0:C, 1:DRC, 2:ASM+DRC)" },
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef TRANS_UI
	{ "use_trans_ui", NULL, rc_bool, &gOpts.use_trans_ui, "1", 0, 0, NULL, "use transparent background for UI text" },
	{ "ui_transparency", NULL, rc_int, &gOpts.ui_transparency, "160", 0, 255, NULL, "transparency of UI background [0 - 255]" },
#endif /* TRANS_UI */

	{ "CORE STATE/PLAYBACK OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "playback", NULL, rc_string, &gOpts.playback, NULL, 0, 0, NULL, "playback an input file" },
	{ "record", NULL, rc_string, &gOpts.record, NULL, 0, 0, NULL, "record an input file" },
	{ "state", NULL, rc_string, &gOpts.state, NULL, 0, 0, NULL, "saved state to load" },
	{ "autosave", NULL, rc_bool, &gOpts.autosave, "0", 0, 0, NULL, "enable automatic restore at startup, and automatic save at exit time" },

	{ "CORE DEBUGGING OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "log", NULL, rc_bool, &gOpts.log, "0", 0, 0, NULL, "generate an error.log file" },
	{ "oslog", NULL, rc_bool, &gOpts.oslog, "0", 0, 0, NULL, "output error.log data to the system debugger" },
	{ "verbose", NULL, rc_bool, &gOpts.verbose, "0", 0, 0, NULL, "display additional diagnostic information" },

	{ "CORE CONFIGURATION OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "readconfig", NULL, rc_bool, &gOpts.readconfig, "0", 0, 0, NULL, "enable loading of configuration files" },

	{ "INPUT DEVICE OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "mouse", NULL, rc_bool, &gOpts.mouse, "0", 0, 0, NULL, "enable mouse input" },
	{ "joystick", NULL, rc_bool, &gOpts.joystick, "0", 0, 0, NULL, "enable joystick input" },
	{ "lightgun", NULL, rc_bool, &gOpts.lightgun, "0", 0, 0, NULL, "enable lightgun input" },
	{ "dual_lightgun", NULL, rc_bool, &gOpts.dual_lightgun, "0", 0, 0, NULL, "enable dual lightgun input" },
	{ "offscreen_reload", NULL, rc_bool, &gOpts.offscreen_reload, "0", 0, 0, NULL, "offscreen shots reload" },
	{ "steadykey", NULL, rc_bool, &gOpts.steadykey, "0", 0, 0, NULL, "enable steadykey support" },
	{ "keyboard_leds", NULL, rc_bool, &gOpts.keyboard_leds, "1", 0, 0, NULL, "enable keyboard LED emulation" },
	{ "led_mode", NULL, rc_string, &gOpts.led_mode, "ps/2", 0, 0, NULL, "LED mode (PS/2|USB)" },
	{ "a2d_deadzone", NULL, rc_float, &gOpts.a2d_deadzone, "0.3", 0.0, 1.0, NULL, "minimal analog value for digital input" },
	{ "ctrlr", NULL, rc_string, &gOpts.ctrlr, "Standard", 0, 0, NULL, "preconfigure for specified controller" },
#ifdef USE_JOY_MOUSE_MOVE
	{ "stickpoint", NULL, rc_bool, &gOpts.stickpoint, "0", 0, 0, NULL, "enable pointing stick input" },
#endif /* USE_JOY_MOUSE_MOVE */
#ifdef JOYSTICK_ID
	{ "joyid1", NULL, rc_int, &gOpts.joyid1, "0", 0, 0, NULL, "set joystick ID (Player1)" },
	{ "joyid2", NULL, rc_int, &gOpts.joyid2, "1", 0, 0, NULL, "set joystick ID (Player2)" },
	{ "joyid3", NULL, rc_int, &gOpts.joyid3, "2", 0, 0, NULL, "set joystick ID (Player3)" },
	{ "joyid4", NULL, rc_int, &gOpts.joyid4, "3", 0, 0, NULL, "set joystick ID (Player4)" },
	{ "joyid5", NULL, rc_int, &gOpts.joyid5, "4", 0, 0, NULL, "set joystick ID (Player5)" },
	{ "joyid6", NULL, rc_int, &gOpts.joyid6, "5", 0, 0, NULL, "set joystick ID (Player6)" },
	{ "joyid7", NULL, rc_int, &gOpts.joyid7, "6", 0, 0, NULL, "set joystick ID (Player7)" },
	{ "joyid8", NULL, rc_int, &gOpts.joyid8, "7", 0, 0, NULL, "set joystick ID (Player8)" },
#endif /* JOYSTICK_ID */
	{ "paddle_device", NULL, rc_string, &gOpts.paddle_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if a paddle control is present" },
	{ "adstick_device", NULL, rc_string, &gOpts.adstick_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if an analog joystick control is present" },
	{ "pedal_device", NULL, rc_string, &gOpts.pedal_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if a pedal control is present" },
	{ "dial_device", NULL, rc_string, &gOpts.dial_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if a dial control is present" },
	{ "trackball_device", NULL, rc_string, &gOpts.trackball_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if a trackball control is present" },
	{ "lightgun_device", NULL, rc_string, &gOpts.lightgun_device, "keyboard", 0, 0, NULL, "enable (keyboard|mouse|joystsick) if a lightgun control is present" },
	{ "digital", NULL, rc_string, &gOpts.digital, "none", 0, 0, NULL, "mark certain joysticks or axes as digital (none|all|j<N>*|j<N>a<M>[,...])" },

	{ "PERFORMANCE OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "autoframeskip", NULL, rc_bool, &gOpts.autoframeskip, "1", 0, 0, NULL, "enable automatic frameskip selection" },
	{ "frameskip", NULL, rc_int, &gOpts.frameskip, "0", 0, 12, NULL, "set frameskip to fixed value, 0-12 (autoframeskip must be disabled)" },
	{ "throttle", NULL, rc_bool, &gOpts.throttle, "1", 0, 0, NULL, "enable throttling to keep game running in sync with real time" },
	{ "sleep", NULL, rc_bool, &gOpts.sleep, "1", 0, 0, NULL, "enable sleeping, which gives time back to other applications when idle" },
	{ "rdtsc", NULL, rc_bool, &gOpts.rdtsc, "0", 0, 0, NULL, "use the RDTSC instruction for timing; faster but may result in uneven performance" },
	{ "priority", NULL, rc_int, &gOpts.priority, "0", -15, 1, NULL, "thread priority for the main game thread; range from -15 to 1" },

	{ "MISC VIDEO OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "frames_to_run", NULL, rc_int, &gOpts.frames_to_run, "0", 0, 0, NULL, "number of frames to run before automatically exiting" },
	{ "mngwrite", NULL, rc_string, &gOpts.mngwrite, NULL, 0, 0, NULL, "optional filename to write a MNG movie of the current session" },

	{ "GLOBAL VIDEO OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "window", NULL, rc_bool, &gOpts.window, "0", 0, 0, NULL, "enable window mode; otherwise, full screen mode is assumed" },
	{ "maximize", NULL, rc_bool, &gOpts.maximize, "1", 0, 0, NULL, "default to maximized windows; otherwise, windows will be minimized" },
	{ "numscreens", NULL, rc_int, &gOpts.numscreens, "1", 1, 4, NULL, "number of screens to create; usually, you want just one" },
	{ "extra_layout", NULL, rc_string, &gOpts.extra_layout, NULL, 0, 0, NULL, "name of an extra layout file to parse" },

	{ "PER-WINDOW VIDEO OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "screen0", NULL, rc_string, &gOpts.screen0, "auto", 0, 0, NULL, "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ "aspect0", NULL, rc_string, &gOpts.aspect0, "auto", 0, 0, NULL, "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ "resolution0", NULL, rc_string, &gOpts.resolution0, "auto", 0, 0, NULL, "preferred resolution of the first screen; format is <width>x<height>[x<depth>[@<refreshrate>]] or 'auto'" },
	{ "view0", NULL, rc_string, &gOpts.view0, "auto", 0, 0, NULL, "preferred view for the first screen" },
	{ "screen1", NULL, rc_string, &gOpts.screen1, "auto", 0, 0, NULL, "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ "aspect1", NULL, rc_string, &gOpts.aspect1, "auto", 0, 0, NULL, "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ "resolution1", NULL, rc_string, &gOpts.resolution1, "auto", 0, 0, NULL, "preferred resolution of the second screen; format is <width>x<height>[x<depth>[@<refreshrate>]] or 'auto'" },
	{ "view1", NULL, rc_string, &gOpts.view1, "auto", 0, 0, NULL, "preferred view for the second screen" },
	{ "screen2", NULL, rc_string, &gOpts.screen2, "auto", 0, 0, NULL, "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ "aspect2", NULL, rc_string, &gOpts.aspect2, "auto", 0, 0, NULL, "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ "resolution2", NULL, rc_string, &gOpts.resolution2, "auto", 0, 0, NULL, "preferred resolution of the third screen; format is <width>x<height>[x<depth>[@<refreshrate>]] or 'auto'" },
	{ "view2", NULL, rc_string, &gOpts.view2, "auto", 0, 0, NULL, "preferred view for the third screen" },
	{ "screen3", NULL, rc_string, &gOpts.screen3, "auto", 0, 0, NULL, "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ "aspect3", NULL, rc_string, &gOpts.aspect3, "auto", 0, 0, NULL, "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ "resolution3", NULL, rc_string, &gOpts.resolution3, "auto", 0, 0, NULL, "preferred resolution of the fourth screen; format is <width>x<height>[x<depth>[@<refreshrate>]] or 'auto'" },
	{ "view3", NULL, rc_string, &gOpts.view3, "auto", 0, 0, NULL, "preferred view for the fourth screen" },

	{ "DIRECTX VIDEO OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "direct3d", NULL, rc_bool, &gOpts.direct3d, "1", 0, 0, NULL, "enable using Direct3D 9 for video rendering if available (preferred)" },
	{ "d3dversion", NULL, rc_int, &gOpts.d3dversion, "9", 8, 9, NULL, "specify the preferred Direct3D version (8 or 9)" },
	{ "waitvsync", NULL, rc_bool, &gOpts.waitvsync, "0", 0, 0, NULL, "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	{ "syncrefresh", NULL, rc_bool, &gOpts.syncrefresh, "0", 0, 0, NULL, "enable using the start of VBLANK for throttling instead of the game time" },
	{ "triplebuffer", NULL, rc_bool, &gOpts.triplebuffer, "0", 0, 0, NULL, "enable triple buffering" },
	{ "switchres", NULL, rc_bool, &gOpts.switchres, "0", 0, 0, NULL, "enable resolution switching" },
	{ "filter", NULL, rc_bool, &gOpts.filter, "1", 0, 0, NULL, "enable bilinear filtering on screen output" },
	{ "full_screen_gamma", NULL, rc_float, &gOpts.full_screen_gamma, "1.0", 0.0, 4.0, NULL, "gamma value in full screen mode" },

	{ NULL,	NULL, rc_end, NULL, NULL, 0, 0,	NULL, NULL }
};

static struct rc_option rc_mamew_opts[] =
{
	{ "PATH AND DIRECTORY OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "rompath", NULL, rc_string, &settings.rompath, "roms", 0, 0, NULL, "path to ROMsets and hard disk images" },
	{ "samplepath", NULL, rc_string, &settings.samplepath, "samples", 0, 0, NULL, "path to samplesets" },
	{ "inipath", NULL, rc_string, &settings.inipath, ".;ini", 0, 0, NULL, "path to ini files" },
	{ "cfg_directory", NULL, rc_string, &settings.cfg_directory, "cfg", 0, 0, NULL, "directory to save configurations" },
	{ "nvram_directory", NULL, rc_string, &settings.nvram_directory, "nvram", 0, 0, NULL, "directory to save nvram contents" },
	{ "memcard_directory", NULL, rc_string, &settings.memcard_directory, "memcard", 0, 0, NULL, "directory to save memory card contents" },
	{ "input_directory", NULL, rc_string, &settings.input_directory, "inp", 0, 0, NULL, "directory to save input device logs" },
	{ "hiscore_directory", NULL, rc_string, &settings.hiscore_directory, "hi", 0, 0, NULL, "directory to save hiscores" },
	{ "state_directory", NULL, rc_string, &settings.state_directory, "sta", 0, 0, NULL, "directory to save states" },
	{ "artwork_directory", NULL, rc_string, &settings.artwork_directory, "artwork", 0, 0, NULL, "path to artwork files" },
	{ "snapshot_directory", NULL, rc_string, &settings.snapshot_directory, "snap", 0, 0, NULL, "directory to save screenshots" },
	{ "diff_directory", NULL, rc_string, &settings.diff_directory, "diff", 0, 0, NULL, "directory to save hard drive image difference files" },
	{ "ctrlr_directory", NULL, rc_string, &settings.ctrlr_directory, "ctrlr", 0, 0, NULL, "path to controller definitions" },
	{ "comment_directory", NULL, rc_string, &settings.comment_directory, "comments", 0, 0, NULL, "directory to save debugger comments" },
#ifdef USE_IPS
	{ "ips_directory", NULL, rc_string, &settings.ips_directory, "ips", 0, 0, NULL, "directory for ips files" },
#endif /* USE_IPS */
	{ "lang_directory", NULL, rc_string, &settings.lang_directory, "lang", 0, 0, NULL, "directory for localized data files" },
	{ "cheat_file", NULL, rc_string, &settings.cheat_file, "cheat.dat", 0, 0, NULL, "cheat filename" },
	{ "history_file", NULL, rc_string, &settings.history_file, "history.dat", 0, 0, NULL, "history database name" },
#ifdef STORY_DATAFILE
	{ "story_file", NULL, rc_string, &settings.story_file, "story.dat", 0, 0, NULL, "story database name" },
#endif /* STORY_DATAFILE */
	{ "mameinfo_file", NULL, rc_string, &settings.mameinfo_file, "mameinfo.dat", 0, 0, NULL, "mameinfo database name" },
	{ "hiscore_file", NULL, rc_string, &settings.hiscore_file, "hiscore.dat", 0, 0, NULL, "high score database name" },

	{ NULL, NULL, rc_link, rc_game_opts, NULL, 0,	0, NULL, NULL },

#ifdef UI_COLOR_DISPLAY
	{ "CORE PALETTE OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "font_blank", NULL, rc_string, &settings.font_blank, "0,0,0", 0, 0, NULL, "font blank color" },
	{ "font_normal", NULL, rc_string, &settings.font_normal, "255,255,255", 0, 0, NULL, "font normal color" },
	{ "font_special", NULL, rc_string, &settings.font_special, "247,203,0", 0, 0, NULL, "font special color" },
	{ "system_background", NULL, rc_string, &settings.system_background, "0,0,255", 0, 0, NULL, "window background color" },
	{ "system_framemedium", NULL, rc_string, &settings.system_framemedium, "192,192,192", 0, 0, NULL, "window frame color (medium)" },
	{ "system_framelight", NULL, rc_string, &settings.system_framelight, "224,224,224", 0, 0, NULL, "window frame color (light)" },
	{ "system_framedark", NULL, rc_string, &settings.system_framedark, "128,128,128", 0, 0, NULL, "window frame color (dark)" },
	{ "osdbar_framemedium", NULL, rc_string, &settings.osdbar_framemedium, "192,192,192", 0, 0, NULL, "OSD bar color (medium)" },
	{ "osdbar_framelight", NULL, rc_string, &settings.osdbar_framelight, "224,224,224", 0, 0, NULL, "OSD bar color (light)" },
	{ "osdbar_framedark", NULL, rc_string, &settings.osdbar_framedark, "128,128,128", 0, 0, NULL, "OSD bar color (dark)" },
	{ "osdbar_defaultbar", NULL, rc_string, &settings.osdbar_defaultbar, "60,120,240", 0, 0, NULL, "OSD bar color (default)" },
	{ "button_red", NULL, rc_string, &settings.button_red, "255,64,64", 0, 0, NULL, "button color (red)" },
	{ "button_yellow", NULL, rc_string, &settings.button_yellow, "255,238,0", 0, 0, NULL, "button color (yellow)" },
	{ "button_green", NULL, rc_string, &settings.button_green, "0,255,64", 0, 0, NULL, "button color (green)" },
	{ "button_blue", NULL, rc_string, &settings.button_blue, "0,170,255", 0, 0, NULL, "button color (blue)" },
	{ "button_purple", NULL, rc_string, &settings.button_purple, "170,0,255", 0, 0, NULL, "button color (purple)" },
	{ "button_pink", NULL, rc_string, &settings.button_pink, "255,0,170", 0, 0, NULL, "button color (pink)" },
	{ "button_aqua", NULL, rc_string, &settings.button_aqua, "0,255,204", 0, 0, NULL, "button color (aqua)" },
	{ "button_silver", NULL, rc_string, &settings.button_silver, "255,0,255", 0, 0, NULL, "button color (silver)" },
	{ "button_navy", NULL, rc_string, &settings.button_navy, "255,160,0", 0, 0, NULL, "button color (navy)" },
	{ "button_lime", NULL, rc_string, &settings.button_lime, "190,190,190", 0, 0, NULL, "button color (lime)" },
	{ "cursor", NULL, rc_string, &settings.cursor, "60,120,240", 0, 0, NULL, "cursor color" },
#endif /* UI_COLOR_DISPLAY */

	{ "CORE LANGUAGE OPTIONS", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "language", NULL, rc_string, &settings.language, "auto", 0, 0, NULL, "select translation language" },
	{ "use_lang_list", NULL, rc_bool, &settings.use_lang_list, "1", 0, 0, NULL, "enable/disable local language game list" },

	{ NULL,	NULL, rc_end, NULL, NULL, 0, 0,	NULL, NULL }
};

static struct rc_option rc_winui_opts[] =
{
	{ "Windows UI specific directory options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "flyer_directory", NULL, rc_string, &settings.flyerdir, "flyers", 0, 0, NULL, "directory for flyers" },
	{ "cabinet_directory", NULL, rc_string, &settings.cabinetdir, "cabinets", 0, 0, NULL, "directory for cabinets" },
	{ "marquee_directory", NULL, rc_string, &settings.marqueedir, "marquees", 0, 0, NULL, "directory for marquees" },
	{ "title_directory", NULL, rc_string, &settings.titlesdir, "titles", 0, 0, NULL, "directory for titles" },
	{ "cpanel_directory", NULL, rc_string, &settings.cpaneldir, "cpanel", 0, 0, NULL, "directory for control panel" },
	{ "icon_directory", NULL, rc_string, &settings.iconsdir, "icons", 0, 0, NULL, "directory for icons" },
	{ "bkground_directory", NULL, rc_string, &settings.bgdir, "bkground", 0, 0, NULL, "directory for bkground" },
	{ "folder_directory", NULL, rc_string, &settings.folderdir, "folders", 0, 0, NULL, "directory for folders-ini" },
#ifdef USE_VIEW_PCBINFO
	{ "pcbinfo_directory", NULL, rc_string, &settings.pcbinfodir, "pcb", 0, 0, NULL, "directory for pcb info" },
#endif /* USE_VIEW_PCBINFO */

	{ "Windows UI specific interface options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "save_version", NULL, rc_string, &rc_dummy_args.save_version, "", 0, 0, NULL, "save version" },
	{ "reset_gui", NULL, rc_bool, &bResetGUI, "0", 0, 0, NULL, "enable version mismatch warning" },
	{ "game_check", NULL, rc_bool, &settings.game_check, "1", 0, 0, NULL, "search for new games" },
	{ "joygui", NULL, rc_bool, &settings.use_joygui, "0", 0, 0, NULL, "allow game selection by a joystick" },
	{ "keygui", NULL, rc_bool, &settings.use_keygui, "0", 0, 0, NULL, "allow game selection by a keyboard" },
	{ "broadcast", NULL, rc_bool, &settings.broadcast, "0", 0, 0, NULL, "broadcast selected game to all windows" },
	{ "random_bg", NULL, rc_bool, &settings.random_bg, "1", 0, 0, NULL, "random select background image" },
	{ "cycle_screenshot", NULL, rc_int, &settings.cycle_screenshot, "0", 0, 99999, NULL, "cycle screen shot image" },
	{ "stretch_screenshot_larger", NULL, rc_bool, &settings.stretch_screenshot_larger, "0", 0, 0, NULL, "stretch screenshot larger" },
	{ "screenshot_bordersize", NULL, rc_int, &settings.screenshot_bordersize, "11", 0, 999, NULL, "screen shot border size" },
	{ "screenshot_bordercolor", NULL, rc_int, &settings.screenshot_bordercolor, "-1", -1, (UINT)-1, NULL, "screen shot border color" },
	{ "inherit_filter", NULL, rc_bool, &settings.inherit_filter, "0", 0, 0, NULL, "inheritable filters" },
	{ "offset_clones", NULL, rc_bool, &settings.offset_clones, "1", 0, 0, NULL, "no offset for clones missing parent in view" },
	{ "game_caption", NULL, rc_bool, &settings.game_caption, "1", 0, 0, NULL, "show game caption" },
#ifdef USE_SHOW_SPLASH_SCREEN
	{ "display_splash_screen", NULL, rc_bool, &settings.display_splash_screen, "0", 0, 0, NULL, "display splash screen on start" },
#endif /* USE_SHOW_SPLASH_SCREEN */

	{ "Windows UI specific general options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
#ifdef MESS
	{ "default_system", NULL, rc_string, &settings.default_game, "nes", 0, 0, NULL, "last selected system name" },
#else
	{ "default_game", NULL, rc_string, &settings.default_game, "puckman", 0, 0, NULL, "last selected game name" },
#endif
	{ "show_toolbar", NULL, rc_bool, &settings.show_toolbar, "1", 0, 0, NULL, "show tool bar" },
	{ "show_statusbar", NULL, rc_bool, &settings.show_statusbar, "1", 0, 0, NULL, "show status bar" },
	{ "show_folderlist", NULL, rc_bool, &settings.show_folderlist, "1", 0, 0, NULL, "show folder list" },
	{ "show_screenshot", NULL, rc_bool, &settings.show_screenshot, "1", 0, 0, NULL, "show image picture" },
	{ "show_screenshottab", NULL, rc_bool, &settings.show_tabctrl, "1", 0, 0, NULL, "show tab control" },
	{ "show_tab_flags", NULL, rc_int, &settings.show_tab_flags, "63", 0, 0, NULL, "show tab control flags" },
	{ "current_tab", NULL, rc_string, &settings.current_tab, "snapshot", 0, 0, NULL, "current image picture" },
#ifdef STORY_DATAFILE
	// TAB_ALL = 10
	{ "datafile_tab", NULL, rc_int, &settings.history_tab, "10", 0, MAX_TAB_TYPES+TAB_SUBTRACT, NULL, "where to show history on tab" },
#else /* STORY_DATAFILE */
	// TAB_ALL = 9
	{ "history_tab", NULL, rc_int, &settings.history_tab, "9", 0, MAX_TAB_TYPES+TAB_SUBTRACT, NULL, "where to show history on tab" },
#endif /* STORY_DATAFILE */
	{ "exec_command", NULL, rc_string, &settings.exec_command, NULL, 0, 0, NULL, "execute command line" },
	{ "exec_wait", NULL, rc_int, &settings.exec_wait, "0", 0, 0, NULL, "execute wait" },
	{ "hide_mouse", NULL, rc_bool, &settings.hide_mouse, "0", 0, 0, NULL, "hide mouse" },
	{ "full_screen", NULL, rc_bool, &settings.full_screen, "0", 0, 0, NULL, "full screen" },

	{ "Windows UI specific window position options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "window_x", NULL, rc_int, &settings.area.x, "0", 0, (UINT)-1, NULL, "window left position" },
	{ "window_y", NULL, rc_int, &settings.area.y, "0", 0, (UINT)-1, NULL, "window top position" },
	{ "window_width", NULL, rc_int, &settings.area.width, "640", 550, (UINT)-1, NULL, "window width" },
	{ "window_height", NULL, rc_int, &settings.area.height, "400", 400, (UINT)-1, NULL, "window height" },
	{ "window_state", NULL, rc_int, &settings.windowstate, "1", 0, 0, NULL, "window state" },

	{ "Windows UI specific list options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "list_mode", NULL, rc_string, &rc_dummy_args.view, "Grouped", 0, 0, ListDecodeString, "view mode" },
	{ "splitters", NULL, rc_string, &rc_dummy_args.splitter, "150,300", 0, 0, SplitterDecodeString, "splitter position" },
	/* re-arrange default column_width, column_order, sort_column */
	{ "column_widths", NULL, rc_string, &rc_dummy_args.column_width, "186,68,84,84,64,88,74,108,60,144,84,60", 0, 0, ColumnDecodeWidths, "column width settings" },
	{ "column_order", NULL, rc_string, &rc_dummy_args.column_order, "0,2,3,4,5,6,7,8,9,10,11,1", 0, 0, ColumnOrderDecodeString, "column order settings" },
	{ "column_shown", NULL, rc_string, &rc_dummy_args.column_shown, "1,0,1,1,1,1,1,1,1,1,1,1", 0, 0, ColumnShownDecodeString, "show or hide column settings" },
	{ "sort_column", NULL, rc_int, &settings.sort_column, "0", 0, COLUMN_MAX-1, NULL, "sort column" },
	{ "sort_reverse", NULL, rc_bool, &settings.sort_reverse, "0", 0, 0, NULL, "sort descending" },
	{ "folder_id", NULL, rc_int, &settings.folder_id, "0", 0, (UINT)-1, NULL, "last selected folder id" },

	{ "Windows UI specific list font options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "list_font", NULL, rc_string, &rc_dummy_args.list_font, "-8,0,0,0,400,0,0,0,0,0,0,0,0", 0, 0, FontDecodeString, "game list font size" },
	{ "list_fontface", NULL, rc_string, &rc_dummy_args.list_fontface, "MS Sans Serif", 0, 0, FontfaceDecodeString, "game list font face" },
	{ "use_broken_icon", NULL, rc_bool, &settings.use_broken_icon, "1", 0, 0, NULL, "use broken icon for not working games" },
	{ "font_color", NULL, rc_int, &settings.list_font_color, "-1", -1, (UINT)-1, NULL, "game list font color" },
	{ "clone_color", NULL, rc_int, &settings.list_clone_color, "8421504", -1, (UINT)-1, NULL, "clone game list font color" },
	{ "broken_color", NULL, rc_int, &settings.list_broken_color, "202", -1, (UINT)-1, NULL, "broken game list font color" },
	{ "custom_color", NULL, rc_string, &rc_dummy_args.custom_color, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", 0, 0, CusColorDecodeString, "custom colors" },

	{ "Windows UI specific GUI joystick options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "ui_joy_up", NULL, rc_string, &rc_dummy_args.ui_joy_up, "1,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_NEG", 0, 0, JoyInfoDecodeString, "joystick to up" },
	{ "ui_joy_down", NULL, rc_string, &rc_dummy_args.ui_joy_down, "1,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_POS", 0, 0, JoyInfoDecodeString, "joystick to down" },
	{ "ui_joy_left", NULL, rc_string, &rc_dummy_args.ui_joy_left, "1,JOYCODE_STICK_AXIS,1,JOYCODE_DIR_NEG", 0, 0, JoyInfoDecodeString, "joystick to left" },
	{ "ui_joy_right", NULL, rc_string, &rc_dummy_args.ui_joy_right, "1,JOYCODE_STICK_AXIS,1,JOYCODE_DIR_POS", 0, 0, JoyInfoDecodeString, "joystick to right" },
	{ "ui_joy_start", NULL, rc_string, &rc_dummy_args.ui_joy_start, "1,JOYCODE_STICK_BTN,1,JOYCODE_DIR_BTN", 0, 0, JoyInfoDecodeString, "joystick to start game" },
	{ "ui_joy_pgup", NULL, rc_string, &rc_dummy_args.ui_joy_pgup, "2,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_NEG", 0, 0, JoyInfoDecodeString, "joystick to page-up" },
	{ "ui_joy_pgdwn", NULL, rc_string, &rc_dummy_args.ui_joy_pgdwn, "2,JOYCODE_STICK_AXIS,2,JOYCODE_DIR_POS", 0, 0, JoyInfoDecodeString, "joystick to page-down" },
	{ "ui_joy_home", NULL, rc_string, &rc_dummy_args.ui_joy_home, NULL, 0, 0, JoyInfoDecodeString, "joystick to home" },
	{ "ui_joy_end", NULL, rc_string, &rc_dummy_args.ui_joy_end, NULL, 0, 0, JoyInfoDecodeString, "joystick to end" },
	{ "ui_joy_ss_change", NULL, rc_string, &rc_dummy_args.ui_joy_ss_change, "2,JOYCODE_STICK_BTN,3,JOYCODE_DIR_BTN", 0, 0, JoyInfoDecodeString, "joystick to change picture" },
	{ "ui_joy_history_up", NULL, rc_string, &rc_dummy_args.ui_joy_history_up, "2,JOYCODE_STICK_BTN,4,JOYCODE_DIR_BTN", 0, 0, JoyInfoDecodeString, "joystick to scroll history up" },
	{ "ui_joy_history_down",NULL, rc_string, &rc_dummy_args.ui_joy_history_down, "2,JOYCODE_STICK_BTN,1,JOYCODE_DIR_BTN", 0, 0, JoyInfoDecodeString, "joystick to scroll history down" },
	{ "ui_joy_exec", NULL, rc_string, &rc_dummy_args.ui_joy_exec, NULL, 0, 0, JoyInfoDecodeString, "joystick execute commandline" },

	{ "Windows UI specific GUI keyboard options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "ui_key_up", NULL, rc_string, &rc_dummy_args.ui_key_up, "KEYCODE_UP", 0, 0, KeySeqDecodeString, "keyboard to up" },
	{ "ui_key_down", NULL, rc_string, &rc_dummy_args.ui_key_down, "KEYCODE_DOWN", 0, 0, KeySeqDecodeString, "keyboard to down" },
	{ "ui_key_left", NULL, rc_string, &rc_dummy_args.ui_key_left, "KEYCODE_LEFT", 0, 0, KeySeqDecodeString, "keyboard to left" },
	{ "ui_key_right", NULL, rc_string, &rc_dummy_args.ui_key_right, "KEYCODE_RIGHT", 0, 0, KeySeqDecodeString, "keyboard to right" },
	{ "ui_key_start", NULL, rc_string, &rc_dummy_args.ui_key_start, "KEYCODE_ENTER NOT KEYCODE_LALT", 0, 0, KeySeqDecodeString, "keyboard to start game" },
	{ "ui_key_pgup", NULL, rc_string, &rc_dummy_args.ui_key_pgup, "KEYCODE_PGUP", 0, 0, KeySeqDecodeString, "keyboard to page-up" },
	{ "ui_key_pgdwn", NULL, rc_string, &rc_dummy_args.ui_key_pgdwn, "KEYCODE_PGDN", 0, 0, KeySeqDecodeString, "keyboard to page-down" },
	{ "ui_key_home", NULL, rc_string, &rc_dummy_args.ui_key_home, "KEYCODE_HOME", 0, 0, KeySeqDecodeString, "keyboard to home" },
	{ "ui_key_end", NULL, rc_string, &rc_dummy_args.ui_key_end, "KEYCODE_END", 0, 0, KeySeqDecodeString, "keyboard to end" },
	{ "ui_key_ss_change", NULL, rc_string, &rc_dummy_args.ui_key_ss_change, "KEYCODE_LALT KEYCODE_0", 0, 0, KeySeqDecodeString, "keyboard to change picture" },
	{ "ui_key_history_up", NULL, rc_string, &rc_dummy_args.ui_key_history_up, "KEYCODE_INSERT", 0, 0, KeySeqDecodeString, "keyboard to history up" },
	{ "ui_key_history_down", NULL, rc_string, &rc_dummy_args.ui_key_history_down, "KEYCODE_DEL", 0, 0, KeySeqDecodeString, "keyboard to history down" },

	{ "ui_key_context_filters", NULL, rc_string, &rc_dummy_args.ui_key_context_filters, "KEYCODE_LCONTROL KEYCODE_F", 0, 0, KeySeqDecodeString, "keyboard to context filters" },
	{ "ui_key_select_random", NULL, rc_string, &rc_dummy_args.ui_key_select_random, "KEYCODE_LCONTROL KEYCODE_R", 0, 0, KeySeqDecodeString, "keyboard to select random" },
	{ "ui_key_game_audit", NULL, rc_string, &rc_dummy_args.ui_key_game_audit, "KEYCODE_LALT KEYCODE_A", 0, 0, KeySeqDecodeString, "keyboard to game audit" },
	{ "ui_key_game_properties", NULL, rc_string, &rc_dummy_args.ui_key_game_properties, "KEYCODE_LALT KEYCODE_ENTER", 0, 0, KeySeqDecodeString, "keyboard to game properties" },
	{ "ui_key_help_contents", NULL, rc_string, &rc_dummy_args.ui_key_help_contents, "KEYCODE_F1", 0, 0, KeySeqDecodeString, "keyboard to help contents" },
	{ "ui_key_update_gamelist", NULL, rc_string, &rc_dummy_args.ui_key_update_gamelist, "KEYCODE_F5", 0, 0, KeySeqDecodeString, "keyboard to update game list" },
	{ "ui_key_view_folders", NULL, rc_string, &rc_dummy_args.ui_key_view_folders, "KEYCODE_LALT KEYCODE_D", 0, 0, KeySeqDecodeString, "keyboard to view folders" },
	{ "ui_key_view_fullscreen", NULL, rc_string, &rc_dummy_args.ui_key_view_fullscreen, "KEYCODE_F11", 0, 0, KeySeqDecodeString, "keyboard to full screen" },
	{ "ui_key_view_pagetab", NULL, rc_string, &rc_dummy_args.ui_key_view_pagetab, "KEYCODE_LALT KEYCODE_B", 0, 0, KeySeqDecodeString, "keyboard to view page tab" },
	{ "ui_key_view_picture_area", NULL, rc_string, &rc_dummy_args.ui_key_view_picture_area, "KEYCODE_LALT KEYCODE_P", 0, 0, KeySeqDecodeString, "keyboard to view picture area" },
	{ "ui_key_view_status", NULL, rc_string, &rc_dummy_args.ui_key_view_status, "KEYCODE_LALT KEYCODE_S", 0, 0, KeySeqDecodeString, "keyboard to view status" },
	{ "ui_key_view_toolbars", NULL, rc_string, &rc_dummy_args.ui_key_view_toolbars, "KEYCODE_LALT KEYCODE_T", 0, 0, KeySeqDecodeString, "keyboard to view toolbars" },

	{ "ui_key_view_tab_cabinet", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_cabinet, "KEYCODE_LALT KEYCODE_3", 0, 0, KeySeqDecodeString, "keyboard to view tab cabinet" },
	{ "ui_key_view_tab_cpanel", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_cpanel, "KEYCODE_LALT KEYCODE_6", 0, 0, KeySeqDecodeString, "keyboard to view tab control panel" },
	{ "ui_key_view_tab_flyer", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_flyer, "KEYCODE_LALT KEYCODE_2", 0, 0, KeySeqDecodeString, "keyboard to view tab flyer" },
	{ "ui_key_view_tab_history", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_history, "KEYCODE_LALT KEYCODE_7", 0, 0, KeySeqDecodeString, "keyboard to view tab history" },
#ifdef STORY_DATAFILE
	{ "ui_key_view_tab_story", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_story, "KEYCODE_LALT KEYCODE_8", 0, 0, KeySeqDecodeString, "keyboard to view tab story" },
#endif /* STORY_DATAFILE */
	{ "ui_key_view_tab_marquee", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_marquee, "KEYCODE_LALT KEYCODE_4", 0, 0, KeySeqDecodeString, "keyboard to view tab marquee" },
	{ "ui_key_view_tab_screenshot", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_screenshot, "KEYCODE_LALT KEYCODE_1", 0, 0, KeySeqDecodeString, "keyboard to view tab screen shot" },
	{ "ui_key_view_tab_title", NULL, rc_string, &rc_dummy_args.ui_key_view_tab_title, "KEYCODE_LALT KEYCODE_5", 0, 0, KeySeqDecodeString, "keyboard to view tab title" },
	{ "ui_key_quit", NULL, rc_string, &rc_dummy_args.ui_key_quit, "KEYCODE_LALT KEYCODE_Q", 0, 0, KeySeqDecodeString, "keyboard to quit application" },

	{ "Windows UI specific folder list hide options", NULL, rc_seperator, NULL, NULL, 0, 0, NULL, NULL },
	{ "folder_hide", NULL, rc_string, &rc_dummy_args.ui_hide_folder, NULL, 0, 0, HideFolderDecodeString, "hide selected item in folder list" },

	{FOLDERFLAG_OPT, NULL, rc_use_function, NULL, NULL, 0, 0, FolderFlagDecodeString, "folder list filters settings" },

	{ NULL,	NULL, rc_end, NULL, NULL, 0, 0,	NULL, NULL }
};

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
	{ SYSTEM_COLOR_FRAMEMEDIUM,  &settings.system_framemedium },
	{ SYSTEM_COLOR_FRAMELIGHT,  &settings.system_framelight },
	{ SYSTEM_COLOR_FRAMEDARK,  &settings.system_framedark },
	{ OSDBAR_COLOR_FRAMEMEDIUM,  &settings.osdbar_framemedium },
	{ OSDBAR_COLOR_FRAMELIGHT,  &settings.osdbar_framelight },
	{ OSDBAR_COLOR_FRAMEDARK,  &settings.osdbar_framedark },
	{ OSDBAR_COLOR_DEFAULTBAR,  &settings.osdbar_defaultbar },
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

/***************************************************************************
    External functions  
 ***************************************************************************/

void OptionsInit()
{
	game_variables_type default_variables;
	int i;

	num_games = GetNumGames();
	//code_init();
	settings.show_folder_flags = NewBits(MAX_FOLDERS);
	SetAllBits(settings.show_folder_flags,TRUE);

	if (!(rc_core = rc_create()))
		exit(1);

	if (rc_register(rc_core, rc_mamew_opts))
		exit(1);

	if (!(rc_game = rc_create()))
		exit(1);

	if (rc_register(rc_game, rc_game_opts))
		exit(1);

	global = gOpts;

	default_variables.play_count  = 0;
	default_variables.play_time = 0;
	default_variables.rom_audit_results = UNKNOWN;
	default_variables.samples_audit_results = UNKNOWN;
	default_variables.options_loaded = FALSE;
	default_variables.use_default = TRUE;
	default_variables.alt_index = -1;

	/* This allocation should be checked */
	game_options = (options_type *)malloc(num_games * sizeof(options_type));
	game_variables = (game_variables_type *)malloc(num_games * sizeof(game_variables_type));

	memset(game_options, 0, num_games * sizeof(options_type));
	for (i = 0; i < num_games; i++)
		game_variables[i] = default_variables;

	build_alt_options();
	build_default_bios();

	initialize_rc_winui_config();

	// Create Backup
	CopySettings(&settings, &backup.settings);
	CopyGameOptions(&global, &backup.global);

	LoadOptions();

	unify_alt_options();

	// have our mame core (file code) know about our rom path
	// this leaks a little, but the win32 file core writes to this string
	SetCorePathList(FILETYPE_ROM, settings.rompath);
	SetCorePathList(FILETYPE_SAMPLE, settings.samplepath);
#ifdef MESS
	SetCorePathList(FILETYPE_HASH, settings.mess.hashdir);
#endif
}

void OptionsExit(void)
{
	int i;

	for (i = 0; i < num_games; i++)
		FreeGameOptions(&game_options[i]);

	for (i = 0; i < num_alt_options; i++)
		FreeGameOptions(alt_options[i].option);

	free(game_options);
	free(game_variables);
	free(alt_options);

	FreeGameOptions(&global);
	FreeGameOptions(&backup.global);

	FreeSettings(&settings);

	free(rc_core);
	free(rc_game);

	for (i = 0; i < num_games; i++)
		free(rc_winui[i]->option->dest);
	free(rc_winui);

	free(folder_flags);
}

// needed to walk the tree
static void rc_free_strings(struct rc_option *option)
{
	int i;

	for (i = 0; option[i].type; i++)
	{
		switch (option[i].type)
		{
		case rc_link:
			rc_free_strings(option[i].dest);
			break;

		case rc_string:
			FreeIfAllocated((char **)option[i].dest);
			break;
		}
	}
}

// frees the sub-data (strings)
void FreeGameOptions(options_type *o)
{
	gOpts = *o;
	rc_free_strings(rc_game->option);
	*o = gOpts;
}

// needed to walk the tree
static void rc_duplicate_strings(struct rc_option *option)
{
	int i;

	for (i = 0; option[i].type; i++)
	{
		char **p;

		switch (option[i].type)
		{
		case rc_link:
			rc_duplicate_strings(option[i].dest);
			break;

		case rc_string:
			p = (char **)option[i].dest;
			if (*p)
				*p = strdup(*p);
			break;
		}
	}
}

// performs a "deep" copy--strings in source are allocated and copied in dest
void CopyGameOptions(const options_type *source, options_type *dest)
{
	gOpts = *source;
	rc_duplicate_strings(rc_game->option);
	*dest = gOpts;
}

// needed to walk the tree
// break *o1 and *o2
static int rc_compare_strings(struct rc_option *option, options_type *o1, options_type *o2)
{
	int i;

	for (i = 0; option[i].type; i++)
	{
		char **p;
		char *s;

		switch (option[i].type)
		{
		case rc_link:
			if (rc_compare_strings(option[i].dest, o1, o2))
				return 1;
			break;

		case rc_string:
			p = (char **)option[i].dest;

			gOpts = *o1;
			s = *p;

			gOpts = *o2;

			if (s != *p)
			{
				if (!s || !*p)
					return 1;

				if (strcmp(s, *p) != 0)
					return 1;
			}

			*p = s;
			*o2 = gOpts;
			break;
		}
	}

	return 0;
}

BOOL IsOptionEqual(options_type *o1, options_type *o2)
{
	options_type opt1, opt2;

	validate_game_option(o1);
	validate_game_option(o2);

	opt1 = *o1;
	opt2 = *o2;

	if (rc_compare_strings(rc_game->option, &opt1, &opt2))
		return FALSE;

	if (memcmp(&opt1, &opt2, sizeof (options_type)) == 0)
		return TRUE;

	return FALSE;
}

BOOL GetGameUsesDefaults(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	return game_variables[driver_index].use_default;
}

BOOL GetFolderUsesDefaults(const char *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return alt_options[alt_index].variable->use_default;
}

void SetGameUsesDefaults(int driver_index, BOOL use_defaults)
{
	assert (0 <= driver_index && driver_index < num_games);

	game_variables[driver_index].use_default = use_defaults;
}

void SetFolderUsesDefaults(const char *name, BOOL use_defaults)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	alt_options[alt_index].variable->use_default = use_defaults;
}

const char *GetUnifiedFolder(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	if (game_variables[driver_index].alt_index == -1)
		return NULL;

	return alt_options[game_variables[driver_index].alt_index].name;
}

int GetUnifiedDriver(const char *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return alt_options[alt_index].driver_index;
}

static options_type * GetAltOptions(alt_options_type *alt_option)
{
	if (alt_option->variable->use_default)
	{
		options_type *opt = GetDefaultOptions();
		char *bios = NULL;

#ifdef USE_IPS
		// HACK: DO NOT INHERIT IPS CONFIGURATION
		char *ips = alt_option->option->ips;

		alt_option->option->ips = NULL;
#endif /* USE_IPS */

		// if bios has been loaded, save it
		if (alt_option->option->bios)
			bios = strdup(alt_option->option->bios);

		// try vector.ini
		if (alt_option->need_vector_config)
			opt = GetVectorOptions();

		// free strings what will be never used now
		FreeGameOptions(alt_option->option);

		CopyGameOptions(opt,alt_option->option);

		// DO NOT OVERRIDE bios by default setting
		if (bios)
		{
			FreeIfAllocated(&alt_option->option->bios);
			alt_option->option->bios = bios;
		}

#ifdef USE_IPS
		alt_option->option->ips = ips;
#endif /* USE_IPS */
	}

	if (alt_option->variable->options_loaded == FALSE)
		LoadAltOptions(alt_option);

	return alt_option->option;
}

BOOL FolderHasVector(const char *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	return alt_options[alt_index].need_vector_config;
}

options_type * GetFolderOptions(const char *name)
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
	return GetFolderOptions("Vector");
}

options_type* GetSourceOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	return GetFolderOptions(GetDriverFilename(driver_index));
}

options_type* GetParentOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	if (DriverIsClone(driver_index))
		return GetGameOptions(DriverParentIndex(driver_index));

	return GetSourceOptions(driver_index);
}

options_type * GetGameOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	if (game_variables[driver_index].use_default)
	{
		options_type *opt = GetParentOptions(driver_index);
#ifdef USE_IPS
		// HACK: DO NOT INHERIT IPS CONFIGURATION
		char *ips = game_options[driver_index].ips;

		game_options[driver_index].ips = NULL;
#endif /* USE_IPS */

		// DO NOT OVERRIDE if game name is same as parent
		if (opt != &game_options[driver_index])
		{
			// free strings what will be never used now
			FreeGameOptions(&game_options[driver_index]);

			CopyGameOptions(opt,&game_options[driver_index]);
		}

#ifdef USE_IPS
		game_options[driver_index].ips = ips;
#endif /* USE_IPS */
	}

	if (game_variables[driver_index].options_loaded == FALSE)
		LoadGameOptions(driver_index);

	return &game_options[driver_index];
}

const game_driver *GetSystemBiosInfo(int bios_index)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	return default_bios[bios_index].drv;
}

const char *GetDefaultBios(int bios_index)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	if (default_bios[bios_index].drv)
	{
		options_type *opt = GetAltOptions(default_bios[bios_index].alt_option);

		return opt->bios;
	}

	return NULL;
}

void SetDefaultBios(int bios_index, const char *value)
{
	assert (0 <= bios_index && bios_index < MAX_SYSTEM_BIOS);

	if (default_bios[bios_index].drv)
	{
		options_type *opt = GetAltOptions(default_bios[bios_index].alt_option);

		FreeIfAllocated(&opt->bios);
		opt->bios = strdup(value);
	}
}

void ResetGUI(void)
{
	bResetGUI = TRUE;
}

int GetLangcode(void)
{
	if (settings.langcode < 0)
	{
		int langcode = lang_find_codepage(GetACP());
		return langcode < 0 ? UI_LANG_EN_US : langcode;
	}

	return settings.langcode;
}

void SetLangcode(int langcode)
{
	if (langcode >= UI_LANG_MAX)
		langcode = -1;
	else if (langcode >= 0)
	{
		UINT codepage = ui_lang_info[langcode].codepage;

		if (OnNT())
		{
			if (!IsValidCodePage(codepage))
			{
				fprintf(stderr, "codepage %d is not supported\n", ui_lang_info[langcode].codepage);
				langcode = -1;
			}
		}
		else if ((langcode != UI_LANG_EN_US) && (codepage != GetACP()))
		{
			fprintf(stderr, "codepage %d is not supported\n", ui_lang_info[langcode].codepage);
			langcode = -1;
		}
	}

	settings.langcode = langcode;

	/* apply to options.langcode for datafile.c */
	options.langcode = GetLangcode();
	InitTranslator(options.langcode);
}

BOOL UseLangList(void)
{
    return settings.use_lang_list;
}

void SetUseLangList(BOOL is_use)
{
    settings.use_lang_list = is_use;

    /* apply to options.use_lang_list for datafile.c */
    options.use_lang_list = is_use;
}

const char * GetImageTabLongName(int tab_index)
{
	return image_tabs_long_name[tab_index];
}

const char * GetImageTabShortName(int tab_index)
{
	return image_tabs_short_name[tab_index];
}

#ifdef UI_COLOR_DISPLAY
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
			*ui_palette_tbl[i].data = strdup(s);
		}
}
#endif /* UI_COLOR_DISPLAY */

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

void SetJoyGUI(BOOL use_joygui)
{
	settings.use_joygui = use_joygui;
}

BOOL GetJoyGUI(void)
{
	return settings.use_joygui;
}

void SetKeyGUI(BOOL use_keygui)
{
	settings.use_keygui = use_keygui;
}

BOOL GetKeyGUI(void)
{
	return settings.use_keygui;
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

void SetSavedFolderID(UINT val)
{
	settings.folder_id = val;
}

UINT GetSavedFolderID(void)
{
	return settings.folder_id;
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
	return TestBit(settings.show_folder_flags, folder);
}

void SetShowFolder(int folder, BOOL show)
{
	if (show)
		SetBit(settings.show_folder_flags, folder);
	else
		ClearBit(settings.show_folder_flags, folder);
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
	settings.show_tabctrl = val;
}

BOOL GetShowTabCtrl(void)
{
	return settings.show_tabctrl;
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
		settings.current_tab = strdup(shortname);
}

const char *GetCurrentTab(void)
{
	return settings.current_tab;
}

void SetDefaultGame(const char *name)
{
	FreeIfAllocated(&settings.default_game);

	if (name != NULL)
		settings.default_game = strdup(name);
}

const char *GetDefaultGame(void)
{
	return settings.default_game;
}

void SetWindowArea(AREA *area)
{
	memcpy(&settings.area, area, sizeof(AREA));
}

void GetWindowArea(AREA *area)
{
	memcpy(area, &settings.area, sizeof(AREA));
}

void SetWindowState(UINT state)
{
	settings.windowstate = state;
}

UINT GetWindowState(void)
{
	return settings.windowstate;
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

void SetListFont(LOGFONTA *font)
{
	memcpy(&settings.list_font, font, sizeof(LOGFONTA));
}

void GetListFont(LOGFONTA *font)
{
	memcpy(font, &settings.list_font, sizeof(LOGFONTA));
}

void SetListFontColor(COLORREF uColor)
{
	if (settings.list_font_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.list_font_color = (COLORREF)-1;
	else
		settings.list_font_color = uColor;
}

COLORREF GetListFontColor(void)
{
	if (settings.list_font_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.list_font_color;
}

void SetListCloneColor(COLORREF uColor)
{
	if (settings.list_clone_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.list_clone_color = (COLORREF)-1;
	else
		settings.list_clone_color = uColor;
}

COLORREF GetListCloneColor(void)
{
	if (settings.list_clone_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.list_clone_color;

}

void SetListBrokenColor(COLORREF uColor)
{
	if (settings.list_broken_color == GetSysColor(COLOR_WINDOWTEXT))
		settings.list_broken_color = (COLORREF)-1;
	else
		settings.list_broken_color = uColor;
}

COLORREF GetListBrokenColor(void)
{
	if (settings.list_broken_color == (COLORREF)-1)
		return (GetSysColor(COLOR_WINDOWTEXT));

	return settings.list_broken_color;

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
	return settings.history_tab;
}

void SetHistoryTab(int tab, BOOL show)
{
	if (show)
		settings.history_tab = tab;
	else
		settings.history_tab = TAB_NONE;
}

void SetColumnWidths(int width[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		settings.column_width[i] = width[i];
}

void GetColumnWidths(int width[])
{
	int i;

	for (i = 0; i < COLUMN_MAX; i++)
		width[i] = settings.column_width[i];
}

void SetSplitterPos(int splitterId, int pos)
{
	if (splitterId < GetSplitterCount())
		settings.splitter[splitterId] = pos;
}

int  GetSplitterPos(int splitterId)
{
	if (splitterId < GetSplitterCount())
		return settings.splitter[splitterId];

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

const char* GetRomDirs(void)
{
	return settings.rompath;
}

void SetRomDirs(const char* paths)
{
	FreeIfAllocated(&settings.rompath);

	if (paths != NULL)
	{
		settings.rompath = strdup(paths);

		// have our mame core (file code) know about it
		// this leaks a little, but the win32 file core writes to this string
		SetCorePathList(FILETYPE_ROM, settings.rompath);
	}
}

const char* GetSampleDirs(void)
{
	return settings.samplepath;
}

void SetSampleDirs(const char* paths)
{
	FreeIfAllocated(&settings.samplepath);

	if (paths != NULL)
	{
		settings.samplepath = strdup(paths);
		
		// have our mame core (file code) know about it
		// this leaks a little, but the win32 file core writes to this string
		SetCorePathList(FILETYPE_SAMPLE, settings.samplepath);
	}
}

const char* GetIniDir(void)
{
	return settings.inipath;
}

void SetIniDir(const char* path)
{
	if (!strcmp(path, settings.inipath))
		return;

	FreeIfAllocated(&settings.inipath);

	if (path != NULL)
		settings.inipath = strdup(path);

	if (MessageBox(0, _Unicode(reload_config_msg), TEXT("Reload configurations"), MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		int i;

		for (i = 0 ; i < num_games; i++)
			LoadGameOptions(i);
	}

	_mkdir(path);
}

const char* GetCtrlrDir(void)
{
	return settings.ctrlr_directory;
}

void SetCtrlrDir(const char* path)
{
	FreeIfAllocated(&settings.ctrlr_directory);

	if (path != NULL)
		settings.ctrlr_directory = strdup(path);
}

const char* GetCfgDir(void)
{
	return settings.cfg_directory;
}

void SetCfgDir(const char* path)
{
	FreeIfAllocated(&settings.cfg_directory);

	if (path != NULL)
		settings.cfg_directory = strdup(path);
}

const char* GetHiDir(void)
{
	return settings.hiscore_directory;
}

void SetHiDir(const char* path)
{
	FreeIfAllocated(&settings.hiscore_directory);

	if (path != NULL)
		settings.hiscore_directory = strdup(path);
}

const char* GetNvramDir(void)
{
	return settings.nvram_directory;
}

void SetNvramDir(const char* path)
{
	FreeIfAllocated(&settings.nvram_directory);

	if (path != NULL)
		settings.nvram_directory = strdup(path);
}

const char* GetInpDir(void)
{
	return settings.input_directory;
}

void SetInpDir(const char* path)
{
	FreeIfAllocated(&settings.input_directory);

	if (path != NULL)
		settings.input_directory = strdup(path);
}

const char* GetImgDir(void)
{
	return settings.snapshot_directory;
}

void SetImgDir(const char* path)
{
	FreeIfAllocated(&settings.snapshot_directory);

	if (path != NULL)
		settings.snapshot_directory = strdup(path);
}

const char* GetStateDir(void)
{
	return settings.state_directory;
}

void SetStateDir(const char* path)
{
	FreeIfAllocated(&settings.state_directory);

	if (path != NULL)
		settings.state_directory = strdup(path);
}

const char* GetArtDir(void)
{
	return settings.artwork_directory;
}

void SetArtDir(const char* path)
{
	FreeIfAllocated(&settings.artwork_directory);

	if (path != NULL)
		settings.artwork_directory = strdup(path);
}

const char* GetMemcardDir(void)
{
	return settings.memcard_directory;
}

void SetMemcardDir(const char* path)
{
	FreeIfAllocated(&settings.memcard_directory);

	if (path != NULL)
		settings.memcard_directory = strdup(path);
}

const char* GetFlyerDir(void)
{
	return settings.flyerdir;
}

void SetFlyerDir(const char* path)
{
	FreeIfAllocated(&settings.flyerdir);

	if (path != NULL)
		settings.flyerdir = strdup(path);
}

const char* GetCabinetDir(void)
{
	return settings.cabinetdir;
}

void SetCabinetDir(const char* path)
{
	FreeIfAllocated(&settings.cabinetdir);

	if (path != NULL)
		settings.cabinetdir = strdup(path);
}

const char* GetMarqueeDir(void)
{
	return settings.marqueedir;
}

void SetMarqueeDir(const char* path)
{
	FreeIfAllocated(&settings.marqueedir);

	if (path != NULL)
		settings.marqueedir = strdup(path);
}

const char* GetTitlesDir(void)
{
	return settings.titlesdir;
}

void SetTitlesDir(const char* path)
{
	FreeIfAllocated(&settings.titlesdir);

	if (path != NULL)
		settings.titlesdir = strdup(path);
}

const char * GetControlPanelDir(void)
{
	return settings.cpaneldir;
}

void SetControlPanelDir(const char *path)
{
	FreeIfAllocated(&settings.cpaneldir);
	if (path != NULL)
		settings.cpaneldir = strdup(path);
}

const char* GetDiffDir(void)
{
	return settings.diff_directory;
}

void SetDiffDir(const char* path)
{
	FreeIfAllocated(&settings.diff_directory);

	if (path != NULL)
		settings.diff_directory = strdup(path);
}

const char* GetCommentDir(void)
{
	return settings.comment_directory;
}

void SetCommentDir(const char* path)
{
	FreeIfAllocated(&settings.comment_directory);

	if (path != NULL)
		settings.comment_directory = strdup(path);
}

#ifdef USE_IPS
const char *GetPatchDir(void)
{
	return settings.ips_directory;
}

void SetPatchDir(const char *path)
{
	FreeIfAllocated(&settings.ips_directory);

	if (path != NULL)
		settings.ips_directory = strdup(path);
}
#endif /* USE_IPS */

const char* GetLangDir(void)
{
	return settings.lang_directory;
}

void SetLangDir(const char* path)
{
	FreeIfAllocated(&settings.lang_directory);

	if (path != NULL)
		settings.lang_directory = strdup(path);
}

const char* GetIconsDir(void)
{
	return settings.iconsdir;
}

void SetIconsDir(const char* path)
{
	FreeIfAllocated(&settings.iconsdir);

	if (path != NULL)
		settings.iconsdir = strdup(path);
}

const char* GetBgDir(void)
{
	return settings.bgdir;
}

void SetBgDir(const char* path)
{
	FreeIfAllocated(&settings.bgdir);

	if (path != NULL)
		settings.bgdir = strdup(path);
}

const char *GetFolderDir(void)
{
	return settings.folderdir;
}

void SetFolderDir(const char *path)
{
	FreeIfAllocated(&settings.folderdir);

	if (path != NULL)
		settings.folderdir = strdup(path);
}

const char* GetCheatFile(void)
{
	return settings.cheat_file;
}

void SetCheatFile(const char* path)
{
	FreeIfAllocated(&settings.cheat_file);

	if (path != NULL)
		settings.cheat_file = strdup(path);
}

const char* GetHistoryFile(void)
{
	return settings.history_file;
}

void SetHistoryFile(const char* path)
{
	FreeIfAllocated(&settings.history_file);

	if (path != NULL)
		settings.history_file = strdup(path);
}

#ifdef STORY_DATAFILE
const char* GetStoryFile(void)
{
	return settings.story_file;
}

void SetStoryFile(const char* path)
{
	FreeIfAllocated(&settings.story_file);

	if (path != NULL)
		settings.story_file = strdup(path);
}
#endif /* STORY_DATAFILE */

#ifdef USE_VIEW_PCBINFO
const char* GetPcbinfoDir(void)
{
	return settings.pcbinfodir;
}

void SetPcbinfoDir(const char* path)
{
	FreeIfAllocated(&settings.pcbinfodir);

	if (path != NULL)
		settings.pcbinfodir = mame_strdup(path);
}
#endif /* USE_VIEW_PCBINFO */

const char* GetMAMEInfoFile(void)
{
	return settings.mameinfo_file;
}

void SetMAMEInfoFile(const char* path)
{
	FreeIfAllocated(&settings.mameinfo_file);

	if (path != NULL)
		settings.mameinfo_file = strdup(path);
}

const char* GetHiscoreFile(void)
{
	return settings.hiscore_file;
}

void SetHiscoreFile(const char* path)
{
	FreeIfAllocated(&settings.hiscore_file);

	if (path != NULL)
		settings.hiscore_file = strdup(path);
}

void ResetGameOptions(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	// make sure it's all loaded up.
	GetGameOptions(driver_index);

	if (!game_variables[driver_index].use_default)
	{
		FreeGameOptions(&game_options[driver_index]);
		game_variables[driver_index].use_default = TRUE;
		
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

	for (i = 0; i < num_games; i++)
		ResetGameOptions(i);

	for (i = 0; i < num_alt_options; i++)
		ResetAltOptions(&alt_options[i]);
}

int GetRomAuditResults(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	return game_variables[driver_index].rom_audit_results;
}

void SetRomAuditResults(int driver_index, int audit_results)
{
	assert(0 <= driver_index && driver_index < num_games);

	game_variables[driver_index].rom_audit_results = audit_results;
}

int GetSampleAuditResults(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	return game_variables[driver_index].samples_audit_results;
}

void SetSampleAuditResults(int driver_index, int audit_results)
{
	assert(0 <= driver_index && driver_index < num_games);

	game_variables[driver_index].samples_audit_results = audit_results;
}

void IncrementPlayCount(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	game_variables[driver_index].play_count++;

	// maybe should do this
	//SavePlayCount(driver_index);
}

int GetPlayCount(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	return game_variables[driver_index].play_count;
}

void ResetPlayCount(int driver_index)
{
	int i = 0;
	assert(driver_index < num_games);
	if ( driver_index < 0 )
	{
		//All games
		for ( i= 0; i< num_games; i++ )
			game_variables[i].play_count = 0;
	}
	else
	{
		game_variables[driver_index].play_count = 0;
	}
}

void ResetPlayTime(int driver_index)
{
	int i = 0;
	assert(driver_index < num_games);
	if ( driver_index < 0 )
	{
		//All games
		for ( i= 0; i< num_games; i++ )
			game_variables[i].play_time = 0;
	}
	else
	{
		game_variables[driver_index].play_time = 0;
	}
}

int GetPlayTime(int driver_index)
{
	assert(0 <= driver_index && driver_index < num_games);

	return game_variables[driver_index].play_time;
}

void IncrementPlayTime(int driver_index, int playtime)
{
	assert(0 <= driver_index && driver_index < num_games);
	game_variables[driver_index].play_time += playtime;
}

void GetTextPlayTime(int driver_index, char *buf)
{
	int hour, minute, second;
	int temp = game_variables[driver_index].play_time;

	assert(0 <= driver_index && driver_index < num_games);

	hour = temp / 3600;
	temp = temp - 3600*hour;
	minute = temp / 60; //Calc Minutes
	second = temp - 60*minute;

	if (hour == 0)
		sprintf(buf, "%d:%02d", minute, second );
	else
		sprintf(buf, "%d:%02d:%02d", hour, minute, second );
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

char * GetExecCommand(void)
{
	static char empty = '\0';

	if (settings.exec_command)
		return settings.exec_command;

	return &empty;
}

void SetExecCommand(char *cmd)
{
	settings.exec_command = cmd;
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

void SetFolderFlags(const char *folderName, DWORD dwFlags)
{
	SaveFolderFlags(folderName, dwFlags);
}

DWORD GetFolderFlags(const char *folderName)
{
	int i;

	for (i = 0; i < num_folder_flags; i++)
		if (!strcmp(folderName, folder_flags[i].name))
			return folder_flags[i].flags;

	return 0;
}

void SaveGameOptions(int driver_index)
{
	int i;

	assert (0 <= driver_index && driver_index < num_games);

	rc_save_game_config(driver_index);

	for (i = 0; i < num_games; i++)
		if (DriverParentIndex(i) == driver_index)
		{
			game_variables[i].use_default = TRUE;
			game_variables[i].options_loaded = FALSE;
		}
}

static void InvalidateGameOptionsInDriver(const char *name)
{
	int i;

	for (i = 0; i < num_games; i++)
	{
		if (game_variables[i].alt_index != -1)
			continue;

		if (strcmp(GetDriverFilename(i), name) == 0)
		{
			game_variables[i].use_default = TRUE;
			game_variables[i].options_loaded = FALSE;
		}
	}
}

static void SaveAltOptions(alt_options_type *alt_option)
{
	rc_save_alt_config(alt_option);

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

void SaveFolderOptions(const char *name)
{
	int alt_index = bsearch_alt_option(name);

	assert (0 <= alt_index && alt_index < num_alt_options);

	SaveAltOptions(&alt_options[alt_index]);
}

void SaveDefaultOptions(void)
{
	int i;

	rc_save_default_config();

	for (i = 0; i < num_alt_options; i++)
	{
		alt_options[i].variable->use_default = TRUE;
		alt_options[i].variable->options_loaded = FALSE;
	}

	for (i = 0; i < num_games; i++)
	{
		game_variables[i].use_default = TRUE;
		game_variables[i].options_loaded = FALSE;
	}

	/* default option has bios tab. so save default bios */
	for (i = 0; i < num_alt_options; i++)
		if (alt_options[i].option->bios)
		{
			char *bios = strdup(alt_options[i].option->bios);

			GetAltOptions(&alt_options[i]);

			FreeIfAllocated(&alt_options[i].option->bios);
			alt_options[i].option->bios = bios;

			rc_save_alt_config(&alt_options[i]);
		}
}

void SaveOptions(void)
{
	int i;

	rc_save_winui_config();
	rc_save_default_config();

	for (i = 0; i < num_games; i++)
		rc_save_game_config(i);

	for (i = 0; i < num_alt_options; i++)
		rc_save_alt_config(&alt_options[i]);
}

/***************************************************************************
    Internal functions
 ***************************************************************************/

static int regist_alt_option(const char *name)
{
	int s = 0;
	int n = num_alt_options;

	while (n > 0)
	{
		int n2 = n / 2;
		int result;

		if (name == alt_options[s + n2].name)
			return -1;

		result = strcmp(name, alt_options[s + n2].name);
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
		alt_options = (alt_options_type *)realloc(alt_options, alt_options_len * sizeof (alt_options_type));

		if (!alt_options)
			exit(0);
	}

	for (n = num_alt_options++; n > s; n--)
		alt_options[n].name = alt_options[n - 1].name;

	alt_options[s].name = name;

	return s;
}

static int bsearch_alt_option(const char *name)
{
	int s = 0;
	int n = num_alt_options;

	while (n > 0)
	{
		int n2 = n / 2;
		int result;

		result = strcmp(name, alt_options[s + n2].name);
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
	int i;

	for (i = 0; i < num_games; i++)
	{
		if (drivers[i]->bios)
		{
			const game_driver *drv = drivers[i];
			int n;

			while (!(drv->flags & NOT_A_DRIVER) && driver_get_clone(drv))
				drv = driver_get_clone(drv);

			for (n = 0; n < MAX_SYSTEM_BIOS; n++)
			{
				if (default_bios[n].drv == NULL)
				{
					int alt_index = bsearch_alt_option(GetFilename(drv->source_file));

					assert(0 <= alt_index && alt_index < num_alt_options);

					default_bios[n].drv = drv;
					default_bios[n].alt_option = &alt_options[alt_index];
					default_bios[n].alt_option->has_bios = TRUE;
					break;
				}
				else if (default_bios[n].drv == drv)
					break;
			}
		}
	}

}

static void build_alt_options(void)
{
	options_type *pOpts;
	game_variables_type *pVars;
	int i;

	alt_options = (alt_options_type *)malloc(alt_options_len * sizeof (alt_options_type));
	num_alt_options = 0;

	if (!alt_options)
		exit(0);

	regist_alt_option("Vector");

	for (i = 0; i < num_games; i++)
		regist_alt_option(GetDriverFilename(i));

	pOpts = (options_type *)malloc(num_alt_options * sizeof (options_type));
	pVars = (game_variables_type *)malloc(num_alt_options * sizeof (game_variables_type));

	if (!pOpts || !pVars)
		exit(0);

	memset(pOpts, 0, num_alt_options * sizeof (options_type));
	memset(pVars, 0, num_alt_options * sizeof (game_variables_type));

	for (i = 0; i < num_alt_options; i++)
	{
		alt_options[i].option = &pOpts[i];
		alt_options[i].variable = &pVars[i];
		alt_options[i].variable->options_loaded = FALSE;
		alt_options[i].variable->use_default = TRUE;
		alt_options[i].has_bios = FALSE;
		alt_options[i].need_vector_config = FALSE;
		alt_options[i].driver_index = -1;
	}

	for (i = 0; i < num_games; i++)
	{
		const char *src = GetDriverFilename(i);
		int n = bsearch_alt_option(src);

		if (!alt_options[n].need_vector_config && DriverIsVector(i))
			alt_options[n].need_vector_config = TRUE;
	}
}

static void  unify_alt_options(void)
{
	int i;

	for (i = 0; i < num_games; i++)
	{
		char buf[16];
		int n;

		sprintf(buf, "%s.c", drivers[i]->name);
		n = bsearch_alt_option(buf);
		if (n == -1)
			continue;

		dprintf("Unify %s", drivers[i]->name);

		game_variables[i].alt_index = n;
		alt_options[n].option = &game_options[i];
		alt_options[n].variable = &game_variables[i];
		alt_options[n].driver_index = i;
	}
}

static int initialize_rc_winui_config(void)
{
	static char unknown[3];
	static struct rc_option flag_opts[] =
	{
		{ "%s_playcount", NULL, rc_int, NULL, "0", 0, (UINT32)-1, NULL, "Play Counts" },
		{ "%s_play_time", NULL, rc_int, NULL, "0", 0, (UINT32)-1, NULL, "Play Time" },
		{ "%s_rom_audit", NULL, rc_int, NULL, unknown, -1, 5, NULL, "Has Roms" },
		{ "%s_samples_audit", NULL, rc_int, NULL, unknown, -1, 5, NULL, "Has Samples" },
		{ NULL,	NULL, rc_end, NULL, NULL, 0, 0,	NULL, NULL }
	};
	struct rc_option *rc;
	int i, j;

#define REGIST_GAME_OPT(n, item)			\
	rc[n].name = buf;				\
	rc[n].dest = &game_variables[i].item;		\
	buf += strlen(buf) + 1;

#define NUM_FLAG_OPTS	(sizeof flag_opts / sizeof *flag_opts)

	sprintf(unknown, "%d", UNKNOWN);

	rc_winui = (struct rc_struct **)malloc(sizeof(struct rc_struct *) * (num_games + 1));
	if (!rc_winui)
		exit(0);

	if (!(rc_winui[num_games] = rc_create()))
		exit(0);

	if (rc_register(rc_winui[num_games], rc_winui_opts))
		exit(0);

	SetLangcode(settings.langcode);
	SetUseLangList(UseLangList());

	/* Setup default font */
	GetTranslatedFont(&settings.list_font);

	for (i = 0; i < num_games; i++)
	{
		char work[NUM_FLAG_OPTS * 32];
		char *buf;
		size_t name_size;

		if (!(rc_winui[i] = rc_create()))
			exit(0);

		name_size = 0;

		for (j = 0; j < NUM_FLAG_OPTS; j++)
		{
			if (flag_opts[j].name == NULL)
				break;
			sprintf(work + name_size, flag_opts[j].name, drivers[i]->name);
			name_size += strlen(work + name_size) + 1;
		}

		buf = malloc(sizeof flag_opts + name_size);
		if (!buf)
			exit(0);

		rc = (struct rc_option *)buf;
		buf += sizeof flag_opts;

		memcpy(buf, work, name_size);

		for (j = 0; j < NUM_FLAG_OPTS; j++)
			rc[j] = flag_opts[j];

		REGIST_GAME_OPT(0, play_count);
		REGIST_GAME_OPT(1, play_time);
		REGIST_GAME_OPT(2, rom_audit_results);
		REGIST_GAME_OPT(3, samples_audit_results);

		if (rc_register(rc_winui[i], rc))
			exit(0);
	}

	return 0;
}

static const char *get_base_config_directory(void)
{
	char full[_MAX_PATH];
	char dir[_MAX_DIR];
	static char path[_MAX_PATH];

	GetModuleFileNameA(GetModuleHandle(NULL), full, _MAX_PATH);
	_splitpath(full, path, dir, NULL, NULL);
	strcat(path, dir);

	if (path[strlen(path) - 1] == '\\')
		path[strlen(path) - 1] = '\0';

	return path;
}

static int rc_load_winui_config(void)
{
	const char *filename;
	char buf[1024];
	mame_file *file;
	int i = num_games;
	int line = 0;

	SetCorePathList(FILETYPE_INI, settings.inipath);
	filename = strlower(WINUI_INI);

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 0)))
		return 0;

	FreeIfAllocated(&rc_dummy_args.save_version);
	rc_dummy_args.save_version = strdup("(unknown)");

	while (mame_fgets(buf, sizeof buf, file))
	{
		struct rc_option *option = NULL;
		char *name, *tmp, *arg = NULL;
		int last;

		line ++;

		/* get option name */
		if (!(name = strtok(buf, " \t\r\n")))
			continue;
		if (name[0] == '#')
			continue;

		/* get complete rest of line */
		arg = strtok(NULL, "\r\n");

		/* ignore white space */
		for (; (*arg == '\t' || *arg == ' '); arg++)
			;

		/* deal with quotations */
		if (arg[0] == '"')
			arg = strtok(arg, "\"");
		else if (arg[0] == '\'')
			arg = strtok(arg, "'");
		else
			arg = strtok(arg, " \t\r\n");

		if (!arg)
		{
			fprintf(stderr,
				_WINDOWS("error: %s requires an argument, on line %d of file: %s\n"),
				name, line, filename);
				continue;
		}

		last = i;
		for (; i <= num_games; i++)
			if ((option = rc_get_option(rc_winui[i], name)) != NULL)
				break;

		if (!option)
			for (i = 0; i < last; i++)
				if ((option = rc_get_option(rc_winui[i], name)) != NULL)
					break;

		if (!option)
		{
			fprintf(stderr, _WINDOWS("error: unknown option %s, on line %d of file: %s\n"),
				name, line, filename);
			i = last;
			continue;
		}
		else if ((tmp = strtok(NULL, " \t\r\n")) && (tmp[0] != '#') )
		{
			fprintf(stderr,
				_WINDOWS("error: trailing garbage: \"%s\" on line: %d of file: %s\n"),
				tmp, line, filename);
		}
		else if (!rc_set_option3(option, arg, 1))
			continue;

		fprintf(stderr, _WINDOWS("   ignoring line\n"));
	}

	mame_fclose(file);

	return 0;
}

static int rc_save_winui_config(void)
{
	const char *filename;
	mame_file *file;
	int i;

        mkdir(settings.inipath);
	SetCorePathList(FILETYPE_INI, settings.inipath);
	filename = strlower(WINUI_INI);

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 1)))
		return 0;

	FreeIfAllocated(&rc_dummy_args.save_version);
	rc_dummy_args.save_version = strdup(GetVersionString());

	ColumnOrderEncodeString();
	ColumnShownEncodeString();
	ColumnEncodeWidths();
	CusColorEncodeString();
	SplitterEncodeString();
	ListEncodeString();
	FontEncodeString();
	FontfaceEncodeString();
	HideFolderEncodeString();
	JoyInfoEncodeString();
	KeySeqEncodeString();

	osd_rc_write(rc_winui[num_games], file, filename);

	rc_write_folder_flags(file);

	for (i = 0; i < num_games; i++)
	{
		if ((game_variables[i].rom_audit_results == UNKNOWN)
		 && (game_variables[i].samples_audit_results == UNKNOWN)
		 && (game_variables[i].play_count == 0))
			continue;

		osd_rc_write(rc_winui[i], file, drivers[i]->description);
	}

	mame_fclose(file);

	return 0;
}

static int rc_write_folder_flags(mame_file *file)
{
	int found = 0;
	int i;

	mame_fprintf(file, "### %s ###\r\n", _UI("Windows UI specific folder list filters options"));

	for (i = 0; i < num_folder_flags; i++)
		if (folder_flags[i].name[0] != '\0')
		{
			found = 1;
			mame_fprintf(file, "%-21s   ", FOLDERFLAG_OPT);
			mame_fprintf(file, "\"%s,%ld\"\r\n", folder_flags[i].name, folder_flags[i].flags);
		}

	if (!found)
		mame_fprintf(file, "# %-19s   <NULL> (not set)\r\n", FOLDERFLAG_OPT);

	mame_fprintf(file, "\r\n");

	return 0;
}

static int rc_load_default_config(void)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int retval;

	SetCorePathList(FILETYPE_INI, get_base_config_directory());
	strcpy(filename, MAME_INI);

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 0)))
		return 0;

	sprintf(filename, "%s", MAME_INI);

	gOpts = global;
	retval = osd_rc_read(rc_core, file, filename, 1, 1);
	global = gOpts;

	mame_fclose(file);

	LanguageDecodeString();

	return retval;
}

static int rc_save_default_config(void)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int retval;

	gOpts = global;
	validate_game_option(&gOpts);
	LanguageEncodeString();

	SetCorePathList(FILETYPE_INI, get_base_config_directory());
	strcpy(filename, strlower(MAME_INI));

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 1)))
		return -1;

	retval = osd_rc_write(rc_core, file, filename);

	mame_fclose(file);

	return retval;
}

static void validate_game_option(options_type *opt)
{
	if (!strcmp(opt->resolution0, "0x0x0@0"))
	{
		FreeIfAllocated(&opt->resolution0);
		opt->resolution0 = strdup("auto");
	}

	if (DirectDraw_GetNumDisplays() < 2)
		FreeIfAllocated(&opt->screen0);
}

static int rc_game_is_changed(struct rc_option *option, void *param)
{
	int offset = (UINT8 *)option->dest - (UINT8 *)&gOpts;
	void *compare = (UINT8 *)param + offset;
	int retval = 0;

	if (offset < 0 || offset >= sizeof (gOpts))
	{
		retval = 1;
	}
	else
	{
		switch (option->type)
		{
		case rc_string:
#ifdef USE_IPS
			// HACK: DO NOT INHERIT IPS CONFIGURATION
			if (option->dest == &gOpts.ips)
				return (gOpts.ips != NULL);
#endif /* USE_IPS */
			if (*(char **)option->dest == *(char **)compare)
				retval = 0;
			else if (!*(char **)option->dest || !*(char **)compare)
				retval = 1;
			else
				retval = strcmp(*(char **)option->dest, *(char **)compare);
			break;
		case rc_bool:
		case rc_int:
			retval = (*(int *)option->dest) - (*(int *)compare);
			break;
		case rc_float:
			retval = memcmp(option->dest, compare, sizeof (float));
			break;
		}
	}

	//if (retval)
	//	dprintf("%s", option->name);

	return retval;
}

static options_type *update_game_use_default(int driver_index)
{
	options_type *opt = GetParentOptions(driver_index);
#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	char *ips;
#endif /* USE_IPS */

	if (opt == &game_options[driver_index])
		return NULL;

#ifdef USE_IPS
	ips = game_options[driver_index].ips;
	game_options[driver_index].ips = NULL;
#endif /* USE_IPS */

	game_variables[driver_index].use_default = IsOptionEqual(&game_options[driver_index], opt);

#ifdef USE_IPS
	if (game_variables[driver_index].use_default && ips)
		dprintf("%s: use_default with ips", drivers[driver_index]->name);

	game_options[driver_index].ips = ips;
#endif /* USE_IPS */

	return opt;
}

static int rc_load_game_config(int driver_index)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int retval;
	int alt_index = game_variables[driver_index].alt_index;

	if (alt_index != -1)
		return rc_load_alt_config(&alt_options[alt_index]);

	game_variables[driver_index].options_loaded = TRUE;
	game_variables[driver_index].use_default = TRUE;
	SetCorePathList(FILETYPE_INI, settings.inipath);
	sprintf(filename, "%s.ini", drivers[driver_index]->name);

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 0)))
		return 0;

	gOpts = game_options[driver_index];
	retval = osd_rc_read(rc_game, file, filename, 1, 1);
	game_options[driver_index] = gOpts;

	update_game_use_default(driver_index);
	mame_fclose(file);

	return retval;
}

static int rc_save_game_config(int driver_index)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int retval;
	options_type *parent;
	int alt_index = game_variables[driver_index].alt_index;

	if (game_variables[driver_index].options_loaded == FALSE)
		return 0;

	if (alt_index != -1)
		return rc_save_alt_config(&alt_options[alt_index]);

	parent = update_game_use_default(driver_index);
	if (parent == NULL)
		return 0;

#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	if (game_variables[driver_index].use_default && !game_options[driver_index].ips)
#else /* USE_IPS */
	if (game_variables[driver_index].use_default)
#endif /* USE_IPS */
	{
		sprintf(filename, "%s\\%s.ini", settings.inipath, drivers[driver_index]->name);
		unlink(filename);
		return 0;
	}

	SetCorePathList(FILETYPE_INI, settings.inipath);
	strcpy(filename, strlower(drivers[driver_index]->name));
	strcat(filename, ".ini");

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 1)))
		return -1;

	gOpts = game_options[driver_index];
	retval = osd_rc_write_changes(rc_game, file, drivers[driver_index]->description,
	                              rc_game_is_changed, parent);

	mame_fclose(file);

	return retval;
}

static options_type *update_alt_use_default(alt_options_type *alt_option)
{
	options_type *opt = GetDefaultOptions();
	char *bios;
#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	char *ips;
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
		dprintf("%s: use_default with ips", alt_option->name);

	alt_option->option->ips = ips;
#endif /* USE_IPS */

	alt_option->option->bios = bios;

	return opt;
}

static int rc_load_alt_config(alt_options_type *alt_option)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int len;
	int retval;

	alt_option->variable->options_loaded = TRUE;
	alt_option->variable->use_default = TRUE;
	SetCorePathList(FILETYPE_INI, settings.inipath);
	sprintf(filename, "%s", alt_option->name);
	len = strlen(filename);

	if (len > 2 && filename[len - 2] == '.' && filename[len - 1] == 'c')
		filename[len - 2] = '\0';
	strcat(filename, ".ini");

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 0)))
		return 0;

	gOpts = *alt_option->option;
	retval = osd_rc_read(rc_game, file, filename, 1, 1);
	*alt_option->option = gOpts;

	update_alt_use_default(alt_option);

	mame_fclose(file);

	return retval;
}

static int rc_save_alt_config(alt_options_type *alt_option)
{
	char filename[_MAX_PATH];
	mame_file *file;
	int len;
	int retval;
	options_type *parent;

	if (alt_option->variable->options_loaded == FALSE)
		return 0;

	parent = update_alt_use_default(alt_option);

	sprintf(filename, "%s", strlower(alt_option->name));
	len = strlen(filename);

	if (len > 2 && filename[len - 2] == '.' && filename[len - 1] == 'c')
		filename[len - 2] = '\0';

#ifdef USE_IPS
	// HACK: DO NOT INHERIT IPS CONFIGURATION
	if (alt_option->variable->use_default && !alt_option->has_bios && !alt_option->option->ips)
#else /* USE_IPS */
	if (alt_option->variable->use_default && !alt_option->has_bios)
#endif /* USE_IPS */
	{
		char buf[_MAX_PATH];

		sprintf(buf, "%s\\%s.ini", settings.inipath, filename);
		unlink(buf);
		return 0;
	}

	SetCorePathList(FILETYPE_INI, settings.inipath);

	if (!(file = mame_fopen(filename, NULL, FILETYPE_INI, 1)))
		return -1;

	gOpts = *alt_option->option;
	retval = osd_rc_write_changes(rc_game, file, alt_option->name,
			rc_game_is_changed, parent);

	mame_fclose(file);

	return retval;
}

static void CopySettings(const settings_type *source, settings_type *dest)
{
	settings_type save = settings;

	settings = *source;
	memset(&gOpts, 0, sizeof(gOpts));

	rc_duplicate_strings(rc_core->option);
	rc_duplicate_strings(rc_winui[num_games]->option);
	settings.show_folder_flags = DuplicateBits(settings.show_folder_flags);

	*dest = settings;
	settings = save;
}

static void FreeSettings(settings_type *p)
{
	settings_type save = settings;
	settings = *p;
	memset(&gOpts, 0, sizeof(gOpts));

	rc_free_strings(rc_core->option);
	rc_free_strings(rc_winui[num_games]->option);

	DeleteBits(p->show_folder_flags);
	p->show_folder_flags = NULL;

	settings = save;
}

#ifndef NEW_RENDER
static int D3DEffectDecode(struct rc_option *option, const char *arg, int priority)
{
	option->priority = priority;

	// reset overrides
	gOpts.d3d_feedback_enable = FALSE;
	gOpts.d3d_scanlines_enable = FALSE;

	return 0;
}

static int D3DFeedbackDecode(struct rc_option *option, const char *arg, int priority)
{
	option->priority = priority;

	// enable override
	gOpts.d3d_feedback_enable = TRUE;

	return 0;
}

static int D3DScanlinesDecode(struct rc_option *option, const char *arg, int priority)
{
	option->priority = priority;

	// enable override
	gOpts.d3d_scanlines_enable = TRUE;

	return 0;
}

static int D3DPrescaleDecode(struct rc_option *option, const char *arg, int priority)
{
	char **p = option->dest;

	option->priority = priority;

	// convert old option to new
	if (!strcmp(arg, "0"))
	{
		FreeIfAllocated(p);
		*p = strdup("auto");
	}

	if (!strcmp(arg, "1"))
	{
		FreeIfAllocated(p);
		*p = strdup("full");
	}

	return 0;
}

static int CleanStretchDecodeString(struct rc_option *option, const char *arg, int priority)
{
	char **p = option->dest;

	option->priority = priority;

	// convert old option to new
	if (!strcmp(arg, "0"))
	{
		FreeIfAllocated(p);
		*p = strdup("auto");
	}

	if (!strcmp(arg, "1"))
	{
		FreeIfAllocated(p);
		*p = strdup("full");
	}

	return 0;
}
#endif

static int LedmodeDecodeString(struct rc_option *option, const char *arg, int priority)
{
	if ( strcmp( arg, "ps/2" ) != 0 &&
		strcmp( arg, "usb" ) != 0 )
	{
		fprintf(stderr, "error: invalid value for led_mode: %s\n", arg);
		return -1;
	}
	option->priority = priority;

	return 0;
}

static void KeySeqEncodeString(void)
{
	KeySeq *ks;
	char **pp;

	pp = &rc_dummy_args.ui_key_up;
	for (ks = &settings.ui_key_up; ks <= &settings.ui_key_quit; ks++, pp++)
	{
		FreeIfAllocated(pp);

		*pp = strdup(ks->seq_string);
	}
}

static int KeySeqDecodeString(struct rc_option *option, const char *arg, int priority)
{
	KeySeq *ks;
	input_seq *is;
	char **pp;

	pp = &rc_dummy_args.ui_key_up;
	ks = &settings.ui_key_up;

	while (pp != option->dest)
	{
		pp++;
		ks++;
	}

	assert (&settings.ui_key_up <= ks && ks <= &settings.ui_key_quit);

	FreeIfAllocated(&ks->seq_string);
	ks->seq_string = *pp;
	*pp = NULL;

	is = &(ks->is);

	//get the new input sequence
	string_to_seq(arg, is);
	//dprintf("seq=%s,,,%04i %04i %04i %04i \n",arg,(*is)[0],(*is)[1],(*is)[2],(*is)[3]);

	FreeIfAllocated((char **)option->dest);

	option->priority = priority;

	return 0;
}

static void JoyInfoEncodeString(void)
{
	int i, j;
	char buf[80];
	int *data;
	char *p;
	char **pp;

	pp = &rc_dummy_args.ui_joy_up;
	for (data = settings.ui_joy_up; data <= settings.ui_joy_exec; data += 4, pp++)
	{
		FreeIfAllocated(pp);

		if (data[0] == 0)
			continue;

		p = buf;
		*p = '\0';

		for (i = 0; i < 4; i++)
		{
			switch (i)
			{
			case 2:
				*p++ = ',';
			case 0:
				sprintf(p, "%d", data[i]);
				break;

			case 1:
				*p++ = ',';
				for (j = 0; joycode_axis[j].name; j++)
					if (joycode_axis[j].value == data[i])
					{
						strcpy(p, joycode_axis[j].name);
						break;
					}
				break;

			case 3:
				*p++ = ',';
				for (j = 0; joycode_dir[j].name; j++)
					if (joycode_dir[j].value == data[i])
					{
						strcpy(p, joycode_dir[j].name);
						break;
					}
				break;
			}

			p += strlen(p);
		}

		*pp = strdup(buf);
	}
}

static int JoyInfoDecodeString(struct rc_option *option, const char *arg, int priority)
{
	int  i;
	char *s, *p;
	int temp[4];
	char buf[80];
	int *data;
	char **pp;

	pp = &rc_dummy_args.ui_joy_up;
	for (i = 0; ; i += 4, pp++)
	{
		if (pp == option->dest)
			break;
	}
	data = settings.ui_joy_up + i;

	assert (settings.ui_joy_up <= data && data <= settings.ui_joy_exec);

	memset(data, 0, sizeof (*data) * 4);

	if (arg == NULL)
		return 0;

	strcpy(buf, arg);
	p = buf;

	for (i = 0; p && i < 4; i++)
	{
		int j = 0;
		s = p;
		
		if ((p = strchr(s, ',')) != NULL && *p == ',')
		{
			*p = '\0';
			p++;
		}

		switch (i)
		{
		case 0:
		case 2:
			temp[i] = atoi(s);
			break;

		case 1:
			for (j = 0; joycode_axis[j].name; j++)
				if (!strcmp(joycode_axis[j].name, s))
				{
					temp[i] = joycode_axis[j].value;
					break;
				}
			break;

		case 3:
			for (j = 0; joycode_dir[j].name; j++)
				if (!strcmp(joycode_dir[j].name, s))
				{
					temp[i] = joycode_dir[j].value;
					break;
				}
			break;
		}

		// lookup fails
		if (!joycode_axis[j].name)
			break;
	}

	FreeIfAllocated((char **)option->dest);

	if ((i != 4) || (p))
	{
		fprintf(stderr, "error invalid value for %s: %s\n", option->name, arg);
		return -1;
	}

	for (i = 0; i < 4; i++)
		data[i] = temp[i];

	option->priority = priority;

	return 0;
}

static int ColumnDecodeString(struct rc_option *option, const char *arg, int *data, int priority)
{
	int  i;
	char *s, *p;
	int temp[COLUMN_MAX];
	char buf[80];

	if (arg == NULL)
		return -1;

	strcpy(buf, arg);
	p = buf;

	for (i = 0; p && i < COLUMN_MAX; i++)
	{
		s = p;
		
		if ((p = strchr(s, ',')) != NULL && *p == ',')
		{
			*p = '\0';
			p++;
		}
		temp[i] = atoi(s);
	}

	FreeIfAllocated((char **)option->dest);

	if ((i != COLUMN_MAX) || (p))
	{
		fprintf(stderr, "error invalid value for %s: %s\n", option->name, arg);
		return -1;
	}

	for (i = 0; i < COLUMN_MAX; i++)
		data[i] = temp[i];

	option->priority = priority;

	return 0;
}

static char *ColumnEncodeString(int *data)
{
	int  i;
	char str[80];

	sprintf(str, "%d", data[0]);
	
	for (i = 1; i < COLUMN_MAX; i++)
		sprintf(str + strlen(str), ",%d", data[i]);

	return strdup(str);
}

static int ColumnOrderDecodeString(struct rc_option *option, const char *arg, int priority)
{
	return ColumnDecodeString(option, arg, settings.column_order, priority);
}

static void ColumnOrderEncodeString(void)
{
	FreeIfAllocated(&rc_dummy_args.column_order);
	rc_dummy_args.column_order = ColumnEncodeString(settings.column_order);
}

static int ColumnShownDecodeString(struct rc_option *option, const char *arg, int priority)
{
	return ColumnDecodeString(option, arg, settings.column_shown, priority);
}

static void  ColumnShownEncodeString(void)
{
	FreeIfAllocated(&rc_dummy_args.column_shown);
	rc_dummy_args.column_shown = ColumnEncodeString(settings.column_shown);
}

static int ColumnDecodeWidths(struct rc_option *option, const char *arg, int priority)
{
	return ColumnDecodeString(option, arg, settings.column_width, priority);
}

static void ColumnEncodeWidths(void)
{
	FreeIfAllocated(&rc_dummy_args.column_width);
	rc_dummy_args.column_width = ColumnEncodeString(settings.column_width);
}

static int CusColorDecodeString(struct rc_option *option, const char *arg, int priority)
{
	int  i;
	char *s, *p;
	COLORREF temp[16];
	char buf[80];

	if (arg == NULL)
		return -1;

	strcpy(buf, arg);
	p = buf;

	for (i = 0; p && i < 16; i++)
	{
		s = p;
		
		if ((p = strchr(s, ',')) != NULL && *p == ',')
		{
			*p = '\0';
			p++;
		}
		temp[i] = atoi(s);
	}

	FreeIfAllocated((char **)option->dest);

	if ((i != 16) || (p))
	{
		fprintf(stderr, "error invalid value for %s: %s\n", option->name, arg);
		return -1;
	}

	for (i = 0; i < 16; i++)
		settings.custom_color[i] = temp[i];

	option->priority = priority;

	return 0;
}

static void CusColorEncodeString(void)
{
	int  i;
	char str[256];

	sprintf(str, "%lu", settings.custom_color[0]);
	
	for (i = 1; i < 16; i++)
		sprintf(str + strlen(str), ",%lu", settings.custom_color[i]);

	FreeIfAllocated(&rc_dummy_args.custom_color);
	rc_dummy_args.custom_color = strdup(str);
}

static int SplitterDecodeString(struct rc_option *option, const char *arg, int priority)
{
	int  i;
	char *s, *p;
	int temp[SPLITTER_MAX];
	char buf[80];

	if (arg == NULL)
		return -1;

	strcpy(buf, arg);
	p = buf;

	for (i = 0; p && i < SPLITTER_MAX; i++)
	{
		s = p;
		
		if ((p = strchr(s, ',')) != NULL && *p == ',')
		{
			*p = '\0';
			p++;
		}
		temp[i] = atoi(s);
	}

	if ((i != SPLITTER_MAX) || (p))
	{
		fprintf(stderr, "error invalid value for %s: %s\n", option->name, arg);
		FreeIfAllocated((char **)option->dest);
		return -1;
	}

	for (i = 0; i < SPLITTER_MAX; i++)
		settings.splitter[i] = temp[i];

	option->priority = priority;
	FreeIfAllocated((char **)option->dest);

	return 0;
}

static void SplitterEncodeString(void)
{
	int  i;
	char str[80];

	sprintf(str, "%d", settings.splitter[0]);
	
	for (i = 1; i < SPLITTER_MAX; i++)
		sprintf(str + strlen(str), ",%d", settings.splitter[i]);

	FreeIfAllocated(&rc_dummy_args.splitter);
	rc_dummy_args.splitter = strdup(str);
}

static int ListDecodeString(struct rc_option *option, const char *arg, int priority)
{
	int i;

	settings.view = VIEW_GROUPED;

	if (arg == NULL)
		return -1;

	for (i = VIEW_LARGE_ICONS; i < VIEW_MAX; i++)
	{
		if (strcmp(arg, view_modes[i]) == 0)
		{
			settings.view = i;
			option->priority = priority;
			FreeIfAllocated((char **)option->dest);

			return 0;
		}
	}

	FreeIfAllocated((char **)option->dest);
	return -1;
}

static void ListEncodeString(void)
{
	FreeIfAllocated(&rc_dummy_args.view);
	rc_dummy_args.view = strdup(view_modes[settings.view]);
}

/* Parse the given comma-delimited string into a LOGFONT structure */
static int FontDecodeString(struct rc_option *option, const char *arg, int priority)
{
	LOGFONTA* f = &settings.list_font;

	if (arg == NULL)
		return -1;

	sscanf(arg, "%li,%li,%li,%li,%li,%i,%i,%i,%i,%i,%i,%i,%i",
	            &f->lfHeight,
	            &f->lfWidth,
	            &f->lfEscapement,
	            &f->lfOrientation,
	            &f->lfWeight,
	            (int*)&f->lfItalic,
	            (int*)&f->lfUnderline,
	            (int*)&f->lfStrikeOut,
	            (int*)&f->lfCharSet,
	            (int*)&f->lfOutPrecision,
	            (int*)&f->lfClipPrecision,
	            (int*)&f->lfQuality,
	            (int*)&f->lfPitchAndFamily);

	option->priority = priority;
	FreeIfAllocated((char **)option->dest);

	return 0;
}

/* Encode the given LOGFONT structure into a comma-delimited string */
static void FontEncodeString(void)
{
	LOGFONTA* f = &settings.list_font;
	char buf[512];

	sprintf(buf, "%li,%li,%li,%li,%li,%i,%i,%i,%i,%i,%i,%i,%i",
	             f->lfHeight,
	             f->lfWidth,
	             f->lfEscapement,
	             f->lfOrientation,
	             f->lfWeight,
	             f->lfItalic,
	             f->lfUnderline,
	             f->lfStrikeOut,
	             f->lfCharSet,
	             f->lfOutPrecision,
	             f->lfClipPrecision,
	             f->lfQuality,
	             f->lfPitchAndFamily);

	FreeIfAllocated(&rc_dummy_args.list_font);
	rc_dummy_args.list_font = strdup(buf);
}

static int FontfaceDecodeString(struct rc_option *option, const char *arg, int priority)
{
	if (arg == NULL)
		return -1;

	strcpy(settings.list_font.lfFaceName, arg);
	option->priority = priority;
	FreeIfAllocated((char **)option->dest);

	return 0;
}

static void FontfaceEncodeString(void)
{
	FreeIfAllocated(&rc_dummy_args.list_fontface);
	rc_dummy_args.list_fontface = strdup(settings.list_font.lfFaceName);
}

static int FolderFlagDecodeString(struct rc_option *option, const char *arg, int priority)
{
	char str[80], *p;

	if (arg == NULL)
		return 0;

	strcpy(str, arg);
	p = strrchr(str, ',');
	if (p == NULL)
	{
		fprintf(stderr, "error invalid value for %s: %s\n", option->name, arg);
		FreeIfAllocated((char **)option->dest);

		return -1;
	}

	*p++ = '\0';

	SaveFolderFlags(str, atoi(p));
	option->priority = priority;

	return 0;
}

static void HideFolderEncodeString(void)
{
	int i;
	int num_saved = 0;
	char buf[2000];

	strcpy(buf,"");

	// we save the ones that are NOT displayed, so we can add new ones
	// and upgraders will see them
	for (i=0; i < MAX_FOLDERS; i++)
	{
		if (TestBit(settings.show_folder_flags, i) == FALSE)
		{
			int j;

			if (num_saved != 0)
				strcat(buf,", ");

			for (j=0; g_folderData[j].m_lpTitle != NULL; j++)
			{
				if (g_folderData[j].m_nFolderId == i && g_folderData[j].short_name)
				{
					strcat(buf, g_folderData[j].short_name);
					num_saved++;
					break;
				}
			}
		}
	}

	FreeIfAllocated(&rc_dummy_args.ui_hide_folder);

	if (num_saved)
		rc_dummy_args.ui_hide_folder = strdup(buf);
}

static int HideFolderDecodeString(struct rc_option *option, const char *arg, int priority)
{
	char *token;

	if (settings.show_folder_flags)
		DeleteBits(settings.show_folder_flags);

	settings.show_folder_flags = NewBits(MAX_FOLDERS);
	SetAllBits(settings.show_folder_flags,TRUE);

	option->priority = priority;

	if (arg == NULL)
		return 0;

	token = strdup(arg);
	FreeIfAllocated(&rc_dummy_args.ui_hide_folder);

	if (token == NULL)
		return 0;

	token = strtok(token, ", \t");
	while (token != NULL)
	{
		int j;

		for (j=0; g_folderData[j].m_lpTitle != NULL; j++)
		{
			if (strcmp(g_folderData[j].short_name, token) == 0)
			{
				ClearBit(settings.show_folder_flags, g_folderData[j].m_nFolderId);
				break;
			}
		}
		token = strtok(NULL, ", \t");
	}

	return 0;
}

static int PaletteDecodeString(struct rc_option *option, const char *arg, int priority)
{
	int pal[3];

	if (arg == NULL)
		return -1;

	if (sscanf(arg, "%d,%d,%d", &pal[0], &pal[1], &pal[2]) != 3 ||
		pal[0] < 0 || pal[0] >= 256 ||
		pal[1] < 0 || pal[1] >= 256 ||
		pal[2] < 0 || pal[2] >= 256 )
	{
		fprintf(stderr, _WINDOWS("error: invalid value for palette: %s\n"), arg);
		return -1;
	}

	option->priority = priority;

	return 0;
}

static int LanguageDecodeString(void)
{
	int langcode;

	if (settings.language == NULL)
		langcode = -1;
	else
		langcode = stricmp(settings.language, "auto") ? lang_find_langname(settings.language) : -1;

	SetLangcode(langcode);
	FreeIfAllocated(&settings.language);

	return 0;
}

static void LanguageEncodeString(void)
{
	int langcode = GetLangcode();

	FreeIfAllocated(&settings.language);
	settings.language = strdup(langcode < 0 ? "auto" : ui_lang_info[langcode].name);
}

static void SaveFolderFlags(const char *folderName, DWORD dwFlags)
{
	int i;

	for (i = 0; i < num_folder_flags; i++)
		if (!strcmp(folder_flags[i].name, folderName))
		{
			if (dwFlags == 0)
				folder_flags[i].name[0] = '\0';
			else
				folder_flags[i].flags = dwFlags;
			return;
		}

	if (dwFlags == 0)
		return;

	for (i = 0; i < num_folder_flags; i++)
		if (folder_flags[i].name[0] == '\0')
			break;

	if (i == num_folder_flags)
	{
		folder_flags_type *tmp;

		tmp = realloc(folder_flags, (num_folder_flags + ALLOC_FOLDERFLAG) * sizeof(folder_flags_type));
		if (!tmp)
		{
			fprintf(stderr, "error: realloc failed in SaveFolderFlags\n");
			return;
		}

		folder_flags = tmp;
		memset(tmp + num_folder_flags, 0, ALLOC_FOLDERFLAG * sizeof(folder_flags_type));
		num_folder_flags += ALLOC_FOLDERFLAG;
	}

	strcpy(folder_flags[i].name, folderName);
	folder_flags[i].flags = dwFlags;
}

static void LoadGameOptions(int driver_index)
{
	assert (0 <= driver_index && driver_index < num_games);

	rc_load_game_config(driver_index);
}

static void LoadAltOptions(alt_options_type *alt_option)
{
	rc_load_alt_config(alt_option);
}

static void LoadDefaultOptions(void)
{
	rc_load_default_config();
}

static void LoadOptions(void)
{
	LoadDefaultOptions();
	rc_load_winui_config();
	SetLangcode(settings.langcode);
	SetUseLangList(UseLangList());

#if 0
	if (!bResetGUI)
	{
		static char oldInfoMsg[400] = 
MAME32NAME " has detected outdated configuration data.\n\n\
The detected configuration data is from Version %s of " MAME32NAME ".\n\
The current version is %s. It is recommended that the\n\
configuration is set to the new defaults.\n\n\
Would you like to use the new configuration?";

		char *current_version;

		current_version = GetVersionString();

		if (rc_dummy_args.save_version && *rc_dummy_args.save_version && strcmp(rc_dummy_args.save_version, current_version) != 0)
		{
			char msg[400];
			sprintf(msg,_UI(oldInfoMsg), rc_dummy_args.save_version, current_version);
			if (MessageBox(0, _Unicode(msg), _Unicode(_UI("Version Mismatch")), MB_YESNO | MB_ICONQUESTION) == IDYES)
				bResetGUI = TRUE;
		}

		FreeIfAllocated(&rc_dummy_args.save_version);
	}
#endif

	if (bResetGUI)
	{
		FreeSettings(&settings);
		settings = backup.settings;
		SetLangcode(settings.langcode);
		SetUseLangList(UseLangList());
	}
	else
		FreeSettings(&backup.settings);

	bResetGUI = FALSE;
}

/* End of options.c */
