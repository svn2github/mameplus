/******************************************************************************

    mameplusdriv.c

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
#include "mameplusdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
const game_driver * const plusdrivers[] =
{
#include "mameplusdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	/* Capcom CPS1 bootleg */
	DRIVER( knightsb2 )	/* bootleg */
	DRIVER( wofb )		/* bootleg */
	DRIVER( wofsj )		/* bootleg, 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wofsja )	/* bootleg, 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wofsjb )	/* bootleg, 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wof3js )	/* bootleg, 1997  Three Sword Masters / San Jian Sheng */
	DRIVER( wof3sj )	/* bootleg, 1997  Three Holy Swords / San Sheng Jian */
	DRIVER( wof3sja )	/* bootleg, 1997  Three Holy Swords / San Sheng Jian */
	DRIVER( wofh ) 		/* bootleg, 1999  Legend of Three Kingdoms' Heroes / Sanguo Yingxiong Zhuan */
	DRIVER( wofha ) 	/* bootleg, 1999  Legend of Three Kingdoms' Heroes / Sanguo Yingxiong Zhuan */
	DRIVER( cawingb )	/* bootleg */
	DRIVER( sf2m8 )		/* bootleg */
	DRIVER( sf2m9 )		/* bootleg */
	DRIVER( sf2m10 )	/* bootleg */
	DRIVER( sf2m11 )	/* bootleg */
	DRIVER( sf2m12 )	/* bootleg */
	DRIVER( sf2m13 )	/* bootleg */
	DRIVER( sf2tlona )	/* bootleg, Tu Long set 1 */
	DRIVER( sf2tlonb )	/* bootleg, Tu Long set 2 */

	/* Capcom CPS1 hack */
	DRIVER( knightsh )	/* hack */
	DRIVER( kodh )		/* hack */
	DRIVER( dinoh )		/* hack */
	DRIVER( dinoha )	/* hack */
	DRIVER( dinohb )	/* hack */

	/* Neo Geo bootleg */
	DRIVER( kof96ep )	/* 0214 (c) 1996 bootleg */
	DRIVER( kof97pla )	/* 0232 (c) 2003 bootleg */
	DRIVER( kf2k1pls )	/* 0262 (c) 2001 bootleg */
	DRIVER( kf2k1pa )	/* 0262 (c) 2001 bootleg */
	DRIVER( cthd2k3a )	/* bootleg of kof2001*/
	DRIVER( kf2k2plb )	/* 0265 (c) 2002 bootleg */
	DRIVER( kf2k2plc )	/* 0265 (c) 2002 bootleg */
	DRIVER( kf2k4pls )	/* bootleg of kof2002 */
	DRIVER( mslug5b )	/* 0268 (c) 2003 bootleg */

	/* Neo Geo prototype */
	DRIVER( bangbedp )	/* 0259 prototype */

	/* CD to MVS Conversion */
	DRIVER( zintrkcd )	/* 0211 hack - CD to MVS Conversion by Razoola */
	DRIVER( fr2ch )		/* 0098 hack - CD to MVS Conversion */

	/* Wii Virtual Console to MVS Conversion */
	DRIVER( ironclad )	/* 0220 (c) 1996 Saurus - Wii Virtual Console to MVS Conversion */

	/* IGS PGM System Games */
	DRIVER( kovqhsgs )	/* (c) 2008 */
	DRIVER( kovlsjb )	/* (c) 2009 */
	DRIVER( kovlsjba )	/* (c) 2009 */
	DRIVER( kovlsqh2 )	/* (c) 2009 */

#ifndef NCP
	/* Konami "Nemesis hardware" games */
	DRIVER( spclone )	/* GX587 (c) 1986 based */
#endif /* !NCP */

#endif	/* DRIVER_RECURSIVE */
