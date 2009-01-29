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
#include "mameplusdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const plusdrivers[] =
{
#include "mameplusdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	/* Capcom CPS1 bootleg */
	DRIVER( knightsb )	/* bootleg */
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
	DRIVER( daimakb )	/* bootleg */
	DRIVER( sf2m8 )		/* bootleg */
	DRIVER( sf2m13 )	/* bootleg */
	DRIVER( sf2tlona )	/* bootleg, Tu Long set 1 */
	DRIVER( sf2tlonb )	/* bootleg, Tu Long set 2 */
	DRIVER( sf2th )		/* bootleg */
	DRIVER( sf2b )		/* bootleg */
//	DRIVER( dinohc )	/* bootleg */
//	DRIVER( punishrh )	/* bootleg, Biaofeng Zhanjing */

	/* Capcom CPS1 hack */
	DRIVER( knightsh )	/* hack */
	DRIVER( kodh )		/* hack */
	DRIVER( dinoh )		/* hack */
	DRIVER( dinoha )	/* hack */
	DRIVER( dinohb )	/* hack */

	/* Capcom CPS2 Phoenix Edition */
	DRIVER( mmatrixd )		/* bootleg */
	DRIVER( dimahoud )		/* bootleg */
	DRIVER( armwar1d )		/* bootleg */

	/* Neo Geo bootleg */
	DRIVER( kof96ep )	/* 0214 bootleg */
	DRIVER( kof97pla )	/* 0232 (c) 2003 bootleg */
	DRIVER( kf2k1pls )	/* 0262 (c) 2001 bootleg */
	DRIVER( kf2k1pa )	/* 0262 (c) 2001 bootleg */
	DRIVER( cthd2k3a )	/* bootleg of kof2001*/
	DRIVER( kf2k2plb )	/* bootleg */
	DRIVER( kf2k2plc )	/* bootleg */
	DRIVER( kf2k4pls )	/* bootleg of kof2002 */
	DRIVER( mslug5b )	/* 0268 (c) 2003 bootleg */

	/* Neo Geo prototype */
	DRIVER( bangbedp )	/* 0259 prototype */

	/* CD to MVS Conversion */
	DRIVER( zintrkcd )	/* 0211 hack - CD to MVS Conversion by Razoola */
	DRIVER( fr2ch )		/* hack - CD to MVS Conversion */

#ifndef NEOCPSPGM
	/* Konami "Nemesis hardware" games */
	DRIVER( spclone )	/* GX587 (c) 1986 based */
#endif /* !NEOCPSPGM */

#endif	/* DRIVER_RECURSIVE */
