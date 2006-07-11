/*
To do:
 - make single step work reliably
*/
/*********************************************************************

    usrintrf.c

    Functions used to handle MAME's user interface.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "osdepend.h"
#include "driver.h"
#include "info.h"
#include "vidhrdw/vector.h"
#include "ui_text.h"
#include "profiler.h"
#include "cheat.h"
#include "datafile.h"
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#ifdef USE_SHOW_TIME
#include <time.h>
#endif /* USE_SHOW_TIME */

#ifdef NEW_RENDER
#include "render.h"
#include "rendfont.h"
#endif

#ifdef USE_SCALE_EFFECTS
#include "osdscale.h"
#include "video.h"
#endif /* USE_SCALE_EFFECTS */

#ifdef MESS
#include "mess.h"
#include "mesintrf.h"
#include "inputx.h"
#endif



/*************************************
 *
 *  Constants
 *
 *************************************/

#define SEL_BITS				12
#define SEL_MASK				((1<<SEL_BITS)-1)

#define MENU_STACK_DEPTH		8
#define MENU_STRING_POOL_SIZE	(64*1024)

#define MAX_ANALOG_ENTRIES		80
#define MAX_DIP_SWITCHES		256
#define MAX_PORT_ENTRIES		1000
#define MAX_SETUPMENU_ITEMS		20
#define MAX_OSD_ITEMS			50

#define ARGB_WHITE				MAKE_ARGB(0xff,0xff,0xff,0xff)
#define ARGB_BLACK				MAKE_ARGB(0xff,0x00,0x00,0x00)

#ifdef NEW_RENDER
#define MENU_TEXTCOLOR			MAKE_ARGB(0xff,0xff,0xff,0xff)
#define MENU_SELECTCOLOR		MAKE_ARGB(0xff,0xff,0xff,0x00)
#define MENU_BACKCOLOR			MAKE_ARGB(0xe0,0x10,0x10,0x30)
#else
#define MENU_TEXTCOLOR			MAKE_ARGB(0xff,0xff,0xff,0xff)
#define MENU_SELECTCOLOR		MAKE_ARGB(0xff,0xff,0xff,0x00)
#define MENU_BACKCOLOR			MAKE_ARGB(0x00,0x00,0x00,0x00)
#endif

enum
{
	ANALOG_ITEM_KEYSPEED = 0,
	ANALOG_ITEM_CENTERSPEED,
	ANALOG_ITEM_REVERSE,
	ANALOG_ITEM_SENSITIVITY,
	ANALOG_ITEM_COUNT
};

#ifdef TRANS_UI
#ifndef NEW_RENDER
#define UI_TRANSPARENT_COLOR	0xfffffffe
#else
#define UI_TRANSPARENT_COLOR	SYSTEM_COLOR_BACKGROUND
#endif
#endif /* TRANS_UI */

enum
{
	LOADSAVE_NONE,
	LOADSAVE_LOAD,
	LOADSAVE_SAVE
};

#define SHORTCUT_MENU_CHEAT	1
#ifdef CMD_LIST
#define SHORTCUT_MENU_COMMAND	2
#endif /* CMD_LIST */



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _input_item_data input_item_data;
struct _input_item_data
{
	input_seq *		seq;
	UINT16 			sortorder;
	UINT8 			digital;
};



/*************************************
 *
 *  Macros
 *
 *************************************/

#define UI_HANDLER_CANCEL		((UINT32)~0)

#ifdef UI_COLOR_DISPLAY
#define UI_BOX_LR_BORDER		3
#define UI_BOX_TB_BORDER		3
#else /* UI_COLOR_DISPLAY */
#define UI_BOX_LR_BORDER		(ui_get_char_width('M') / 2)
#define UI_BOX_TB_BORDER		(ui_get_char_width('M') / 2)
#endif /* UI_COLOR_DISPLAY */

#ifdef NEW_RENDER
#define UI_FONT_NAME			NULL
//#define UI_FONT_HEIGHT			(1.0f / 25.0f)
#define UI_FONT_HEIGHT			ui_font_height
#define UI_LINE_WIDTH			(1.0f / (float)ui_screen_height)
#define UI_SCALE_TO_INT_X(x)		((int)((float)(x) * ui_screen_width + 0.5f))
#define UI_SCALE_TO_INT_Y(y)		((int)((float)(y) * ui_screen_height + 0.5f))
#define UI_UNSCALE_TO_FLOAT_X(x)	((float)(x) / (float)ui_screen_width)
#define UI_UNSCALE_TO_FLOAT_Y(y)	((float)(y) / (float)ui_screen_height)
#endif

#ifdef UI_COLOR_DISPLAY
#define UI_SCROLL_TEXT_COLOR		MAX_COLORTABLE
#endif /* UI_COLOR_DISPLAY */



/*************************************
 *
 *  External variables
 *
 *************************************/

#ifdef AUTO_PAUSE_PLAYBACK
extern void *playback;
#endif /* AUTO_PAUSE_PLAYBACK */

#ifdef USE_SHOW_INPUT_LOG
extern int show_input_log;
extern UINT8 command_buffer[COMMAND_LOG_BUFSIZE];
extern int command_counter;
#endif /* USE_SHOW_INPUT_LOG */



/*************************************
 *
 *  Local variables
 *
 *************************************/

#ifdef INP_CAPTION
static int next_caption_frame, caption_timer;
#endif /* INP_CAPTION */

#ifdef NEW_RENDER
static rgb_t uifont_colortable[MAX_COLORTABLE];
static render_texture *bgtexture;
static mame_bitmap *bgbitmap;
#endif

static rgb_t ui_bgcolor;

#ifdef NEW_RENDER
static render_font *ui_font;
static float ui_font_height;

static int ui_screen_width, ui_screen_height;
#endif

/* current UI handler */
static UINT32 (*ui_handler_callback)(UINT32);
static UINT32 ui_handler_param;

#ifndef NEW_RENDER
/* raw coordinates, relative to the real scrbitmap */
static rectangle uirawbounds;

/* rotated coordinates, easier to deal with */
static rectangle uirotbounds;
static int uirotwidth, uirotheight;
static int uirotcharwidth, uirotcharheight;
#endif

static int multiline_text_box_visible_lines;
static int multiline_text_box_target_lines;

static int message_window_scroll;

/* menu states */
static UINT32 menu_state;
static ui_menu_handler menu_handler;

static int menu_stack_index;
static UINT32 menu_stack_state[MENU_STACK_DEPTH];
static ui_menu_handler menu_stack_handler[MENU_STACK_DEPTH];

static UINT32 menu_string_pool_offset;
static char menu_string_pool[MENU_STRING_POOL_SIZE];


static int single_step;
static int auto_pause;
static int scroll_reset;
static int ui_lock_scroll;
static rgb_t text_color;

static int showfps;
static int showfpstemp;

static int show_profiler;

static UINT8 ui_dirty;

static char popup_text[200];
static int popup_text_counter;


static UINT32 onscrd_state;
static void (*onscrd_fnc[MAX_OSD_ITEMS])(int increment,int arg);
static int onscrd_arg[MAX_OSD_ITEMS];
static int onscrd_total_items;


static input_seq starting_seq;


#ifndef NEW_RENDER
/* -- begin this stuff will go away with the new rendering system */
#define MAX_RENDER_ELEMENTS	1000

struct _render_element
{
	int x, y;
	int x2, y2;
	UINT16 type;
	rgb_t color;
};
typedef struct _render_element render_element;

static render_element elemlist[MAX_RENDER_ELEMENTS];
static int elemindex;
/* -- end this stuff will go away with the new rendering system */
#endif

#ifdef USE_SHOW_TIME
static int show_time = 0;
static int Show_Time_Position;
static void display_time(void);
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
static void display_input_log(void);
#endif /* USE_SHOW_INPUT_LOG */



/*************************************
 *
 *  Local prototypes
 *
 *************************************/

static void ui_draw_message_window_scroll(const char *text);
static int ui_window_scroll_keys(void);

static void draw_multiline_text_box(const char *text, int offset, int justify, float xpos, float ypos);
static void create_font(void);
static void onscrd_init(void);

#ifndef NEW_RENDER
static void handle_keys(mame_bitmap *bitmap);
#else
static void handle_keys(void);
#endif
static void ui_display_profiler(void);
static void ui_display_popup(void);
static int setup_menu(int selected);
static void showcharset(mame_bitmap *bitmap);
static void initiate_load_save(int type);
static UINT32 update_load_save(UINT32 state);

static UINT32 menu_main(UINT32 state);
static UINT32 menu_default_input_groups(UINT32 state);
static UINT32 menu_default_input(UINT32 state);
static UINT32 menu_game_input(UINT32 state);
#ifdef USE_CUSTOM_BUTTON
static UINT32 menu_custom_button(UINT32 state);
#endif /* USE_CUSTOM_BUTTON */
static UINT32 menu_autofire(UINT32 state);
static UINT32 menu_switches(UINT32 state);
static UINT32 menu_analog(UINT32 state);
static UINT32 menu_joystick_calibrate(UINT32 state);
static UINT32 menu_documents(UINT32 state);
static UINT32 menu_document_contents(UINT32 state);
#ifdef CMD_LIST
static UINT32 menu_command(UINT32 state);
static UINT32 menu_command_contents(UINT32 state);
#endif /* CMD_LIST */
static UINT32 menu_cheat(UINT32 state);
static UINT32 menu_memory_card(UINT32 state);
static UINT32 menu_video(UINT32 state);
#ifdef USE_SCALE_EFFECTS
static UINT32 menu_scale_effect(UINT32 state);
#endif /* USE_SCALE_EFFECTS */
static UINT32 menu_reset_game(UINT32 state);
static UINT32 menu_game_info(UINT32 state);

#ifndef MESS
static UINT32 menu_bookkeeping(UINT32 state);
#else
static UINT32 menu_file_manager(UINT32 state);
static UINT32 menu_tape_control(UINT32 state);
#endif

static int sprintf_game_info(char *buf);

static UINT32 ui_handler_startup(UINT32 state);
static UINT32 ui_handler_menu(UINT32 state);
static UINT32 ui_handler_osd(UINT32 state);
static UINT32 ui_handler_font_warning(UINT32 state);
static UINT32 ui_handler_disclaimer(UINT32 state);
static UINT32 ui_handler_warnings(UINT32 state);
static UINT32 ui_handler_gameinfo(UINT32 state);
static UINT32 ui_handler_showgfx(UINT32 state);
static UINT32 ui_handler_confirm_quit(UINT32 state);

static void showgfx_exit(void);

#ifndef NEW_RENDER
/* -- begin this stuff will go away with the new rendering system */
static void ui_raw2rot_rect(rectangle *rect);
static void ui_rot2raw_rect(rectangle *rect);
static void add_line(int x1, int y1, int x2, int y2, rgb_t color);
static void add_fill(int left, int top, int right, int bottom, rgb_t color);
static void add_char(int x, int y, UINT16 ch, int color);
static void render_ui(mame_bitmap *dest);
/* -- end this stuff will go away with the new rendering system */
#else
static void build_bgtexture(void);
static void free_bgtexture(void);
INLINE rgb_t ui_get_rgb_color(rgb_t color);

#define add_line(x0,y0,x1,y1,color)	render_ui_add_line(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), UI_LINE_WIDTH, ui_get_rgb_color(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA))
static void add_fill(int x0, int y0, int x1, int y1, rgb_t color);
#define add_char(x,y,ch,color)		render_ui_add_char(UI_UNSCALE_TO_FLOAT_X(x), UI_UNSCALE_TO_FLOAT_Y(y), UI_FONT_HEIGHT, render_get_ui_aspect(), ui_get_rgb_color(color), ui_font, ch)

INLINE void add_outlined_box_fp(float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	float hw = UI_LINE_WIDTH * 0.5f;
	render_ui_add_rect(x0, y0, x1, y1, backcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x0 + hw, y0 + hw, x1 - hw, y0 + hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x1 - hw, y0 + hw, x1 - hw, y1 - hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x1 - hw, y1 - hw, x0 + hw, y1 - hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_ui_add_line(x0 + hw, y1 - hw, x0 + hw, y0 + hw, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


#endif

static void add_filled_box(int x0, int y0, int x1, int y1);
#ifdef USE_SHOW_INPUT_LOG
static void add_filled_box_noedge(int x0, int y0, int x1, int y1);
#endif /* USE_SHOW_INPUT_LOG */



/*************************************
 *
 *  UI handler callbacks
 *
 *************************************/

INLINE void ui_set_handler(UINT32 (*callback)(UINT32), int param)
{
	ui_handler_callback = callback;
	ui_handler_param = param;
}


/*-------------------------------------------------
    erase_screen - erase the screen
-------------------------------------------------*/

INLINE void erase_screen(mame_bitmap *bitmap)
{
#ifndef NEW_RENDER
	fillbitmap(bitmap, get_black_pen(), NULL);
	schedule_full_refresh();
#endif
}




/*************************************
 *
 *  Main initialization
 *
 *************************************/

int ui_init(void)
{
	/* load the localization file */
#if 0
	if (uistring_init(options.language_file) != 0)
#else
	if (uistring_init() != 0)
#endif
		fatalerror("uistring_init failed");

#ifndef NEW_RENDER
	/* build up the font */
	create_font();
#else
	build_bgtexture();

	ui_font = render_font_alloc("ui.bdf");
	if (uifont_need_font_warning())
	{
		options.langcode = UI_LANG_EN_US;
		set_langcode(options.langcode);
		fprintf(stderr, "error: loading local font file\nUse %s\n",
			ui_lang_info[options.langcode].description);

		/* re-load the localization file */
#if 0
		if (uistring_init(options.language_file) != 0)
#else
		if (uistring_init() != 0)
#endif
			fatalerror("uistring_init failed");
	}

	ui_set_visible_area(
		Machine->drv->screen[0].default_visible_area.min_x,
		Machine->drv->screen[0].default_visible_area.min_y,
		Machine->drv->screen[0].default_visible_area.max_x,
		Machine->drv->screen[0].default_visible_area.max_y);

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
		uifont_colortable[UI_TRANSPARENT_COLOR] = MAKE_ARGB(
				options.ui_transparency,
				options.uicolortable[UI_TRANSPARENT_COLOR][0],
				options.uicolortable[UI_TRANSPARENT_COLOR][1],
				options.uicolortable[UI_TRANSPARENT_COLOR][2]);
#endif /* TRANS_UI */
#endif

#ifdef INP_CAPTION
	next_caption_frame = -1;
	caption_timer = 0;
#endif /* INP_CAPTION */

#ifdef TRANS_UI
	ui_bgcolor = UI_TRANSPARENT_COLOR;
#else /* TRANS_UI */
#ifdef UI_COLOR_DISPLAY
	ui_bgcolor = SYSTEM_COLOR_BACKGROUND;
#else /* UI_COLOR_DISPLAY */
#ifdef NEW_RENDER
	ui_bgcolor = MENU_BACKCOLOR;
#else
	ui_bgcolor = ARGB_BLACK;
#endif
#endif /* UI_COLOR_DISPLAY */
#endif /* TRANS_UI */

	text_color = ARGB_WHITE;

	/* initialize the menu state */
	ui_menu_stack_reset();

	/* initialize the on-screen display system */
	//onscrd_init();

	/* reset globals */
	single_step = FALSE;
	ui_set_handler(ui_handler_startup, 0);

	add_exit_callback(ui_exit);
	return 0;
}



/*************************************
 *
 *  Clean up
 *
 *************************************/

void ui_exit(void)
{
	// free the showgfx stuff
	showgfx_exit();

#ifdef NEW_RENDER
	if (ui_font)
		render_font_free(ui_font);
	ui_font = NULL;
#else
	uifont_freefont();
#endif
}



/*************************************
 *
 *  Startup screens
 *
 *************************************/

int ui_display_startup_screens(int show_disclaimer, int show_warnings, int show_gameinfo)
{
#ifndef NEW_RENDER
	mame_bitmap *bitmap = artwork_get_ui_bitmap();
#endif
	int state;

	/* initialize the on-screen display system */
	onscrd_init();

	auto_pause = FALSE;
	scroll_reset = TRUE;
#ifdef USE_SHOW_TIME
	show_time = 0;
	Show_Time_Position = 0;
#endif /* USE_SHOW_TIME */

#ifndef NEW_RENDER
	/* disable artwork for the start */
	artwork_enable(FALSE);

	/* before doing anything else, update the video and audio system once */
	update_video_and_audio();
#endif

	/* loop over states */
	ui_set_handler(NULL, 0);
	for (state = -1; state < 3 && !mame_is_scheduled_event_pending(); state++)
	{
		/* pick the next state */
		switch (state)
		{
			case -1:
				if (uifont_need_font_warning())
					ui_set_handler(ui_handler_font_warning, 0);
				break;

			case 0:
				if (show_disclaimer)
					ui_set_handler(ui_handler_disclaimer, 0);
				break;

			case 1:
				if (show_warnings)
					ui_set_handler(ui_handler_warnings, 0);
				break;

			case 2:
				if (show_gameinfo)
					ui_set_handler(ui_handler_gameinfo, 0);
				break;
		}

		/* clear the input memory */
		while (code_read_async() != CODE_NONE) ;

		/* clear the giant string buffer */
		giant_string_buffer[0] = 0;

		/* loop while we have a handler */
		while (ui_handler_callback != NULL && !mame_is_scheduled_event_pending())
		{
			/* reset the contents of the screen */
#ifndef NEW_RENDER
			erase_screen(bitmap);
#endif

			/* render and update */
#ifndef NEW_RENDER
			render_ui(bitmap);
#endif
			video_frame_update();

			/* call the handler */
			if (ui_handler_param == 1000)
				break;
		}

		/* clear the handler and force an update */
		ui_set_handler(NULL, 0);
		video_frame_update();

		scroll_reset = TRUE;

		if (ui_handler_param == UI_HANDLER_CANCEL)
			return 1;
	}

	/* clear the input memory */
	while (code_read_async() != CODE_NONE) ;

#ifndef NEW_RENDER
	/* enable artwork now */
	artwork_enable(TRUE);
#endif

	return 0;
}



/*************************************
 *
 *  Set the UI visible area
 *  (called by OSD layer, will go
 *  away with new rendering system)
 *
 *************************************/

void ui_set_visible_area(int xmin, int ymin, int xmax, int ymax)
{
#ifndef NEW_RENDER
	/* fill in the rect */
	uirawbounds.min_x = xmin;
	uirawbounds.min_y = ymin;
	uirawbounds.max_x = xmax;
	uirawbounds.max_y = ymax;

	/* orient it */
	uirotbounds = uirawbounds;
	ui_raw2rot_rect(&uirotbounds);

	/* make some easier-to-access globals */
	uirotwidth = uirotbounds.max_x - uirotbounds.min_x + 1;
	uirotheight = uirotbounds.max_y - uirotbounds.min_y + 1;

	/* rebuild the font */
	create_font();
#else
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
#endif
}



/*************************************
 *
 *  Update and rendering frontend
 *
 *************************************/

#ifndef NEW_RENDER
void ui_update_and_render(mame_bitmap *bitmap)
#else
void ui_update_and_render(void)
#endif
{
#ifdef NEW_RENDER
	render_container_empty(render_container_get_ui());

	/* if we're paused, dim the whole screen */
	if (mame_get_phase() >= MAME_PHASE_RESET && mame_is_paused())
	{
		int alpha = (1.0f - options.pause_bright) * 255.0f;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			render_ui_add_rect(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
#endif

	/* first draw the FPS counter and profiler */
	ui_display_fps();
	ui_display_profiler();

	/* call the current UI handler */
	if (ui_handler_callback != NULL)
	{
		ui_handler_param = (*ui_handler_callback)(ui_handler_param);
		if (ui_handler_param == UI_HANDLER_CANCEL)
			ui_set_handler(NULL, UI_HANDLER_CANCEL);
	}

	/* otherwise, we handle non-menu cases */
	else
	{
		/* if we're single-stepping, pause now */
		if (single_step)
		{
			mame_pause(TRUE);
			single_step = FALSE;
		}

#ifndef NEW_RENDER
		handle_keys(bitmap);
#else
		handle_keys();
#endif

		/* then let the cheat engine display its stuff */
		if (options.cheat)
			cheat_display_watches();
	}

	/* finally, display any popup messages */
	ui_display_popup();

#ifdef MESS
	/* let MESS display its stuff */
	mess_ui_update();
#endif

#ifndef NEW_RENDER
	/* flush the UI to the bitmap */
	render_ui(bitmap);

	/* decrement the dirty count */
	if (ui_dirty)
		ui_dirty--;
#endif
}



/*************************************
 *
 *  Accessors for global variables
 *
 *************************************/

int ui_is_dirty(void)
{
	return ui_dirty;
}


int ui_is_onscrd_active(void)
{
	return (ui_handler_callback == ui_handler_osd);
}


int ui_is_setup_active(void)
{
	return (ui_handler_callback == ui_handler_menu);
}



/*************************************
 *
 *  Enable/disable FPS display
 *
 *************************************/

void ui_show_fps_temp(double seconds)
{
	if (!showfps)
		showfpstemp = (int)(seconds * Machine->refresh_rate[0]);
}


void ui_set_show_fps(int show)
{
	showfps = show;
	if (!show)
	{
		showfps = 0;
		showfpstemp = 0;
		schedule_full_refresh();
	}
}


int ui_get_show_fps(void)
{
	return showfps || showfpstemp;
}



/*************************************
 *
 *  Enable/disable profiler display
 *
 *************************************/

void ui_set_show_profiler(int show)
{
	if (show)
	{
		show_profiler = 1;
		profiler_start();
	}
	else
	{
		show_profiler = 0;
		profiler_stop();
		schedule_full_refresh();
	}
}


int ui_get_show_profiler(void)
{
	return show_profiler;
}



/*************************************
 *
 *  Popup message registration
 *
 *************************************/

void CLIB_DECL ui_popup(const char *text,...)
{
	int seconds;
	va_list arg;
	va_start(arg,text);
	vsprintf(popup_text,text,arg);
	va_end(arg);
	seconds = strlen(popup_text) / 40 + 2;
	popup_text_counter = seconds * Machine->refresh_rate[0];
}


void CLIB_DECL ui_popup_time(int seconds, const char *text,...)
{
	va_list arg;
	va_start(arg,text);
	vsprintf(popup_text,text,arg);
	va_end(arg);
	popup_text_counter = seconds * Machine->refresh_rate[0];
}



/*************************************
 *
 *  Simple text renderer
 *
 *************************************/

void ui_draw_text(const char *buf, int x, int y)
{
	int ui_width, ui_height;
	ui_get_bounds(&ui_width, &ui_height);
	ui_draw_text_full(buf, x, y, ui_width - x, 0, 0, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);
}



/*************************************
 *
 *  Full featured text renderer
 *
 *************************************/

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



/*************************************
 *
 *  Menu rendering
 *
 *************************************/

void ui_draw_menu(const ui_menu_item *items, int numitems, int selected)
{
	const char *up_arrow = ui_getstring(UI_uparrow);
	const char *down_arrow = ui_getstring(UI_downarrow);
	const char *left_arrow = ui_getstring(UI_leftarrow);
	const char *right_arrow = ui_getstring(UI_rightarrow);
	const char *left_hilight = ui_getstring(UI_lefthilight);
	const char *right_hilight = ui_getstring(UI_righthilight);

	int left_hilight_width = ui_get_string_width(left_hilight);
	int right_hilight_width = ui_get_string_width(right_hilight);
	int left_arrow_width = ui_get_string_width(left_arrow);
	int right_arrow_width = ui_get_string_width(right_arrow);
	int line_height = ui_get_line_height();
	int gutter_width;

	int effective_width, effective_left;
	int visible_width, visible_height;
	int visible_top, visible_left;
	int ui_width, ui_height;
	int selected_subitem_too_big = 0;
	int visible_lines;
	int top_line;
	int itemnum, linenum;

	/* the left/right gutters are the max of all stuff that might go in there */
	gutter_width = MAX(left_hilight_width, right_hilight_width);
	gutter_width = MAX(gutter_width, left_arrow_width);
	gutter_width = MAX(gutter_width, right_arrow_width);

	/* start with the bounds */
	ui_get_bounds(&ui_width, &ui_height);

	/* compute the width and height of the full menu */
	visible_width = 0;
	visible_height = 0;
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		const ui_menu_item *item = &items[itemnum];
		int total_width;

		/* compute width of left hand side */
		total_width = gutter_width + ui_get_string_width(item->text) + gutter_width;

		/* add in width of right hand side */
		if (item->subtext)
			total_width += 2 * gutter_width + ui_get_string_width(item->subtext);

		/* track the maximum */
		if (total_width > visible_width)
			visible_width = total_width;

		/* track the height as well */
		visible_height += line_height;
	}

	/* if we are too wide or too tall, clamp it down */
	if (visible_width + 2 * UI_BOX_LR_BORDER > ui_width)
		visible_width = ui_width - 2 * UI_BOX_LR_BORDER;
	if (visible_height + 2 * UI_BOX_TB_BORDER > ui_height)
		visible_height = ui_height - 2 * UI_BOX_TB_BORDER;
	visible_lines = visible_height / line_height;
	visible_height = visible_lines * line_height;

	/* compute top/left of inner menu area by centering */
	visible_left = (ui_width - visible_width) / 2;
	visible_top = (ui_height - visible_height) / 2;

	/* first add us a box */
	add_filled_box(	visible_left - UI_BOX_LR_BORDER,
					visible_top - UI_BOX_TB_BORDER,
					visible_left + visible_width - 1 + UI_BOX_LR_BORDER,
					visible_top + visible_height - 1 + UI_BOX_TB_BORDER);

	/* determine the first visible line based on the current selection */
	top_line = selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= numitems)
		top_line = numitems - visible_lines;

	/* determine effective positions taking into account the hilighting arrows */
	effective_width = visible_width - 2 * gutter_width;
	effective_left = visible_left + gutter_width;

	/* loop over visible lines */
	for (linenum = 0; linenum < visible_lines; linenum++)
	{
		int line_y = visible_top + linenum * line_height;
		int itemnum = top_line + linenum;
		const ui_menu_item *item = &items[itemnum];
		rgb_t itemfg = MENU_TEXTCOLOR;

		/* if we're selected, draw with a different background */
		if (itemnum == selected)
#ifdef UI_COLOR_DISPLAY
			add_fill(visible_left, line_y,
			         visible_left + visible_width - 1, line_y + ui_get_line_height() - 1,
			         CURSOR_COLOR);
#else
			itemfg = MENU_SELECTCOLOR;
#endif /* UI_COLOR_DISPLAY */

		/* if we're on the top line, display the up arrow */
		if (linenum == 0 && top_line != 0)
			ui_draw_text_full(up_arrow, effective_left, line_y, effective_width, 0, 1,
						JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);

		/* if we're on the bottom line, display the down arrow */
		else if (linenum == visible_lines - 1 && itemnum != numitems - 1)
			ui_draw_text_full(down_arrow, effective_left, line_y, effective_width, 0, 1,
						JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);

		/* if we don't have a subitem, just draw the string centered */
		else if (!item->subtext)
			ui_draw_text_full(item->text, effective_left, line_y, effective_width, 0, 1,
						JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);

		/* otherwise, draw the item on the left and the subitem text on the right */
		else
		{
			int subitem_invert = item->flags & MENU_FLAG_INVERT;
			const char *subitem_text = item->subtext;
			int item_width, subitem_width;

			rgb_t fgcolor = itemfg;
			rgb_t bgcolor = ARGB_BLACK;

			if (subitem_invert)
			{
#ifdef UI_COLOR_DISPLAY
				fgcolor = FONT_COLOR_SPECIAL;
#else /* UI_COLOR_DISPLAY */
				fgcolor = ARGB_BLACK;
				bgcolor = itemfg;
#endif /* UI_COLOR_DISPLAY */
			}

			/* draw the left-side text */
			ui_draw_text_full(item->text, effective_left, line_y, effective_width, 0, 1,
						JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, &item_width, NULL);

			/* give 2 spaces worth of padding */
			item_width += 2 * gutter_width;

			/* if the subitem doesn't fit here, display dots */
			if (ui_get_string_width(subitem_text) > effective_width - item_width)
			{
				subitem_text = "...";
				if (itemnum == selected)
					selected_subitem_too_big = 1;
			}

			/* draw the subitem right-justified */
			ui_draw_text_full(subitem_text, effective_left + item_width, line_y, effective_width - item_width, 0, 1,
#ifdef UI_COLOR_DISPLAY
						JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, &subitem_width, NULL);
#else /* UI_COLOR_DISPLAY */
						JUSTIFY_RIGHT, WRAP_TRUNCATE, item_invert ? DRAW_OPAQUE : DRAW_NORMAL, fgcolor, bgcolor, &subitem_width, NULL);
#endif /* UI_COLOR_DISPLAY */

			/* apply arrows */
			if (itemnum == selected && (item->flags & MENU_FLAG_LEFT_ARROW))
				ui_draw_text_full(left_arrow, effective_left + effective_width - subitem_width - left_arrow_width, line_y, left_arrow_width, 0, 1,
							JUSTIFY_LEFT, WRAP_NEVER, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);
			if (itemnum == selected && (item->flags & MENU_FLAG_RIGHT_ARROW))
				ui_draw_text_full(right_arrow, visible_left, line_y, visible_width, 0, 1,
							JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);
		}

#ifndef UI_COLOR_DISPLAY
		/* draw the arrows for selected items */
		if (itemnum == selected)
		{
			ui_draw_text_full(left_hilight, visible_left, line_y, visible_width, 0, 1,
						JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);
			if (!(item->flags & (MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW)))
				ui_draw_text_full(right_hilight, visible_left, line_y, visible_width, 0, 1,
							JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, itemfg, ARGB_BLACK, NULL, NULL);
		}
#endif /* !UI_COLOR_DISPLAY */
	}

	/* if the selected subitem is too big, display it in a separate offset box */
	if (selected_subitem_too_big)
	{
		const ui_menu_item *item = &items[selected];
		int linenum = selected - top_line;
		int line_y = visible_top + linenum * line_height;
		int target_width, target_height;
		int target_x, target_y;

		/* compute the multi-line target width/height */
		ui_draw_text_full(item->subtext, 0, 0, visible_width * 3 / 4, 0, 0,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);

		/* determine the target location */
		target_x = visible_left + visible_width - target_width - UI_BOX_LR_BORDER;
		target_y = line_y + line_height + UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > visible_height)
			target_y = line_y - target_height - UI_BOX_TB_BORDER;

		/* add a box around that */
		add_filled_box(	target_x - UI_BOX_LR_BORDER,
						target_y - UI_BOX_TB_BORDER,
						target_x + target_width - 1 + UI_BOX_LR_BORDER,
						target_y + target_height - 1 + UI_BOX_TB_BORDER);
		ui_draw_text_full(item->subtext, target_x, target_y, target_width, 0, target_height / ui_get_line_height(),
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}
}



/*************************************
 *
 *  Menu management
 *
 *************************************/

void ui_menu_stack_reset(void)
{
	menu_handler = NULL;
	menu_state = 0;
	menu_stack_index = 0;
}


UINT32 ui_menu_stack_push(ui_menu_handler new_handler, UINT32 new_state)
{
	if (menu_stack_index >= MENU_STACK_DEPTH)
		fatalerror("Menu stack overflow!");

	/* save the old state/handler */
	menu_stack_handler[menu_stack_index] = menu_handler;
	menu_stack_state[menu_stack_index] = menu_state;
	menu_stack_index++;

	/* set the new ones */
	menu_handler = new_handler;
	menu_state = new_state;

	/* force a refresh */
	schedule_full_refresh();
	return new_state;
}


UINT32 ui_menu_stack_pop(void)
{
	if (menu_stack_index <= 0)
		fatalerror("Menu stack underflow!");

	/* restore the old state/handler */
	menu_stack_index--;
	menu_handler = menu_stack_handler[menu_stack_index];

	/* force a refresh */
	schedule_full_refresh();

	return menu_stack_state[menu_stack_index];
}



/*************************************
 *
 *  Generic menu keys
 *
 *************************************/

int ui_menu_generic_keys(int *selected, int num_items)
{
	static int counter = 0;
	static int fast = 6;
	int ui_width, ui_height;
	int pan_lines;

	ui_get_bounds(&ui_width, &ui_height);
	pan_lines = ((ui_height - 2 * UI_BOX_TB_BORDER) / ui_get_line_height()) - 3;

	/* hitting cancel or selecting the last item returns to the previous menu */
	if (input_ui_pressed(IPT_UI_CANCEL) || (*selected == num_items - 1 && input_ui_pressed(IPT_UI_SELECT)))
	{
		*selected = ui_menu_stack_pop();
		return 1;
	}

	/* up backs up by one item */
	if (input_ui_pressed_repeat(IPT_UI_UP, fast))
		*selected = (*selected + num_items - 1) % num_items;

	/* down advances by one item */
	if (input_ui_pressed_repeat(IPT_UI_DOWN, fast))
		*selected = (*selected + 1) % num_items;

	if (input_port_type_pressed(IPT_UI_UP,0) || input_port_type_pressed(IPT_UI_DOWN,0))
	{
		if (++counter == 25)
		{
			fast--;
			if (fast < 2)
				fast = 2;

			counter = 0;
		}
	}
	else
	{
		fast = 6;
		counter = 0;
	}

	/* pan-up goes to previous page */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_UP,8))
	{
		*selected -= pan_lines;
		if (*selected <0)
			*selected = 0;
	}

	/* pan-down goes to next page */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_DOWN,8))
	{
		*selected += pan_lines;
		if (*selected >= num_items)
			*selected = num_items - 1;
	}

	/* home goes to the start */
	if (input_ui_pressed(IPT_UI_HOME))
		*selected = 0;

	/* end goes to the last */
	if (input_ui_pressed(IPT_UI_END))
		*selected = num_items - 1;

	return 0;
}



/*************************************
 *
 *  Multiline message box rendering
 *
 *************************************/

static void draw_multiline_text_box(const char *text, int offset, int justify, float xpos, float ypos)
{
	int target_width, target_height;
	int ui_width, ui_height;
	int target_x, target_y;
	int margin_x, margin_y;

	/* start with the bounds */
	ui_get_bounds(&ui_width, &ui_height);

	/* compute the multi-line target width/height */
	ui_draw_text_full(text, 0, 0, ui_width - 2 * UI_BOX_LR_BORDER - 2, 0, 0,
				justify, WRAP_WORD, DRAW_NONE, text_color, ARGB_BLACK, &target_width, &target_height);

	multiline_text_box_target_lines = target_height / ui_get_line_height();

#ifdef UI_COLOR_DISPLAY
	margin_x = ui_get_char_width(' ') / 2 + UI_BOX_LR_BORDER;
	if (target_width + 2 * margin_x > ui_width)
		margin_x = (ui_width - target_width) / 2;

	margin_y = ui_get_line_height() / 2 + UI_BOX_TB_BORDER;
#else /* UI_COLOR_DISPLAY */
	margin_x = UI_BOX_LR_BORDER;
	margin_y = UI_BOX_TB_BORDER;
#endif /* UI_COLOR_DISPLAY */

	if (target_height > ui_height - 2 * margin_y)
		target_height = ((ui_height - 2 * margin_y) / ui_get_line_height()) * ui_get_line_height();

	multiline_text_box_visible_lines = target_height / ui_get_line_height();

	/* determine the target location */
	target_x = (int)(xpos * ui_width) - target_width / 2;
	target_y = (int)(ypos * ui_height) - target_height / 2;

	/* make sure we stay on-screen */
	if (target_x < margin_x)
		target_x = margin_x;
	if (target_x + target_width + margin_x > ui_width)
		target_x = ui_width - margin_x - target_width;
	if (target_y < margin_y)
		target_y = margin_y;
	if (target_y + target_height + margin_y > ui_height)
		target_y = ui_height - margin_y - target_height;

	/* add a box around that */
	add_filled_box(target_x - margin_x,
	               target_y - margin_y,
	               target_x + target_width - 1 + margin_x,
	               target_y + target_height - 1 + margin_y);

	ui_draw_text_full(text, target_x, target_y, target_width, offset, multiline_text_box_visible_lines,
				justify, WRAP_WORD, DRAW_NORMAL, text_color, ARGB_BLACK, NULL, NULL);
}


void ui_draw_message_window(const char *text)
{
	draw_multiline_text_box(text, 0, JUSTIFY_LEFT, 0.5, 0.5);
}


#ifdef INP_CAPTION
static void ui_draw_message_window_under(const char *text)
{
	draw_multiline_text_box(text, 0, JUSTIFY_LEFT, 0.5, 1.0);
}
#endif /* INP_CAPTION */


static void ui_draw_message_window_scroll(const char *text)
{
	text_color = UI_SCROLL_TEXT_COLOR;
	draw_multiline_text_box(text, message_window_scroll, JUSTIFY_LEFT, 0.5, 0.5);
	text_color = ARGB_WHITE;
}


static int ui_window_scroll_keys(void)
{
	static int counter = 0;
	static int fast = 6;
	int pan_lines;
	int max_scroll;

	max_scroll = multiline_text_box_target_lines - multiline_text_box_visible_lines;
	pan_lines = multiline_text_box_visible_lines - 1;

	if (scroll_reset)
	{
		message_window_scroll = 0;
		scroll_reset = 0;
		ui_lock_scroll = FALSE;
	}

	if (!ui_lock_scroll)
	{
		int do_scroll = FALSE;

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
	}

	if (input_ui_pressed(IPT_UI_TOGGLE_LOCK_SCROLL))
		ui_lock_scroll = !ui_lock_scroll;

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
	int ui_width, ui_height;

	ui_get_bounds(&ui_width, &ui_height);

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
			ui_draw_text(buf, ui_width - width, ui_height - ui_get_line_height());
			break;

		case 1:
			ui_draw_text(buf, ui_width - width, 0);
			break;

		case 2:
			ui_draw_text(buf, 0, 0);
			break;

		case 3:
			ui_draw_text(buf, 0, ui_height - ui_get_line_height());
			break;
	}
}
#endif /* USE_SHOW_TIME */

#ifdef USE_SHOW_INPUT_LOG
static void display_input_log(void)
{
	int ui_width, ui_height;

	ui_get_bounds(&ui_width, &ui_height);

	add_filled_box_noedge(0, ui_height - ui_get_line_height(), ui_width - 1, ui_height - 1);
	ui_draw_text(command_buffer, 0, ui_height - ui_get_line_height());

	if (--command_counter == 0) {
		schedule_full_refresh();
		memset(command_buffer, 0, COMMAND_LOG_BUFSIZE);
	}
}
#endif /* USE_SHOW_INPUT_LOG */




/*************************************
 *
 *  Create the UI font
 *
 *************************************/

static void create_font(void)
{
#ifndef NEW_RENDER
	uifont_buildfont(&uirotcharwidth, &uirotcharheight);
#endif
}



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
				next_caption_timer = 5 * Machine->refresh_rate[0];	// 5sec.
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
		ui_draw_message_window_under(caption_text);
		caption_timer--;
	}
}
#endif /* INP_CAPTION */


/*************************************
 *
 *  Keyboard handling
 *
 *************************************/

#ifndef NEW_RENDER
static void handle_keys(mame_bitmap *bitmap)
#else
static void handle_keys(void)
#endif
{
	int is_paused = mame_is_paused();

#ifdef MESS
	if (options.disable_normal_ui || ((Machine->gamedrv->flags & GAME_COMPUTER) && !mess_ui_active()))
		return;
#endif

	/* if the user pressed ESC, stop the emulation */
	if (input_ui_pressed(IPT_UI_CANCEL))
		ui_set_handler(ui_handler_confirm_quit, 0);

	/* turn on menus if requested */
	if (input_ui_pressed(IPT_UI_CONFIGURE))
		ui_set_handler(ui_handler_menu, 0);

	if (options.cheat && input_ui_pressed(IPT_UI_CHEAT))
		ui_set_handler(ui_handler_menu, SHORTCUT_MENU_CHEAT);

#ifdef CMD_LIST
	if (input_ui_pressed(IPT_UI_COMMAND))
		ui_set_handler(ui_handler_menu, SHORTCUT_MENU_COMMAND);
#endif /* CMD_LIST */

	/* if the on-screen display isn't up and the user has toggled it, turn it on */
#ifdef MAME_DEBUG
	if (!Machine->debug_mode)
#endif
		if (input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY))
			ui_set_handler(ui_handler_osd, 0);

	/* handle a reset request */
	if (input_ui_pressed(IPT_UI_RESET_MACHINE))
		mame_schedule_hard_reset();
	if (input_ui_pressed(IPT_UI_SOFT_RESET))
		mame_schedule_soft_reset();

	/* handle a request to display graphics/palette */
#ifndef NEW_RENDER
	if (input_ui_pressed(IPT_UI_SHOW_GFX))
	{
		osd_sound_enable(0);
		showcharset(bitmap);
		osd_sound_enable(1);
	}
#else
	if (input_ui_pressed(IPT_UI_SHOW_GFX))
	{
		if (!is_paused)
			mame_pause(TRUE);
		ui_set_handler(ui_handler_showgfx, is_paused);
	}
#endif

	/* handle a save state request */
	if (input_ui_pressed(IPT_UI_SAVE_STATE))
		initiate_load_save(LOADSAVE_SAVE);

	/* handle a load state request */
	if (input_ui_pressed(IPT_UI_LOAD_STATE))
		initiate_load_save(LOADSAVE_LOAD);

	/* handle a save snapshot request */
	if (input_ui_pressed(IPT_UI_SNAPSHOT))
		snapshot_save_all_screens();

#ifdef INP_CAPTION
	draw_caption();
#endif /* INP_CAPTION */

	/* toggle pause */
	if (auto_pause || input_ui_pressed(IPT_UI_PAUSE))
	{
		auto_pause = FALSE;

		/* with a shift key, it is single step */
		if (is_paused && (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT)))
		{
			single_step = TRUE;
			mame_pause(FALSE);
		}
		else
			mame_pause(!mame_is_paused());
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

		schedule_full_refresh();
		memset(command_buffer, 0, COMMAND_LOG_BUFSIZE);
	}

	/* show popup message if input exist any log */
	if (show_input_log && command_counter)
		display_input_log();
#endif /* USE_SHOW_INPUT_LOG */

	/* toggle movie recording */
	if (input_ui_pressed(IPT_UI_RECORD_MOVIE))
		record_movie_toggle();

	/* toggle profiler display */
	if (input_ui_pressed(IPT_UI_SHOW_PROFILER))
		ui_set_show_profiler(!ui_get_show_profiler());

	/* toggle FPS display */
	if (input_ui_pressed(IPT_UI_SHOW_FPS))
		ui_set_show_fps(!ui_get_show_fps());

	/* toggle crosshair display */
	if (input_ui_pressed(IPT_UI_TOGGLE_CROSSHAIR))
		drawgfx_toggle_crosshair();
}



/*************************************
 *
 *  Initial startup handler
 *
 *************************************/

static char startup_text[1024];

void ui_set_startup_text(const char *text, int force)
{
	static cycles_t lastupdatetime = 0;
	cycles_t curtime = osd_cycles();

	strncpy(startup_text, text, sizeof(startup_text));

	/* don't update more than 4 times/second */
	if (force || (curtime - lastupdatetime) > osd_cycles_per_second() / 4)
	{
		lastupdatetime = curtime;
		video_frame_update();
	}
}


static UINT32 ui_handler_startup(UINT32 state)
{
	draw_multiline_text_box(startup_text, 0, JUSTIFY_CENTER, 0.5, 0.5);
	return 0;
}



/*************************************
 *
 *  Menu handler
 *
 *************************************/

static UINT32 ui_handler_menu(UINT32 state)
{
	if (state == SHORTCUT_MENU_CHEAT)
	{
		if (menu_handler != menu_cheat)
			ui_menu_stack_reset();

		if (((menu_state >> 31) & 1) == 0)
			ui_menu_stack_reset();

		/* if we have no menus stacked up, start with the cheat menu */
		if (menu_handler == NULL)
			ui_menu_stack_push(menu_cheat, (1 << 31) | (1 << 30) | (1 << 8) | 1);
	}
	else
	{
		if (menu_handler == menu_cheat)
		{
			if (((menu_state >> 31) & 1) != 0)
				ui_menu_stack_reset();
		}
	}

#ifdef CMD_LIST
	if (state == SHORTCUT_MENU_COMMAND)
	{
		if (menu_handler != menu_command && menu_handler != menu_command_contents)
			ui_menu_stack_reset();

		if ((menu_state >> 24) == 0)
			ui_menu_stack_reset();

		/* if we have no menus stacked up, start with the command menu */
		if (menu_handler == NULL)
			ui_menu_stack_push(menu_command, 1 << 24);
	}
	else
	{
		if (menu_handler == menu_command || menu_handler == menu_command_contents)
		{
			if ((menu_state >> 24) != 0)
				ui_menu_stack_reset();
		}
	}
#endif /* CMD_LIST */

	/* if we have no menus stacked up, start with the main menu */
	if (menu_handler == NULL)
		ui_menu_stack_push(menu_main, 0);

	/* update the menu state */
	menu_state = (*menu_handler)(menu_state);

	/* if the menus are to be hidden, return a cancel here */
	if (state == SHORTCUT_MENU_CHEAT)
	{
		if (input_ui_pressed(IPT_UI_CHEAT) || menu_handler == NULL)
			return UI_HANDLER_CANCEL;
	}
	else
#ifdef CMD_LIST
	if (state == SHORTCUT_MENU_COMMAND)
	{
		if (input_ui_pressed(IPT_UI_COMMAND) || menu_handler == NULL)
			return UI_HANDLER_CANCEL;
	}
	else
#endif /* CMD_LIST */
	if (input_ui_pressed(IPT_UI_CONFIGURE) || menu_handler == NULL)
		return UI_HANDLER_CANCEL;

	return state;
}



/*************************************
 *
 *  Main menu
 *
 *************************************/

static UINT32 menu_main(UINT32 state)
{
#define ADD_MENU(name, handler, param) \
do { \
	item_list[menu_items].text = ui_getstring(name); \
	handler_list[menu_items] = handler; \
	param_list[menu_items] = param; \
	menu_items++; \
} while (0)

	ui_menu_handler handler_list[20];
	ui_menu_item item_list[20];
	UINT32 param_list[20];
	int has_categories = FALSE;
	int has_configs = FALSE;
	int has_analog = FALSE;
	int has_dips = FALSE;
	input_port_entry *in;
	int menu_items = 0;

	/* scan the input port array to see what options we need to enable */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (input_port_active(in))
		{
			if (in->type == IPT_DIPSWITCH_NAME)
				has_dips = TRUE;
			if (port_type_is_analog(in->type))
				has_analog = TRUE;
			if (in->type == IPT_CONFIG_NAME)
				has_configs = TRUE;
			if (in->category > 0)
				has_categories = TRUE;
		}

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* add default input item */
	ADD_MENU(UI_inputgeneral, menu_default_input_groups, 0);

	/* add game-specific input item */
	ADD_MENU(UI_inputspecific, menu_game_input, 0);

#ifdef USE_CUSTOM_BUTTON
	if (custom_buttons)
		ADD_MENU(UI_custombuttons, menu_custom_button, 0);
#endif /* USE_CUSTOM_BUTTON */

	ADD_MENU(UI_autofire, menu_autofire, 0);

	/* add DIP switches menu */
	if (has_dips)
		ADD_MENU(UI_dipswitches, menu_switches, (IPT_DIPSWITCH_NAME << 16) | (IPT_DIPSWITCH_SETTING << 24));

#ifdef MESS
	/* add configurables menu */
	if (has_configs)
		ADD_MENU(UI_configuration, menu_switches, (IPT_CONFIG_NAME << 16) | (IPT_CONFIG_SETTING << 24));

	/* add categories menu */
	if (has_categories)
		ADD_MENU(UI_categories, menu_switches, (IPT_CATEGORY_NAME << 16) | (IPT_CATEGORY_SETTING << 24));
#endif /* MESS */

	/* add analog settings menu */
	if (has_analog)
		ADD_MENU(UI_analogcontrols, menu_analog, 0);

  	/* add joystick calibration menu */
  	if (osd_joystick_needs_calibration())
		ADD_MENU(UI_calibrate, menu_joystick_calibrate, 0);

#ifndef MESS
  	/* add bookkeeping menu */
	ADD_MENU(UI_bookkeeping, menu_bookkeeping, 0);
#endif

	/* add game info menu */
	ADD_MENU(UI_gameinfo, menu_game_info, 0);

#ifdef MESS
  	/* add image info menu */
	ADD_MENU(UI_imageinfo, ui_menu_image_info, 0);

  	/* add image info menu */
	ADD_MENU(UI_filemanager, menu_file_manager, 1);

#if HAS_WAVE
  	/* add tape control menu */
	if (device_find(Machine->devices, IO_CASSETTE))
		ADD_MENU(UI_tapecontrol, menu_tape_control, 1);
#endif /* HAS_WAVE */
#endif /* MESS */

  	/* add game document menu */
	ADD_MENU(UI_gamedocuments, menu_documents, 0);

#ifdef NEW_RENDER
	/* add video options menu */
	ADD_MENU(UI_video, menu_video, 1000 << 16);
#endif

#ifdef USE_SCALE_EFFECTS
	ADD_MENU(UI_scaleeffect, menu_scale_effect, scale_effect.effect);
#endif /* USE_SCALE_EFFECTS */

	/* add cheat menu */
	if (options.cheat)
		ADD_MENU(UI_cheat, menu_cheat, 1);

	/* add memory card menu */
	if (Machine->drv->memcard_handler != NULL)
		ADD_MENU(UI_memorycard, menu_memory_card, 0);

	/* add reset and exit menus */
	ADD_MENU(UI_resetgame, menu_reset_game, 0);
	ADD_MENU(UI_returntogame, NULL, 0);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, state);

	/* handle the keys */
	if (ui_menu_generic_keys((int *) &state, menu_items))
		return state;
	if (input_ui_pressed(IPT_UI_SELECT))
		return ui_menu_stack_push(handler_list[menu_state], param_list[menu_state]);

	return state;

#undef ADD_MENU
}



/*************************************
 *
 *  Default input groups menu
 *
 *************************************/

static UINT32 menu_default_input_groups(UINT32 state)
{
	ui_menu_item item_list[IPG_TOTAL_GROUPS + 2];
	int menu_items = 0;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* build up the menu */
	for (menu_items = 0; menu_items < IPG_TOTAL_GROUPS; menu_items++)
		item_list[menu_items].text = ui_getstring(UI_uigroup + menu_items);

	/* add an item for the return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, state);

	/* handle the keys */
	if (ui_menu_generic_keys((int *) &state, menu_items))
		return state;
	if (input_ui_pressed(IPT_UI_SELECT))
		return ui_menu_stack_push(menu_default_input, state << 16);

	return state;
}



/*************************************
 *
 *  Common input menu routines
 *
 *************************************/

INLINE int input_menu_update_polling(input_seq *selected_seq, int *record_next, int *polling)
{
	int result = seq_read_async(selected_seq, !*record_next);

	/* continue polling only if we get a negative result */
	*polling = (result < 0);

	/* a zero result means we're done with the sequence; indicate we can append to it */
	if (result == 0)
		*record_next = TRUE;

	/* a positive result means the user cancelled */
	else if (result > 0)
	{
		*record_next = FALSE;
		return TRUE;
	}

	/* return FALSE if the user didn't cancel */
	return FALSE;
}


INLINE void input_menu_toggle_none_default(input_seq *selected_seq, input_seq *original_seq, const input_seq *selected_defseq)
{
	/* if we used to be "none", toggle to the default value */
	if (seq_get_1(original_seq) == CODE_NONE)
		seq_copy(selected_seq, selected_defseq);

	/* otherwise, toggle to "none" */
	else
		seq_set_1(selected_seq, CODE_NONE);
}



/*************************************
 *
 *  Default input settings menu
 *
 *************************************/

INLINE void default_input_menu_add_item(ui_menu_item *item, const char *format, const char *name, const input_seq *seq, const input_seq *defseq)
{
	/* set the item text using the formatting string provided */
	item->text = &menu_string_pool[menu_string_pool_offset];
	menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], format, name) + 1;

	/* set the subitem text from the sequence */
	item->subtext = &menu_string_pool[menu_string_pool_offset];
	seq_name(seq, &menu_string_pool[menu_string_pool_offset], sizeof(menu_string_pool) - menu_string_pool_offset);
	menu_string_pool_offset += strlen(&menu_string_pool[menu_string_pool_offset]) + 1;

	/* invert if different from the default */
	if (seq_cmp(seq, defseq))
		item->flags |= MENU_FLAG_INVERT;
}


static UINT32 menu_default_input(UINT32 state)
{
	static input_seq starting_seq;

	ui_menu_item item_list[MAX_INPUT_PORTS * MAX_BITS_PER_PORT];
	const input_port_default_entry *indef;
	input_port_default_entry *in;
	const input_seq *selected_defseq = NULL;
	input_seq *selected_seq = NULL;
	UINT8 selected_is_analog = FALSE;
	int selected = state & 0x3fff;
	int record_next = (state >> 14) & 1;
	int polling = (state >> 15) & 1;
	int group = state >> 16;
	int reset_selected = FALSE;
	int menu_items = 0;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* iterate over the input ports and add menu items */
	for (in = get_input_port_list(), indef = get_input_port_list_defaults(); in->type != IPT_END; in++, indef++)

		/* add if we match the group and we have a valid name */
		if (in->group == group && in->name && in->name[0] != 0)
		{
			/* if not analog, just add a standard entry for this item */
			if (!port_type_is_analog(in->type))
			{
				if (menu_items == selected)
				{
					selected_seq = &in->defaultseq;
					selected_defseq = &indef->defaultseq;
				}
				default_input_menu_add_item(&item_list[menu_items++], "%s", _(in->name), &in->defaultseq, &indef->defaultseq);
			}

			/* if we are analog, add three items */
			else
			{
				if (menu_items == selected)
				{
					selected_seq = &in->defaultseq;
					selected_defseq = &indef->defaultseq;
					selected_is_analog = TRUE;
				}
				default_input_menu_add_item(&item_list[menu_items++], _("%s Analog"), _(in->name), &in->defaultseq, &indef->defaultseq);

				if (menu_items == selected)
				{
					selected_seq = &in->defaultdecseq;
					selected_defseq = &indef->defaultdecseq;
				}
				default_input_menu_add_item(&item_list[menu_items++], _("%s Dec"), _(in->name), &in->defaultdecseq, &indef->defaultdecseq);

				if (menu_items == selected)
				{
					selected_seq = &in->defaultincseq;
					selected_defseq = &indef->defaultincseq;
				}
				default_input_menu_add_item(&item_list[menu_items++], _("%s Inc"), _(in->name), &in->defaultincseq, &indef->defaultincseq);
			}
		}

	/* if we're polling, just put an empty entry and arrows for the subitem */
	if (polling)
	{
		item_list[selected].subtext = "   ";
		item_list[selected].flags = MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	}

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntogroup);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* if we're polling, read the sequence */
	if (polling)
	{
		if (input_menu_update_polling(selected_seq, &record_next, &polling))
			input_menu_toggle_none_default(selected_seq, &starting_seq, selected_defseq);
	}

	/* otherwise, handle the keys */
	else
	{
		int prevsel = selected;

		/* handle generic menu keys */
		if (ui_menu_generic_keys(&selected, menu_items))
			return selected;

		/* if an item was selected, start polling on it */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			seq_read_async_start(selected_is_analog);
			seq_copy(&starting_seq, selected_seq);
			polling = TRUE;
		}

		/* if the clear key was pressed, reset the selected item */
		if (input_ui_pressed(IPT_UI_CLEAR))
		{
			input_menu_toggle_none_default(selected_seq, selected_seq, selected_defseq);
			record_next = FALSE;
		}

		/* if the selection changed, update and reset the "record first" flag */
		if (selected != prevsel)
			record_next = FALSE;
	}

	/* if we are to reset ourselves, do it now */
	if (reset_selected && selected != menu_items - 1)
	{
		input_menu_toggle_none_default(selected_seq, &starting_seq, selected_defseq);
		record_next = FALSE;
	}

	return selected | (record_next << 14) | (polling << 15) | (group << 16);
}



/*************************************
 *
 *  Game-specific input settings menu
 *
 *************************************/

INLINE void game_input_menu_add_item(ui_menu_item *item, const char *format, input_port_entry *in, void *ref, int which)
{
	/* set the item text using the formatting string provided */
	item->text = &menu_string_pool[menu_string_pool_offset];
	menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], format, _(input_port_name(in))) + 1;

	/* set the subitem text from the sequence */
	item->subtext = &menu_string_pool[menu_string_pool_offset];
	seq_name(input_port_seq(in, which), &menu_string_pool[menu_string_pool_offset], sizeof(menu_string_pool) - menu_string_pool_offset);
	menu_string_pool_offset += strlen(&menu_string_pool[menu_string_pool_offset]) + 1;

	/* invert if different from the default */
	if (seq_cmp(input_port_seq(in, which), input_port_default_seq(in->type, in->player, which)))
		item->flags |= MENU_FLAG_INVERT;

	/* keep the sequence pointer as a ref and OR in our extra flags */
	item->ref = ref;
}


INLINE UINT16 compute_port_sort_order(const input_port_entry *in)
{
	if (in->type >= IPT_START1 && in->type <= __ipt_analog_end)
		return (in->type << 2) | (in->player << 12);
	return in->type | 0xf000;
}


static int compare_game_inputs(const void *i1, const void *i2)
{
	const ui_menu_item *item1 = i1;
	const ui_menu_item *item2 = i2;
	const input_item_data *data1 = item1->ref;
	const input_item_data *data2 = item2->ref;
	return (data1->sortorder < data2->sortorder) ? -1 : (data1->sortorder > data2->sortorder) ? 1 : 0;
}


static UINT32 menu_game_input(UINT32 state)
{
	static const input_seq default_seq = SEQ_DEF_1(CODE_DEFAULT);

	ui_menu_item item_list[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	input_item_data item_data[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	input_item_data *selected_item_data;
	int selected = state & 0x3fff;
	int record_next = (state >> 14) & 1;
	int polling = (state >> 15) & 1;
	input_port_entry *in;
	int menu_items = 0;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* iterate over the input ports and add menu items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)

		/* add if we match the group and we have a valid name */
		if (input_port_name(in) != NULL &&
#ifdef MESS
			(in->category == 0 || input_category_active(in->category)) &&
#endif /* MESS */
			((in->type == IPT_OTHER && in->name != IP_NAME_DEFAULT) || port_type_to_group(in->type, in->player) != IPG_INVALID))
		{
			/* if not analog, just add a standard entry for this item */
			if (!port_type_is_analog(in->type))
			{
				item_data[menu_items].seq = &in->seq;
				item_data[menu_items].sortorder = compute_port_sort_order(in);
				item_data[menu_items].digital = TRUE;
				game_input_menu_add_item(&item_list[menu_items], "%s", in, &item_data[menu_items], SEQ_TYPE_STANDARD);
				menu_items++;
			}

			/* if we are analog, add three items */
			else
			{
				item_data[menu_items].seq = &in->seq;
				item_data[menu_items].sortorder = compute_port_sort_order(in);
				item_data[menu_items].digital = FALSE;
				game_input_menu_add_item(&item_list[menu_items], _("%s Analog"), in, &item_data[menu_items], SEQ_TYPE_STANDARD);
				menu_items++;

				item_data[menu_items].seq = &in->analog.decseq;
				item_data[menu_items].sortorder = compute_port_sort_order(in) | 1;
				item_data[menu_items].digital = TRUE;
				game_input_menu_add_item(&item_list[menu_items], _("%s Dec"), in, &item_data[menu_items], SEQ_TYPE_DECREMENT);
				menu_items++;

				item_data[menu_items].seq = &in->analog.incseq;
				item_data[menu_items].sortorder = compute_port_sort_order(in) | 2;
				item_data[menu_items].digital = TRUE;
				game_input_menu_add_item(&item_list[menu_items], _("%s Inc"), in, &item_data[menu_items], SEQ_TYPE_INCREMENT);
				menu_items++;
			}
		}

	/* sort the list canonically */
	qsort(item_list, menu_items, sizeof(item_list[0]), compare_game_inputs);

	/* if we're polling, just put an empty entry and arrows for the subitem */
	if (polling)
	{
		item_list[selected].subtext = "   ";
		item_list[selected].flags = MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	}

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* if we're polling, read the sequence */
	selected_item_data = item_list[selected].ref;
	if (polling)
	{
		if (input_menu_update_polling(selected_item_data->seq, &record_next, &polling))
			input_menu_toggle_none_default(selected_item_data->seq, &starting_seq, &default_seq);
	}

	/* otherwise, handle the keys */
	else
	{
		int prevsel = selected;

		/* handle generic menu keys */
		if (ui_menu_generic_keys(&selected, menu_items))
			return selected;

		/* if an item was selected, start polling on it */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			seq_read_async_start(!selected_item_data->digital);
			seq_copy(&starting_seq, selected_item_data->seq);
			polling = TRUE;
		}

		/* if the clear key was pressed, reset the selected item */
		if (input_ui_pressed(IPT_UI_CLEAR))
		{
			input_menu_toggle_none_default(selected_item_data->seq, selected_item_data->seq, &default_seq);
			record_next = FALSE;
		}

		/* if the selection changed, update and reset the "record first" flag */
		if (selected != prevsel)
			record_next = FALSE;
	}

	return selected | (record_next << 14) | (polling << 15);
}



#ifdef USE_CUSTOM_BUTTON
static UINT32 menu_custom_button(UINT32 state)
{
	ui_menu_item item_list[MAX_PLAYERS * MAX_CUSTOM_BUTTONS + 2];
	int selected = state;
	int menu_items = 0;
	UINT16 *custom_item[MAX_PLAYERS * MAX_CUSTOM_BUTTONS];
	int is_neogeo = !mame_stricmp(Machine->gamedrv->source_file+12, "neogeo.c");
	int buttons = 0;
	input_port_entry *in;
	int i;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	if (custom_buttons == 0 || Machine->input_ports == 0)
		return ui_menu_stack_pop();

	memset(custom_item, 0, sizeof custom_item);

	/* iterate over the input ports and add menu items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		int b, p;

		p = in->player;
		b = in->type;

		if (b >= IPT_BUTTON1 && b < IPT_BUTTON1 + MAX_NORMAL_BUTTONS)
		{
			b -= IPT_BUTTON1;
			if (b >= buttons)
				buttons = b + 1;
			continue;
		}

		if (b >= IPT_CUSTOM1 && b < IPT_CUSTOM1 + custom_buttons)
		{
			char colorbutton = is_neogeo ? 'A' : 'a';
			char *s = &menu_string_pool[menu_string_pool_offset];
			int n = 1;

			item_list[menu_items].text = _(input_port_name(in));

			b -= IPT_CUSTOM1;
			custom_item[menu_items] = &custom_button[p][b];

			for (i = 0; i < MAX_NORMAL_BUTTONS; i++, n <<= 1)
				if (*custom_item[menu_items] & n)
				{
					if (&menu_string_pool[menu_string_pool_offset] != s)
					{
						menu_string_pool[menu_string_pool_offset++] = '_';
						menu_string_pool[menu_string_pool_offset++] = '+';
					}

					menu_string_pool[menu_string_pool_offset++] = '_';
					menu_string_pool[menu_string_pool_offset++] = colorbutton + i;
				}

			menu_string_pool[menu_string_pool_offset++] = '\0';

			convert_command_move(s);
			item_list[menu_items++].subtext = s;
		}
	}

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* handle generic menu keys */
	if (ui_menu_generic_keys(&selected, menu_items))
		return selected;

	if (selected != menu_items - 1)
		for (i = 0; i < buttons; i++)
		{
			int keycode = KEYCODE_1 + i;

			if (i == 9)
				keycode = KEYCODE_0;

			if (code_pressed_memory(keycode))
				*custom_item[selected] ^= 1 << i;
		}

	return selected;
}
#endif /* USE_CUSTOM_BUTTON */


static UINT32 menu_autofire(UINT32 state)
{
	ui_menu_item item_list[200];
	int selected = state;
	int menu_items = 0;
	input_port_entry *entry[200];
	input_port_entry *in;
	int autofire_delay;
	int players = 0;
	int i;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	if (Machine->input_ports == 0)
		return ui_menu_stack_pop();

	memset(entry, 0, sizeof(entry));

	/* iterate over the input ports and add menu items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		int type = in->type;

		if (input_port_name(in)	&& (
		    (type >= IPT_BUTTON1 && type < IPT_BUTTON1 + MAX_NORMAL_BUTTONS)
#ifdef USE_CUSTOM_BUTTON
		    || (type >= IPT_CUSTOM1 && type < IPT_CUSTOM1 + MAX_CUSTOM_BUTTONS)
#endif /* USE_CUSTOM_BUTTON */
		   ))
		{
			int value = in->autofire_setting;

			entry[menu_items] = in;

			if (players < in->player + 1)
				players = in->player + 1;

			item_list[menu_items].text = _(input_port_name(in));
			item_list[menu_items++].subtext = ui_getstring(UI_autofireoff + value);
		}
	}

	if (menu_items == 0)
		return ui_menu_stack_pop();

	autofire_delay = menu_items;

	for (i = 0; i < players; i++)
	{
		item_list[menu_items].text = &menu_string_pool[menu_string_pool_offset];
		menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "P%d %s", i + 1, ui_getstring(UI_autofiredelay)) + 1;
		item_list[menu_items++].subtext = &menu_string_pool[menu_string_pool_offset];
		menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%d", options.autofiredelay[i]) + 1;
	}

	item_list[menu_items++].text = ui_getstring (UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	if (ui_menu_generic_keys(&selected, menu_items))
		return selected;

	if (selected >= autofire_delay && selected < autofire_delay + players)
	{
		i = selected - autofire_delay;

		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		{
			options.autofiredelay[i]++;
			if (options.autofiredelay[i] > 99)
				options.autofiredelay[i] = 99;
		}
		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		{
			options.autofiredelay[i]--;
			if (options.autofiredelay[i] < 1)
				options.autofiredelay[i] = 1;
		}
	}
	else if (selected < autofire_delay)
	{
		int selected_value = entry[selected]->autofire_setting;

		if (input_ui_pressed_repeat(IPT_UI_RIGHT,8))
		{
			if (++selected_value > 2)
				selected_value = 0;
		}
		if (input_ui_pressed_repeat(IPT_UI_LEFT,8))
		{
			if (--selected_value < 0)
				selected_value = 2;
		}

		entry[selected]->autofire_setting = selected_value;
	}

	return selected;
}


/*************************************
 *
 *  DIP switch/configurations/categories
 *  settings menu
 *
 *************************************/

INLINE void switch_menu_add_item(ui_menu_item *item, const input_port_entry *in, int switch_entry, void *ref)
{
	const input_port_entry *tin;

	/* set the text to the name and the subitem text to invalid */
	item->text = _(input_port_name(in));
	item->subtext = NULL;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if this is a match, set the subtext */
			if (in->default_value == tin->default_value)
				item->subtext = _(input_port_name(tin));

			/* else if we haven't seen a match yet, show a left arrow */
			else if (!item->subtext)
				item->flags |= MENU_FLAG_LEFT_ARROW;

			/* else if we have seen a match, show a right arrow */
			else
				item->flags |= MENU_FLAG_RIGHT_ARROW;
		}

	/* if no matches, we're invalid */
	if (!item->subtext)
		item->subtext = ui_getstring(UI_INVALID);

	/* stash our reference */
	item->ref = ref;
}


static void switch_menu_pick_previous(input_port_entry *in, int switch_entry)
{
	int last_value = in->default_value;
	const input_port_entry *tin;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if this is a match, we're done */
			if (in->default_value == tin->default_value)
			{
				in->default_value = last_value;
				return;
			}

			/* otherwise, keep track of last one found */
			else
				last_value = tin->default_value;
		}
}


static void switch_menu_pick_next(input_port_entry *in, int switch_entry)
{
	const input_port_entry *tin;
	int foundit = FALSE;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if we found the current selection, we pick the next one */
			if (foundit)
			{
				in->default_value = tin->default_value;
				return;
			}

			/* if this is a match, note it */
			if (in->default_value == tin->default_value)
				foundit = TRUE;
		}
}


/*
static int compare_switch_inputs(const void *i1, const void *i2)
{
    const ui_menu_item *item1 = i1;
    const ui_menu_item *item2 = i2;
    const input_port_entry *data1 = item1->ref;
    const input_port_entry *data2 = item2->ref;
    return strcmp(input_port_name(data1), input_port_name(data2));
}
*/


static UINT32 menu_switches(UINT32 state)
{
	ui_menu_item item_list[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	int switch_entry = (state >> 24) & 0xff;
	int switch_name = (state >> 16) & 0xff;
	int selected = state & 0xffff;
	input_port_entry *selected_in = NULL;
	input_port_entry *in;
	int menu_items = 0;
	int changed = FALSE;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* loop over input ports and set up the current values */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (in->type == switch_name && input_port_active(in) && input_port_condition(in))
			switch_menu_add_item(&item_list[menu_items++], in, switch_entry, in);

	/* sort the list */
//  qsort(item_list, menu_items, sizeof(item_list[0]), compare_switch_inputs);
	selected_in = item_list[selected].ref;

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* handle generic menu keys */
	if (ui_menu_generic_keys(&selected, menu_items))
		return selected;

	/* handle left/right arrows */
	if (input_ui_pressed(IPT_UI_LEFT) && (item_list[selected].flags & MENU_FLAG_LEFT_ARROW))
	{
		switch_menu_pick_previous(selected_in, switch_entry);
		changed = TRUE;
	}
	if (input_ui_pressed(IPT_UI_RIGHT) && (item_list[selected].flags & MENU_FLAG_RIGHT_ARROW))
	{
		switch_menu_pick_next(selected_in, switch_entry);
		changed = TRUE;
	}

	/* update the selection to match the existing entry in case things got shuffled */
	/* due to conditional DIPs changing things */
	if (changed)
	{
		int newsel = 0;
		input_port_update_defaults();
		for (in = Machine->input_ports; in->type != IPT_END; in++)
			if (in->type == switch_name && input_port_active(in) && input_port_condition(in))
			{
				if (selected_in == in)
				{
					selected = newsel;
					break;
				}
				newsel++;
			}
	}

	return selected | (switch_name << 16) | (switch_entry << 24);
}



/*************************************
 *
 *  Analog settings menu
 *
 *************************************/

INLINE void analog_menu_add_item(ui_menu_item *item, const input_port_entry *in, int append_string, int which_item)
{
	int value, minval, maxval;

	/* set the item text using the formatting string provided */
	item->text = &menu_string_pool[menu_string_pool_offset];
	menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%s %s", _(input_port_name(in)), ui_getstring(append_string)) + 1;

	/* set the subitem text */
	item->subtext = &menu_string_pool[menu_string_pool_offset];
	switch (which_item)
	{
		default:
		case ANALOG_ITEM_KEYSPEED:
			value = in->analog.delta;
			minval = 1;
			maxval = 255;
			menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%d", value) + 1;
			break;

		case ANALOG_ITEM_CENTERSPEED:
			value = in->analog.centerdelta;
			minval = 0;
			maxval = 255;
			menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%d", value) + 1;
			break;

		case ANALOG_ITEM_REVERSE:
			value = in->analog.reverse;
			minval = 0;
			maxval = 1;
			item->subtext = value ? ui_getstring(UI_on) : ui_getstring(UI_off);
			break;

		case ANALOG_ITEM_SENSITIVITY:
			value = in->analog.sensitivity;
			minval = 1;
			maxval = 255;
			menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%d%%", value) + 1;
			break;
	}

	/* put on arrows */
	if (value > minval)
		item->flags |= MENU_FLAG_LEFT_ARROW;
	if (value < maxval)
		item->flags |= MENU_FLAG_RIGHT_ARROW;
}


static UINT32 menu_analog(UINT32 state)
{
	ui_menu_item item_list[MAX_INPUT_PORTS * 4 * ANALOG_ITEM_COUNT];
	input_port_entry *selected_in = NULL;
	input_port_entry *in;
	int menu_items = 0;
	int delta = 0;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* loop over input ports and add the items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (port_type_is_analog(in->type))
		{
			/* track the selected item */
			if (state >= menu_items && state < menu_items + 4)
				selected_in = in;

			/* add four items for each one */
			analog_menu_add_item(&item_list[menu_items++], in, UI_keyjoyspeed, ANALOG_ITEM_KEYSPEED);
			analog_menu_add_item(&item_list[menu_items++], in, UI_centerspeed, ANALOG_ITEM_CENTERSPEED);
			analog_menu_add_item(&item_list[menu_items++], in, UI_reverse, ANALOG_ITEM_REVERSE);
			analog_menu_add_item(&item_list[menu_items++], in, UI_sensitivity, ANALOG_ITEM_SENSITIVITY);
		}

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, state);

	/* handle generic menu keys */
	if (ui_menu_generic_keys((int *) &state, menu_items))
		return state;

	/* handle left/right arrows */
	if (input_ui_pressed_repeat(IPT_UI_LEFT,6) && (item_list[state].flags & MENU_FLAG_LEFT_ARROW))
		delta = -1;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT,6) && (item_list[state].flags & MENU_FLAG_RIGHT_ARROW))
		delta = 1;

	/* adjust the appropriate value */
	if (delta != 0 && selected_in)
		switch (state % ANALOG_ITEM_COUNT)
		{
			case ANALOG_ITEM_KEYSPEED:		selected_in->analog.delta += delta;			break;
			case ANALOG_ITEM_CENTERSPEED:	selected_in->analog.centerdelta += delta;	break;
			case ANALOG_ITEM_REVERSE:		selected_in->analog.reverse += delta;		break;
			case ANALOG_ITEM_SENSITIVITY:	selected_in->analog.sensitivity += delta;	break;
		}

	return state;
}



/*************************************
 *
 *  Joystick calibration menu
 *
 *************************************/

static UINT32 menu_joystick_calibrate(UINT32 state)
{
	const char *msg;

	/* two states */
	switch (state)
	{
		/* state 0 is just starting */
		case 0:
			osd_joystick_start_calibration();
			state++;
			break;

		/* state 1 is where we spend our time */
		case 1:

			/* get the message; if none, we're done */
			msg = osd_joystick_calibrate_next();
			if (msg == NULL)
			{
				osd_joystick_end_calibration();
				return ui_menu_stack_pop();
			}

			/* display the message */
			ui_draw_message_window(msg);

			/* handle cancel and select */
			if (input_ui_pressed(IPT_UI_CANCEL))
				return ui_menu_stack_pop();
			if (input_ui_pressed(IPT_UI_SELECT))
				osd_joystick_calibrate();
			break;
	}

	return state;
}



/*************************************
 *
 *  Bookkeeping information screen
 *
 *************************************/

#ifndef MESS
static UINT32 menu_bookkeeping(UINT32 state)
{
	char buf[2048];
	char *bufptr = buf;
	int selected = 0;
	int ctrnum;
	mame_time total_time;

	/* show total time first */
	total_time = mame_timer_get_time();
	if (total_time.seconds >= 60 * 60)
		bufptr += sprintf(bufptr, "%s: %d:%02d:%02d\n\n", ui_getstring(UI_totaltime), total_time.seconds / (60*60), (total_time.seconds / 60) % 60, total_time.seconds % 60);
	else
		bufptr += sprintf(bufptr, "%s: %d:%02d\n\n", ui_getstring(UI_totaltime), (total_time.seconds / 60) % 60, total_time.seconds % 60);

	/* show tickets at the top */
	if (dispensed_tickets)
		bufptr += sprintf(bufptr, "%s: %d\n\n", ui_getstring(UI_tickets), dispensed_tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		/* display the coin counter number */
		bufptr += sprintf(bufptr, "%s %c: ", ui_getstring(UI_coin), ctrnum + 'A');

		/* display how many coins */
		if (!coin_count[ctrnum])
			bufptr += sprintf(bufptr, "%s", ui_getstring(UI_NA));
		else
			bufptr += sprintf(bufptr, "%d", coin_count[ctrnum]);

		/* display whether or not we are locked out */
		if (coinlockedout[ctrnum])
			bufptr += sprintf(bufptr, " %s", ui_getstring(UI_locked));
		*bufptr++ = '\n';
	}

	/* make it look like a menu */
	bufptr += sprintf(bufptr, "\n\t%s %s %s", ui_getstring(UI_lefthilight), ui_getstring(UI_returntomain), ui_getstring(UI_righthilight));

	/* draw the text */
	ui_draw_message_window(buf);

	/* handle the keys */
	ui_menu_generic_keys(&selected, 1);
	return selected;
}
#endif



/*************************************
 *
 *  Game information screen
 *
 *************************************/

static UINT32 menu_game_info(UINT32 state)
{
	char buf[2048];
	char *bufptr = buf;
	int selected = 0;
	int res;

	/* add the game info */
	bufptr += sprintf_game_info(bufptr);

	/* make it look like a menu */
	bufptr += sprintf(bufptr, "\n\t%s %s %s", ui_getstring(UI_lefthilight), ui_getstring(UI_returntomain), ui_getstring(UI_righthilight));

	/* draw the text */
	ui_draw_message_window_scroll(buf);

	res = ui_window_scroll_keys();
	if (res > 0)
		return ui_menu_stack_pop();

	return selected;
}



/*************************************
 *
 *  Douments menu
 *
 *************************************/

static UINT32 menu_documents(UINT32 state)
{
#define NUM_DOCUMENTS	(UI_keyjoyspeed - UI_history)

	ui_menu_item item_list[NUM_DOCUMENTS + 2];
	int menu_items = 0;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* build up the menu */
	for (menu_items = 0; menu_items < NUM_DOCUMENTS; menu_items++)
		item_list[menu_items].text = ui_getstring(UI_history + menu_items);

	/* add an item for the return */
	item_list[menu_items++].text = ui_getstring(UI_returntoprior);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, state);

	/* handle the keys */
	if (ui_menu_generic_keys(&state, menu_items))
		return state;
	if (input_ui_pressed(IPT_UI_SELECT))
	{
#ifdef CMD_LIST
		if (state + UI_history == UI_command)
			return ui_menu_stack_push(menu_command, 0);
#endif /* CMD_LIST */

		return ui_menu_stack_push(menu_document_contents, state << 24);
	}

	return state;

#undef NUM_DOCUMENT
}


static UINT32 menu_document_contents(UINT32 state)
{
	static char *bufptr = NULL;
	static const game_driver *last_drv;
	static int last_selected;
	static int last_dattype;
	int bufsize = 256 * 1024; // 256KB of history.dat buffer, enough for everything
	int dattype = (state >> 24) + UI_history;
	int selected = 0;
	int res;

	if (bufptr)
	{
		if (Machine->gamedrv != last_drv || selected != last_selected || dattype != last_dattype)
		{
			/* force buffer to be recreated */
			free (bufptr);
			bufptr = NULL;
		}
	}

	if (!bufptr)
	{
		/* allocate a buffer for the text */
		bufptr = malloc(bufsize);

		if (bufptr)
		{
			int game_paused = mame_is_paused();

			/* Disable sound to prevent strange sound*/
			if (!game_paused)
				mame_pause(TRUE);

			if ((dattype == UI_history && (load_driver_history(Machine->gamedrv, bufptr, bufsize) == 0))
#ifdef STORY_DATAFILE
			 || (dattype == UI_story && (load_driver_story(Machine->gamedrv, bufptr, bufsize) == 0))
#endif /* STORY_DATAFILE */
			 || (dattype == UI_mameinfo && (load_driver_mameinfo(Machine->gamedrv, bufptr, bufsize) == 0))
			 || (dattype == UI_drivinfo && (load_driver_drivinfo(Machine->gamedrv, bufptr, bufsize) == 0))
			 || (dattype == UI_statistics && (load_driver_statistics(bufptr, bufsize) == 0)))
			{
				last_drv = Machine->gamedrv;
				last_selected = selected;
				last_dattype = dattype;

				strcat(bufptr, "\n\t");
				strcat(bufptr, ui_getstring(UI_lefthilight));
				strcat(bufptr, " ");
				strcat(bufptr, ui_getstring(UI_returntoprior));
				strcat(bufptr, " ");
				strcat(bufptr, ui_getstring(UI_righthilight));
				strcat(bufptr, "\n");
			}
			else
			{
				free(bufptr);
				bufptr = NULL;
			}

			if (!game_paused)
				mame_pause(FALSE);
		}
	}

	/* draw the text */
	if (bufptr)
		ui_draw_message_window_scroll(bufptr);
	else
	{
		char msg[80];

		strcpy(msg, "\t");

		switch (dattype)
		{
		case UI_history:
			strcat(msg, ui_getstring(UI_historymissing));
			break;
#ifdef STORY_DATAFILE
		case UI_story:
			strcat(msg, ui_getstring(UI_storymissing));
			break;
#endif /* STORY_DATAFILE */
		case UI_mameinfo:
			strcat(msg, ui_getstring(UI_mameinfomissing));
			break;
		case UI_drivinfo:
			strcat(msg, ui_getstring(UI_drivinfomissing));
			break;
		case UI_statistics:
			strcat(msg, ui_getstring(UI_statisticsmissing));
			break;
		}

		strcat(msg, "\n\n\t");
		strcat(msg, ui_getstring(UI_lefthilight));
		strcat(msg, " ");
		strcat(msg, ui_getstring(UI_returntoprior));
		strcat(msg, " ");
		strcat(msg, ui_getstring(UI_righthilight));

		ui_draw_message_window_scroll(msg);
	}

	res = ui_window_scroll_keys();
	if (res > 0)
		return ui_menu_stack_pop();

	return ((dattype - UI_history) << 24) | selected;
}


#ifdef CMD_LIST
static UINT32 menu_command(UINT32 state)
{
	int selected = state & ((1 << 24) - 1);
	int shortcut = state >> 24;
	int menu_items = 0;
	const char *item[256];
	int total;

	total = command_sub_menu(Machine->gamedrv, item);
	if (total)
	{
		int last_selected = selected;

		ui_menu_item item_list[256 + 2];

		/* reset the menu and string pool */
		memset(item_list, 0, sizeof(item_list));
		menu_string_pool_offset = 0;

		for (menu_items = 0; menu_items < total; menu_items++)
			item_list[menu_items].text = item[menu_items];

		/* add empty line */
		item_list[menu_items++].text = "";

		/* add an item for the return */
		item_list[menu_items++].text = &menu_string_pool[menu_string_pool_offset];
		menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset],
		                                   "\t%s",shortcut ? ui_getstring(UI_returntogame) : ui_getstring(UI_returntoprior)) + 1;

		/* draw the menu */
		ui_draw_menu(item_list, menu_items, selected);

		/* handle generic menu keys */
		if (ui_menu_generic_keys(&selected, menu_items))
			return selected;

		/* skip empty line */
		if (selected == total)
		{
			if (last_selected > selected)
				selected = total - 1;
			else
				selected = menu_items - 1;
		}

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			if (selected < total)
				return ui_menu_stack_push(menu_command_contents, (shortcut << 24) | selected);
		}
	}
	else
	{
		char buf[80];
		int res;

		strcpy(buf, "\t");
		strcat(buf, ui_getstring(UI_commandmissing));
		strcat(buf, "\n\n\t");
		strcat(buf, ui_getstring(UI_lefthilight));
		strcat(buf, " ");
		if (shortcut)
			strcat(buf, ui_getstring(UI_returntogame));
		else
			strcat(buf, ui_getstring(UI_returntoprior));
		strcat(buf, " ");
		strcat(buf, ui_getstring(UI_righthilight));

		ui_draw_message_window_scroll(buf);

		res = ui_window_scroll_keys();
		if (res > 0)
			return ui_menu_stack_pop();
	}

	return (shortcut << 24) | selected;
}


static UINT32 menu_command_contents(UINT32 state)
{
	static char *bufptr = NULL;
	static const game_driver *last_drv;
	static int last_selected;
	static int last_shortcut;
	int bufsize = 64 * 1024; // 64KB of command.dat buffer, enough for everything
	int selected = state & ((1 << 24) - 1);
	int shortcut = state >> 24;
	int res;

	if (bufptr)
	{
		if (Machine->gamedrv != last_drv || selected != last_selected || shortcut != last_shortcut)
		{
			/* force buffer to be recreated */
			free (bufptr);
			bufptr = NULL;
		}
	}

	if (!bufptr)
	{
		/* allocate a buffer for the text */
		bufptr = malloc(bufsize);

		if (bufptr)
		{
			int game_paused = mame_is_paused();

			/* Disable sound to prevent strange sound*/
			if (!game_paused)
				mame_pause(TRUE);

			if (load_driver_command_ex(Machine->gamedrv, bufptr, bufsize, selected) == 0)
			{
				last_drv = Machine->gamedrv;
				last_selected = selected;
				last_shortcut = shortcut;

				convert_command_move(bufptr);

				strcat(bufptr, "\n\t");
				strcat(bufptr, ui_getstring(UI_lefthilight));
				strcat(bufptr, " ");
				if (shortcut)
					strcat(bufptr, ui_getstring(UI_returntogame));
				else
					strcat(bufptr, ui_getstring(UI_returntoprior));
				strcat(bufptr, " ");
				strcat(bufptr, ui_getstring(UI_righthilight));
				strcat(bufptr, "\n");
			}
			else
			{
				free(bufptr);
				bufptr = NULL;
			}

			if (!game_paused)
				mame_pause(FALSE);
		}
	}

	/* draw the text */
	if (bufptr)
		ui_draw_message_window_scroll(bufptr);
	else
	{
		char buf[80];

		strcpy(buf, "\t");
		strcat(buf, ui_getstring(UI_commandmissing));
		strcat(buf, "\n\n\t");
		strcat(buf, ui_getstring(UI_lefthilight));
		strcat(buf, " ");
		if (shortcut)
			strcat(buf, ui_getstring(UI_returntogame));
		else
			strcat(buf, ui_getstring(UI_returntoprior));
		strcat(buf, " ");
		strcat(buf, ui_getstring(UI_righthilight));

		ui_draw_message_window_scroll(buf);
	}

	res = ui_window_scroll_keys();
	if (res > 0)
		return ui_menu_stack_pop();

	return (shortcut << 24) | selected;
}
#endif /* CMD_LIST */



/*************************************
 *
 *  Cheat menu
 *
 *************************************/

static UINT32 menu_cheat(UINT32 state)
{
	state = cheat_menu(state);
	if ((state & ((1 << 8) - 1)) == 0)
		return ui_menu_stack_pop();

	return state;
}



/*************************************
 *
 *  Memory card menu
 *
 *************************************/

static UINT32 menu_memory_card(UINT32 state)
{
	ui_menu_item item_list[5];
	int menu_items = 0;
	int cardnum = state >> 16;
	int selected = state & 0xffff;
	int insertindex = -1, ejectindex = -1, createindex = -1;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* add the card select menu */
	item_list[menu_items].text = ui_getstring(UI_selectcard);
	item_list[menu_items].subtext = &menu_string_pool[menu_string_pool_offset];
	menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%d", cardnum) + 1;
	if (cardnum > 0)
		item_list[menu_items].flags |= MENU_FLAG_LEFT_ARROW;
	if (cardnum < 1000)
		item_list[menu_items].flags |= MENU_FLAG_RIGHT_ARROW;
	menu_items++;

	/* add the remaining items */
	item_list[insertindex = menu_items++].text = ui_getstring(UI_loadcard);
	if (memcard_present() != -1)
		item_list[ejectindex = menu_items++].text = ui_getstring(UI_ejectcard);
	item_list[createindex = menu_items++].text = ui_getstring(UI_createcard);

	/* add an item for the return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* handle the keys */
	if (ui_menu_generic_keys(&selected, menu_items))
		return selected;
	if (selected == 0 && input_ui_pressed(IPT_UI_LEFT) && cardnum > 0)
		cardnum--;
	if (selected == 0 && input_ui_pressed(IPT_UI_RIGHT) && cardnum < 1000)
		cardnum++;

	/* handle actions */
	if (input_ui_pressed(IPT_UI_SELECT))
	{
		/* handle load */
		if (selected == insertindex)
		{
			if (memcard_insert(cardnum) == 0)
			{
				ui_popup("%s", ui_getstring(UI_loadok));
				ui_menu_stack_reset();
				return 0;
			}
			else
				ui_popup("%s", ui_getstring(UI_loadfailed));
		}

		/* handle eject */
		else if (selected == ejectindex)
		{
			memcard_eject();
			ui_popup("%s", ui_getstring(UI_cardejected));
		}

		/* handle create */
		else if (selected == createindex)
		{
			if (memcard_create(cardnum, FALSE) == 0)
				ui_popup("%s", ui_getstring(UI_cardcreated));
			else
				ui_popup("%s\n%s", ui_getstring(UI_cardcreatedfailed), ui_getstring(UI_cardcreatedfailed2));
		}
	}

	return selected | (cardnum << 16);
}



/*************************************
 *
 *  Video menu
 *
 *************************************/

#ifdef NEW_RENDER
static const char *translate_view(const unsigned char *s)
{
	unsigned char *p = &menu_string_pool[menu_string_pool_offset];
	const unsigned char *idx[8];
	const unsigned char **pp;

	pp = idx;
	while (*s)
	{
		if (isdigit(*s))
		{
			*pp++ = s;

			*p++ = '%';
			*p++ = 'd';

			for (s++; *s; s++)
				if (!isdigit(*s))
					break;
		}
		else
			*p++ = *s++;
	}
	*p = '\0';

	s = _(&menu_string_pool[menu_string_pool_offset]);
	menu_string_pool_offset += strlen(s) + 1;

	p = &menu_string_pool[menu_string_pool_offset];

	pp = idx;
	while (*s)
	{
		if (s[0] == '%' && s[1] == 'd')
		{
			s += 2;

			while (isdigit(**pp))
				*p++ = *(*pp)++;
			pp++;
		}
		else
			*p++ = *s++;
	}
	*p = '\0';

	s = &menu_string_pool[menu_string_pool_offset];
	menu_string_pool_offset += strlen(s) + 1;

	return s;
}

static UINT32 menu_video(UINT32 state)
{
	ui_menu_item item_list[100];
	render_target *targetlist[16];
	int curtarget = state >> 16;
	int selected = state & 0xffff;
	int menu_items = 0;
	int targets;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* find the targets */
	for (targets = 0; targets < ARRAY_LENGTH(targetlist); targets++)
	{
		targetlist[targets] = render_target_get_indexed(targets);
		if (targetlist[targets] == NULL)
			break;
	}

	/* if we have a current target of 1000, we may need to select from multiple targets */
	if (curtarget == 1000)
	{
		/* count up the targets, creating menu items for them */
		for ( ; menu_items < targets; menu_items++)
		{
			/* create a string for the item */
			item_list[menu_items].text = &menu_string_pool[menu_string_pool_offset];
			menu_string_pool_offset += sprintf(&menu_string_pool[menu_string_pool_offset], "%s%d", ui_getstring(UI_screen), menu_items) + 1;
		}

		/* if we only ended up with one, auto-select it */
		if (menu_items == 1)
			return menu_video(0 << 16);

		/* add an item for moving the UI */
		item_list[menu_items++].text = "Move User Interface";

		/* add an item to return */
		item_list[menu_items++].text = ui_getstring(UI_returntomain);

		/* draw the menu */
		ui_draw_menu(item_list, menu_items, selected);

		/* handle the keys */
		if (ui_menu_generic_keys(&selected, menu_items))
			return selected;

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			if (selected == menu_items - 2)
			{
				render_target *uitarget = render_get_ui_target();
				int targnum;

				for (targnum = 0; targnum < targets; targnum++)
					if (targetlist[targnum] == uitarget)
						break;
				targnum = (targnum + 1) % targets;
				render_set_ui_target(targetlist[targnum]);
			}
			else
				return ui_menu_stack_push(menu_video, (selected << 16) | render_target_get_view(render_target_get_indexed(selected)));
		}
	}

	/* otherwise, draw the list of layouts */
	else
	{
		render_target *target = targetlist[curtarget];
		int layermask;
		assert(target != NULL);

		/* add all the views */
		for ( ; menu_items < ARRAY_LENGTH(item_list); menu_items++)
		{
			const char *name = render_target_get_view_name(target, menu_items);
			if (name == NULL)
				break;

			/* create a string for the item */
			item_list[menu_items].text = translate_view(name);
		}

		/* add an item to rotate */
		item_list[menu_items++].text = _("Rotate View");

		/* add an item to enable/disable backdrops */
		layermask = render_target_get_layer_config(target);
		if (layermask & LAYER_CONFIG_ENABLE_BACKDROP)
			item_list[menu_items++].text = _("Hide Backdrops");
		else
			item_list[menu_items++].text = _("Show Backdrops");

		/* add an item to enable/disable overlays */
		if (layermask & LAYER_CONFIG_ENABLE_OVERLAY)
			item_list[menu_items++].text = _("Hide Overlays");
		else
			item_list[menu_items++].text = _("Show Overlays");

		/* add an item to enable/disable bezels */
		if (layermask & LAYER_CONFIG_ENABLE_BEZEL)
			item_list[menu_items++].text = _("Hide Bezels");
		else
			item_list[menu_items++].text = _("Show Bezels");

		/* add an item to enable/disable cropping */
		if (layermask & LAYER_CONFIG_ZOOM_TO_SCREEN)
			item_list[menu_items++].text = _("Show Full Artwork");
		else
			item_list[menu_items++].text = _("Crop to Screen");

		/* add an item to return */
		item_list[menu_items++].text = ui_getstring(UI_returntoprior);

		/* draw the menu */
		ui_draw_menu(item_list, menu_items, selected);

		/* handle the keys */
		if (ui_menu_generic_keys(&selected, menu_items))
			return selected;

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			/* rotate */
			if (selected == menu_items - 6)
			{
				render_target_set_orientation(target, orientation_add(ROT90, render_target_get_orientation(target)));
				if (target == render_get_ui_target())
					render_container_set_orientation(render_container_get_ui(), orientation_add(ROT270, render_container_get_orientation(render_container_get_ui())));
			}

			/* show/hide backdrops */
			else if (selected == menu_items - 5)
			{
				layermask ^= LAYER_CONFIG_ENABLE_BACKDROP;
				render_target_set_layer_config(target, layermask);
			}

			/* show/hide overlays */
			else if (selected == menu_items - 4)
			{
				layermask ^= LAYER_CONFIG_ENABLE_OVERLAY;
				render_target_set_layer_config(target, layermask);
			}

			/* show/hide bezels */
			else if (selected == menu_items - 3)
			{
				layermask ^= LAYER_CONFIG_ENABLE_BEZEL;
				render_target_set_layer_config(target, layermask);
			}

			/* crop/uncrop artwork */
			else if (selected == menu_items - 2)
			{
				layermask ^= LAYER_CONFIG_ZOOM_TO_SCREEN;
				render_target_set_layer_config(target, layermask);
			}

			/* else just set the selected view */
			else
				render_target_set_view(target, selected);
		}
	}

	return selected | (curtarget << 16);
}
#endif



#ifdef USE_SCALE_EFFECTS
/*************************************
 *
 *  Scale Effect menu
 *
 *************************************/

static UINT32 menu_scale_effect(UINT32 state)
{
	ui_menu_item item_list[100];
	int selected = state;
	int menu_items = 0;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	item_list[0].text = _("None");

	/* count up the targets, creating menu items for them */
	for (menu_items = 1 ; menu_items < ARRAY_LENGTH(item_list); menu_items++)
	{
		const char *desc = scale_desc(menu_items);
		if (desc == NULL)
			break;

		/* create a string for the item */
		item_list[menu_items].text = desc;
	}

	/* add an item to return */
	item_list[menu_items++].text = ui_getstring(UI_returntomain);

	/* draw the menu */
	ui_draw_menu(item_list, menu_items, selected);

	/* handle the keys */
	if (ui_menu_generic_keys(&selected, menu_items))
		return selected;

	/* handle actions */
	if (input_ui_pressed(IPT_UI_SELECT))
	{
		video_exit_scale_effect();
		scale_decode(scale_name(selected)); 
		video_init_scale_effect();
	}

	return selected;
}
#endif /* USE_SCALE_EFFECTS */



/*************************************
 *
 *  Game reset action
 *
 *************************************/

static UINT32 menu_reset_game(UINT32 state)
{
	/* request a reset */
	mame_schedule_soft_reset();

	/* reset the menu stack */
	ui_menu_stack_reset();
	return 0;
}



/*************************************
 *
 *  MESS-specific menus
 *
 *************************************/

#ifdef MESS
static UINT32 menu_file_manager(UINT32 state)
{
	int result = filemanager(state);
	if (result == 0)
	return ui_menu_stack_pop();
	return result;
}


static UINT32 menu_tape_control(UINT32 state)
{
	int result = tapecontrol(state);
	if (result == 0)
	return ui_menu_stack_pop();
	return result;
}
#endif








static int sprintf_game_info(char *buf)
{
	char *bufptr = buf;
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
		bufptr += sprintf(bufptr,"\n%s:\n%d x %d %s %f Hz\n",
				ui_getstring(UI_screenres),
				Machine->visible_area[0].max_x - Machine->visible_area[0].min_x + 1,
				Machine->visible_area[0].max_y - Machine->visible_area[0].min_y + 1,
				(Machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? _("(V)") : _("(H)"),
				Machine->refresh_rate[0]);
	return bufptr - buf;
}






#ifdef NEW_RENDER

typedef struct _showgfx_state showgfx_state;
struct _showgfx_state
{
	UINT8		mode;				/* which mode are we in? */

	/* intermediate bitmaps */
	UINT8		bitmap_dirty;		/* is the bitmap dirty? */
	mame_bitmap *bitmap;			/* bitmap for drawing gfx and tilemaps */
	render_texture *texture;		/* texture for rendering the above bitmap */

	/* palette-specific data */
	struct
	{
		int		which;
		int		offset;
		int		count;
	} palette;

	struct
	{
		int		set;
		int		offset[MAX_GFX_ELEMENTS];
		int		color[MAX_GFX_ELEMENTS];
		int		count[MAX_GFX_ELEMENTS];
		UINT8	rotate[MAX_GFX_ELEMENTS];
	} gfxset;

	struct
	{
		int		which;
		int		xoffs;
		int		yoffs;
		int		zoom;
		UINT8	rotate;
		UINT8	init;
	} tilemap;
};

static showgfx_state showgfx;


static void showgfx_exit(void)
{
	if (showgfx.texture != NULL)
		render_texture_free(showgfx.texture);
	showgfx.texture = NULL;

	if (showgfx.bitmap != NULL)
		bitmap_free(showgfx.bitmap);
	showgfx.bitmap = NULL;
}


static void handle_palette_keys(showgfx_state *state)
{
	int total;

	/* handle zoom (minus,plus) */
	if (input_ui_pressed(IPT_UI_ZOOM_OUT))
		state->palette.count /= 2;
	if (input_ui_pressed(IPT_UI_ZOOM_IN))
		state->palette.count *= 2;

	/* clamp within range */
	if (state->palette.count <= 4)
		state->palette.count = 4;
	if (state->palette.count > 64)
		state->palette.count = 64;

	/* handle colormap selection (open bracket,close bracket) */
	if (input_ui_pressed(IPT_UI_PREV_GROUP))
		state->palette.which--;
	if (input_ui_pressed(IPT_UI_NEXT_GROUP))
		state->palette.which++;

	/* clamp within range */
	if (state->palette.which < 0)
		state->palette.which = 1;
	if (state->palette.which > (Machine->drv->color_table_len != 0))
		state->palette.which = (Machine->drv->color_table_len != 0);

	/* cache some info in locals */
	total = state->palette.which ? Machine->drv->color_table_len : Machine->drv->total_colors;

	/* handle keyboard navigation */
	if (input_ui_pressed_repeat(IPT_UI_UP, 4))
		state->palette.offset -= state->palette.count;
	if (input_ui_pressed_repeat(IPT_UI_DOWN, 4))
		state->palette.offset += state->palette.count;
	if (input_ui_pressed_repeat(IPT_UI_PAGE_UP, 6))
		state->palette.offset -= state->palette.count * state->palette.count;
	if (input_ui_pressed_repeat(IPT_UI_PAGE_DOWN, 6))
		state->palette.offset += state->palette.count * state->palette.count;
	if (input_ui_pressed_repeat(IPT_UI_HOME, 4))
		state->palette.offset = 0;
	if (input_ui_pressed_repeat(IPT_UI_END, 4))
		state->palette.offset = total;

	/* clamp within range */
	if (state->palette.offset + state->palette.count > ((total + state->palette.count - 1) / state->palette.count))
		state->palette.offset = ((total + state->palette.count - 1) / state->palette.count) * state->palette.count - state->palette.count * state->palette.count;
	if (state->palette.offset < 0)
		state->palette.offset = 0;
}



static void showgfx_palette_handler(showgfx_state *state)
{
	int total = state->palette.which ? Machine->drv->color_table_len : Machine->drv->total_colors;
	UINT16 *pens = state->palette.which ? Machine->game_colortable : NULL;
	const char *title = state->palette.which ? "COLORTABLE" : "PALETTE";
	float cellwidth, cellheight;
	float chwidth, chheight;
	float titlewidth;
	float x0, y0;
	render_bounds cellboxbounds;
	render_bounds boxbounds;
	int x, y, skip;

	/* lazy init */
	if (state->palette.count == 0)
		state->palette.count = 16;

	/* add a half character padding for the box */
	chheight = UI_FONT_HEIGHT;
	chwidth = render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the character cell box bounds starts a half character in from the box */
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	/* add space on the left for 5 characters of text, plus a half character of padding */
	cellboxbounds.x0 += 5.5f * chwidth;

	/* add space on the top for a title, a half line of padding, a header, and another half line */
	cellboxbounds.y0 += 3.0f * chheight;

	/* figure out the title and expand the outer box to fit */
	titlewidth = render_font_get_string_width(ui_font, chheight, render_get_ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	/* go ahead and draw the outer box now */
	add_outlined_box_fp(boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, MENU_BACKCOLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		render_ui_add_char(x0, y0, chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, title[x]);
		x0 += render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), title[x]);
	}

	/* compute the cell size */
	cellwidth = (cellboxbounds.x1 - cellboxbounds.x0) / (float)state->palette.count;
	cellheight = (cellboxbounds.y1 - cellboxbounds.y0) / (float)state->palette.count;

	/* draw the top column headers */
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < state->palette.count; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		render_ui_add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, "0123456789ABCDEF"[x & 0xf]);

		/* if we're skipping, draw a point between the character and the box to indicate which */
		/* one it's referring to */
		if (skip != 0)
			render_ui_add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + cellboxbounds.y0), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* draw the side column headers */
	skip = (int)(chheight / cellheight);
	for (y = 0; y < state->palette.count; y += 1 + skip)

		/* only display if there is data to show */
		if (state->palette.offset + y * state->palette.count < total)
		{
			char buffer[10];

			/* if we're skipping, draw a point between the character and the box to indicate which */
			/* one it's referring to */
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				render_ui_add_point(0.5f * (x0 + cellboxbounds.x0), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			/* draw the row header */
			sprintf(buffer, "%5X", state->palette.offset + y * state->palette.count);
			for (x = 4; x >= 0; x--)
			{
				x0 -= render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), buffer[x]);
				render_ui_add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, buffer[x]);
			}
		}

	/* now add the rectangles for the colors */
	for (y = 0; y < state->palette.count; y++)
		for (x = 0; x < state->palette.count; x++)
		{
			int index = state->palette.offset + y * state->palette.count + x;
			if (index < total)
			{
				pen_t pen = (pens != NULL) ? pens[index] : index;
				render_ui_add_rect(cellboxbounds.x0 + x * cellwidth, cellboxbounds.y0 + y * cellheight,
									cellboxbounds.x0 + (x + 1) * cellwidth, cellboxbounds.y0 + (y + 1) * cellheight,
									0xff000000 | game_palette[pen], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}

	/* handle keys */
	handle_palette_keys(state);
}


static void handle_gfxset_keys(showgfx_state *state, int xcells, int ycells)
{
	showgfx_state oldstate = *state;
	gfx_element *gfx;
	int temp, set;

	/* handle gfxset selection (open bracket,close bracket) */
	if (input_ui_pressed(IPT_UI_PREV_GROUP))
	{
		for (temp = state->gfxset.set - 1; temp >= 0; temp--)
			if (Machine->gfx[temp] != NULL)
				break;
		if (temp >= 0)
			state->gfxset.set = temp;
	}
	if (input_ui_pressed(IPT_UI_NEXT_GROUP))
	{
		for (temp = state->gfxset.set + 1; temp < MAX_GFX_ELEMENTS; temp++)
			if (Machine->gfx[temp] != NULL)
				break;
		if (temp < MAX_GFX_ELEMENTS)
			state->gfxset.set = temp;
	}

	/* cache some info in locals */
	set = state->gfxset.set;
	gfx = Machine->gfx[set];

	/* handle cells per line (minus,plus) */
	if (input_ui_pressed(IPT_UI_ZOOM_OUT))
		state->gfxset.count[set] = xcells - 1;
	if (input_ui_pressed(IPT_UI_ZOOM_IN))
		state->gfxset.count[set] = xcells + 1;

	/* clamp within range */
	if (state->gfxset.count[set] < 2)
		state->gfxset.count[set] = 2;
	if (state->gfxset.count[set] > 32)
		state->gfxset.count[set] = 32;

	/* handle rotation (R) */
	if (input_ui_pressed(IPT_UI_ROTATE))
		state->gfxset.rotate[set] = orientation_add(ROT90, state->gfxset.rotate[set]);

	/* handle navigation within the cells (up,down,pgup,pgdown) */
	if (input_ui_pressed_repeat(IPT_UI_UP, 4))
		state->gfxset.offset[set] -= xcells;
	if (input_ui_pressed_repeat(IPT_UI_DOWN, 4))
		state->gfxset.offset[set] += xcells;
	if (input_ui_pressed_repeat(IPT_UI_PAGE_UP, 6))
		state->gfxset.offset[set] -= xcells * ycells;
	if (input_ui_pressed_repeat(IPT_UI_PAGE_DOWN, 6))
		state->gfxset.offset[set] += xcells * ycells;
	if (input_ui_pressed_repeat(IPT_UI_HOME, 4))
		state->gfxset.offset[set] = 0;
	if (input_ui_pressed_repeat(IPT_UI_END, 4))
		state->gfxset.offset[set] = gfx->total_elements;

	/* clamp within range */
	if (state->gfxset.offset[set] + xcells * ycells > ((gfx->total_elements + xcells - 1) / xcells) * xcells)
		state->gfxset.offset[set] = ((gfx->total_elements + xcells - 1) / xcells) * xcells - xcells * ycells;
	if (state->gfxset.offset[set] < 0)
		state->gfxset.offset[set] = 0;

	/* handle color selection (left,right) */
	if (input_ui_pressed_repeat(IPT_UI_LEFT, 4))
		state->gfxset.color[set] -= 1;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT, 4))
		state->gfxset.color[set] += 1;

	/* clamp within range */
	if (state->gfxset.color[set] >= (int)gfx->total_colors)
		state->gfxset.color[set] = gfx->total_colors - 1;
	if (state->gfxset.color[set] < 0)
		state->gfxset.color[set] = 0;

	/* if something changed, we need to force an update to the bitmap */
	if (state->gfxset.set != oldstate.gfxset.set ||
		state->gfxset.offset[set] != oldstate.gfxset.offset[set] ||
		state->gfxset.rotate[set] != oldstate.gfxset.rotate[set] ||
		state->gfxset.color[set] != oldstate.gfxset.color[set] ||
		state->gfxset.count[set] != oldstate.gfxset.count[set])
	{
		state->bitmap_dirty = TRUE;
	}
}


static void draw_gfx_item(const gfx_element *gfx, int index, mame_bitmap *bitmap, int dstx, int dsty, int color, int rotate)
{
	int width = (rotate & ORIENTATION_SWAP_XY) ? gfx->height : gfx->width;
	int height = (rotate & ORIENTATION_SWAP_XY) ? gfx->width : gfx->height;
	UINT32 rowpixels = bitmap->rowpixels;
	const pen_t *palette = game_palette;
	const UINT16 *colortable = NULL;
	int x, y;

	/* select either the raw palette or the colortable */
	if (Machine->game_colortable != NULL)
		colortable = &Machine->game_colortable[(gfx->colortable - Machine->remapped_colortable) + color * gfx->color_granularity];
	else
		palette += color * gfx->color_granularity;

	/* loop over rows in the cell */
	for (y = 0; y < height; y++)
	{
		UINT32 *dest = (UINT32 *)bitmap->base + (dsty + y) * rowpixels + dstx;
		UINT8 *src = gfx->gfxdata + index * gfx->char_modulo;

		/* loop over columns in the cell */
		for (x = 0; x < width; x++)
		{
			int effx = x, effy = y;
			rgb_t pixel;
			UINT8 *s;

			/* compute effective x,y values after rotation */
			if (rotate & ORIENTATION_FLIP_X)
				effx = gfx->width - 1 - effx;
			if (rotate & ORIENTATION_FLIP_Y)
				effy = gfx->height - 1 - effy;
			if (rotate & ORIENTATION_SWAP_XY)
				{ int temp = effx; effx = effy; effy = temp; }

			/* get a pointer to the start of this source row */
			s = src + effy * gfx->line_modulo;

			/* extract the pixel */
			if (gfx->flags & GFX_PACKED)
				pixel = (s[effx/2] >> ((effx & 1) * 4)) & 0xf;
			else
				pixel = s[effx];
			if (colortable != NULL)
				pixel = colortable[pixel];
			*dest++ = 0xff000000 | palette[pixel];
		}
	}
}


static void update_gfxset_bitmap(showgfx_state *state, int xcells, int ycells, gfx_element *gfx)
{
	int set = state->gfxset.set;
	int cellxpix, cellypix;
	int x, y;

	/* compute the number of source pixels in a cell */
	cellxpix = 1 + ((state->gfxset.rotate[set] & ORIENTATION_SWAP_XY) ? gfx->height : gfx->width);
	cellypix = 1 + ((state->gfxset.rotate[set] & ORIENTATION_SWAP_XY) ? gfx->width : gfx->height);

	/* realloc the bitmap if it is too small */
	if (state->bitmap == NULL || state->texture == NULL || state->bitmap->depth != 32 || state->bitmap->width != cellxpix * xcells || state->bitmap->height != cellypix * ycells)
	{
		/* free the old stuff */
		if (state->bitmap != NULL)
			bitmap_free(state->bitmap);
		if (state->texture != NULL)
			render_texture_free(state->texture);

		/* allocate new stuff */
		state->bitmap = bitmap_alloc_depth(cellxpix * xcells, cellypix * ycells, 32);
		state->texture = render_texture_alloc(state->bitmap, NULL, NULL, TEXFORMAT_ARGB32, NULL, NULL);

		/* force a redraw */
		state->bitmap_dirty = TRUE;
	}

	/* handle the redraw */
	if (state->bitmap_dirty)
	{
		/* loop over rows */
		for (y = 0; y < ycells; y++)
		{
			rectangle cellbounds;

			/* make a rect that covers this row */
			cellbounds.min_x = 0;
			cellbounds.max_x = state->bitmap->width - 1;
			cellbounds.min_y = y * cellypix;
			cellbounds.max_y = (y + 1) * cellypix - 1;

			/* only display if there is data to show */
			if (state->gfxset.offset[set] + y * xcells < gfx->total_elements)
			{
				/* draw the individual cells */
				for (x = 0; x < xcells; x++)
				{
					int index = state->gfxset.offset[set] + y * xcells + x;

					/* update the bounds for this cell */
					cellbounds.min_x = x * cellxpix;
					cellbounds.max_x = (x + 1) * cellxpix - 1;

					/* only render if there is data */
					if (index < gfx->total_elements)
						draw_gfx_item(gfx, index, state->bitmap, cellbounds.min_x, cellbounds.min_y, state->gfxset.color[set], state->gfxset.rotate[set]);

					/* otherwise, fill with transparency */
					else
						fillbitmap(state->bitmap, 0, &cellbounds);
				}
			}

			/* otherwise, fill with transparency */
			else
				fillbitmap(state->bitmap, 0, &cellbounds);
		}

		/* reset the texture to force an update */
		render_texture_set_bitmap(state->texture, state->bitmap, NULL, NULL, TEXFORMAT_ARGB32);
		state->bitmap_dirty = FALSE;
	}
}


static void showgfx_gfxset_handler(showgfx_state *state)
{
	int set = state->gfxset.set;
	gfx_element *gfx = Machine->gfx[set];
	float fullwidth, fullheight;
	float cellwidth, cellheight;
	float chwidth, chheight;
	float titlewidth;
	float cellaspect;
	float x0, y0;
	render_bounds cellboxbounds;
	render_bounds boxbounds;
	int cellboxwidth, cellboxheight;
	int targwidth, targheight;
	int cellxpix, cellypix;
	int xcells, ycells;
	int pixelscale = 0;
	int x, y, skip;
	char title[100];

	/* lazy initialization */
	if (state->gfxset.count[0] == 0)
		for (x = 0; x < MAX_GFX_ELEMENTS; x++)
		{
			state->gfxset.rotate[x] = Machine->gamedrv->flags & ORIENTATION_MASK;
			state->gfxset.count[x] = 16;
		}

	/* get the bounds of our target */
	render_target_get_bounds(render_get_ui_target(), &targwidth, &targheight, NULL);

	/* add a half character padding for the box */
	chheight = UI_FONT_HEIGHT;
	chwidth = render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the character cell box bounds starts a half character in from the box */
	cellboxbounds = boxbounds;
	cellboxbounds.x0 += 0.5f * chwidth;
	cellboxbounds.x1 -= 0.5f * chwidth;
	cellboxbounds.y0 += 0.5f * chheight;
	cellboxbounds.y1 -= 0.5f * chheight;

	/* add space on the left for 5 characters of text, plus a half character of padding */
	cellboxbounds.x0 += 5.5f * chwidth;

	/* add space on the top for a title, a half line of padding, a header, and another half line */
	cellboxbounds.y0 += 3.0f * chheight;

	/* convert back to pixels */
	cellboxwidth = (cellboxbounds.x1 - cellboxbounds.x0) * (float)targwidth;
	cellboxheight = (cellboxbounds.y1 - cellboxbounds.y0) * (float)targheight;

	/* compute the number of source pixels in a cell */
	cellxpix = 1 + ((state->gfxset.rotate[state->gfxset.set] & ORIENTATION_SWAP_XY) ? gfx->height : gfx->width);
	cellypix = 1 + ((state->gfxset.rotate[state->gfxset.set] & ORIENTATION_SWAP_XY) ? gfx->width : gfx->height);

	/* compute the largest pixel scale factor that still fits */
	xcells = state->gfxset.count[set];
	while (xcells > 1)
	{
		pixelscale = (cellboxwidth / xcells) / cellxpix;
		if (pixelscale != 0)
			break;
		xcells--;
	}

	/* worst case, we need a pixel scale of 1 */
	pixelscale = MAX(1, pixelscale);

	/* in the Y direction, we just display as many as we can */
	ycells = cellboxheight / (pixelscale * cellypix);

	/* now determine the actual cellbox size */
	cellboxwidth = MIN(cellboxwidth, xcells * pixelscale * cellxpix);
	cellboxheight = MIN(cellboxheight, ycells * pixelscale * cellypix);

	/* compute the size of a single cell at this pixel scale factor, as well as the aspect ratio */
	cellwidth = (cellboxwidth / (float)xcells) / (float)targwidth;
	cellheight = (cellboxheight / (float)ycells) / (float)targheight;
	cellaspect = cellwidth / cellheight;

	/* working from the new width/height, recompute the boxbounds */
	fullwidth = (float)cellboxwidth / (float)targwidth + 6.5f * chwidth;
	fullheight = (float)cellboxheight / (float)targheight + 4.0f * chheight;

	/* recompute boxbounds from this */
	boxbounds.x0 = (1.0f - fullwidth) * 0.5f;
	boxbounds.x1 = boxbounds.x0 + fullwidth;
	boxbounds.y0 = (1.0f - fullheight) * 0.5f;
	boxbounds.y1 = boxbounds.y0 + fullheight;

	/* figure out the title and expand the outer box to fit */
	for (x = 0; x < MAX_GFX_ELEMENTS && Machine->gfx[x] != NULL; x++) ;
	sprintf(title, "GFX %d/%d %dx%d COLOR %X", state->gfxset.set, x - 1, gfx->width, gfx->height, state->gfxset.color[set]);
	titlewidth = render_font_get_string_width(ui_font, chheight, render_get_ui_aspect(), title);
	x0 = 0.0f;
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
		x0 = boxbounds.x0 - (0.5f - 0.5f * (titlewidth + chwidth));

	/* go ahead and draw the outer box now */
	add_outlined_box_fp(boxbounds.x0 - x0, boxbounds.y0, boxbounds.x1 + x0, boxbounds.y1, MENU_BACKCOLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		render_ui_add_char(x0, y0, chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, title[x]);
		x0 += render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), title[x]);
	}

	/* draw the top column headers */
	skip = (int)(chwidth / cellwidth);
	for (x = 0; x < xcells; x += 1 + skip)
	{
		x0 = boxbounds.x0 + 6.0f * chwidth + (float)x * cellwidth;
		y0 = boxbounds.y0 + 2.0f * chheight;
		render_ui_add_char(x0 + 0.5f * (cellwidth - chwidth), y0, chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, "0123456789ABCDEF"[x & 0xf]);

		/* if we're skipping, draw a point between the character and the box to indicate which */
		/* one it's referring to */
		if (skip != 0)
			render_ui_add_point(x0 + 0.5f * cellwidth, 0.5f * (y0 + chheight + boxbounds.y0 + 3.5f * chheight), UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* draw the side column headers */
	skip = (int)(chheight / cellheight);
	for (y = 0; y < ycells; y += 1 + skip)

		/* only display if there is data to show */
		if (state->gfxset.offset[set] + y * xcells < gfx->total_elements)
		{
			char buffer[10];

			/* if we're skipping, draw a point between the character and the box to indicate which */
			/* one it's referring to */
			x0 = boxbounds.x0 + 5.5f * chwidth;
			y0 = boxbounds.y0 + 3.5f * chheight + (float)y * cellheight;
			if (skip != 0)
				render_ui_add_point(0.5f * (x0 + boxbounds.x0 + 6.0f * chwidth), y0 + 0.5f * cellheight, UI_LINE_WIDTH, ARGB_WHITE, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			/* draw the row header */
			sprintf(buffer, "%5X", state->gfxset.offset[set] + y * xcells);
			for (x = 4; x >= 0; x--)
			{
				x0 -= render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), buffer[x]);
				render_ui_add_char(x0, y0 + 0.5f * (cellheight - chheight), chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, buffer[x]);
			}
		}

	/* update the bitmap */
	update_gfxset_bitmap(state, xcells, ycells, gfx);

	/* add the final quad */
	render_ui_add_quad(boxbounds.x0 + 6.0f * chwidth, boxbounds.y0 + 3.5f * chheight,
						boxbounds.x0 + 6.0f * chwidth + (float)cellboxwidth / (float)targwidth,
						boxbounds.y0 + 3.5f * chheight + (float)cellboxheight / (float)targheight,
						ARGB_WHITE, state->texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	/* handle keyboard navigation before drawing */
	handle_gfxset_keys(state, xcells, ycells);
}


static void handle_tilemap_keys(showgfx_state *state, int viswidth, int visheight)
{
	showgfx_state oldstate = *state;
	UINT32 mapwidth, mapheight;
	int step;

	/* handle tilemap selection (open bracket,close bracket) */
	if (input_ui_pressed(IPT_UI_PREV_GROUP))
		state->tilemap.which--;
	if (input_ui_pressed(IPT_UI_NEXT_GROUP))
		state->tilemap.which++;

	/* clamp within range */
	if (state->tilemap.which < 0)
		state->tilemap.which = 0;
	if (state->tilemap.which >= tilemap_count())
		state->tilemap.which = tilemap_count() - 1;

	/* cache some info in locals */
	tilemap_nb_size(state->tilemap.which, &mapwidth, &mapheight);

	/* handle zoom (minus,plus) */
	if (input_ui_pressed(IPT_UI_ZOOM_OUT))
		state->tilemap.zoom--;
	if (input_ui_pressed(IPT_UI_ZOOM_IN))
		state->tilemap.zoom++;

	/* clamp within range */
	if (state->tilemap.zoom < 0)
		state->tilemap.zoom = 0;
	if (state->tilemap.zoom > 8)
		state->tilemap.zoom = 8;
	if (state->tilemap.zoom != oldstate.tilemap.zoom)
	{
		if (state->tilemap.zoom != 0)
			ui_popup(_("Zoom = %d"), state->tilemap.zoom);
		else
			ui_popup(_("Zoom Auto"));
	}

	/* handle rotation (R) */
	if (input_ui_pressed(IPT_UI_ROTATE))
		state->tilemap.rotate = orientation_add(ROT90, state->tilemap.rotate);

	/* handle navigation (up,down,left,right) */
	step = 8;
	if (code_pressed(KEYCODE_LSHIFT)) step = 1;
	if (code_pressed(KEYCODE_LCONTROL)) step = 64;
	if (input_ui_pressed_repeat(IPT_UI_UP, 4))
		state->tilemap.yoffs -= step;
	if (input_ui_pressed_repeat(IPT_UI_DOWN, 4))
		state->tilemap.yoffs += step;
	if (input_ui_pressed_repeat(IPT_UI_LEFT, 6))
		state->tilemap.xoffs -= step;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT, 6))
		state->tilemap.xoffs += step;

	/* clamp within range */
	while (state->tilemap.xoffs < 0)
		state->tilemap.xoffs += mapwidth;
	while (state->tilemap.xoffs >= mapwidth)
		state->tilemap.xoffs -= mapwidth;
	while (state->tilemap.yoffs < 0)
		state->tilemap.yoffs += mapheight;
	while (state->tilemap.yoffs >= mapheight)
		state->tilemap.yoffs -= mapheight;

	/* if something changed, we need to force an update to the bitmap */
	if (state->tilemap.which != oldstate.tilemap.which ||
		state->tilemap.xoffs != oldstate.tilemap.xoffs ||
		state->tilemap.yoffs != oldstate.tilemap.yoffs ||
		state->tilemap.rotate != oldstate.tilemap.rotate)
	{
		state->bitmap_dirty = TRUE;
	}
}


static void update_tilemap_bitmap(showgfx_state *state, int width, int height)
{
	int format;

	/* swap the coordinates back if they were talking about a rotated surface */
	if (state->tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = width; width = height; height = temp; }

	/* pick our texture format */
	if (Machine->color_depth == 16)
		format = TEXFORMAT_PALETTE16;
	else if (Machine->color_depth == 15)
		format = TEXFORMAT_RGB15;
	else
		format = TEXFORMAT_RGB32;

	/* realloc the bitmap if it is too small */
	if (state->bitmap == NULL || state->texture == NULL || state->bitmap->depth != Machine->color_depth || state->bitmap->width != width || state->bitmap->height != height)
	{
		/* free the old stuff */
		if (state->bitmap != NULL)
			bitmap_free(state->bitmap);
		if (state->texture != NULL)
			render_texture_free(state->texture);

		/* allocate new stuff */
		state->bitmap = bitmap_alloc_depth(width, height, Machine->color_depth);
		state->texture = render_texture_alloc(state->bitmap, NULL, adjusted_palette, format, NULL, NULL);

		/* force a redraw */
		state->bitmap_dirty = TRUE;
	}

	/* handle the redraw */
	if (state->bitmap_dirty)
	{
		tilemap_nb_draw(state->bitmap, state->tilemap.which, state->tilemap.xoffs, state->tilemap.yoffs);

		/* reset the texture to force an update */
		render_texture_set_bitmap(state->texture, state->bitmap, NULL, adjusted_palette, format);
		state->bitmap_dirty = FALSE;
	}
}


static void showgfx_tilemap_handler(showgfx_state *state)
{
	float chwidth, chheight;
	render_bounds mapboxbounds;
	render_bounds boxbounds;
	int targwidth, targheight;
	float titlewidth;
	float x0, y0;
	int mapboxwidth, mapboxheight;
	int maxxscale, maxyscale;
	UINT32 mapwidth, mapheight;
	int x, pixelscale;
	char title[100];

	/* lazy initialization */
	if (!state->tilemap.init)
	{
		state->tilemap.rotate = Machine->gamedrv->flags & ORIENTATION_MASK;
		state->tilemap.init = TRUE;
	}

	/* get the bounds of our target */
	render_target_get_bounds(render_get_ui_target(), &targwidth, &targheight, NULL);

	/* get the size of the tilemap itself */
	tilemap_nb_size(state->tilemap.which, &mapwidth, &mapheight);
	if (state->tilemap.rotate & ORIENTATION_SWAP_XY)
		{ UINT32 temp = mapwidth; mapwidth = mapheight; mapheight = temp; }

	/* add a half character padding for the box */
	chheight = UI_FONT_HEIGHT;
	chwidth = render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), '0');
	boxbounds.x0 = 0.0f + 0.5f * chwidth;
	boxbounds.x1 = 1.0f - 0.5f * chwidth;
	boxbounds.y0 = 0.0f + 0.5f * chheight;
	boxbounds.y1 = 1.0f - 0.5f * chheight;

	/* the tilemap box bounds starts a half character in from the box */
	mapboxbounds = boxbounds;
	mapboxbounds.x0 += 0.5f * chwidth;
	mapboxbounds.x1 -= 0.5f * chwidth;
	mapboxbounds.y0 += 0.5f * chheight;
	mapboxbounds.y1 -= 0.5f * chheight;

	/* add space on the top for a title and a half line of padding */
	mapboxbounds.y0 += 1.5f * chheight;

	/* convert back to pixels */
	mapboxwidth = (mapboxbounds.x1 - mapboxbounds.x0) * (float)targwidth;
	mapboxheight = (mapboxbounds.y1 - mapboxbounds.y0) * (float)targheight;

	/* determine the maximum integral scaling factor */
	pixelscale = state->tilemap.zoom;
	if (pixelscale == 0)
	{
		for (maxxscale = 1; mapwidth * (maxxscale + 1) < mapboxwidth; maxxscale++) ;
		for (maxyscale = 1; mapheight * (maxyscale + 1) < mapboxheight; maxyscale++) ;
		pixelscale = MIN(maxxscale, maxyscale);
	}

	/* recompute the final box size */
	mapboxwidth = MIN(mapboxwidth, mapwidth * pixelscale);
	mapboxheight = MIN(mapboxheight, mapheight * pixelscale);

	/* recompute the bounds, centered within the existing bounds */
	mapboxbounds.x0 += 0.5f * ((mapboxbounds.x1 - mapboxbounds.x0) - (float)mapboxwidth / (float)targwidth);
	mapboxbounds.x1 = mapboxbounds.x0 + (float)mapboxwidth / (float)targwidth;
	mapboxbounds.y0 += 0.5f * ((mapboxbounds.y1 - mapboxbounds.y0) - (float)mapboxheight / (float)targheight);
	mapboxbounds.y1 = mapboxbounds.y0 + (float)mapboxheight / (float)targheight;

	/* now recompute the outer box against this new info */
	boxbounds.x0 = mapboxbounds.x0 - 0.5f * chwidth;
	boxbounds.x1 = mapboxbounds.x1 + 0.5f * chwidth;
	boxbounds.y0 = mapboxbounds.y0 - 2.0f * chheight;
	boxbounds.y1 = mapboxbounds.y1 + 0.5f * chheight;

	/* figure out the title and expand the outer box to fit */
	sprintf(title, "TMAP %d/%d %dx%d OFFS %d,%d", state->tilemap.which, tilemap_count() - 1, mapwidth, mapheight, state->tilemap.xoffs, state->tilemap.yoffs);
	titlewidth = render_font_get_string_width(ui_font, chheight, render_get_ui_aspect(), title);
	if (boxbounds.x1 - boxbounds.x0 < titlewidth + chwidth)
	{
		boxbounds.x0 = 0.5f - 0.5f * (titlewidth + chwidth);
		boxbounds.x1 = boxbounds.x0 + titlewidth + chwidth;
	}

	/* go ahead and draw the outer box now */
	add_outlined_box_fp(boxbounds.x0, boxbounds.y0, boxbounds.x1, boxbounds.y1, MENU_BACKCOLOR);

	/* draw the title */
	x0 = 0.5f - 0.5f * titlewidth;
	y0 = boxbounds.y0 + 0.5f * chheight;
	for (x = 0; title[x] != 0; x++)
	{
		render_ui_add_char(x0, y0, chheight, render_get_ui_aspect(), ARGB_WHITE, ui_font, title[x]);
		x0 += render_font_get_char_width(ui_font, chheight, render_get_ui_aspect(), title[x]);
	}

	/* update the bitmap */
	update_tilemap_bitmap(state, mapboxwidth / pixelscale, mapboxheight / pixelscale);

	/* add the final quad */
	render_ui_add_quad(mapboxbounds.x0, mapboxbounds.y0,
						mapboxbounds.x1, mapboxbounds.y1,
						ARGB_WHITE, state->texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(state->tilemap.rotate));

	/* handle keyboard input */
	handle_tilemap_keys(state, mapboxwidth, mapboxheight);
}


static UINT32 ui_handler_showgfx(UINT32 uistate)
{
	showgfx_state *state = &showgfx;

	/* if we have nothing, implicitly cancel */
	if (Machine->drv->total_colors == 0 && Machine->drv->color_table_len == 0 && Machine->gfx[0] == NULL && tilemap_count() == 0)
		goto cancel;

	/* if we're not paused, mark the bitmap dirty */
	if (!mame_is_paused())
		state->bitmap_dirty = TRUE;

	/* switch off the state to display something */
again:
	switch (state->mode)
	{
		case 0:
			/* if we have a palette, display it */
			if (Machine->drv->total_colors > 0)
			{
				showgfx_palette_handler(state);
				break;
			}

			/* fall through...*/
			state->mode++;

		case 1:
			/* if we have graphics sets, display them */
			if (Machine->gfx[0] != NULL)
			{
				showgfx_gfxset_handler(state);
				break;
			}

			/* fall through...*/
			state->mode++;

		case 2:
			/* if we have tilemaps, display them */
			if (tilemap_count() > 0)
			{
				showgfx_tilemap_handler(state);
				break;
			}

			state->mode = 0;
			goto again;
	}

	/* handle keys */
	if (input_ui_pressed(IPT_UI_SELECT))
	{
		state->mode = (state->mode + 1) % 3;
		state->bitmap_dirty = TRUE;
	}

	if (input_ui_pressed(IPT_UI_PAUSE))
		mame_pause(!mame_is_paused());

	if (input_ui_pressed(IPT_UI_CANCEL) || input_ui_pressed(IPT_UI_SHOW_GFX))
		goto cancel;

	return uistate;

cancel:
	if (!uistate)
		mame_pause(FALSE);
	state->bitmap_dirty = TRUE;
	return UI_HANDLER_CANCEL;
}
#endif




#ifndef NEW_RENDER
static void showcharset(mame_bitmap *bitmap)
{
	int i;
	char buf[80];
	int mode,bank,color,firstdrawn;
	int palpage;
	int changed;
	int total_colors = 0;
	pen_t *colortable = NULL;
	int cpx=0,cpy,skip_chars=0,skip_tmap=0;
	int tilemap_xpos = 0;
	int tilemap_ypos = 0;

	mode = 0;
	bank = 0;
	color = 0;
	firstdrawn = 0;
	palpage = 0;

	changed = 1;

	/* mark all the tilemaps dirty on entry so they are re-drawn consistently in the viewer */
	tilemap_mark_all_tiles_dirty(NULL);

	do
	{
		static const rectangle fullrect = { 0, 10000, 0, 10000 };
		int back = ui_bgcolor;

		ui_bgcolor = ARGB_BLACK;

		/* mark the whole thing dirty */
		artwork_mark_ui_dirty(fullrect.min_x, fullrect.min_y, fullrect.max_x, fullrect.max_y);
		ui_dirty = 5;

		switch (mode)
		{
			case 0: /* palette or clut */
			{
				if (bank == 0)	/* palette */
				{
					total_colors = Machine->drv->total_colors;
					colortable = Machine->pens;
					strcpy(buf,"PALETTE");
				}
				else if (bank == 1)	/* clut */
				{
					total_colors = Machine->drv->color_table_len;
					colortable = Machine->remapped_colortable;
					strcpy(buf,"CLUT");
				}
				else
				{
					buf[0] = 0;
					total_colors = 0;
					colortable = 0;
				}

				/*if (changed) -- temporary */
				{
					erase_screen(bitmap);

					if (total_colors)
					{
						int sx,sy,colors;
						int column_heading_max;
						struct bounds;

						colors = total_colors - 256 * palpage;
						if (colors > 256) colors = 256;

						/* min(colors, 16) */
						if (colors < 16)
							column_heading_max = colors;
						else
							column_heading_max = 16;

						for (i = 0;i < column_heading_max;i++)
						{
							char bf[40];

							sx = 3*uirotcharwidth + (uirotcharwidth*4/3)*(i % 16);
							sprintf(bf,"%X",i);
							ui_draw_text(bf,sx,2*uirotcharheight);
							if (16*i < colors)
							{
								sy = 3*uirotcharheight + (uirotcharheight)*(i % 16);
								sprintf(bf,"%3X",i+16*palpage);
								ui_draw_text(bf,0,sy);
							}
						}

						for (i = 0;i < colors;i++)
						{
							rectangle bounds;
							bounds.min_x = uirotbounds.min_x + 3*uirotcharwidth + (uirotcharwidth*4/3)*(i % 16);
							bounds.min_y = uirotbounds.min_y + 2*uirotcharheight + (uirotcharheight)*(i / 16) + uirotcharheight;
							bounds.max_x = bounds.min_x + uirotcharwidth*4/3 - 1;
							bounds.max_y = bounds.min_y + uirotcharheight - 1;
							ui_rot2raw_rect(&bounds);
							fillbitmap(bitmap, colortable[i + 256*palpage], &bounds);
						}
					}
					else
						ui_draw_text("N/A",3*uirotcharwidth,2*uirotcharheight);

					ui_draw_text(buf,0,0);
					changed = 0;
				}

				break;
			}
			case 1: /* characters */
			{
				int crotwidth = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? Machine->gfx[bank]->height : Machine->gfx[bank]->width;
				int crotheight = (Machine->ui_orientation & ORIENTATION_SWAP_XY) ? Machine->gfx[bank]->width : Machine->gfx[bank]->height;
				cpx = uirotwidth / crotwidth;
				if (cpx == 0) cpx = 1;
				cpy = (uirotheight - uirotcharheight) / crotheight;
				if (cpy == 0) cpy = 1;
				skip_chars = cpx * cpy;
				/*if (changed) -- temporary */
				{
					int flipx,flipy;
					int lastdrawn=0;

					erase_screen(bitmap);

					/* validity check after char bank change */
					if (firstdrawn >= Machine->gfx[bank]->total_elements)
					{
						firstdrawn = Machine->gfx[bank]->total_elements - skip_chars;
						if (firstdrawn < 0) firstdrawn = 0;
					}

					flipx = 0;
					flipy = 0;

					for (i = 0; i+firstdrawn < Machine->gfx[bank]->total_elements && i<cpx*cpy; i++)
					{
						rectangle bounds;
						bounds.min_x = (i % cpx) * crotwidth + uirotbounds.min_x;
						bounds.min_y = uirotcharheight + (i / cpx) * crotheight + uirotbounds.min_y;
						bounds.max_x = bounds.min_x + crotwidth - 1;
						bounds.max_y = bounds.min_y + crotheight - 1;
						ui_rot2raw_rect(&bounds);

						drawgfx(bitmap,Machine->gfx[bank],
								i+firstdrawn,color,  /*sprite num, color*/
								flipx,flipy,bounds.min_x,bounds.min_y,
								0,Machine->gfx[bank]->colortable ? TRANSPARENCY_NONE : TRANSPARENCY_NONE_RAW,0);

						lastdrawn = i+firstdrawn;
					}

					sprintf(buf,"GFXSET %d COLOR %2X CODE %X-%X",bank,color,firstdrawn,lastdrawn);
					ui_draw_text(buf,0,0);
					changed = 0;
				}

				break;
			}
			case 2: /* Tilemaps */
			{
				/*if (changed) -- temporary */
				{
					UINT32 tilemap_width, tilemap_height;
					tilemap_nb_size (bank, &tilemap_width, &tilemap_height);
					while (tilemap_xpos < 0)
						tilemap_xpos += tilemap_width;
					tilemap_xpos %= tilemap_width;

					while (tilemap_ypos < 0)
						tilemap_ypos += tilemap_height;
					tilemap_ypos %= tilemap_height;

					erase_screen(bitmap);
					tilemap_nb_draw (bitmap, bank, tilemap_xpos, tilemap_ypos);
					sprintf(buf, "TILEMAP %d (%dx%d)  X:%d  Y:%d", bank, tilemap_width, tilemap_height, tilemap_xpos, tilemap_ypos);
					ui_draw_text(buf,0,0);
					changed = 0;
					skip_tmap = 0;
				}
				break;
			}
		}

		ui_bgcolor = back;

		render_ui(bitmap);
		update_video_and_audio();

		if (code_pressed(KEYCODE_LCONTROL) || code_pressed(KEYCODE_RCONTROL))
		{
			skip_chars = cpx;
			skip_tmap = 8;
		}
		if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		{
			skip_chars = 1;
			skip_tmap = 1;
		}


		if (input_ui_pressed_repeat(IPT_UI_RIGHT,6))
		{
			int next_bank, next_mode;
			int jumped;

			next_mode = mode;
			next_bank = bank+1;
			do {
				jumped = 0;
				switch (next_mode)
				{
					case 0:
						if (next_bank == 2 || Machine->drv->color_table_len == 0)
						{
							jumped = 1;
							next_mode++;
							next_bank = 0;
						}
						break;
					case 1:
						if (next_bank == MAX_GFX_ELEMENTS || !Machine->gfx[next_bank])
						{
							jumped = 1;
							next_mode++;
							next_bank = 0;
						}
						break;
					case 2:
						if (next_bank == tilemap_count())
							next_mode = -1;
						break;
				}
			}	while (jumped);
			if (next_mode != -1 )
			{
				bank = next_bank;
				mode = next_mode;
//              firstdrawn = 0;
				changed = 1;
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_LEFT,6))
		{
			int next_bank, next_mode;

			next_mode = mode;
			next_bank = bank-1;
			while(next_bank < 0 && next_mode >= 0)
			{
				next_mode = next_mode - 1;
				switch (next_mode)
				{
					case 0:
						if (Machine->drv->color_table_len == 0)
							next_bank = 0;
						else
							next_bank = 1;
						break;
					case 1:
						next_bank = MAX_GFX_ELEMENTS-1;
						while (next_bank >= 0 && !Machine->gfx[next_bank])
							next_bank--;
						break;
					case 2:
						next_bank = tilemap_count() - 1;
						break;
				}
			}
			if (next_mode != -1 )
			{
				bank = next_bank;
				mode = next_mode;
//              firstdrawn = 0;
				changed = 1;
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_PGDN,4))
		{
			switch (mode)
			{
				case 0:
				{
					if (256 * (palpage + 1) < total_colors)
					{
						palpage++;
						changed = 1;
					}
					break;
				}
				case 1:
				{
					if (firstdrawn + skip_chars < Machine->gfx[bank]->total_elements)
					{
						firstdrawn += skip_chars;
						changed = 1;
					}
					break;
				}
				case 2:
				{
					if (skip_tmap)
						tilemap_ypos -= skip_tmap;
					else
						tilemap_ypos -= bitmap->height/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_PGUP,4))
		{
			switch (mode)
			{
				case 0:
				{
					if (palpage > 0)
					{
						palpage--;
						changed = 1;
					}
					break;
				}
				case 1:
				{
					firstdrawn -= skip_chars;
					if (firstdrawn < 0) firstdrawn = 0;
					changed = 1;
					break;
				}
				case 2:
				{
					if (skip_tmap)
						tilemap_ypos += skip_tmap;
					else
						tilemap_ypos += bitmap->height/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_D,4))
		{
			switch (mode)
			{
				case 2:
				{
					if (skip_tmap)
						tilemap_xpos -= skip_tmap;
					else
						tilemap_xpos -= bitmap->width/4;
					changed = 1;
					break;
				}
			}
		}

		if (code_pressed_memory_repeat(KEYCODE_G,4))
		{
			switch (mode)
			{
				case 2:
				{
					if (skip_tmap)
						tilemap_xpos += skip_tmap;
					else
						tilemap_xpos += bitmap->width/4;
					changed = 1;
					break;
				}
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_UP,6))
		{
			switch (mode)
			{
				case 1:
				{
					if (color < Machine->gfx[bank]->total_colors - 1)
					{
						color++;
						changed = 1;
					}
					break;
				}
			}
		}

		if (input_ui_pressed_repeat(IPT_UI_DOWN,6))
		{
			switch (mode)
			{
				case 0:
					break;
				case 1:
				{
					if (color > 0)
					{
						color--;
						changed = 1;
					}
				}
			}
		}

		if (input_ui_pressed(IPT_UI_SNAPSHOT))
			save_screen_snapshot(bitmap);

#ifdef INP_CAPTION
		draw_caption();
#endif /* INP_CAPTION */
	} while (!input_ui_pressed(IPT_UI_SHOW_GFX) &&
			!input_ui_pressed(IPT_UI_CANCEL) &&
			!mame_is_scheduled_event_pending());

	schedule_full_refresh();

	/* mark all the tilemaps dirty on exit so they are updated correctly on the next frame */
	tilemap_mark_all_tiles_dirty(NULL);
}
#endif


static UINT32 ui_handler_font_warning(UINT32 state)
{
	/* DO NOT tlanslate this message */
	static const char *font_warning_string =
		"Local font file is not installed. "
		"You must install CJK font into font directory first.\n\n"
		"Please download from:\n"
		"http://mameplus.emu-france.com/\n\n"
		"Press ESC to exit, type OK to continue.";
	int res;

	ui_draw_message_window_scroll(font_warning_string);

	res = ui_window_scroll_keys();
	if (res == 0)
	{
		/* an 'O' or left joystick kicks us to the next state */
		if (code_pressed_memory(KEYCODE_O) || input_ui_pressed(IPT_UI_LEFT))
			return 1;

		/* a 'K' or right joystick exits the state */
		if (state == 1 && (code_pressed_memory(KEYCODE_K) || input_ui_pressed(IPT_UI_RIGHT)))
			return 1000;
	}

	/* if the user cancels, exit out completely */
	//if (input_ui_pressed(IPT_UI_CANCEL))
	if (res == 2)
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	return state;
}


static UINT32 ui_handler_disclaimer(UINT32 state)
{
	int res;

	if (giant_string_buffer[0] == 0)
	{
		char *bufptr = giant_string_buffer;

		bufptr += sprintf(bufptr, "%s\n\n", ui_getstring(UI_copyright1));
		bufptr += sprintf(bufptr, ui_getstring(UI_copyright2), options.use_lang_list ? _LST(Machine->gamedrv->description) : Machine->gamedrv->description);
		bufptr += sprintf(bufptr, "\n\n%s", ui_getstring(UI_copyright3));
	}

	ui_draw_message_window_scroll(giant_string_buffer);

	res = ui_window_scroll_keys();
	if (res == 0)
	{
		/* an 'O' or left joystick kicks us to the next state */
		if (code_pressed_memory(KEYCODE_O) || input_ui_pressed(IPT_UI_LEFT))
			return 1;

		/* a 'K' or right joystick exits the state */
		if (state == 1 && (code_pressed_memory(KEYCODE_K) || input_ui_pressed(IPT_UI_RIGHT)))
			return 1000;
	}

	/* if the user cancels, exit out completely */
	//if (input_ui_pressed(IPT_UI_CANCEL))
	if (res == 2)
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	return state;
}


static UINT32 ui_handler_warnings(UINT32 state)
{
#define WARNING_FLAGS (	GAME_NOT_WORKING | \
						GAME_UNEMULATED_PROTECTION | \
						GAME_WRONG_COLORS | \
						GAME_IMPERFECT_COLORS | \
						GAME_NO_SOUND |  \
						GAME_IMPERFECT_SOUND |  \
						GAME_IMPERFECT_GRAPHICS | \
						GAME_NO_COCKTAIL)
	int i;
	int res;

	if (rom_load_warnings() == 0 && !(Machine->gamedrv->flags & WARNING_FLAGS))
		return 1000;

	if (giant_string_buffer[0] == 0)
	{
		char *bufptr = giant_string_buffer;

		if (rom_load_warnings() > 0)
		{
			bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_incorrectroms));
			if (Machine->gamedrv->flags & WARNING_FLAGS)
				*bufptr++ = '\n';
		}

		if (Machine->gamedrv->flags & WARNING_FLAGS)
		{
			bufptr += sprintf(bufptr, "%s\n\n", ui_getstring(UI_knownproblems));

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

			if (Machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
			{
				const game_driver *maindrv;
				const game_driver *clone_of;
				int foundworking;

				if (Machine->gamedrv->flags & GAME_NOT_WORKING)
					bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_brokengame));
				if (Machine->gamedrv->flags & GAME_UNEMULATED_PROTECTION)
					bufptr += sprintf(bufptr, "%s\n", ui_getstring(UI_brokenprotection));

				clone_of = driver_get_clone(Machine->gamedrv);
				if (clone_of != NULL && !(clone_of->flags & NOT_A_DRIVER))
	 				maindrv = clone_of;
				else
					maindrv = Machine->gamedrv;

				foundworking = 0;
				i = 0;
				while (drivers[i])
				{
					if (drivers[i] == maindrv || driver_get_clone(drivers[i]) == maindrv)
					{
						if ((drivers[i]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) == 0)
						{
							if (foundworking == 0)
								bufptr += sprintf(bufptr, "\n\n%s\n\n", ui_getstring(UI_workingclones));
							bufptr += sprintf(bufptr, "%s\n", drivers[i]->name);
							foundworking = 1;
						}
					}
					i++;
				}
			}
		}

		bufptr += sprintf(bufptr, "\n\n%s", ui_getstring(UI_typeok));
	}

	ui_draw_message_window_scroll(giant_string_buffer);

	res = ui_window_scroll_keys();
	if (res == 0)
	{
		/* an 'O' or left joystick kicks us to the next state */
		if (code_pressed_memory(KEYCODE_O) || input_ui_pressed(IPT_UI_LEFT))
			return 1;

		/* a 'K' or right joystick exits the state */
		if (state == 1 && (code_pressed_memory(KEYCODE_K) || input_ui_pressed(IPT_UI_RIGHT)))
			return 1000;
	}

	/* if the user cancels, exit out completely */
	//if (input_ui_pressed(IPT_UI_CANCEL))
	if (res == 2)
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	return state;
}


static UINT32 ui_handler_gameinfo(UINT32 state)
{
	int res;

	if (giant_string_buffer[0] == 0)
	{
		char *bufptr = giant_string_buffer;

		/* state 0 is the standard game info */
		if (state == 0)
		{
			/* add the game info */
			bufptr += sprintf_game_info(bufptr);

			/* append MAME version and ask for select key */
			bufptr += sprintf(bufptr, "\n\t%s %s\n\t%s", ui_getstring(UI_mame), build_version, ui_getstring(UI_selectkey));
		}

		/* state 1 is the image info for MESS */
		else
		{
#ifdef MESS
			/* add the game info */
			bufptr += ui_sprintf_image_info(bufptr);
#endif
		}
	}

	/* draw the window */
	ui_draw_message_window_scroll(giant_string_buffer);

	res = ui_window_scroll_keys();

	/* allow cancelling */
	//if (input_ui_pressed(IPT_UI_CANCEL))
	if (res == 2)
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	/* advance to the next state */
	if (code_read_async() != CODE_NONE)
	{
		if (res == 1)
		{
			state++;
			giant_string_buffer[0] = 0;
#ifdef MESS
			if (state >= 2)
#else
			if (state >= 1)
#endif
				return 1000;
		}
	}

	return state;
}







/*********************************************************************

  start of On Screen Display handling

*********************************************************************/

/*************************************
 *
 *  OSD handler
 *
 *************************************/

static UINT32 ui_handler_osd(UINT32 state)
{
	int increment = 0;

	if (input_ui_pressed_repeat(IPT_UI_LEFT,6))
		increment = -1;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT,6))
		increment = 1;

	if (input_ui_pressed_repeat(IPT_UI_DOWN,6))
		onscrd_state = (onscrd_state + 1) % onscrd_total_items;
	if (input_ui_pressed_repeat(IPT_UI_UP,6))
		onscrd_state = (onscrd_state + onscrd_total_items - 1) % onscrd_total_items;

	(*onscrd_fnc[onscrd_state])(increment, onscrd_arg[onscrd_state]);

	if (input_ui_pressed(IPT_UI_ON_SCREEN_DISPLAY) || input_ui_pressed(IPT_UI_CANCEL))
		return UI_HANDLER_CANCEL;
	if (input_ui_pressed(IPT_UI_CONFIGURE))
		ui_set_handler(ui_handler_menu, 0);

	return 0;
}


/*-------------------------------------------------
    drawbar - draw a thermometer bar
-------------------------------------------------*/

static void drawbar(int leftx, int topy, int width, int height, float percentage, float default_percentage)
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



static void displayosd(const char *text, int minval, int maxval, int defval, int curval)
{
	float percentage = (float)(curval - minval) / (float)(maxval - minval);
	float default_percentage = (float)(defval - minval) / (float)(maxval - minval);
	int space_width = ui_get_char_width(' ');
	int line_height = ui_get_line_height();
	int ui_width, ui_height;
	int text_height;

	/* get our UI bounds */
	ui_get_bounds(&ui_width, &ui_height);

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
	drawbar(2 * space_width, line_height + ui_height - UI_BOX_TB_BORDER - text_height - line_height*3/4,
			ui_width - 2 * space_width, line_height*3/4, percentage, default_percentage);

	/* draw the actual text */
	ui_draw_text_full(text, space_width + UI_BOX_LR_BORDER, line_height + ui_height - UI_BOX_TB_BORDER - text_height, ui_width - 2 * UI_BOX_LR_BORDER, 0, text_height / ui_get_line_height(),
				JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);
}

static void onscrd_adjuster(int increment,int arg)
{
	input_port_entry *in = &Machine->input_ports[arg];
	char buf[80];
	int value;

	if (increment)
	{
		value = in->default_value & 0xff;
		value += increment;
		if (value > 100) value = 100;
		if (value < 0) value = 0;
		in->default_value = (in->default_value & ~0xff) | value;
	}
	value = in->default_value & 0xff;

	sprintf(buf,"%s %d%%",_(in->name),value);

	displayosd(buf, 0, 100, in->default_value >> 8, value);
}

static void onscrd_volume(int increment,int arg)
{
	char buf[20];
	int attenuation;

	if (increment)
	{
		attenuation = osd_get_mastervolume();
		attenuation += increment;
		if (attenuation > 0) attenuation = 0;
		if (attenuation < -32) attenuation = -32;
		osd_set_mastervolume(attenuation);
	}
	attenuation = osd_get_mastervolume();

	sprintf(buf,"%s %3ddB", ui_getstring (UI_volume), attenuation);
	displayosd(buf, -32, 0, 0, attenuation);
}

static void onscrd_mixervol(int increment,int arg)
{
	static void *driver = 0;
	char buf[40];
	float volume;
	int ch;
	int doallchannels = 0;
	int proportional = 0;


	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		doallchannels = 1;
	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;
	if (code_pressed(KEYCODE_LALT) || code_pressed(KEYCODE_RALT))
		proportional = 1;

	if (increment)
	{
		if (proportional)
		{
			static float old_vol[100];
			int num_vals = sound_get_user_gain_count();
			float ratio = 1.0;
			int overflow = 0;

			if (driver != Machine->drv)
			{
				driver = (void *)Machine->drv;
				for (ch = 0; ch < num_vals; ch++)
					old_vol[ch] = sound_get_user_gain(ch);
			}

			volume = sound_get_user_gain(arg);
			if (old_vol[arg])
				ratio = (volume + increment * 0.02) / old_vol[arg];

			for (ch = 0; ch < num_vals; ch++)
			{
				volume = ratio * old_vol[ch];
				if (volume < 0 || volume > 2.0)
					overflow = 1;
			}

			if (!overflow)
			{
				for (ch = 0; ch < num_vals; ch++)
				{
					volume = ratio * old_vol[ch];
					sound_set_user_gain(ch,volume);
				}
			}
		}
		else
		{
			driver = 0; /* force reset of saved volumes */

			volume = sound_get_user_gain(arg);
			volume += increment * 0.02;
			if (volume > 2.0) volume = 2.0;
			if (volume < 0) volume = 0;

			if (doallchannels)
			{
				int num_vals = sound_get_user_gain_count();
				for (ch = 0;ch < num_vals;ch++)
					sound_set_user_gain(ch,volume);
			}
			else
				sound_set_user_gain(arg,volume);
		}
	}
	volume = sound_get_user_gain(arg);

	if (proportional)
		sprintf(buf,"%s %s %4.2f", ui_getstring (UI_allchannels), ui_getstring (UI_relative), volume);
	else if (doallchannels)
		sprintf(buf,"%s %s %4.2f", ui_getstring (UI_allchannels), ui_getstring (UI_volume), volume);
	else
		sprintf(buf,"%s %s %4.2f",sound_get_user_gain_name(arg), ui_getstring (UI_volume), volume);
	displayosd(buf, 0, 200, sound_get_default_gain(arg) * 100, volume * 100);
}

static void onscrd_brightness(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int brightness;

	if (increment)
	{
		brightness = floor(render_container_get_brightness(container) * 1000.0f + 0.5f);
		brightness += 10 * increment;
		if (brightness < 800) brightness = 800;
		if (brightness > 1200) brightness = 1200;
		render_container_set_brightness(container, (float)brightness / 1000.0f);
	}
	brightness = floor(render_container_get_brightness(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %3d%%", arg, ui_getstring (UI_brightness), brightness / 10);
	else
		sprintf(buf,"%s %3d%%", ui_getstring (UI_brightness), brightness / 10);
	displayosd(buf, 800, 1200, 1000, brightness);
}

static void onscrd_contrast(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int contrast;

	if (increment)
	{
		contrast = floor(render_container_get_contrast(container) * 1000.0f + 0.5f);
		contrast += 50 * increment;
		if (contrast < 100) contrast = 100;
		if (contrast > 2000) contrast = 2000;
		render_container_set_contrast(container, (float)contrast / 1000.0f);
	}
	contrast = floor(render_container_get_contrast(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %3d%%", arg, ui_getstring (UI_contrast), contrast / 10);
	else
		sprintf(buf,"%s %3d%%", ui_getstring (UI_contrast), contrast / 10);
	displayosd(buf, 100, 2000, 1000, contrast);
}

static void onscrd_gamma(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int gamma;

	if (increment)
	{
		gamma = floor(render_container_get_gamma(container) * 1000.0f + 0.5f);
		gamma += 50 * increment;
		if (gamma < 500) gamma = 500;
		if (gamma > 3000) gamma = 3000;
		render_container_set_gamma(container, (float)gamma / 1000.0f);
	}
	gamma = floor(render_container_get_gamma(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %4.2f", arg, ui_getstring (UI_gamma), (double)gamma / 1000.0f);
	else
		sprintf(buf,"%s %4.2f", ui_getstring (UI_gamma), (double)gamma / 1000.0f);
	displayosd(buf, 500, 3000, 1000, gamma);
}

static void onscrd_xscale(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int xscale;

	if (increment)
	{
		xscale = floor(render_container_get_xscale(container) * 1000.0f + 0.5f);
		xscale += 2 * increment;
		if (xscale < 800) xscale = 800;
		if (xscale > 1200) xscale = 1200;
		render_container_set_xscale(container, (float)xscale / 1000.0f);
	}
	xscale = floor(render_container_get_xscale(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %5.3f", arg, "Horiz stretch", (double)xscale / 1000.0f);
	else
		sprintf(buf,"%s %5.3f", "Horiz stretch", (double)xscale / 1000.0f);
	displayosd(buf, 800, 1200, 1000, xscale);
}

static void onscrd_yscale(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int yscale;

	if (increment)
	{
		yscale = floor(render_container_get_yscale(container) * 1000.0f + 0.5f);
		yscale += 2 * increment;
		if (yscale < 800) yscale = 800;
		if (yscale > 1200) yscale = 1200;
		render_container_set_yscale(container, (float)yscale / 1000.0f);
	}
	yscale = floor(render_container_get_yscale(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %5.3f", arg, "Vert stretch", (double)yscale / 1000.0f);
	else
		sprintf(buf,"%s %5.3f", "Vert stretch", (double)yscale / 1000.0f);
	displayosd(buf, 800, 1200, 1000, yscale);
}

static void onscrd_xoffset(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int xoffset;

	if (increment)
	{
		xoffset = floor(render_container_get_xoffset(container) * 1000.0f + 0.5f);
		xoffset += 2 * increment;
		if (xoffset < -200) xoffset = -200;
		if (xoffset > 200) xoffset = 200;
		render_container_set_xoffset(container, (float)xoffset / 1000.0f);
	}
	xoffset = floor(render_container_get_xoffset(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %5.3f", arg, "Horiz position", (double)xoffset / 1000.0f);
	else
		sprintf(buf,"%s %5.3f", "Horiz position", (double)xoffset / 1000.0f);
	displayosd(buf, -200, 200, 0, xoffset);
}

static void onscrd_yoffset(int increment,int arg)
{
	render_container *container = render_container_get_screen(arg);
	char buf[40];
	int yoffset;

	if (increment)
	{
		yoffset = floor(render_container_get_yoffset(container) * 1000.0f + 0.5f);
		yoffset += 2 * increment;
		if (yoffset < -200) yoffset = -200;
		if (yoffset > 200) yoffset = 200;
		render_container_set_yoffset(container, (float)yoffset / 1000.0f);
	}
	yoffset = floor(render_container_get_yoffset(container) * 1000.0f + 0.5f);

	if (Machine->drv->screen[1].tag != NULL)
		sprintf(buf,"Screen %d %s %5.3f", arg, "Vert position", (double)yoffset / 1000.0f);
	else
		sprintf(buf,"%s %5.3f", "Vert position", (double)yoffset / 1000.0f);
	displayosd(buf, -200, 200, 0, yoffset);
}

static void onscrd_vector_flicker(int increment,int arg)
{
	char buf[1000];
	float flicker_correction;

	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;

	if (increment)
	{
		flicker_correction = vector_get_flicker();

		flicker_correction += increment;
		if (flicker_correction < 0.0) flicker_correction = 0.0;
		if (flicker_correction > 100.0) flicker_correction = 100.0;

		vector_set_flicker(flicker_correction);
	}
	flicker_correction = vector_get_flicker();

	sprintf(buf,"%s %1.2f", ui_getstring (UI_vectorflicker), flicker_correction);
	displayosd(buf, 0, 100, 0, flicker_correction);
}

static void onscrd_overclock(int increment,int arg)
{
	char buf[30];
	double overclock;
	int cpu, doallcpus = 0, oc;

	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		doallcpus = 1;
	if (!code_pressed(KEYCODE_LCONTROL) && !code_pressed(KEYCODE_RCONTROL))
		increment *= 5;
	if( increment )
	{
		overclock = cpunum_get_clockscale(arg);
		overclock += 0.01 * increment;
		if (overclock < 0.01) overclock = 0.01;
		if (overclock > 4.0) overclock = 4.0;
		if( doallcpus )
			for( cpu = 0; cpu < cpu_gettotalcpu(); cpu++ )
				cpunum_set_clockscale(cpu, overclock);
		else
			cpunum_set_clockscale(arg, overclock);
	}

	oc = 100 * cpunum_get_clockscale(arg) + 0.5;

	if( doallcpus )
		sprintf(buf,"%s %s %3d%%", ui_getstring (UI_allcpus), ui_getstring (UI_overclock), oc);
	else
		sprintf(buf,"%s %s%d %3d%%", ui_getstring (UI_overclock), ui_getstring (UI_cpu), arg, oc);
	displayosd(buf, 1, 400, 100, oc);
}

static void onscrd_refresh(int increment,int arg)
{
	float delta = Machine->refresh_rate[0] - Machine->drv->screen[0].refresh_rate;
	char buf[30];

	increment *= 1000;
	if (code_pressed(KEYCODE_LSHIFT) || code_pressed(KEYCODE_RSHIFT))
		increment /= 10;
	if (code_pressed(KEYCODE_LCONTROL) || code_pressed(KEYCODE_RCONTROL))
		increment /= 100;
	if (code_pressed(KEYCODE_LALT) || code_pressed(KEYCODE_LALT))
		increment /= 1000;
	if (increment)
	{
		float newrate;
		delta += 0.001 * increment;
		if (delta > 10)
			delta = 10;
		if (delta < -10)
			delta = -10;

		newrate = Machine->drv->screen[0].refresh_rate;
		if (delta != 0)
			newrate = (floor(newrate * 1000) / 1000) + delta;
		set_refresh_rate(0, newrate);
	}

	sprintf(buf,"%s %.3f", ui_getstring (UI_refresh_rate), Machine->refresh_rate[0]);
	displayosd(buf, -10, 10, 0, delta);
}

static void onscrd_init(void)
{
	input_port_entry *in;
	int item,ch;
	int scrnum;

	item = 0;

	{
		int num_vals = sound_get_user_gain_count();
		onscrd_fnc[item] = onscrd_volume;
		onscrd_arg[item] = 0;
		item++;

		for (ch = 0;ch < num_vals;ch++)
		{
			onscrd_fnc[item] = onscrd_mixervol;
			onscrd_arg[item] = ch;
			item++;
		}
	}

	for (in = Machine->input_ports; in && in->type != IPT_END; in++)
		if ((in->type & 0xff) == IPT_ADJUSTER)
		{
			onscrd_fnc[item] = onscrd_adjuster;
			onscrd_arg[item] = in - Machine->input_ports;
			item++;
		}

	if (options.cheat)
	{
		for (ch = 0;ch < cpu_gettotalcpu();ch++)
		{
			onscrd_fnc[item] = onscrd_overclock;
			onscrd_arg[item] = ch;
			item++;
		}
		onscrd_fnc[item] = onscrd_refresh;
		onscrd_arg[item] = ch;
		item++;
	}

	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (Machine->drv->screen[scrnum].tag != NULL)
		{
			onscrd_fnc[item] = onscrd_brightness;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_contrast;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_gamma;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_xscale;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_yscale;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_xoffset;
			onscrd_arg[item] = scrnum;
			item++;

			onscrd_fnc[item] = onscrd_yoffset;
			onscrd_arg[item] = scrnum;
			item++;
		}

	if (Machine->drv->video_attributes & VIDEO_TYPE_VECTOR)
	{
		onscrd_fnc[item] = onscrd_vector_flicker;
		onscrd_arg[item] = 0;
		item++;
	}

	onscrd_total_items = item;
}


/*********************************************************************

  end of On Screen Display handling

*********************************************************************/

static void initiate_load_save(int type)
{
	ui_set_handler(update_load_save, type);
	mame_pause(TRUE);
}


static UINT32 update_load_save(UINT32 state)
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
			ui_popup(_("Save cancelled"));
		else
			ui_popup(_("Load cancelled"));

		/* reset the state */
		mame_pause(FALSE);
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
	sprintf(filename, "%s-%c", Machine->gamedrv->name, file);
	if (state == LOADSAVE_SAVE)
	{
		ui_popup(_("Save to position %c"), file);
		mame_schedule_save(filename);
	}
	else
	{
		ui_popup(_("Load from position %c"), file);
		mame_schedule_load(filename);
	}

	/* remove the pause and reset the state */
	mame_pause(FALSE);
	return UI_HANDLER_CANCEL;
}


static UINT32 ui_handler_confirm_quit(UINT32 state)
{
	const char *quit_message =
		"Quit the game?\n\n"
		"Press Select key/button to quit,\n"
		"Cancel key/button to continue.";

	if (!options.confirm_quit)
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	ui_draw_message_window(_(quit_message));

	if (input_ui_pressed(IPT_UI_SELECT))
	{
		mame_schedule_exit();
		return UI_HANDLER_CANCEL;
	}

	if (input_ui_pressed(IPT_UI_CANCEL))
	{
		return UI_HANDLER_CANCEL;
	}

	return 0;
}


void ui_auto_pause(void)
{
	auto_pause = 1;
}


void ui_display_fps(void)
{
	int ui_width, ui_height;

	/* if we're not currently displaying, skip it */
	if (!showfps && !showfpstemp)
		return;

	ui_get_bounds(&ui_width, &ui_height);

	/* get the current FPS text */
	ui_draw_text_full(osd_get_fps_text(mame_get_performance_info()), 0, 0, ui_width, 0, 0,
				JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);

	/* update the temporary FPS display state */
	if (showfpstemp)
	{
		showfpstemp--;
		if (!showfps && showfpstemp == 0)
			schedule_full_refresh();
	}
}

static void ui_display_profiler(void)
{
	int ui_width, ui_height;

	if (show_profiler)
	{
		ui_get_bounds(&ui_width, &ui_height);
		ui_draw_text_full(profiler_get_text(), 0, 0, ui_width, 0, 0, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ui_bgcolor, NULL, NULL);
	}
}

static void ui_display_popup(void)
{
	/* show popup message if any */
	if (popup_text_counter > 0)
	{
		draw_multiline_text_box(popup_text, 0, JUSTIFY_CENTER, 0.5f, 0.9f);

		if (--popup_text_counter == 0)
			schedule_full_refresh();
	}
}






/*************************************
 *
 *  Temporary rendering system
 *
 *************************************/

#ifndef NEW_RENDER

void ui_get_bounds(int *width, int *height)
{
	*width = uirotwidth;
	*height = uirotheight;
}


int ui_get_line_height(void)
{
	return uirotcharheight + 2;
}


int ui_get_char_width(UINT16 ch)
{
	if (ch > 0x00ff)
		return uirotcharwidth * 2;

	return uirotcharwidth;
}


static void ui_raw2rot_rect(rectangle *rect)
{
	int temp, w, h;

	/* get the effective screen size, including artwork */
	artwork_get_screensize(&w, &h);

	/* apply X flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		temp = w - rect->min_x - 1;
		rect->min_x = w - rect->max_x - 1;
		rect->max_x = temp;
	}

	/* apply Y flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		temp = h - rect->min_y - 1;
		rect->min_y = h - rect->max_y - 1;
		rect->max_y = temp;
	}

	/* apply X/Y swap first */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		temp = rect->min_x; rect->min_x = rect->min_y; rect->min_y = temp;
		temp = rect->max_x; rect->max_x = rect->max_y; rect->max_y = temp;
	}
}


static void ui_rot2raw_rect(rectangle *rect)
{
	int temp, w, h;

	/* get the effective screen size, including artwork */
	artwork_get_screensize(&w, &h);

	/* apply X/Y swap first */
	if (Machine->ui_orientation & ORIENTATION_SWAP_XY)
	{
		temp = rect->min_x; rect->min_x = rect->min_y; rect->min_y = temp;
		temp = rect->max_x; rect->max_x = rect->max_y; rect->max_y = temp;
	}

	/* apply X flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_X)
	{
		temp = w - rect->min_x - 1;
		rect->min_x = w - rect->max_x - 1;
		rect->max_x = temp;
	}

	/* apply Y flip */
	if (Machine->ui_orientation & ORIENTATION_FLIP_Y)
	{
		temp = h - rect->min_y - 1;
		rect->min_y = h - rect->max_y - 1;
		rect->max_y = temp;
	}
}


static void add_line(int x1, int y1, int x2, int y2, rgb_t color)
{
	if (elemindex < ARRAY_LENGTH(elemlist))
	{
		elemlist[elemindex].x = (x1 < x2) ? x1 : x2;
		elemlist[elemindex].y = (y1 < y2) ? y1 : y2;
		elemlist[elemindex].x2 = (x1 < x2) ? x2 : x1;
		elemlist[elemindex].y2 = (y1 < y2) ? y2 : y1;
		elemlist[elemindex].type = 0xfffe;
		elemlist[elemindex].color = color;
		elemindex++;
	}
}


static void add_fill(int left, int top, int right, int bottom, rgb_t color)
{
	add_line(left, top, right, bottom, color);
}


static void add_char(int x, int y, UINT16 ch, int color)
{
	if (elemindex < ARRAY_LENGTH(elemlist))
	{
		elemlist[elemindex].x = x;
		elemlist[elemindex].y = y;
		elemlist[elemindex].type = ch;
		elemlist[elemindex].color = color;
		elemindex++;
	}
}


static void render_ui(mame_bitmap *dest)
{
	int ui_need_scroll = FALSE;
	int i;

#ifndef UI_COLOR_DISPLAY
	uifont_colortable[0] = get_black_pen();
	uifont_colortable[1] = get_white_pen();
	uifont_colortable[2] = get_white_pen();
	uifont_colortable[3] = get_black_pen();
#endif /* !UI_COLOR_DISPLAY */

#ifdef USE_PALETTE_MAP
	if (elemindex)
		update_palettemap();
#endif /* USE_PALETTE_MAP */

	for (i = 0; i < elemindex; i++)
	{
		render_element *elem = &elemlist[i];
		rectangle bounds;
		rgb_t color = elem->color;

		if (color == UI_SCROLL_TEXT_COLOR)
			ui_need_scroll = TRUE;

#ifdef UI_COLOR_DISPLAY
		if (color == ARGB_BLACK)
			color = get_black_pen();
		else if (color == ARGB_WHITE)
			color = get_white_pen();
		else if (color == UI_SCROLL_TEXT_COLOR)
		{
			if (ui_lock_scroll)
				color = uifont_colortable[FONT_COLOR_SPECIAL];
			else
				color = get_white_pen();
		}
		else if (color < MAX_COLORTABLE)
			color = uifont_colortable[color];
#else /* UI_COLOR_DISPLAY */
		if (color & 0xffffff)
			color = get_white_pen();
		else
			color = get_black_pen();
#endif /* UI_COLOR_DISPLAY */

		switch (elem->type)
		{
			case 0xffff:	/* box */
			case 0xfffe:	/* line */
				bounds.min_x = uirotbounds.min_x + elem->x;
				bounds.min_y = uirotbounds.min_y + elem->y;
				bounds.max_x = uirotbounds.min_x + elem->x2;
				bounds.max_y = uirotbounds.min_y + elem->y2;
				ui_rot2raw_rect(&bounds);
				sect_rect(&bounds, &uirawbounds);
#ifdef TRANS_UI
				if (elem->color == UI_TRANSPARENT_COLOR)
#ifdef UI_COLOR_DISPLAY
					fillbitmap_ts(dest, uifont_colortable[SYSTEM_COLOR_BACKGROUND], &bounds);
#else /* UI_COLOR_DISPLAY */
					fillbitmap_ts(dest, get_black_pen(), &bounds);
#endif /* UI_COLOR_DISPLAY */
				else
#endif /* TRANS_UI */
				fillbitmap(dest, color, &bounds);
				artwork_mark_ui_dirty(bounds.min_x, bounds.min_y, bounds.max_x, bounds.max_y);
				ui_dirty = 5;
				break;

			default:
				bounds.min_x = uirotbounds.min_x + elem->x;
				bounds.min_y = uirotbounds.min_y + elem->y + 1;
				bounds.max_x = bounds.min_x + ui_get_char_width(elem->type) - 1;
				bounds.max_y = bounds.min_y + uirotcharheight - 1;
				ui_rot2raw_rect(&bounds);
				uifont_drawchar(dest, elem->type, color, bounds.min_x, bounds.min_y, &uirawbounds);
				break;
		}
	}

	if (!ui_need_scroll)
		ui_lock_scroll = FALSE;

	elemindex = 0;
}

static void add_filled_box_color(int x1, int y1, int x2, int y2, rgb_t color);


#else
void ui_get_bounds(int *width, int *height)
{
	*width = UI_SCALE_TO_INT_X(1.0f);
	*height = UI_SCALE_TO_INT_Y(1.0f);
}


int ui_get_line_height(void)
{
	return UI_SCALE_TO_INT_Y(UI_FONT_HEIGHT);
}


int ui_get_char_width(UINT16 ch)
{
	return UI_SCALE_TO_INT_X(render_font_get_char_width(ui_font, UI_FONT_HEIGHT, render_get_ui_aspect(), ch));
}

#if 0
int ui_get_string_width(const char *s)
{
	return UI_SCALE_TO_INT_X(render_font_get_string_width(ui_font, UI_FONT_HEIGHT, render_get_ui_aspect(), s));
}
#endif

static void build_bgtexture(void)
{
	float r = (float)options.uicolortable[UI_TRANSPARENT_COLOR][0];
	float g = (float)options.uicolortable[UI_TRANSPARENT_COLOR][1];
	float b = (float)options.uicolortable[UI_TRANSPARENT_COLOR][2];
	UINT8 a = 0xff;
	int i;

	bgbitmap = bitmap_alloc_depth(1, 1024, 32);
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

		*(UINT32 *)bgbitmap->line[i] = MAKE_ARGB(a, (UINT8)(r * gradual), (UINT8)(g * gradual), (UINT8)(b * gradual));
	}

	bgtexture = render_texture_alloc(bgbitmap, NULL, NULL, TEXFORMAT_ARGB32, render_texture_hq_scale, NULL);
	add_exit_callback(free_bgtexture);
}

static void free_bgtexture(void)
{
	bitmap_free(bgbitmap);
	bgbitmap = NULL;
	render_texture_free(bgtexture);
	bgtexture = NULL;
}

INLINE rgb_t ui_get_rgb_color(rgb_t color)
{
	if (color < MAX_COLORTABLE)
		return uifont_colortable[color];

	// fixme
	if (color == UI_SCROLL_TEXT_COLOR)
		return ARGB_WHITE;

	return color;
}

static void add_fill(int x0, int y0, int x1, int y1, rgb_t color)
{
	x1++;
	y1++;

	if (color == UI_TRANSPARENT_COLOR)
		render_ui_add_quad(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), MAKE_ARGB(0xff, 0xff, 0xff, 0xff), bgtexture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	else
		render_ui_add_rect(UI_UNSCALE_TO_FLOAT_X(x0), UI_UNSCALE_TO_FLOAT_Y(y0), UI_UNSCALE_TO_FLOAT_X(x1), UI_UNSCALE_TO_FLOAT_Y(y1), ui_get_rgb_color(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

#endif


int ui_get_string_width(const char *s)
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


static void add_filled_box_color(int x1, int y1, int x2, int y2, rgb_t color)
{
#ifdef UI_COLOR_DISPLAY
	add_fill(x1 + 3, y1 + 3, x2 - 3, y2 - 3, color);

	/* top edge */
	add_line(x1,     y1,     x2,     y1,     SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y1 + 1, x2 - 1, y1 + 1, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1 + 2, y1 + 2, x2 - 2, y1 + 2, SYSTEM_COLOR_FRAMEDARK);

	/* bottom edge */
	add_line(x1 + 3, y2 - 2, x2 - 2, y2 - 2, SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y2 - 1, x2 - 1, y2 - 1, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1,     y2,     x2,     y2,     SYSTEM_COLOR_FRAMEDARK);

	/* left edge */
	add_line(x1,     y1 + 1, x1,     y2 - 1, SYSTEM_COLOR_FRAMELIGHT);
	add_line(x1 + 1, y1 + 2, x1 + 1, y2 - 2, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x1 + 2, y1 + 3, x1 + 2, y2 - 2, SYSTEM_COLOR_FRAMEDARK);

	/* right edge */
	add_line(x2 - 2, y1 + 3, x2 - 2, y2 - 3, SYSTEM_COLOR_FRAMELIGHT);
	add_line(x2 - 1, y1 + 2, x2 - 1, y2 - 2, SYSTEM_COLOR_FRAMEMEDIUM);
	add_line(x2,     y1 + 1, x2,     y2 - 1, SYSTEM_COLOR_FRAMEDARK);
#else /* UI_COLOR_DISPLAY */
	add_fill(x1 + 1, y1 + 1, x2 - 1, y2 - 1, color);

	add_line(x1, y1, x2, y1, ARGB_WHITE);
	add_line(x2, y1, x2, y2, ARGB_WHITE);
	add_line(x2, y2, x1, y2, ARGB_WHITE);
	add_line(x1, y2, x1, y1, ARGB_WHITE);
#endif /* UI_COLOR_DISPLAY */
}


static void add_filled_box(int x0, int y0, int x1, int y1)
{
	add_filled_box_color(x0, y0, x1, y1, ui_bgcolor);
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
