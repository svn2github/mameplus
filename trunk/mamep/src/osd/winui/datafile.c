/***************************************************************************

  M.A.M.E.UI  -  Multiple Arcade Machine Emulator with User Interface
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse,
  Copyright (C) 2003-2007 Chris Kirmse and the MAME32/MAMEUI team.

  This file is part of MAMEUI, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

/****************************************************************************
 *      datafile.c
 *      History database engine
 *
 *      Token parsing by Neil Bradley
 *      Modifications and higher-level functions by John Butler
 ****************************************************************************/
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// standard C headers
#include <assert.h>
#include <ctype.h>

// MAME/MAMEUI headers
#include "emu.h"
#include "emuopts.h" // For OPTION_LANGPATH
#include "osdcomm.h"
#include "datafile.h"
#include "mui_opts.h" // For MameUIGlobal()

/****************************************************************************
 *      token parsing constants
 ****************************************************************************/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CR	0x0d	/* '\n' and '\r' meanings are swapped in some */
#define LF	0x0a	/*     compilers (e.g., Mac compilers) */

enum
{
	TOKEN_COMMA,
	TOKEN_EQUALS,
	TOKEN_SYMBOL,
	TOKEN_LINEBREAK,
	TOKEN_INVALID=-1
};

#define MAX_TOKEN_LENGTH        4096


/****************************************************************************
 *      datafile constants
 ****************************************************************************/
#define DATAFILE_TAG '$'

static const char *DATAFILE_TAG_KEY = "$info";
static const char *DATAFILE_TAG_BIO = "$bio";
#ifdef STORY_DATAFILE
static const char *DATAFILE_TAG_STORY = "$story";
#endif /* STORY_DATAFILE */
static const char *DATAFILE_TAG_MAME = "$mame";
static const char *DATAFILE_TAG_DRIV = "$drv";

#define FILE_MERGED	1
#define FILE_ROOT	2
#define FILE_TYPEMAX	((FILE_MERGED | FILE_ROOT) + 1)

static windows_options *datafile_options;

struct tDatafileIndex
{
	UINT64 offset;
	const game_driver *driver;
};

static struct tDatafileIndex *hist_idx[FILE_TYPEMAX];
#ifdef STORY_DATAFILE
static struct tDatafileIndex *story_idx[FILE_TYPEMAX];
#endif /* STORY_DATAFILE */
static struct tDatafileIndex *mame_idx[FILE_TYPEMAX];
static struct tDatafileIndex *driv_idx;


/****************************************************************************
 *      private data for parsing functions
 ****************************************************************************/
static emu_file *fp = NULL;				/* Our file pointer */
static UINT64 dwFilePos;				/* file position */
static UINT8 bToken[MAX_TOKEN_LENGTH];		/* Our current token */

/* an array of driver name/drivers array index sorted by driver name
   for fast look up by name */
typedef struct
{
	const char *name;
	int index;
} driver_data_type;

typedef struct
{
	const char *srcdriver;
	int index;
} srcdriver_data_type;

static driver_data_type *sorted_drivers = NULL;
static srcdriver_data_type *sorted_srcdrivers = NULL;
static int num_games;
static void flush_index(void);
static void ParseClose(void);


/****************************************************************************
 *      startup and shutdown functions
 ****************************************************************************/

void winui_datafile_init(windows_options &options)
{
	datafile_options = &options;

	num_games = driver_list::total();
}

void winui_datafile_exit(void)
{
	ParseClose();

	flush_index();

	if (sorted_drivers)
	{
		free(sorted_drivers);
		sorted_drivers = NULL;
	}

	if (sorted_srcdrivers)
	{
		free(sorted_srcdrivers);
		sorted_srcdrivers = NULL;
	}
}


/**************************************************************************
 **************************************************************************
 *
 *      Parsing functions
 *
 **************************************************************************
 **************************************************************************/

/*
 * DriverDataCompareFunc -- compare function for GetGameNameIndex
 */
static int CLIB_DECL DriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((driver_data_type *)arg1)->name, ((driver_data_type *)arg2)->name );
}

/*
 * GetGameNameIndex -- given a driver name (in lowercase), return
 * its index in the main drivers[] array, or -1 if it's not found.
 */
static int GetGameNameIndex(const char *name)
{
	driver_data_type *driver_index_info;
	driver_data_type key;
	key.name = name;

	if (sorted_drivers == NULL)
	{
		/* initialize array of game names/indices */
		int i;

		sorted_drivers = (driver_data_type *)malloc(sizeof(driver_data_type) * num_games);
		for (i = 0; i < num_games; i++)
		{
			sorted_drivers[i].name = driver_list::driver(i).name;
			sorted_drivers[i].index = i;
		}
		qsort(sorted_drivers, num_games, sizeof(driver_data_type), DriverDataCompareFunc);
	}

	/* uses our sorted array of driver names to get the index in log time */
	driver_index_info = (driver_data_type *)bsearch(&key, sorted_drivers,num_games, sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

}


/****************************************************************************
 *      Create an array with sorted sourcedrivers for the function
 *      index_datafile_drivinfo to speed up the datafile access
 ****************************************************************************/

static int SrcDriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((srcdriver_data_type *)arg1)->srcdriver, ((srcdriver_data_type *)arg2)->srcdriver );
}


static int GetSrcDriverIndex(const char *srcdriver)
{
	srcdriver_data_type *srcdriver_index_info;
	srcdriver_data_type key;
	unsigned char *s = (unsigned char *)core_strdup(srcdriver);
	int i;

	if (s == NULL)
		return -1;

	for (i = 0; s[i]; i++)
		s[i] = tolower(s[i]);

	if (sorted_srcdrivers == NULL)
	{
		/* initialize array of game names/indices */
		int i;

		sorted_srcdrivers = (srcdriver_data_type *)malloc(sizeof(srcdriver_data_type) * num_games);
		for (i = 0; i < num_games; i++)
		{
			sorted_srcdrivers[i].srcdriver = driver_list::driver(i).source_file+17;
			sorted_srcdrivers[i].index = i;
		}
		qsort(sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),SrcDriverDataCompareFunc);
	}

	key.srcdriver = (const char *)s;
	srcdriver_index_info = (srcdriver_data_type *)bsearch(&key, sorted_srcdrivers, num_games, sizeof(srcdriver_data_type),
	                               SrcDriverDataCompareFunc);
	osd_free(s);

	if (srcdriver_index_info == NULL)
		return -1;

	return srcdriver_index_info->index;
}


/****************************************************************************
 *      GetNextToken - Pointer to the token string pointer
 *                     Pointer to position within file
 *
 *      Returns token, or TOKEN_INVALID if at end of file
 ****************************************************************************/
static UINT32 GetNextToken(UINT8 **ppszTokenText, UINT64 *pdwPosition)
{
	UINT32 dwLength;			/* Length of symbol */
	UINT8 *pbTokenPtr = bToken;		/* Point to the beginning */
	UINT8 bData;				/* Temporary data byte */

	while (1)
	{
		dwFilePos = fp->tell();
		bData = fp->getc();		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (fp->eof())
			return(TOKEN_INVALID);

		/* If it's not whitespace, then let's start eating characters */

		if (' ' != bData && '\t' != bData)
		{
			/* Store away our file position (if given on input) */

			if (pdwPosition)
				*pdwPosition = dwFilePos;

			/* If it's a separator, special case it */

			if (',' == bData || '=' == bData)
			{
				*pbTokenPtr++ = bData;
				*pbTokenPtr = '\0';
				dwFilePos = fp->tell();

				if (',' == bData)
					return(TOKEN_COMMA);
				else
					return(TOKEN_EQUALS);
			}

			/* Otherwise, let's try for a symbol */

			if (bData > ' ')
			{
				dwLength = 0;			/* Assume we're 0 length to start with */

				/* Loop until we've hit something we don't understand */

				while (bData != ',' &&
				       bData != '=' &&
				       bData != ' ' &&
				       bData != '\t' &&
				       bData != '\n' &&
				       bData != '\r' &&
				       fp->eof() == 0)
				{
					dwFilePos = fp->tell();
					*pbTokenPtr++ = bData;	/* Store our byte */
					++dwLength;
					assert(dwLength < MAX_TOKEN_LENGTH);
					bData = fp->getc();
				}

				/* If it's not the end of the file, put the last received byte */
				/* back. We don't want to touch the file position, though if */
				/* we're past the end of the file. Otherwise, adjust it. */

				if (0 == fp->eof())
				{
					fp->seek(dwFilePos, SEEK_SET);
				}

				/* Null terminate the token */

				*pbTokenPtr = '\0';

				/* Connect up the */

				if (ppszTokenText)
					*ppszTokenText = bToken;

				return(TOKEN_SYMBOL);
			}

			/* Not a symbol. Let's see if it's a cr/cr, lf/lf, or cr/lf/cr/lf */
			/* sequence */

			if (LF == bData)
			{
				UINT64 dwPos;

				/* Unix style perhaps? */

				dwFilePos = fp->tell();
				bData = fp->getc();		/* Peek ahead */
				dwPos = fp->tell();
				fp->seek(dwFilePos, SEEK_SET);	/* Force a retrigger if subsequent LF's */

				if (LF == bData)		/* Two LF's in a row - it's a UNIX hard CR */
				{
					dwFilePos = dwPos;
					*pbTokenPtr++ = bData;	/* A real linefeed */
					*pbTokenPtr = '\0';
					return(TOKEN_LINEBREAK);
				}

				/* Otherwise, fall through and keep parsing. */

			}
			else
			if (CR == bData)			/* Carriage return? */
			{
				/* Figure out if it's Mac or MSDOS format */

				dwFilePos = fp->tell();
				bData = fp->getc();		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					fp->seek(dwFilePos, SEEK_SET);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';
					return(TOKEN_LINEBREAK);
				}
				else
				if (LF == bData)		/* MSDOS format! */
				{
						UINT64 dwPos;

						dwFilePos = fp->tell();	/* Our file position to reset to */
						dwPos = dwFilePos;	/* Here so we can reposition things */

						/* Look for a followup CR/LF */

						bData = fp->getc();	/* Get the next byte */

						if (CR == bData)	/* CR! Good! */
						{
							bData = fp->getc();	/* Get the next byte */

							/* We need to do this to pick up subsequent CR/LF sequences */

							fp->seek(dwPos, SEEK_SET);

							if (pdwPosition)
								*pdwPosition = dwPos;

							if (LF == bData) 	/* LF? Good! */
							{
								*pbTokenPtr++ = '\r';
								*pbTokenPtr++ = '\n';
								*pbTokenPtr = '\0';

								return(TOKEN_LINEBREAK);
							}
						}
						else
						{
							fp->seek(dwFilePos, SEEK_SET);	/* Put the character back. No good */
						}
					}
				else
				{
					fp->seek(dwFilePos, SEEK_SET);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}
	}
}


/****************************************************************************
 *      ParseClose - Closes the existing opened file (if any)
 ****************************************************************************/
static void ParseClose(void)
{
	/* If the file is open, time for fclose. */
	if (fp)
	{
		fp->close();
		global_free(fp);
	}

	fp = NULL;
}

/****************************************************************************
 *      ParseOpen - Open up file for reading
 ****************************************************************************/
static UINT8 ParseOpen(const char *pszFilename)
{
	file_error filerr;

	ParseClose();

	/* Open file up in binary mode */
	fp = global_alloc(emu_file(OPEN_FLAG_READ));
	filerr = fp->open(pszFilename);
	if (filerr != FILERR_NONE)
		return(FALSE);

	/* Otherwise, prepare! */
	dwFilePos = 0;

	/* identify text file type first */
	fp->getc();
	fp->seek(dwFilePos, SEEK_SET);

	return(TRUE);
}

/****************************************************************************
 *      ParseSeek - Move the file position indicator
 ****************************************************************************/
static UINT8 ParseSeek(UINT64 offset, int whence)
{
	int result = fp->seek(offset, whence);

	if (0 == result)
	{
		dwFilePos = fp->tell();
	}
	return (UINT8)result;
}


/**************************************************************************
 *      index_datafile
 *      Create an index for the records in the currently open datafile.
 *
 *      Returns 0 on error, or the number of index entries created.
 **************************************************************************/
static int index_datafile (struct tDatafileIndex **_index)
{
	struct tDatafileIndex *idx;
	int count = 0;
	UINT32 token = TOKEN_SYMBOL;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
	idx = *_index = (tDatafileIndex *)malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
	while (count < num_games && TOKEN_INVALID != token)
	{
		UINT64 tell;
		UINT8 *s;

		token = GetNextToken (&s, &tell);
		if (TOKEN_SYMBOL != token) continue;

		/* DATAFILE_TAG_KEY identifies the driver */
		if (!core_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
		{
			token = GetNextToken (&s, &tell);
			if (TOKEN_EQUALS == token)
			{
				int done = 0;

				token = GetNextToken (&s, &tell);
				while (count < num_games && !done && TOKEN_SYMBOL == token)
				{
					int game_index;
					UINT8 *p;
					for (p = s; *p; p++)
						*p = tolower(*p);

					game_index = GetGameNameIndex((char *)s);
					if (game_index >= 0)
					{
						idx->driver = &driver_list::driver(game_index);
						idx->offset = tell;
						idx++;
						count++;
						/* done = 1;  Not done, as we must process other clones in list */

					}
					if (!done)
					{
						token = GetNextToken (&s, &tell);

						if (TOKEN_COMMA == token)
							token = GetNextToken (&s, &tell);
						else
							done = 1;	/* end of key field */
					}
				}
			}
		}
	}

	/* mark end of index */
	idx->offset = 0L;
	idx->driver = 0;
	return count;
}

static void flush_index(void)
{
	int i;

	for (i = 0; i < FILE_TYPEMAX; i++)
	{
		if (i & FILE_ROOT)
			continue;

		if (hist_idx[i])
		{
			free(hist_idx[i]);
			hist_idx[i] = 0;
		}

#ifdef STORY_DATAFILE
		if (story_idx[i])
		{
			free(story_idx[i]);
			story_idx[i] = 0;
		}
#endif /* STORY_DATAFILE */

		if (mame_idx[i])
		{
			free(mame_idx[i]);
			mame_idx[i] = 0;
		}
	}
}


static int index_datafile_drivinfo (struct tDatafileIndex **_index)
{
	struct tDatafileIndex *idx;
	int count = 0;
	UINT32 token = TOKEN_SYMBOL;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
	idx = *_index = (tDatafileIndex *)malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
	while (count < num_games && TOKEN_INVALID != token)
	{
		UINT64 tell;
		UINT8 *s;

		token = GetNextToken (&s, &tell);
		if (TOKEN_SYMBOL != token) continue;

		/* DATAFILE_TAG_KEY identifies the driver */
		if (!core_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
		{
			token = GetNextToken (&s, &tell);
			if (TOKEN_EQUALS == token)
			{
				int done = 0;

				token = GetNextToken (&s, &tell);
				while (count < num_games && !done && TOKEN_SYMBOL == token)
				{
					int src_index;
					src_index = GetSrcDriverIndex((const char*)s);
					if (src_index >= 0)
					{
						idx->driver = &driver_list::driver(src_index);
						idx->offset = tell;
						idx++;
						count++;
						/* done = 1;  Not done, as we must process other clones in list */
						//break;
					}

					if (!done)
					{
						token = GetNextToken (&s, &tell);
						if (TOKEN_COMMA == token)
							token = GetNextToken (&s, &tell);
						else
							done = 1;	/* end of key field */
					}
				}
			}
		}
	}

	/* mark end of index */
	idx->offset = 0L;
	idx->driver = 0;
	return count;
}

/**************************************************************************
 *      load_datafile_text
 *
 *      Loads text field for a driver into the buffer specified. Specify the
 *      driver, a pointer to the buffer, the buffer size, the index created by
 *      index_datafile(), and the desired text field (e.g., DATAFILE_TAG_BIO).
 *
 *      Returns 0 if successful.
 **************************************************************************/
static int load_datafile_text (const game_driver *drv, char *buffer, int bufsize,
                               struct tDatafileIndex *idx, const char *tag)
{
	int offset = 0;
	int found = 0;
	UINT32  token = TOKEN_SYMBOL;
	UINT32  prev_token = TOKEN_SYMBOL;

	*buffer = '\0';

	/* find driver in datafile index */
	while (idx->driver)
	{

		if (idx->driver == drv) break;

		idx++;
	}
	if (idx->driver == 0) return 1;	/* driver not found in index */

	/* seek to correct point in datafile */
	if (ParseSeek (idx->offset, SEEK_SET)) return 1;

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		UINT8 *s;
		int len;
		UINT64 tell;

		token = GetNextToken (&s, &tell);
		if (TOKEN_INVALID == token) continue;

		if (found)
		{
			/* end entry when a tag is encountered */
			if (TOKEN_SYMBOL == token && DATAFILE_TAG == s[0] && TOKEN_LINEBREAK == prev_token) break;

			prev_token = token;

			/* translate platform-specific linebreaks to '\n' */
			if (TOKEN_LINEBREAK == token)
				strcpy ((char *)s, "\n");

			/* append a space to words */
			if (TOKEN_LINEBREAK != token)
				strcat ((char *)s, " ");

			/* remove extraneous space before commas */
			if (TOKEN_COMMA == token)
			{
				--buffer;
				--offset;
				*buffer = '\0';
			}

			/* Get length of text to add to the buffer */
			len = strlen ((char *)s);

			/* Check for buffer overflow */
			/* For some reason we can get a crash if we try */
			/* to use the last 30 characters of the buffer  */
			if ((bufsize - offset) - len <= 45)
			{
				strcpy ((char *)s, " ...[TRUNCATED]");
				len = strlen((char *)s);
				strcpy (buffer, (char *)s);
				buffer += len;
				offset += len;
				break;
			}

			/* add this word to the buffer */
			strcpy (buffer, (char *)s);
			buffer += len;
			offset += len;
		}
		else
		{
			if (TOKEN_SYMBOL == token)
			{
				/* looking for requested tag */
				if (!core_strnicmp (tag, (char *)s, strlen (tag)))
				{
					found = 1;
				}
				else if (!core_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
					break;	/* error: tag missing */
			}
		}
	}
	return (!found);
}


static int load_drivfile_text (const game_driver *drv, char *buffer, int bufsize,
	struct tDatafileIndex *idx, const char *tag)
{
	int     offset = 0;
	int     found = 0;
	UINT32  token = TOKEN_SYMBOL;
	UINT32  prev_token = TOKEN_SYMBOL;
	struct tDatafileIndex *idx_save = idx;

	*buffer = '\0';

	/* find driver in datafile index */
	while (idx->driver)
	{
		if (idx->driver->source_file == drv->source_file) break;
		idx++;
	}
	// MSVC doesn't work above, retry but slow. :-(
	if (idx->driver == 0)
	{
		idx = idx_save;

		while (idx->driver)
		{
			if (!strcmp(idx->driver->source_file, drv->source_file))
				break;
			idx++;
		}
	}

	if (idx->driver == 0) return 1;	/* driver not found in index */

	/* seek to correct point in datafile */
	if (ParseSeek (idx->offset, SEEK_SET)) return 1;

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		UINT8 *s;
		int len;
		UINT64 tell;

		token = GetNextToken (&s, &tell);
		if (TOKEN_INVALID == token) continue;

		if (found)
		{
			/* end entry when a tag is encountered */
			if (TOKEN_SYMBOL == token && DATAFILE_TAG == s[0] && TOKEN_LINEBREAK == prev_token) break;

			prev_token = token;

			/* translate platform-specific linebreaks to '\n' */
			if (TOKEN_LINEBREAK == token)
				strcpy ((char *)s, "\n");

			/* append a space to words */
			if (TOKEN_LINEBREAK != token)
				strcat ((char *)s, " ");

			/* remove extraneous space before commas */
			if (TOKEN_COMMA == token)
			{
				--buffer;
				--offset;
				*buffer = '\0';
			}

			/* add this word to the buffer */
			len = strlen ((char *)s);
			if ((len + offset) >= bufsize) break;
			strcpy (buffer, (char *)s);
			buffer += len;
			offset += len;
		}
		else
		{
			if (TOKEN_SYMBOL == token)
			{
				/* looking for requested tag */
				if (!core_strnicmp (tag, (char *)s, strlen (tag)))
					found = 1;
				else if (!core_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
					break;	/* error: tag missing */
			}
		}
	}
	return (!found);
}


/**************************************************************************
 *	load_datafile
 *
 *	Returns 0 if successful,
 *              1 if failure.
 *
 *	NOTE: For efficiency the indices are never freed (intentional leak).
 **************************************************************************/
static int load_datafile (const game_driver *drv, char *buffer, int bufsize,
                             const char *tag, int where, struct tDatafileIndex *idx[],
                             const char *separated_dir, const char *merged_name)
{
	const game_driver *gdrv;
	char filename[80];
	char *base;
	int cl;

	filename[0] = '\0';

	if (where != FILE_ROOT)
	{
		sprintf(filename, "%s\\%s\\",
	        	MameUIGlobal().value(OPTION_LANGPATH),
			ui_lang_info[lang_get_langcode()].name);
	}

	base = filename + strlen(filename);

	gdrv = drv;
	while (gdrv && gdrv->name[0])
	{
		int i;

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;
			int err;

			if (i & FILE_MERGED)
			{
				strcpy(base, merged_name);

				/* try to open datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index if necessary */
				if (idx[i])
					status = 1;
				else
					status = (index_datafile (&idx[i]) != 0);

				/* load text */
				if (idx[i])
				{
					int len = strlen (buffer);

					err = load_datafile_text (gdrv, buffer+len, bufsize-len, idx[i], tag);
					if (err) status = 0;
				}
			}
			else
			{
				const game_driver *pdrv;

				strcpy(base, separated_dir);
				strcat(base, gdrv->name);
				strcat(base, ".dat");

				/* try to open datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index */
				if (idx[i])
				{
					free(idx[i]);
					idx[i] = 0;
				}

				status = (index_datafile (&idx[i]) != 0);

				pdrv = drv;
				do
				{
					int len = strlen (buffer);

					err = load_datafile_text (pdrv, buffer+len, bufsize-len, idx[i], tag);

					if (pdrv == gdrv)
						break;

					int g = driver_list::clone(*pdrv);
					if (g !=-1) pdrv = &driver_list::driver(g); else pdrv = NULL;
				} while (err && pdrv);

				if (err) status = 0;
			}

			ParseClose ();

			if (status)
				return 0;
		}

		cl = driver_list::clone(*gdrv);
		if (cl != -1) gdrv = &driver_list::driver(cl); else gdrv = NULL;
	}

	return 1;
}


/**************************************************************************
 *	flush_index_if_needed
 **************************************************************************/
static void flush_index_if_needed(void)
{
	static int oldLangCode = -1;

	if (oldLangCode != lang_get_langcode())
	{
		flush_index();

		oldLangCode = lang_get_langcode();
	}
}


/**************************************************************************
 *	load_driver_history
 *	Load history text for the specified driver into the specified buffer.
 *	Combines $bio field of HISTORY.DAT with $mame field of MAMEINFO.DAT.
 *
 *	Returns 0 if successful.
 *
 *	NOTE: For efficiency the indices are never freed (intentional leak).
 **************************************************************************/
int load_driver_history (const game_driver *drv, char *buffer, int bufsize)
{
	int result = 1;

	*buffer = 0;

	flush_index_if_needed();

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, 0, hist_idx,
	                         "history/", "history.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, FILE_ROOT, hist_idx,
	                         "history/", MameUISettings().value(MUIOPTION_HISTORY_FILE));

	return result;
}

#ifdef STORY_DATAFILE
int load_driver_story (const game_driver *drv, char *buffer, int bufsize)
{
	int result = 1;

	*buffer = 0;

	{
		int skip_pos = strlen(buffer);
		int check_pos;

		strcpy(buffer + skip_pos, _("\nStory:\n"));
		check_pos = strlen(buffer);

		result &= load_datafile (drv, buffer, bufsize,
		                         DATAFILE_TAG_STORY, 0, story_idx,
		                         "story/", "story.dat");
		result &= load_datafile (drv, buffer, bufsize,
		                         DATAFILE_TAG_STORY, FILE_ROOT, story_idx,
		                         "story/", MameUISettings().value(MUIOPTION_STORY_FILE));

		if (buffer[check_pos] == '\0')
			buffer[skip_pos] = '\0';
	}

	return result;
}
#endif /* STORY_DATAFILE */

int load_driver_mameinfo (const game_driver *drv, char *buffer, int bufsize)
{
	const game_driver *clone_of;
	int result = 1;
	int i;

	*buffer = 0;

	strcat(buffer, _("Mameinfo:\n"));

	/* List the game info 'flags' */
	if (drv->flags &
	    ( GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS |
	      GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL | GAME_REQUIRES_ARTWORK))
	{
		strcat(buffer, _("GAME: "));
		strcat(buffer, _LST(drv->description));
		strcat(buffer, "\n");

		if (drv->flags & GAME_NOT_WORKING)
		strcat(buffer, _("THIS GAME DOESN'T WORK. The emulation for this game is not yet complete. "
						"There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n"));

		if (drv->flags & GAME_UNEMULATED_PROTECTION)
			strcat(buffer, _("The game has protection which isn't fully emulated.\n"));

		if (drv->flags & GAME_IMPERFECT_GRAPHICS)
			strcat(buffer, _("The video emulation isn't 100% accurate.\n"));

		if (drv->flags & GAME_WRONG_COLORS)
			strcat(buffer, _("The colors are completely wrong.\n"));

		if (drv->flags & GAME_IMPERFECT_COLORS)
			strcat(buffer, _("The colors aren't 100% accurate.\n"));

		if (drv->flags & GAME_NO_SOUND)
			strcat(buffer, _("The game lacks sound.\n"));

		if (drv->flags & GAME_IMPERFECT_SOUND)
			strcat(buffer, _("The sound emulation isn't 100% accurate.\n"));

		if (drv->flags & GAME_NO_COCKTAIL)
			strcat(buffer, _("Screen flipping in cocktail mode is not supported.\n"));

		if (drv->flags & GAME_REQUIRES_ARTWORK)
			strcat(buffer, _("The game requires external artwork files\n"));

		strcat(buffer, "\n");
	}

	flush_index_if_needed();

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, 0, mame_idx,
	                         "mameinfo/", "mameinfo.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, FILE_ROOT, mame_idx,
	                         "mameinfo/", MameUISettings().value(MUIOPTION_MAMEINFO_FILE));

	int cl = driver_list::clone(*drv);
	if (cl != -1) clone_of = &driver_list::driver(cl); else clone_of = NULL;
	if (clone_of && !(clone_of->flags & GAME_IS_BIOS_ROOT))
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, _LST(clone_of->description));
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; i < driver_list::total(); i++)
		{
			if (!core_stricmp (drv->parent, driver_list::driver(i).parent))
			{
				strcat(buffer, _LST(driver_list::driver(i).description));
				strcat(buffer, "\n");
			}
		}
	}
	else
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, _LST(drv->description));
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; i < driver_list::total(); i++)
		{
			if (!core_stricmp (drv->name, driver_list::driver(i).parent))
			{
				strcat(buffer, _LST(driver_list::driver(i).description));
				strcat(buffer, "\n");
			}
		}
	}

	return 0;
}

int load_driver_drivinfo (const game_driver *drv, char *buffer, int bufsize)
{
	int drivinfo = 0;
	// int i;

	*buffer = 0;

	/* Print source code file */
	sprintf (buffer, _("\nDRIVER: %s\n"), drv->source_file+17);

	/* Try to open mameinfo datafile - driver section */
	if (ParseOpen (MameUISettings().value(MUIOPTION_MAMEINFO_FILE)))
	{
		int err;

		/* create index if necessary */
		if (driv_idx)
			drivinfo = 1;
		else
			drivinfo = (index_datafile_drivinfo (&driv_idx) != 0);

		/* load informational text (append) */
		if (driv_idx)
		{
			int len = strlen (buffer);

			err = load_drivfile_text (drv, buffer+len, bufsize-len,
			                          driv_idx, DATAFILE_TAG_DRIV);

			if (err) drivinfo = 0;
		}
		ParseClose ();
	}

	return (drivinfo == 0);
}
