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

#else	/* DRIVER_RECURSIVE */

#if 1//ndef NEOCPSMAME
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

	DRIVER( megaduck )	/* Megaduck											*/

	/* SEGA */
	DRIVER( megadrij )	/* 1988 Sega Mega Drive (Japan)						*/
	DRIVER( genesis )	/* 1989 Sega Genesis (USA)							*/
	DRIVER( megadriv )	/* 1990 Sega Mega Drive (Europe)					*/

	/* NEC */
	DRIVER( pce )		/* PC/Engine NEC 1987-1993							*/
	DRIVER( tg16 )		/* Turbo Grafix-16  NEC 1989-1993					*/
	DRIVER( sgx )		/* SuperGrafX NEC 1989								*/

	/* CAPCOM */
	DRIVER( sfzch )		/* CPS Changer (Street Fighter ZERO)				*/

	/* BANDAI */
	DRIVER( wswan )		/* Bandai WonderSwan Handheld						*/
	DRIVER( wscolor )	/* Bandai WonderSwan Color Handheld					*/

#endif /* NEOCPSMAME */

#endif	/* DRIVER_RECURSIVE */
