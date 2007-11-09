/***************************************************************************
	pong.c
	Machine driver

	J. Buchmueller, November '99
***************************************************************************/

#include "driver.h"
#include "sndintrf.h"
#include "video/pong.h"

extern struct SamplesSound_interface pong_samples_interface;

static INPUT_PORTS_START( pong )
	PORT_START		/* IN0 buttons */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x00, "Ending Score" )
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW,	IPT_UNUSED )

	PORT_START		/* IN1 control 1 */
	PORT_BIT( 0x1ff, (PONG_MAX_V-15)/2, IPT_AD_STICK_Y) PORT_MINMAX(PONG_VBLANK - 12, 255) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START		/* IN2 control 2 */
	PORT_BIT( 0x1ff, (PONG_MAX_V-15)/2, IPT_AD_STICK_Y) PORT_MINMAX(PONG_VBLANK - 12, 255) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)
INPUT_PORTS_END

static unsigned short colortable_source[] =
{
	0,1,
	1,0,
	2,0,
};

#define PONG_CLOCK	PONG_FPS * PONG_MAX_V * PONG_MAX_H + 49410   /* Pong uses a standard US TV crystal with 14,318,100 Hz */

static int pong_video[] = {
    PONG_MAX_H, PONG_MAX_V,
    0, PONG_HSYNC0, PONG_HSYNC1, PONG_HBLANK,
    0, PONG_VSYNC0, PONG_VSYNC1, PONG_VBLANK
};

static PALETTE_INIT( pong )
{
	palette_set_color_rgb(machine, 0,0x00,0x00,0x00); /* black */
	palette_set_color_rgb(machine, 1,0xff,0xff,0xff); /* white (1k resistor) */
	palette_set_color_rgb(machine, 2,0xd4,0xd4,0xd4); /* slightly darker white (1.2k resistor) */
	memcpy(colortable,colortable_source,sizeof(colortable_source));
}

static MACHINE_DRIVER_START( pong )

	/* basic machine hardware */
	MDRV_CPU_ADD(GENSYNC, PONG_CLOCK)
	MDRV_CPU_CONFIG(pong_video)
	MDRV_CPU_VBLANK_INT(pong_vh_scanline, PONG_MAX_V)

	MDRV_SCREEN_REFRESH_RATE(PONG_FPS)
	MDRV_SCREEN_VBLANK_TIME(0)  /* frames per second, vblank duration */

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(PONG_MAX_H, PONG_MAX_V)
	MDRV_SCREEN_VISIBLE_AREA(PONG_HBLANK, PONG_MAX_H-1, PONG_VBLANK, PONG_MAX_V-1)
	MDRV_PALETTE_LENGTH(3)
	MDRV_COLORTABLE_LENGTH(sizeof(colortable_source)/sizeof(colortable_source[0]))

	MDRV_PALETTE_INIT(pong)
	MDRV_VIDEO_START(pong)
	MDRV_VIDEO_UPDATE(pong)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(pong_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

#define rom_pong 0	/* this game has no ROMs */

GAME( 1972, pong, 0, pong, pong, 0, ROT0, "Atari", "Pong" ,0 )
