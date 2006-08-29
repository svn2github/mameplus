/******************************************************************************

    mamedriv.c

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
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
#include "plusdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const plusdrivers[] =
{
#include "plusdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

#ifndef NEOCPSMAME
	DRIVER( monaco )	/* (c) 1979 SEGA */
#endif /* NEOCPSMAME */

	DRIVER( knightsh )	/* hack */
	DRIVER( knightsb )	/* bootleg */
	DRIVER( sf2m8 )		/* hack */
	DRIVER( sf2m13 )	/* hack */
	DRIVER( sf2ceh )	/* hack, Hispanic 990804 */
	DRIVER( sf2tlona )	/* hack, Tu Long set 1 */
	DRIVER( sf2tlonb )	/* hack, Tu Long set 2 */
	DRIVER( wofb )		/* bootleg */
	DRIVER( wofsj )		/* 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wofsja )	/* 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wofsjb )	/* 1995  Holy Sword Three Kingdoms / Sheng Jian San Guo */
	DRIVER( wof3js )	/* 1997  Three Sword Masters / San Jian Sheng */
	DRIVER( wof3sj )	/* 1997  Three Holy Swords / San Sheng Jian */
	DRIVER( wof3sja )	/* 1997  Three Holy Swords / San Sheng Jian */
	DRIVER( wofh ) 		/* 1999  Legend of Three Kingdoms' Heroes / Sanguo Yingxiong Zhuan */
	DRIVER( wofha ) 	/* 1999  Legend of Three Kingdoms' Heroes / Sanguo Yingxiong Zhuan */
	DRIVER( dinob )		/* bootleg */
	DRIVER( dinoh )		/* hack */
	DRIVER( dinoha )	/* hack */
	DRIVER( dinohb )	/* hack */
	DRIVER( pnicku )
	DRIVER( sfzch )		/* 1995  Street Fighter Zero (CPS Changer) */
	DRIVER( sfach )		/* 1995  Street Fighter Alpha (Publicity CPS Changer) */

#ifndef NEOCPSMAME
	DRIVER( vantris )	/* (c) 1998 Norbert Kehrer */
#endif /* NEOCPSMAME */

	DRIVER( kof96ep )	/* 0214 bootleg */
	DRIVER( kof97pla )
	DRIVER( kf2k1pls )	/* 0262 (c) 2001 bootleg */
	DRIVER( kf2k1pa )	/* 0262 (c) 2001 bootleg */
	DRIVER( cthd2k3a )	/* bootleg of kof2001*/
	DRIVER( kof2002b )	/* 0265 (c) 2002 bootleg */
	DRIVER( kf2k2plb )	/* bootleg */
	DRIVER( kf2k2plc )	/* bootleg */
	DRIVER( kf2k4pls )	/* bootleg of kof2002 */
	DRIVER( matrimbl )	/* 0266 (c) 2002 bootleg */
	DRIVER( mslug5b )

	/* CD to MVS Conversion */
	DRIVER( zintrkcd )	/* 0211 hack - CD to MVS Conversion by Razoola */

	/* homebrew */
	DRIVER( frogfest )	/* 0202 (c) 2005 Rastersoft */
	DRIVER( columnsn )
	DRIVER( poknight )
	DRIVER( neodemo )
	DRIVER( neo2500 )
	DRIVER( syscheck )	/* FFFF (c) 2004 Blastar */
	DRIVER( neonopon )	/* xxxx (c) 2002 Blastar */
	DRIVER( neopong )	/* xxxx (c) 2002 Neodev */
	DRIVER( npong10 )	/* xxxx (c) 2002 Neodev */
	DRIVER( ngem2k )
	DRIVER( cnbe )
	DRIVER( ltorb1 )
	DRIVER( beast )


#ifdef USE_NEOGEO_DEPRECATED
	/* CD to MVS Conversion */
	DRIVER( fr2ch )
	DRIVER( sthoopcd )
	/* decrypted junk */
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
	DRIVER( svcd )	/* 0269 (c) 2003 Playmore / Capcom */
	DRIVER( jckeygpd )
	DRIVER( kf2k3pcd )	/* 0271 (c) 2003 Playmore - JAMMA PCB */
	DRIVER( kogd )	/* Bootleg */
	DRIVER( pnyaad )	/* 0267 (c) 2003 Aiky / Taito */
	DRIVER( bangbedp )	/* 0259 (c) 2000 Visco */
#endif /* USE_NEOGEO_DEPRECATED */

#endif	/* DRIVER_RECURSIVE */
