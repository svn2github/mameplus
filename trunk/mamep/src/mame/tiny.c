/******************************************************************************

    tiny.c

    mamedriv.c substitute file for "tiny" MAME builds.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of used drivers. Drivers have to be included here to be recognized
    by the executable.

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
#include "tiny.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
#ifndef DRIVER_SWITCH
const game_driver * const drivers[] =
#else /* DRIVER_SWITCH */
const game_driver ** drivers = NULL;
const game_driver * const mamedrivers[] =
#endif /* DRIVER_SWITCH */
{
#include "tiny.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	DRIVER( robby )		/* (c) 1981 Bally Midway */
	DRIVER( gridlee )	/* [1983 Videa] prototype - no copyright notice */
	DRIVER( alienar )	/* (c) 1985 Duncan Brown */
	DRIVER( carpolo )	/* (c) 1977 Exidy */
	DRIVER( sidetrac )	/* (c) 1979 Exidy */
	DRIVER( targ )		/* (c) 1980 Exidy */
	DRIVER( spectar )	/* (c) 1980 Exidy */
	DRIVER( teetert )	/* (c) 1982 Exidy */
	DRIVER( circus )	/* (c) 1977 Exidy */
	DRIVER( robotbwl )	/* (c) 197? Exidy */
	DRIVER( crash )		/* (c) 1979 Exidy */
	DRIVER( ripcord )	/* (c) 1979 Exidy */
	DRIVER( starfire )	/* (c) 1979 Exidy */
	DRIVER( starfirea )	/* (c) 1979 Exidy */
	DRIVER( fireone )	/* (c) 1979 Exidy */
	DRIVER( starfir2 )	/* (c) 1979 Exidy */
	DRIVER( wrally )	/* (c) 1993 - Ref 930705 */


	/* Cave games */
	/* Cave was formed in 1994 from the ruins of Toaplan, like Raizing was. */
	DRIVER( pwrinst2 )	/* (c) 1994 Atlus */
	DRIVER( pwrinst2j )	/* (c) 1994 Atlus */
	DRIVER( plegends )	/* (c) 1994 Atlus */
	DRIVER( plegendsj )	/* (c) 1994 Atlus */
	DRIVER( mazinger )	/* (c) 1994 Banpresto (country is in EEPROM) */
	DRIVER( mazingerj )	/* (c) 1994 Banpresto (country is in EEPROM) */
	DRIVER( donpachi )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachij )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachikr )	/* (c) 1995 Atlus/Cave */
	DRIVER( donpachihk )	/* (c) 1995 Atlus/Cave */
	DRIVER( metmqstr )	/* (c) 1995 Banpresto / Pandorabox */
	DRIVER( nmaster )	/* (c) 1995 Banpresto / Pandorabox */
	DRIVER( sailormn )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnu )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnj )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnk )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnt )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnh )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormno )	/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnou )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnoj )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnok )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnot )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( sailormnoh )/* (c) 1995 Banpresto (country is in EEPROM) */
	DRIVER( agallet )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletu )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletj )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalletk )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agallett )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( agalleth )	/* (c) 1996 Banpresto / Gazelle (country is in EEPROM) */
	DRIVER( hotdogst )	/* (c) 1996 Marble */
	DRIVER( ddonpach )	/* (c) 1997 Atlus/Cave */
	DRIVER( ddonpachj )	/* (c) 1997 Atlus/Cave */
	DRIVER( dfeveron )	/* (c) 1998 Cave + Nihon System license */
	DRIVER( feversos )	/* (c) 1998 Cave + Nihon System license */
	DRIVER( esprade )	/* (c) 1998 Atlus/Cave */
	DRIVER( espradej )	/* (c) 1998 Atlus/Cave (Japan) */
	DRIVER( espradejo )	/* (c) 1998 Atlus/Cave (Japan) */
	DRIVER( uopoko )	/* (c) 1998 Cave + Jaleco license */
	DRIVER( uopokoj )	/* (c) 1998 Cave + Jaleco license */
	DRIVER( guwange )	/* (c) 1999 Atlus/Cave */
	DRIVER( gaia )		/* (c) 1999 Noise Factory */
	DRIVER( theroes )	/* (c) 2001 Primetek Investments */
	DRIVER( korokoro )	/* (c) 1999 Takumi */
	DRIVER( crusherm )	/* (c) 1999 Takumi */
	DRIVER( tjumpman )	/* (c) 1999 Namco */

	/* Psikyo games */
	DRIVER( samuraia )	/* (c) 1993 (World) */
	DRIVER( sngkace )	/* (c) 1993 (Japan) */
	DRIVER( gunbird )	/* (c) 1994 */
	DRIVER( gunbirdk )	/* (c) 1994 */
	DRIVER( gunbirdj )	/* (c) 1994 */
	DRIVER( btlkroad )	/* (c) 1994 */
	DRIVER( s1945 )		/* (c) 1995 */
	DRIVER( s1945a )	/* (c) 1995 */
	DRIVER( s1945j )	/* (c) 1995 */
	DRIVER( s1945jn )	/* (c) 1995 */
	DRIVER( s1945bl )	/* (c) 1995 (Hong Kong bootleg) */
	DRIVER( s1945k )	/* (c) 1995 */
	DRIVER( tengai )	/* (c) 1996 */
	DRIVER( tengaij )	/* (c) 1996 */
	DRIVER( s1945ii )	/* (c) 1997 */
	DRIVER( soldivid )	/* (c) 1997 */
	DRIVER( sbomberb )	/* (c) 1998 */
	DRIVER( daraku )	/* (c) 1998 */
	DRIVER( gunbird2 )	/* (c) 1998 */
	DRIVER( s1945iii )	/* (c) 1999 */
	DRIVER( dragnblz )	/* (c) 2000 */
	DRIVER( tgm2 )		/* (c) 2000 */
	DRIVER( tgm2p )		/* (c) 2000 */
	DRIVER( gnbarich )	/* (c) 2001 */
	DRIVER( mjgtaste )	/* (c) 2002 */

#endif	/* DRIVER_RECURSIVE */
