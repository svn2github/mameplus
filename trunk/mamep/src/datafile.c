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
#include "osd_cpu.h"
#include "driver.h"
#include "datafile.h"
#include "hash.h"
#include "statistics.h"


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

struct tDatafileIndex
{
	long offset;
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
	long offset;
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
static long dwFilePos;				/* file position */
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
		for (i=0;i<num_games;i++)
		{
			sorted_drivers[i].name = drivers[i]->name;
			sorted_drivers[i].index = i;
		}
		qsort(sorted_drivers,num_games,sizeof(driver_data_type),DriverDataCompareFunc);
	}

	/* uses our sorted array of driver names to get the index in log time */
	driver_index_info = bsearch(&key,sorted_drivers,num_games,sizeof(driver_data_type),
	                            DriverDataCompareFunc);

	if (driver_index_info == NULL)
		return -1;

	return driver_index_info->index;

}


/****************************************************************************
 *      Create an array with sorted sourcedrivers for the function
 *      index_datafile_drivinfo to speed up the datafile access
 ****************************************************************************/

typedef struct
{
	const char *srcdriver;
	int index;
} srcdriver_data_type;
static srcdriver_data_type *sorted_srcdrivers = NULL;
static int num_games;


static int SrcDriverDataCompareFunc(const void *arg1,const void *arg2)
{
	return strcmp( ((srcdriver_data_type *)arg1)->srcdriver, ((srcdriver_data_type *)arg2)->srcdriver );
}


static int GetSrcDriverIndex(const char *srcdriver)
{
	srcdriver_data_type *srcdriver_index_info;
	srcdriver_data_type key;
	key.srcdriver = srcdriver;

	if (sorted_srcdrivers == NULL)
	{
		/* initialize array of game names/indices */
		int i;
		num_games = 0;
		while (drivers[num_games] != NULL)
			num_games++;

		sorted_srcdrivers = (srcdriver_data_type *)malloc(sizeof(srcdriver_data_type) * num_games);
		for (i=0;i<num_games;i++)
		{
			sorted_srcdrivers[i].srcdriver = drivers[i]->source_file+12;
			sorted_srcdrivers[i].index = i;
		}
		qsort(sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),SrcDriverDataCompareFunc);
	}

	srcdriver_index_info = bsearch(&key,sorted_srcdrivers,num_games,sizeof(srcdriver_data_type),
	                               SrcDriverDataCompareFunc);

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
static UINT32 GetNextToken(UINT8 **ppszTokenText, long *pdwPosition)
{
	UINT32 dwLength;			/* Length of symbol */
	long dwPos;				/* Temporary position */
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
				++dwFilePos;

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
					++dwFilePos;
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
					mame_ungetc(bData, fp);
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
				/* Unix style perhaps? */

				bData = mame_fgetc(fp);		/* Peek ahead */
				mame_ungetc(bData, fp);		/* Force a retrigger if subsequent LF's */

				if (LF == bData)		/* Two LF's in a row - it's a UNIX hard CR */
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* A real linefeed */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}

				/* Otherwise, fall through and keep parsing. */

			}
			else if (CR == bData)			/* Carriage return? */
			{
				/* Figure out if it's Mac or MSDOS format */

				++dwFilePos;
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_ungetc(bData, fp);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
#ifdef CMD_LIST
					if (mame32jp_wrap)
					{
						mame_ungetc(bData, fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
#endif /* CMD_LIST */
						++dwFilePos;		/* Our file position to reset to */
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
					 	       --dwFilePos;
							mame_ungetc(bData, fp);	/* Put the character back. No good */
						}
					}
#ifdef CMD_LIST
				}
#endif /* CMD_LIST */
				else
				{
					--dwFilePos;
					mame_ungetc(bData, fp);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}

		++dwFilePos;
	}
}


#ifdef CMD_LIST
/****************************************************************************
 *      GetNextToken_ex - Pointer to the token string pointer
 *                        Pointer to position within file
 *
 *      Returns token, or TOKEN_INVALID if at end of file
 ****************************************************************************/
static UINT32 GetNextToken_ex(UINT8 **ppszTokenText, long *pdwPosition)
{
	UINT32 dwLength;			/* Length of symbol */
	long dwPos;				/* Temporary position */
	UINT8 *pbTokenPtr = bToken;		/* Point to the beginning */
	UINT8 bData;				/* Temporary data byte */

	while (1)
	{
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
				++dwFilePos;

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
					++dwFilePos;
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
					mame_ungetc(bData, fp);
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
				/* Unix style perhaps? */

				bData = mame_fgetc(fp);		/* Peek ahead */
				mame_ungetc(bData, fp);		/* Force a retrigger if subsequent LF's */

				if (LF == bData)		/* Two LF's in a row - it's a UNIX hard CR */
				{
					++dwFilePos;
					*pbTokenPtr++ = bData;	/* A real linefeed */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}

				/* Otherwise, fall through and keep parsing. */

			}
			else if (CR == bData)			/* Carriage return? */
			{
				/* Figure out if it's Mac or MSDOS format */

				++dwFilePos;
				bData = mame_fgetc(fp);		/* Peek ahead */

				/* We don't need to bother with EOF checking. It will be 0xff if */
				/* it's the end of the file and will be caught by the outer loop. */

				if (CR == bData)		/* Mac style hard return! */
				{
					/* Do not advance the file pointer in case there are successive */
					/* CR/CR sequences */

					/* Stuff our character back upstream for successive CR's */

					mame_ungetc(bData, fp);

					*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
					*pbTokenPtr = '\0';

					return(TOKEN_LINEBREAK);
				}
				else if (LF == bData)		/* MSDOS format! */
				{
					if (mame32jp_wrap)
					{
						mame_ungetc(bData, fp);

						*pbTokenPtr++ = bData;	/* A real carriage return (hard) */
						*pbTokenPtr = '\0';

						return(TOKEN_LINEBREAK);
					}
					else
					{
						++dwFilePos;		/* Our file position to reset to */
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
								--dwFilePos;
							mame_ungetc(bData, fp);	/* Put the character back. No good */
						}
					}
				}
				else
				{
					--dwFilePos;
					mame_ungetc(bData, fp);	/* Put the character back. No good */
				}

				/* Otherwise, fall through and keep parsing */
			}
		}

		++dwFilePos;
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
	mame_file_error filerr;

	/* Open file up in binary mode */
	filerr = mame_fopen(SEARCHPATH_DATAFILE, pszFilename, OPEN_FLAG_READ, &fp);
	if (filerr != FILERR_NONE)
		return(FALSE);

	/* Otherwise, prepare! */

	dwFilePos = 0;
	return(TRUE);
}


/****************************************************************************
 *      ParseSeek - Move the file position indicator
 ****************************************************************************/
static UINT8 ParseSeek(long offset, int whence)
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

	num_games = 0;
	while (drivers[num_games] != NULL)
		num_games++;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
        idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
        while (count < num_games && TOKEN_INVALID != token)
	{
		long tell;
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

	long tell;
	long cmdtag_offset = 0;
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

static int index_datafile_drivinfo (struct tDatafileIndex **_index)
{
	struct tDatafileIndex *idx;
	int count = 0;
	UINT32 token = TOKEN_SYMBOL;

	num_games = 0;
	while (drivers[num_games] != NULL)
		num_games++;

	/* rewind file */
	if (ParseSeek (0L, SEEK_SET)) return 0;

	/* allocate index */
	idx = *_index = malloc ((num_games + 1) * sizeof (struct tDatafileIndex));
	if (NULL == idx) return 0;

	/* loop through datafile */
	while (count < num_games && TOKEN_INVALID != token)
	{
		long tell;
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
					strlwr(s);
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
	int     offset = 0;
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
		long tell;

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
	long tell;

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
		long tell;

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
	        	options_get_string(OPTION_LOCALIZED_DIRECTORY),
			ui_lang_info[options.langcode].name);
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

	if (oldLangCode != options.langcode)
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

		oldLangCode = options.langcode;
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
	                         "history/", options_get_string(OPTION_HISTORY_FILE));

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
		                         "story/", options_get_string(OPTION_STORY_FILE));

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
	machine_config game;
	int result = 1;
	int i;

	*buffer = 0;

	strcat(buffer, "MAMEInfo:\n");
	expand_machine_driver(drv->drv, &game);

	/* List the game info 'flags' */
	if (drv->flags &
	    ( GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS |
	      GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL))
	{
		strcat(buffer, _("GAME: "));
		strcat(buffer, options.use_lang_list?
			_LST(drv->description):
			drv->description);
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
	                         "mameinfo/", options_get_string(OPTION_MAMEINFO_FILE));

	strcat(buffer, _("\nROM REGION:\n"));
	for (region = rom_first_region(drv); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			char name[100];
			int length;

			length = 0;
			for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
				length += ROM_GETLENGTH(chunk);

			sprintf(name," %-12s ",ROM_GETNAME(rom));
			strcat(buffer, name);
			sprintf(name,"%6x ",length);
			strcat(buffer, name);
			switch (ROMREGION_GETTYPE(region))
			{
			case REGION_CPU1: strcat(buffer, "cpu1"); break;
			case REGION_CPU2: strcat(buffer, "cpu2"); break;
			case REGION_CPU3: strcat(buffer, "cpu3"); break;
			case REGION_CPU4: strcat(buffer, "cpu4"); break;
			case REGION_CPU5: strcat(buffer, "cpu5"); break;
			case REGION_CPU6: strcat(buffer, "cpu6"); break;
			case REGION_CPU7: strcat(buffer, "cpu7"); break;
			case REGION_CPU8: strcat(buffer, "cpu8"); break;
			case REGION_GFX1: strcat(buffer, "gfx1"); break;
			case REGION_GFX2: strcat(buffer, "gfx2"); break;
			case REGION_GFX3: strcat(buffer, "gfx3"); break;
			case REGION_GFX4: strcat(buffer, "gfx4"); break;
			case REGION_GFX5: strcat(buffer, "gfx5"); break;
			case REGION_GFX6: strcat(buffer, "gfx6"); break;
			case REGION_GFX7: strcat(buffer, "gfx7"); break;
			case REGION_GFX8: strcat(buffer, "gfx8"); break;
			case REGION_PROMS: strcat(buffer, "prom"); break;
			case REGION_SOUND1: strcat(buffer, "snd1"); break;
			case REGION_SOUND2: strcat(buffer, "snd2"); break;
			case REGION_SOUND3: strcat(buffer, "snd3"); break;
			case REGION_SOUND4: strcat(buffer, "snd4"); break;
			case REGION_SOUND5: strcat(buffer, "snd5"); break;
			case REGION_SOUND6: strcat(buffer, "snd6"); break;
			case REGION_SOUND7: strcat(buffer, "snd7"); break;
			case REGION_SOUND8: strcat(buffer, "snd8"); break;
			case REGION_USER1: strcat(buffer, "usr1"); break;
			case REGION_USER2: strcat(buffer, "usr2"); break;
			case REGION_USER3: strcat(buffer, "usr3"); break;
			case REGION_USER4: strcat(buffer, "usr4"); break;
			case REGION_USER5: strcat(buffer, "usr5"); break;
			case REGION_USER6: strcat(buffer, "usr6"); break;
			case REGION_USER7: strcat(buffer, "usr7"); break;
			case REGION_USER8: strcat(buffer, "usr8"); break;
			}

		sprintf(name," %7x\n",ROM_GETOFFSET(rom));
		strcat(buffer, name);

		}

	clone_of = driver_get_clone(drv);
	if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, options.use_lang_list?
			_LST(clone_of->description):
			clone_of->description);
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->parent, drivers[i]->parent)) 
			{
				strcat(buffer, options.use_lang_list?
					_LST(drivers[i]->description):
					drivers[i]->description);
				strcat(buffer, "\n");
			}
		}
	}
	else
	{
		strcat(buffer, _("\nORIGINAL:\n"));
		strcat(buffer, options.use_lang_list?
			_LST(drv->description):
			drv->description);
		strcat(buffer, _("\n\nCLONES:\n"));
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (drv->name, drivers[i]->parent)) 
			{
				strcat(buffer, options.use_lang_list?
					_LST(drivers[i]->description):
					drivers[i]->description);
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
	sprintf (buffer, _("\nSOURCE: %s\n"), drv->source_file+12);

	/* Try to open mameinfo datafile - driver section */
	if (ParseOpen (options_get_string(OPTION_MAMEINFO_FILE)))
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

/* Redundant Info
	strcat(buffer,"\nGAMES SUPPORTED:\n");
	for (i = 0; drivers[i]; i++)
	{
		if (!mame_stricmp (drv->source_file, drivers[i]->source_file)) 
		{
			strcat(buffer, options.use_lang_list?
				_LST(drivers[i]->description)):
				drivers[i]->description);
			strcat(buffer,"\n");
		}
	}
*/

	return (drivinfo == 0);
}

int load_driver_statistics (char *buffer, int bufsize)
{
	const rom_entry *region, *rom, *chunk;
	const rom_entry *pregion, *prom, *fprom=NULL;
	const game_driver *clone_of = NULL;
	const input_port_entry *inp;

	char name[100];
	char year[4];
	int i, n, x, y;
	int all = 0, cl = 0, vec = 0, vecc = 0, neo = 0, neoc = 0;
	int pch = 0, pchc = 0, deco = 0, decoc = 0, cvs = 0, cvsc = 0, noyear = 0, nobutt = 0, noinput = 0;
	int vertical = 0, verticalc = 0, horizontal = 0, horizontalc = 0;
	int clone = 0, stereo = 0, stereoc = 0;
	int sum = 0, xsum = 0, files = 0, mfiles = 0, hddisk = 0;
	int rsum = 0, ndgame = 0, ndsum = 0, gndsum = 0, gndsumc = 0, bdgame = 0, bdsum = 0, gbdsum = 0, gbdsumc = 0;
	int bitx = 0, bitc = 0, shad = 0, shadc = 0, hghs = 0, hghsc = 0;
	int rgbd = 0, rgbdc = 0;
	static int flags[20], romsize[10];
	static int numcpu[4][CPU_COUNT], numsnd[4][SOUND_COUNT], sumcpu[MAX_CPU+1], sumsound[MAX_SOUND+1];
	static int resx[400], resy[400], resnum[400];
	static int palett[300], palettnum[300], control[35];
	static int fpsnum[50];
	float fps[50];

	*buffer = 0;

	strcat(buffer, APPNAME " ");
	strcat(buffer, build_version);

	for (i = 0; drivers[i]; i++)
	{ 
		int controltmp[6];
		machine_config drv;
		expand_machine_driver(drivers[i]->drv, &drv);
 
		all++;

		clone_of = driver_get_clone(drivers[i]);
		if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
		{
			clone = 1;
			cl++;
		}
		else
			clone = 0;


		/* Calc all graphic resolution and aspect ratio numbers */
		if (drivers[i]->flags & ORIENTATION_SWAP_XY)
		{
			x = drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1;
			y = drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1;
		}
		else
		{
			x = drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1;
			y = drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1;
		}

		if (drv.video_attributes & VIDEO_TYPE_VECTOR)
		{
	 		vec++;
			if (clone) vecc++;
		}
		else
		{
			/* Store all resolutions, without the Vector games */
			for (n = 0; n < 200; n++)
			{
				if (resx[n] == x && resy[n] == y)
				{
					resnum [n]++;
					break;
				}

				if (resx[n] == 0)
				{
					resx[n] = x;
					resy[n] = y;
					resnum [n]++;
					break;
				}

			}

		}

		/* Calc all palettesize numbers */
		x = drv.total_colors;
		for (n = 0; n < 150; n++)
		{
			if (palett[n] == x)
			{
				palettnum [n]++;
				break;
			}

			if (palett[n] == 0)
			{
				palett[n] = x;
				palettnum [n]++;
				break;
			}
		}


		if (!mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
		{
			neo++;
			if (clone) neoc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "playch10.c"))
		{
			pch++;
			if (clone) pchc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "decocass.c"))
		{
			deco++;
			if (clone) decoc++;
		}

		if (!mame_stricmp (drivers[i]->source_file+12, "cvs.c"))
		{
			cvs++;
			if (clone) cvsc++;
		}

		if (drivers[i]->flags & ORIENTATION_SWAP_XY)
		{
	 		vertical++;
			if (clone) verticalc++;
		}
		else
		{
			horizontal++;
			if (clone) horizontalc++;
		}

		x = 0;
		for (y = 0; y < MAX_SPEAKER; y++)
			if (drv.speaker[y].tag != NULL)
				x++;

		if (x > 1)
		{
	 		stereo++;
			if (clone) stereoc++;
		}


		/* Calc GAME INPUT and numbers */
		n = 0, x = 0, y = 0;
		memset(controltmp, 0, sizeof controltmp);
		
		if (drivers[i]->construct_ipt)
		{
			begin_resource_tracking();
		
			inp = input_port_allocate(drivers[i]->construct_ipt, NULL);
			while (inp->type != IPT_END)
			{
				if (n < inp->player + 1)
					n = inp->player + 1;

				switch (inp->type)
				{
				case IPT_JOYSTICK_LEFT:
				case IPT_JOYSTICK_RIGHT:
					if (!y)
						y = 1;
					break;
				case IPT_JOYSTICK_UP:
				case IPT_JOYSTICK_DOWN:
					if (inp->four_way)
						y = 2;
					else
						y = 3;
					break;
				case IPT_JOYSTICKRIGHT_LEFT:
				case IPT_JOYSTICKRIGHT_RIGHT:
				case IPT_JOYSTICKLEFT_LEFT:
				case IPT_JOYSTICKLEFT_RIGHT:
					if (!y)
						y = 4;
					break;
				case IPT_JOYSTICKRIGHT_UP:
				case IPT_JOYSTICKRIGHT_DOWN:
				case IPT_JOYSTICKLEFT_UP:
				case IPT_JOYSTICKLEFT_DOWN:
					if (inp->four_way)
						y = 5;
					else
						y = 6;
					break;
				case IPT_BUTTON1:
					if (x<1) x = 1;
					break;
				case IPT_BUTTON2:
					if (x<2) x = 2;
					break;
				case IPT_BUTTON3:
					if (x<3) x = 3;
					break;
				case IPT_BUTTON4:
					if (x<4) x = 4;
					break;
				case IPT_BUTTON5:
					if (x<5) x = 5;
					break;
				case IPT_BUTTON6:
					if (x<6) x = 6;
					break;
				case IPT_BUTTON7:
					if (x<7) x = 7;
					break;
				case IPT_BUTTON8:
					if (x<8) x = 8;
					break;
				case IPT_BUTTON9:
					if (x<9) x = 9;
					break;
				case IPT_BUTTON10:
					if (x<10) x = 10;
					break;
				case IPT_PADDLE:
					controltmp[0] = 1;
					break;
				case IPT_DIAL:
					controltmp[1] = 1;
					break;
				case IPT_TRACKBALL_X:
				case IPT_TRACKBALL_Y:
					controltmp[2] = 1;
					break;
				case IPT_AD_STICK_X:
				case IPT_AD_STICK_Y:
					controltmp[3] = 1;
					break;
				case IPT_LIGHTGUN_X:
				case IPT_LIGHTGUN_Y:
					controltmp[4] = 1;
					break;
				case IPT_PEDAL:
					controltmp[5] = 1;
					break;
				}
				++inp;
			}
			end_resource_tracking();
		}	
		if (n) control[n]++;
		if (x) control[x+10]++;
		if (y) control[y+20]++;

		if (!y)
		{
			noinput++;
			for (y = 0; y < sizeof (controltmp) / sizeof (*controltmp); y++)
				if (controltmp[y])
				{
					noinput--;
					break;
				}
		}

		for (y = 0; y < sizeof (controltmp) / sizeof (*controltmp); y++)
			if (controltmp[y])
				control[y + 27]++;


		/* Calc all Frames_Per_Second numbers */
		fps[0] = drv.screen[0].defstate.refresh;
		for (n = 1; n < 50; n++)
		{
			if (fps[n] == fps[0])
			{
				fpsnum[n]++;
				break;
			}

			if (fpsnum[n] == 0)
			{
				fps[n] = fps[0];
				fpsnum[n]++;
				fpsnum[0]++;
				break;
			}
		}


		/* Calc number of various info 'flags' in original and clone games */
		if (mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
		{
			if (drivers[i]->flags & GAME_NOT_WORKING)
			{
	 			flags[1]++;
				if (clone) flags[11]++;
			}

			if (drivers[i]->flags & GAME_UNEMULATED_PROTECTION)
			{
	 			flags[2]++;
				if (clone) flags[12]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_GRAPHICS)
			{
	 			flags[3]++;
				if (clone) flags[13]++;
			}
			if (drivers[i]->flags & GAME_WRONG_COLORS)
			{
	 			flags[4]++;
				if (clone) flags[14]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_COLORS)
			{
	 			flags[5]++;
				if (clone) flags[15]++;
			}
			if (drivers[i]->flags & GAME_NO_SOUND)
			{
	 			flags[6]++;
				if (clone) flags[16]++;
			}
			if (drivers[i]->flags & GAME_IMPERFECT_SOUND)
			{
	 			flags[7]++;
				if (clone) flags[17]++;
			}
			if (drivers[i]->flags & GAME_NO_COCKTAIL)
			{
	 			flags[8]++;
				if (clone) flags[18]++;
			}

			if (drv.video_attributes & VIDEO_NEEDS_6BITS_PER_GUN)
			{
	 			bitx++;
				if (clone) bitc++;
			}

			if (drv.video_attributes & VIDEO_RGB_DIRECT)
			{
	 			rgbd++;
				if (clone) rgbdc++;
			}

			if (drv.video_attributes & VIDEO_HAS_SHADOWS)
			{
	 			shad++;
				if (clone) shadc++;
			}

			if (drv.video_attributes & VIDEO_HAS_HIGHLIGHTS)
			{
	 			hghs++;
				if (clone) hghsc++;
			}


			if (!clone)
			{
				/* Calc all CPU's only in ORIGINAL games */
				y = 0;
				n = 0;
				while (n < MAX_CPU && drv.cpu[n].cpu_type)
				{
					int type = drv.cpu[n].cpu_type;
					int count = 0;
					char cpu_name[256];

					n++;

					while (n < MAX_CPU
						&& drv.cpu[n].cpu_type == type)
					{
						count++;
						n++;
					}

					strcpy(cpu_name, cputype_shortname(type));
					if (cpu_name[0] == '\0')
						continue;

					if (count < 4)
					{
						numcpu[count][type]++;
						y++;
					}
				}

				sumcpu[y]++;


				/* Calc all Sound hardware only in ORIGINAL games */
				y = 0;
				n = 0;
				while (n < MAX_SOUND && drv.sound[n].sound_type)
				{
					int type = drv.sound[n].sound_type;
					int count = 0;
					char sound_name[256];

					n++;

					while (n < MAX_CPU
						&& drv.sound[n].sound_type == type)
					{
						count++;
						n++;
					}

					strcpy(sound_name, sndtype_shortname(type));
					if (sound_name[0] == '\0')
						continue;

					if (type == SOUND_FILTER_VOLUME
					 || type == SOUND_FILTER_RC
					 || type == SOUND_FILTER_LOWPASS)
						continue;

					if (count < 4)
					{
						numsnd[count][type]++;
						y++;
					}
				}

				sumsound[y]++;
			}
		}


		/* Calc number of ROM files and file size */
		for (region = rom_first_region(drivers[i]); region; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				int length, in_parent, is_disk;
				is_disk = ROMREGION_ISDISKDATA(region);



				length = 0;
				in_parent = 0;

				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					length += ROM_GETLENGTH(chunk)/32;

				if (clone_of)
				{
					fprom=NULL;
					for (pregion = rom_first_region(clone_of); pregion; pregion = rom_next_region(pregion))
						for (prom = rom_first_file(pregion); prom; prom = rom_next_file(prom))
							if (hash_data_is_equal(ROM_GETHASHDATA(rom), ROM_GETHASHDATA(prom), 0))
							{
								if (!fprom || !strcmp(ROM_GETNAME(prom), name))
									fprom=prom;
								in_parent = 1;
							}

				}

				if (!is_disk)
				{
					sum += length;
					rsum += length;
					files++;

					if(in_parent)
					{
						xsum += length;
						mfiles++;
					}
				}
				else
					hddisk++;

				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_NO_DUMP))
				{
					ndsum++;
					ndgame = 1;
				}

				if (hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP))
				{
					bdsum++;
					bdgame = 1;
				}

			}

		if (ndgame)
		{
			gndsum++;
			if (clone) gndsumc++;
			ndgame = 0;
		}

		if (bdgame)
		{
			gbdsum++;
			if (clone) gbdsumc++;
			bdgame = 0;
		}


		rsum = rsum/32;
		if(rsum < 5)
			romsize[0]++;
		if(rsum >= 5 && rsum < 10)
			romsize[1]++;
		if(rsum >= 10 && rsum < 50)
			romsize[2]++;
		if(rsum >= 50 && rsum < 100)
			romsize[3]++;
		if(rsum >= 100 && rsum < 500)
			romsize[4]++;
		if(rsum >= 500 && rsum < 1000)
			romsize[5]++;
		if(rsum >= 1000 && rsum < 5000)
			romsize[6]++;
		if(rsum >= 5000 && rsum < 10000)
			romsize[7]++;
		if(rsum >= 10000 && rsum < 50000)
			romsize[8]++;
		if(rsum >= 50000)
			romsize[9]++;
		rsum = 0;

	}

	sum = sum/32;
	xsum = xsum/32;
	noyear = all;		/* See Print Year and Games */
	// already calculated.
	//noinput = all;		/* See Input */
	nobutt = all;		/* See Input Buttons */


	sprintf(name, _("\n\n %4d GAMES (ALL)\n %4d ORIGINALS  + %4d CLNS\n %4d NEOGEO     + %4d\n"), all, all-cl-neo+neoc, cl-neoc, neo-neoc, neoc);
	strcat(buffer, name);
	sprintf(name, _(" %4d PLAYCHOICE + %4d\n %4d DECO CASS  + %4d\n %4d CVS        + %4d\n"), pch-pchc, pchc, deco-decoc, decoc, cvs-cvsc, cvsc);
	strcat(buffer, name);
	sprintf(name, _(" %4d RASTER     + %4d\n %4d VECTOR     + %4d\n"), all-vec-cl+vecc, cl-vecc, vec-vecc, vecc);
	strcat(buffer, name);
	sprintf(name, _(" %4d HORIZONTAL + %4d\n %4d VERTICAL   + %4d\n"), horizontal-horizontalc, horizontalc, vertical-verticalc, verticalc);
	strcat(buffer, name);
	sprintf(name, _(" %4d STEREO     + %4d\n"), stereo-stereoc, stereoc);
	strcat(buffer, name);
	sprintf(name, _(" %4d HARDDISK\n"), hddisk);
	strcat(buffer, name);

	/* Print number of various info 'flags' */
	strcat(buffer,_("\n\nGAME INFO FLAGS   : ORIG CLNS\n\n"));
	sprintf(name, _("NON-WORKING        : %3d  %3d\n"), flags[1]-flags[11], flags[11]);
	strcat(buffer, name);
	sprintf(name, _("UNEMULATED PROTEC. : %3d  %3d\n"), flags[2]-flags[12], flags[12]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT GRAPHICS : %3d  %3d\n"), flags[3]-flags[13], flags[13]);
	strcat(buffer, name);
	sprintf(name, _("WRONG COLORS       : %3d  %3d\n"), flags[4]-flags[14], flags[14]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT COLORS   : %3d  %3d\n"), flags[5]-flags[15], flags[15]);
	strcat(buffer, name);
	sprintf(name, _("NO SOUND           : %3d  %3d\n"), flags[6]-flags[16], flags[16]);
	strcat(buffer, name);
	sprintf(name, _("IMPERFECT SOUND    : %3d  %3d\n"), flags[7]-flags[17], flags[17]);
	strcat(buffer, name);
	sprintf(name, _("NO COCKTAIL        : %3d  %3d\n"), flags[8]-flags[18], flags[18]);
	strcat(buffer, name);
	sprintf(name, _("NO GOOD DUMP KNOWN : %3d  %3d\n(%3d ROMS IN %d GAMES)\n"), gndsum-gndsumc, gndsumc, ndsum, gndsum);
	strcat(buffer, name);
	sprintf(name, _("ROM NEEDS REDUMP   : %3d  %3d\n(%3d ROMS IN %d GAMES)\n"), gbdsum-gbdsumc, gbdsumc, bdsum, gbdsum);
	strcat(buffer, name);




	/* Print Year and Games - Note: Some games have no year*/
	strcat(buffer,_("\n\nYEAR: ORIG  CLNS NEOGEO  ALL\n\n"));
	for (x = 1972; x < 2004; x++)
	{

		all = 0; cl = 0; neo = 0; neoc = 0;

		sprintf(year,"%d",x);
		for (i = 0; drivers[i]; i++)
		{
			if (!mame_stricmp (year, drivers[i]->year))
			{ 
				all++;

				if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
					cl++;
				if (!mame_stricmp (drivers[i]->source_file+12, "neogeo.c"))
				{
					neo++;
					if (clone_of && !(clone_of->flags & NOT_A_DRIVER))
						neoc++;
				}
			}
		}

		sprintf(name, "%d: %3d   %3d   %3d    %3d\n", x, all-cl-neo+neoc, cl-neoc, neo, all);
		strcat(buffer, name);

		noyear = noyear - all;	/* Number of games with no year informations */

	}
	sprintf(name, "19??:                    %3d\n\n", noyear);
	strcat(buffer, name);


	/* Print number of ROM files and whole file size */
	if(sum > 524288)
		sprintf(name, _("\nROMFILES: %d\n%d + %d MERGED ROMS\n\nFILESIZE: %d MB (UNCOMPRESSED)\n%d MB + %d MB MERGED\n"), files, files-mfiles, mfiles, sum/1024, sum/1024-xsum/1024, xsum/1024);
	else
		sprintf(name, _("\n\nROMFILES: %d\n%d + %d MERGED ROMS\n\nFILESIZE: %d KB (UNCOMPRESSED)\n%d KB + %d KB MERGED\n"), files, files-mfiles, mfiles, sum, sum-xsum, xsum);
	strcat(buffer, name);



	/* Print the typical sizes of all supported romsets in kbytes */
	strcat(buffer,_("\n\nTYPICAL ROMSET SIZE (ALL)\n"));

	sprintf(name, "\n    0 -     5 KB:  %4d\n    5 -    10 KB:  %4d\n   10 -    50 KB:  %4d\n   50 -   100 KB:  %4d\n", romsize[0], romsize[1], romsize[2], romsize[3]);
	strcat(buffer, name);
	sprintf(name, "  100 -   500 KB:  %4d\n  500 -  1000 KB:  %4d\n 1000 -  5000 KB:  %4d\n 5000 - 10000 KB:  %4d\n", romsize[4], romsize[5], romsize[6], romsize[7]);
	strcat(buffer, name);
	sprintf(name, "10000 - 50000 KB:  %4d\n      > 50000 KB:  %4d\n", romsize[8], romsize[9]);
	strcat(buffer, name);



	/* Print year and the number of added games */
	strcat(buffer,_("\n\nYEAR    NEW GAMES\n\n"));
	for (i = 0; stat_newgames[i]; i++)
	{
		strcat(buffer,stat_newgames[i]);
		strcat(buffer,"\n");
	}



	/* Print all games and their maximum CPU's */
	strcat(buffer,_("\n\nCPU HARDWARE (ORIGINAL GAMES)\n\n"));
	for (n = 0; n < MAX_CPU+1; n++)
	{
		if (sumcpu[n])
		{
			sprintf(name, _(" GAMES WITH  %d CPUs: %4d\n"), n, sumcpu[n]);
			strcat(buffer, name);
		}

	}


	/* Print all used CPU's and numbers of original games they used */
	strcat(buffer,"\n         CPU:  (1)  (2)  (3)  (4)\n\n");
	all = 0;
	for (i = 1; i < CPU_COUNT; i++)
	{
		if (numcpu[0][i] || numcpu[1][i] || numcpu[2][i] || numcpu[3][i])
		{
			sprintf(name, "%12s: %4d %4d %4d %4d\n", cputype_shortname(i), numcpu[0][i], numcpu[1][i], numcpu[2][i], numcpu[3][i]);
			strcat(buffer, name);
			all = all + numcpu[0][i] + numcpu[1][i] + numcpu[2][i] + numcpu[3][i];
		}
	}

	/* Print the number of all CPU the original games */
	sprintf(name, _("\n   TOTAL: %4d\n"), all);
	strcat(buffer, name);


	/* Print all games and their maximum number of sound subsystems */
	strcat(buffer,_("\n\nSOUND SYSTEM (ORIGINAL GAMES)\n\n"));
	for (n = 0; n < MAX_SOUND+1; n++)
	{
		if (sumsound[n])
		{
			sprintf(name, _(" GAMES WITH  %d SNDINTRF: %4d\n"), n, sumsound[n]);
			strcat(buffer, name);
		}

	}

	/* Print all Sound hardware and numbers of games they used */
	strcat(buffer,_("\n    SNDINTRF:  (1)  (2)  (3)  (4)\n\n"));
	all = 0;
	for (i = 1; i < SOUND_COUNT; i++)
	{
		if (numsnd[0][i] || numsnd[1][i] || numsnd[2][i] || numsnd[3][i])
		{
			sprintf(name, "%12s: %4d %4d %4d %4d\n", sndtype_shortname(i), numsnd[0][i], numsnd[1][i], numsnd[2][i], numsnd[3][i]);
			strcat(buffer, name);
			all = all + numsnd[0][i] + numsnd[1][i] + numsnd[2][i] + numsnd[3][i];
		}
	}

	sprintf(name, _("\n    TOTAL: %4d\n"), all);
	strcat(buffer, name);


	/* Print all Input Controls and numbers of all games */
	strcat(buffer, _("\n\nCABINET INPUT CONTROL: (ALL)\n\n"));
	for (n = 1; n < 9; n++)
	{
		if (control[n])
		{

			sprintf(name, _("     PLAYERS %d:  %4d\n"), n, control[n]);
			strcat(buffer, name);
		}
	}

	strcat(buffer, "\n");
	if (control[21]) { sprintf(name, _("       JOY2WAY:  %4d\n"), control[21]); strcat(buffer, name); }
	if (control[22]) { sprintf(name, _("       JOY4WAY:  %4d\n"), control[22]); strcat(buffer, name); }
	if (control[23]) { sprintf(name, _("       JOY8WAY:  %4d\n"), control[23]); strcat(buffer, name); }
	if (control[24]) { sprintf(name, _(" DOUBLEJOY2WAY:  %4d\n"), control[24]); strcat(buffer, name); }
	if (control[25]) { sprintf(name, _(" DOUBLEJOY4WAY:  %4d\n"), control[25]); strcat(buffer, name); }
	if (control[26]) { sprintf(name, _(" DOUBLEJOY8WAY:  %4d\n"), control[26]); strcat(buffer, name); }
	if (control[27]) { sprintf(name, _("        PADDLE:  %4d\n"), control[27]); strcat(buffer, name); }
	if (control[28]) { sprintf(name, _("          DIAL:  %4d\n"), control[28]); strcat(buffer, name); }
	if (control[29]) { sprintf(name, _("     TRACKBALL:  %4d\n"), control[29]); strcat(buffer, name); }
	if (control[30]) { sprintf(name, _("      AD STICK:  %4d\n"), control[30]); strcat(buffer, name); }
	if (control[31]) { sprintf(name, _("      LIGHTGUN:  %4d\n"), control[31]); strcat(buffer, name); }
	if (control[32]) { sprintf(name, _("         PEDAL:  %4d\n"), control[32]); strcat(buffer, name); }

	sprintf(name, _("         OTHER:  %4d\n"), noinput);
	strcat(buffer, name);

	strcat(buffer, "\n");
	for (n = 11; n < 21; n++)
	{
		if (control[n])
		{

			sprintf(name, _("    BUTTONS%3d:  %4d\n"), n-10, control[n]);
			strcat(buffer, name);
			nobutt = nobutt - control[n];			
		}
	}

	sprintf(name, _("    NO BUTTONS:  %4d\n"), nobutt);
	strcat(buffer, name);


	/* Print the video_attributes */
	strcat(buffer,_("\n\nVIDEO NEEDS... : ORIG   CLNS\n\n"));
	sprintf(name, _("24-BIT DISPLAY : %3d  + %3d\n"), bitx-bitc, bitc);
	strcat(buffer, name);
	sprintf(name, _("HI/TRUE BITMAP : %3d  + %3d\n"), rgbd-rgbdc, rgbdc);
	strcat(buffer, name);
	sprintf(name, _("SHADOWS        : %3d  + %3d\n"), shad-shadc, shadc);
	strcat(buffer, name);
	sprintf(name, _("HIGHLIGHTS     : %3d  + %3d\n"), hghs-hghsc, hghsc);
	strcat(buffer, name);


	/* FRAMES_PER_SECOND: Sort and print all fps */
	sprintf(name,_("\n\nFRAMES PER SECOND (%d): (ALL)\n\n"), fpsnum[0]);
	strcat(buffer, name);
	for (y = 1; y < 50; y++)
	{
		fps[0] = 199;
		for (n = 1; n < 50; n++)
		{
			if (fpsnum[n] && fps[0] > fps[n])
				fps[0] = fps[n];
		}

		for (n = 1; n < 50; n++)	/* Print fps and number*/
		{
			if (fps[0] == fps[n])
			{
				sprintf(name, "  FPS %f:  %4d\n", fps[n], fpsnum[n]);
				strcat(buffer, name);
				fpsnum[n] = 0;
			}
		}

	}

	if (fpsnum[0] > 48)
		strcat(buffer, "\nWARNING: FPS number too high!\n");


	/* RESOLUTION: Sort all games resolutions by x and y */
	cl = 0;
	for (all = 0; all < 200; all++)
	{
		x = 999;
		for (n = 0; n < 200; n++)
		{
			if (resx[n] && x > resx[n])
				x = resx[n];
		}

		y = 999;
		for (n = 0; n < 200; n++)
		{
			if (x == resx[n] && y > resy[n])
				y = resy[n];
		}

		for (n = 0; n < 200; n++)
		{
			if (x == resx[n] && y == resy[n])
			{
				/* Store all sorted resolutions in the higher array */
				resx[200+cl] = resx[n];
				resy[200+cl] = resy[n];
				resnum[200+cl] = resnum[n];
				cl++;
				resx[n] = 0, resy[n] = 0, resnum[n] = 0;
			}
		}
	}


	/* PALETTESIZE: Sort the palettesizes */
	x = 0;
	for (y = 0; y < 150; y++)
	{
		i = 99999;
		for (n = 0; n < 150; n++)
		{
			if (palett[n] && i > palett[n])
				i = palett[n];
		}

		for (n = 0; n < 150; n++)	/* Store all sorted resolutions in the higher array */
		{
			if (i == palett[n])
			{
				palett[150+x] = palett[n];
				palettnum[150+x] = palettnum[n];
				x++;
				palett[n] = 0, palettnum[n] = 0;
			}
		}
	}

	/* RESOLUTION & PALETTESIZE: Print all resolutions and palettesizes */
	sprintf(name,_("\n\nRESOLUTION & PALETTESIZE: (ALL)\n    (%d)          (%d)\n\n"), cl, x);
	strcat(buffer, name);
	for (n = 0; n < 200; n++)
	{

		if (resx[n+200])
		{
			sprintf(name, "  %dx%d: %3d    ", resx[n+200], resy[n+200], resnum[n+200]);
			strcat(buffer, name);
		}

		if (n < 150 && palett[n+150])
		{
			sprintf(name, "%5d: %3d\n", palett[n+150], palettnum[n+150]);
			strcat(buffer, name);
		}
		else
			if (resx[n+200])	strcat(buffer, "\n");

	}

	if (cl > 198)
		strcat(buffer, "\nWARNING: Resolution number too high!\n");

	if (x > 148)
		strcat(buffer, "\nWARNING: Palettesize number too high!\n");


	/* MAME HISTORY - Print all MAME versions + Drivers + Supported Games (generated with MAMEDiff) */
	strcat(buffer,_("\n\nVERSION - DRIVERS - SUPPORT:\n\n"));
	for (i = 0; stat_history[i]; i++)
	{
		strcat(buffer,stat_history[i]);
		strcat(buffer,"\n");
	}


	/* Calc all MAME versions and print all version */
	for (i = 0; stat_versions[i]; i++){}
	sprintf(name, _("\n\nMAME VERSIONS (%3d)\n\n"), i);
	strcat(buffer, name);

	for (i = 0; stat_versions[i]; i++)
	{
		strcat(buffer,stat_versions[i]);
		strcat(buffer,"\n");
	}


	/* CLEAR ALL DATA ARRAYS */
	memset(numcpu, 0, sizeof numcpu);
	memset(numsnd, 0, sizeof numsnd);
	memset(resx, 0, sizeof resx);
	memset(resy, 0, sizeof resy);
	memset(resnum, 0, sizeof resnum);
	memset(palett, 0, sizeof palett);
	memset(palettnum, 0, sizeof palettnum);

	memset(flags, 0, sizeof flags);
	memset(romsize, 0, sizeof romsize);
	memset(control, 0, sizeof control);
	memset(fps, 0, sizeof fps);
	memset(fpsnum, 0, sizeof fpsnum);
	memset(sumcpu, 0, sizeof sumcpu);
	memset(sumsound, 0, sizeof sumsound);

	return 0;

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
		        	options_get_string(OPTION_LOCALIZED_DIRECTORY),
				ui_lang_info[options.langcode].name);
		}

		base = filename + strlen(filename);

		for (i = where; i <= where + FILE_MERGED; i += FILE_MERGED)
		{
			int status = 0;

			if (i & FILE_MERGED)
			{
				if (where & FILE_ROOT)
					strcpy(base, options_get_string(OPTION_COMMAND_FILE));
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
