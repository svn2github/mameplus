/******************************************************************************

    mamedriv.c

    Copyright Nicola Salmoria and the MAME Team.
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
#include "mamehbdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const homebrewdrivers[] =
{
#include "mamehbdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

#ifndef NEOCPSPGM
	/* Homebrew */
	DRIVER( vantris )	/* http://web.utanet.at/nkehrer */
#endif /* NEOCPSPGM */

	/* Neo Geo homebrew */
	DRIVER( frogfest )	/* 0202 (c) 2005 Rastersoft */
	DRIVER( columnsn )
	DRIVER( poknight )
	DRIVER( neodemo )
	DRIVER( neo2500 )
	DRIVER( syscheck )	/* xxxx (c) 2004 Blastar */
	DRIVER( neonopon )	/* xxxx (c) 2002 Blastar */
	DRIVER( neopong )	/* xxxx (c) 2002 Neodev */
	DRIVER( neoponga )	/* xxxx (c) 2002 Neodev */
	DRIVER( ngem2k )
	DRIVER( cnbe )
	DRIVER( ltorb1 )
	DRIVER( beast )

#endif	/* DRIVER_RECURSIVE */
