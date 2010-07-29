/******************************************************************************

    mamedecrypteddriv.c

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
#include "mamedecrypteddriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
const game_driver * const decrypteddrivers[] =
{
#include "mamedecrypteddriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	/* Neo Geo decrypted */
	DRIVER( zupapad )	/* 0070 Zupapa - released in 2001, 1994 prototype probably exists */
	DRIVER( kof99d )	/* 0251 (c) 1999 SNK */
	DRIVER( ganryud )	/* 0252 (c) 1999 Visco */
	DRIVER( garoud )	/* 0253 (c) 1999 SNK */
	DRIVER( s1945pd )	/* 0254 (c) 1999 Psikyo */
	DRIVER( preisl2d )	/* 0255 (c) 1999 Yumekobo */
	DRIVER( mslug3d )	/* 0256 (c) 2000 SNK */
	DRIVER( kof2000d )	/* 0257 (c) 2000 SNK */
	DRIVER( nitdd )		/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( sengok3d )	/* 0261 (c) 2001 SNK */
	DRIVER( kof2001d )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( mslug4d )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( rotdd )		/* 0264 (c) 2002 Evoga */
	DRIVER( kof2002d )	/* 0265 (c) 2002 Eolith / Playmore */
	DRIVER( matrimd )	/* 0266 (c) 2002 Atlus */
	DRIVER( ct2k3ad )	/* Bootleg */
	DRIVER( cthd2k3d )	/* Bootleg */
	DRIVER( kof10thd )	/* Bootleg */
	DRIVER( kof2003d )	/* 0271 (c) 2003 Playmore */
	DRIVER( mslug5d )	/* 0268 (c) 2003 Playmore */
	DRIVER( samsho5d )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsh5sd )	/* 0272 (c) 2003 Playmore */
	DRIVER( lans2k4d )	/* Bootleg */
	DRIVER( svcd )		/* 0269 (c) 2003 Playmore / Capcom */
	DRIVER( jckeygpd )
	DRIVER( kf2k3pcd )	/* 0271 (c) 2003 Playmore - JAMMA PCB */
	DRIVER( kogd )		/* Bootleg */
	DRIVER( pnyaad )	/* 0267 (c) 2003 Aiky / Taito */

#endif	/* DRIVER_RECURSIVE */
