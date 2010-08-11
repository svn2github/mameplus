/******************************************************************************

    mamehbdriv.c

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

    The list of all available drivers. Drivers have to be included here to be
    recognized by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "emu.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) GAME_EXTERN(NAME);
#include "mamehbdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
const game_driver * const homebrewdrivers[] =
{
#include "mamehbdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

#ifndef NCP
	/* Homebrew */
	DRIVER( vantris )	/* http://web.utanet.at/nkehrer */
#endif /* !NCP */

	/* Neo Geo homebrew */
	DRIVER( frogfest )	/* (c) 2005 Rastersoft */
	DRIVER( poknight )	/* http://www.neobitz.com */
	DRIVER( neodemo )	/* (c) 2002 Chaos */
	DRIVER( neo2500 )	/* (c) 2004 Blastar */
	DRIVER( syscheck )	/* (c) 200? Blastar */
	DRIVER( neonopon )	/* (c) 2002 Blastar */
	DRIVER( neopong )	/* (c) 2002 Neodev */
	DRIVER( neoponga )	/* (c) 2002 Neodev */
	DRIVER( ngem2k )	/* (c) 2006 Blastar */
	DRIVER( cnbe )	/* (c) 2006 Blastar */
	DRIVER( ltorb1 )	/* (c) 2005 Blastar */
	DRIVER( beast )	/* http://www.neobitz.com */

	/* Neo Geo hack */
	DRIVER( kof96cn )
	DRIVER( kof96ae )	/* 2007 EGHT hack */
	DRIVER( kof97cn )	/* 2007 EGHT hack */
	DRIVER( kof97xt )
	DRIVER( kof98ae )	/* 2009 EGHT hack */
	DRIVER( kf2k2ps2 )	/* 2007 EGHT hack */
	DRIVER( columnsh )	/* http://www.neobitz.com */

#endif	/* DRIVER_RECURSIVE */
