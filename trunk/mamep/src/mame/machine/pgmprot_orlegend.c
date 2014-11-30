/***********************************************************************
 PGM ASIC3 PGM protection emulation

 this seems similar to the IGS025? Is the physical chip ASIC3, or is
 that just what the game calls it?

 Used by:

 Oriental Legend

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

void pgm_asic3_state::asic3_compute_hold(int y, int z)
{
	int mode;
	unsigned short old = m_asic3_hold;

	m_asic3_hold = ((old << 1) | (old >> 15));

	m_asic3_hold ^= 0x2bad;
	m_asic3_hold ^= BIT(z, y);
	m_asic3_hold ^= BIT(m_asic3_x, 2) << 10;
	m_asic3_hold ^= BIT(old, 5);

	if (!strcmp(machine().system().name,"orlegend111c") ||
		!strcmp(machine().system().name,"orlegendca"))
		mode = ioport("Region")->read();
	else if (!strcmp(machine().system().name,"orlegend111t"))
		mode = 4;
	else
		mode = 2;

	switch (mode) // The mode is dependent on the region
	{
		case 0:
		case 1: // Other
			m_asic3_hold ^= BIT(old, 10) ^ BIT(old, 8) ^ (BIT(m_asic3_x, 0) << 1) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 14);
		break;

		case 2: // Korea
			m_asic3_hold ^= BIT(old, 10) ^ BIT(old, 8) ^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 12);
		break;

		case 3: // China
			m_asic3_hold ^= BIT(old,  7) ^ BIT(old, 6) ^ (BIT(m_asic3_x, 0) << 4) ^ (BIT(m_asic3_x, 1) << 6) ^ (BIT(m_asic3_x, 3) << 12);
		break;

		case 4: // Taiwan
			m_asic3_hold ^= BIT(old,  7) ^ BIT(old, 6) ^ (BIT(m_asic3_x, 0) << 3) ^ (BIT(m_asic3_x, 1) << 8) ^ (BIT(m_asic3_x, 3) << 14);
		break;
	}
}

READ16_MEMBER(pgm_asic3_state::pgm_asic3_r)
{
	switch (m_asic3_reg)
	{
		case 0x00: // region is supplied by the protection device
		{
			if (!strcmp(machine().system().name,"orlegend111c") ||
				!strcmp(machine().system().name,"orlegendca"))
				return (m_asic3_latch[0] & 0xf7) | ((ioport("Region")->read() << 3) & 0x08);
			else if (!strcmp(machine().system().name,"orlegend111t"))
				return (m_asic3_latch[0] & 0xf7) | ((4 << 3) & 0x08);
			else
				return (m_asic3_latch[0] & 0xf7) | ((2 << 3) & 0x08);
		}

		case 0x01:
			return m_asic3_latch[1];

		case 0x02: // region is supplied by the protection device
		{
			if (!strcmp(machine().system().name,"orlegend111c") ||
				!strcmp(machine().system().name,"orlegendca"))
				return (m_asic3_latch[2] & 0x7f) | ((ioport("Region")->read() << 6) & 0x80);
			else if (!strcmp(machine().system().name,"orlegend111t"))
				return (m_asic3_latch[2] & 0x7f) | ((4 << 6) & 0x80);
			else
				return (m_asic3_latch[2] & 0x7f) | ((2 << 6) & 0x80);
		}

		case 0x03:
			return BITSWAP8(m_asic3_hold, 5,2,9,7,10,13,12,15);

		// case $157674, expected return $157686
		case 0x20: return 0x49; // "IGS"
		case 0x21: return 0x47;
		case 0x22: return 0x53;

		case 0x24: return 0x41;
		case 0x25: return 0x41;
		case 0x26: return 0x7f;
		case 0x27: return 0x41;
		case 0x28: return 0x41;

		case 0x2a: return 0x3e;
		case 0x2b: return 0x41;
		case 0x2c: return 0x49;
		case 0x2d: return 0xf9;
		case 0x2e: return 0x0a;

		case 0x30: return 0x26;
		case 0x31: return 0x49;
		case 0x32: return 0x49;
		case 0x33: return 0x49;
		case 0x34: return 0x32;

	//  default:
	//       logerror("ASIC3 R: CMD %2.2X PC: %6.6x\n", m_asic3_reg, space.device().safe_pc());
	}

	return 0;
}

WRITE16_MEMBER(pgm_asic3_state::pgm_asic3_w)
{
	if (offset == 0) {
		m_asic3_reg = data;
		return;
	}

	switch (m_asic3_reg)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			m_asic3_latch[m_asic3_reg] = data << 1;
		break;

	//  case 0x03: // move.w  #$88, $c0400e.l
	//  case 0x04: // move.w  #$84, $c0400e.l
	//  case 0x05: // move.w  #$A0, $c0400e.l
	//  break;

		case 0x40:
			m_asic3_hilo = (m_asic3_hilo << 8) | data;
		break;

		case 0x41: // Same as CMD 40. What is the purpose of writing data here again??
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		break;

		case 0x48:
		{
			m_asic3_x = 0;
			if ((m_asic3_hilo & 0x0090) == 0) m_asic3_x |= 0x01;
			if ((m_asic3_hilo & 0x0006) == 0) m_asic3_x |= 0x02;
			if ((m_asic3_hilo & 0x9000) == 0) m_asic3_x |= 0x04;
			if ((m_asic3_hilo & 0x0a00) == 0) m_asic3_x |= 0x08;
		}
		break;

	//  case 0x50: // move.w  #$50, $c0400e.l
	//  break;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
			asic3_compute_hold(m_asic3_reg & 0x07, data);
		break;

		case 0xa0:
			m_asic3_hold = 0;
		break;

		default:
				logerror("ASIC3 W: CMD %2.2X DATA: %4.4x, PC: %6.6x\n", m_asic3_reg, data, space.device().safe_pc());
	}
}

/* Oriental Legend INIT */

DRIVER_INIT_MEMBER(pgm_asic3_state,orld111c)
{
	pgm_basic_init();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xC04000, 0xC0400f, read16_delegate(FUNC(pgm_asic3_state::pgm_asic3_r),this), write16_delegate(FUNC(pgm_asic3_state::pgm_asic3_w),this));

	m_asic3_reg = 0;
	m_asic3_latch[0] = 0;
	m_asic3_latch[1] = 0;
	m_asic3_latch[2] = 0;
	m_asic3_x = 0;
	m_asic3_hilo = 0;
	m_asic3_hold = 0;

	save_item(NAME(m_asic3_reg));
	save_item(NAME(m_asic3_latch));
	save_item(NAME(m_asic3_x));
	save_item(NAME(m_asic3_hilo));
	save_item(NAME(m_asic3_hold));
}

DRIVER_INIT_MEMBER(pgm_asic3_state,orlegend)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
	DRIVER_INIT_CALL(orld111c);
	mem16[0x146ae4/2]=0x4e71;
	mem16[0x146ae6/2]=0x4e71;
}

DRIVER_INIT_MEMBER(pgm_asic3_state,orld112c)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
	DRIVER_INIT_CALL(orld111c);
	mem16[0x146af4/2]=0x4e71;
	mem16[0x146af6/2]=0x4e71;
}

DRIVER_INIT_MEMBER(pgm_asic3_state,orld111t)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
	DRIVER_INIT_CALL(orld111c);
	mem16[0x1468a8/2]=0x4e71;
	mem16[0x1468aa/2]=0x4e71;
}

DRIVER_INIT_MEMBER(pgm_asic3_state,orld105k)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();
	DRIVER_INIT_CALL(orld111c);
	mem16[0x146450/2]=0x4e71;
	mem16[0x146452/2]=0x4e71;
}


INPUT_PORTS_START( orlegend )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0007,  0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( orld112c )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0007,  0x0001, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( orld111c )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0003,  0x0002, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0001, "Hong Kong (duplicate)" ) // again?
	PORT_CONFSETTING(      0x0002, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0003, "China (duplicate)" ) // again?
INPUT_PORTS_END

INPUT_PORTS_START( orld111t )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0007,  0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( orld105k )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0007,  0x0003, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(pgm_asic3_state, pgm_asic3_reset)
{
	// internal roms aren't fully dumped
	UINT8 *mem8 = (UINT8 *)memregion("maincpu")->base();

	if (!strcmp(machine().system().name, "orlegend")) mem8[0x1d1749] = ioport("Region")->read();
	if (!strcmp(machine().system().name, "orlegende")) mem8[0x1d1759] = ioport("Region")->read();
	if (!strcmp(machine().system().name, "orlegendc")) mem8[0x1d15cf] = ioport("Region")->read();
	if (!strcmp(machine().system().name, "orlegend111t")) mem8[0x1d1287] = ioport("Region")->read();
	if (!strcmp(machine().system().name, "orlegend105k")) mem8[0x1d0e47] = ioport("Region")->read();

	MACHINE_RESET_CALL_MEMBER(pgm);
}


MACHINE_CONFIG_START( pgm_asic3, pgm_asic3_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_MACHINE_RESET_OVERRIDE(pgm_asic3_state, pgm_asic3_reset)
MACHINE_CONFIG_END
