/****************************************************************************

    Paranoia
    Driver by Mariusz Wojcieszek

    Notes:
    - jamma interface is not emulated, hence the game is marked as 'not working'
    - rom mapping, memory maps and clocks for jamma interface cpus are probably not correct

Paranoia by Naxat Soft 1990

CPU Z84C00A85 (Z80A CPU)

Xtal : 18.000 Mhz

Ram : GM76C28A (Goldstar)

Ram : 2x W2416K-70 (Winbond)

Else :

Winbond WF19054

Sound : Nec D8085AHC + Nec D8155HC

This board has also :

HuC6260A (Hudson)
HuC6270  (Hudson)
HuC6280A (Hudson)
2x HSRM2564LM12
1x HSRM2564LM10

****************************************************************************/

#include "driver.h"
#include "machine/pcecommn.h"
#include "video/vdc.h"
#include "cpu/h6280/h6280.h"
#include "sound/c6280.h"

static INPUT_PORTS_START( paranoia )
    PORT_START_TAG( "IN" )
    PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button I */
    PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button II */
    PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* select */
    PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
    PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
    PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
    PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x000000, 0x03FFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_BASE( &pce_user_ram )
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_READWRITE( vdc_0_r, vdc_0_w )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE( vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_READWRITE( C6280_r, C6280_0_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_READWRITE( H6280_timer_r, H6280_timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE( pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_READWRITE( H6280_irq_status_r, H6280_irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , ADDRESS_SPACE_IO, 8)
	AM_RANGE( 0x00, 0x03) AM_READWRITE( vdc_0_r, vdc_0_w )
ADDRESS_MAP_END

static WRITE8_HANDLER( paranoia_8085_d000_w )
{
	//logerror( "D000 (8085) write %02x\n", data );
}

static WRITE8_HANDLER( paranoia_8085_8155_w )
{
	switch( offset )
	{
	case 0: logerror( "8155 Command register write %x, timer command = %x, interrupt enable = %x, ports = %x\n", data, (data >> 6) & 3, (data >> 4) & 3, data & 0xf ); break;
	case 1: logerror( "8155 I/O Port A write %x\n", data ); break;
	case 2: logerror( "8155 I/O Port B write %x\n", data ); break;
	case 3: logerror( "8155 I/O Port C (or control) write %x\n", data ); break;
	case 4: logerror( "8155 Timer low 8 bits write %x\n", data ); break;
	case 5: logerror( "8155 Timer high 6 bits write %x, timer mode %x\n", data & 0x3f, (data >> 6) & 3); break;
	}
}

ADDRESS_MAP_START(paranoia_8085_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x7fff) AM_ROM
	AM_RANGE( 0x8000, 0x80ff) AM_RAM
	AM_RANGE( 0x8100, 0x8105) AM_WRITE( paranoia_8085_8155_w )
	AM_RANGE( 0xd000, 0xd000) AM_WRITE( paranoia_8085_d000_w )
	AM_RANGE( 0xe000, 0xe1ff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(paranoia_8085_io_map, ADDRESS_SPACE_IO, 8)
ADDRESS_MAP_END

ADDRESS_MAP_START(paranoia_z80_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x3fff) AM_ROM
	AM_RANGE( 0x6000, 0x67ff) AM_RAM
	AM_RANGE( 0x7000, 0x73ff) AM_RAM
ADDRESS_MAP_END

static READ8_HANDLER(paranoia_z80_io_01_r)
{
	return 0;
}

static READ8_HANDLER(paranoia_z80_io_02_r)
{
	return 0;
}

static WRITE8_HANDLER(paranoia_z80_io_17_w)
{
}

static WRITE8_HANDLER(paranoia_z80_io_37_w)
{
}

ADDRESS_MAP_START(paranoia_z80_io_map, ADDRESS_SPACE_IO, 8)
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE( 0x01, 0x01 ) AM_READ( paranoia_z80_io_01_r )
	AM_RANGE( 0x02, 0x02 ) AM_READ( paranoia_z80_io_02_r )
	AM_RANGE( 0x17, 0x17 ) AM_WRITE( paranoia_z80_io_17_w )
	AM_RANGE( 0x37, 0x37 ) AM_WRITE( paranoia_z80_io_37_w )
ADDRESS_MAP_END

static MACHINE_DRIVER_START( paranoia )
	/* basic machine hardware */
	MDRV_CPU_ADD(H6280, PCE_MAIN_CLOCK/3)
	MDRV_CPU_PROGRAM_MAP(pce_mem, 0)
	MDRV_CPU_IO_MAP(pce_io, 0)
	MDRV_CPU_VBLANK_INT(pce_interrupt, VDC_LPF)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(1)

	MDRV_CPU_ADD(8085A, 18000000/6)
	MDRV_CPU_PROGRAM_MAP(paranoia_8085_map,0)
	MDRV_CPU_IO_MAP(paranoia_8085_io_map,0)

	MDRV_CPU_ADD(Z80, 18000000/6)
	MDRV_CPU_PROGRAM_MAP(paranoia_z80_map,0)
	MDRV_CPU_IO_MAP(paranoia_z80_io_map,0)

    /* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_ADD("main",0)
	MDRV_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)
	/* MDRV_GFXDECODE( pce_gfxdecodeinfo ) */
	MDRV_PALETTE_LENGTH(1024)
	MDRV_PALETTE_INIT( vce )
	MDRV_COLORTABLE_LENGTH(1024)

	MDRV_VIDEO_START( pce )
	MDRV_VIDEO_UPDATE( pce )

	MDRV_SPEAKER_STANDARD_STEREO("left","right")
	MDRV_SOUND_ADD(C6280, PCE_MAIN_CLOCK/6)
	MDRV_SOUND_ROUTE(0, "left", 1.00)
	MDRV_SOUND_ROUTE(1, "right", 1.00)

MACHINE_DRIVER_END

ROM_START(paranoia)
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD( "5.201", 0x00000, 0x40000, CRC(9893e0e6) SHA1(b3097e7f163e4a067cf32f290e59657a8b5e271b) )

	ROM_REGION( 0x8000, REGION_CPU2, 0 )
	ROM_LOAD( "6.29", 0x0000, 0x8000, CRC(5517532e) SHA1(df8f1621abf1f0c65d86d406cd79d97ec233c378) )

	ROM_REGION( 0x20000, REGION_CPU3, 0 )
	ROM_LOAD( "1.319", 0x00000, 0x8000, CRC(ef9f85d8) SHA1(951239042b56cd256daf1965ead2949e2bddcd8b) )
	ROM_LOAD( "2.318", 0x08000, 0x8000, CRC(a35fccca) SHA1(d50e9044a97fe77f31e3198bb6759ba451359069) )
	ROM_LOAD( "3.317", 0x10000, 0x8000, CRC(e3e48ec1) SHA1(299820d0e4fb2fd947c7a52f1c49e2e4d0dd050a) )
	ROM_LOAD( "4.352", 0x18000, 0x8000, CRC(11297fed) SHA1(17a294e65ba1c4806307602dee4c7c627ad1fcfd) )
ROM_END

static DRIVER_INIT(paranoia)
{
	driver_init_pce(machine);
};

GAME( 1990, paranoia, 0, paranoia, paranoia, paranoia, ROT0, "Naxat Soft", "Paranoia", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )