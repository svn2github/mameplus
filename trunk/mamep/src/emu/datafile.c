/****************************************************************************
 *      datafile.c
 *      History database engine
 *
 *      Token parsing by Neil Bradley
 *      Modifications and higher-level functions by John Butler
 ****************************************************************************/

/****************************************************************************
 *      include files
 ****************************************************************************/
#include <assert.h>
#include <ctype.h>
#include "driver.h"
#include "osd_cpu.h"
#include "timer.h"
#include "datafile.h"
#include "hash.h"


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

#define MAX_TOKEN_LENGTH	1024


/****************************************************************************
 *      datafile constants
 ****************************************************************************/
#define MAX_MENUIDX_ENTRIES 64
#define DATAFILE_TAG '$'

const char *DATAFILE_TAG_KEY = "$info";
const char *DATAFILE_TAG_BIO = "$bio";
#ifdef STORY_DATAFILE
const char *DATAFILE_TAG_STORY = "$story";
#endif /* STORY_DATAFILE */
const char *DATAFILE_TAG_MAME = "$mame";
const char *DATAFILE_TAG_DRIV = "$drv";

#ifdef CMD_LIST
const char *DATAFILE_TAG_COMMAND = "$cmd";
const char *DATAFILE_TAG_END = "$end";
#endif /* CMD_LIST */

#define FILE_MERGED	1
#define FILE_ROOT	2
#define FILE_TYPEMAX	((FILE_MERGED | FILE_ROOT) + 1)

static core_options *datafile_options;

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

#ifdef CMD_LIST
struct tMenuIndex
{
	UINT64 offset;
	char *menuitem;
};

static struct tDatafileIndex *cmnd_idx[FILE_TYPEMAX];
static struct tMenuIndex *menu_idx;
static char *menu_filename;

static int mame32jp_wrap;
#endif /* CMD_LIST */

/****************************************************************************
 *      private data for parsing functions
 ****************************************************************************/
static mame_file *fp;				/* Our file pointer */
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


/****************************************************************************
 *      startup and shutdown functions
 ****************************************************************************/

void datafile_init(core_options *options)
{
	datafile_options = options;

	while (drivers[num_games] != NULL)
		num_games++;
}

void datafile_exit(void)
{
	flush_index();

	if (sorted_drivers == NULL)
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
			sorted_drivers[i].name = drivers[i]->name;
			sorted_drivers[i].index = i;
		}
		qsort(sorted_drivers, num_games, sizeof(driver_data_type), DriverDataCompareFunc);
	}

	/* uses our sorted array of driver names to get the index in log time */
	driver_index_info = bsearch(&key, sorted_drivers,num_games, sizeof(driver_data_type),
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
	unsigned char *s = mame_strdup(srcdriver);
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
			sorted_srcdrivers[i].srcdriver = drivers[i]->source_file+17;
			sorted_srcdrivers[i].index = i;
		}
		qsort(sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),SrcDriverDataCompareFunc);
	}

	key.srcdriver = s;
	srcdriver_index_info = bsearch(&key, sorted_srcdrivers, num_games, sizeof(srcdriver_data_type),
	                               SrcDriverDataCompareFunc);
	free(s);

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
		dwFilePos = mame_ftell(fp);
		bData = mame_fgetc(fp);		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (mame_feof(fp))
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
				dwFilePos = mame_ftell(fp);

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
				       mame_feof(fp) == 0)
				{
					dwFilePos = mame_ftell(fp);
					*pbTokenPtr++ = bData;	/* Store our byte */
					++dwLength;
					assert(dwLength < MAX_TOKEN_LENGTH);
					bData = mame_fgetc(fp);
				}

				/* If it's not the end of the file, put the last received byte */
				/* back. We don't want to touch the file position, though if */
				/* we're past the end of the file. Otherwise, adjust it. */

				if (0 == mame_feof(fp))
				{
					mame_fseek(fp, dwFilePos, SEEK_SET);
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

				dwFilePos = mame_ftell(fp);
				bData = mame_fgetc(fp);		/* Peek ahead */
				dwPos = mame_ftell(fp);
				mame_fseek(fp, dwFilePos, SEEK_SET);	/* Force a retrigger if subsequent LF's */

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

				dwFilePos = mame_ftell(fp);
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_fseek(fp, dwFilePos, SEEK_SET);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
#ifdef CMD_LIST
					if (mame32jp_wrap)
					{
						dwFilePos = mame_ftell(fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
#endif /* CMD_LIST */
						UINT64 dwPos;

						dwFilePos = mame_ftell(fp);	/* Our file position to reset to */
						dwPos = dwFilePos;	/* Here so we can reposition things */

						/* Look for a followup CR/LF */

						bData = mame_fgetc(fp);	/* Get the next byte */

						if (CR == bData)	/* CR! Good! */
						{
							bData = mame_fgetc(fp);	/* Get the next byte */

							/* We need to do this to pick up subsequent CR/LF sequences */

							mame_fseek(fp, dwPos, SEEK_SET);

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
							mame_fseek(fp, dwFilePos, SEEK_SET);	/* Put the character back. No good */
						}
					}
#ifdef CMD_LIST
				}
#endif /* CMD_LIST */
				else
				{
					mame_fseek(fp, dwFilePos, SEEK_SET);	/* Put the character back. No good */
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
		dwFilePos = mame_ftell(fp);
		bData = mame_fgetc(fp);		/* Get next character */

		/* If we're at the end of the file, bail out */

		if (mame_feof(fp))
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
				dwFilePos = mame_ftell(fp);

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
				       mame_feof(fp) == 0)
				{
					dwFilePos = mame_ftell(fp);
					*pbTokenPtr++ = bData;	/* Store our byte */
					++dwLength;
					assert(dwLength < MAX_TOKEN_LENGTH);
					bData = mame_fgetc(fp);
				}

				/* If it's not the end of the file, put the last received byte */
				/* back. We don't want to touch the file position, though if */
				/* we're past the end of the file. Otherwise, adjust it. */

				if (0 == mame_feof(fp))
				{
					mame_fseek(fp, dwFilePos, SEEK_SET);
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

				dwFilePos = mame_ftell(fp);
				bData = mame_fgetc(fp);		/* Peek ahead */
				dwPos = mame_ftell(fp);
				mame_fseek(fp, dwFilePos, SEEK_SET);	/* Force a retrigger if subsequent LF's */

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

				dwFilePos = mame_ftell(fp);
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_fseek(fp, dwFilePos, SEEK_SET);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
					if (mame32jp_wrap)
					{
						dwFilePos = mame_ftell(fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
						UINT64 dwPos;

						dwFilePos = mame_ftell(fp);	/* Our file position to reset to */
						dwPos = dwFilePos;	/* Here so we can reposition things */

						/* Look for a followup CR/LF */

						bData = mame_fgetc(fp);	/* Get the next byte */

						if (CR == bData)	/* CR! Good! */
						{
							bData = mame_fgetc(fp);	/* Get the next byte */

							/* We need to do this to pick up subsequent CR/LF sequences */

							mame_fseek(fp, dwPos, SEEK_SET);

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
							mame_fseek(fp, dwFilePos, SEEK_SET);	/* Put the character back. No good */
						}
					}
				}
				else
				{
					mame_fseek(fp, dwFilePos, SEEK_SET);	/* Put the character back. No good */
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
		mame_fclose(fp);

	fp = NULL;
}


/****************************************************************************
 *      ParseOpen - Open up file for reading
 ****************************************************************************/
static UINT8 ParseOpen(const char *pszFilename)
{
	file_error filerr;

	/* Open file up in binary mode */
	filerr = mame_fopen_options(datafile_options, SEARCHPATH_DATAFILE, pszFilename, OPEN_FLAG_READ, &fp);
	if (filerr != FILERR_NONE)
		return(FALSE);

	/* Otherwise, prepare! */

	dwFilePos = 0;

	/* identify text file type first */
	mame_fgetc(fp);
	mame_fseek(fp, dwFilePos, SEEK_SET);

	return(TRUE);
}


/****************************************************************************
 *      ParseSeek - Move the file position indicator
 ****************************************************************************/
static UINT8 ParseSeek(UINT64 offset, int whence)
{
	int result = mame_fseek(fp, offset, whence);

	if (0 == result)
	{
		dwFilePos = mame_ftell(fp);
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
        idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
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
						idx->driver = drivers[game_index];
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
		gdrv = driver_get_clone(gdrv);
	} while (!gd_idx->driver && gdrv);

	if (gdrv == 0) return 0;	/* driver not found in Data_file_index */

	/* seek to correct point in datafile */
	if (ParseSeek (gd_idx->offset, SEEK_SET)) return 0;

	/* allocate index */
	m_idx = *_index = malloc(MAX_MENUIDX_ENTRIES * sizeof (struct tMenuIndex));
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

			m_idx->menuitem = malloc(strlen((char *)s)+1);
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
			free(m_idx->menuitem);
			m_idx++;
		}

		free(*_index);
		*_index = NULL;
	}
}
#endif /* CMD_LIST */

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

#ifdef CMD_LIST
		if (cmnd_idx[i])
		{
			free_menuidx(&menu_idx);
			free(cmnd_idx[i]);
			cmnd_idx[i] = 0;
		}
#endif /* CMD_LIST */
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
	idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
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
					int src_index;
					src_index = GetSrcDriverIndex(s);
					if (src_index >= 0)
					{
						idx->driver = drivers[src_index];
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
#ifdef CMD_LIST
	int first = 1;
#endif /* CMD_LIST */

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
#ifdef CMD_LIST
			if (!mame32jp_wrap && TOKEN_LINEBREAK != token)
#else /* CMD_LIST */
			if (TOKEN_LINEBREAK != token)
#endif /* CMD_LIST */
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
				if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
				{
					found = 1;
#ifdef CMD_LIST
					if (first && mame32jp_wrap)
					{
						mame_fseek(fp, 2l, SEEK_CUR);
						first = 0;
					}
#endif /* CMD_LIST */
				}
				else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
					break;	/* error: tag missing */
			}
		}
	}
	return (!found);
}

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
				if (!mame_strnicmp (tag, (char *)s, strlen (tag)))
					found = 1;
				else if (!mame_strnicmp (DATAFILE_TAG_KEY, (char *)s, strlen (DATAFILE_TAG_KEY)))
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

	filename[0] = '\0';

	if (where != FILE_ROOT)
	{
		sprintf(filename, "%s\\%s\\",
	        	options_get_string(datafile_options, OPTION_LOCALIZED_DIRECTORY),
			ui_lang_info[lang_get_langcode()].name);
	}

	base = filename + strlen(filename);

	for (gdrv = drv; gdrv && gdrv->name[0]; gdrv = driver_get_clone(gdrv))
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

					pdrv = driver_get_clone(pdrv);
				} while (err && pdrv);

				if (err) status = 0;
			}

			ParseClose ();

			if (status)
				return 0;
		}
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

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

	flush_index_if_needed();

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, 0, hist_idx,
	                         "history/", "history.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_BIO, FILE_ROOT, hist_idx,
	                         "history/", options_get_string(datafile_options, OPTION_HISTORY_FILE));

	return result;
}

#ifdef STORY_DATAFILE
int load_driver_story (const game_driver *drv, char *buffer, int bufsize)
{
	int result = 1;

	*buffer = 0;

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

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
		                         "story/", options_get_string(datafile_options, OPTION_STORY_FILE));

		if (buffer[check_pos] == '\0')
			buffer[skip_pos] = '\0';
	}

	return result;
}
#endif /* STORY_DATAFILE */

int load_driver_mameinfo (const game_driver *drv, char *buffer, int bufsize)
{
	const rom_entry *region, *rom, *chunk;
	const game_driver *clone_of;
	int result = 1;
	int i;

	*buffer = 0;

	strcat(buffer, "MAMEInfo:\n");

	/* List the game info 'flags' */
	if (drv->flags &
	    ( GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS |
	      GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL))
	{
		strcat(buffer, _("GAME: "));
		strcat(buffer, _LST(drv->description));
		strcat(buffer, "\n");

		if (drv->flags & GAME_NOT_WORKING)
			strcat(buffer, _("THIS GAME DOESN'T WORK. You won't be able to make it work correctly.  Don't bother.\n"));

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

		strcat(buffer, "\n");
	}	

#ifdef CMD_LIST
	mame32jp_wrap = 0;
#endif /* CMD_LIST */

	flush_index_if_needed();

	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, 0, mame_idx,
	                         "mameinfo/", "mameinfo.dat");
	result &= load_datafile (drv, buffer, bufsize,
	                         DATAFILE_TAG_MAME, FILE_ROOT, mame_idx,
	                         "mameinfo/", options_get_string(datafile_options, OPTION_MAMEINFO_FILE));

	strcat(buffer, _("\nROM REGION:\n"));
	for (region = rom_first_region(drv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			char name[100];
			int length;

			length = 0;
			for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
				length += ROM_GETLENGTH(chunk);

			//sprintf(name," %-12s ",ROM_GETNAME(rom));
			//strcat(buffer, name);
			//sprintf(name,"%6x ",length);
			sprintf(name," %s %08x ",ROM_GETNAME(rom),length);
			strcat(buffer, name);
			strcat(buffer, ROMREGION_GETTAG(region));

			//sprintf(name," %7x\n",ROM_GETOFFSET(rom));
			sprintf(name," %08x\n",ROM_GETOFFSET(rom));
			strcat(buffer, name);
		}

	clone_of = driver_get_clone(drv);
	if (clone_of && !(clone_of->flags & GAME_IS_BIOS_ROOT))
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, _LST(clone_of->description));
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->parent, drivers[i]->parent)) 
			{
				strcat(buffer, _LST(drivers[i]->description));
				strcat(buffer, "\n");
			}
		}
	}
	else
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, _LST(drv->description));
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->name, drivers[i]->parent)) 
			{
				strcat(buffer, _LST(drivers[i]->description));
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
	sprintf (buffer, _("\nSOURCE: %s\n"), drv->source_file+17);

	/* Try to open mameinfo datafile - driver section */
	if (ParseOpen (options_get_string(datafile_options, OPTION_MAMEINFO_FILE)))
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



#ifdef CMD_LIST
/**************************************************************************
 *	find_command
 **************************************************************************/
static int find_command (const game_driver *drv)
{
	int where;
	int i;

	flush_index_if_needed();

	if (menu_filename)
		free(menu_filename);

	for (where = 0; where <= FILE_ROOT; where += FILE_ROOT)
	{
		char filename[80];
		char *base;

		filename[0] = '\0';

		if (where != FILE_ROOT)
		{
			sprintf(filename, "%s\\%s\\",
		        	options_get_string(datafile_options, OPTION_LOCALIZED_DIRECTORY),
				ui_lang_info[lang_get_langcode()].name);
		}

		base = filename + strlen(filename);

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;

			if (i & FILE_MERGED)
			{
				if (where & FILE_ROOT)
					strcpy(base, options_get_string(datafile_options, OPTION_COMMAND_FILE));
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

				for (gdrv = drv; !status && gdrv && gdrv->name[0]; gdrv = driver_get_clone(gdrv))
				{
					strcpy(base, "command\\");
					strcat(base, gdrv->name);
					strcat(base, ".dat");

					/* try to open command datafile */
					if (!ParseOpen (filename))
						continue;

					if (cmnd_idx[i])
					{
						free(cmnd_idx[i]);
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
