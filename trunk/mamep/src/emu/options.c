/***************************************************************************

    options.c

    Options file and command line management.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "mamecore.h"
#include "mame.h"
#include "fileio.h"
#include "options.h"
#include "osd_so.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "strconv.h"

#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_ENTRY_NAMES		4



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _options_data options_data;
struct _options_data
{
	options_data *			next;				/* link to the next data */
	const char *			names[MAX_ENTRY_NAMES]; /* array of possible names */
	UINT32					flags;				/* flags from the entry */
	UINT32					seqid;				/* sequence ID; bumped on each change */
	int						error_reported;		/* have we reported an error on this option yet? */
	const char *			data;				/* data for this item */
	const char *			defdata;			/* default data for this item */
	const char *			description;		/* description for this item */
	void					(*callback)(const char *arg);	/* callback to be invoked when parsing */
	int						mark;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const char *option_unadorned[MAX_UNADORNED_OPTIONS] =
{
	"<UNADORNED0>",
	"<UNADORNED1>",
	"<UNADORNED2>",
	"<UNADORNED3>",
	"<UNADORNED4>",
	"<UNADORNED5>",
	"<UNADORNED6>",
	"<UNADORNED7>",
	"<UNADORNED8>",
	"<UNADORNED9>",
	"<UNADORNED10>",
	"<UNADORNED11>",
	"<UNADORNED12>",
	"<UNADORNED13>",
	"<UNADORNED14>",
	"<UNADORNED15>"
};



/***************************************************************************
    BUILT-IN (CORE) OPTIONS
***************************************************************************/

static const options_entry core_options[] =
{
	// unadorned options - only a single one supported at the moment
	{ "<UNADORNED0>",                NULL,        0,                 NULL },

#ifdef DRIVER_SWITCH
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE CONFIGURATION OPTIONS" },
	{ "driver_config",               "mame,plus", 0,                 "switch drivers"},
#endif /* DRIVER_SWITCH */

	// file and directory options
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SEARCH PATH OPTIONS" },
	{ "rompath;rp;biospath;bp",      "roms",      0,                 "path to ROMsets and hard disk images" },
#ifdef MESS
	{ "hashpath;hash_directory;hash","hash",      0,                 "path to hash files" },
#endif /* MESS */
	{ "samplepath;sp",               "samples",   0,                 "path to samplesets" },
	{ "artpath;artwork_directory",   "artwork",   0,                 "path to artwork files" },
	{ "ctrlrpath;ctrlr_directory",   "ctrlr",     0,                 "path to controller definitions" },
	{ "inipath",                     "ini",       0,                 "path to ini files" },
	{ "fontpath",                    "lang",      0,                 "path to font files" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE OUTPUT DIRECTORY OPTIONS" },
	{ "cfg_directory",               "cfg",       0,                 "directory to save configurations" },
	{ "nvram_directory",             "nvram",     0,                 "directory to save nvram contents" },
	{ "memcard_directory",           "memcard",   0,                 "directory to save memory card contents" },
	{ "input_directory",             "inp",       0,                 "directory to save input device logs" },
#ifdef USE_HISCORE
	{ "hiscore_directory",           "hi",        0,                 "directory to save hiscores" },
#endif /* USE_HISCORE */
	{ "state_directory",             "sta",       0,                 "directory to save states" },
	{ "snapshot_directory",          "snap",      0,                 "directory to save screenshots" },
	{ "diff_directory",              "diff",      0,                 "directory to save hard drive image difference files" },
	{ "translation_directory",       "lang",      0,                 "directory for translation table data" },
	{ "comment_directory",           "comments",  0,                 "directory to save debugger comments" },
#ifdef USE_IPS
	{ "ips_directory",               "ips",       0,                 "directory for ips files" },
#else /* USE_IPS */
	{ "ips_directory",               "ips",       OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* USE_IPS */
	{ "localized_directory",         "lang",      0,                 "directory for localized data files" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE FILENAME OPTIONS" },
	{ "cheat_file",                  "cheat.dat", 0,                 "cheat filename" },
	{ "history_file",               "history.dat",0,                 "history database name" },
#ifdef STORY_DATAFILE
	{ "story_file",                 "story.dat",  0,                 "story database name" },
#else /* STORY_DATAFILE */
	{ "story_file",                 "story.dat", OPTION_DEPRECATED,  "(disabled by compiling option)" },
#endif /* STORY_DATAFILE */
	{ "mameinfo_file",              "mameinfo.dat",0,                "mameinfo database name" },
#ifdef CMD_LIST
	{ "command_file",               "command.dat",0,                 "command list database name" },
#else /* CMD_LIST */
	{ "command_file",               "command.dat",OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* CMD_LIST */
#ifdef USE_HISCORE
	{ "hiscore_file",               "hiscore.dat",0,                 "high score database name" },
#else /* STORY_DATAFILE */
	{ "hiscore_file",               "hiscore.dat",OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* USE_HISCORE */

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE STATE/PLAYBACK OPTIONS" },
	{ "state",                       NULL,        0,                 "saved state to load" },
	{ "autosave",                    "0",         OPTION_BOOLEAN,    "enable automatic restore at startup, and automatic save at exit time" },
	{ "playback;pb",                 NULL,        0,                 "playback an input file" },
	{ "record;rec",                  NULL,        0,                 "record an input file" },
	{ "mngwrite",                    NULL,        0,                 "optional filename to write a MNG movie of the current session" },
	{ "wavwrite",                    NULL,        0,                 "optional filename to write a WAV file of the current session" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE PERFORMANCE OPTIONS" },
	{ "autoframeskip;afs",           "0",         OPTION_BOOLEAN,    "enable automatic frameskip selection" },
	{ "frameskip;fs",                "0",         0,                 "set frameskip to fixed value, 0-12 (autoframeskip must be disabled)" },
	{ "seconds_to_run;str",          "0",         0,                 "number of emulated seconds to run before automatically exiting" },
	{ "throttle",                    "1",         OPTION_BOOLEAN,    "enable throttling to keep game running in sync with real time" },
	{ "sleep",                       "1",         OPTION_BOOLEAN,    "enable sleeping, which gives time back to other applications when idle" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE ROTATION OPTIONS" },
	{ "rotate",                      "1",         OPTION_BOOLEAN,    "rotate the game screen according to the game's orientation needs it" },
	{ "ror",                         "0",         OPTION_BOOLEAN,    "rotate screen clockwise 90 degrees" },
	{ "rol",                         "0",         OPTION_BOOLEAN,    "rotate screen counterclockwise 90 degrees" },
	{ "autoror",                     "0",         OPTION_BOOLEAN,    "automatically rotate screen clockwise 90 degrees if vertical" },
	{ "autorol",                     "0",         OPTION_BOOLEAN,    "automatically rotate screen counterclockwise 90 degrees if vertical" },
	{ "flipx",                       "0",         OPTION_BOOLEAN,    "flip screen left-right" },
	{ "flipy",                       "0",         OPTION_BOOLEAN,    "flip screen upside-down" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE ARTWORK OPTIONS" },
	{ "artwork_crop;artcrop",        "0",         OPTION_BOOLEAN,    "crop artwork to game screen size" },
	{ "use_backdrops;backdrop",      "1",         OPTION_BOOLEAN,    "enable backdrops if artwork is enabled and available" },
	{ "use_overlays;overlay",        "1",         OPTION_BOOLEAN,    "enable overlays if artwork is enabled and available" },
	{ "use_bezels;bezel",            "1",         OPTION_BOOLEAN,    "enable bezels if artwork is enabled and available" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SCREEN OPTIONS" },
	{ "brightness",                  "1.0",       0,                 "default game screen brightness correction" },
	{ "contrast",                    "1.0",       0,                 "default game screen contrast correction" },
	{ "gamma",                       "1.0",       0,                 "default game screen gamma correction" },
	{ "pause_brightness",            "0.65",      0,                 "amount to scale the screen brightness when paused" },
#ifdef USE_SCALE_EFFECTS
	{ "scale_effect",                "none",      0,                 "image enhancement effect" },
#endif /* USE_SCALE_EFFECTS */

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE VECTOR OPTIONS" },
	{ "antialias;aa",                "1",         OPTION_BOOLEAN,    "use antialiasing when drawing vectors" },
	{ "beam",                        "1.0",       0,                 "set vector beam width" },
	{ "flicker",                     "0",         0,                 "set vector flicker effect" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE SOUND OPTIONS" },
	{ "sound",                       "1",         OPTION_BOOLEAN,    "enable sound output" },
	{ "samplerate;sr",               "48000",     0,                 "set sound output sample rate" },
	{ "samples",                     "1",         OPTION_BOOLEAN,    "enable the use of external samples if available" },
	{ "volume;vol",                  "0",         0,                 "sound volume in decibels (-32 min, 0 max)" },
#ifdef USE_VOLUME_AUTO_ADJUST
	{ "volume_adjust",               "0",         OPTION_BOOLEAN,    "enable/disable volume auto adjust" },
#else /* USE_VOLUME_AUTO_ADJUST */
	{ "volume_adjust",               "0",         OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* USE_VOLUME_AUTO_ADJUST */

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE INPUT OPTIONS" },
	{ "ctrlr",                       "Standard",  0,                 "preconfigure for specified controller" },

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE DEBUGGING OPTIONS" },
	{ "log",                         "0",         OPTION_BOOLEAN,    "generate an error.log file" },
#ifdef MAME_DEBUG
	{ "debug;d",                     "1",         OPTION_BOOLEAN,    "enable/disable debugger" },
	{ "debugscript",                 NULL,        0,                 "script for debugger" },
#else
	{ "debug;d",                     "1",         OPTION_DEPRECATED, "(debugger-only command)" },
	{ "debugscript",                 NULL,        OPTION_DEPRECATED, "(debugger-only command)" },
#endif

	{ NULL,                          NULL,        OPTION_HEADER,     "CORE MISC OPTIONS" },
	{ "bios",                        "default",   0,                 "select the system BIOS to use" },
	{ "cheat;c",                     "0",         OPTION_BOOLEAN,    "enable cheat subsystem" },
	{ "skip_gameinfo",               "0",         OPTION_BOOLEAN,    "skip displaying the information screen at startup" },
#ifdef USE_IPS
	{ "ips",                         NULL,        0,                 "ips datfile name"},
#else /* USE_IPS */
	{ "ips",                         NULL,        OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* USE_IPS */
	{ "confirm_quit",                "1",         OPTION_BOOLEAN,    "quit game with confirmation" },
#ifdef AUTO_PAUSE_PLAYBACK
	{ "auto_pause_playback",         "0",         OPTION_BOOLEAN,    "automatic pause when playback is finished" },
#else /* AUTO_PAUSE_PLAYBACK */
	{ "auto_pause_playback",         "0",         OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* AUTO_PAUSE_PLAYBACK */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	/* ks hcmame s switch m68k core */
	{ "m68k_core",                   "c",         0,                 "change m68k core (0:C, 1:DRC, 2:ASM+DRC)" },
#else /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
	{ "m68k_core",                   "c",         OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef TRANS_UI
	{ "use_trans_ui",                "1",         OPTION_BOOLEAN,    "use transparent background for UI text" },
	{ "ui_transparency",             "224",       0,                 "transparency of UI background [0 - 255]" },
#else /* TRANS_UI */
	{ "use_trans_ui",                "1",         OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "ui_transparency",             "224",       OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* TRANS_UI */

	// palette options
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE PALETTE OPTIONS" },
#ifdef UI_COLOR_DISPLAY
	{ "font_blank",                  "0,0,0",       0,               "font blank color" },
	{ "font_normal",                 "255,255,255", 0,               "font normal color" },
	{ "font_special",                "247,203,0",   0,               "font special color" },
	{ "system_background",           "16,16,48",    0,               "window background color" },
	{ "button_red",                  "255,64,64",   0,               "button color (red)" },
	{ "button_yellow",               "255,238,0",   0,               "button color (yellow)" },
	{ "button_green",                "0,255,64",    0,               "button color (green)" },
	{ "button_blue",                 "0,170,255",   0,               "button color (blue)" },
	{ "button_purple",               "170,0,255",   0,               "button color (purple)" },
	{ "button_pink",                 "255,0,170",   0,               "button color (pink)" },
	{ "button_aqua",                 "0,255,204",   0,               "button color (aqua)" },
	{ "button_silver",               "255,0,255",   0,               "button color (silver)" },
	{ "button_navy",                 "255,160,0",   0,               "button color (navy)" },
	{ "button_lime",                 "190,190,190", 0,               "button color (lime)" },
	{ "cursor",                      "60,120,240",  0,               "cursor color" },
#else /* UI_COLOR_DISPLAY */
	{ "font_blank",                  "0,0,0",       OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "font_normal",                 "255,255,255", OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "font_special",                "247,203,0",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "system_background",           "16,16,48",    OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_red",                  "255,64,64",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_yellow",               "255,238,0",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_green",                "0,255,64",    OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_blue",                 "0,170,255",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_purple",               "170,0,255",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_pink",                 "255,0,170",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_aqua",                 "0,255,204",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_silver",               "255,0,255",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_navy",                 "255,160,0",   OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "button_lime",                 "190,190,190", OPTION_DEPRECATED, "(disabled by compiling option)" },
	{ "cursor",                      "60,120,240",  OPTION_DEPRECATED, "(disabled by compiling option)" },
#endif /* UI_COLOR_DISPLAY */

	// language options
	{ NULL,                          NULL,        OPTION_HEADER,     "CORE LANGUAGE OPTIONS" },
	{ "language;lang",               "auto",      0,                 "select translation language" },
	{ "use_lang_list",               "1",         OPTION_BOOLEAN,    "enable/disable local language game list" },

	{ NULL }
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static options_data *		datalist;
static options_data **		datalist_nextptr = NULL;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static options_data *find_entry_data(const char *string, int is_command_line);
static void update_data(options_data *data, const char *newdata);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    generate_find_cache
-------------------------------------------------*/

struct find_cache_entry
{
	const char *name;
	options_data *data;
};

static struct
{
	options_data *current_entry;
	struct find_cache_entry *cache;
	int count;
} find_cache;

static int compare_entry(const void *compare1, const void *compare2)
{
	const struct find_cache_entry *p1 = (const struct find_cache_entry *)compare1;
	const struct find_cache_entry *p2 = (const struct find_cache_entry *)compare2;

	return strcmp(p1->name, p2->name);
}

static void generate_find_cache(void)
{
	struct find_cache_entry *p;
	options_data *data;
	int len;

	find_cache.current_entry = datalist;

	if (find_cache.cache != NULL)
		free(find_cache.cache);

	len = 256;

	find_cache.cache = malloc(len * sizeof (*find_cache.cache));
	p = find_cache.cache;
	find_cache.count = 0;

	/* scan all entries */
	for (data = datalist; data != NULL; data = data->next)
		if (!(data->flags & OPTION_HEADER))
		{
			int namenum;

			/* loop over names */
			for (namenum = 0; namenum < ARRAY_LENGTH(data->names); namenum++)
				if (data->names[namenum] != NULL)
				{
					if (find_cache.count >= len)
					{
						len *= 2;
						find_cache.cache = realloc(find_cache.cache, len * sizeof (*find_cache.cache));
						p = find_cache.cache + find_cache.count;
					}

					p->data = data;
					p->name = data->names[namenum];
					p++;
					find_cache.count++;
				}
		}

	qsort(find_cache.cache, find_cache.count, sizeof (*find_cache.cache), compare_entry);
}

/*-------------------------------------------------
    copy_string - allocate a copy of a string
-------------------------------------------------*/

static const char *copy_string(const char *start, const char *end)
{
	char *result;

	/* copy of a NULL is NULL */
	if (start == NULL)
		return NULL;

	/* if no end, figure it out */
	if (end == NULL)
		end = start + strlen(start);

	/* allocate, copy, and NULL-terminate */
	result = malloc_or_die((end - start) + 1);
	memcpy(result, start, end - start);
	result[end - start] = 0;

	return result;
}


/*-------------------------------------------------
    separate_names - separate a list of semicolon
    separated names into individual strings
-------------------------------------------------*/

static int separate_names(const char *srcstring, const char *results[], int maxentries)
{
	const char *start;
	int curentry;

	/* start with the original string and loop over entries */
	start = srcstring;
	for (curentry = 0; curentry < maxentries; curentry++)
	{
		/* find the end of this entry and copy the string */
		const char *end = strchr(start, ';');
		results[curentry] = copy_string(start, end);

		/* if we hit the end of the source, stop */
		if (end == NULL)
			break;
		start = end + 1;
	}
	return curentry;
}


/*-------------------------------------------------
    options_init - initialize the options system
-------------------------------------------------*/

void options_init(const options_entry *entrylist)
{
	if (find_cache.current_entry == datalist)
	{
		if (find_cache.cache != NULL)
			free(find_cache.cache);

		find_cache.cache = NULL;
		find_cache.current_entry = NULL;
	}

	/* free anything we currently have */
	options_free_entries();
	datalist_nextptr = &datalist;

	/* add core options and optionally add options that are passed to us */
	options_add_entries(core_options);
	if (entrylist != NULL)
		options_add_entries(entrylist);
}


/*-------------------------------------------------
    options_add_entries - add entries to the
    current options sets
-------------------------------------------------*/

void options_add_entries(const options_entry *entrylist)
{
	options_data *data;

	assert_always(datalist_nextptr != NULL, "Missing call to options_init()!");

	if (find_cache.current_entry == datalist)
	{
		if (find_cache.cache != NULL)
			free(find_cache.cache);

		find_cache.cache = NULL;
		find_cache.current_entry = NULL;
	}

	/* loop over entries until we hit a NULL name */
	for ( ; entrylist->name != NULL || (entrylist->flags & OPTION_HEADER); entrylist++)
	{
		/* allocate a new item */
		data = malloc_or_die(sizeof(*data));
		memset(data, 0, sizeof(*data));

		/* separate the names, copy the flags, and set the value equal to the default */
		if (entrylist->name != NULL)
			separate_names(entrylist->name, data->names, ARRAY_LENGTH(data->names));
		data->flags = entrylist->flags;
		data->data = copy_string(entrylist->defvalue, NULL);
		data->defdata = copy_string(entrylist->defvalue, NULL);
		data->description = entrylist->description;

		/* fill it in and add to the end of the list */
		*datalist_nextptr = data;
		datalist_nextptr = &data->next;
	}
}


/*-------------------------------------------------
    options_set_option_default_value - change the
    default value of an option
-------------------------------------------------*/

void options_set_option_default_value(const char *name, const char *defvalue)
{
	options_data *data = find_entry_data(name, TRUE);
	assert(data != NULL);

	/* free the existing value and make a copy for the new one */
	/* note that we assume this is called before any data is processed */
	if (data->data != NULL)
		free((void *)data->data);
	if (data->defdata != NULL)
		free((void *)data->defdata);
	data->data = copy_string(defvalue, NULL);
	data->defdata = copy_string(defvalue, NULL);
}


/*-------------------------------------------------
    options_set_option_callback - specifies a
    callback to be invoked when parsing options
-------------------------------------------------*/

void options_set_option_callback(const char *name, void (*callback)(const char *arg))
{
	options_data *data = find_entry_data(name, TRUE);
	assert(data != NULL);
	data->callback = callback;
}


/*-------------------------------------------------
    options_free_entries - free all the entries
    that were added
-------------------------------------------------*/

void options_free_entries(void)
{
	if (find_cache.current_entry == datalist)
	{
		if (find_cache.cache != NULL)
			free(find_cache.cache);

		find_cache.cache = NULL;
		find_cache.current_entry = NULL;
	}

	/* free all of the registered data */
	while (datalist != NULL)
	{
		options_data *temp = datalist;
		int i;

		datalist = temp->next;

		/* free names and data, and finally the element itself */
		for (i = 0; i < ARRAY_LENGTH(temp->names); i++)
			if (temp->names[i] != NULL)
				free((void *)temp->names[i]);
		if (temp->defdata != NULL)
			free((void *)temp->defdata);
		if (temp->data != NULL)
			free((void *)temp->data);
		free(temp);
	}

	/* reset the nextptr */
	datalist_nextptr = &datalist;
}


/*-------------------------------------------------
    options_parse_command_line - parse a series
    of command line arguments
-------------------------------------------------*/

int options_parse_command_line(int argc, char **argv)
{
	int unadorned_index = 0;
	int arg;

	/* loop over commands, looking for options */
	for (arg = 1; arg < argc; arg++)
	{
		options_data *data;
		const char *optionname;
		const char *newdata;

		/* determine the entry name to search for */
		if (argv[arg][0] == '-')
			optionname = &argv[arg][1];
		else
		{
			optionname = OPTION_UNADORNED(unadorned_index);
			unadorned_index++;
		}

		/* find our entry */
		data = find_entry_data(optionname, TRUE);
		if (data == NULL)
		{
			mame_printf_error(_("Error: unknown option: %s\n"), argv[arg]);
			return 1;
		}
		if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL)) != 0)
			continue;

		/* get the data for this argument, special casing booleans */
		if ((data->flags & (OPTION_BOOLEAN | OPTION_COMMAND)) != 0)
			newdata = (strncmp(&argv[arg][1], "no", 2) == 0) ? "0" : "1";
		else if (argv[arg][0] != '-')
			newdata = argv[arg];
		else if (arg + 1 < argc)
			newdata = argv[++arg];
		else
		{
			mame_printf_error(_("Error: option %s expected a parameter\n"), argv[arg]);
			return 1;
		}

		/* invoke callback, if present */
		if (data->callback != NULL)
			(*data->callback)(newdata);

		/* allocate a new copy of data for this */
		update_data(data, newdata);
	}
	return 0;
}


/*-------------------------------------------------
    options_parse_ini_file - parse a series
    of entries in an INI file
-------------------------------------------------*/

int options_parse_ini_file(mame_file *inifile)
{
	/* loop over data */
	while (mame_fgets(giant_string_buffer, GIANT_STRING_BUFFER_SIZE, inifile) != NULL)
	{
		char *optionname, *optiondata, *temp;
		options_data *data;
		int inquotes = FALSE;

		/* find the name */
		for (optionname = giant_string_buffer; *optionname != 0; optionname++)
			if (!isspace(*optionname))
				break;

		/* skip comments */
		if (*optionname == 0 || *optionname == '#')
			continue;

		/* scan forward to find the first space */
		for (temp = optionname; *temp != 0; temp++)
			if (isspace(*temp))
				break;

		/* if we hit the end early, print a warning and continue */
		if (*temp == 0)
		{
			mame_printf_warning(_("Warning: invalid line in INI: %s"), giant_string_buffer);
			continue;
		}

		/* NULL-terminate */
		*temp++ = 0;
		optiondata = temp;

		/* scan the data, stopping when we hit a comment */
		for (temp = optiondata; *temp != 0; temp++)
		{
			if (*temp == '"')
				inquotes = !inquotes;
			if (*temp == '#' && !inquotes)
				break;
		}
		*temp = 0;

		/* find our entry */
		data = find_entry_data(optionname, FALSE);
		if (data == NULL)
		{
			mame_printf_warning(_("Warning: unknown option in INI: %s\n"), optionname);
			continue;
		}
		if ((data->flags & OPTION_DEPRECATED) != 0)
			continue;

		/* allocate a new copy of data for this */
		update_data(data, optiondata);
	}
	return 0;
}


/*-------------------------------------------------
    options_output_command_line_marked - output the
    current marked state as command line string
-------------------------------------------------*/

int options_output_command_line_marked(char *buf)
{
	options_data *data;
	int total = 1;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* output entries for all non-deprecated and non-command items */
		if (data->mark && (data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0 && data->names[0][0] != 0)
		{
			int len = 2 + strlen(data->names[0]);

			if (data->flags & OPTION_BOOLEAN)
			{
				int value = FALSE;

				sscanf(data->data, "%d", &value);

				if (value)
				{
					if (buf)
						sprintf(buf, "-%s ", data->names[0]);
				}
				else
				{
					if (buf)
						sprintf(buf, "-no%s ", data->names[0]);
					len += 2;
				}
			}
			else
			{
				if (data->data != NULL)
				{
					if (strchr(data->data, ' ') != NULL || strchr(data->data, '#') != NULL)
					{
						if (buf)
							sprintf(buf, "-%s \"%s\" ", data->names[0], data->data);
						len += 3 + strlen(data->data);
					}
					else
					{
						if (buf)
							sprintf(buf, "-%s %s ", data->names[0], data->data);
						len += 1 + strlen(data->data);
					}
				}
			}

			if (buf)
				buf += len;
			total += len;
		}
	}

	if (buf)
	{
		if (total > 1)
			buf--;
		*buf = '\0';
	}

	return total;
}

/*-------------------------------------------------
    translate_description - translate description
    by UI_MSG_MAME or UI_MSG_OSD0
-------------------------------------------------*/

static const char *translate_description(const options_data *data)
{
	const char *desc = _(data->description);

	if (desc == data->description)
		return lang_message(UI_MSG_OSD0, desc);

	return desc;
}

/*-------------------------------------------------
    options_output_ini_file - output the current
    state to an INI file
-------------------------------------------------*/

void options_output_ini_file(FILE *inifile)
{
	options_data *data;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			fprintf(inifile, "\n#\n# %s\n#\n", translate_description(data));

		/* skip UNADORNED options */
		else if (data->description == NULL)
			;

		/* otherwise, output entries for all non-deprecated and non-command items */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0 && data->names[0][0] != 0)
		{
			if (data->data == NULL)
				fprintf(inifile, "# %-23s <NULL> (not set)\n", data->names[0]);
			else if (strchr(data->data, ' ') != NULL || strchr(data->data, '#') != NULL)
				fprintf(inifile, "%-25s \"%s\"\n", data->names[0], data->data);
			else
				fprintf(inifile, "%-25s %s\n", data->names[0], data->data);
		}
	}
}


/*-------------------------------------------------
    options_output_ini_file_marked - output the
    current marked state to an INI file
-------------------------------------------------*/

void options_output_ini_file_marked(FILE *inifile)
{
	options_data *data;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			fprintf(inifile, "\n#\n# %s\n#\n", translate_description(data));

		/* skip UNADORNED options */
		else if (data->description == NULL)
			;

		/* otherwise, output entries for all non-deprecated and non-command items */
		else if (data->mark && (data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0 && data->names[0][0] != 0)
		{
			if (data->data == NULL)
				fprintf(inifile, "# %-23s <NULL> (not set)\n", data->names[0]);
			else if (strchr(data->data, ' ') != NULL || strchr(data->data, '#') != NULL)
				fprintf(inifile, "%-25s \"%s\"\n", data->names[0], data->data);
			else
				fprintf(inifile, "%-25s %s\n", data->names[0], data->data);
		}
	}
}


/*-------------------------------------------------
    options_output_ini_mame_file - output the current
    state to an INI file
-------------------------------------------------*/

void options_output_ini_mame_file(mame_file *inifile)
{
	options_data *data;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			mame_fprintf(inifile, "\n#\n# %s\n#\n", translate_description(data));

		/* skip UNADORNED options */
		else if (data->description == NULL)
			;

		/* otherwise, output entries for all non-deprecated and non-command items */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0 && data->names[0][0] != 0)
		{
			if (data->data == NULL)
				mame_fprintf(inifile, "# %-23s <NULL> (not set)\n", data->names[0]);
			else if (strchr(data->data, ' ') != NULL || strchr(data->data, '#') != NULL)
				mame_fprintf(inifile, "%-25s \"%s\"\n", data->names[0], data->data);
			else
				mame_fprintf(inifile, "%-25s %s\n", data->names[0], data->data);
		}
	}
}


/*-------------------------------------------------
    options_output_ini_mame_file_marked - output the
    current marked state to an INI file
-------------------------------------------------*/

void options_output_ini_mame_file_marked(mame_file *inifile)
{
	options_data *data;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			mame_fprintf(inifile, "\n#\n# %s\n#\n", translate_description(data));

		/* skip UNADORNED options */
		else if (data->description == NULL)
			;

		/* otherwise, output entries for all non-deprecated and non-command items */
		else if (data->mark && (data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL | OPTION_COMMAND)) == 0 && data->names[0][0] != 0)
		{
			if (data->data == NULL)
				mame_fprintf(inifile, "# %-23s <NULL> (not set)\n", data->names[0]);
			else if (strchr(data->data, ' ') != NULL || strchr(data->data, '#') != NULL)
				mame_fprintf(inifile, "%-25s \"%s\"\n", data->names[0], data->data);
			else
				mame_fprintf(inifile, "%-25s %s\n", data->names[0], data->data);
		}
	}
}


/*-------------------------------------------------
    options_output_help - output option help to
    a file
-------------------------------------------------*/

void options_output_help(void)
{
	options_data *data;

	/* loop over all items */
	for (data = datalist; data != NULL; data = data->next)
	{
		/* header: just print */
		if ((data->flags & OPTION_HEADER) != 0)
			mame_printf_info("\n#\n# %s\n#\n", translate_description(data));
		/* otherwise, output entries for all non-deprecated items */
		else if ((data->flags & (OPTION_DEPRECATED | OPTION_INTERNAL)) == 0 && data->description != NULL)
			mame_printf_info("-%-20s%s\n", data->names[0], translate_description(data));
	}
}


/*-------------------------------------------------
    options_get_string - return data formatted
    as a string
-------------------------------------------------*/

const char *options_get_string(const char *name)
{
	options_data *data = find_entry_data(name, FALSE);
	assert(data != NULL);
	return (data == NULL) ? "" : data->data;
}


/*-------------------------------------------------
    options_get_seqid - return the seqid for an
    entry
-------------------------------------------------*/

UINT32 options_get_seqid(const char *name)
{
	options_data *data = find_entry_data(name, FALSE);
	return (data == NULL) ? 0 : data->seqid;
}


/*-------------------------------------------------
    options_get_bool - return data formatted as
    a boolean
-------------------------------------------------*/

int options_get_bool(const char *name)
{
	options_data *data = find_entry_data(name, FALSE);
	int value = FALSE;

	if (data == NULL)
		mame_printf_error(_("Unexpected boolean option %s queried\n"), name);
	else if (data->data == NULL || sscanf(data->data, "%d", &value) != 1 || value < 0 || value > 1)
	{
		if (data->defdata != NULL)
		{
			options_set_string(name, data->defdata);
			sscanf(data->data, "%d", &value);
		}
		if (!data->error_reported)
		{
			mame_printf_error(_("Illegal boolean value for %s; reverting to %d\n"), data->names[0], value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_int - return data formatted as
    an integer
-------------------------------------------------*/

int options_get_int(const char *name)
{
	options_data *data = find_entry_data(name, FALSE);
	int value = 0;

	if (data == NULL)
		mame_printf_error(_("Unexpected integer option %s queried\n"), name);
	else if (data->data == NULL || sscanf(data->data, "%d", &value) != 1)
	{
		if (data->defdata != NULL)
		{
			options_set_string(name, data->defdata);
			sscanf(data->data, "%d", &value);
		}
		if (!data->error_reported)
		{
			mame_printf_error(_("Illegal integer value for %s; reverting to %d\n"), data->names[0], value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_float - return data formatted as
    a float
-------------------------------------------------*/

float options_get_float(const char *name)
{
	options_data *data = find_entry_data(name, FALSE);
	float value = 0;

	if (data == NULL)
		mame_printf_error(_("Unexpected float option %s queried\n"), name);
	else if (data->data == NULL || sscanf(data->data, "%f", &value) != 1)
	{
		if (data->defdata != NULL)
		{
			options_set_string(name, data->defdata);
			sscanf(data->data, "%f", &value);
		}
		if (!data->error_reported)
		{
			mame_printf_error(_("Illegal float value for %s; reverting to %f\n"), data->names[0], (double)value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_get_int_range - return data formatted
    as an integer and clamped to within the given
    range
-------------------------------------------------*/

int options_get_int_range(const char *name, int minval, int maxval)
{
	options_data *data = find_entry_data(name, FALSE);
	int value = 0;

	if (data == NULL)
		mame_printf_error(_("Unexpected integer option %s queried\n"), name);
	else if (data->data == NULL || sscanf(data->data, "%d", &value) != 1)
	{
		if (data->defdata != NULL)
		{
			options_set_string(name, data->defdata);
			value = options_get_int(name);
		}
		if (!data->error_reported)
		{
			mame_printf_error(_("Illegal integer value for %s; reverting to %d\n"), data->names[0], value);
			data->error_reported = TRUE;
		}
	}
	else if (value < minval || value > maxval)
	{
		options_set_string(name, data->defdata);
		value = options_get_int(name);
		if (!data->error_reported)
		{
			mame_printf_error(_("Invalid %s value (must be between %d and %d); reverting to %d\n"), data->names[0], minval, maxval, value);
			data->error_reported = TRUE;
		}
	}

	return value;
}


/*-------------------------------------------------
    options_get_float_range - return data formatted
    as a float and clamped to within the given
    range
-------------------------------------------------*/

float options_get_float_range(const char *name, float minval, float maxval)
{
	options_data *data = find_entry_data(name, FALSE);
	float value = 0;

	if (data == NULL)
		mame_printf_error(_("Unexpected float option %s queried\n"), name);
	else if (data->data == NULL || sscanf(data->data, "%f", &value) != 1)
	{
		if (data->defdata != NULL)
		{
			options_set_string(name, data->defdata);
			value = options_get_float(name);
		}
		if (!data->error_reported)
		{
			mame_printf_error(_("Illegal float value for %s; reverting to %f\n"), data->names[0], (double)value);
			data->error_reported = TRUE;
		}
	}
	else if (value < minval || value > maxval)
	{
		options_set_string(name, data->defdata);
		value = options_get_float(name);
		if (!data->error_reported)
		{
			mame_printf_error(_("Invalid %s value (must be between %f and %f); reverting to %f\n"), data->names[0], (double)minval, (double)maxval, (double)value);
			data->error_reported = TRUE;
		}
	}
	return value;
}


/*-------------------------------------------------
    options_set_string - set a string value
-------------------------------------------------*/

void options_set_string(const char *name, const char *value)
{
	options_data *data = find_entry_data(name, FALSE);
	assert(data != NULL);

	if (value == NULL)
		data->data = NULL;
	else
		update_data(data, value);
}


/*-------------------------------------------------
    options_set_bool - set a boolean value
-------------------------------------------------*/

void options_set_bool(const char *name, int value)
{
	char temp[4];
	assert(value == TRUE || value == FALSE);
	sprintf(temp, "%d", value);
	options_set_string(name, temp);
}


/*-------------------------------------------------
    options_set_int - set an integer value
-------------------------------------------------*/

void options_set_int(const char *name, int value)
{
	char temp[20];
	sprintf(temp, "%d", value);
	options_set_string(name, temp);
}


/*-------------------------------------------------
    options_set_float - set a float value
-------------------------------------------------*/

void options_set_float(const char *name, float value)
{
	char temp[100];
	sprintf(temp, "%f", value);
	options_set_string(name, temp);
}


/*-------------------------------------------------
    find_entry_data - locate an entry whose name
    matches the given string
-------------------------------------------------*/

#if 1
static options_data *find_entry_data(const char *string, int is_command_line)
{
	struct find_cache_entry temp, *result;
	int has_no_prefix;

	assert_always(datalist_nextptr != NULL, "Missing call to options_init()!");

	/* determine up front if we should look for "no" boolean options */
	has_no_prefix = (is_command_line && string[0] == 'n' && string[1] == 'o');

	if (find_cache.current_entry != datalist)
		generate_find_cache();

	temp.name = has_no_prefix ? &string[2] : &string[0];

	result = bsearch(&temp, find_cache.cache, find_cache.count, sizeof (*find_cache.cache), compare_entry);
	if (result)
		return result->data;

	return NULL;
}

#else
static options_data *find_entry_data(const char *string, int is_command_line)
{
	options_data *data;
	int has_no_prefix;

	assert_always(datalist_nextptr != NULL, "Missing call to options_init()!");

	/* determine up front if we should look for "no" boolean options */
	has_no_prefix = (is_command_line && string[0] == 'n' && string[1] == 'o');

	/* scan all entries */
	for (data = datalist; data != NULL; data = data->next)
		if (!(data->flags & OPTION_HEADER))
		{
			const char *compareme = (has_no_prefix && (data->flags & OPTION_BOOLEAN)) ? &string[2] : &string[0];
			int namenum;

			/* loop over names */
			for (namenum = 0; namenum < ARRAY_LENGTH(data->names); namenum++)
				if (data->names[namenum] != NULL && strcmp(compareme, data->names[namenum]) == 0)
					return data;
		}

	/* didn't find it at all */
	return NULL;
}
#endif


/*-------------------------------------------------
    update_data - update the data value for a
    given entry
-------------------------------------------------*/

static void update_data(options_data *data, const char *newdata)
{
	const unsigned char *dataend = newdata + strlen(newdata) - 1;
	const unsigned char *datastart = newdata;

	/* strip off leading/trailing spaces */
	while (isspace(*datastart) && datastart <= dataend)
		datastart++;
	while (isspace(*dataend) && datastart <= dataend)
		dataend--;

	/* strip off quotes */
	if (datastart != dataend && *datastart == '"' && *dataend == '"')
		datastart++, dataend--;

	/* allocate a copy of the data */
	if (data->data)
		free((void *)data->data);
	data->data = copy_string(datastart, dataend + 1);
 
	/* bump the seqid and clear the error reporting */
 	data->seqid++;
	data->error_reported = FALSE;
	data->mark = TRUE;
}


void options_clear_output_mark(void)
{
	options_data *data;

	for (data = datalist; data != NULL; data = data->next)
		data->mark = FALSE;
}


void *options_get_datalist(void)
{
	return datalist;
}


void options_set_datalist(void *newlist)
{
	options_data *p = newlist;
	datalist = p;

	if (p == NULL)
	{
		datalist_nextptr = &datalist;
		return;
	}

	while (p->next != NULL)
		p = p->next;

	datalist_nextptr = &p->next;
}
