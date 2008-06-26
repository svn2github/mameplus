/******************************************************************************

    mamedriv.c

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of all available drivers. Drivers have to be included here to be
    recognized by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "driver.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) extern game_driver driver_##NAME;
#include "mameconsoledriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const consoledrivers[] =
{
#include "mameconsoledriv.c"
	0	/* end of array */
};

#else /* DRIVER_RECURSIVE */

/****************CONSOLES****************************************************/

#ifdef MAMEMESS
	/* NINTENDO */
	DRIVER( nes )		/* Nintendo Entertainment System					*/
	DRIVER( nespal )	/* Nintendo Entertainment System					*/
	DRIVER( famicom )
	DRIVER( famitwin )	/* Sharp Famicom Twin System						*/
	DRIVER( gameboy )	/* Nintendo GameBoy Handheld						*/
	DRIVER( supergb )	/* Nintendo Super GameBoy SNES Cartridge			*/
	DRIVER( gbpocket )	/* Nintendo GameBoy Pocket Handheld					*/
	DRIVER( gbcolor )	/* Nintendo GameBoy Color Handheld					*/
	DRIVER( snes )		/* Nintendo Super Nintendo NTSC						*/
	DRIVER( snespal )	/* Nintendo Super Nintendo PAL						*/
	DRIVER( gba )

	DRIVER( megaduck )	/* Megaduck											*/

	/* SEGA */
	DRIVER( gamegear )	/* Sega GameGear									*/
	DRIVER( gamegeaj )	/* Sega GameGear (Japan)							*/
	DRIVER( sms )		/* Sega Master System II (NTSC)						*/
	DRIVER( sms1 )		/* Sega Master System I (NTSC)						*/
	DRIVER( sms1pal )	/* Sega Master System I (PAL)						*/
	DRIVER( smspal )	/* Sega Master System II (PAL)						*/
	DRIVER( smsj )		/* Sega Master System (Japan) with FM Chip			*/
	DRIVER( sg1000m3 )	/* Sega SG-1000 Mark III (Japan)					*/
	DRIVER( sms2kr )	/* Samsung Gam*Boy II (Korea)						*/
	DRIVER( smssdisp )	/* Sega Master System Store Display Unit			*/

	DRIVER( megadrij )	/* 1988 Sega Mega Drive (Japan)						*/
	DRIVER( genesis )	/* 1989 Sega Genesis (USA)							*/
	DRIVER( gensvp )	/* 1993 Sega Genesis (USA w/SVP chip)					*/
	DRIVER( megadriv )	/* 1990 Sega Mega Drive (Europe)					*/
#ifndef NEOCPSMAME	//fixme: compile bug?
	/* NEC */
	DRIVER( pce )		/* PC/Engine NEC 1987-1993							*/
	DRIVER( tg16 )		/* Turbo Grafix-16  NEC 1989-1993					*/
	DRIVER( sgx )		/* SuperGrafX NEC 1989								*/
#endif 
	/* CAPCOM */
	DRIVER( sfzch )		/* CPS Changer (Street Fighter ZERO)				*/

	/* BANDAI */
	DRIVER( wswan )		/* Bandai WonderSwan Handheld						*/
	DRIVER( wscolor )	/* Bandai WonderSwan Color Handheld					*/
#endif /* MAMEMESS */

#endif	/* DRIVER_RECURSIVE */
