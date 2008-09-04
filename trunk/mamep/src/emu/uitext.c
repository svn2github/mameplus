/*********************************************************************

    uitext.c

    Functions used to retrieve text used by MAME, to aid in
    translation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "uitext.h"

#ifdef MAMEMESS
#define MESS
#endif /* MAMEMESS */
#ifdef MESS
extern const char *mess_default_text[];
#endif /* MESS */


/* All entries in this table must match the enum ordering in "uitext.h" */
static const char *const mame_default_text[] =
{
	APPNAME,

	/* misc stuff */
	"Return to Main Menu",
	"Press Any Key",
	"On",
	"Off",
	"OK",
	"Address",
	"Value",
	"stereo",
	"Screen Resolution",
	"Text",
	"Relative",
	"ALL CHANNELS",
	"Brightness",
	"Contrast",
	"Gamma",
	"Vector Flicker",
	"ALL CPUS",
	HISTORYNAME " not available",
#ifdef STORY_DATAFILE
	"Story not available",
#endif /* STORY_DATAFILE */
	"Mameinfo not available",
	"Driverinfo not available",
#ifdef CMD_LIST
	"Command List not available",
#endif /* CMD_LIST */

	/* special characters */
	"\xc2\xab",
	"\xc2\xbb",
	"(more)",
	"(more)",
	"\x1a",
	"\x1b",

	/* main menu */
	"Calibrate Joysticks",
	"Reset " CAPSTARTGAMENOUN,
	/* documents menu */
	CAPSTARTGAMENOUN " " HISTORYNAME,
#ifdef STORY_DATAFILE
	CAPSTARTGAMENOUN " Story",
#endif /* STORY_DATAFILE */
	CAPSTARTGAMENOUN " Mameinfo",
	CAPSTARTGAMENOUN " Driverinfo",
#ifdef CMD_LIST
	"Show Command List",
#endif /* CMD_LIST */

	"Rotate Clockwise",
	"Rotate Counter-clockwise",
	"Flip X",
	"Flip Y",

	/* centering */
	"Center",

	NULL
};



static const char *const *const default_text[] =
{
	mame_default_text,
#ifdef MESS
	mess_default_text,
#endif /* MESS */
	NULL
};



static const char **trans_text;


#if 0
int uistring_init (mame_file *langfile)
{
	/*
        TODO: This routine needs to do several things:
            - load an external font if needed
            - determine the number of characters in the font
            - deal with multibyte languages

    */

	int i, j, str;
	char curline[255];
	char section[255] = "\0";
	char *ptr;
	int string_count;

	/* count the total amount of strings */
	string_count = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			string_count++;
	}

	/* allocate the translated text array, and set defaults */
	trans_text = auto_malloc(sizeof(const char *) * string_count);

	/* copy in references to all of the strings */
	str = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			trans_text[str++] = default_text[i][j];
	}

	memset(&lang, 0, sizeof(lang));

	/* if no language file, exit */
	if (!langfile)
		return 0;

	while (mame_fgets (curline, sizeof(curline) / sizeof(curline[0]), langfile) != NULL)
	{
		/* Ignore commented and blank lines */
		if (curline[0] == ';') continue;
		if (curline[0] == '\n') continue;
		if (curline[0] == '\r') continue;

		if (curline[0] == '[')
		{
			ptr = strtok (&curline[1], "]");
			/* Found a section, indicate as such */
			strcpy (section, ptr);

			/* Skip to the next line */
			continue;
		}

		/* Parse the LangInfo section */
		if (strcmp (section, "LangInfo") == 0)
		{
			ptr = strtok (curline, "=");
			if (strcmp (ptr, "Version") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				sscanf (ptr, "%d", &lang.version);
			}
			else if (strcmp (ptr, "Language") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.langname, ptr);
			}
			else if (strcmp (ptr, "Author") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.author, ptr);
			}
			else if (strcmp (ptr, "Font") == 0)
			{
				ptr = strtok (NULL, "\n\r");
				strcpy (lang.fontname, ptr);
			}
		}

		/* Parse the Strings section */
		if (strcmp (section, "Strings") == 0)
		{
			/* Get all text up to the first line ending */
			ptr = strtok (curline, "\n\r");

			/* Find a matching default string */
			str = 0;
			for (i = 0; default_text[i]; i++)
			{
				for (j = 0; default_text[i][j]; j++)
				{
					if (strcmp (curline, default_text[i][j]) == 0)
				{
					char transline[255];

					/* Found a match, read next line as the translation */
					mame_fgets (transline, 255, langfile);

					/* Get all text up to the first line ending */
					ptr = strtok (transline, "\n\r");

					/* Allocate storage and copy the string */
						trans_text[str] = auto_strdup(transline);
					}
					str++;
				}
			}
		}
	}

	/* indicate success */
	return 0;
}
#else
int uistring_init (void)
{
	int i, j, str;
	int string_count;

	/* count the total amount of strings */
	string_count = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			string_count++;
	}

	/* allocate the translated text array, and set defaults */
	trans_text = auto_malloc(sizeof(const char *) * string_count);

	/* copy in references to all of the strings */
	str = 0;
	for (i = 0; default_text[i]; i++)
	{
		for (j = 0; default_text[i][j]; j++)
			trans_text[str++] = _(default_text[i][j]);
	}

	/* indicate success */
	return 0;
}
#endif


const char * ui_getstring (int string_num)
{
	//mamep: fixme: since 124u2
	if (trans_text)
		return trans_text[string_num];
	else 
		return "";
}
