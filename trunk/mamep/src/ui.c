/*********************************************************************

    ui.c

    Functions used to handle MAME's user interface.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "osdepend.h"
#include "vidhrdw/vector.h"
#include "profiler.h"
#include "cheat.h"
#include "datafile.h"
#ifdef USE_SHOW_TIME
#include <time.h>
#endif /* USE_SHOW_TIME */

#include "render.h"
#include "rendfont.h"
#include "ui.h"
#include "uimenu.h"
#include "uigfx.h"
#include "uitext.h"

#ifdef MESS
#include "mess.h"
#include "uimess.h"
#include "inputx.h"
#endif

#include <ctype.h>
#include <stdarg.h>
#include <math.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	LOADSAVE_NONE,
	LOADSAVE_LOAD,
	LOADSAVE_SAVE
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _slider_state slider_state;
struct _slider_state
{
	INT32			minval;				/* minimum value */
	INT32			defval;				/* default value */
	INT32			maxval;				/* maximum value */
	INT32			incval;				/* increment value */
	INT32			(*update)(INT32 newval, char *buffer, int arg); /* callback */
	int				arg;				/* argument */
};



/***************************************************************************
    MACROS
***************************************************************************/

#define UI_FONT_NAME				NULL
#define UI_SCALE_TO_INT_X(x)		((int)((float)(x) * ui_screen_width + 0.5f))
#define UI_SCALE_TO_INT_Y(y)		((int)((float)(y) * ui_screen_height + 0.5f))
#define UI_UNSCALE_TO_FLOAT_X(x)	((float)(x) / (float)ui_screen_width)
#define UI_UNSCALE_TO_FLOAT_Y(y)	((float)(y) / (float)ui_screen_height)



/***************************************************************************
    EXTERNAL VARIABLES
***************************************************************************/

#ifdef USE_SHOW_INPUT_LOG
extern int show_input_log;
extern UINT8 command_buffer[COMMAND_LOG_BUFSIZE];
extern int command_counter;
#endif /* USE_SHOW_INPUT_LOG */



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

#ifdef INP_CAPTION
static int next_caption_frame, caption_timer;
#endif /* INP_CAPTION */

static rgb_t uifont_colortable[MAX_COLORTABLE];
static render_texture *bgtexture;
static mame_bitmap *bgbitmap;

static rgb_t ui_bgcolor;

static render_font *ui_font;

float ui_font_height;
int ui_screen_width, ui_screen_height;

static int multiline_text_box_visible_lines;
static int multiline_text_box_target_lines;

static int message_window_scroll;

static int auto_pause;
static int scroll_reset;

/* current UI handler */
static UINT32 (*ui_handler_callback)(UINT32);
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
static char messagebox_text[4096];
static rgb_t messagebox_backcolor;

/* slider info */
static slider_state slider_list[100];
static int slider_count;
static int slider_current;

static int display_rescale_message;
static int allow_rescale;

#ifdef USE_SHOW_TIME
static int show_time = 0;
static int Show_Time_Position;
static void display_time(void);
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
static void display_input_log(void);
#endif /* USE_SHOW_INPUT_LOG */

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_exit(running_machine *machine);
static int rescale_notifier(running_machine *machine, int width, int height);

/* text generators */
static int sprintf_font_warning(char *buffer);
static int sprintf_disclaimer(char *buffer);
static int sprintf_warnings(char *buffer);

/* UI handlers */
static UINT32 handler_messagebox(UINT32 state);
static UINT32 handler_messagebox_ok(UINT32 state);
static UINT32 handler_messagebox_selectkey(UINT32 state);
static UINT32 handler_ingame(UINT32 state);
static UINT32 handler_slider(UINT32 state);
static UINT32 handler_load_save(UINT32 state);
static UINT32 handler_confirm_quit(UINT32 state);

/* slider controls */
static void slider_init(void);
static void slider_display(const char *text, int minval, int maxval, int defval, int curval);
static void slider_draw_bar(int leftx, int topy, int width, int height, float percentage, float default_percentage);
static INT32 slider_volume(INT32 newval, char *buffer, int arg);
static INT32 slider_mixervol(INT32 newval, char *buffer, int arg);
static INT32 slider_adjuster(INT32 newval, char *buffer, int arg);
static INT32 slider_overclock(INT32 newval, char *buffer, int arg);
static INT32 slider_refresh(INT32 newval, char *buffer, int arg);
static INT32 slider_brightness(INT32 newval, char *buffer, int arg);
static INT32 slider_contrast(INT32 newval, char *buffer, int arg);
static INT32 slider_gamma(INT32 newval, char *buffer, int arg);
static INT32 slider_xscale(INT32 newval, char *buffer, int arg);
static INT32 slider_yscale(INT32 newval, char *buffer, int arg);
static INT32 slider_xoffset(INT32 newval, char *buffer, int arg);
static INT32 slider_yoffset(INT32 newval, char *buffer, int arg);
static INT32 slider_flicker(INT32 newval, char *buffer, int arg);
static INT32 slider_beam(INT32 newval, char *buffer, int arg);
#ifdef MAME_DEBUG
static INT32 slider_crossscale(INT32 newval, char *buffer, int arg);
static INT32 slider_crossoffset(INT32 newval, char *buffer, int arg);
#endif

static void build_bgtexture(running_machine *machine);
static void free_bgtexture(running_machine *machine);

#define add_line(x0,y0,x1,y1,color)	render_ui_add_line(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), UI_LINE_WIDTH, ui_get_rgb_color(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA))
#define add_char(x,y,ch,color)		render_ui_add_char(UI_UNSCALE_TO_FLOAT_X(x), UI_UNSCALE_TO_FLOAT_Y(y), UI_TARGET_FONT_HEIGHT, render_get_ui_aspect(), ui_get_rgb_color(color), ui_font, ch)

static void add_filled_box_color(int x1, int y1, int x2, int y2, rgb_t color);

#ifdef USE_SHOW_INPUT_LOG
static void add_filled_box_noedge(int x0, int y0, int x1, int y1);
#endif /* USE_SHOW_INPUT_LOG */



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ui_set_handler - set a callback/parameter
    pair for the current UI handler
-------------------------------------------------*/

INLINE UINT32 ui_set_handler(UINT32 (*callback)(UINT32), UINT32 param)
{
	ui_handler_callback = callback;
	ui_handler_param = param;
	return param;
}


/*-------------------------------------------------
    slider_config - configure a slider entry
-------------------------------------------------*/

INLINE void slider_config(slider_state *state, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, INT32 (*update)(INT32, char *, int), int arg)
{
	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
}


INLINE rgb_t ui_get_rgb_color(rgb_t color)
{
	if (color < MAX_COLORTABLE)
		return uifont_colortable[color];

	return color;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ui_init - set up the user interface
-------------------------------------------------*/

int ui_init(running_machine *machine)
{
	/* make sure we clean up after ourselves */
	add_exit_callback(machine, ui_exit);

	/* load the localization file */
	if (uistring_init() != 0)
		fatalerror("uistring_init failed");

	build_bgtexture(machine);
	ui_font = render_font_alloc("ui.bdf");
	if (uifont_need_font_warning())
	{
		options.langcode = UI_LANG_EN_US;
		set_langcode(options.langcode);
		fprintf(stderr, "error: loading local font file\nUse %s\n",
			ui_lang_info[options.langcode].description);

		/* re-load the localization file */
		if (uistring_init() != 0)
			fatalerror("uistring_init failed");
	}

	ui_set_visible_area(
		Machine->screen[0].visarea.min_x,
		Machine->screen[0].visarea.min_y,
		Machine->screen[0].visarea.max_x,
		Machine->screen[0].visarea.max_y);

	{
		int i;

		for (i = 0; i < MAX_COLORTABLE; i++)
			uifont_colortable[i] = MAKE_ARGB(
				0xff,
				options.uicolortable[i][0],
				options.uicolortable[i][1],
				options.uicolortable[i][2]);
	}

#ifdef TRANS_UI
	if (options.use_transui)
		uifont_colortable[UI_FILLCOLOR] = MAKE_ARGB(
				options.ui_transparency,
				options.uicolortable[UI_FILLCOLOR][0],
				options.uicolortable[UI_FILLCOLOR][1],
				options.uicolortable[UI_FILLCOLOR][2]);
#endif /* TRANS_UI */

#ifdef INP_CAPTION
	next_caption_frame = -1;
	caption_timer = 0;
#endif /* INP_CAPTION */

	ui_bgcolor = UI_FILLCOLOR;

	/* initialize the other UI bits */
	ui_menu_init(machine);
	ui_gfx_init(machine);

	/* reset globals */
	single_step = FALSE;
	ui_set_handler(handler_messagebox, 0);

	/* request callbacks when the renderer does resizing */
	render_set_rescale_notify(machine, rescale_notifier);
	allow_rescale = 0;
	display_rescale_message = FALSE;
	return 0;
}


/*-------------------------------------------------
    ui_exit - clean up ourselves on exit
-------------------------------------------------*/

static void ui_exit(running_machine *machine)
{
	/* free the font */
	if (ui_font != NULL)
		render_font_free(ui_font);
	ui_font = NULL;
}


/*-------------------------------------------------
    rescale_notifier - notifier to trigger a
    rescale message during long operations
-------------------------------------------------*/

static int rescale_notifier(running_machine *machine, int width, int height)
{
	/* always allow "small" rescaling */
	if (width < 500 || height < 500)
		return TRUE;

	/* if we've currently disallowed rescaling, turn on a message next frame */
	if (allow_rescale == 0)
		display_rescale_message = TRUE;

	/* only allow rescaling once we're sure the message is visible */
	return (allow_rescale == 1);
}


/*-------------------------------------------------
    ui_display_startup_screens - display the
    various startup screens
-------------------------------------------------*/

int ui_display_startup_screens(int show_disclaimer, int show_warnings, int show_gameinfo)
{
#ifdef MESS
	const int maxstate = 4;
#else
	const int maxstate = 3;
#endif
	int state;

	/* initialize the on-screen display system */
	slider_init();

	auto_pause = FALSE;
	scroll_reset = TRUE;
#ifdef USE_SHOW_TIME
	show_time = 0;
	Show_Time_Position = 0;
#endif /* USE_SHOW_TIME */

	/* loop over states */
	ui_set_handler(handler_ingame, 0);
	for (state = -1; state < maxstate && !mame_is_scheduled_event_pending(Machine); state++)
	{
		/* default to standard colors */
		messagebox_backcolor = UI_FILLCOLOR;

		/* pick the next state */
		switch (state)
		{
			case -1:
				if (uifont_need_font_warning() && sprintf_font_warning(messagebox_text))
					ui_set_handler(handler_messagebox_ok, 0);
				break;

			case 0:
				if (show_disclaimer && sprintf_disclaimer(messagebox_text))
					ui_set_handler(handler_messagebox_ok, 0);
				break;

			case 1:
				if (show_warnings && sprintf_warnings(messagebox_text))
				{
					ui_set_handler(handler_messagebox_ok, 0);
					if (Machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
						messagebox_backcolor = MAKE_ARGB(0xe0,0x60,0x10,0x10);
				}
				break;

			case 2:
				if (show_gameinfo && sprintf_game_info(messagebox_text))
				{
					char *bufptr = messagebox_text + strlen(messagebox_text);

					/* append MAME version and ask for select key */
					bufptr += sprintf(bufptr, "\n\t%s %s\n\t%s", ui_getstring(UI_mame), build_version, ui_getstring(UI_selectkey));

					ui_set_handler(handler_messagebox_selectkey, 0);
				}
				break;
#ifdef MESS
			case 3:
				break;
#endif
		}

		/* clear the input memory */
		while (code_read_async() != CODE_NONE) ;

		/* loop while we have a handler */
		while (ui_handler_callback != handler_ingame && !mame_is_scheduled_event_pending(Machine))
			video_frame_update();

		/* clear the handler and force an update */
		ui_set_handler(handler_ingame, 0);
		video_frame_update();

		scroll_reset = TRUE;
	}

	/* clear the input memory */
	while (code_read_async() != CODE_NONE) ;

	return 0;
}


/*-------------------------------------------------
    ui_set_startup_text - set the text to display
    at startup
-------------------------------------------------*/

void ui_set_startup_text(const char *text, int force)
{
	static osd_ticks_t lastupdatetime = 0;
	osd_ticks_t curtime = osd_ticks();

	/* copy in the new text */
	strncpy(messagebox_text, text, sizeof(messagebox_text));
	messagebox_backcolor = UI_FILLCOLOR;

	/* don't update more than 4 times/second */
	if (force || (curtime - lastupdatetime) > osd_ticks_per_second() / 4)
	{
		lastupdatetime = curtime;
		video_frame_update();
	}
}


/*-------------------------------------------------
    ui_update_and_render - update the UI and
    render it; called by video.c
-------------------------------------------------*/

void ui_update_and_render(void)
{
	/* always start clean */
	render_container_empty(render_container_get_ui());

	/* if we're paused, dim the whole screen */
	if (mame_get_phase(Machine) >= MAME_PHASE_RESET && (single_step || mame_is_paused(Machine)))
	{
		int alpha = (1.0f - options.pause_bright) * 255.0f;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			render_ui_add_rect(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* call the current UI handler */
	assert(ui_handler_callback != NULL);
	ui_handler_param = (*ui_handler_callback)(ui_handler_param);

	/* cancel takes us back to the ingame handler */
	if (ui_handler_param == UI_HANDLER_CANCEL)
		ui_set_handler(handler_ingame, 0);

	/* add a message if we are rescaling */
	if (display_rescale_message)
	{
		display_rescale_message = FALSE;
		if (allow_rescale == 0)
			allow_rescale = 2;
		ui_draw_text_box("Updating Artwork...", JUSTIFY_CENTER, 0.5f, 0.5f, messagebox_backcolor);
	}

	/* decrement the frame counter if it is non-zero */
	else if (allow_rescale != 0)
		allow_rescale--;

#ifdef MESS
	/* let MESS display its stuff */
	mess_ui_update();
#endif
}


/*-------------------------------------------------
    ui_get_font - return the UI font
-------------------------------------------------*/

render_font *ui_get_font(void)
{
	return ui_font;
}


/*-------------------------------------------------
    ui_get_line_height - return the current height
    of a line
-------------------------------------------------*/

int ui_get_line_height(void)
{
	return UI_SCALE_TO_INT_Y(UI_TARGET_FONT_HEIGHT);
}


/*-------------------------------------------------
    ui_get_char_width - return the width of a
    single character
-------------------------------------------------*/

int ui_get_char_width(UINT16 ch)
{
	return UI_SCALE_TO_INT_X(render_font_get_char_width(ui_font, UI_TARGET_FONT_HEIGHT, render_get_ui_aspect(), ch));
}


/*-------------------------------------------------
    ui_get_string_width - return the width of a
    character string
-------------------------------------------------*/

int ui_get_string_width(const char *s)
#if 0
{
	return UI_SCALE_TO_INT_X(render_font_get_string_width(ui_font, UI_TARGET_FONT_HEIGHT, render_get_ui_aspect(), s));
}
#else
{
	int len = 0;

	while (*s)
	{
		UINT16 code;
		int increment;

		increment = uifont_decodechar(s, &code);
#ifdef UI_COLOR_DISPLAY
		if (increment == 3)
		{
			s++;
			continue;
		}
#endif /* UI_COLOR_DISPLAY */

		len += ui_get_char_width(code);
		s += increment;
	}

	return len;
}
#endif


/*-------------------------------------------------
    ui_draw_outlined_box - add primitives to draw
    an outlined box with the given background
    color
-------------------------------------------------*/

void ui_draw_outlined_box(float x0, float y0, float x1, float y1, rgb_t backcolor)
{
#if 0
	float hw = UI_LINE_WIDTH * 0.5f;

#ifdef UI_COLOR_DISPLAY
	if (backcolor == UI_FILLCOLOR)
		render_ui_add_quad(x0, y0, x1, y1, MAKE_ARGB(0xff, 0xff, 0xff, 0xff), bgtexture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	else
#endif /* UI_COLOR_DISPLAY */
		render_ui_add_rect(x0, y0, x1, y1, backcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	render_ui_add_line(x0 + hw, y0 + hw, x1 - hw, y0 + hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x1 - hw, y0 + hw, x1 - hw, y1 - hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x1 - hw, y1 - hw, x0 + hw, y1 - hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x0 + hw, y1 - hw, x0 + hw, y0 + hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
#else
	int px0 = x0 * (float)ui_screen_width;
	int py0 = y0 * (float)ui_screen_height;
	int px1 = x1 * (float)ui_screen_width;
	int py1 = y1 * (float)ui_screen_height;

	add_filled_box_color(px0, py0, px1, py1, backcolor);
#endif
}


/*-------------------------------------------------
    ui_draw_text - simple text renderer
-------------------------------------------------*/

void ui_draw_text(const char *buf, int x, int y)
{
	ui_draw_text_full(buf, x, y, ui_screen_width - x, 0, 0, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);
}


/*-------------------------------------------------
    ui_draw_text_full - full featured text
    renderer with word wrapping, justification,
    and full size computation
-------------------------------------------------*/

void ui_draw_text_full(const char *origs, int x, int y, int wrapwidth, int offset, int maxlines, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, int *totalwidth, int *totalheight)
{
	const unsigned char *s = (const unsigned char *)origs;
	const unsigned char *linestart;
	int cury = y;
	int maxwidth = 0;
	const unsigned char *uparrow = NULL;

	if (offset)
		uparrow = ui_getstring (UI_uparrow);

	/* if we don't want wrapping, guarantee a huge wrapwidth */
	if (wrap == WRAP_NEVER)
		wrapwidth = 1 << 30;
	if (wrapwidth <= 0)
		return;

	/* loop over lines */
	while (*s)
	{
		const unsigned char *lastspace = NULL;
		int line_justify = justify;
		int lastspace_width = 0;
		int curwidth = 0;
		int curx = x;
		const unsigned char *lastchar;
		int lastchar_width = 0;
		const unsigned char *lasttruncate;
		int lasttruncate_width = 0;
		int truncate_width = 3 * ui_get_char_width('.');
		UINT16 code;
		int increment;
		const unsigned char *end;
		int has_DBC = FALSE;

		/* if the line starts with a tab character, center it regardless */
		if (*s == '\t')
		{
			s++;
			line_justify = JUSTIFY_CENTER;
		}

		/* remember the starting position of the line */
		linestart = s;

		lastchar = linestart;
		lasttruncate = linestart;

		/* loop while we have characters and are less than the wrapwidth */
		while (*s && curwidth <= wrapwidth)
		{
			/* if we hit a newline, stop immediately */
			if (*s == '\n')
				break;

			/* if we hit a space, remember the location and the width there */
			if (*s == ' ' || has_DBC)
			{
				lastspace = s;
				lastspace_width = curwidth;
				has_DBC = FALSE;
			}

			lastchar = s;
			lastchar_width = curwidth;

			if (curwidth + truncate_width <= wrapwidth)
			{
				lasttruncate = s;
				lasttruncate_width = curwidth;
			}

			increment = uifont_decodechar(s, &code);
#ifdef UI_COLOR_DISPLAY
			if (increment == 3)
			{
				s++;
				continue;
			}
#endif /* UI_COLOR_DISPLAY */

			if (code > 0x00ff)
				has_DBC = TRUE;

			/* add the width of this character and advance */
			curwidth += ui_get_char_width(code);
			s += increment;
		}

		/* if we accumulated too much for the current width, we need to back off */
		if (curwidth > wrapwidth)
		{
			/* if we're word wrapping, back up to the last space if we can */
			if (wrap == WRAP_WORD)
			{
				/* if we hit a space, back up to there with the appropriate width */
				if (lastspace)
				{
					s = lastspace;
					curwidth = lastspace_width;
				}

				/* if we didn't hit a space, back up one character */
				else if (s > linestart && lastchar > linestart)
				{
					s = lastchar;
					curwidth = lastchar_width;
				}
			}

			/* if we're truncating, make sure we have enough space for the ... */
			else if (wrap == WRAP_TRUNCATE)
			{
				s = lasttruncate;
				curwidth = lasttruncate_width;
			}

			while (s > linestart + 1)
			{
				if (s[-1] != ' ')
					break;

				s--;
				curwidth -= ui_get_char_width(' ');
			}
		}

		/* align according to the justfication */
		if (line_justify == JUSTIFY_CENTER)
			curx += (wrapwidth - curwidth) / 2;
		else if (line_justify == JUSTIFY_RIGHT)
			curx += wrapwidth - curwidth;

		/* track the maximum width of any given line */
		if (curwidth > maxwidth)
			maxwidth = curwidth;

		end = s;

		if (offset == 0 && uparrow)
		{
			linestart = uparrow;
			uparrow = NULL;

			curwidth = ui_get_string_width(linestart);
			end = linestart + strlen(linestart);

			if (curwidth > maxwidth)
				maxwidth = curwidth;

			curx = x + (wrapwidth - curwidth) / 2;
		}

		if (maxlines == 1)
		{
			const unsigned char *check = s;

			/* skip past any spaces at the beginning of the next line */
			if (wrap == WRAP_TRUNCATE)
			{
				while (*check)
				{
					if (*check == '\n')
						break;

					check++;
				}
			}
			else
			{
				if (*check == '\n')
					check++;
				else
					while (*check && isspace(*check)) check++;
			}

			if (*check)
			{
				linestart = ui_getstring (UI_downarrow);
				end = linestart + strlen(linestart);

				curwidth = ui_get_string_width(linestart);
				if (curwidth > maxwidth)
					maxwidth = curwidth;

				curx = x + (wrapwidth - curwidth) / 2;
			}
		}

		/* if opaque, add a black box */
		if (draw == DRAW_OPAQUE)
			add_fill(curx, cury, curx + curwidth - 1, cury + ui_get_line_height() - 1, bgcolor);

		/* loop from the line start and add the characters */
		while (offset == 0 && draw != DRAW_NONE && linestart < end)
		{
			increment = uifont_decodechar(linestart, &code);
#ifdef UI_COLOR_DISPLAY
			if (increment == 3)
			{
				linestart++;
				continue;
			}
#endif /* UI_COLOR_DISPLAY */

			add_char(curx, cury, code, fgcolor);
			curx += ui_get_char_width(code);
			linestart += increment;
		}

		/* append ellipses if needed */
		if (wrap == WRAP_TRUNCATE && *s != 0 && draw != DRAW_NONE)
		{
			add_char(curx, cury, '.', fgcolor);
			curx += ui_get_char_width('.');
			add_char(curx, cury, '.', fgcolor);
			curx += ui_get_char_width('.');
			add_char(curx, cury, '.', fgcolor);
			curx += ui_get_char_width('.');
		}

		/* if we're not word-wrapping, we're done */
		if (wrap != WRAP_WORD)
			break;

		if (offset)
			offset--;
		else
		{
			/* advance by a row */
			cury += ui_get_line_height();

			if (maxlines)
			{
				maxlines--;
				if (maxlines <= 0)
					draw = DRAW_NONE;
			}
		}

		/* skip past any spaces at the beginning of the next line */
		if (*s == '\n')
			s++;
		else
			while (*s && isspace(*s)) s++;
	}

	/* report the width and height of the resulting space */
	if (totalwidth)
		*totalwidth = maxwidth;
	if (totalheight)
		*totalheight = cury - y;
}


/*-------------------------------------------------
    ui_draw_text_box - draw a multiline text
    message with a box around it
-------------------------------------------------*/

static void ui_draw_text_box_scroll(const char *text, int offset, int justify, float xpos, float ypos, rgb_t backcolor)
{
	int target_width, target_height;
	int target_x, target_y;
	int margin_x, margin_y;

	/* compute the multi-line target width/height */
	ui_draw_text_full(text, 0, 0, ui_screen_width - 2 * UI_BOX_LR_BORDER - 2, 0, 0,
				justify, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);

	multiline_text_box_target_lines = target_height / ui_get_line_height();

#ifdef UI_COLOR_DISPLAY
	margin_x = ui_get_char_width(' ') / 2 + UI_BOX_LR_BORDER;
	if (target_width + 2 * margin_x > ui_screen_width)
		margin_x = (ui_screen_width - target_width) / 2;

	margin_y = ui_get_line_height() / 2 + UI_BOX_TB_BORDER;
#else /* UI_COLOR_DISPLAY */
	margin_x = UI_BOX_LR_BORDER;
	margin_y = UI_BOX_TB_BORDER;
#endif /* UI_COLOR_DISPLAY */

	if (target_height > ui_screen_height - 2 * margin_y)
		target_height = ((ui_screen_height - 2 * margin_y) / ui_get_line_height()) * ui_get_line_height();

	multiline_text_box_visible_lines = target_height / ui_get_line_height();

	/* determine the target location */
	target_x = (int)(xpos * ui_screen_width) - target_width / 2;
	target_y = (int)(ypos * ui_screen_height) - target_height / 2;

	/* make sure we stay on-screen */
	if (target_x < margin_x)
		target_x = margin_x;
	if (target_x + target_width + margin_x > ui_screen_width)
		target_x = ui_screen_width - margin_x - target_width;
	if (target_y < margin_y)
		target_y = margin_y;
	if (target_y + target_height + margin_y > ui_screen_height)
		target_y = ui_screen_height - margin_y - target_height;

	/* add a box around that */
	add_filled_box_color(target_x - margin_x,
	               target_y - margin_y,
	               target_x + target_width - 1 + margin_x,
	               target_y + target_height - 1 + margin_y, backcolor);

	ui_draw_text_full(text, target_x, target_y, target_width, offset, multiline_text_box_visible_lines,
				justify, WRAP_WORD, DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
}


void ui_draw_text_box(const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	ui_draw_text_box_scroll(text, message_window_scroll, justify, xpos, ypos, backcolor);
}


int ui_window_scroll_keys(void)
{
	static int counter = 0;
	static int fast = 6;
	int pan_lines;
	int max_scroll;
	int do_scroll = FALSE;

	max_scroll = multiline_text_box_target_lines - multiline_text_box_visible_lines;
	pan_lines = multiline_text_box_visible_lines - 1;

	if (scroll_reset)
	{
		message_window_scroll = 0;
		scroll_reset = 0;
	}

	/* up backs up by one item */
	if (input_ui_pressed_repeat(IPT_UI_UP, fast))
	{
		message_window_scroll--;
		do_scroll = TRUE;
	}

	/* down advances by one item */
	if (input_ui_pressed_repeat(IPT_UI_DOWN, fast))
	{
		message_window_scroll++;
		do_scroll = TRUE;
	}

	/* pan-up goes to previous page */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_UP,8))
	{
		message_window_scroll -= pan_lines;
		do_scroll = TRUE;
	}

	/* pan-down goes to next page */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_DOWN,8))
	{
		message_window_scroll += pan_lines;
		do_scroll = TRUE;
	}

	/* home goes to the start */
	if (input_ui_pressed(IPT_UI_HOME))
	{
		message_window_scroll = 0;
		do_scroll = TRUE;
	}

	/* end goes to the last */
	if (input_ui_pressed(IPT_UI_END))
	{
		message_window_scroll = max_scroll;
		do_scroll = TRUE;
	}

	if (message_window_scroll < 0)
		message_window_scroll = 0;
	if (message_window_scroll > max_scroll)
		message_window_scroll = max_scroll;

	if (input_port_type_pressed(IPT_UI_UP,0) || input_port_type_pressed(IPT_UI_DOWN,0))
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

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		message_window_scroll = 0;
		return 1;
	}
	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		message_window_scroll = 0;
		return 2;
	}

	return 0;
}

/*-------------------------------------------------
    ui_popup - popup a message for a standard
    amount of time
-------------------------------------------------*/

void CLIB_DECL ui_popup(const char *text, ...)
{
	int seconds;
	va_list arg;

	/* extract the text */
	va_start(arg,text);
	vsprintf(messagebox_text, text, arg);
	messagebox_backcolor = UI_FILLCOLOR;
	va_end(arg);

	/* set a timer */
	seconds = (int)strlen(messagebox_text) / 40 + 2;
	popup_text_end = osd_ticks() + osd_ticks_per_second() * seconds;
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
	vsprintf(messagebox_text, text, arg);
	messagebox_backcolor = UI_FILLCOLOR;
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
	if (show)
	{
		show_profiler = TRUE;
		profiler_start();
	}
	else
	{
		show_profiler = FALSE;
		profiler_stop();
	}
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
    ui_is_menu_active - return TRUE if the menu
    UI handler is active
-------------------------------------------------*/

int ui_is_menu_active(void)
{
	return (ui_handler_callback == ui_menu_ui_handler);
}


/*-------------------------------------------------
    ui_is_slider_active - return TRUE if the slider
    UI handler is active
-------------------------------------------------*/

int ui_is_slider_active(void)
{
	return (ui_handler_callback == handler_slider);
}



/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    sprintf_font_warning - print the warning
    text about font to the given buffer
-------------------------------------------------*/

static int sprintf_font_warning(char *buffer)
{
	static const char *font_warning_string =
		"Local font file is not installed. "
		"You must install CJK font into font directory first.\n\n"
		"Please download from:\n"
		"http://mameplus.emu-france.com/\n\n"
		"Press ESC to exit, type OK to continue.";

	strcpy(buffer, font_warning_string);
	return strlen(buffer);
}

/*-------------------------------------------------
    sprintf_disclaimer - print the disclaimer
    text to the given buffer
-------------------------------------------------*/

static int sprintf_disclaimer(char *buffer)
{
	char *bufptr = buffer;
	bufptr += sprintf(bufptr, "%s\n\n", ui_getstring(UI_copyright1));
	bufptr += sprintf(bufptr, ui_getstring(UI_copyright2),
		options.use_lang_list ? _LST(Machine->gamedrv->description) : Machine->gamedrv->description);
	bufptr += sprintf(bufptr, "\n\n%s", ui_getstring(UI_copyright3));
	return bufptr - buffer;
}


/*-------------------------------------------------
    sprintf_warnings - print the warning flags
    text to the given buffer
-------------------------------------------------*/

static int sprintf_warnings(char *buffer)
{
#define WARNING_FLAGS (	GAME_NOT_WORKING | \
						GAME_UNEMULATED_PROTECTION | \
						GAME_WRONG_COLORS | \
						GAME_IMPERFECT_COLORS | \
						GAME_NO_SOUND |  \
						GAME_IMPERFECT_SOUND |  \
						GAME_IMPERFECT_GRAPHICS | \
						GAME_NO_COCKTAIL)
	char *bufptr = buffer;
	int i;

	/* if no warnings, nothing to return */
	if (rom_load_warnings() == 0 && !(Machine->gamedrv->flags & WARNING_FLAGS))
		return 0;

	/* add a warning if any ROMs were loaded with warnings */
	if (rom_load_warnings() > 0)
	{
		bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_incorrectroms));
		if (Machine->gamedrv->flags & WARNING_FLAGS)
			*bufptr++ = '\n';
	}

	/* if we have at least one warning flag, print the general header */
	if (Machine->gamedrv->flags & WARNING_FLAGS)
	{
		bufptr += sprintf(bufptr, "%s\n\n", ui_getstring(UI_knownproblems));

		/* add one line per warning flag */
#ifdef MESS
		if (Machine->gamedrv->flags & GAME_COMPUTER)
			bufptr += sprintf(bufptr, "%s\n\n%s\n", ui_getstring(UI_comp1), ui_getstring(UI_comp2));
#endif
		if (Machine->gamedrv->flags & GAME_IMPERFECT_COLORS)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_imperfectcolors));
		if (Machine->gamedrv->flags & GAME_WRONG_COLORS)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_wrongcolors));
		if (Machine->gamedrv->flags & GAME_IMPERFECT_GRAPHICS)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_imperfectgraphics));
		if (Machine->gamedrv->flags & GAME_IMPERFECT_SOUND)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_imperfectsound));
		if (Machine->gamedrv->flags & GAME_NO_SOUND)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_nosound));
		if (Machine->gamedrv->flags & GAME_NO_COCKTAIL)
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_nococktail));

		/* if there's a NOT WORKING or UNEMULATED PROTECTION warning, make it stronger */
		if (Machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
		{
			const game_driver *maindrv;
			const game_driver *clone_of;
			int foundworking;

			/* add the strings for these warnings */
			if (Machine->gamedrv->flags & GAME_NOT_WORKING)
				bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_brokengame));
			if (Machine->gamedrv->flags & GAME_UNEMULATED_PROTECTION)
				bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_brokenprotection));

			/* find the parent of this driver */
			clone_of = driver_get_clone(Machine->gamedrv);
			if (clone_of != NULL && !(clone_of->flags & NOT_A_DRIVER))
 				maindrv = clone_of;
			else
				maindrv = Machine->gamedrv;

			/* scan the driver list for any working clones and add them */
			foundworking = 0;
			for (i = 0; drivers[i] != NULL; i++)
				if (drivers[i] == maindrv || driver_get_clone(drivers[i]) == maindrv)
					if ((drivers[i]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) == 0)
					{
						/* this one works, add a header and display the name of the clone */
						if (foundworking == 0)
							bufptr += sprintf(bufptr, "\n\n%s\n\n", ui_getstring(UI_workingclones));
						bufptr += sprintf(bufptr, "%s\n", drivers[i]->name);
						foundworking = 1;
					}
		}
	}

	/* add the 'press OK' string */
	bufptr += sprintf(bufptr, "\n\n%s", ui_getstring(UI_typeok));
	return bufptr - buffer;
}


#ifdef USE_SHOW_TIME

#define DISPLAY_AMPM 0

static void display_time(void)
{
	char buf[20];
#if DISPLAY_AMPM
	char am_pm[] = "am";
#endif /* DISPLAY_AMPM */
	int width;
	time_t ltime;
	struct tm *today;

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
	width = ui_get_string_width(buf);
	switch(Show_Time_Position)
	{
		case 0:
			ui_draw_text(buf, ui_screen_width - width, ui_screen_height - ui_get_line_height());
			break;

		case 1:
			ui_draw_text(buf, ui_screen_width - width, 0);
			break;

		case 2:
			ui_draw_text(buf, 0, 0);
			break;

		case 3:
			ui_draw_text(buf, 0, ui_screen_height - ui_get_line_height());
			break;
	}
}
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
static void display_input_log(void)
{
	add_filled_box_noedge(0, ui_screen_height - ui_get_line_height(), ui_screen_width - 1, ui_screen_height - 1);
	ui_draw_text(command_buffer, 0, ui_screen_height - ui_get_line_height());

	if (--command_counter == 0)
		memset(command_buffer, 0, COMMAND_LOG_BUFSIZE);
}
#endif /* USE_SHOW_INPUT_LOG */




#ifdef INP_CAPTION
//============================================================
//	draw_caption
//============================================================

static void draw_caption(void)
{
	static char next_caption[512], caption_text[512];
	static int next_caption_timer;

	if (options.caption && next_caption_frame < 0)
	{
		char	read_buf[512];
skip_comment:
		if (mame_fgets(read_buf, 511, options.caption) == NULL)
		{
			mame_fclose(options.caption);
			options.caption = NULL;
		}
		else
		{
			char	buf[16] = "";
			int		i, j;

			for (i = 0, j = 0; i < 16; i++)
			{
				if (read_buf[i] == '\t' || read_buf[i] == ' ')
					continue;
				if ((read_buf[i] == '#' || read_buf[i] == '\r' || read_buf[i] == '\n') && j == 0)
					goto skip_comment;
				if (read_buf[i] < '0' || read_buf[i] > '9')
				{
					buf[j++] ='\0';
					break;
				}
				buf[j++] = read_buf[i];
			}

			next_caption_frame = strtol(buf, NULL, 10);
			next_caption_timer = 0;
			if (next_caption_frame == 0)
			{
				next_caption_frame = cpu_getcurrentframe();
				strcpy(next_caption, _("Error: illegal caption file"));
				mame_fclose(options.caption);
				options.caption = NULL;
			}

			for (;;i++)
			{
				if (read_buf[i] == '(')
				{
					for (i++, j = 0;;i++)
					{
						if (read_buf[i] == '\t' || read_buf[i] == ' ')
							continue;
						if (read_buf[i] < '0' || read_buf[i] > '9')
						{
							buf[j++] ='\0';
							break;
						}
						buf[j++] = read_buf[i];
					}

					next_caption_timer = strtol(buf, NULL, 10);

					for (;;i++)
					{
						if (read_buf[i] == '\t' || read_buf[i] == ' ')
							continue;
						if (read_buf[i] == ':')
							break;
					}
				}
				if (read_buf[i] != '\t' && read_buf[i] != ' ' && read_buf[i] != ':')
					break;
			}
			if (next_caption_timer == 0)
			{
				next_caption_timer = 5 * Machine->screen[0].refresh;	// 5sec.
			}

			strcpy(next_caption, &read_buf[i]);

			for (i = 0; next_caption[i] != '\0'; i++)
			{
				if (next_caption[i] == '\r' || next_caption[i] == '\n')
				{
					next_caption[i] = '\0';
					break;
				}
			}
		}
	}

	if (next_caption_timer && next_caption_frame <= cpu_getcurrentframe())
	{
		caption_timer = next_caption_timer;
		strcpy(caption_text, next_caption);
		next_caption_frame = -1;
		next_caption_timer = 0;
	}

	if (caption_timer)
	{
		ui_draw_text_box(caption_text, JUSTIFY_LEFT, 0.5, 1.0, messagebox_backcolor);
		caption_timer--;
	}
}
#endif /* INP_CAPTION */


/*-------------------------------------------------
    sprintf_game_info - print the game info text
    to the given buffer
-------------------------------------------------*/

int sprintf_game_info(char *buffer)
{
	char *bufptr = buffer;
	int cpunum, sndnum;
	int count;

	/* print description, manufacturer, and CPU: */
	bufptr += sprintf(bufptr, "%s\n%s %s\n\n%s:\n",
		options.use_lang_list ? _LST(Machine->gamedrv->description) : Machine->gamedrv->description,
		Machine->gamedrv->year,
		options.use_lang_list ? _MANUFACT(Machine->gamedrv->manufacturer) : Machine->gamedrv->manufacturer,
		ui_getstring(UI_cpu));

	/* loop over all CPUs */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].cpu_type; cpunum += count)
	{
		int type = Machine->drv->cpu[cpunum].cpu_type;
		int clock = Machine->drv->cpu[cpunum].cpu_clock;

		/* count how many identical CPUs we have */
		for (count = 1; cpunum + count < MAX_CPU; count++)
			if (Machine->drv->cpu[cpunum + count].cpu_type != type ||
		        Machine->drv->cpu[cpunum + count].cpu_clock != clock)
		    	break;

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			bufptr += sprintf(bufptr, "%dx", count);
		bufptr += sprintf(bufptr, "%s", cputype_name(type));

		/* display clock in kHz or MHz */
		if (clock >= 1000000)
			bufptr += sprintf(bufptr, " %d.%06d MHz\n", clock / 1000000, clock % 1000000);
		else
			bufptr += sprintf(bufptr, " %d.%03d kHz\n", clock / 1000, clock % 1000);
	}

	/* append the Sound: string */
	bufptr += sprintf(bufptr, "\n%s:\n", ui_getstring(UI_sound));

	/* loop over all sound chips */
	for (sndnum = 0; sndnum < MAX_SOUND && Machine->drv->sound[sndnum].sound_type; sndnum += count)
	{
		int type = Machine->drv->sound[sndnum].sound_type;
		int clock = sndnum_clock(sndnum);

		/* count how many identical sound chips we have */
		for (count = 1; sndnum + count < MAX_SOUND; count++)
			if (Machine->drv->sound[sndnum + count].sound_type != type ||
		        sndnum_clock(sndnum + count) != clock)
		    	break;

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			bufptr += sprintf(bufptr, "%dx", count);
		bufptr += sprintf(bufptr, "%s", sndnum_name(sndnum));

		/* display clock in kHz or MHz */
		if (clock >= 1000000)
			bufptr += sprintf(bufptr, " %d.%06d MHz\n", clock / 1000000, clock % 1000000);
		else if (clock != 0)
			bufptr += sprintf(bufptr, " %d.%03d kHz\n", clock / 1000, clock % 1000);
		else
			*bufptr++ = '\n';
	}

	/* display vector information for vector games */
	if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
		bufptr += sprintf(bufptr, "\n%s\n", ui_getstring(UI_vectorgame));

	/* display screen resolution and refresh rate info for raster games */
	else
		bufptr += sprintf(bufptr,"\n%s:\n%d x %d (%s) %f Hz\n",
				ui_getstring(UI_screenres),
				Machine->screen[0].visarea.max_x - Machine->screen[0].visarea.min_x + 1,
				Machine->screen[0].visarea.max_y - Machine->screen[0].visarea.min_y + 1,
				(Machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "V" : "H",
				Machine->screen[0].refresh);
	return bufptr - buffer;
}



/***************************************************************************
    UI HANDLERS
***************************************************************************/

/*-------------------------------------------------
    handler_messagebox - displays the current
    messagebox_text string but handles no input
-------------------------------------------------*/

static UINT32 handler_messagebox(UINT32 state)
{
	ui_draw_text_box(messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);
	return 0;
}


/*-------------------------------------------------
    handler_messagebox_ok - displays the current
    messagebox_text string and waits for an OK
-------------------------------------------------*/

static UINT32 handler_messagebox_ok(UINT32 state)
{
	int res;

	/* draw a standard message window */
	ui_draw_text_box(messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	res = ui_window_scroll_keys();
	if (res == 0)
	{
		/* an 'O' or left joystick kicks us to the next state */
		if (state == 0 && (code_pressed_memory(KEYCODE_O) || input_ui_pressed(IPT_UI_LEFT)))
			state++;

		/* a 'K' or right joystick exits the state */
		else if (state == 1 && (code_pressed_memory(KEYCODE_K) || input_ui_pressed(IPT_UI_RIGHT)))
			state = UI_HANDLER_CANCEL;
	}

	/* if the user cancels, exit out completely */
	if (res == 2)
	{
		mame_schedule_exit(Machine);
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/*-------------------------------------------------
    handler_messagebox_selectkey - displays the
    current messagebox_text string and waits for
    selectkey press
-------------------------------------------------*/

static UINT32 handler_messagebox_selectkey(UINT32 state)
{
	int res;

	/* draw a standard message window */
	ui_draw_text_box(messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	res = ui_window_scroll_keys();
	/* if the user cancels, exit out completely */
	if (res == 2)
	{
		mame_schedule_exit(Machine);
		state = UI_HANDLER_CANCEL;
	}

	/* if select key is pressed, just exit */
	if (res == 1)
	{
		if (code_read_async() != CODE_NONE)
			state = UI_HANDLER_CANCEL;
	}

	return state;
}


/*-------------------------------------------------
    handler_ingame - in-game handler takes care
    of the standard keypresses
-------------------------------------------------*/

static UINT32 handler_ingame(UINT32 state)
{
	int is_paused = mame_is_paused(Machine);

	/* first draw the FPS counter */
	if (showfps || osd_ticks() < showfps_end)
		ui_draw_text_full(osd_get_fps_text(mame_get_performance_info()), 0, 0, ui_screen_width, 0, 0,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);
	else
		showfps_end = 0;

	/* draw the profiler if visible */
	if (show_profiler)
		ui_draw_text_full(profiler_get_text(), 0, 0, ui_screen_width, 0, 0, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);

	/* let the cheat engine display its stuff */
	if (options.cheat)
		cheat_display_watches();

	/* display any popup messages */
	if (osd_ticks() < popup_text_end)
		ui_draw_text_box(messagebox_text, JUSTIFY_CENTER, 0.5f, 0.9f, messagebox_backcolor);
	else
		popup_text_end = 0;

	/* if we're single-stepping, pause now */
	if (single_step)
	{
		mame_pause(Machine, TRUE);
		single_step = FALSE;
	}

#ifdef MESS
	if (options.disable_normal_ui || ((Machine->gamedrv->flags & GAME_COMPUTER) && !mess_ui_active()))
		return 0;
#endif

	/* if the user pressed ESC, stop the emulation */
	if (input_ui_pressed(IPT_UI_CANCEL))
		return ui_set_handler(handler_confirm_quit, 0);

	/* turn on menus if requested */
	if (input_ui_pressed(IPT_UI_CONFIGURE))
		return ui_set_handler(ui_menu_ui_handler, 0);

	if (options.cheat && input_ui_pressed(IPT_UI_CHEAT))
		return ui_set_handler(ui_menu_ui_handler, SHORTCUT_MENU_CHEAT);

#ifdef CMD_LIST
	if (input_ui_pressed(IPT_UI_COMMAND))
		return ui_set_handler(ui_menu_ui_handler, SHORTCUT_MENU_COMMAND);
#endif /* CMD_LIST */

	/* if the on-screen display isn't up and the user has toggled it, turn it on */
#ifdef MAME_DEBUG
	if (!Machine->debug_mode)
#endif
		if (input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
			return ui_set_handler(handler_slider, 0);

	/* handle a reset request */
	if (input_ui_pressed(IPT_UI_RESET_MACHINE))
		mame_schedule_hard_reset(Machine);
	if (input_ui_pressed(IPT_UI_SOFT_RESET))
		mame_schedule_soft_reset(Machine);

	/* handle a request to display graphics/palette */
	if (input_ui_pressed(IPT_UI_SHOW_GFX))
	{
		if (!is_paused)
			mame_pause(Machine, TRUE);
		return ui_set_handler(ui_gfx_ui_handler, is_paused);
	}

	/* handle a save state request */
	if (input_ui_pressed(IPT_UI_SAVE_STATE))
	{
		mame_pause(Machine, TRUE);
		return ui_set_handler(handler_load_save, LOADSAVE_SAVE);
	}

	/* handle a load state request */
	if (input_ui_pressed(IPT_UI_LOAD_STATE))
	{
		mame_pause(Machine, TRUE);
		return ui_set_handler(handler_load_save, LOADSAVE_LOAD);
	}

	/* handle a save snapshot request */
	if (input_ui_pressed(IPT_UI_SNAPSHOT))
		video_save_active_screen_snapshots();

#ifdef INP_CAPTION
	draw_caption();
#endif /* INP_CAPTION */

	/* toggle pause */
	if (input_ui_pressed(IPT_UI_PAUSE))
	{
		/* with a shift key, it is single step */
		if (is_paused && (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT)))
		{
			single_step = TRUE;
			mame_pause(Machine, FALSE);
		}
		else
			mame_pause(Machine, !mame_is_paused(Machine));
	}


#ifdef USE_SHOW_TIME
	if (input_ui_pressed(IPT_UI_TIME))
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
		display_time();
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
	if (input_ui_pressed(IPT_UI_SHOW_INPUT_LOG))
	{
		show_input_log ^= 1;
		memset(command_buffer, 0, COMMAND_LOG_BUFSIZE);
	}

	/* show popup message if input exist any log */
	if (show_input_log && command_counter)
		display_input_log();
#endif /* USE_SHOW_INPUT_LOG */

	/* toggle movie recording */
	if (input_ui_pressed(IPT_UI_RECORD_MOVIE))
	{
		if (!video_is_movie_active())
		{
			video_movie_begin_recording(NULL);
			popmessage(_("REC START"));
		}
		else
		{
			video_movie_end_recording();
			popmessage(_("REC STOP"));
		}
	}

	/* toggle profiler display */
	if (input_ui_pressed(IPT_UI_SHOW_PROFILER))
		ui_set_show_profiler(!ui_get_show_profiler());

	/* toggle FPS display */
	if (input_ui_pressed(IPT_UI_SHOW_FPS))
		ui_set_show_fps(!ui_get_show_fps());

	/* toggle crosshair display */
	if (input_ui_pressed(IPT_UI_TOGGLE_CROSSHAIR))
		video_crosshair_toggle();

	return 0;
}


/*-------------------------------------------------
    handler_slider - displays the current slider
    and calls the slider handler
-------------------------------------------------*/

static UINT32 handler_slider(UINT32 state)
{
	slider_state *cur = &slider_list[slider_current];
	INT32 increment = 0, newval;
	char textbuf[256];

	/* left/right control the increment */
	if (input_ui_pressed_repeat(IPT_UI_LEFT,6))
		increment = -cur->incval;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT,6))
		increment = cur->incval;

	/* alt goes to 1, shift goes 10x smaller, control goes 10x larger */
	if (increment != 0)
	{
		if (code_pressed(KEYCODE_LALT) || code_pressed(KEYCODE_RALT))
			increment = (increment < 0) ? -1 : 1;
		if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
			increment = (increment < -10 || increment > 10) ? (increment / 10) : ((increment < 0) ? -1 : 1);
		if (code_pressed(KEYCODE_LCONTROL) || code_pressed(KEYCODE_RCONTROL))
			increment *= 10;
	}

	/* determine the new value */
	newval = (*cur->update)(0, NULL, cur->arg) + increment;

	/* select resets to the default value */
	if (input_ui_pressed(IPT_UI_SELECT))
		newval = cur->defval;

	/* clamp within bounds */
	if (newval < cur->minval)
		newval = cur->minval;
	if (newval > cur->maxval)
		newval = cur->maxval;

	/* update the new data and get the text */
	(*cur->update)(newval, textbuf, cur->arg);

	/* display the UI */
	slider_display(textbuf, cur->minval, cur->maxval, cur->defval, newval);

	/* up/down select which slider to control */
	if (input_ui_pressed_repeat(IPT_UI_DOWN,6))
		slider_current = (slider_current + 1) % slider_count;
	if (input_ui_pressed_repeat(IPT_UI_UP,6))
		slider_current = (slider_current + slider_count - 1) % slider_count;

	/* the slider toggle or ESC will cancel out of our display */
	if (input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY) || input_ui_pressed(IPT_UI_CANCEL))
		return UI_HANDLER_CANCEL;

	/* the menu key will take us directly to the menu */
	if (input_ui_pressed(IPT_UI_CONFIGURE))
		return ui_set_handler(ui_menu_ui_handler, 0);

	return 0;
}


/*-------------------------------------------------
    handler_load_save - leads the user through
    specifying a game to save or load
-------------------------------------------------*/

static UINT32 handler_load_save(UINT32 state)
{
	char filename[20];
	input_code code;
	char file = 0;

	/* if we're not in the middle of anything, skip */
	if (state == LOADSAVE_NONE)
		return 0;

	/* okay, we're waiting for a key to select a slot; display a message */
	if (state == LOADSAVE_SAVE)
		ui_draw_message_window(_("Select position to save to"));
	else
		ui_draw_message_window(_("Select position to load from"));

	/* check for cancel key */
	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		/* display a popup indicating things were cancelled */
		if (state == LOADSAVE_SAVE)
			popmessage(_("Save cancelled"));
		else
			popmessage(_("Load cancelled"));

		/* reset the state */
		mame_pause(Machine, FALSE);
		return UI_HANDLER_CANCEL;
	}

	/* fetch a code; if it's none, we're done */
	code = code_read_async();
	if (code == CODE_NONE)
		return state;

	/* check for A-Z or 0-9 */
	if (code >= KEYCODE_A && code <= KEYCODE_Z)
		file = code - KEYCODE_A + 'a';
	if (code >= KEYCODE_0 && code <= KEYCODE_9)
		file = code - KEYCODE_0 + '0';
	if (code >= KEYCODE_0_PAD && code <= KEYCODE_9_PAD)
		file = code - KEYCODE_0_PAD + '0';
	if (!file)
		return state;

	/* display a popup indicating that the save will proceed */
	sprintf(filename, "%s-%c.sta", Machine->gamedrv->name, file);
	if (state == LOADSAVE_SAVE)
	{
		popmessage(_("Save to position %c"), file);
		mame_schedule_save(Machine, filename);
	}
	else
	{
		popmessage(_("Load from position %c"), file);
		mame_schedule_load(Machine, filename);
	}

	/* remove the pause and reset the state */
	mame_pause(Machine, FALSE);
	return UI_HANDLER_CANCEL;
}



static UINT32 handler_confirm_quit(UINT32 state)
{
	const char *quit_message =
		"Quit the game?\n\n"
		"Press Select key/button to quit,\n"
		"Cancel key/button to continue.";

	if (!options.confirm_quit)
	{
		mame_schedule_exit(Machine);
		return UI_HANDLER_CANCEL;
	}

	ui_draw_message_window(_(quit_message));

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		mame_schedule_exit(Machine);
		return UI_HANDLER_CANCEL;
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		return UI_HANDLER_CANCEL;
	}

	return 0;
}


/***************************************************************************
    SLIDER CONTROLS
***************************************************************************/

/*-------------------------------------------------
    slider_init - initialize the list of slider
    controls
-------------------------------------------------*/

static void slider_init(void)
{
	int numitems, item;
	input_port_entry *in;

	slider_count = 0;

	/* add overall volume */
	slider_config(&slider_list[slider_count++], -32, 0, 0, 1, slider_volume, 0);

	/* add per-channel volume */
	numitems = sound_get_user_gain_count();
	for (item = 0; item < numitems; item++)
		slider_config(&slider_list[slider_count++], 0, sound_get_default_gain(item) * 1000.0f + 0.5f, 2000, 20, slider_mixervol, item);

	/* add analog adjusters */
	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if ((in->type & 0xff) == IPT_ADJUSTER)
			slider_config(&slider_list[slider_count++], 0, in->default_value >> 8, 100, 1, slider_adjuster, in - Machine->input_ports);

	if (options.cheat)
	{
		/* add CPU overclocking */
		numitems = cpu_gettotalcpu();
		for (item = 0; item < numitems; item++)
			slider_config(&slider_list[slider_count++], 50, 1000, 4000, 50, slider_overclock, item);

		/* add refresh rate tweaker */
		slider_config(&slider_list[slider_count++], -10000, 0, 10000, 1000, slider_refresh, 0);
	}

	for (item = 0; item < MAX_SCREENS; item++)
		if (Machine->drv->screen[item].tag != NULL)
		{
			/* add standard brightness/contrast/gamma controls per-screen */
			slider_config(&slider_list[slider_count++], 100, 1000, 2000, 10, slider_brightness, item);
			slider_config(&slider_list[slider_count++], 100, 1000, 2000, 50, slider_contrast, item);
			slider_config(&slider_list[slider_count++], 100, 1000, 3000, 50, slider_gamma, item);

			/* add scale and offset controls per-screen */
			slider_config(&slider_list[slider_count++], 800, 1000, 1200, 2, slider_xscale, item);
			slider_config(&slider_list[slider_count++], 800, 1000, 1200, 2, slider_yscale, item);
			slider_config(&slider_list[slider_count++], -200, 0, 200, 2, slider_xoffset, item);
			slider_config(&slider_list[slider_count++], -200, 0, 200, 2, slider_yoffset, item);
		}

	if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
	{
		/* add flicker control */
		slider_config(&slider_list[slider_count++], 0, 0, 1000, 10, slider_flicker, 0);
		slider_config(&slider_list[slider_count++], 10, 100, 1000, 10, slider_beam, 0);
	}

#ifdef MAME_DEBUG
	/* add crosshair adjusters */
	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if (in->analog.crossaxis != CROSSHAIR_AXIS_NONE && in->player == 0)
		{
			slider_config(&slider_list[slider_count++], -3000, 1000, 3000, 100, slider_crossscale, in->analog.crossaxis);
			slider_config(&slider_list[slider_count++], -3000, 0, 3000, 100, slider_crossoffset, in->analog.crossaxis);
		}
#endif
}


/*-------------------------------------------------
    slider_display - display a slider box with
    text
-------------------------------------------------*/

static void slider_display(const char *text, int minval, int maxval, int defval, int curval)
{
	float percentage = (float)(curval - minval) / (float)(maxval - minval);
	float default_percentage = (float)(defval - minval) / (float)(maxval - minval);
	int space_width = ui_get_char_width(' ');
	int line_height = ui_get_line_height();
	int text_height;
	int ui_width = ui_screen_width;
	int ui_height = ui_screen_height;

	/* leave a spaces' worth of room along the left/right sides, and a lines' worth on the top/bottom */
	ui_width -= 2 * space_width;
	ui_height -= 2 * line_height;

#ifdef UI_COLOR_DISPLAY
	line_height = line_height * 16 / 10;
#endif /* UI_COLOR_DISPLAY */

	/* determine the text height */
	ui_draw_text_full(text, 0, 0, ui_width - 2 * UI_BOX_LR_BORDER, 0, 0,
				JUSTIFY_CENTER, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);

	/* add a box around the whole area */
	add_filled_box(	space_width,
					line_height + ui_height - text_height - line_height - 2 * UI_BOX_TB_BORDER,
					space_width + ui_width,
					line_height + ui_height);

	/* draw the thermometer */
	slider_draw_bar(2 * space_width, line_height + ui_height - UI_BOX_TB_BORDER - text_height - line_height*3/4,
			ui_width - 2 * space_width, line_height*3/4, percentage, default_percentage);

	/* draw the actual text */
	ui_draw_text_full(text, space_width + UI_BOX_LR_BORDER, line_height + ui_height - UI_BOX_TB_BORDER - text_height, ui_width - 2 * UI_BOX_LR_BORDER, 0, text_height / ui_get_line_height(),
				JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);
}


/*-------------------------------------------------
    slider_draw_bar - draw a slider thermometer
-------------------------------------------------*/

static void slider_draw_bar(int leftx, int topy, int width, int height, float percentage, float default_percentage)
{
	int current_x, default_x;
	int bar_top, bar_bottom;

	/* compute positions */
	bar_top = topy + (height + 7)/8;
	bar_bottom = topy + (height - 1) - (height + 7)/8;
	default_x = leftx + (width - 1) * default_percentage;
	current_x = leftx + (width - 1) * percentage;

	/* draw the top and bottom lines */
	add_line(leftx, bar_top, leftx + width - 1, bar_top, ARGB_WHITE);
	add_line(leftx, bar_bottom, leftx + width - 1, bar_bottom, ARGB_WHITE);

#ifdef UI_COLOR_DISPLAY
	/* top of the bar */
	add_line(leftx, bar_top + 1, current_x, bar_top + 1, OSDBAR_COLOR_FRAMEDARK);

	/* left of the bar */
	add_line(leftx, bar_top + 2, leftx, bar_bottom - 1, OSDBAR_COLOR_FRAMEDARK);

	/* right of the bar */
	add_line(current_x, bar_top+ 2, current_x, bar_bottom - 1, OSDBAR_COLOR_FRAMELIGHT);

	/* fill in the percentage */
	add_fill(leftx + 1, bar_top + 2, current_x - 1, bar_bottom - 1, OSDBAR_COLOR_FRAMEMEDIUM);

	/* draw default marker */
	add_line(default_x, bar_top - 1, default_x, bar_bottom + 1, OSDBAR_COLOR_DEFAULTBAR);
#else /* UI_COLOR_DISPLAY */
	/* draw default marker */
	add_line(default_x, topy, default_x, bar_top, ARGB_WHITE);
	add_line(default_x, bar_bottom, default_x, topy + height - 1, ARGB_WHITE);

	/* fill in the percentage */
	add_fill(leftx, bar_top + 1, current_x, bar_bottom - 1, ARGB_WHITE);
#endif /* UI_COLOR_DISPLAY */
}


/*-------------------------------------------------
    slider_volume - global volume slider callback
-------------------------------------------------*/

static INT32 slider_volume(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		osd_set_mastervolume(newval);
		sprintf(buffer, "%s %3ddB", ui_getstring(UI_volume), osd_get_mastervolume());
	}
	return osd_get_mastervolume();
}


/*-------------------------------------------------
    slider_mixervol - single channel volume
    slider callback
-------------------------------------------------*/

static INT32 slider_mixervol(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		sound_set_user_gain(arg, (float)newval * 0.001f);
		sprintf(buffer, "%s %s %4.2f", sound_get_user_gain_name(arg), ui_getstring(UI_volume), sound_get_user_gain(arg));
	}
	return floor(sound_get_user_gain(arg) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_adjuster - analog adjuster slider
    callback
-------------------------------------------------*/

static INT32 slider_adjuster(INT32 newval, char *buffer, int arg)
{
	input_port_entry *in = &Machine->input_ports[arg];
	if (buffer != NULL)
	{
		in->default_value = (in->default_value & ~0xff) | (newval & 0xff);
		sprintf(buffer, "%s %d%%", _(in->name), in->default_value & 0xff);
	}
	return in->default_value & 0xff;
}


/*-------------------------------------------------
    slider_overclock - CPU overclocker slider
    callback
-------------------------------------------------*/

static INT32 slider_overclock(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		cpunum_set_clockscale(arg, (float)newval * 0.001f);
		sprintf(buffer, "%s %s%d %3.0f%%", ui_getstring(UI_overclock), ui_getstring(UI_cpu), arg, floor(cpunum_get_clockscale(arg) * 100.0f + 0.5f));
	}
	return floor(cpunum_get_clockscale(arg) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_refresh - refresh rate slider callback
-------------------------------------------------*/

static INT32 slider_refresh(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		screen_state *state = &Machine->screen[arg];
		video_screen_configure(arg, state->width, state->height, &state->visarea, Machine->drv->screen[arg].defstate.refresh + (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, ui_getstring(UI_refresh_rate), Machine->screen[arg].refresh);
	}
	return floor((Machine->screen[arg].refresh - Machine->drv->screen[arg].defstate.refresh) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_brightness - screen brightness slider
    callback
-------------------------------------------------*/

static INT32 slider_brightness(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_brightness(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, ui_getstring(UI_brightness), render_container_get_brightness(container));
	}
	return floor(render_container_get_brightness(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_contrast - screen contrast slider
    callback
-------------------------------------------------*/

static INT32 slider_contrast(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_contrast(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, ui_getstring(UI_contrast), render_container_get_contrast(container));
	}
	return floor(render_container_get_contrast(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_gamma - screen gamma slider callback
-------------------------------------------------*/

static INT32 slider_gamma(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_gamma(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, ui_getstring(UI_gamma), render_container_get_gamma(container));
	}
	return floor(render_container_get_gamma(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_xscale(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_xscale(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, _("Horiz stretch"), render_container_get_xscale(container));
	}
	return floor(render_container_get_xscale(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_yscale(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_yscale(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, _("Vert stretch"), render_container_get_yscale(container));
	}
	return floor(render_container_get_yscale(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_xoffset(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_xoffset(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, _("Horiz position"), render_container_get_xoffset(container));
	}
	return floor(render_container_get_xoffset(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_yoffset(INT32 newval, char *buffer, int arg)
{
	render_container *container = render_container_get_screen(arg);
	if (buffer != NULL)
	{
		render_container_set_yoffset(container, (float)newval * 0.001f);
		sprintf(buffer, _("Screen %d %s %.3f"), arg, _("Vert position"), render_container_get_yoffset(container));
	}
	return floor(render_container_get_yoffset(container) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_flicker - vector flicker slider
    callback
-------------------------------------------------*/

static INT32 slider_flicker(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		vector_set_flicker((float)newval * 0.1f);
		sprintf(buffer, "%s %1.2f", ui_getstring(UI_vectorflicker), vector_get_flicker());
	}
	return floor(vector_get_flicker() * 10.0f + 0.5f);
}


/*-------------------------------------------------
    slider_beam - vector beam width slider
    callback
-------------------------------------------------*/

static INT32 slider_beam(INT32 newval, char *buffer, int arg)
{
	if (buffer != NULL)
	{
		vector_set_beam((float)newval * 0.01f);
		sprintf(buffer, "%s %1.2f", _("Beam Width"), vector_get_beam());
	}
	return floor(vector_get_beam() * 100.0f + 0.5f);
}


/*-------------------------------------------------
    slider_crossscale - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossscale(INT32 newval, char *buffer, int arg)
{
	input_port_entry *in;

	if (buffer != NULL)
	{
		for (in = Machine->input_ports; in && in->type != IPT_END; in++)
			if (in->analog.crossaxis == arg)
				in->analog.crossscale = (float)newval * 0.001f;
		sprintf(buffer, "%s %s %1.3f", "Crosshair Scale", (in->analog.crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	}
	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if (in->analog.crossaxis == arg)
			return floor(in->analog.crossscale * 1000.0f + 0.5f);
	return 0;
}
#endif


/*-------------------------------------------------
    slider_crossoffset - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossoffset(INT32 newval, char *buffer, int arg)
{
	input_port_entry *in;

	if (buffer != NULL)
	{
		for (in = Machine->input_ports; in && in->type != IPT_END; in++)
			if (in->analog.crossaxis == arg)
				in->analog.crossoffset = (float)newval * 0.001f;
		sprintf(buffer, "%s %s %1.3f", "Crosshair Offset", (in->analog.crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	}
	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if (in->analog.crossaxis == arg)
			return floor(in->analog.crossoffset * 1000.0f + 0.5f);
	return 0;
}
#endif


void add_fill(int x0, int y0, int x1, int y1, rgb_t color)
{
	x1++;
	y1++;

#ifdef UI_COLOR_DISPLAY
	if (color == UI_FILLCOLOR)
		render_ui_add_quad(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), MAKE_ARGB(0xff, 0xff, 0xff, 0xff), bgtexture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	else
#endif /* UI_COLOR_DISPLAY */
		render_ui_add_rect(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), ui_get_rgb_color(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


static void add_filled_box_color(int x1, int y1, int x2, int y2, rgb_t color)
{
#ifdef UI_COLOR_DISPLAY
	add_fill(x1 + 3, y1 + 3, x2 - 3, y2 - 3, color);

	/* top edge */
	add_line(x1,     y1,     x2,     y1,     SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y1 + 1, x2 - 1, y1 + 1, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1 + 2, y1 + 2, x2 - 2, y1 + 2, SYSTEM_COLOR_FRAMEDARK);

	/* bottom edge */
	add_line(x1 + 2, y2 - 2, x2 - 2, y2 - 2, SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y2 - 1, x2 - 1, y2 - 1, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1,     y2,     x2,     y2,     SYSTEM_COLOR_FRAMEDARK);

	/* left edge */
	add_line(x1,     y1 + 1, x1,     y2,     SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y1 + 2, x1 + 1, y2 - 1, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1 + 2, y1 + 3, x1 + 2, y2 - 2, SYSTEM_COLOR_FRAMEDARK);

	/* right edge */
	add_line(x2 - 2, y1 + 2, x2 - 2, y2 - 1, SYSTEM_COLOR_FRAMELIGHT);
	add_line(x2 - 1, y1 + 1, x2 - 1, y2,     SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x2,     y1,     x2,     y2 + 1, SYSTEM_COLOR_FRAMEDARK);
#else /* UI_COLOR_DISPLAY */
	add_fill(x1 + 1, y1 + 1, x2 - 1, y2 - 1, color);

	add_line(x1, y1, x2, y1, ARGB_WHITE);
	add_line(x2, y1, x2, y2, ARGB_WHITE);
	add_line(x2, y2, x1, y2, ARGB_WHITE);
	add_line(x1, y2, x1, y1, ARGB_WHITE);
#endif /* UI_COLOR_DISPLAY */
}


void add_filled_box(int x0, int y0, int x1, int y1)
{
	add_filled_box_color(x0, y0, x1, y1, ui_bgcolor);
}


void ui_auto_pause(void)
{
	auto_pause = 1;
}


#ifdef USE_SHOW_INPUT_LOG
static void add_filled_box_noedge(int x0, int y0, int x1, int y1)
{
#ifdef UI_COLOR_DISPLAY
	add_fill(x0, y0, x1, y1, ui_bgcolor);
#else /* UI_COLOR_DISPLAY */
	add_fill(x0, y0, x1, y1, ARGB_BLACK);
#endif /* UI_COLOR_DISPLAY */
}
#endif /* USE_SHOW_INPUT_LOG */


/*************************************
 *
 *  Set the UI visible area
 *  (called by OSD layer, will go
 *  away with new rendering system)
 *
 *************************************/

void ui_set_visible_area(int xmin, int ymin, int xmax, int ymax)
{
	float height;
	int lines;

	ui_screen_width = xmax - xmin + 1;
	ui_screen_height = ymax - ymin + 1;

	if (options.ui_lines)
	{
		ui_font_height = 1.0f / (float)options.ui_lines;
		return;
	}

	height = render_font_get_pixel_height(ui_font);
	lines = (float)ui_screen_height / height;

	if (lines < 16)
	{
		ui_font_height = 1.0f / 20.0f;
		return;
	}

	while (lines >= 40)
	{
		height *= 2.0f;
		lines = (float)ui_screen_height / height;
	}

	ui_font_height = height / (float)ui_screen_height;
}

static void build_bgtexture(running_machine *machine)
{
#ifdef UI_COLOR_DISPLAY
	float r = (float)options.uicolortable[UI_FILLCOLOR][0];
	float g = (float)options.uicolortable[UI_FILLCOLOR][1];
	float b = (float)options.uicolortable[UI_FILLCOLOR][2];
#else /* UI_COLOR_DISPLAY */
	UINT8 r = 0x10;
	UINT8 g = 0x10;
	UINT8 b = 0x30;
#endif /* UI_COLOR_DISPLAY */
	UINT8 a = 0xff;
	int i;

	bgbitmap = bitmap_alloc_format(1, 1024, BITMAP_FORMAT_RGB32);
	if (!bgbitmap)
		fatalerror("build_bgtexture failed");

#ifdef TRANS_UI
	if (options.use_transui)
		a = options.ui_transparency;
#endif /* TRANS_UI */

	for (i = 0; i < bgbitmap->height; i++)
	{
		double gradual = (float)(1024 - i) / 1024.0f + 0.1f;

		if (gradual > 1.0f)
			gradual = 1.0f;
		else if (gradual < 0.2f)
			gradual = 0.2f;

		*BITMAP_ADDR32(bgbitmap, i, 0) = MAKE_ARGB(a, (UINT8)(r * gradual), (UINT8)(g * gradual), (UINT8)(b * gradual));
	}

	bgtexture = render_texture_alloc(bgbitmap, NULL, 0, TEXFORMAT_ARGB32, render_texture_hq_scale, NULL);
	add_exit_callback(machine, free_bgtexture);
}

static void free_bgtexture(running_machine *machine)
{
	bitmap_free(bgbitmap);
	bgbitmap = NULL;
	render_texture_free(bgtexture);
	bgtexture = NULL;
}
