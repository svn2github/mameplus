#include "driver.h"
#include "megadriv.h"


READ16_HANDLER ( topshoot_ram_r )
{
	UINT16 *topshoot_ram = (UINT16*)memory_region(REGION_CPU1)+(0x200000/2);

	return topshoot_ram[offset];
}

WRITE16_HANDLER( topshoot_ram_w )
{
	UINT16 *topshoot_ram = (UINT16*)memory_region(REGION_CPU1)+(0x200000/2);

	topshoot_ram[offset] = data;
}

READ16_HANDLER( topshoot_unk_r )
{
	megadrive_ram[0x4004>>1] = 0x20; // coins or bet? .. maybe the MCU puts the inputs direct in RAM.
	return mame_rand(Machine);
}

DRIVER_INIT( topshoot )
{
//	UINT8 *ROM = memory_region(REGION_CPU1);
//	memcpy(ROM + 0x00000, ROM + 0x400000, 0x400000); /* default rom */
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x200000, 0x201fff, 0, 0, topshoot_ram_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x200000, 0x201fff, 0, 0, topshoot_ram_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x202000, 0x203fff, 0, 0, topshoot_ram_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x202000, 0x203fff, 0, 0, topshoot_ram_w);

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x400004, 0x400005, 0, 0, topshoot_unk_r);
//	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xa10000, 0xa1001f, 0, 0, topshoot_unk_r);

	init_megadriv(machine);
}



ROM_START( topshoot ) /* Top Shooter (c)1995 Sun Mixing */
	ROM_REGION( 0x400000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "tc574000ad_u11_2.bin", 0x000000, 0x080000, CRC(b235c4d9) SHA1(fbb308a5f6e769f3277824cb6a3b50c308969ac2) )
	ROM_LOAD16_BYTE( "tc574000ad_u12_1.bin", 0x000001, 0x080000, CRC(e826f6ad) SHA1(23ec8bb608f954d3b915f061e7076c0c63b8259e) )
ROM_END

GAME( 1995, topshoot,        0,        megadriv,    megadriv,    topshoot, ROT0,   "Sun Mixing", "Top Shooter (Arcade)", 0 )

