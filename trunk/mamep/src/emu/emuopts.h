/***************************************************************************

    emuopts.h

    Options file and command line management.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMUOPTS_H__
#define __EMUOPTS_H__

#include "mamecore.h"
#include "options.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* option priorities */
#define OPTION_PRIORITY_CMDLINE			OPTION_PRIORITY_HIGH
#define OPTION_PRIORITY_INI				OPTION_PRIORITY_NORMAL
#define OPTION_PRIORITY_MAME_INI		(OPTION_PRIORITY_NORMAL + 1)
#define OPTION_PRIORITY_DEBUG_INI		(OPTION_PRIORITY_MAME_INI + 1)
#define OPTION_PRIORITY_ORIENTATION_INI	(OPTION_PRIORITY_DEBUG_INI + 1)
#define OPTION_PRIORITY_VECTOR_INI		(OPTION_PRIORITY_ORIENTATION_INI + 1)
#define OPTION_PRIORITY_SOURCE_INI		(OPTION_PRIORITY_VECTOR_INI + 1)
#define OPTION_PRIORITY_GPARENT_INI		(OPTION_PRIORITY_SOURCE_INI + 1)
#define OPTION_PRIORITY_PARENT_INI		(OPTION_PRIORITY_GPARENT_INI + 1)
#define OPTION_PRIORITY_DRIVER_INI		(OPTION_PRIORITY_PARENT_INI + 1)

/* core options */
#define OPTION_GAMENAME				OPTION_UNADORNED(0)

/* core configuration options */
#define OPTION_READCONFIG			"readconfig"
#ifdef DRIVER_SWITCH
#define OPTION_DRIVER_CONFIG			"driver_config"
#endif /* DRIVER_SWITCH */

/* core search path options */
#define OPTION_ROMPATH				"rompath"
#ifdef MAMEMESS
#define OPTION_HASHPATH				"hashpath"
#endif
#define OPTION_SAMPLEPATH			"samplepath"
#define OPTION_ARTPATH				"artpath"
#define OPTION_CTRLRPATH			"ctrlrpath"
#define OPTION_INIPATH				"inipath"
#define OPTION_FONTPATH				"fontpath"
#define OPTION_CHEATPATH			"cheatpath"
#define OPTION_CROSSHAIRPATH		"crosshairpath"
#define OPTION_LANGPATH				"langpath"
#ifdef USE_IPS
#define OPTION_IPSPATH  			"ips_directory"
#endif /* USE_IPS */

/* core directory options */
#define OPTION_CFG_DIRECTORY		"cfg_directory"
#define OPTION_NVRAM_DIRECTORY		"nvram_directory"
#define OPTION_MEMCARD_DIRECTORY	"memcard_directory"
#define OPTION_INPUT_DIRECTORY		"input_directory"
#define OPTION_STATE_DIRECTORY		"state_directory"
#define OPTION_SNAPSHOT_DIRECTORY	"snapshot_directory"
#define OPTION_DIFF_DIRECTORY		"diff_directory"
#define OPTION_COMMENT_DIRECTORY	"comment_directory"
#ifdef USE_HISCORE
#define OPTION_HISCORE_DIRECTORY	"hiscore_directory"
#endif /* USE_HISCORE */

/* core filename options */
#ifdef CMD_LIST
#define OPTION_COMMAND_FILE			"command_file"
#endif /* CMD_LIST */
#ifdef USE_HISCORE
#define OPTION_HISCORE_FILE			"hiscore_file"
#endif /* USE_HISCORE */

/* core state/playback options */
#define OPTION_STATE				"state"
#define OPTION_AUTOSAVE				"autosave"
#define OPTION_PLAYBACK				"playback"
#define OPTION_RECORD				"record"
#define OPTION_MNGWRITE				"mngwrite"
#define OPTION_AVIWRITE				"aviwrite"
#define OPTION_WAVWRITE				"wavwrite"
#define OPTION_SNAPNAME				"snapname"
#define OPTION_SNAPSIZE				"snapsize"
#define OPTION_SNAPVIEW				"snapview"
#define OPTION_BURNIN				"burnin"

/* core performance options */
#define OPTION_AUTOFRAMESKIP		"autoframeskip"
#define OPTION_FRAMESKIP			"frameskip"
#define OPTION_SECONDS_TO_RUN		"seconds_to_run"
#define OPTION_THROTTLE				"throttle"
#define OPTION_SLEEP				"sleep"
#define OPTION_SPEED				"speed"
#define OPTION_REFRESHSPEED			"refreshspeed"

/* core rotation options */
#define OPTION_ROTATE				"rotate"
#define OPTION_ROR					"ror"
#define OPTION_ROL					"rol"
#define OPTION_AUTOROR				"autoror"
#define OPTION_AUTOROL				"autorol"
#define OPTION_FLIPX				"flipx"
#define OPTION_FLIPY				"flipy"

/* core artwork options */
#define OPTION_ARTWORK_CROP			"artwork_crop"
#define OPTION_USE_BACKDROPS		"use_backdrops"
#define OPTION_USE_OVERLAYS			"use_overlays"
#define OPTION_USE_BEZELS			"use_bezels"

/* core screen options */
#define OPTION_BRIGHTNESS			"brightness"
#define OPTION_CONTRAST				"contrast"
#define OPTION_GAMMA				"gamma"
#define OPTION_PAUSE_BRIGHTNESS		"pause_brightness"
#ifdef USE_SCALE_EFFECTS
#define OPTION_SCALE_EFFECT		"scale_effect"
#endif /* USE_SCALE_EFFECTS */

/* core vector options */
#define OPTION_ANTIALIAS			"antialias"
#define OPTION_BEAM					"beam"
#define OPTION_FLICKER				"flicker"

/* core sound options */
#define OPTION_SOUND				"sound"
#define OPTION_SAMPLERATE			"samplerate"
#define OPTION_SAMPLES				"samples"
#define OPTION_VOLUME				"volume"
#ifdef USE_VOLUME_AUTO_ADJUST
#define OPTION_VOLUME_ADJUST			"volume_adjust"
#endif /* USE_VOLUME_AUTO_ADJUST */

/* core input options */
#define OPTION_COIN_LOCKOUT			"coin_lockout"
#define OPTION_CTRLR				"ctrlr"
#define OPTION_MOUSE				"mouse"
#define OPTION_JOYSTICK				"joystick"
#define OPTION_LIGHTGUN				"lightgun"
#define OPTION_MULTIKEYBOARD		"multikeyboard"
#define OPTION_MULTIMOUSE			"multimouse"
#define OPTION_PADDLE_DEVICE		"paddle_device"
#define OPTION_ADSTICK_DEVICE		"adstick_device"
#define OPTION_PEDAL_DEVICE			"pedal_device"
#define OPTION_DIAL_DEVICE			"dial_device"
#define OPTION_TRACKBALL_DEVICE		"trackball_device"
#define OPTION_LIGHTGUN_DEVICE		"lightgun_device"
#define OPTION_POSITIONAL_DEVICE	"positional_device"
#define OPTION_MOUSE_DEVICE			"mouse_device"
#define OPTION_JOYSTICK_MAP			"joystick_map"
#define OPTION_JOYSTICK_DEADZONE	"joystick_deadzone"
#define OPTION_JOYSTICK_SATURATION	"joystick_saturation"
#define OPTION_STEADYKEY			"steadykey"
#define OPTION_OFFSCREEN_RELOAD		"offscreen_reload"

/* core debugging options */
#define OPTION_VERBOSE				"verbose"
#define OPTION_LOG					"log"
#define OPTION_DEBUG				"debug"
#define OPTION_DEBUGSCRIPT			"debugscript"
#define OPTION_UPDATEINPAUSE		"update_in_pause"

/* core misc options */
#define OPTION_BIOS					"bios"
#define OPTION_CHEAT				"cheat"
#define OPTION_SKIP_GAMEINFO		"skip_gameinfo"
#define OPTION_CONFIRM_QUIT			"confirm_quit"
#ifdef USE_IPS
#define OPTION_IPS				"ips"
#endif /* USE_IPS */
#ifdef AUTO_PAUSE_PLAYBACK
#define OPTION_AUTO_PAUSE_PLAYBACK		"auto_pause"
#endif /* AUTO_PAUSE_PLAYBACK */
#ifdef TRANS_UI
#define OPTION_UI_TRANSPARENCY			"ui_transparency"
#endif /* TRANS_UI */

#ifdef UI_COLOR_DISPLAY
/* core palette options */
#define OPTION_SYSTEM_BACKGROUND		"main_background"
#define OPTION_CURSOR_SELECTED_TEXT		"cursor_sel_text"
#define OPTION_CURSOR_SELECTED_BG		"cursor_sel_background"
#define OPTION_CURSOR_HOVER_TEXT		"cursor_hov_text"
#define OPTION_CURSOR_HOVER_BG			"cursor_hov_background"
#define OPTION_BUTTON_RED			"button_red"
#define OPTION_BUTTON_YELLOW			"button_yellow"
#define OPTION_BUTTON_GREEN			"button_green"
#define OPTION_BUTTON_BLUE			"button_blue"
#define OPTION_BUTTON_PURPLE			"button_purple"
#define OPTION_BUTTON_PINK			"button_pink"
#define OPTION_BUTTON_AQUA			"button_aqua"
#define OPTION_BUTTON_SILVER			"button_silver"
#define OPTION_BUTTON_NAVY			"button_navy"
#define OPTION_BUTTON_LIME			"button_lime"
#endif /* UI_COLOR_DISPLAY */

/* core language options */
#define OPTION_LANGUAGE				"language"
#define OPTION_USE_LANG_LIST			"use_lang_list"



/***************************************************************************
    GLOBALS
***************************************************************************/

extern const options_entry mame_core_options[];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

core_options *mame_options_init(const options_entry *entries);

#endif	/* __EMUOPTS_H__ */
