/***************************************************************************

    clifront.c

    Command-line interface frontend for MAME.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "emuopts.h"
#include "clifront.h"
#include "hash.h"
#include "jedparse.h"
#include "audit.h"
#include "info.h"
#include "unzip.h"
#include "romload.h"
#include "sound/samples.h"
#include "osd_so.h" 
#include <ctype.h>



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _romident_status romident_status;
struct _romident_status
{
	int			total;				/* total files processed */
	int			matches;			/* number of matches found */
	int			nonroms;			/* number of non-ROM files found */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void parse_ini_file(const char *name);
static int execute_simple_commands(const char *exename);
static int execute_commands(const char *exename);
#ifdef DRIVER_SWITCH
void assign_drivers(void);
#endif /* DRIVER_SWITCH */
static void display_help(void);
static void setup_language(void); 
/* informational functions */
static int info_listxml(const char *gamename);
static int info_listgames(const char *gamename);
static int info_listfull(const char *gamename);
static int info_listsource(const char *gamename);
static int info_listclones(const char *gamename);
static int info_listcrc(const char *gamename);
static int info_listroms(const char *gamename);
static int info_listsamples(const char *gamename);
static int info_verifyroms(const char *gamename);
static int info_verifysamples(const char *gamename);
static int info_romident(const char *gamename);

/* utilities */
static const char *extract_base_name(const char *name);
static void romident(const char *filename, romident_status *status);
static void identify_file(const char *name, romident_status *status);
static void identify_data(const char *name, const UINT8 *data, int length, romident_status *status);
static void namecopy(char *name_ref, const char *desc);
static void match_roms(const char *hash, int length, int *found);



/***************************************************************************
    COMMAND-LINE OPTIONS
***************************************************************************/

const options_entry cli_options[] =
{
	/* core commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "CORE COMMANDS" },
	{ "help;h;?",                 "0",        OPTION_COMMAND,    "show help message" },
	{ "validate;valid",           "0",        OPTION_COMMAND,    "perform driver validation on all game drivers" },

	/* configuration commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "CONFIGURATION COMMANDS" },
	{ "createconfig;cc",          "0",        OPTION_COMMAND,    "create the default configuration file" },
	{ "showconfig;sc",            "0",        OPTION_COMMAND,    "display running parameters" },
	{ "showusage;su",             "0",        OPTION_COMMAND,    "show this help" },

	/* frontend commands */
	{ NULL,                       NULL,       OPTION_HEADER,     "FRONTEND COMMANDS" },
	{ "listxml;lx",               "0",        OPTION_COMMAND,    "all available info on driver in XML format" },
	{ "listgames",                "0",        OPTION_COMMAND,    "year, manufacturer and full name" },
	{ "listfull;ll",              "0",        OPTION_COMMAND,    "short name, full name" },
	{ "listsource;ls",            "0",        OPTION_COMMAND,    "driver sourcefile" },
	{ "listclones;lc",            "0",        OPTION_COMMAND,    "show clones" },
	{ "listcrc",                  "0",        OPTION_COMMAND,    "CRC-32s" },
	{ "listroms",                 "0",        OPTION_COMMAND,    "list required roms for a driver" },
	{ "listsamples",              "0",        OPTION_COMMAND,    "list optional samples for a driver" },
	{ "verifyroms",               "0",        OPTION_COMMAND,    "report romsets that have problems" },
	{ "verifysamples",            "0",        OPTION_COMMAND,    "report samplesets that have problems" },
	{ "romident",                 "0",        OPTION_COMMAND,    "compare files with known MAME roms" },
#ifdef MESS
	{ "listdevices",              "0",        OPTION_COMMAND,    "list available devices" },
#endif

	/* config options */
	{ NULL,                       NULL,       OPTION_HEADER,     "CONFIGURATION OPTIONS" },
	{ "readconfig;rc",            "1",        OPTION_BOOLEAN,    "enable loading of configuration files" },

	{ NULL }
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    is_directory_separator - is a given character
    a directory separator? The following logic
    works for most platforms
-------------------------------------------------*/

INLINE int is_directory_separator(char c)
{
	return (c == '\\' || c == '/' || c == ':');
}


/*-------------------------------------------------
    filename_ends_with - does the given
    filename end with the specified extension?
-------------------------------------------------*/

INLINE int filename_ends_with(const char *filename, const char *extension)
{
	int namelen = strlen(filename);
	int extlen = strlen(extension);
	int matches = TRUE;

	/* work backwards checking for a match */
	while (extlen > 0)
		if (tolower(filename[--namelen]) != tolower(extension[--extlen]))
		{
			matches = FALSE;
			break;
		}

	return matches;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    cli_execute - execute a game via the standard
    command line interface
-------------------------------------------------*/

int cli_execute(int argc, char **argv, const options_entry *osd_options)
{
	const char *exename = extract_base_name(argv[0]);
	const char *sourcename = NULL;
	const char *gamename = NULL;
	const game_driver *driver = NULL;
	machine_config drv;
	int result;

	/* initialize the options manager and add the CLI-specific options */
	mame_options_init(osd_options);
	options_add_entries(mame_options(), cli_options);

	/* parse the command line first; if we fail here, we're screwed */
	if (options_parse_command_line(mame_options(), argc, argv))
	{
		result = MAMERR_INVALID_CONFIG;
		goto error;
	}

	/* now parse the core set of INI files */
	parse_ini_file(CONFIGNAME);
	parse_ini_file(exename);
#ifdef MAME_DEBUG
	parse_ini_file("debug");
#endif

	// reparse the command line to ensure its options override all
	// note that we re-fetch the gamename here as it will get overridden
	options_parse_command_line(mame_options(), argc, argv);
	setup_language(); 
#ifdef DRIVER_SWITCH
	assign_drivers();
#endif /* DRIVER_SWITCH */ 

	/* parse the simple commmands before we go any further */
	result = execute_simple_commands(exename);
	if (result != -1)
		goto error;

	/* find out what game we might be referring to */
	gamename = options_get_string(mame_options(), OPTION_GAMENAME);
	if (gamename != NULL)
		gamename = extract_base_name(gamename);

	/* if we have a valid game driver, parse game-specific INI files */
	if (gamename != NULL)
		driver = driver_get_name(gamename);
	if (driver != NULL)
	{
		const game_driver *parent = driver_get_clone(driver);
		const game_driver *gparent = (parent != NULL) ? driver_get_clone(parent) : NULL;

		/* expand the machine driver to look at the info */
		expand_machine_driver(driver->drv, &drv);

		/* parse vector.ini for vector games */
		if (drv.video_attributes & VIDEO_TYPE_VECTOR)
			parse_ini_file("vector");

		/* then parse sourcefile.ini */
		sourcename = extract_base_name(driver->source_file);
		parse_ini_file(sourcename);

		/* then parent the grandparent, parent, and game-specific INIs */
		if (gparent != NULL)
			parse_ini_file(gparent->name);
		if (parent != NULL)
			parse_ini_file(parent->name);

#ifdef USE_IPS
		// HACK: DO NOT INHERIT IPS CONFIGURATION
		options_set_string(mame_options(), OPTION_IPS, NULL);
#endif /* USE_IPS */

		parse_ini_file(driver->name);
	}

	/* reparse the command line to ensure its options override all */
	options_parse_command_line(mame_options(), argc, argv);

	setup_language(); 
	/* execute any commands specified */
	result = execute_commands(exename);
	if (result != -1)
		goto error;

	/* if no driver specified, display help */
	if (gamename == NULL)
	{
		result = MAMERR_INVALID_CONFIG;
		display_help();
		goto error;
	}

	/* if we don't have a valid driver selected, offer some suggestions */
	if (driver == NULL)
	{
		const game_driver *matches[10];
		int drvnum;

		/* get the top 10 approximate matches */
		driver_get_approx_matches(gamename, ARRAY_LENGTH(matches), matches);

		/* print them out */
		mame_printf_error(_("\n\"%s\" approximately matches the following\n"
				"supported " GAMESNOUN " (best match first):\n\n"), gamename);
		for (drvnum = 0; drvnum < ARRAY_LENGTH(matches); drvnum++)
			if (matches[drvnum] != NULL)
				mame_printf_error("%-10s%s\n", matches[drvnum]->name, _LST(matches[drvnum]->description));

		/* exit with an error */
		result = MAMERR_NO_SUCH_GAME;
		goto error;
	}

	/* run the game */
	result = run_game(driver);

error:
#ifdef DRIVER_SWITCH
	free(drivers);
#endif /* DRIVER_SWITCH */ 

	/* free our options and exit */
	options_free(mame_options());
	if (exename != NULL)
		free((void *)exename);
	if (sourcename != NULL)
		free((void *)sourcename);
	if (gamename != NULL)
		free((void *)gamename);
	return result;
}


/*-------------------------------------------------
    parse_ini_file - parse a single INI file
-------------------------------------------------*/

static void parse_ini_file(const char *name)
{
	file_error filerr;
	mame_file *file;
	char *fname;

	/* don't parse if it has been disabled */
	if (!options_get_bool(mame_options(), CLIOPTION_READCONFIG))
		return;

	/* open the file; if we fail, that's ok */
	fname = assemble_2_strings(name, ".ini");
	filerr = mame_fopen(strcmp(CONFIGNAME, name) == 0 ? SEARCHPATH_RAW : SEARCHPATH_INI, fname, OPEN_FLAG_READ, &file);
	free(fname);
	if (filerr != FILERR_NONE)
		return;

	/* parse the file and close it */
	options_parse_ini_file(mame_options(), mame_core_file(file));
	mame_fclose(file);
}


/*-------------------------------------------------
    help_output - output callback for printing
    requested help information
-------------------------------------------------*/

static void help_output(const char *s)
{
	mame_printf_info("%s", s);
}


/*-------------------------------------------------
    execute_simple_commands - execute basic
    commands that don't require any context
-------------------------------------------------*/

static int execute_simple_commands(const char *exename)
{
	/* help? */
	if (options_get_bool(mame_options(), CLIOPTION_HELP))
	{
		setup_language();
		display_help();
		return MAMERR_NONE;
	}

	/* showusage? */
	if (options_get_bool(mame_options(), CLIOPTION_SHOWUSAGE))
	{
		setup_language();
		mame_printf_info(_("Usage: %s [%s] [options]\n\nOptions:\n"), exename, GAMENOUN);
		options_output_help(mame_options(), help_output);
		return MAMERR_NONE;
	}

	/* validate? */
	if (options_get_bool(mame_options(), CLIOPTION_VALIDATE))
	{
		extern int mame_validitychecks(const game_driver *driver);
		return mame_validitychecks(NULL);
	}

	return -1;
}


/*-------------------------------------------------
    execute_commands - execute various frontend
    commands
-------------------------------------------------*/

static int execute_commands(const char *exename)
{
	static const struct
	{
		const char *option;
		int (*function)(const char *gamename);
	} info_commands[] =
	{
		{ CLIOPTION_LISTXML,		info_listxml },
		{ CLIOPTION_LISTGAMES,		info_listgames },
		{ CLIOPTION_LISTFULL,		info_listfull },
		{ CLIOPTION_LISTSOURCE,		info_listsource },
		{ CLIOPTION_LISTCLONES,		info_listclones },
		{ CLIOPTION_LISTCRC,		info_listcrc },
#ifdef MESS
		{ CLIOPTION_LISTDEVICES,	info_listdevices },
#endif
		{ CLIOPTION_LISTROMS,		info_listroms },
		{ CLIOPTION_LISTSAMPLES,	info_listsamples },
		{ CLIOPTION_VERIFYROMS,		info_verifyroms },
		{ CLIOPTION_VERIFYSAMPLES,	info_verifysamples },
		{ CLIOPTION_ROMIDENT,		info_romident }
	};
	int i;

	/* createconfig? */
	if (options_get_bool(mame_options(), CLIOPTION_CREATECONFIG))
	{
		const char *filename;
		file_error filerr;
		mame_file *file;

		/* make the output filename */
		filename = assemble_2_strings(exename, ".ini");
		filerr = mame_fopen(NULL, exename, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS, &file);
		free((void *)filename);

		/* error if unable to create the file */
		if (filerr != FILERR_NONE)
		{
			mame_printf_error(_("Unable to create file %s.ini\n"), exename);
			return MAMERR_FATALERROR;
		}

		/* output the configuration and exit cleanly */
		options_output_ini_file(mame_options(), mame_core_file(file));
		mame_fclose(file);
		return MAMERR_NONE;
	}

	/* showconfig? */
	if (options_get_bool(mame_options(), CLIOPTION_SHOWCONFIG))
	{
		options_output_ini_stdfile(mame_options(), stdout);
		return MAMERR_NONE;
	}

	/* informational commands? */
	for (i = 0; i < ARRAY_LENGTH(info_commands); i++)
		if (options_get_bool(mame_options(), info_commands[i].option))
		{
			const char *gamename = options_get_string(mame_options(), OPTION_GAMENAME);
			return (*info_commands[i].function)((gamename == NULL) ? "*" : gamename);
		}

	return -1;
}


/*-------------------------------------------------
    display_help - display help to standard
    output
-------------------------------------------------*/

static void display_help(void)
{
#ifndef MESS
	mame_printf_info(_("M.A.M.E. v%s - Multiple Arcade Machine Emulator\n"
		   "Copyright (C) 1997-2007 by Nicola Salmoria and the MAME Team\n\n"),build_version);
	mame_printf_info("%s\n", _(mame_disclaimer));
	mame_printf_info(_("Usage:  MAME gamename [options]\n\n"
		   "        MAME -showusage    for a brief list of options\n"
		   "        MAME -showconfig   for a list of configuration options\n"
		   "        MAME -createconfig to create a mame.ini\n\n"
		   "For usage instructions, please consult the file windows.txt\n"));
#else
	showmessinfo();
#endif
}



#ifdef DRIVER_SWITCH
void assign_drivers(void)
{
	static const struct
	{
		const char *name;
		const game_driver * const *driver;
	} drivers_table[] =
	{
		{ "mame",	mamedrivers },
#ifndef TINY_NAME
		{ "plus",	plusdrivers },
		{ "homebrew",	homebrewdrivers },
		{ "neod",	neoddrivers },
	#ifndef NEOCPSMAME
		{ "noncpu",	noncpudrivers },
		{ "hazemd",	hazemddrivers },
	#endif /* NEOCPSMAME */
#endif /* !TINY_NAME */
		{ NULL }
	};

	UINT32 enabled = 0;
	int i, n;

#ifndef TINY_NAME
	const char *drv_option = options_get_string(mame_options(), OPTION_DRIVER_CONFIG);
	if (drv_option)
	{
		char *temp = mame_strdup(drv_option);
		if (temp)
		{
			char *p = strtok(temp, ",");
 			while (p)
			{
				char *s = mame_strtrim(p);	//get individual driver name
				if (s[0])
				{
					if (mame_stricmp(s, "all") == 0)
					{
						enabled = (UINT32)-1;
						break;
					}

					for (i = 0; drivers_table[i].name; i++)
						if (mame_stricmp(s, drivers_table[i].name) == 0)
						{
							enabled |= 1 << i;
							break;
						}

					if (!drivers_table[i].name)
						mame_printf_error("Illegal value for %s = %s\n", OPTION_DRIVER_CONFIG, s);
				}
				free(s);
 				p = strtok(NULL, ",");
			}
 			free(temp);
		}
	}
#endif /* !TINY_NAME */

	if (enabled == 0)
		enabled = 1;	// default to mamedrivers

	n = 0;
	for (i = 0; drivers_table[i].name; i++)
		if (enabled & (1 << i))
		{
			int c;

			for (c = 0; drivers_table[i].driver[c]; c++)
				n++;
		}

	if (drivers)
		free(drivers);
	drivers = malloc((n + 1) * sizeof (game_driver*));

	n = 0;
	for (i = 0; drivers_table[i].name; i++)
		if (enabled & (1 << i))
		{
			int c;

			for (c = 0; drivers_table[i].driver[c]; c++)
				drivers[n++] = drivers_table[i].driver[c];
		}

	drivers[n] = NULL;
}
#endif /* DRIVER_SWITCH */



//============================================================
//  setup_language
//============================================================

static void setup_language(void)
{
	const char *langname = options_get_string(mame_options(), OPTION_LANGUAGE);
	int use_lang_list =options_get_bool(mame_options(), OPTION_USE_LANG_LIST);

	int langcode = mame_stricmp(langname, "auto") ?
		lang_find_langname(langname) :
		lang_find_codepage(osd_get_default_codepage());

	if (langcode < 0)
	{
		langcode = UI_LANG_EN_US;
		lang_set_langcode(langcode);
		set_osdcore_acp(ui_lang_info[langcode].codepage);

		if (mame_stricmp(langname, "auto"))
			mame_printf_error("error: invalid value for language: %s\nUse %s\n",
		                langname, ui_lang_info[langcode].description);
	}

	lang_set_langcode(langcode);
	set_osdcore_acp(ui_lang_info[langcode].codepage);

	lang_message_enable(UI_MSG_LIST, use_lang_list);
	lang_message_enable(UI_MSG_MANUFACTURE, use_lang_list);
}



/***************************************************************************
    INFORMATIONAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    info_listxml - output the XML data for one
    or more games
-------------------------------------------------*/

static int info_listxml(const char *gamename)
{
	/* since print_mame_xml expands the machine driver, we need to set things up */
	init_resource_tracking();
	cpuintrf_init(NULL);
	sndintrf_init(NULL);

	print_mame_xml(stdout, drivers, gamename);

	/* clean up our tracked resources */
	exit_resource_tracking();
	return MAMERR_NONE;
}


/*-------------------------------------------------
    info_listfull - output the name and description
    of one or more games
-------------------------------------------------*/

static int info_listfull(const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if ((drivers[drvindex]->flags & NOT_A_DRIVER) == 0 && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			char name[200];

			/* print the header on the first one */
			if (count == 0)
				mame_printf_info(_("Name:     Description:\n"));

			count++;

			/* output the remaining information */
			mame_printf_info("%-10s", drivers[drvindex]->name);

			if (lang_message_is_enabled(UI_MSG_LIST))
			{
				strcpy(name, _LST(drivers[drvindex]->description));
				mame_printf_info("\"%s\"\n", name);
				continue;
			}

			namecopy(name,drivers[drvindex]->description);
			mame_printf_info("\"%s",name);

			/* print the additional description only if we are listing clones */
			{
				char *pdest = pdest = strstr(drivers[drvindex]->description, " (");

				if (pdest != NULL && pdest > drivers[drvindex]->description)
					mame_printf_info("%s", pdest);
			}

			mame_printf_info("\"\n");
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_listgames
-------------------------------------------------*/

int info_listgames(const char *gamename)
{
	int drvindex, count = 0;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if ((drivers[drvindex]->flags & NOT_A_DRIVER) == 0 && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			char name[200];

			mame_printf_info("%-5s%-36s ",drivers[drvindex]->year, _MANUFACT(drivers[drvindex]->manufacturer));

			if (lang_message_is_enabled(UI_MSG_LIST))
			{
				strcpy(name, _LST(drivers[drvindex]->description));
				mame_printf_info("\"%s\"\n", name);
				continue;
			}

			namecopy(name,drivers[drvindex]->description);
			mame_printf_info("\"%s",name);

			/* print the additional description only if we are listing clones */
			{
				char *pdest = strstr(drivers[drvindex]->description, " (");

				if (pdest != NULL && pdest > drivers[drvindex]->description)
					mame_printf_info("%s", pdest);
			}

			mame_printf_info("\"\n");

			count++;
		}

	/* return an error if none found */
	return (count > 0) ? 0 : 1;
}


/*-------------------------------------------------
    info_listsource - output the name and source
    filename of one or more games
-------------------------------------------------*/

static int info_listsource(const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			/* output the remaining information */
			mame_printf_info("%-8s %s\n", drivers[drvindex]->name, drivers[drvindex]->source_file);
			count++;
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_listclones - output the name and source
    filename of one or more games
-------------------------------------------------*/

static int info_listclones(const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		const game_driver *clone_of = driver_get_clone(drivers[drvindex]);

		/* if we are a clone, and either our name matches the gamename, or the clone's name matches, display us */
		if (clone_of != NULL && (clone_of->flags & NOT_A_DRIVER) == 0)
			if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0 || mame_strwildcmp(gamename, clone_of->name) == 0)
			{
				/* print the header on the first one */
				if (count == 0)
					mame_printf_info(_("Name:    Clone of:\n"));

				/* output the remaining information */
				mame_printf_info("%-8s %-8s\n", drivers[drvindex]->name, clone_of->name);
				count++;
			}
	}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_listcrc - output the CRC and name of
    all ROMs referenced by MAME
-------------------------------------------------*/

static int info_listcrc(const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			const rom_entry *region, *rom;

			/* iterate over regions, and then ROMs within the region */
			for (region = rom_first_region(drivers[drvindex]); region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					char hashbuf[HASH_BUF_SIZE];

					/* if we have a CRC, display it */
					if (hash_data_extract_printable_checksum(ROM_GETHASHDATA(rom), HASH_CRC, hashbuf))
						mame_printf_info("%s %-12s %s\n", hashbuf, ROM_GETNAME(rom), _LST(drivers[drvindex]->description));
				}

			count++;
		}

	/* return an error if none found */
	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_listroms - output the list of ROMs
    referenced by a given game or set of games
-------------------------------------------------*/

static int info_listroms(const char *gamename)
{
	int drvindex, count = 0;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			const rom_entry *region, *rom, *chunk;

			/* print the header */
			if (count > 0)
				mame_printf_info("\n");
			mame_printf_info(_("This is the list of the ROMs required for driver \"%s\".\n"
					"Name            Size Checksum\n"), drivers[drvindex]->name);

			/* iterate over regions and then ROMs within the region */
			for (region = drivers[drvindex]->rom; region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					const char *name = ROM_GETNAME(rom);
					const char* hash = ROM_GETHASHDATA(rom);
					char hashbuf[HASH_BUF_SIZE];
					int length = -1;

					/* accumulate the total length of all chunks */
					if (ROMREGION_ISROMDATA(region))
					{
						length = 0;
						for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
							length += ROM_GETLENGTH(chunk);
					}

					/* start with the name */
					mame_printf_info("%-12s ", name);

					/* output the length next */
					if (length >= 0)
						mame_printf_info("%7d", length);
					else
						mame_printf_info("       ");

					/* output the hash data */
					if (!hash_data_has_info(hash, HASH_INFO_NO_DUMP))
					{
						if (hash_data_has_info(hash, HASH_INFO_BAD_DUMP))
							mame_printf_info(_(" BAD"));

						hash_data_print(hash, 0, hashbuf);
						mame_printf_info(" %s", hashbuf);
					}
					else
						mame_printf_info(_(" NO GOOD DUMP KNOWN"));

					/* end with a CR */
					mame_printf_info("\n");
				}

			count++;
		}

	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_listsamples - output the list of samples
    referenced by a given game or set of games
-------------------------------------------------*/

static int info_listsamples(const char *gamename)
{
	int count = 0;

#if (HAS_SAMPLES)
	int drvindex;

	/* since we expand the machine driver, we need to set things up */
	init_resource_tracking();
	cpuintrf_init(NULL);
	sndintrf_init(NULL);

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			machine_config drv;
			int sndnum;

			/* expand the machine driver */
			expand_machine_driver(drivers[drvindex]->drv, &drv);

			/* find samples interfaces */
			for (sndnum = 0; drv.sound[sndnum].sound_type && sndnum < MAX_SOUND; sndnum++)
				if (drv.sound[sndnum].sound_type == SOUND_SAMPLES)
				{
					const char **samplenames = ((struct Samplesinterface *)drv.sound[sndnum].config)->samplenames;
					int sampnum;

					/* if the list is legit, walk it and print the sample info */
					if (samplenames != NULL)
						for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
							mame_printf_info("%s\n", samplenames[sampnum]);
				}

			count++;
		}

	/* clean up our tracked resources */
	exit_resource_tracking();
#else
	mame_printf_error("Samples not supported in this build\n");
#endif

	return (count > 0) ? MAMERR_NONE : MAMERR_NO_SUCH_GAME;
}


/*-------------------------------------------------
    info_verifyroms - verify the ROM sets of
    one or more games
-------------------------------------------------*/

static int info_verifyroms(const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			audit_record *audit;
			int audit_records;
			int res;

			/* audit the ROMs in this set */
			audit_records = audit_images(drvindex, AUDIT_VALIDATE_FAST, &audit);
			res = audit_summary(drvindex, audit_records, audit, TRUE);
			if (audit_records > 0)
				free(audit);

			/* if not found, count that and leave it at that */
			if (res == NOTFOUND)
				notfound++;

			/* else display information about what we discovered */
			else
			{
				const game_driver *clone_of;

				/* output the name of the driver and its clone */
				mame_printf_info(_("romset %s "), drivers[drvindex]->name);
				clone_of = driver_get_clone(drivers[drvindex]);
				if (clone_of != NULL)
					mame_printf_info("[%s] ", clone_of->name);

				/* switch off of the result */
				switch (res)
				{
					case INCORRECT:
						mame_printf_info(_("is bad\n"));
						incorrect++;
						break;

					case CORRECT:
						mame_printf_info(_("is good\n"));
						correct++;
						break;

					case BEST_AVAILABLE:
						mame_printf_info(_("is best available\n"));
						correct++;
						break;
				}
			}
		}

	/* clear out any cached files */
	zip_file_cache_clear();

	/* if we didn't get anything at all, display a generic end message */
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			mame_printf_info(_("romset \"%s\" not found!\n"), gamename);
		else
			mame_printf_info(_("romset \"%s\" not supported!\n"), gamename);
		return MAMERR_NO_SUCH_GAME;
	}

	/* otherwise, print a summary */
	else
	{
		mame_printf_info(_("%d romsets found, %d were OK.\n"), correct + incorrect, correct);
		return (incorrect > 0) ? MAMERR_MISSING_FILES : MAMERR_NONE;
	}
}


/*-------------------------------------------------
    info_verifysamples - verify the sample sets of
    one or more games
-------------------------------------------------*/

static int info_verifysamples(const char *gamename)
{
	int correct = 0;
	int incorrect = 0;
	int notfound = 0;
	int drvindex;

	/* now iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			audit_record *audit;
			int audit_records;
			int res;

			/* audit the samples in this set */
			audit_records = audit_samples(drvindex, &audit);
			res = audit_summary(drvindex, audit_records, audit, TRUE);
			if (audit_records > 0)
				free(audit);
			else
				continue;

			/* if not found, count that and leave it at that */
			if (res == NOTFOUND)
				notfound++;

			/* else display information about what we discovered */
			else
			{
				mame_printf_info(_("sampleset %s "), drivers[drvindex]->name);

				/* switch off of the result */
				switch (res)
				{
					case INCORRECT:
						mame_printf_info(_("is bad\n"));
						incorrect++;
						break;

					case CORRECT:
						mame_printf_info(_("is good\n"));
						correct++;
						break;

					case BEST_AVAILABLE:
						mame_printf_info(_("is best available\n"));
						correct++;
						break;
				}
			}
		}

	/* if we didn't get anything at all, display a generic end message */
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			mame_printf_error(_("sampleset \"%s\" not found!\n"), gamename);
		else
			mame_printf_error(_("sampleset \"%s\" not supported!\n"), gamename);
		return MAMERR_NO_SUCH_GAME;
	}

	/* otherwise, print a summary */
	else
	{
		mame_printf_info("%d samplesets found, %d were OK.\n", correct + incorrect, correct);
		return (incorrect > 0) ? MAMERR_MISSING_FILES : MAMERR_NONE;
	}
}


/*-------------------------------------------------
    info_romident - identify ROMs by looking for
    matches in our internal database
-------------------------------------------------*/

static int info_romident(const char *gamename)
{
	romident_status status;

	/* a NULL gamename is a fatal error */
	if (gamename == NULL)
		return MAMERR_FATALERROR;

	/* do the identification */
	romident(gamename, &status);

	/* clear out any cached files */
	zip_file_cache_clear();

	/* return the appropriate error code */
	if (status.matches == status.total)
		return MAMERR_NONE;
	else if (status.matches == status.total - status.nonroms)
		return MAMERR_IDENT_NONROMS;
	else if (status.matches > 0)
		return MAMERR_IDENT_PARTIAL;
	else
		return MAMERR_IDENT_NONE;
}



/***************************************************************************
    UTILITIES
***************************************************************************/

/*-------------------------------------------------
    extract_base_name - extract the base name
    from a filename; note that this makes
    assumptions about path separators
-------------------------------------------------*/

static const char *extract_base_name(const char *name)
{
	char *result, *dest;
	const char *start;

	/* find the start of the name */
	start = name + strlen(name);
	while (start > name && !is_directory_separator(start[-1]))
		start--;

	/* allocate memory for the new string */
	result = malloc(strlen(start) + 1);
	if (result == NULL)
		return NULL;

	/* copy in the base name up to the extension */
	dest = result;
	while (*start != 0 && *start != '.')
		*dest++ = *start++;
	*dest = 0;

	return result;
}


/*-------------------------------------------------
    romident - identify files
-------------------------------------------------*/

static void romident(const char *filename, romident_status *status)
{
	osd_directory *directory;

	/* reset the status */
	memset(status, 0, sizeof(*status));

	/* first try to open as a directory */
	directory = osd_opendir(filename);
	if (directory != NULL)
	{
		const osd_directory_entry *entry;

		/* iterate over all files in the directory */
		while ((entry = osd_readdir(directory)) != NULL)
			if (entry->type == ENTTYPE_FILE)
			{
				const char *curfile = assemble_3_strings(filename, PATH_SEPARATOR, entry->name);
				identify_file(curfile, status);
				free((void *)curfile);
			}
		osd_closedir(directory);
	}

	/* if that failed, and the filename ends with .zip, identify as a ZIP file */
	else if (filename_ends_with(filename, ".zip"))
	{
		/* first attempt to examine it as a valid ZIP file */
		zip_file *zip = NULL;
		zip_error ziperr = zip_file_open(filename, &zip);
		if (ziperr == ZIPERR_NONE && zip != NULL)
		{
			const zip_file_header *entry;

			/* loop over entries in the ZIP, skipping empty files and directories */
			for (entry = zip_file_first_file(zip); entry; entry = zip_file_next_file(zip))
				if (entry->uncompressed_length != 0)
				{
					UINT8 *data = (UINT8 *)malloc(entry->uncompressed_length);
					if (data != NULL)
					{
						/* decompress data into RAM and identify it */
						ziperr = zip_file_decompress(zip, data, entry->uncompressed_length);
						if (ziperr == ZIPERR_NONE)
							identify_data(entry->filename, data, entry->uncompressed_length, status);
						free(data);
					}
				}

			/* close up */
			zip_file_close(zip);
		}
	}

	/* otherwise, identify as a raw file */
	else
		identify_file(filename, status);
}


/*-------------------------------------------------
    identify_file - identify a file; if it is a
    ZIP file, scan it and identify all enclosed
    files
-------------------------------------------------*/

static void identify_file(const char *name, romident_status *status)
{
	file_error filerr;
	osd_file *file;
	UINT64 length;

	/* open for read and process if it opens and has a valid length */
	filerr = osd_open(name, OPEN_FLAG_READ, &file, &length);
	if (filerr == FILERR_NONE && length > 0 && (UINT32)length == length)
	{
		UINT8 *data = (UINT8 *)malloc(length);
		if (data != NULL)
		{
			UINT32 bytes;

			/* read file data into RAM and identify it */
			filerr = osd_read(file, data, 0, length, &bytes);
			if (filerr == FILERR_NONE)
				identify_data(name, data, bytes, status);
			free(data);
		}
		osd_close(file);
	}
}


/*-------------------------------------------------
    identify_data - identify a buffer full of
    data; if it comes from a .JED file, parse the
    fusemap into raw data first
-------------------------------------------------*/

static void identify_data(const char *name, const UINT8 *data, int length, romident_status *status)
{
	char hash[HASH_BUF_SIZE];
	UINT8 *tempjed = NULL;
	const char *basename;
	int found = 0;
	jed_data jed;

	/* if this is a '.jed' file, process it into raw bits first */
	if (filename_ends_with(name, ".jed") && jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		/* now determine the new data length and allocate temporary memory for it */
		length = jedbin_output(&jed, NULL, 0);
		tempjed = malloc(length);
		if (tempjed == NULL)
			return;

		/* create a binary output of the JED data and use that instead */
		jedbin_output(&jed, tempjed, length);
		data = tempjed;
	}

	/* compute the hash of the data */
	hash_data_clear(hash);
	hash_compute(hash, data, length, HASH_SHA1 | HASH_CRC);

	/* output the name */
	status->total++;
	basename = extract_base_name(name);
	mame_printf_info("%-20s", (basename != NULL) ? basename : name);
	if (basename != NULL)
		free((void *)basename);

	/* see if we can find a match in the ROMs */
	match_roms(hash, length, &found);

	/* if we didn't find it, try to guess what it might be */
	if (found == 0)
	{
		/* if not a power of 2, assume it is a non-ROM file */
		if ((length & (length - 1)) != 0)
		{
			mame_printf_info(_("NOT A ROM\n"));
			status->nonroms++;
		}

		/* otherwise, it's just not a match */
		else
			mame_printf_info(_("NO MATCH\n"));
	}

	/* if we did find it, count it as a match */
	else
		status->matches++;

	/* free any temporary JED data */
	if (tempjed != NULL)
		free(tempjed);
}


static void namecopy(char *name_ref, const char *desc)
{
	char name[200];

	if (lang_message_is_enabled(UI_MSG_LIST))
	{
		strcpy(name, _LST(desc));
		if (strstr(name," (")) *strstr(name," (") = 0;
		sprintf(name_ref,"%s",name);
		return;
	}

	strcpy(name,desc);

	/* remove details in parenthesis */
	if (strstr(name," (")) *strstr(name," (") = 0;

	/* Move leading "The" to the end */
	if (strncmp(name,"The ",4) == 0)
		sprintf(name_ref,"%s, The",name+4);
	else
		sprintf(name_ref,"%s",name);
}


/*-------------------------------------------------
    match_roms - scan for a matching ROM by hash
-------------------------------------------------*/

static void match_roms(const char *hash, int length, int *found)
{
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		const rom_entry *region, *rom;

		/* iterate over regions and files within the region */
		for (region = rom_first_region(drivers[drvindex]); region; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				if (hash_data_is_equal(hash, ROM_GETHASHDATA(rom), 0))
				{
					int baddump = hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP);

					/* output information about the match */
					if (*found != 0)
						mame_printf_info("                    ");
					mame_printf_info("= %s%-20s  %s\n", baddump ? _("(BAD) ") : "", ROM_GETNAME(rom), _LST(drivers[drvindex]->description));
					(*found)++;
				}
	}
}
