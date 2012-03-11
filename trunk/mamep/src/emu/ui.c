/*********************************************************************

    ui.c

    Functions used to handle MAME's user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "video/vector.h"
#include "machine/laserdsc.h"
#include "render.h"
#include "cheat.h"
#include "rendfont.h"
#include "ui.h"
#include "uiinput.h"
#include "uimain.h"
#include "uigfx.h"
#ifdef CMD_LIST
#include "cmddata.h"
#endif /* CMD_LIST */

#ifdef MAMEMESS
#define MESS
#endif /* MAMEMESS */

#include <ctype.h>
#ifdef USE_SHOW_TIME
#include <time.h>
#endif /* USE_SHOW_TIME */


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	LOADSAVE_NONE,
	LOADSAVE_LOAD,
	LOADSAVE_SAVE
};

//mamep: to render as fixed-width font
enum
{
	CHAR_WIDTH_HALFWIDTH = 0,
	CHAR_WIDTH_FULLWIDTH,
	CHAR_WIDTH_UNKNOWN
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

/* list of natural keyboard keys that are not associated with UI_EVENT_CHARs */
static const input_item_id non_char_keys[] =
{
	ITEM_ID_ESC,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_NUMLOCK,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_PAUSE,
	ITEM_ID_CANCEL
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

#ifdef UI_COLOR_DISPLAY
static rgb_t uifont_colortable[MAX_COLORTABLE];
#endif /* UI_COLOR_DISPLAY */
static rgb_t ui_bgcolor;
static render_texture *bgtexture;
static bitmap_argb32 *bgbitmap;

static int multiline_text_box_visible_lines;
static int multiline_text_box_target_lines;

//mamep: to render as fixed-width font
static int draw_text_fixed_mode;
static int draw_text_scroll_offset;

static int message_window_scroll;
static int scroll_reset;

static void build_bgtexture(running_machine &machine);
static void free_bgtexture(running_machine &machine);

#ifdef TRANS_UI
static int ui_transparency;
#endif /* TRANS_UI */

/* font for rendering */
static render_font *ui_font;

/* current UI handler */
static UINT32 (*ui_handler_callback)(running_machine &, render_container *, UINT32);
static UINT32 ui_handler_param;

/* flag to track single stepping */
static int single_step;

/* FPS counter display */
static int showfps;
static osd_ticks_t showfps_end;

/* profiler display */
static int show_profiler;

/* popup text display */
static osd_ticks_t popup_text_end;

/* messagebox buffer */
static astring messagebox_text;
static rgb_t messagebox_backcolor;

/* slider info */
static slider_state *slider_list;
static slider_state *slider_current;

/* natural keyboard info */
static int ui_use_natural_keyboard;
static UINT8 non_char_keys_down[(ARRAY_LENGTH(non_char_keys) + 7) / 8];

#ifdef USE_SHOW_TIME
static int show_time = 0;
static int Show_Time_Position;
#endif /* USE_SHOW_TIME */


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_exit(running_machine &machine);

/* text generators */
static astring &disclaimer_string(running_machine &machine, astring &buffer);
static astring &warnings_string(running_machine &machine, astring &buffer);

/* UI handlers */
static UINT32 handler_messagebox(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_ok(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_anykey(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_ingame(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_load_save(running_machine &machine, render_container *container, UINT32 state);
static UINT32 handler_confirm_quit(running_machine &machine, render_container *container, UINT32 state);

/* slider controls */
static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg);
static slider_state *slider_init(running_machine &machine);
static INT32 slider_volume(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_mixervol(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_adjuster(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overclock(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_refresh(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_brightness(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_contrast(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_gamma(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_flicker(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_beam(running_machine &machine, void *arg, astring *string, INT32 newval);
static char *slider_get_screen_desc(screen_device &screen);
#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine &machine, void *arg, astring *string, INT32 newval);
static INT32 slider_crossoffset(running_machine &machine, void *arg, astring *string, INT32 newval);
#endif


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ui_set_handler - set a callback/parameter
    pair for the current UI handler
-------------------------------------------------*/

INLINE UINT32 ui_set_handler(UINT32 (*callback)(running_machine &, render_container *, UINT32), UINT32 param)
{
	ui_handler_callback = callback;
	ui_handler_param = param;
	return param;
}


#ifdef UI_COLOR_DISPLAY
rgb_t ui_get_rgb_color(rgb_t color)
{
	if (color < MAX_COLORTABLE)
		return uifont_colortable[color];

	return color;
}
#endif /* UI_COLOR_DISPLAY */


/*-------------------------------------------------
    is_breakable_char - is a given unicode
    character a possible line break?
-------------------------------------------------*/

INLINE int is_breakable_char(unicode_char ch)
{
	/* regular spaces and hyphens are breakable */
	if (ch == ' ' || ch == '-')
		return TRUE;

	/* In the following character sets, any character is breakable:
        Hiragana (3040-309F)
        Katakana (30A0-30FF)
        Bopomofo (3100-312F)
        Hangul Compatibility Jamo (3130-318F)
        Kanbun (3190-319F)
        Bopomofo Extended (31A0-31BF)
        CJK Strokes (31C0-31EF)
        Katakana Phonetic Extensions (31F0-31FF)
        Enclosed CJK Letters and Months (3200-32FF)
        CJK Compatibility (3300-33FF)
        CJK Unified Ideographs Extension A (3400-4DBF)
        Yijing Hexagram Symbols (4DC0-4DFF)
        CJK Unified Ideographs (4E00-9FFF) */
	if (ch >= 0x3040 && ch <= 0x9fff)
		return TRUE;

	/* Hangul Syllables (AC00-D7AF) are breakable */
	if (ch >= 0xac00 && ch <= 0xd7af)
		return TRUE;

	/* CJK Compatibility Ideographs (F900-FAFF) are breakable */
	if (ch >= 0xf900 && ch <= 0xfaff)
		return TRUE;

	return FALSE;
}


//mamep: check fullwidth character.
//mame core does not support surrogate pairs U+10000-U+10FFFF
INLINE int is_fullwidth_char(unicode_char uchar)
{
	switch (uchar)
	{
	// Chars in Latin-1 Supplement
	// font width depends on your font
	case 0x00a7:
	case 0x00a8:
	case 0x00b0:
	case 0x00b1:
	case 0x00b4:
	case 0x00b6:
	case 0x00d7:
	case 0x00f7:
		return CHAR_WIDTH_UNKNOWN;
	}

	// Greek and Coptic
	// font width depends on your font
	if (uchar >= 0x0370 && uchar <= 0x03ff)
		return CHAR_WIDTH_UNKNOWN;

	// Cyrillic
	// font width depends on your font
	if (uchar >= 0x0400 && uchar <= 0x04ff)
		return CHAR_WIDTH_UNKNOWN;

	if (uchar < 0x1000)
		return CHAR_WIDTH_HALFWIDTH;

	// Halfwidth CJK Chars
	if (uchar >= 0xff61 && uchar <= 0xffdc)
		return CHAR_WIDTH_HALFWIDTH;

	// Halfwidth Symbols Variants
	if (uchar >= 0xffe8 && uchar <= 0xffee)
		return CHAR_WIDTH_HALFWIDTH;

	return CHAR_WIDTH_FULLWIDTH;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

#ifdef UI_COLOR_DISPLAY
/*-------------------------------------------------
    setup_palette - set up the ui palette
-------------------------------------------------*/

static void setup_palette(running_machine &machine)
{
	static struct
	{
		const char *name;
		int color;
		UINT8 defval[3];
	} palette_decode_table[] =
	{
		{ OPTION_SYSTEM_BACKGROUND,     SYSTEM_COLOR_BACKGROUND,  { 16,16,48 } },
		{ OPTION_CURSOR_SELECTED_TEXT,  CURSOR_SELECTED_TEXT,     { 255,255,255 } },
		{ OPTION_CURSOR_SELECTED_BG,    CURSOR_SELECTED_BG,       { 60,120,240 } },
		{ OPTION_CURSOR_HOVER_TEXT,     CURSOR_HOVER_TEXT,        { 120,180,240 } },
		{ OPTION_CURSOR_HOVER_BG,       CURSOR_HOVER_BG,          { 32,32,0 } },
		{ OPTION_BUTTON_RED,            BUTTON_COLOR_RED,         { 255,64,64 } },
		{ OPTION_BUTTON_YELLOW,         BUTTON_COLOR_YELLOW,      { 255,238,0 } },
		{ OPTION_BUTTON_GREEN,          BUTTON_COLOR_GREEN,       { 0,255,64 } },
		{ OPTION_BUTTON_BLUE,           BUTTON_COLOR_BLUE,        { 0,170,255 } },
		{ OPTION_BUTTON_PURPLE,         BUTTON_COLOR_PURPLE,      { 170,0,255 } },
		{ OPTION_BUTTON_PINK,           BUTTON_COLOR_PINK,        { 255,0,170 } },
		{ OPTION_BUTTON_AQUA,           BUTTON_COLOR_AQUA,        { 0,255,204 } },
		{ OPTION_BUTTON_SILVER,         BUTTON_COLOR_SILVER,      { 255,0,255 } },
		{ OPTION_BUTTON_NAVY,           BUTTON_COLOR_NAVY,        { 255,160,0 } },
		{ OPTION_BUTTON_LIME,           BUTTON_COLOR_LIME,        { 190,190,190 } },
		{ NULL }
	};

	int i;

#ifdef TRANS_UI
	ui_transparency = 255;

	ui_transparency = machine.options().int_value(OPTION_UI_TRANSPARENCY);
	if (ui_transparency < 0 || ui_transparency > 255)
	{
		mame_printf_error(_("Illegal value for %s = %s\n"), OPTION_UI_TRANSPARENCY, machine.options().value(OPTION_UI_TRANSPARENCY));
		ui_transparency = 215;
	}
#endif /* TRANS_UI */

	for (i = 0; palette_decode_table[i].name; i++)
	{
		const char *value = machine.options().value(palette_decode_table[i].name);
		int col = palette_decode_table[i].color;
		int r = palette_decode_table[i].defval[0];
		int g = palette_decode_table[i].defval[1];
		int b = palette_decode_table[i].defval[2];
		int rate;

		if (value)
		{
			int pal[3];

			if (sscanf(value, "%d,%d,%d", &pal[0], &pal[1], &pal[2]) != 3 ||
				pal[0] < 0 || pal[0] >= 256 ||
				pal[1] < 0 || pal[1] >= 256 ||
				pal[2] < 0 || pal[2] >= 256 )
			{
				mame_printf_error(_("error: invalid value for palette: %s\n"), value);
				continue;
			}

			r = pal[0];
			g = pal[1];
			b = pal[2];
		}

		rate = 0xff;
#ifdef TRANS_UI
		if (col == UI_BACKGROUND_COLOR)
			rate = ui_transparency;
		else
		if (col == CURSOR_SELECTED_BG)
		{
			rate = ui_transparency / 2;
			if (rate < 128)
				rate = 128; //cursor should be visible
		}
#endif /* TRANS_UI */

		uifont_colortable[col] = MAKE_ARGB(rate, r, g, b);
	}
}
#endif /* UI_COLOR_DISPLAY */


/*-------------------------------------------------
    ui_init - set up the user interface
-------------------------------------------------*/

int ui_init(running_machine &machine)
{
	/* make sure we clean up after ourselves */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_exit), &machine));

#ifdef UI_COLOR_DISPLAY
	setup_palette(machine);
#endif /* UI_COLOR_DISPLAY */
	build_bgtexture(machine);
	ui_bgcolor = UI_BACKGROUND_COLOR;

	/* initialize the other UI bits */
	ui_menu::init(machine);
	ui_gfx_init(machine);

#ifdef CMD_LIST
	datafile_init(machine, &machine.options());
#endif /* CMD_LIST */

#ifdef USE_SHOW_TIME
	show_time = 0;
	Show_Time_Position = 0;
#endif /* USE_SHOW_TIME */

	/* reset globals */
	single_step = FALSE;
	ui_set_handler(handler_messagebox, 0);
	/* retrieve options */
	ui_use_natural_keyboard = machine.options().natural_keyboard();

	return 0;
}


/*-------------------------------------------------
    ui_exit - clean up ourselves on exit
-------------------------------------------------*/

static void ui_exit(running_machine &machine)
{
	/* free the font */
	machine.render().font_free(ui_font);
	ui_font = NULL;
}


/*-------------------------------------------------
    ui_display_startup_screens - display the
    various startup screens
-------------------------------------------------*/

int ui_display_startup_screens(running_machine &machine, int first_time, int show_disclaimer)
{
	const int maxstate = 3;
	int str = machine.options().seconds_to_run();
	int show_gameinfo = !machine.options().skip_gameinfo();
	int show_warnings = !machine.options().skip_gameinfo();
	int state;

	/* disable everything if we are using -str for 300 or fewer seconds, or if we're the empty driver,
       or if we are debugging */
	if (!first_time || (str > 0 && str < 60*5) || &machine.system() == &GAME_NAME(___empty) || (machine.debug_flags & DEBUG_FLAG_ENABLED) != 0)
		show_gameinfo = show_warnings = show_disclaimer = FALSE;

	/* initialize the on-screen display system */
	slider_list = slider_current = slider_init(machine);

	/* loop over states */
	ui_set_handler(handler_ingame, 0);
	for (state = 0; state < maxstate && !machine.scheduled_event_pending() && !ui_menu::stack_has_special_main_menu(); state++)
	{
		/* default to standard colors */
		messagebox_backcolor = UI_BACKGROUND_COLOR;

		/* pick the next state */
		switch (state)
		{
			case 0:
				if (show_disclaimer && disclaimer_string(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_ok, 0);
				break;

			case 1:
				if (show_warnings && warnings_string(machine, messagebox_text).len() > 0)
				{
					ui_set_handler(handler_messagebox_ok, 0);
					if (machine.system().flags & (GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS | GAME_REQUIRES_ARTWORK | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NO_SOUND))
						messagebox_backcolor = UI_YELLOW_COLOR;
					if (machine.system().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL))
						messagebox_backcolor = UI_RED_COLOR;
				}
				break;

			case 2:
				if (show_gameinfo && game_info_astring(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_anykey, 0);
				break;
		}

		/* clear the input memory */
		machine.input().reset_polling();
		while (machine.input().poll_switches() != INPUT_CODE_INVALID) ;

		/* loop while we have a handler */
		while (ui_handler_callback != handler_ingame && !machine.scheduled_event_pending() && !ui_menu::stack_has_special_main_menu())
			machine.video().frame_update();

		/* clear the handler and force an update */
		ui_set_handler(handler_ingame, 0);
		machine.video().frame_update();
	}

	/* if we're the empty driver, force the menus on */
	if (ui_menu::stack_has_special_main_menu())
		ui_set_handler(ui_menu::ui_handler, 0);

	return 0;
}


/*-------------------------------------------------
    ui_set_startup_text - set the text to display
    at startup
-------------------------------------------------*/

void ui_set_startup_text(running_machine &machine, const char *text, int force)
{
	static osd_ticks_t lastupdatetime = 0;
	osd_ticks_t curtime = osd_ticks();

	/* copy in the new text */
	messagebox_text.cpy(text);
	messagebox_backcolor = UI_BACKGROUND_COLOR;

	/* don't update more than 4 times/second */
	if (force || (curtime - lastupdatetime) > osd_ticks_per_second() / 4)
	{
		lastupdatetime = curtime;
		machine.video().frame_update();
	}
}


/*-------------------------------------------------
    ui_update_and_render - update the UI and
    render it; called by video.c
-------------------------------------------------*/

void ui_update_and_render(running_machine &machine, render_container *container)
{
	/* always start clean */
	container->empty();

	/* if we're paused, dim the whole screen */
	if (machine.phase() >= MACHINE_PHASE_RESET && (single_step || machine.paused()))
	{
		int alpha = (1.0f - machine.options().pause_brightness()) * 255.0f;
		if (ui_menu::stack_has_special_main_menu())
			alpha = 255;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			container->add_rect(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* render any cheat stuff at the bottom */
	if (machine.phase() >= MACHINE_PHASE_RESET)
		machine.cheat().render_text(*container);

	/* call the current UI handler */
	assert(ui_handler_callback != NULL);
	ui_handler_param = (*ui_handler_callback)(machine, container, ui_handler_param);

	/* display any popup messages */
	if (osd_ticks() < popup_text_end)
		ui_draw_text_box(container, messagebox_text, JUSTIFY_CENTER, 0.5f, 0.9f, messagebox_backcolor);
	else
		popup_text_end = 0;

	/* cancel takes us back to the ingame handler */
	if (ui_handler_param == UI_HANDLER_CANCEL)
		ui_set_handler(handler_ingame, 0);
}


/*-------------------------------------------------
    ui_get_font - return the UI font
-------------------------------------------------*/

render_font *ui_get_font(running_machine &machine)
{
	/* allocate the font and messagebox string */
	if (ui_font == NULL)
		ui_font = machine.render().font_alloc(machine.options().ui_font());
	return ui_font;
}


/*-------------------------------------------------
    ui_get_line_height - return the current height
    of a line
-------------------------------------------------*/

float ui_get_line_height(running_machine &machine)
{
	INT32 raw_font_pixel_height = ui_get_font(machine)->pixel_height();
	render_target &ui_target = machine.render().ui_target();
	INT32 target_pixel_height = ui_target.height();
	float one_to_one_line_height;
	float scale_factor;

	/* mamep: to avoid division by zero */
	if (target_pixel_height == 0)
		return 0.0f;

	/* compute the font pixel height at the nominal size */
	one_to_one_line_height = (float)raw_font_pixel_height / (float)target_pixel_height;

	/* determine the scale factor */
	scale_factor = UI_TARGET_FONT_HEIGHT / one_to_one_line_height;

	/* if our font is small-ish, do integral scaling */
	if (raw_font_pixel_height < 24)
	{
		/* do we want to scale smaller? only do so if we exceed the threshhold */
		if (scale_factor <= 1.0f)
		{
			if (one_to_one_line_height < UI_MAX_FONT_HEIGHT || raw_font_pixel_height < 12)
				scale_factor = 1.0f;
		}

		/* otherwise, just ensure an integral scale factor */
		else
			scale_factor = floor(scale_factor);
	}

	/* otherwise, just make sure we hit an even number of pixels */
	else
	{
		INT32 height = scale_factor * one_to_one_line_height * (float)target_pixel_height;
		scale_factor = (float)height / (one_to_one_line_height * (float)target_pixel_height);
	}

	return scale_factor * one_to_one_line_height;
}


/*-------------------------------------------------
    ui_get_char_width - return the width of a
    single character
-------------------------------------------------*/

float ui_get_char_width(running_machine &machine, unicode_char ch)
{
	return ui_get_font(machine)->char_width(ui_get_line_height(machine), machine.render().ui_aspect(), ch);
}


//mamep: to render as fixed-width font
float ui_get_char_width_no_margin(running_machine &machine, unicode_char ch)
{
	return ui_get_font(machine)->char_width_no_margin(ui_get_line_height(machine), machine.render().ui_aspect(), ch);
}


float ui_get_char_fixed_width(running_machine &machine, unicode_char uchar, double halfwidth, double fullwidth)
{
	float chwidth;

	switch (is_fullwidth_char(uchar))
	{
	case CHAR_WIDTH_HALFWIDTH:
		return halfwidth;

	case CHAR_WIDTH_UNKNOWN:
		chwidth = ui_get_char_width_no_margin(machine, uchar);
		if (chwidth <= halfwidth)
			return halfwidth;
	}

	return fullwidth;
}


/*-------------------------------------------------
    ui_get_string_width - return the width of a
    character string
-------------------------------------------------*/

float ui_get_string_width(running_machine &machine, const char *s)
{
	return ui_get_font(machine)->utf8string_width(ui_get_line_height(machine), machine.render().ui_aspect(), s);
}


/*-------------------------------------------------
    ui_draw_box - add primitives to draw
    a box with the given background color
-------------------------------------------------*/

static void ui_draw_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor)
{
#ifdef UI_COLOR_DISPLAY
	if (backcolor == UI_BACKGROUND_COLOR)
		container->add_quad(x0, y0, x1, y1, MAKE_ARGB(0xff, 0xff, 0xff, 0xff), bgtexture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	else
#endif /* UI_COLOR_DISPLAY */
		container->add_rect(x0, y0, x1, y1, backcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


/*-------------------------------------------------
    ui_draw_outlined_box - add primitives to draw
    an outlined box with the given background
    color
-------------------------------------------------*/

void ui_draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	ui_draw_box(container, x0, y0, x1, y1, backcolor);
	container->add_line(x0, y0, x1, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x1, y0, x1, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x1, y1, x0, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container->add_line(x0, y1, x0, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


/*-------------------------------------------------
    ui_draw_text - simple text renderer
-------------------------------------------------*/

void ui_draw_text(render_container *container, const char *buf, float x, float y)
{
	ui_draw_text_full(container, buf, x, y, 1.0f - x, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/*-------------------------------------------------
    ui_draw_text_full - full featured text
    renderer with word wrapping, justification,
    and full size computation
-------------------------------------------------*/

void ui_draw_text_full(render_container *container, const char *origs, float x, float y, float origwrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	running_machine &machine = container->manager().machine();
	float lineheight = ui_get_line_height(machine);
	const char *ends = origs + strlen(origs);
	float wrapwidth = origwrapwidth;
	const char *s = origs;
	const char *linestart;
	float cury = y;
	float maxwidth = 0;
	const char *s_temp;
	const char *up_arrow = NULL;
	const char *down_arrow = _("(more)");

	//mamep: control scrolling text
	int curline = 0;

	//mamep: render as fixed-width font
	float fontwidth_halfwidth = 0.0f;
	float fontwidth_fullwidth = 0.0f;

	if (draw_text_fixed_mode)
	{
		int scharcount;
		int len = strlen(origs);
		int n;

		for (n = 0; len > 0; n += scharcount, len -= scharcount)
		{
			unicode_char schar;
			float scharwidth;

			scharcount = uchar_from_utf8(&schar, &origs[n], len);
			if (scharcount == -1)
				break;

			scharwidth = ui_get_char_width_no_margin(machine, schar);
			if (is_fullwidth_char(schar))
			{
				if (fontwidth_fullwidth < scharwidth)
					fontwidth_fullwidth = scharwidth;
			}
			else
			{
				if (fontwidth_halfwidth < scharwidth)
					fontwidth_halfwidth = scharwidth;
			}
		}

		if (fontwidth_fullwidth < fontwidth_halfwidth * 2.0f)
			fontwidth_fullwidth = fontwidth_halfwidth * 2.0f;
		if (fontwidth_halfwidth < fontwidth_fullwidth / 2.0f)
			fontwidth_halfwidth = fontwidth_fullwidth / 2.0f;
	}

	//mamep: check if we are scrolling
	if (draw_text_scroll_offset)
		up_arrow = _("(more)");
	if (draw_text_scroll_offset == multiline_text_box_target_lines - multiline_text_box_visible_lines)
		down_arrow = NULL;

	/* if we don't want wrapping, guarantee a huge wrapwidth */
	if (wrap == WRAP_NEVER)
		wrapwidth = 1000000.0f;
	if (wrapwidth <= 0)
		return;

	/* loop over lines */
	while (*s != 0)
	{
		const char *lastbreak = NULL;
		int line_justify = justify;
		unicode_char schar;
		int scharcount;
		float lastbreak_width = 0;
		float curwidth = 0;
		float curx = x;

		/* get the current character */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		/* if the line starts with a tab character, center it regardless */
		if (schar == '\t')
		{
			s += scharcount;
			line_justify = JUSTIFY_CENTER;
		}

		/* remember the starting position of the line */
		linestart = s;

		/* loop while we have characters and are less than the wrapwidth */
		while (*s != 0 && curwidth <= wrapwidth)
		{
			float chwidth;

			/* get the current chcaracter */
			scharcount = uchar_from_utf8(&schar, s, ends - s);
			if (scharcount == -1)
				break;

			/* if we hit a newline, stop immediately */
			if (schar == '\n')
				break;

			//mamep: render as fixed-width font
			if (draw_text_fixed_mode)
				chwidth = ui_get_char_fixed_width(machine, schar, fontwidth_halfwidth, fontwidth_fullwidth);
			else
				/* get the width of this character */
			chwidth = ui_get_char_width(machine, schar);

			/* if we hit a space, remember the location and width *without* the space */
			if (schar == ' ')
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}

			/* add the width of this character and advance */
			curwidth += chwidth;
			s += scharcount;

			/* if we hit any non-space breakable character, remember the location and width
               *with* the breakable character */
			if (schar != ' ' && is_breakable_char(schar) && curwidth <= wrapwidth)
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}
		}

		/* if we accumulated too much for the current width, we need to back off */
		if (curwidth > wrapwidth)
		{
			/* if we're word wrapping, back up to the last break if we can */
			if (wrap == WRAP_WORD)
			{
				/* if we hit a break, back up to there with the appropriate width */
				if (lastbreak != NULL)
				{
					s = lastbreak;
					curwidth = lastbreak_width;
				}

				/* if we didn't hit a break, back up one character */
				else if (s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					//mamep: render as fixed-width font
					if (draw_text_fixed_mode)
						curwidth -= ui_get_char_fixed_width(machine, schar, fontwidth_halfwidth, fontwidth_fullwidth);
					else
						curwidth -= ui_get_char_width(machine, schar);
				}
			}

			/* if we're truncating, make sure we have enough space for the ... */
			else if (wrap == WRAP_TRUNCATE)
			{
				/* add in the width of the ... */
				curwidth += 3.0f * ui_get_char_width(machine, '.');

				/* while we are above the wrap width, back up one character */
				while (curwidth > wrapwidth && s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					curwidth -= ui_get_char_width(machine, schar);
				}
			}
		}

		//mamep: add scrolling arrow
		if (draw != DRAW_NONE
		 && ((curline == 0 && up_arrow)
		 ||  (curline == multiline_text_box_visible_lines - 1 && down_arrow)))
		{
			if (curline == 0)
				linestart = up_arrow;
			else
				linestart = down_arrow;

			curwidth = ui_get_string_width(machine, linestart);
			ends = linestart + strlen(linestart);
			s_temp = ends;
			line_justify = JUSTIFY_CENTER;
		}
		else
			s_temp = s;

		/* align according to the justfication */
		if (line_justify == JUSTIFY_CENTER)
			curx += (origwrapwidth - curwidth) * 0.5f;
		else if (line_justify == JUSTIFY_RIGHT)
			curx += origwrapwidth - curwidth;

		/* track the maximum width of any given line */
		if (curwidth > maxwidth)
			maxwidth = curwidth;

		/* if opaque, add a black box */
		if (draw == DRAW_OPAQUE)
			ui_draw_box(container, curx, cury, curx + curwidth, cury + lineheight, bgcolor);

		/* loop from the line start and add the characters */

		while (linestart < s_temp)
		{
			/* get the current character */
			unicode_char linechar;
			int linecharcount = uchar_from_utf8(&linechar, linestart, ends - linestart);
			if (linecharcount == -1)
				break;

			//mamep: consume the offset lines
			if (draw_text_scroll_offset == 0 && draw != DRAW_NONE)
			{
				//mamep: render as fixed-width font
				if (draw_text_fixed_mode)
				{
					float width = ui_get_char_fixed_width(machine, linechar, fontwidth_halfwidth, fontwidth_fullwidth);
					float xmargin = (width - ui_get_char_width(machine, linechar)) / 2.0f;

					container->add_char(curx + xmargin, cury, lineheight, machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), linechar);
					curx += width;
				}
				else
				{
					container->add_char(curx, cury, lineheight, machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), linechar);
					curx += ui_get_char_width(machine, linechar);
				}
			}
			linestart += linecharcount;
		}

		/* append ellipses if needed */
		if (wrap == WRAP_TRUNCATE && *s != 0 && draw != DRAW_NONE)
		{
			container->add_char(curx, cury, lineheight, machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_char_width(machine, '.');
			container->add_char(curx, cury, lineheight, machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_char_width(machine, '.');
			container->add_char(curx, cury, lineheight, machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), '.');
			curx += ui_get_char_width(machine, '.');
		}

		/* if we're not word-wrapping, we're done */
		if (wrap != WRAP_WORD)
			break;

		//mamep: text scrolling
		if (draw_text_scroll_offset > 0)
			draw_text_scroll_offset--;
		else
		/* advance by a row */
		{
			cury += lineheight;

			//mamep: skip overflow text
			//there's a bug when viewing the game information and bookkeeping,so we have to commet it
 			if (draw != DRAW_NONE && curline == multiline_text_box_visible_lines - 1 && down_arrow)
				break;

			//mamep: controll scrolling text
			if (draw_text_scroll_offset == 0)
				curline++;
		}

		/* skip past any spaces at the beginning of the next line */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		if (schar == '\n')
			s += scharcount;
		else
			while (*s && (schar < 0x80) && isspace(schar))
			{
				s += scharcount;
				scharcount = uchar_from_utf8(&schar, s, ends - s);
				if (scharcount == -1)
					break;
			}
	}

	/* report the width and height of the resulting space */
	if (totalwidth)
		*totalwidth = maxwidth;
	if (totalheight)
		*totalheight = cury - y;
}


static int ui_draw_text_set_fixed_width_mode(int mode)
{
	int mode_save = draw_text_fixed_mode;

	draw_text_fixed_mode = mode;

	return mode_save;
}


void ui_draw_text_full_fixed_width(render_container *container, const char *origs, float x, float y, float wrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	int mode_save = ui_draw_text_set_fixed_width_mode(TRUE);

	ui_draw_text_full(container, origs, x, y, wrapwidth, justify, wrap, draw, fgcolor, bgcolor, totalwidth, totalheight);
	ui_draw_text_set_fixed_width_mode(mode_save);
}


void ui_draw_text_full_scroll(render_container *container, const char *origs, float x, float y, float wrapwidth, int offset, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	int offset_save = draw_text_scroll_offset;

	draw_text_scroll_offset = offset;
	ui_draw_text_full(container, origs, x, y, wrapwidth, justify, wrap, draw, fgcolor, bgcolor, totalwidth, totalheight);

	draw_text_scroll_offset = offset_save;
}


/*-------------------------------------------------
    ui_draw_text_box - draw a multiline text
    message with a box around it
-------------------------------------------------*/

void ui_draw_text_box_scroll(render_container *container, const char *text, int offset, int justify, float xpos, float ypos, rgb_t backcolor)
{
	float line_height = ui_get_line_height(container->manager().machine());
	float max_width = 2.0f * ((xpos <= 0.5f) ? xpos : 1.0f - xpos) - 2.0f * UI_BOX_LR_BORDER;
	float target_width = max_width;
	float target_height = line_height;
	float target_x = 0, target_y = 0;
	float last_target_height = 0;

	// limit this iteration to a finite number of passes
	for (int pass = 0; pass < 5; pass++)
	{
		/* determine the target location */
		target_x = xpos - 0.5f * target_width;
		target_y = ypos - 0.5f * target_height;

		/* make sure we stay on-screen */
		if (target_x < UI_BOX_LR_BORDER)
			target_x = UI_BOX_LR_BORDER;
		if (target_x + target_width + UI_BOX_LR_BORDER > 1.0f)
			target_x = 1.0f - UI_BOX_LR_BORDER - target_width;
		if (target_y < UI_BOX_TB_BORDER)
			target_y = UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > 1.0f)
			target_y = 1.0f - UI_BOX_TB_BORDER - target_height;

		/* compute the multi-line target width/height */
		ui_draw_text_full(container, text, target_x, target_y, target_width + 0.00001f,
					justify, WRAP_WORD, DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &target_width, &target_height);

		multiline_text_box_target_lines = (int)(target_height / line_height + 0.5f);
		if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
			target_height = floor((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;
		multiline_text_box_visible_lines = (int)(target_height / line_height + 0.5f);

		/* if we match our last value, we're done */
		if (target_height == last_target_height)
			break;
		last_target_height = target_height;
	}

	/* add a box around that */
	ui_draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
					 target_y - UI_BOX_TB_BORDER,
					 target_x + target_width + UI_BOX_LR_BORDER,
					 target_y + target_height + UI_BOX_TB_BORDER, backcolor);
	ui_draw_text_full_scroll(container, text, target_x, target_y, target_width + 0.00001f, offset,
				justify, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


void ui_draw_text_box(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	ui_draw_text_box_scroll(container, text, message_window_scroll, justify, xpos, ypos, backcolor);
}


#if defined(CMD_LIST) || defined(USE_SHOW_TIME)
void ui_draw_text_box_fixed_width(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	int mode_save = draw_text_fixed_mode;

	draw_text_fixed_mode = 1;
	ui_draw_text_box_scroll(container, text, message_window_scroll, justify, xpos, ypos, backcolor);

	draw_text_fixed_mode = mode_save;
}
#endif /* CMD_LIST */


int ui_window_scroll_keys(running_machine &machine)
{
	static int counter = 0;
	static int fast = 6;
	int pan_lines;
	int max_scroll;
	int do_scroll = FALSE;

	max_scroll = multiline_text_box_target_lines - multiline_text_box_visible_lines;
	pan_lines = multiline_text_box_visible_lines - 2;

	if (scroll_reset)
	{
		message_window_scroll = 0;
		scroll_reset = 0;
	}

	/* up backs up by one item */
	if (ui_input_pressed_repeat(machine, IPT_UI_UP, fast))
	{
		message_window_scroll--;
		do_scroll = TRUE;
	}

	/* down advances by one item */
	if (ui_input_pressed_repeat(machine, IPT_UI_DOWN, fast))
	{
		message_window_scroll++;
		do_scroll = TRUE;
	}

	/* pan-up goes to previous page */
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_UP,8))
	{
		message_window_scroll -= pan_lines;
		do_scroll = TRUE;
	}

	/* pan-down goes to next page */
	if (ui_input_pressed_repeat(machine, IPT_UI_PAGE_DOWN,8))
	{
		message_window_scroll += pan_lines;
		do_scroll = TRUE;
	}

	/* home goes to the start */
	if (ui_input_pressed(machine, IPT_UI_HOME))
	{
		message_window_scroll = 0;
		do_scroll = TRUE;
	}

	/* end goes to the last */
	if (ui_input_pressed(machine, IPT_UI_END))
	{
		message_window_scroll = max_scroll;
		do_scroll = TRUE;
	}

	if (message_window_scroll < 0)
		message_window_scroll = 0;
	if (message_window_scroll > max_scroll)
		message_window_scroll = max_scroll;

	if (input_type_pressed(machine, IPT_UI_UP,0) || input_type_pressed(machine, IPT_UI_DOWN,0))
	{
		if (++counter == 25)
		{
			fast--;
			if (fast < 1)
				fast = 0;

			counter = 0;
		}
	}
	else
	{
		fast = 6;
		counter = 0;
	}

	if (do_scroll)
		return -1;

	if (ui_input_pressed(machine, IPT_UI_SELECT))
	{
		message_window_scroll = 0;
		return 1;
	}
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		message_window_scroll = 0;
		return 2;
	}

	return 0;
}


/*-------------------------------------------------
    ui_popup_time - popup a message for a specific
    amount of time
-------------------------------------------------*/

void CLIB_DECL ui_popup_time(int seconds, const char *text, ...)
{
	va_list arg;

	/* extract the text */
	va_start(arg,text);
	messagebox_text.vprintf(text, arg);
	messagebox_backcolor = UI_BACKGROUND_COLOR;
	va_end(arg);

	/* set a timer */
	popup_text_end = osd_ticks() + osd_ticks_per_second() * seconds;
}


/*-------------------------------------------------
    ui_show_fps_temp - show the FPS counter for
    a specific period of time
-------------------------------------------------*/

void ui_show_fps_temp(double seconds)
{
	if (!showfps)
		showfps_end = osd_ticks() + seconds * osd_ticks_per_second();
}


/*-------------------------------------------------
    ui_set_show_fps - show/hide the FPS counter
-------------------------------------------------*/

void ui_set_show_fps(int show)
{
	showfps = show;
	if (!show)
	{
		showfps = 0;
		showfps_end = 0;
	}
}


/*-------------------------------------------------
    ui_get_show_fps - return the current FPS
    counter visibility state
-------------------------------------------------*/

int ui_get_show_fps(void)
{
	return showfps || (showfps_end != 0);
}


/*-------------------------------------------------
    ui_set_show_profiler - show/hide the profiler
-------------------------------------------------*/

void ui_set_show_profiler(int show)
{
	show_profiler = show;
	g_profiler.enable(show);
}


/*-------------------------------------------------
    ui_get_show_profiler - return the current
    profiler visibility state
-------------------------------------------------*/

int ui_get_show_profiler(void)
{
	return show_profiler;
}


/*-------------------------------------------------
    ui_show_menu - show the menus
-------------------------------------------------*/

void ui_show_menu(void)
{
	ui_set_handler(ui_menu::ui_handler, 0);
}


/*-------------------------------------------------
    ui_is_menu_active - return TRUE if the menu
    UI handler is active
-------------------------------------------------*/

int ui_is_menu_active(void)
{
	return (ui_handler_callback == ui_menu::ui_handler);
}



/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    disclaimer_string - print the disclaimer
    text to the given buffer
-------------------------------------------------*/

static astring &disclaimer_string(running_machine &machine, astring &string)
{
	string.cpy(_("Usage of emulators in conjunction with ROMs you don't own is forbidden by copyright law.\n\n"));
	string.catprintf(_("IF YOU ARE NOT LEGALLY ENTITLED TO PLAY \"%s\" ON THIS EMULATOR, PRESS ESC.\n\n"), _LST(machine.system().description));
	string.cat(_("Otherwise, type OK or move the joystick left then right to continue"));
	return string;
}


/*-------------------------------------------------
    warnings_string - print the warning flags
    text to the given buffer
-------------------------------------------------*/

static astring &warnings_string(running_machine &machine, astring &string)
{
#define WARNING_FLAGS (	GAME_NOT_WORKING | \
						GAME_UNEMULATED_PROTECTION | \
						GAME_MECHANICAL | \
						GAME_WRONG_COLORS | \
						GAME_IMPERFECT_COLORS | \
						GAME_REQUIRES_ARTWORK | \
						GAME_NO_SOUND |  \
						GAME_IMPERFECT_SOUND |  \
						GAME_IMPERFECT_GRAPHICS | \
						GAME_NO_COCKTAIL)

	string.reset();

	/* if no warnings, nothing to return */
	if (rom_load_warnings(machine) == 0 && rom_load_knownbad(machine) == 0 && !(machine.system().flags & WARNING_FLAGS))
		return string;

	/* add a warning if any ROMs were loaded with warnings */
	if (rom_load_warnings(machine) > 0)
	{
		string.cat(_("One or more ROMs/CHDs for this game are incorrect. The "));
		string.cat(emulator_info::get_gamenoun());
		string.cat(_(" may not run correctly.\n"));
		if (machine.system().flags & WARNING_FLAGS)
			string.cat("\n");
	}

	/* if we have at least one warning flag, print the general header */
	if ((machine.system().flags & WARNING_FLAGS) || rom_load_knownbad(machine) > 0)
	{
		string.cat(_("There are known problems with this "));
		string.cat(emulator_info::get_gamenoun());
		string.cat("\n\n");

		/* add a warning if any ROMs are flagged BAD_DUMP/NO_DUMP */
		if (rom_load_knownbad(machine) > 0) {
			string.cat(_("One or more ROMs/CHDs for this "));
			string.cat(emulator_info::get_gamenoun());
			string.cat(_(" have not been correctly dumped.\n"));
		}
		/* add one line per warning flag */
		if (input_machine_has_keyboard(machine))
			string.cat(_("The keyboard emulation may not be 100% accurate.\n"));
		if (machine.system().flags & GAME_IMPERFECT_COLORS)
			string.cat(_("The colors aren't 100% accurate.\n"));
		if (machine.system().flags & GAME_WRONG_COLORS)
			string.cat(_("The colors are completely wrong.\n"));
		if (machine.system().flags & GAME_IMPERFECT_GRAPHICS)
			string.cat(_("The video emulation isn't 100% accurate.\n"));
		if (machine.system().flags & GAME_IMPERFECT_SOUND)
			string.cat(_("The sound emulation isn't 100% accurate.\n"));
		if (machine.system().flags & GAME_NO_SOUND)
			string.cat(_("The game lacks sound.\n"));
		if (machine.system().flags & GAME_NO_COCKTAIL)
			string.cat(_("Screen flipping in cocktail mode is not supported.\n"));

		/* check if external artwork is present before displaying this warning? */
		if (machine.system().flags & GAME_REQUIRES_ARTWORK)
			string.cat(_("The game requires external artwork files\n"));

		/* if there's a NOT WORKING, UNEMULATED PROTECTION or GAME MECHANICAL warning, make it stronger */
		if (machine.system().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL))
		{
			/* add the strings for these warnings */
			if (machine.system().flags & GAME_UNEMULATED_PROTECTION)
				string.cat(_("The game has protection which isn't fully emulated.\n"));
			if (machine.system().flags & GAME_NOT_WORKING) {
				string.cat(_("\nTHIS "));
				string.cat(emulator_info::get_capgamenoun());
				string.cat(_(" DOESN'T WORK. The emulation for this game is not yet complete. "
					 "There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n"));
			}
			if (machine.system().flags & GAME_MECHANICAL) {
				string.cat(_("\nCertain elements of this "));
				string.cat(emulator_info::get_gamenoun());
				string.cat(_(" cannot be emulated as it requires actual physical interaction or consists of mechanical devices. "
					 "It is not possible to fully play this "));
				string.cat(emulator_info::get_gamenoun());
				string.cat(".\n");
			}

			/* find the parent of this driver */
			driver_enumerator drivlist(machine.options());
			int maindrv = drivlist.find(machine.system());
			int clone_of = drivlist.non_bios_clone(maindrv);
			if (clone_of != -1)
				maindrv = clone_of;

			/* scan the driver list for any working clones and add them */
			bool foundworking = false;
			while (drivlist.next())
				if (drivlist.current() == maindrv || drivlist.clone() == maindrv)
					if ((drivlist.driver().flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_MECHANICAL)) == 0)
					{
						/* this one works, add a header and display the name of the clone */
						if (!foundworking)
							string.cat(_("\n\nThere are working clones of this game: "));
						else
							string.cat(", ");
						string.cat(drivlist.driver().name);
						foundworking = true;
					}

			if (foundworking)
				string.cat("\n");
		}
	}

	/* add the 'press OK' string */
	string.cat(_("\n\nType OK or move the joystick left then right to continue"));
	return string;
}


/*-------------------------------------------------
    game_info_astring - populate an allocated
    string with the game info text
-------------------------------------------------*/

astring &game_info_astring(running_machine &machine, astring &string)
{
	/* print description, manufacturer, and CPU: */
	astring tempstr;
	string.printf("%s\n%s %s\nDriver: %s\n\nCPU:\n", _LST(machine.system().description), machine.system().year, _MANUFACT(machine.system().manufacturer), core_filename_extract_base(tempstr, machine.system().source_file).cstr());

	/* loop over all CPUs */
	execute_interface_iterator execiter(machine.root_device());
	tagmap_t<UINT8> exectags;
	for (device_execute_interface *exec = execiter.first(); exec != NULL; exec = execiter.next())
	{
		if (exectags.add(exec->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
			continue;
		/* get cpu specific clock that takes internal multiplier/dividers into account */
		int clock = exec->device().clock();

		/* count how many identical CPUs we have */
		int count = 1;
		execute_interface_iterator execinneriter(machine.root_device());
		for (device_execute_interface *scan = execinneriter.first(); scan != NULL; scan = execinneriter.next())
		{
			if (exec->device().type() == scan->device().type() && exec->device().clock() == scan->device().clock())
				if (exectags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
			count++;
		}

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(exec->device().name());

		/* display clock in kHz or MHz */
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
	}

	/* loop over all sound chips */
	sound_interface_iterator snditer(machine.root_device());
	tagmap_t<UINT8> soundtags;
	bool found_sound = false;
	for (device_sound_interface *sound = snditer.first(); sound != NULL; sound = snditer.next())
	{
		if (soundtags.add(sound->device().tag(), 1, FALSE) == TMERR_DUPLICATE)
			continue;

		/* append the Sound: string */
		if (!found_sound)
			string.cat(_("\nSound:\n"));
		found_sound = true;

		/* count how many identical sound chips we have */
		int count = 1;
		sound_interface_iterator sndinneriter(machine.root_device());
		for (device_sound_interface *scan = sndinneriter.first(); scan != NULL; scan = sndinneriter.next())
		{
			if (sound->device().type() == scan->device().type() && sound->device().clock() == scan->device().clock())
				if (soundtags.add(scan->device().tag(), 1, FALSE) != TMERR_DUPLICATE)
			count++;
		}
		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(sound->device().name());

		/* display clock in kHz or MHz */
		int clock = sound->device().clock();
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else if (clock != 0)
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
		else
			string.cat("\n");
	}

	/* display screen information */
	string.cat(_("\nVideo:\n"));
	screen_device_iterator scriter(machine.root_device());
	int scrcount = scriter.count();
	if (scrcount == 0)
		string.cat(_("None\n"));
	else
	{
		for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		{
			if (scrcount > 1)
			{
				string.cat(slider_get_screen_desc(*screen));
				string.cat(": ");
			}

			if (screen->screen_type() == SCREEN_TYPE_VECTOR)
				string.cat(_("Vector\n"));
			else
			{
				const rectangle &visarea = screen->visible_area();

				string.catprintf("%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz\n",
						visarea.width(), visarea.height(),
						(machine.system().flags & ORIENTATION_SWAP_XY) ? "V" : "H",
						ATTOSECONDS_TO_HZ(screen->frame_period().attoseconds));
			}
		}
	}

	return string;
}



/***************************************************************************
    UI HANDLERS
***************************************************************************/

/*-------------------------------------------------
    handler_messagebox - displays the current
    messagebox_text string but handles no input
-------------------------------------------------*/

static UINT32 handler_messagebox(running_machine &machine, render_container *container, UINT32 state)
{
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);
	return 0;
}


/*-------------------------------------------------
    handler_messagebox_ok - displays the current
    messagebox_text string and waits for an OK
-------------------------------------------------*/

static UINT32 handler_messagebox_ok(running_machine &machine, render_container *container, UINT32 state)
{
	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* an 'O' or left joystick kicks us to the next state */
	if (state == 0 && (machine.input().code_pressed_once(KEYCODE_O) || ui_input_pressed(machine, IPT_UI_LEFT)))
		state++;

	/* a 'K' or right joystick exits the state */
	else if (state == 1 && (machine.input().code_pressed_once(KEYCODE_K) || ui_input_pressed(machine, IPT_UI_RIGHT)))
		state = UI_HANDLER_CANCEL;

	/* if the user cancels, exit out completely */
	else if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine.schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/*-------------------------------------------------
    handler_messagebox_anykey - displays the
    current messagebox_text string and waits for
    any keypress
-------------------------------------------------*/

static UINT32 handler_messagebox_anykey(running_machine &machine, render_container *container, UINT32 state)
{
	int res = ui_window_scroll_keys(machine);

	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* if the user cancels, exit out completely */
	if (res == 2)
	{
		machine.schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	/* if select key is pressed, just exit */
	if (res == 1)
	{
		/* if any key is pressed, just exit */
		if (machine.input().poll_switches() != INPUT_CODE_INVALID)
		state = UI_HANDLER_CANCEL;
	}

	return state;
}

/*-------------------------------------------------
    process_natural_keyboard - processes any
    natural keyboard input
-------------------------------------------------*/

static void process_natural_keyboard(running_machine &machine)
{
	ui_event event;
	int i, pressed;
	input_item_id itemid;
	input_code code;
	UINT8 *key_down_ptr;
	UINT8 key_down_mask;

	/* loop while we have interesting events */
	while (ui_input_pop_event(machine, &event))
	{
		/* if this was a UI_EVENT_CHAR event, post it */
		if (event.event_type == UI_EVENT_CHAR)
			inputx_postc(machine, event.ch);
	}

	/* process natural keyboard keys that don't get UI_EVENT_CHARs */
	for (i = 0; i < ARRAY_LENGTH(non_char_keys); i++)
	{
		/* identify this keycode */
		itemid = non_char_keys[i];
		code = machine.input().code_from_itemid(itemid);

		/* ...and determine if it is pressed */
		pressed = machine.input().code_pressed(code);

		/* figure out whey we are in the key_down map */
		key_down_ptr = &non_char_keys_down[i / 8];
		key_down_mask = 1 << (i % 8);

		if (pressed && !(*key_down_ptr & key_down_mask))
		{
			/* this key is now down */
			*key_down_ptr |= key_down_mask;

			/* post the key */
			inputx_postc(machine, UCHAR_MAMEKEY_BEGIN + code.item_id());
		}
		else if (!pressed && (*key_down_ptr & key_down_mask))
		{
			/* this key is now up */
			*key_down_ptr &= ~key_down_mask;
		}
	}
}

/*-------------------------------------------------
    ui_paste - does a paste from the keyboard
-------------------------------------------------*/

void ui_paste(running_machine &machine)
{
	/* retrieve the clipboard text */
	char *text = osd_get_clipboard_text();

	/* was a result returned? */
	if (text != NULL)
	{
		/* post the text */
		inputx_post_utf8(machine, text);

		/* free the string */
		osd_free(text);
	}
}

/*-------------------------------------------------
    ui_image_handler_ingame - execute display
    callback function for each image device
-------------------------------------------------*/

void ui_image_handler_ingame(running_machine &machine)
{
	/* run display routine for devices */
	if (machine.phase() == MACHINE_PHASE_RUNNING)
	{
		image_interface_iterator iter(machine.root_device());
		for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
			image->call_display();
	}
}


#ifdef USE_SHOW_TIME

#define DISPLAY_AMPM 0

static void ui_display_time(running_machine &machine, render_container *container)
{
	char buf[20];
#if DISPLAY_AMPM
	char am_pm[] = "am";
#endif /* DISPLAY_AMPM */
	float width;
	time_t ltime;
	struct tm *today;
	float line_height = ui_get_line_height(machine);

	time(&ltime);
	today = localtime(&ltime);

#if DISPLAY_AMPM
	if( today->tm_hour > 12 )
	{
		strcpy( am_pm, "pm" );
		today->tm_hour -= 12;
	}
	if( today->tm_hour == 0 ) /* Adjust if midnight hour. */
		today->tm_hour = 12;
#endif /* DISPLAY_AMPM */

#if DISPLAY_AMPM
	sprintf(buf, "%02d:%02d:%02d %s", today->tm_hour, today->tm_min, today->tm_sec, am_pm);
#else
	sprintf(buf, "%02d:%02d:%02d", today->tm_hour, today->tm_min, today->tm_sec);
#endif /* DISPLAY_AMPM */
	width = ui_get_string_width(machine, buf) + UI_LINE_WIDTH * 2.0f;
	switch(Show_Time_Position)
	{
		case 0:
			ui_draw_text_box_fixed_width(container, buf, JUSTIFY_LEFT, 1.0f - width, 1.0f - line_height, UI_BACKGROUND_COLOR);
			break;

		case 1:
			ui_draw_text_box_fixed_width(container, buf, JUSTIFY_LEFT, 1.0f - width, 0.0f, UI_BACKGROUND_COLOR);
			break;

		case 2:
			ui_draw_text_box_fixed_width(container, buf, JUSTIFY_LEFT, 0.0f + width, 0.0f, UI_BACKGROUND_COLOR);
			break;

		case 3:
			ui_draw_text_box_fixed_width(container, buf, JUSTIFY_LEFT, 0.0f + width, 1.0f - line_height, UI_BACKGROUND_COLOR);
			break;
	}
}
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
/*-------------------------------------------------
    ui_display_input_log -
    show popup message if input exist any log
-------------------------------------------------*/

static void ui_display_input_log(running_machine &machine, render_container *container)
{
	double time_now = machine.time().as_double();
	double time_display = attotime::from_msec(1000).as_double();
	double time_fadeout = attotime::from_msec(1000).as_double();
	float curx;
	int i;

	if (!command_buffer[0].code)
		return;

	// adjust time for load state
	{
		double max = 0.0f;
		int i;

		for (i = 0; command_buffer[i].code; i++)
			if (max < command_buffer[i].time)
				max = command_buffer[i].time;

		if (max > time_now)
		{
			double adjust = max - time_now;

			for (i = 0; command_buffer[i].code; i++)
				command_buffer[i].time -= adjust;
		}
	}

	// find position to start display
	curx = 1.0f - UI_LINE_WIDTH;
	for (i = 0; command_buffer[i].code; i++)
		curx -= ui_get_char_width(machine, command_buffer[i].code);

	for (i = 0; command_buffer[i].code; i++)
	{
		if (curx >= UI_LINE_WIDTH)
			break;

		curx += ui_get_char_width(machine, command_buffer[i].code);
	}

	ui_draw_box(container, 0.0f, 1.0f - ui_get_line_height(machine), 1.0f, 1.0f, UI_BACKGROUND_COLOR);

	for (; command_buffer[i].code; i++)
	{
		double rate = time_now - command_buffer[i].time;

		if (rate < time_display + time_fadeout)
		{
			int level = 255 - ((rate - time_display) / time_fadeout) * 255;
			rgb_t fgcolor;

			if (level > 255)
				level = 255;

			fgcolor = MAKE_ARGB(255, level, level, level);

			container->add_char(curx, 1.0f - ui_get_line_height(machine), ui_get_line_height(machine), machine.render().ui_aspect(), fgcolor, *ui_get_font(machine), command_buffer[i].code);
		}
		curx += ui_get_char_width(machine, command_buffer[i].code);
	}
}
#endif /* USE_SHOW_INPUT_LOG */

/*-------------------------------------------------
    handler_ingame - in-game handler takes care
    of the standard keypresses
-------------------------------------------------*/

static UINT32 handler_ingame(running_machine &machine, render_container *container, UINT32 state)
{
	bool is_paused = machine.paused();

	/* first draw the FPS counter */
	if (showfps || osd_ticks() < showfps_end)
	{
		astring tempstring;
		ui_draw_text_full_fixed_width(container, machine.video().speed_text(tempstring), 0.0f, 0.0f, 1.0f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}
	else
		showfps_end = 0;

	/* draw the profiler if visible */
	if (show_profiler)
	{
		astring profilertext;
		g_profiler.text(machine, profilertext);
		ui_draw_text_full(container, profilertext, 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);
	}

	/* if we're single-stepping, pause now */
	if (single_step)
	{
		machine.pause();
		single_step = FALSE;
	}

	/* determine if we should disable the rest of the UI */
	int ui_disabled = (input_machine_has_keyboard(machine) && !machine.ui_active());

	/* is ScrLk UI toggling applicable here? */
	if (input_machine_has_keyboard(machine))
	{
		/* are we toggling the UI with ScrLk? */
		if (ui_input_pressed(machine, IPT_UI_TOGGLE_UI))
		{
			/* toggle the UI */
			machine.set_ui_active(!machine.ui_active());

			/* display a popup indicating the new status */
			if (machine.ui_active())
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: PARTIAL Emulation",
					"UI:   Enabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
			else
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: FULL Emulation",
					"UI:   Disabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
		}
	}

	/* is the natural keyboard enabled? */
	if (ui_get_use_natural_keyboard(machine) && (machine.phase() == MACHINE_PHASE_RUNNING))
		process_natural_keyboard(machine);

	if (!ui_disabled)
	{
		/* paste command */
		if (ui_input_pressed(machine, IPT_UI_PASTE))
			ui_paste(machine);
	}

	ui_image_handler_ingame(machine);

	if (ui_disabled) return ui_disabled;

	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		if (!machine.options().confirm_quit())
			machine.schedule_exit();
		else
			return ui_set_handler(handler_confirm_quit, 0);
	}

	/* turn on menus if requested */
	if (ui_input_pressed(machine, IPT_UI_CONFIGURE))
		return ui_set_handler(ui_menu::ui_handler, 0);

	/* if the on-screen display isn't up and the user has toggled it, turn it on */
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) == 0 && ui_input_pressed(machine, IPT_UI_ON_SCREEN_DISPLAY))
		return ui_set_handler(ui_menu_sliders::ui_handler, 1);

	/* handle a reset request */
	if (ui_input_pressed(machine, IPT_UI_RESET_MACHINE))
		machine.schedule_hard_reset();
	if (ui_input_pressed(machine, IPT_UI_SOFT_RESET))
		machine.schedule_soft_reset();

	/* handle a request to display graphics/palette */
	if (ui_input_pressed(machine, IPT_UI_SHOW_GFX))
	{
		if (!is_paused)
			machine.pause();
		return ui_set_handler(ui_gfx_ui_handler, is_paused);
	}

	/* handle a save state request */
	if (ui_input_pressed(machine, IPT_UI_SAVE_STATE))
	{
		machine.pause();
		return ui_set_handler(handler_load_save, LOADSAVE_SAVE);
	}

	/* handle a load state request */
	if (ui_input_pressed(machine, IPT_UI_LOAD_STATE))
	{
		machine.pause();
		return ui_set_handler(handler_load_save, LOADSAVE_LOAD);
	}

	/* handle a save snapshot request */
	if (ui_input_pressed(machine, IPT_UI_SNAPSHOT))
		machine.video().save_active_screen_snapshots();

#ifdef INP_CAPTION
	draw_caption(machine, container);
#endif /* INP_CAPTION */

	/* toggle pause */
	if (ui_input_pressed(machine, IPT_UI_PAUSE))
	{
		/* with a shift key, it is single step */
		if (is_paused && (machine.input().code_pressed(KEYCODE_LSHIFT) || machine.input().code_pressed(KEYCODE_RSHIFT)))
		{
			single_step = TRUE;
			machine.resume();
		}
		else if (machine.paused())
			machine.resume();
		else
			machine.pause();
	}

#ifdef USE_SHOW_TIME
	if (ui_input_pressed(machine, IPT_UI_TIME))
	{
		if (show_time)
		{
			Show_Time_Position++;

			if (Show_Time_Position > 3)
			{
				Show_Time_Position = 0;
				show_time = 0;
			}
		}
		else
		{
			Show_Time_Position = 0;
			show_time = 1;
		}
	}

	if (show_time)
		ui_display_time(machine, container);
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
	if (ui_input_pressed(machine, IPT_UI_SHOW_INPUT_LOG))
	{
		show_input_log ^= 1;
		command_buffer[0].code = '\0';
	}

	/* show popup message if input exist any log */
	if (show_input_log)
		ui_display_input_log(machine, container);
#endif /* USE_SHOW_INPUT_LOG */

	/* handle a toggle cheats request */
	if (ui_input_pressed(machine, IPT_UI_TOGGLE_CHEAT))
		machine.cheat().set_enable(!machine.cheat().enabled());

	/* toggle movie recording */
	if (ui_input_pressed(machine, IPT_UI_RECORD_MOVIE))
	{
		if (!machine.video().is_recording())
		{
			machine.video().begin_recording(NULL, video_manager::MF_MNG);
			popmessage(_("REC START"));
		}
		else
		{
			machine.video().end_recording();
			popmessage(_("REC STOP"));
		}
	}

	/* toggle profiler display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_PROFILER))
		ui_set_show_profiler(!ui_get_show_profiler());

	/* toggle FPS display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_FPS))
		ui_set_show_fps(!ui_get_show_fps());

	/* increment frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_INC))
	{
		/* get the current value and increment it */
		int newframeskip = machine.video().frameskip() + 1;
		if (newframeskip > MAX_FRAMESKIP)
			newframeskip = -1;
		machine.video().set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* decrement frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_DEC))
	{
		/* get the current value and decrement it */
		int newframeskip = machine.video().frameskip() - 1;
		if (newframeskip < -1)
			newframeskip = MAX_FRAMESKIP;
		machine.video().set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* toggle throttle? */
	if (ui_input_pressed(machine, IPT_UI_THROTTLE))
		machine.video().set_throttled(!machine.video().throttled());

	/* check for fast forward */
	if (input_type_pressed(machine, IPT_UI_FAST_FORWARD, 0))
	{
		machine.video().set_fastforward(true);
		ui_show_fps_temp(0.5);
	}
	else
		machine.video().set_fastforward(false);

	return 0;
}


/*-------------------------------------------------
    handler_load_save - leads the user through
    specifying a game to save or load
-------------------------------------------------*/

static UINT32 handler_load_save(running_machine &machine, render_container *container, UINT32 state)
{
	char filename[20];
	input_code code;
	char file = 0;

	/* if we're not in the middle of anything, skip */
	if (state == LOADSAVE_NONE)
		return 0;

	/* okay, we're waiting for a key to select a slot; display a message */
	if (state == LOADSAVE_SAVE)
		ui_draw_message_window(container, _("Select position to save to"));
	else
		ui_draw_message_window(container, _("Select position to load from"));

	/* check for cancel key */
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		/* display a popup indicating things were cancelled */
		if (state == LOADSAVE_SAVE)
			popmessage(_("Save cancelled"));
		else
			popmessage(_("Load cancelled"));

		/* reset the state */
		machine.resume();
		return UI_HANDLER_CANCEL;
	}

	/* check for A-Z or 0-9 */
	for (input_item_id id = ITEM_ID_A; id <= ITEM_ID_Z; id++)
		if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
			file = id - ITEM_ID_A + 'a';
	if (file == 0)
		for (input_item_id id = ITEM_ID_0; id <= ITEM_ID_9; id++)
			if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
				file = id - ITEM_ID_0 + '0';
	if (file == 0)
		for (input_item_id id = ITEM_ID_0_PAD; id <= ITEM_ID_9_PAD; id++)
			if (machine.input().code_pressed_once(input_code(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, id)))
				file = id - ITEM_ID_0_PAD + '0';
	if (file == 0)
		return state;

	/* display a popup indicating that the save will proceed */
	sprintf(filename, "%c", file);
	if (state == LOADSAVE_SAVE)
	{
		popmessage(_("Save to position %c"), file);
		machine.schedule_save(filename);
	}
	else
	{
		popmessage(_("Load from position %c"), file);
		machine.schedule_load(filename);
	}

	/* remove the pause and reset the state */
	machine.resume();
	return UI_HANDLER_CANCEL;
}


/*-------------------------------------------------
 handler_confirm_quit - leads the user through
 confirming quit emulation
 -------------------------------------------------*/

static UINT32 handler_confirm_quit(running_machine &machine, render_container *container, UINT32 state)
{
	astring quit_message(_("Are you sure you want to quit?\n\n"));
	quit_message.cat(_("Press ''UI Select'' (default: Enter) to quit,\n"));
	quit_message.cat(_("Press ''UI Cancel'' (default: Esc) to return to emulation."));

	ui_draw_text_box(container, quit_message, JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);
	machine.pause();

	/* if the user press ENTER, quit the game */
	if (ui_input_pressed(machine, IPT_UI_SELECT))
		machine.schedule_exit();

	/* if the user press ESC, just continue */
	else if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine.resume();
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/***************************************************************************
    SLIDER CONTROLS
***************************************************************************/

/*-------------------------------------------------
    ui_get_slider_list - get the list of sliders
-------------------------------------------------*/

const slider_state *ui_get_slider_list(void)
{
	return slider_list;
}


/*-------------------------------------------------
    slider_alloc - allocate a new slider entry
-------------------------------------------------*/

static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = (slider_state *)auto_alloc_array_clear(machine, UINT8, size);

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	strcpy(state->description, title);

	return state;
}


/*-------------------------------------------------
    slider_init - initialize the list of slider
    controls
-------------------------------------------------*/

static slider_state *slider_init(running_machine &machine)
{
	input_field_config *field;
	input_port_config *port;
	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;
	astring string;
	int item;

	/* add overall volume */
	*tailptr = slider_alloc(machine, _("Master Volume"), -32, 0, 0, 1, slider_volume, NULL);
	tailptr = &(*tailptr)->next;

	/* add per-channel volume */
	mixer_input info;
	for (item = 0; machine.sound().indexed_mixer_input(item, info); item++)
	{
		INT32 maxval = 2000;
		INT32 defval = info.stream->initial_input_gain(info.inputnum) * 1000.0f + 0.5f;

		if (defval > 1000)
			maxval = 2 * defval;

		info.stream->input_name(info.inputnum, string);
		string.cat(_(" Volume"));
		*tailptr = slider_alloc(machine, string, 0, defval, maxval, 20, slider_mixervol, (void *)(FPTR)item);
		tailptr = &(*tailptr)->next;
	}

	/* add analog adjusters */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist().first(); field != NULL; field = field->next())
			if (field->type == IPT_ADJUSTER)
			{
				void *param = (void *)field;
				*tailptr = slider_alloc(machine, field->name, 0, field->defvalue, 100, 1, slider_adjuster, param);
				tailptr = &(*tailptr)->next;
			}

	/* add CPU overclocking (cheat only) */
	if (machine.options().cheat())
	{
		execute_interface_iterator iter(machine.root_device());
		for (device_execute_interface *exec = iter.first(); exec != NULL; exec = iter.next())
		{
			void *param = (void *)&exec->device();
			string.printf(_("Overclock CPU %s"), exec->device().tag());
			//mamep: 4x overclock
			*tailptr = slider_alloc(machine, string, 10, 1000, 4000, 50, slider_overclock, param);
			tailptr = &(*tailptr)->next;
		}
	}

	/* add screen parameters */
	screen_device_iterator scriter(machine.root_device());
	for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
	{
		int defxscale = floor(screen->xscale() * 1000.0f + 0.5f);
		int defyscale = floor(screen->yscale() * 1000.0f + 0.5f);
		int defxoffset = floor(screen->xoffset() * 1000.0f + 0.5f);
		int defyoffset = floor(screen->yoffset() * 1000.0f + 0.5f);
		void *param = (void *)screen;

		/* add refresh rate tweaker */
		if (machine.options().cheat())
		{
			string.printf(_("%s Refresh Rate"), slider_get_screen_desc(*screen));
			*tailptr = slider_alloc(machine, string, -33000, 0, 33000, 1000, slider_refresh, param);
			tailptr = &(*tailptr)->next;
		}

		/* add standard brightness/contrast/gamma controls per-screen */
		string.printf(_("%s Brightness"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 10, slider_brightness, param);
		tailptr = &(*tailptr)->next;
		string.printf(_("%s Contrast"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 50, slider_contrast, param);
		tailptr = &(*tailptr)->next;
		string.printf(_("%s Gamma"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 3000, 50, slider_gamma, param);
		tailptr = &(*tailptr)->next;

		/* add scale and offset controls per-screen */
		string.printf(_("%s Horiz Stretch"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defxscale, 1500, 2, slider_xscale, param);
		tailptr = &(*tailptr)->next;
		string.printf(_("%s Horiz Position"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_xoffset, param);
		tailptr = &(*tailptr)->next;
		string.printf(_("%s Vert Stretch"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defyscale, 1500, 2, slider_yscale, param);
		tailptr = &(*tailptr)->next;
		string.printf(_("%s Vert Position"), slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_yoffset, param);
		tailptr = &(*tailptr)->next;
	}

	laserdisc_device_iterator lditer(machine.root_device());
	for (laserdisc_device *laserdisc = lditer.first(); laserdisc != NULL; laserdisc = lditer.next())
		if (laserdisc->overlay_configured())
		{
			laserdisc_overlay_config config;
			laserdisc->get_overlay_config(config);
			int defxscale = floor(config.m_overscalex * 1000.0f + 0.5f);
			int defyscale = floor(config.m_overscaley * 1000.0f + 0.5f);
			int defxoffset = floor(config.m_overposx * 1000.0f + 0.5f);
			int defyoffset = floor(config.m_overposy * 1000.0f + 0.5f);
			void *param = (void *)laserdisc;

			/* add scale and offset controls per-overlay */
			string.printf(_("Laserdisc '%s' Horiz Stretch"), laserdisc->tag());
			*tailptr = slider_alloc(machine, string, 500, (defxscale == 0) ? 1000 : defxscale, 1500, 2, slider_overxscale, param);
			tailptr = &(*tailptr)->next;
			string.printf(_("Laserdisc '%s' Horiz Position"), laserdisc->tag());
			*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_overxoffset, param);
			tailptr = &(*tailptr)->next;
			string.printf(_("Laserdisc '%s' Vert Stretch"), laserdisc->tag());
			*tailptr = slider_alloc(machine, string, 500, (defyscale == 0) ? 1000 : defyscale, 1500, 2, slider_overyscale, param);
			tailptr = &(*tailptr)->next;
			string.printf(_("Laserdisc '%s' Vert Position"), laserdisc->tag());
			*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_overyoffset, param);
			tailptr = &(*tailptr)->next;
		}

	for (screen_device *screen = scriter.first(); screen != NULL; screen = scriter.next())
		if (screen->screen_type() == SCREEN_TYPE_VECTOR)
		{
			/* add flicker control */
			*tailptr = slider_alloc(machine, _("Vector Flicker"), 0, 0, 1000, 10, slider_flicker, NULL);
			tailptr = &(*tailptr)->next;
			*tailptr = slider_alloc(machine, _("Beam Width"), 10, 100, 1000, 10, slider_beam, NULL);
			tailptr = &(*tailptr)->next;
			break;
		}

#ifdef MAME_DEBUG
	/* add crosshair adjusters */
	for (port = machine.m_portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist().first(); field != NULL; field = field->next())
			if (field->crossaxis != CROSSHAIR_AXIS_NONE && field->player == 0)
			{
				void *param = (void *)field;
				string.printf(_("Crosshair Scale %s"), (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 1000, 3000, 100, slider_crossscale, param);
				tailptr = &(*tailptr)->next;
				string.printf(_("Crosshair Offset %s"), (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 0, 3000, 100, slider_crossoffset, param);
				tailptr = &(*tailptr)->next;
			}
#endif

	return listhead;
}


/*-------------------------------------------------
    slider_volume - global volume slider callback
-------------------------------------------------*/

static INT32 slider_volume(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		machine.sound().set_attenuation(newval);
	if (string != NULL)
		string->printf("%3ddB", machine.sound().attenuation());
	return machine.sound().attenuation();
}


/*-------------------------------------------------
    slider_mixervol - single channel volume
    slider callback
-------------------------------------------------*/

static INT32 slider_mixervol(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	mixer_input info;
	if (!machine.sound().indexed_mixer_input((FPTR)arg, info))
		return 0;
	if (newval != SLIDER_NOCHANGE)
	{
		INT32 curval = floor(info.stream->input_gain(info.inputnum) * 1000.0f + 0.5f);
		if (newval > curval && (newval - curval) <= 4) newval += 4; // round up on increment
		info.stream->set_input_gain(info.inputnum, (float)newval * 0.001f);
	}
	if (string != NULL)
		string->printf("%4.2f", info.stream->input_gain(info.inputnum));
	return floor(info.stream->input_gain(info.inputnum) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_adjuster - analog adjuster slider
    callback
-------------------------------------------------*/

static INT32 slider_adjuster(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	const input_field_config *field = (const input_field_config *)arg;
	input_field_user_settings settings;

	input_field_get_user_settings(field, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.value = newval;
		input_field_set_user_settings(field, &settings);
	}
	if (string != NULL)
		string->printf("%d%%", settings.value);
	return settings.value;
}


/*-------------------------------------------------
    slider_overclock - CPU overclocker slider
    callback
-------------------------------------------------*/

static INT32 slider_overclock(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	device_t *cpu = (device_t *)arg;
	if (newval != SLIDER_NOCHANGE)
		cpu->set_clock_scale((float)newval * 0.001f);
	if (string != NULL)
		string->printf("%3.0f%%", floor(cpu->clock_scale() * 100.0f + 0.5f));
	return floor(cpu->clock_scale() * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_refresh - refresh rate slider callback
-------------------------------------------------*/

static INT32 slider_refresh(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	double defrefresh = ATTOSECONDS_TO_HZ(screen->refresh_attoseconds());
	double refresh;

	if (newval != SLIDER_NOCHANGE)
	{
		int width = screen->width();
		int height = screen->height();
		const rectangle &visarea = screen->visible_area();
		screen->configure(width, height, visarea, HZ_TO_ATTOSECONDS(defrefresh + (double)newval * 0.001));
	}
	if (string != NULL)
		string->printf("%.3ffps", ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds));
	refresh = ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds);
	return floor((refresh - defrefresh) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_brightness - screen brightness slider
    callback
-------------------------------------------------*/

static INT32 slider_brightness(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_brightness = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_brightness);
	return floor(settings.m_brightness * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_contrast - screen contrast slider
    callback
-------------------------------------------------*/

static INT32 slider_contrast(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_contrast = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_contrast);
	return floor(settings.m_contrast * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_gamma - screen gamma slider callback
-------------------------------------------------*/

static INT32 slider_gamma(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_gamma = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_gamma);
	return floor(settings.m_gamma * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_xscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xscale = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_xscale);
	return floor(settings.m_xscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_yscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yscale = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_yscale);
	return floor(settings.m_yscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_xoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_xoffset = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_xoffset);
	return floor(settings.m_xoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_yoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container::user_settings settings;

	screen->container().get_user_settings(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_yoffset = (float)newval * 0.001f;
		screen->container().set_user_settings(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_yoffset);
	return floor(settings.m_yoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overxscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscalex = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overscalex);
	return floor(settings.m_overscalex * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overyscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overscaley = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overscaley);
	return floor(settings.m_overscaley * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_overxoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposx = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overposx);
	return floor(settings.m_overposx * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_overyoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	laserdisc_device *laserdisc = (laserdisc_device *)arg;
	laserdisc_overlay_config settings;

	laserdisc->get_overlay_config(settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.m_overposy = (float)newval * 0.001f;
		laserdisc->set_overlay_config(settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.m_overposy);
	return floor(settings.m_overposy * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_flicker - vector flicker slider
    callback
-------------------------------------------------*/

static INT32 slider_flicker(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_flicker((float)newval * 0.1f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_flicker());
	return floor(vector_get_flicker() * 10.0f + 0.5f);
}


/*-------------------------------------------------
    slider_beam - vector beam width slider
    callback
-------------------------------------------------*/

static INT32 slider_beam(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_beam((float)newval * 0.01f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_beam());
	return floor(vector_get_beam() * 100.0f + 0.5f);
}


/*-------------------------------------------------
    slider_get_screen_desc - returns the
    description for a given screen
-------------------------------------------------*/

static char *slider_get_screen_desc(screen_device &screen)
{
	screen_device_iterator iter(screen.machine().root_device());
	int scrcount = iter.count();
	static char descbuf[256];

	if (scrcount > 1)
		sprintf(descbuf, _("Screen '%s'"), screen.tag());
	else
		strcpy(descbuf, _("Screen"));

	return descbuf;
}

/*-------------------------------------------------
    slider_crossscale - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	input_field_config *field = (input_field_config *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->crossscale = (float)newval * 0.001f;
	if (string != NULL)
		string->printf("%s %s %1.3f", _("Crosshair Scale"), (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	return floor(field->crossscale * 1000.0f + 0.5f);
}
#endif


/*-------------------------------------------------
    slider_crossoffset - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossoffset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	input_field_config *field = (input_field_config *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->crossoffset = (float)newval * 0.001f;
	if (string != NULL)
		string->printf("%s %s %1.3f", _("Crosshair Offset"), (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	return field->crossoffset;
}
#endif


/*-------------------------------------------------
    ui_get_use_natural_keyboard - returns
    whether the natural keyboard is active
-------------------------------------------------*/

int ui_get_use_natural_keyboard(running_machine &machine)
{
	return ui_use_natural_keyboard;
}



/*-------------------------------------------------
    ui_set_use_natural_keyboard - specifies
    whether the natural keyboard is active
-------------------------------------------------*/

void ui_set_use_natural_keyboard(running_machine &machine, int use_natural_keyboard)
{
	ui_use_natural_keyboard = use_natural_keyboard;
	astring error;
	machine.options().set_value(OPTION_NATURAL_KEYBOARD, use_natural_keyboard, OPTION_PRIORITY_CMDLINE, error);
	assert(!error);
}



static void build_bgtexture(running_machine &machine)
{
#ifdef UI_COLOR_DISPLAY
	float r = (float)RGB_RED(uifont_colortable[UI_BACKGROUND_COLOR]);
	float g = (float)RGB_GREEN(uifont_colortable[UI_BACKGROUND_COLOR]);
	float b = (float)RGB_BLUE(uifont_colortable[UI_BACKGROUND_COLOR]);
#else /* UI_COLOR_DISPLAY */
	UINT8 r = 0x10;
	UINT8 g = 0x10;
	UINT8 b = 0x30;
#endif /* UI_COLOR_DISPLAY */
	UINT8 a = 0xff;
	int i;

#ifdef TRANS_UI
	a = ui_transparency;
#endif /* TRANS_UI */

	bgbitmap = auto_alloc(machine, bitmap_argb32(1, 1024));
	if (bgbitmap == NULL)
		fatalerror("build_bgtexture failed");

	for (i = 0; i < bgbitmap->height(); i++)
	{
		double gradual = (float)(1024 - i) / 1024.0f + 0.1f;

		if (gradual > 1.0f)
			gradual = 1.0f;
		else if (gradual < 0.1f)
			gradual = 0.1f;

		bgbitmap->pix32(i, 0) = MAKE_ARGB(a, (UINT8)(r * gradual), (UINT8)(g * gradual), (UINT8)(b * gradual));
	}

	bgtexture = machine.render().texture_alloc(render_texture::hq_scale);
	bgtexture->set_bitmap(*bgbitmap, bgbitmap->cliprect(), TEXFORMAT_ARGB32);
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(free_bgtexture), &machine));
}


static void free_bgtexture(running_machine &machine)
{
	machine.render().texture_free(bgtexture);
	bgtexture = NULL;
}
