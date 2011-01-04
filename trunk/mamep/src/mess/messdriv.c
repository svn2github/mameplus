/******************************************************************************

  messdriv.c

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
#define DRIVER(NAME) extern const game_driver driver_##NAME;
#include "messdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const consoledrivers[] =
{
#include "messdriv.c"
  0             /* end of array */
};

#else /* DRIVER_RECURSIVE */

/****************CONSOLES****************************************************/


#ifdef MAMEMESS
	/* Atari */
	DRIVER( a2600 )	 /* Atari 2600	   */
	DRIVER( a2600p )	/* Atari 2600 PAL	   */
	DRIVER( a5200 )	 /* Atari 5200	   */
	DRIVER( a7800 )	 /* Atari 7800 NTSC	  */
	DRIVER( a7800p )	/* Atari 7800 PAL	   */

	/* Nintendo */
	DRIVER( nes )       /* Nintendo Entertainment System */
	DRIVER( nespal )	/* Nintendo Entertainment System PAL */
	DRIVER( m82 )       /* Nintendo M82 Display Unit */
	DRIVER( famicom )   /* Nintendo Family Computer (a.k.a. Famicom) + Disk System add-on */
	DRIVER( fami_key )  /* Nintendo Family Computer (a.k.a. Famicom) + Disk System add-on + Keyboard add-on */
	DRIVER( famitwin )  /* Sharp Famicom Twin System */
	DRIVER( drpcjr )    /* Bung Doctor PC Jr */
	DRIVER( dendy )     /* Dendy (Classic russian famiclone) */
	DRIVER( gameboy )   /* Nintendo Game Boy Handheld */
	DRIVER( supergb )   /* Nintendo Super Game Boy SNES Cartridge */
	DRIVER( gbpocket )  /* Nintendo Game Boy Pocket Handheld */
	DRIVER( gblight )   /* Nintendo Game Boy Light Handheld */
	DRIVER( gbcolor )   /* Nintendo Game Boy Color Handheld */
	DRIVER( gba )       /* Nintendo Game Boy Advance Handheld */
	DRIVER( snes )      /* Nintendo Super Nintendo NTSC */
	DRIVER( snespal )   /* Nintendo Super Nintendo PAL */
	DRIVER( snessfx )   /* Nintendo Super Nintendo NTSC w/SuperFX CPU*/
	DRIVER( snespsfx )  /* Nintendo Super Nintendo PAL w/SuperFX CPU */
	DRIVER( snesdsp )   /* Nintendo Super Nintendo NTSC w/DSP-x CPU*/
	DRIVER( snespdsp )  /* Nintendo Super Nintendo PAL w/DSP-x CPU */
	DRIVER( snesst )    /* Nintendo Super Nintendo NTSC w/Sufami Turbo base cart */
	DRIVER( snesbsx )   /* Nintendo Super Nintendo NTSC w/BS-X compatible cart  */

	DRIVER( megaduck )  /* Megaduck	 */

	/* SEGA */
	DRIVER( gamegear )  /* Sega GameGear	*/
	DRIVER( gamegeaj )  /* Sega GameGear (Japan)	*/
	DRIVER( sms )	   /* Sega Master System II (NTSC)	 */
	DRIVER( sms1 )	  /* Sega Master System I (NTSC)	  */
	DRIVER( sms1pal )   /* Sega Master System I (PAL)	   */
	DRIVER( smspal )	/* Sega Master System II (PAL)	  */
	DRIVER( smsj )	  /* Sega Master System (Japan) with FM Chip	  */
	DRIVER( sg1000m3 )  /* Sega SG-1000 Mark III (Japan)	*/
	DRIVER( sms2kr )	/* Samsung Gam*Boy II (Korea)	   */
	DRIVER( smssdisp )  /* Sega Master System Store Display Unit	*/

	DRIVER( megadrij )  /* 1988 Sega Mega Drive (Japan)	 */
	DRIVER( genesis )   /* 1989 Sega Genesis (USA)	  */
	DRIVER( gensvp )	/* 1993 Sega Genesis (USA w/SVP chip)	   */
	DRIVER( megadriv )  /* 1990 Sega Mega Drive (Europe)	*/
	DRIVER( pico )	  /* 1994 Sega Pico (Europe)	  */
	DRIVER( picou )	 /* 1994 Sega Pico (USA)	 */
	DRIVER( picoj )	 /* 1993 Sega Pico (Japan)	   */

	/* NEC */
	DRIVER( pce )	   /* PC/Engine NEC 1987-1993	  */
	DRIVER( tg16 )	  /* Turbo Grafix-16  NEC 1989-1993	   */
	DRIVER( sgx )	   /* SuperGrafX NEC 1989	  */

	/* Capcom */
	DRIVER( sfach )	 /* CPS Changer (Street Fighter Alpha)	   */
	DRIVER( sfzbch )	/* CPS Changer (Street Fighter ZERO Brazil)	 */
	DRIVER( sfzch )	 /* CPS Changer (Street Fighter ZERO)	*/
	DRIVER( wofch )	 /* CPS Changer (Tenchi Wo Kurau II)	 */

	/* BANDAI */
	DRIVER( wswan )	 /* Bandai WonderSwan Handheld	   */
	DRIVER( wscolor )   /* Bandai WonderSwan Color Handheld	 */

	/* SNK */
	DRIVER( ngp )	   /* NeoGeo Pocket	*/
	DRIVER( ngpc )	  /* NeoGeo Pocket Color	  */

/****************COMPUTERS***************************************************/

    /* ASCII & Microsoft */
	DRIVER( msx )	   /* 1983 MSX	 */
	DRIVER( msx2 )	  /* 1985 MSX2	*/
	DRIVER( msx2p )	 /* 1988 MSX2+ Japan	 */

/*********** To sort (mostly skeleton drivers) ******************************/

	DRIVER( supracan )

#endif /* MAMEMESS */
#endif /* DRIVER_RECURSIVE */
