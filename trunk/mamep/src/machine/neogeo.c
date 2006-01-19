#include "driver.h"
#include "machine/pd4990a.h"
#include "neogeo.h"
#include "inptport.h"
#include "state.h"
#include <time.h>
#include "sound/2610intf.h"


#ifdef USE_NEOGEO_HACKS
int system_bios;
#endif /* USE_NEOGEO_HACKS */

extern int neogeo_sram_locked;
extern offs_t neogeo_sram_protection_hack;
extern void *record;
extern void *playback;

extern int neogeo_rng;
extern int neogeo_prot_data;

UINT16 *neogeo_ram16;
UINT16 *neogeo_sram16;


/***************** MEMCARD GLOBAL VARIABLES ******************/
int neogeo_memcard_is_loaded;

static UINT8 *neogeo_memcard;		/* Pointer to 2kb RAM zone */
static int memcard_number;

UINT8 *neogeo_game_vectors;

static void neogeo_custom_memory(void);
static void neogeo_register_sub_savestate(void);


/* This function is called on every reset */
MACHINE_INIT( neogeo )
{
#ifdef USE_NEOGEO_HACKS
	UINT16 src, res, *mem16= (UINT16 *)memory_region(REGION_USER1);
#endif /* USE_NEOGEO_HACKS */
	time_t ltime;
	struct tm *today;
	int memcard_manager = 0; // FIXME


	/* Reset variables & RAM */
	memset (neogeo_ram16, 0, 0x10000);

#ifdef USE_NEOGEO_HACKS
	if (Machine->gamedrv->bios)
	{
		if (system_bios == NEOGEO_BIOS_EURO)
		{
			/* Set up machine country */
			src = readinputport(5);
			res = src & 0x3;

			/* Console/arcade mode */
			if (src & 0x04) 
				res |= 0x8000;

			/* write the ID in the system BIOS ROM */
			mem16[0x0200] = res;

			if (memcard_manager==1)
			{
				memcard_manager=0;
				mem16[0x11b1a/2] = 0x500a;
			}
			else
			{
				mem16[0x11b1a/2] = 0x1b6a;
			}
		}

		if (system_bios == NEOGEO_BIOS_DEBUG)
		{
			/* Set up machine country */
			src = readinputport(5);
			res = src & 0x3;

			/* write the ID in the system BIOS ROM */
			mem16[0x0200] = res;

			if (memcard_manager==1)
			{
				memcard_manager=0;
				mem16[0x11b1a/2] = 0x3cac;
			}
			else
			{
				mem16[0x1194c/2] = 0x1b6a;
			}
		}
	}

	if (neogeo_has_trackball)
	{
		/* Set up machine country */
		src = readinputport(5);
		res = src & 0x3;

		/* Console/arcade mode */
		if (src & 0x04) 
			res |= 0x8000;

		/* write the ID in the system BIOS ROM */
		mem16[0x0200] = res;

		if (memcard_manager==1)
		{
			memcard_manager=0;
			mem16[0x10c44/2] = 0x4366;
		}
		else
		{
			mem16[0x10c44/2] = 0x0c94;
		}
	}
#endif /* USE_NEOGEO_HACKS */

	time(&ltime);
	today = localtime(&ltime);

	/* Disable Real Time Clock if the user selects to record or playback an .inp file   */
	/* This is needed in order to playback correctly an .inp on several games,as these  */
	/* use the RTC of the NEC pd4990a as pseudo-random number generator   -kal 8 apr 02 */
	if( record != 0 || playback != 0 )
	{
		pd4990a.seconds = 0;
		pd4990a.minutes = 0;
		pd4990a.hours = 0;
		pd4990a.days = 0;
		pd4990a.month = 0;
		pd4990a.year = 0;
		pd4990a.weekday = 0;
	}
	else
	{
		pd4990a.seconds = ((today->tm_sec/10)<<4) + (today->tm_sec%10);
		pd4990a.minutes = ((today->tm_min/10)<<4) + (today->tm_min%10);
		pd4990a.hours = ((today->tm_hour/10)<<4) + (today->tm_hour%10);
		pd4990a.days = ((today->tm_mday/10)<<4) + (today->tm_mday%10);
		pd4990a.month = (today->tm_mon + 1);
		pd4990a.year = (((today->tm_year%100)/10)<<4) + (today->tm_year%10);
		pd4990a.weekday = today->tm_wday;
	}

	neogeo_rng = 0x2345;	/* seed for the protection RNG in KOF99 onwards */
}


/* This function is only called once per game. */
DRIVER_INIT( neogeo )
{
	extern struct YM2610interface neogeo_ym2610_interface;
	UINT16 *mem16 = (UINT16 *)memory_region(REGION_CPU1);
	int tileno,numtiles;

#ifdef USE_NEOGEO_HACKS
	system_bios = determine_bios_rom(Machine->gamedrv->bios);
#endif /* USE_NEOGEO_HACKS */

	numtiles = memory_region_length(REGION_GFX3)/128;
	for (tileno = 0;tileno < numtiles;tileno++)
	{
		unsigned char swap[128];
		UINT8 *gfxdata;
		int x,y;
		unsigned int pen;

		gfxdata = &memory_region(REGION_GFX3)[128 * tileno];

		memcpy(swap,gfxdata,128);

		for (y = 0;y < 16;y++)
		{
			UINT32 dw;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[64 + 4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[64 + 4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[64 + 4*y + 2] >> x) & 1) << 1;
				pen |=	(swap[64 + 4*y	  ] >> x) & 1;
				dw |= pen << 4*x;
			}
			*(gfxdata++) = dw>>0;
			*(gfxdata++) = dw>>8;
			*(gfxdata++) = dw>>16;
			*(gfxdata++) = dw>>24;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[4*y + 2] >> x) & 1) << 1;
				pen |=	(swap[4*y	 ] >> x) & 1;
				dw |= pen << 4*x;
			}
			*(gfxdata++) = dw>>0;
			*(gfxdata++) = dw>>8;
			*(gfxdata++) = dw>>16;
			*(gfxdata++) = dw>>24;
		}
	}

	if (memory_region(REGION_SOUND2))
	{
		logerror("using memory region %d for Delta T samples\n",REGION_SOUND2);
		neogeo_ym2610_interface.pcmromb = REGION_SOUND2;
	}
	else
	{
		logerror("using memory region %d for Delta T samples\n",REGION_SOUND1);
		neogeo_ym2610_interface.pcmromb = REGION_SOUND1;
	}

	/* Allocate ram banks */
	neogeo_ram16 = auto_malloc (0x10000);
	if (!neogeo_ram16)
		return;
	memory_set_bankptr(1, neogeo_ram16);

	/* Set the biosbank */
	memory_set_bankptr(3, memory_region(REGION_USER1));

	/* Set the 2nd ROM bank */
	if (memory_region_length(REGION_CPU1) > 0x100000)
		neogeo_set_cpu1_second_bank(0x100000);
	else
		neogeo_set_cpu1_second_bank(0x000000);

	/* Set the sound CPU ROM banks */
	neogeo_init_cpu2_setbank();

	/* Allocate and point to the memcard - bank 5 */
	neogeo_memcard = auto_malloc(0x800);
	if (!neogeo_memcard)
		return;
	memset(neogeo_memcard, 0, 0x800);
	neogeo_memcard_is_loaded = 0;
	memcard_number = 0;

	init_memcard();
	memcard_intf.create = neogeo_memcard_create;
	memcard_intf.load = neogeo_memcard_load;
	memcard_intf.save = neogeo_memcard_save;
	memcard_intf.eject = neogeo_memcard_eject;

	mem16 = (UINT16 *)memory_region(REGION_USER1);

	/* irritating maze uses a trackball */
	if (!strcmp(Machine->gamedrv->name,"irrmaze"))
	{
		neogeo_has_trackball = 1;

#ifdef USE_NEOGEO_HACKS
		/* TODO: check the memcard manager patch in neogeo_init_machine(), */
		/* it probably has to be moved as well */
		/* Remove memory check for now */
		mem16[0x10c2a/2] = 0x4e71;
		mem16[0x10c2c/2] = 0x4e71;
		mem16[0x10c40/2] = 0x4ef9;
		mem16[0x10c42/2] = 0x00c1;
		mem16[0x10c44/2] = 0x0c94;

		/* Patch bios rom, for Calendar errors */
		mem16[0x10d3e/2] = 0x4e71;
		mem16[0x10d40/2] = 0x4e71;
		mem16[0x10d46/2] = 0x4e71;
		mem16[0x10d48/2] = 0x4e71;

		/* Rom internal checksum fails for now.. */
		mem16[0x10d8c/2] = 0x4e71;
		mem16[0x10d8e/2] = 0x4e71;
#endif /* USE_NEOGEO_HACKS */
	}
	else
	{
		neogeo_has_trackball = 0;

#ifdef USE_NEOGEO_HACKS
		if (Machine->gamedrv->bios)
		{
			if (system_bios == NEOGEO_BIOS_EURO)
			{
				/* Remove memory check for now */
				mem16[0x11b00/2] = 0x4e71;
				mem16[0x11b02/2] = 0x4e71;
				mem16[0x11b16/2] = 0x4ef9;
				mem16[0x11b18/2] = 0x00c1;
				mem16[0x11b1a/2] = 0x1b6a;

				/* Patch bios rom, for Calendar errors */
				mem16[0x11c14/2] = 0x4e71;
				mem16[0x11c16/2] = 0x4e71;
				mem16[0x11c1c/2] = 0x4e71;
				mem16[0x11c1e/2] = 0x4e71;

				/* Rom internal checksum fails for now.. */
				mem16[0x11c62/2] = 0x4e71;
				mem16[0x11c64/2] = 0x4e71;
			}
		}
#endif /* USE_NEOGEO_HACKS */
	}

	{ /* info from elsemi, this is how nebula works, is there a better way in mame? */
		UINT8* gamerom = memory_region(REGION_CPU1);
		neogeo_game_vectors = auto_malloc (0x80);
		memcpy( neogeo_game_vectors, gamerom, 0x80 );
	}

	/* setup SRAM protection */
	install_sram_protection();

	/* register state save */
	neogeo_register_main_savestate();
	neogeo_register_sub_savestate();
}

/******************************************************************************/

WRITE16_HANDLER (neogeo_select_bios_vectors)
{
	UINT8* gamerom = memory_region(REGION_CPU1);
	UINT8* biosrom = memory_region(REGION_USER1);

	memcpy( gamerom, biosrom, 0x80 );
}

WRITE16_HANDLER (neogeo_select_game_vectors)
{
	UINT8* gamerom = memory_region(REGION_CPU1);
	memcpy( gamerom, neogeo_game_vectors, 0x80 );
}

/******************************************************************************/



NVRAM_HANDLER( neogeo )
{
	if (read_or_write)
	{
		/* Save the SRAM settings */
		mame_fwrite_msbfirst(file,neogeo_sram16,0x2000);

		/* save the memory card */
		neogeo_memcard_save();
	}
	else
	{
		/* Load the SRAM settings for this game */
		if (file)
			mame_fread_msbfirst(file,neogeo_sram16,0x2000);
		else
			memset(neogeo_sram16,0,0x10000);

		/* load the memory card */
		neogeo_memcard_load(memcard_number);
	}
}



/*
    INFORMATION:

    Memory card is a 2kb battery backed RAM.
    It is accessed thru 0x800000-0x800FFF.
    Even bytes are always 0xFF
    Odd bytes are memcard data (0x800 bytes)

    Status byte at 0x380000: (BITS ARE ACTIVE *LOW*)

    0 PAD1 START
    1 PAD1 SELECT
    2 PAD2 START
    3 PAD2 SELECT
    4 --\  MEMORY CARD
    5 --/  INSERTED
    6 MEMORY CARD WRITE PROTECTION
    7 UNUSED (?)
*/




/********************* MEMCARD ROUTINES **********************/
READ16_HANDLER( neogeo_memcard16_r )
{
	if (neogeo_memcard_is_loaded)
		return neogeo_memcard[offset] | 0xff00;
	else
		return ~0;
}

WRITE16_HANDLER( neogeo_memcard16_w )
{
	if (ACCESSING_LSB)
	{
		if (neogeo_memcard_is_loaded)
			neogeo_memcard[offset] = data & 0xff;
	}
}

int neogeo_memcard_load(int number)
{
	char name[16];
	mame_file *f;

	sprintf(name, "MEMCARD.%03d", memcard_number);
	if ((f=mame_fopen(0, name, FILETYPE_MEMCARD, 0))!=0)
	{
		mame_fread(f, neogeo_memcard, 0x800);
		mame_fclose(f);

		neogeo_memcard_is_loaded = 1;
		memcard_number = number;

		return 1;
	}

	return 0;
}

void neogeo_memcard_save(void)
{
	char name[16];
	mame_file *f;

	if (neogeo_memcard_is_loaded)
	{
		sprintf(name, "MEMCARD.%03d", memcard_number);
		if ((f=mame_fopen(0, name, FILETYPE_MEMCARD, 1))!=0)
		{
			mame_fwrite(f, neogeo_memcard, 0x800);
			mame_fclose(f);
		}
	}
}

void neogeo_memcard_eject(void)
{
	neogeo_memcard_save();

	memset(neogeo_memcard, 0, 0x800);
	neogeo_memcard_is_loaded = 0;
}

int neogeo_memcard_create(int number)
{
	char buf[0x800];
	char name[16];
	mame_file *f1, *f2;

	sprintf(name, "MEMCARD.%03d", number);
	if ((f1=mame_fopen(0, name, FILETYPE_MEMCARD, 0))==0)
	{
		if ((f2=mame_fopen(0, name, FILETYPE_MEMCARD, 1))!=0)
		{
			mame_fwrite(f2, buf, 0x800);
			mame_fclose(f2);
			return 1;
		}
	}
	else
		mame_fclose(f1);

	return 0;
}

/******************************************************************************/

static void neogeo_register_sub_savestate(void)
{
	UINT8* gamevector = memory_region(REGION_CPU1);

	state_save_register_int   ("neogeo", 0, "neogeo_sram_locked",      &neogeo_sram_locked);
	state_save_register_UINT16("neogeo", 0, "neogeo_ram16",            neogeo_ram16,             0x10000/2);
	state_save_register_UINT8 ("neogeo", 0, "neogeo_memcard",          neogeo_memcard,           0x800);
	state_save_register_UINT8 ("neogeo", 0, "gamevector",              gamevector,               0x80);
	state_save_register_int   ("neogeo", 0, "memcard_is_loaded",       &neogeo_memcard_is_loaded);
	state_save_register_int   ("neogeo", 0, "memcard_number",          &memcard_number);
	state_save_register_int   ("neogeo", 0, "neogeo_prot_data",        &neogeo_prot_data);
}
