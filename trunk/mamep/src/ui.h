/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __USRINTRF_H__
#define __USRINTRF_H__

#include "mamecore.h"
#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* preferred font height; use ui_get_line_height() to get actual height */
#ifndef UI_COLOR_DISPLAY
#define UI_TARGET_FONT_HEIGHT	(1.0f / 25.0f)
#endif /* UI_COLOR_DISPLAY */

/* width of lines drawn in the UI */
#ifdef UI_COLOR_DISPLAY
#define UI_LINE_WIDTH			(1.0f / (float)ui_screen_height)
#else /* UI_COLOR_DISPLAY */
#define UI_LINE_WIDTH			(1.0f / 500.0f)
#endif /* UI_COLOR_DISPLAY */

/* border between outlines and inner text on left/right and top/bottom sides */
#ifdef UI_COLOR_DISPLAY
#define UI_BOX_LR_BORDER		3
#define UI_BOX_TB_BORDER		3
#else /* UI_COLOR_DISPLAY */
#define UI_BOX_LR_BORDER		(UI_TARGET_FONT_HEIGHT * 0.25f)
#define UI_BOX_TB_BORDER		(UI_TARGET_FONT_HEIGHT * 0.25f)
#endif /* UI_COLOR_DISPLAY */

/* handy colors */
#define ARGB_WHITE				MAKE_ARGB(0xff,0xff,0xff,0xff)
#define ARGB_BLACK				MAKE_ARGB(0xff,0x00,0x00,0x00)
#ifdef UI_COLOR_DISPLAY
#define UI_FILLCOLOR			SYSTEM_COLOR_BACKGROUND
#else /* UI_COLOR_DISPLAY */
#define UI_FILLCOLOR			MAKE_ARGB(0xe0,0x10,0x10,0x30)
#endif /* UI_COLOR_DISPLAY */

/* cancel return value for a UI handler */
#define UI_HANDLER_CANCEL		((UINT32)~0)

#define SHORTCUT_MENU_CHEAT	1
#ifdef CMD_LIST
#define SHORTCUT_MENU_COMMAND	2
#endif /* CMD_LIST */

/* justification options for ui_draw_text_full */
enum
{
	JUSTIFY_LEFT = 0,
	JUSTIFY_CENTER,
	JUSTIFY_RIGHT
};

/* word wrapping options for ui_draw_text_full */
enum
{
	WRAP_NEVER,
	WRAP_TRUNCATE,
	WRAP_WORD
};

/* drawing options for ui_draw_text_full */
enum
{
	DRAW_NONE,
	DRAW_NORMAL,
	DRAW_OPAQUE
};



/***************************************************************************
    MACROS
***************************************************************************/

#define ui_draw_message_window(text) ui_draw_text_box(text, JUSTIFY_LEFT, 0.5f, 0.5f, UI_FILLCOLOR)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* main init/exit routines */
int ui_init(running_machine *machine);

/* display the startup screens */
int ui_display_startup_screens(int show_disclaimer, int show_warnings, int show_gameinfo);

/* set the current text to display at startup */
void ui_set_startup_text(const char *text, int force);

/* once-per-frame update and render */
void ui_update_and_render(void);

/* returns the current UI font */
render_font *ui_get_font(void);

/* returns the line height of the font used by the UI system */
int ui_get_line_height(void);

/* returns the width of a character or string in the UI font */
int ui_get_char_width(UINT16 ch);
int ui_get_string_width(const char *s);

void add_fill(int x0, int y0, int x1, int y1, rgb_t color);
void add_filled_box(int x0, int y0, int x1, int y1);

/* draw an outlined box filled with a given color */
void ui_draw_outlined_box(float x0, float y0, float x1, float y1, rgb_t backcolor);

/* simple text draw at the given coordinates */
void ui_draw_text(const char *buf, int x, int y);

/* full-on text draw with all the options */
void ui_draw_text_full(const char *origs, int x, int y, int wrapwidth, int offset, int maxlines, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, int *totalwidth, int *totalheight);

/* draw a multi-line message with a box around it */
void ui_draw_text_box(const char *text, int justify, float xpos, float ypos, rgb_t backcolor);

/* display a temporary message at the bottom of the screen */
void CLIB_DECL ui_popup_time(int seconds, const char *text, ...) ATTR_PRINTF(2,3);

/* get/set whether or not the FPS is displayed */
void ui_show_fps_temp(double seconds);
void ui_set_show_fps(int show);
int ui_get_show_fps(void);

/* get/set whether or not the profiler is displayed */
void ui_set_show_profiler(int show);
int ui_get_show_profiler(void);

/* return true if a menu or the slider is displayed */
int ui_is_menu_active(void);
int ui_is_slider_active(void);

/* print the game info string into a buffer */
int sprintf_game_info(char *buffer);

/* called by the OSD layer to set the UI area of the screen */
void ui_set_visible_area(int xmin, int ymin, int xmax, int ymax);

void ui_auto_pause(void);

int ui_window_scroll_keys(void);

extern int ui_screen_width, ui_screen_height;
#endif	/* __USRINTRF_H__ */
