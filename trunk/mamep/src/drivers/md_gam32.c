/* 32x games */

#include "driver.h"
#include "megadriv.h"


ROM_START( 32x_bios )
	ROM_REGION( 0x400000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION( 0x400000, REGION_CPU3, 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, REGION_CPU4, 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END

ROM_START( 32x_knuk )
	ROM_REGION16_BE( 0x400000, REGION_USER1, 0 ) /* 68000 Code */
	ROM_LOAD( "32x_knuk.bin", 0x000000,  0x300000, CRC(d0b0b842) SHA1(0c2fff7bc79ed26507c08ac47464c3af19f7ced7) )

	ROM_REGION( 0x400000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_COPY(REGION_USER1,0,0,0x400000)
	ROM_LOAD( "32x_g_bios.bin", 0x000000,  0x000100, CRC(5c12eae8) SHA1(dbebd76a448447cb6e524ac3cb0fd19fc065d944) )

	ROM_REGION( 0x400000, REGION_CPU3, 0 ) /* SH2 Code */
	ROM_LOAD( "32x_m_bios.bin", 0x000000,  0x000800, CRC(dd9c46b8) SHA1(1e5b0b2441a4979b6966d942b20cc76c413b8c5e) )

	ROM_REGION( 0x400000, REGION_CPU4, 0 ) /* SH2 Code */
	ROM_LOAD( "32x_s_bios.bin", 0x000000,  0x000400, CRC(bfda1fe5) SHA1(4103668c1bbd66c5e24558e73d4f3f92061a109a) )
ROM_END


GAME( 1900, 32x_bios,    0,        _32x,        megadriv,    _32x,    ROT0,   "Unsorted", "32X Bios", GAME_NOT_WORKING|NOT_A_DRIVER )
GAME( 1900, 32x_knuk,    32x_bios, _32x,        megadriv,    _32x, ROT0,   "Unsorted", "Knuckles Chaotix", GAME_NOT_WORKING )



