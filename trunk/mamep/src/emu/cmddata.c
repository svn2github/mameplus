/***************************************************************************

    cmddata.c

    Command List datafile engine

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

****************************************************************************

    Token parsing by Neil Bradley
    Modifications and higher-level functions by John Butler

***************************************************************************/

#include <assert.h>
#include <ctype.h>
#include "emu.h"
#include "emuopts.h"
#include "cmddata.h"


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

#define MAX_TOKEN_LENGTH	4096


/****************************************************************************
 *      datafile constants
 ****************************************************************************/
#define MAX_MENUIDX_ENTRIES 64
#define DATAFILE_TAG '$'

static const char *DATAFILE_TAG_KEY = "$info";
#ifdef CMD_LIST
static const char *DATAFILE_TAG_COMMAND = "$cmd";
static const char *DATAFILE_TAG_END = "$end";
#endif /* CMD_LIST */

#define FILE_MERGED	1
#define FILE_ROOT	2
#define FILE_TYPEMAX	((FILE_MERGED | FILE_ROOT) + 1)

static running_machine *m_machine = NULL;
static emu_options *datafile_options = NULL;

struct tDatafileIndex
{
	UINT64 offset;
	const game_driver *driver;
};

#ifdef CMD_LIST
struct tMenuIndex
{
	UINT64 offset;
	char *menuitem;
};

static struct tDatafileIndex *cmnd_idx[FILE_TYPEMAX];
static struct tMenuIndex *menu_idx = NULL;
static char *menu_filename = NULL;

static int mame32jp_wrap;
#endif /* CMD_LIST */


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
static driver_data_type *sorted_drivers = NULL;
static int num_games;
static void flush_index(void);
static void ParseClose(void);


/****************************************************************************
 *      startup and shutdown functions
 ****************************************************************************/

static void datafile_exit(running_machine &machine)
{
	ParseClose();

	flush_index();

	if (sorted_drivers == NULL)
	{
		auto_free(*m_machine, sorted_drivers);
		sorted_drivers = NULL;
	}
}

void datafile_init(running_machine &machine, emu_options *options)
{
	m_machine = &machine;
	datafile_options = options;

	num_games = driver_list::total();

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(datafile_exit), &machine));
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

		sorted_drivers = auto_alloc_array(*m_machine, driver_data_type, sizeof(driver_data_type) * num_games);
		for (i = 0; i < num_games; i++)
		{
			sorted_drivers[i].name = driver_list::driver(i).name;
			sorted_drivers[i].index = i;
		}
		qsort(sorted_drivers, num_games, sizeof(driver_data_type), DriverDataCompareFunc);
	}

	/* uses our sorted array of driver names to get the index in log time */
	driver_index_info = (driver_data_type *)bsearch(&key, sorted_drivers, num_games, sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

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
#ifdef CMD_LIST
	UINT8 space, character;

	if (mame32jp_wrap)
	{
		space     = '\t';
		character = ' ' - 1;
	}
	else
	{
		space     = ' ';
		character = ' ';
	}
#endif /* CMD_LIST */

	while (1)
	{
		dwFilePos = fp->tell();
		bData = fp->getc();		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (fp->eof())
			return(TOKEN_INVALID);

		/* If it's not whitespace, then let's start eating characters */

#ifdef CMD_LIST
		if (space != bData && '\t' != bData)
#else /* CMD_LIST */
		if (' ' != bData && '\t' != bData)
#endif /* CMD_LIST */
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

#ifdef CMD_LIST
			if (bData > character)
#else /* CMD_LIST */
			if (bData > ' ')
#endif /* CMD_LIST */
			{
				dwLength = 0;			/* Assume we're 0 length to start with */

				/* Loop until we've hit something we don't understand */

				while (bData != ',' &&
				       bData != '=' &&
#ifdef CMD_LIST
				       bData != space &&
#else /* CMD_LIST */
				       bData != ' ' &&
#endif /* CMD_LIST */
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
				else if (LF == bData)		/* MSDOS format! */
				{
#ifdef CMD_LIST
					if (mame32jp_wrap)
					{
						dwFilePos = fp->tell();

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
#endif /* CMD_LIST */
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
#ifdef CMD_LIST
				}
#endif /* CMD_LIST */
				else
				{
					fp->seek(dwFilePos, SEEK_SET);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}
	}
}


#ifdef CMD_LIST
/****************************************************************************
 *      GetNextToken_ex - Pointer to the token string pointer
 *                        Pointer to position within file
 *
 *      Returns token, or TOKEN_INVALID if at end of file
 ****************************************************************************/
static UINT32 GetNextToken_ex(UINT8 **ppszTokenText, UINT64 *pdwPosition)
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

		if ('\t' != bData)
		{
			/* Store away our file position (if given on input) */

			if (pdwPosition)
				*pdwPosition = dwFilePos;

			/* Fixed: exclude a separator of comma */
			/* If it's a separator, special case it */
//			if (',' == bData || '=' == bData)
			if ('=' == bData)
			{
				*pbTokenPtr++ = bData;
				*pbTokenPtr = '\0';
				dwFilePos = fp->tell();

				return(TOKEN_EQUALS);
			}

			/* Otherwise, let's try for a symbol */
			if (bData >= ' ')
			{
				dwLength = 0;			/* Assume we're 0 length to start with */

				/* Loop until we've hit something we don't understand */

				/* Fixed: exclude a separator of comma and equal */
				while (
//				      (bData != ',' &&
//				       bData != '=' &&
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
			else if (CR == bData)			/* Carriage return? */
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
				else if (LF == bData)		/* MSDOS format! */
				{
					if (mame32jp_wrap)
					{
						dwFilePos = fp->tell();

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
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
#endif /* CMD_LIST */


/****************************************************************************
 *      ParseClose - Closes the existing opened file (if any)
 ****************************************************************************/
static void ParseClose(void)
{
	/* If the file is open, time for fclose. */

	if (fp)
	{
		fp->close();
		auto_free(*m_machine, fp);
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
	fp = auto_alloc(*m_machine, emu_file(OPEN_FLAG_READ));
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
	idx = *_index = auto_alloc_array(*m_machine, tDatafileIndex, (num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
	while (count < num_games && TOKEN_INVALID != token)
	{
		UINT64 tell;
		UINT8 *s;

		token = GetNextToken (&s, &tell);
		if (TOKEN_SYMBOL != token) continue;

		/* DATAFILE_TAG_KEY identifies the driver */
		if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
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

#ifdef CMD_LIST
		if (cmnd_idx[i])
		{
			auto_free(*m_machine, cmnd_idx[i]);
			cmnd_idx[i] = 0;
		}
#endif /* CMD_LIST */
	}
}


#ifdef CMD_LIST
/**************************************************************************
 *	index_menuidx
 *	
 *
 *	Returns 0 on error, or the number of index entries created.
 **************************************************************************/
static int index_menuidx (const game_driver *drv, struct tDatafileIndex *d_idx, struct tMenuIndex **_index)
{
	struct tMenuIndex *m_idx;
	const game_driver *gdrv;
	struct tDatafileIndex *gd_idx;
	int cl;
	int m_count = 0;
	UINT32 token = TOKEN_SYMBOL;

	UINT64 tell;
	UINT64 cmdtag_offset = 0;
	UINT8 *s;

	mame32jp_wrap = 1;

	gdrv = drv;
	do
	{
		gd_idx = d_idx;

		/* find driver in datafile index */
		while (gd_idx->driver)
		{
			if (gd_idx->driver == gdrv) break;
			gd_idx++;
		}

		if (gd_idx->driver == gdrv) break;
		cl = driver_list::clone(*gdrv);
		if (cl == -1) break;
		gdrv = &driver_list::driver(cl);
	} while (!gd_idx->driver && gdrv);

	if (gdrv == 0) return 0;	/* driver not found in Data_file_index */

	/* seek to correct point in datafile */
	if (ParseSeek (gd_idx->offset, SEEK_SET)) return 0;

	/* allocate index */
	m_idx = *_index = auto_alloc_array(*m_machine, tMenuIndex, MAX_MENUIDX_ENTRIES * sizeof (struct tMenuIndex));
	if (NULL == m_idx) return 0;

	/* loop through between $cmd= */
	token = GetNextToken (&s, &tell);
	while ((m_count < (MAX_MENUIDX_ENTRIES - 1)) && TOKEN_INVALID != token)
	{
		if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
			break;

		/* DATAFILE_TAG_COMMAND identifies the driver */
		if (!mame_strnicmp (DATAFILE_TAG_COMMAND, (char *)s, strlen (DATAFILE_TAG_COMMAND)))
		{
			cmdtag_offset = tell;
			token = GetNextToken_ex (&s, &tell);

			if (token == TOKEN_EQUALS)
				token = GetNextToken_ex (&s, &tell);
			else
			{
				while (TOKEN_SYMBOL != token)
					token = GetNextToken_ex (&s, &tell);
			}

			m_idx->menuitem = auto_alloc_array(*m_machine, char, strlen((char *)s)+1);
			strcpy(m_idx->menuitem, (char *)s);

			m_idx->offset = cmdtag_offset;

			m_idx++;
			m_count++;
		}
		token = GetNextToken (&s, &tell);
	}

	/* mark end of index */
	m_idx->offset = 0L;
	m_idx->menuitem = NULL;

	return m_count;
}

static void free_menuidx(struct tMenuIndex **_index)
{
	if (*_index)
	{
		struct tMenuIndex *m_idx = *_index;

		while(m_idx->menuitem != NULL)
		{
			auto_free(*m_machine, m_idx->menuitem);
			m_idx++;
		}

		auto_free(*m_machine, *_index);
		*_index = NULL;
	}
}
#endif /* CMD_LIST */


#ifdef CMD_LIST
/**************************************************************************
 *	load_datafile_text_ex
 *
 *	Loads text field for a driver into the buffer specified. Specify the
 *	driver, a pointer to the buffer, the buffer size, the index created by
 *	index_datafile(), and the desired text field (e.g., DATAFILE_TAG_BIO).
 *
 *	Returns 0 if successful.
 **************************************************************************/
static int load_datafile_text_ex (char *buffer, int bufsize,
	const char *tag, struct tMenuIndex *m_idx, const int menu_sel)
{
	int offset = 0;
	UINT32	token = TOKEN_SYMBOL;
	UINT32 	prev_token = TOKEN_SYMBOL;
	UINT8 *s = NULL;
	int len;
	UINT64 tell;

	*buffer = '\0';

	/* seek to correct point in datafile */
	if (ParseSeek ((m_idx + menu_sel)->offset, SEEK_SET)) return 1;

	/* read text until tag is found */
	while (TOKEN_INVALID != token)
	{
		token = GetNextToken (&s, &tell);

		if (TOKEN_INVALID == token)
			break;

		if (TOKEN_SYMBOL == token)
		{
			/* looking for requested tag */
			if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
			{
				token = GetNextToken_ex (&s, &tell);

				if (TOKEN_EQUALS == token)
					token = GetNextToken_ex (&s, &tell);
				else
				{
					while (TOKEN_SYMBOL != token)
						token = GetNextToken_ex (&s, &tell);
				}

				break;
			}
			else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
			{
				token = TOKEN_INVALID;
				break;	/* error: tag missing */
			}
		}
	}

	/* read text until buffer is full or end of entry is encountered */
	while (TOKEN_INVALID != token)
	{
		/* end entry when a tag is encountered */
		if (TOKEN_SYMBOL == token && !mame_strnicmp (DATAFILE_TAG_END, (char *)s, strlen (DATAFILE_TAG_END)))
			break;

		prev_token = token;

		/* translate platform-specific linebreaks to '\n' */
		if (TOKEN_LINEBREAK == token)
			strcpy ((char *)s, "\n");

		/* add this word to the buffer */
		len = strlen ((char *)s);
		if ((len + offset) >= bufsize) break;
		strcpy (buffer, (char *)s);
		buffer += len;
		offset += len;

		token = GetNextToken_ex (&s, &tell);
	}

	return TOKEN_INVALID == token;
}
#endif /* CMD_LIST */


#ifdef CMD_LIST
/**************************************************************************
 *	find_command
 **************************************************************************/
static int find_command (const game_driver *drv)
{
	int where;
	int i;

	if (menu_filename)
		osd_free(menu_filename);

	for (where = 0; where <= FILE_ROOT; where += FILE_ROOT)
	{
		char filename[80];
		char *base;

		filename[0] = '\0';

		if (where != FILE_ROOT)
		{
			sprintf(filename, "%s\\%s\\",
		        	datafile_options->value(OPTION_LANGPATH),
				ui_lang_info[lang_get_langcode()].name);
		}

		base = filename + strlen(filename);

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;

			if (i & FILE_MERGED)
			{
				if (where & FILE_ROOT)
					strcpy(base, datafile_options->value(OPTION_COMMAND_FILE));
				else
					strcpy(base, "command.dat");

				/* try to open command datafile */
				if (!ParseOpen (filename))
					continue;

				/* create index if necessary */
				if (cmnd_idx[i])
					status = 1;
				else
				{
					status = (index_datafile (&cmnd_idx[i]) != 0);
					free_menuidx(&menu_idx);
				}

				/* create menu_index */
				status = (index_menuidx (drv, cmnd_idx[i], &menu_idx) != 0);

				if (!status)
					free_menuidx(&menu_idx);

				ParseClose ();
			}
			else
			{
				const game_driver *gdrv;
				int cl;

				for (gdrv = drv; !status && gdrv && gdrv->name[0] && ((cl = driver_list::clone(*gdrv)) != -1); gdrv = &driver_list::driver(cl))
				{
					strcpy(base, "command\\");
					strcat(base, gdrv->name);
					strcat(base, ".dat");

					/* try to open command datafile */
					if (!ParseOpen (filename))
						continue;

					if (cmnd_idx[i])
					{
						auto_free(*m_machine, cmnd_idx[i]);
						cmnd_idx[i] = 0;
					}

					status = (index_datafile (&cmnd_idx[i]) != 0);
					free_menuidx(&menu_idx);

					/* create menu_index */
					status = (index_menuidx (drv, cmnd_idx[i], &menu_idx) != 0);

					if (!status)
						free_menuidx(&menu_idx);

					ParseClose ();
				}
			}

			if (status)
			{
				menu_filename = mame_strdup(filename);

				return 0;
			}
		}
	}

	return 1;
}


/**************************************************************************
 *	load_driver_command_ex
 **************************************************************************/
int load_driver_command_ex (const game_driver *drv, char *buffer, int bufsize, const int menu_sel)
{
	*buffer = 0;

	//if (find_command (drv))
	//	return 1;

	if (!menu_filename)
		return 1;

	/* try to open command datafile */
	if (ParseOpen (menu_filename))
	{
		int err;

		err = load_datafile_text_ex (buffer, bufsize,
		                             DATAFILE_TAG_COMMAND, menu_idx, menu_sel);

		ParseClose ();

		if (!err)
			return 0;
	}

	return 1;
}


/**************************************************************************
 *	command_sub_menu
 **************************************************************************/
UINT8 command_sub_menu(const game_driver *drv, const char *menu_item[])
{
	if (find_command (drv))
		return 0;

	if (menu_idx)
	{
		struct tMenuIndex *m_idx = menu_idx;
		int total = 0;

		while(m_idx->menuitem != NULL)
		{
			menu_item[total++] = m_idx->menuitem;
			m_idx++;
		}

		return total;
	}

	return 0;
}
#endif /* CMD_LIST */
