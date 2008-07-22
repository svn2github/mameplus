/***************************************************************************

    uitext.h

    Functions used to retrieve text used by MAME, to aid in
    translation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __uitext_H__
#define __uitext_H__

#include "mamecore.h"

/* Important: this must match the default_text list in uitext.c! */
enum
{
	UI_mame = 0,

	/* misc menu stuff */
	UI_returntomain,
	UI_anykey,
	UI_on,
	UI_off,
	UI_OK,
	UI_address,
	UI_value,
	UI_stereo,
	UI_screenres,
	UI_text,
	UI_relative,
	UI_allchannels,
	UI_brightness,
	UI_contrast,
	UI_gamma,
	UI_vectorflicker,
	UI_allcpus,
	UI_historymissing,
#ifdef STORY_DATAFILE
	UI_storymissing,
#endif /* STORY_DATAFILE */
	UI_mameinfomissing,
	UI_drivinfomissing,
	UI_statisticsmissing,
#ifdef CMD_LIST
	UI_commandmissing,
#endif /* CMD_LIST */

	/* special characters */
	UI_leftarrow,
	UI_rightarrow,
	UI_uparrow,
	UI_downarrow,
	UI_lefthilight,
	UI_righthilight,

	/* main menu */
	UI_calibrate,
	UI_resetgame,
	/* documents menu */
	UI_history,
#ifdef STORY_DATAFILE
	UI_story,
#endif /* STORY_DATAFILE */
	UI_mameinfo,
	UI_drivinfo,
	UI_statistics,
#ifdef CMD_LIST
	UI_command,
#endif /* CMD_LIST */

	UI_rotate_clockwise,
	UI_rotate_counterclockwise,
	UI_flip_x,
	UI_flip_y,

	/* centering */
	UI_center,
	
	UI_last_mame_entry
};

#ifdef MAMEMESS
#include "mslegacy.h"
#endif

struct _lang_struct
{
	int version;
	int multibyte;			/* UNUSED: 1 if this is a multibyte font/language */
	UINT8 *fontdata;		/* pointer to the raw font data to be decoded */
	UINT16 fontglyphs;		/* total number of glyps in the external font - 1 */
	char langname[255];
	char fontname[255];
	char author[255];
};
typedef struct _lang_struct lang_struct;

#if 0
int uistring_init (mame_file *language_file);
#else
int uistring_init (void);
#endif

const char * ui_getstring (int string_num);

#endif /* __uitext_H__ */
