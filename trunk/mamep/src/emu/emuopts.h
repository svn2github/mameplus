/***************************************************************************

    emuopts.h

    Options file and command line management.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
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
#define OPTION_PRIORITY_CMDLINE		OPTION_PRIORITY_HIGH
#define OPTION_PRIORITY_INI			OPTION_PRIORITY_NORMAL

/* core options */
#define OPTION_GAMENAME				OPTION_UNADORNED(0)

/* core configuration options */
#define OPTION_READCONFIG			"readconfig"

/* core configuration options */
#define OPTION_DRIVER_CONFIG			"driver_config"

/* core search path options */
#define OPTION_ROMPATH				"rompath"
//#ifdef MESS
#define OPTION_HASHPATH				"hashpath"
//#endif
#define OPTION_SAMPLEPATH			"samplepath"
#define OPTION_ARTPATH				"artpath"
#define OPTION_CTRLRPATH			"ctrlrpath"
#define OPTION_INIPATH				"inipath"
#define OPTION_FONTPATH				"fontpath"
#define OPTION_TRANSLATION_DIRECTORY		"translation_directory"
#define OPTION_LOCALIZED_DIRECTORY		"localized_directory"
#ifdef USE_IPS
#define OPTION_IPS_DIRECTORY			"ips_directory"
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
#define OPTION_CHEAT_FILE			"cheat_file"
#define OPTION_HISTORY_FILE			"history_file"
#ifdef STORY_DATAFILE
#define OPTION_STORY_FILE			"story_file"
#endif /* STORY_DATAFILE */
#define OPTION_MAMEINFO_FILE			"mameinfo_file"
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
#define OPTION_WAVWRITE				"wavwrite"

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
#ifdef USE_IPS
#define OPTION_IPS				"ips"
#endif /* USE_IPS */
#define OPTION_CONFIRM_QUIT			"confirm_quit"
#ifdef AUTO_PAUSE_PLAYBACK
#define OPTION_AUTO_PAUSE_PLAYBACK		"auto_pause_playback"
#endif /* AUTO_PAUSE_PLAYBACK */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
#define OPTION_M68K_CORE			"m68k_core"
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef TRANS_UI
#define OPTION_USE_TRANS_UI			"use_trans_ui"
#define OPTION_UI_TRANSPARENCY			"ui_transparency"
#endif /* TRANS_UI */

#ifdef UI_COLOR_DISPLAY
#define OPTION_FONT_BLANK			"font_blank"
#define OPTION_FONT_NORMAL			"font_normal"
#define OPTION_FONT_SPECIAL			"font_special"
#define OPTION_SYSTEM_BACKGROUND		"system_background"
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
#define OPTION_CURSOR				"cursor"
#endif /* UI_COLOR_DISPLAY */

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
