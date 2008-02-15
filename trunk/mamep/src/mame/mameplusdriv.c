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

	DRIVER( knightsh )	/* hack */
	DRIVER( knightsb )	/* bootleg */
	DRIVER( sf2m8 )		/* hack */
	DRIVER( sf2m13 )	/* hack */
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
	DRIVER( wofhfh ) 	/* 1999  Fire Phoenix / Huo Fenghuang */
	DRIVER( dinob )		/* bootleg */
	DRIVER( dinoh )		/* hack */
	DRIVER( dinoha )	/* hack */
	DRIVER( dinohb )	/* hack */
	DRIVER( cawingb )	/* bootleg */
	DRIVER( kodh )		/* bootleg */
	DRIVER( ddtodd )	/* 12/04/1994 (c) 1993 (Euro) Phoenix Edition */
	DRIVER( avspd )		/* 20/05/1994 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( ringdstd )	/* 02/09/1994 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( xmcotad )	/* 05/01/1995 (c) 1994 (Euro) Phoenix Edition */
	DRIVER( nwarrud )	/* 06/04/1995 (c) 1995 (US) Phoenix Edition */
	DRIVER( sfad )		/* 27/07/1995 (c) 1995 (Euro) Phoenix Edition */
	DRIVER( 19xxd )		/* 07/12/1995 (c) 1996 (US) Phoenix Edition */
	DRIVER( ddsomud )	/* 19/06/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( spf2xjd )	/* 31/05/1996 (c) 1996 (Japan) Phoenix Edition */
	DRIVER( megamn2d )	/* 08/07/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( sfz2aad )	/* 26/08/1996 (c) 1996 (Asia) Phoenix Edition */
	DRIVER( xmvsfu1d )	/* 04/10/1996 (c) 1996 (US) Phoenix Edition */
	DRIVER( batcird )	/* 19/03/1997 (c) 1997 (Euro) Phoenix Edition */
	DRIVER( vsavd )		/* 19/05/1997 (c) 1997 (Euro) Phoenix Edition */
	DRIVER( mvscud )	/* 23/01/1998 (c) 1998 (US) Phoenix Edition */
	DRIVER( sfa3ud )	/* 04/09/1998 (c) 1998 (US) Phoenix Edition */
	DRIVER( gwingjd )	/* 23/02/1999 (c) 1999 Takumi (Japan) Phoenix Edition */
	DRIVER( 1944d )		/* 20/06/2000 (c) 2000 Eighting/Raizing (US) Phoenix Edition */
	DRIVER( hsf2d )		/* 02/02/2004 (c) 2004 (Asia) Phoenix Edition */
	DRIVER( dstlku1d )	/* 05/07/1994 (c) 1994 (Phoenix Edition, US 940705) */
	DRIVER( progerjd )	/* 17/01/2001 (c) 2001 Cave (Phoenix Edition, Japan) */
	DRIVER( ssf2ud )	/* 11/09/1993 (c) 1993 (Phoenix Edition, US) */
	DRIVER( mshud )		/* 24/10/1995 (c) 1995 (Phoenix Edition, US) */
	DRIVER( sfz2ad )	/* 27/02/1996 (c) 1996 (Phoenix Edition, ASIA) */

	DRIVER( kof96ep )	/* 0214 bootleg */
	DRIVER( kof97pla )	/* 0232 (c) 2003 bootleg */
	DRIVER( kf2k1pls )	/* 0262 (c) 2001 bootleg */
	DRIVER( kf2k1pa )	/* 0262 (c) 2001 bootleg */
	DRIVER( cthd2k3a )	/* bootleg of kof2001*/
	DRIVER( kof2002b )	/* 0265 (c) 2002 bootleg */
	DRIVER( kf2k2plb )	/* bootleg */
	DRIVER( kf2k2plc )	/* bootleg */
	DRIVER( kf2k4pls )	/* bootleg of kof2002 */
	DRIVER( matrimbl )	/* 0266 (c) 2002 bootleg */
	DRIVER( mslug5b )	/* 0268 (c) 2003 bootleg */
	DRIVER( rbff1a )    /* 095  (c) 1995 */
	DRIVER( mslug5h )   /* 0268 (c) 2003 */
	DRIVER( kof2003h )	/* 0271 (c) 2003 */

	/* CD to MVS Conversion */
	DRIVER( zintrkcd )	/* 0211 hack - CD to MVS Conversion by Razoola */
	DRIVER( fr2ch )

#ifndef NEOCPSMAME
#if 1 /* SPCLONE */
	/* Konami "Nemesis hardware" games */
	DRIVER( spclone )	/* GX587 (c) 1986 based */
#endif /* SPCLONE */
	/* Kaneko 16 Bit Game  */
	DRIVER( shogware )	/* 1992 (c) Shogun Warriors (Europe Rev.xx)(Kaneko 1992) */
#endif /* NEOCPSMAME */

#endif	/* DRIVER_RECURSIVE */
