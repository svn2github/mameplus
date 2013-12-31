/*

Cave CV1000 hardware

Games by Cave ID number:

CA011  Mushihime-Sama
CA012  Ibara
CA012B Ibara Kuro Black Label
CA013  Espgaluda II
CA014  Pink Sweets: Ibara Sorekara
CA015  Mushihime-Sama Futari
CA015B Mushihime-Sama Futari Black Label
CA016  Muchi Muchi Pork!
CA017  Deathsmiles
CA017B Deathsmiles Mega Black Label
CA019  Do-Don-Pachi Dai-Fukkatsu
CA019B Do-Don-Pachi Dai-Fukkatsu Black Label
CA021  Akai Katana

CMDL01 Medal Mahjong Moukari Bancho

Note: CA018 - DoDonPachi Dai-Ou-Jou Tamashii on PGM2 platform
      CA020 - Deathsmiles II: Makai no Merry Christmas on unknown custom platform

PCB CV1000-B / CV1000-D
+--------------------------------------------+
|                                            |
|                                            |
|                                            |
|                 VOL                        |
|                                            |
+-+        +-----+ +-----+       X3          |
  |        | U24 | | U23 |                   |
+-+        +-----+ +-----+    +------+       |
|                             |Yamaha|       |
|            U25*    U26*     |YMZ770|       |
|                             |      |       |
|J  D5                        +------+       |
|A  D2                                       |
|M  D3                    +-----+ +-----+    |
|M  D4         P2*        | U7  | | U6  |    |
|A  D1                    +-----+ +-----+    |
|          +-------+                         |
|C         |P4 JTAG|                         |
|o         +-------+       +-------+         |
|n                         |Altera |  +--+   |
|n          D6             |Cyclone|  |  |   |
|e     X1      S1          |       |  |U1|   |
|c    S3 +---------+       +-------+  |  |   |
|t       |         |                  +--+   |
|e       | Hitachi |                         |
|r       |         |     S2                  |
|        |   SH3   |   +-----------------+   |
|        |         |   |       P3        |   |
|        +---------+   +-----------------+   |
+-+    X2                                    |
  |       +--+  +--+    +---+  U27           |
+-+       |  |  |  |    |U13|                |
|+-+      |U2|  |U4|    +---+                |
||P|      |  |  |  |                 __      |
||8| U12* +--+  +--+     U10        /  \     |
|+-+                               |C126|    |
|     P5* P7*                       \__/     |
+--------------------------------------------+

* Denotes not populated

  CPU: Hitachi 6417709S SH3 clocked at 102.4MHz (12.800MHz * 8)
Sound: Yamaha YMZ770C-F clocked at 16.384MHz
Other: Altera Cyclone EP1C12 FPGA
       Altera EPM7032 (MAX 7000 Series) at U13

OSC:
 X1 12.800MHz (SH3 clock derived from this)
 X2 32.768kHz (Used by the RTC)
 X3 16.384MHz (Yamaha YMZ770C-F clock)

Memory:
 U6 (SDRAM)  MT46V16M16 ? 4 MBit x 16 x 4 banks, RAM (256 MBit)
 U7 (SDRAM)  MT46V16M16 ? 4 MBit x 16 x 4 banks, RAM (256 MBit)
 U1 (SDRAM)  MT48LC2M32 ? 512K x 32 x 4 banks, (64 MBit) for CV1000-B
 U1 (SDRAM)  IS42S32400 - 1024K x 32 x 4 banks, (128 MBit) for CV1000-D

Roms:
      U4 (FLASH)  29LV160BB 16M-Bit CMOS 3.0V, Boot device, FPGA bit file, main program code for CV1000-B
      U4 (FLASH)  S29JL032H 32M-Bit CMOS 3.0V, Boot device, FPGA bit file, main program code for CV1000-D
      U2 (FLASH)  K9F1G08U0M 128M x 8 Bit / 64M x 16 Bit NAND. Graphics data.
 U23-U24 (FLASH)  MBM 29DL321, 32M-Bit CMOS 3.0V. Sound data.
 U25-U26 (FLASH)  MBM 29DL321, not populated

Battery:
 C126 CR2450, Powers the RTC (Real Time Clock) U10. Look at the garden clock in Ibara. NOT present on CV1000-D

Dipswitches & Push Buttons:
 S1 (DIL SWITCH) Half Pitch DIL Switch x 1, function unknown
 S2 (DIL SWITCH) Half Pitch DIL Switch x 4, SW1=Setup, other switches unknown
 S3 (MICRO PUSH BUTTON) Test switch, same as on the JAMMA connector
.
Connectors:
 P2 (IDC CONNECTOR 20 PIN) function unknown, P2 is not always mounted
 P4 (IDC CONNECTOR 14 PIN) JTAG connector
 P8 (IDC CONNECTOR 10 PIN) Advanced User Debugger
 P3 (CONNECTOR) Most likely an expansion port, P3 is not always mounted
 P5 (CONNECTOR) D9 serial connector. Used for the mahjong Touchscreen titles.  Also mounted on early Mushihime-Sama PCB's
 P7 (CONNECTOR) Network port pinout. Never seen mounted on any PCB.

Misc:
   U27 (SUPERVISOR) MAX 690S 3.0V Microprocessor Supervisory Circuit.
   U10 (RTC & EEPROM) RTC 9701, Serial RTC Module with EEPROM 4 kbit (256x16 bit), controlled by Altera EPM7032 U13.
   U12 (RS-232 TRANCEIVER) MAX 3244E RS-232 Tranceiver, only mounted when P5 is mounted.
 D1-D6 (LED) Status LED's. D6 lights up at power on then shuts off, D2 indicates coinage.

Note: * The Altera EPM7032 usually stamped / labeled with the Cave game ID number as listed above.
      * Actual flash ROMs will vary by manufacturer but will be compatible with flash ROM listed.
      * The CV1000-D revision PCB has double the RAM at U1, double the ROM at U4 and no battery.
        The CV1000-D is used for Dodonpachi Daifukkatsu and later games. Commonly referred to as SH3B PCB.

Information by The Sheep, rtw, Ex-Cyber, BrianT & Guru

------------------------------------------------------

 To enter service mode in most cases hold down 0 (Service 2) for a few seconds
  (I believe it's the test button on the PCB)
 Some games also use the test dipswitch as an alternative method.

ToDo:

Improve Blending precision?
 - I'm not sure what precision the original HW mixes with, source data is 555 RGB with 1 bit transparency (16-bits)
   and the real VRAM is also clearly in this format.  The Alpha values supplied however are 8bpp, and the 'Tint'
   values use 0x20 for 'normal' (not 0x1f)

Overall screen brightness / contrast (see test mode)
 - Could convert ram back to 16-bit and use a palette lookup at the final blit.. probably easiest / quickest.

Touchscreen
 - Used for mmmbanc, needs SH3 serial support.

Remaining Video issues
 - mmpork startup screen flicker - the FOR USE IN JAPAN screen doesn't appear on the real PCB until after the graphics are fully loaded, it still displays 'please wait' until that point.
 - is the use of the 'scroll' registers 100% correct? (related to above?)
 - Sometimes the 'sprites' in mushisam lag by a frame vs the 'backgrounds' is this a timing problem, does the real game do it?

Speedups
 - Blitter is already tightly optimized
 - Need SH3 recompiler?

*/

#include "emu.h"
#include "cpu/sh4/sh4.h"
#include "cpu/sh4/sh3comn.h"
#include "profiler.h"
#include "machine/rtc9701.h"
#include "sound/ymz770.h"
#include "mcfglgcy.h"
#include "scrlegcy.h"

static UINT64* cavesh3_ram;
static UINT16* cavesh3_ram16;
static UINT16* cavesh3_ram16_copy = 0;




class cavesh3_state : public driver_device
{
public:
	cavesh3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	UINT8* flashregion;
	UINT8* flashwritemap;

	osd_work_queue *	queue;					/* work queue */
	osd_work_item * blitter_request;

	// blit timing
	emu_timer *cavesh3_blitter_delay_timer;
	int blitter_busy;

	// read/write methods
	DECLARE_WRITE8_MEMBER(ibara_flash_io_w);
	DECLARE_WRITE8_MEMBER(flash_addr_w);
	DECLARE_WRITE8_MEMBER(flash_cmd_w);
	DECLARE_WRITE8_MEMBER(flash_data_w);
	DECLARE_WRITE8_MEMBER(serial_rtc_eeprom_w);
	DECLARE_WRITE8_MEMBER(flash_enab_w);
	DECLARE_WRITE32_MEMBER(cavesh3_blitter_w);
	DECLARE_WRITE32_MEMBER(cavesh_gfx_exec_w);
	//DECLARE_WRITE64_MEMBER(ibara_fpga_w);
	//DECLARE_WRITE64_MEMBER(cavesh3_nop_write);
	DECLARE_READ8_MEMBER(ibara_flash_io_r);
	DECLARE_READ8_MEMBER(flash_io_r);
	DECLARE_READ8_MEMBER(serial_rtc_eeprom_r);
	//DECLARE_READ8_MEMBER(flash_ready_r);
	DECLARE_READ32_MEMBER(cavesh3_blitter_r);
	DECLARE_READ32_MEMBER(cavesh_gfx_ready_r);
	DECLARE_READ64_MEMBER(ibara_flash_port_e_r);
	DECLARE_READ64_MEMBER(ibara_fpga_r);

	// Driver init methods
	DECLARE_DRIVER_INIT(mushisam);
	DECLARE_DRIVER_INIT(mushisama);
	DECLARE_DRIVER_INIT(espgal2);
	DECLARE_DRIVER_INIT(ddpdfk);
	DECLARE_READ64_MEMBER(mushisam_speedup_r);
	DECLARE_READ64_MEMBER(mushisama_speedup_r);
	DECLARE_READ64_MEMBER(espgal2_speedup_r);
	DECLARE_READ64_MEMBER(ddpdfk_speedup_r);

	DECLARE_MACHINE_START(cavesh3);
	DECLARE_MACHINE_RESET(cavesh3);
	DECLARE_VIDEO_START(cavesh3);
	INTERRUPT_GEN_MEMBER(cavesh3_interrupt);
	TIMER_CALLBACK_MEMBER(cavesh3_blitter_delay_callback);
	required_device<cpu_device> m_maincpu;
};

/***************************************************************************
                                Video Hardware
***************************************************************************/

UINT8 cavesh3_colrtable[0x20][0x40];
UINT8 cavesh3_colrtable_rev[0x20][0x40];
UINT8 cavesh3_colrtable_add[0x20][0x20];
struct _clr_t
{
	UINT8 b,g,r,t;
};



typedef struct _clr_t clr_t;

union colour_t
{
	clr_t trgb;
	UINT32 u32;
};



// r5g5b5 ro clr_t
INLINE void pen_to_clr(UINT32 pen, clr_t *clr)
{
// --t- ---- rrrr r--- gggg g--- bbbb b---  format
	clr->r = (pen >> (16+3));// & 0x1f;
	clr->g = (pen >>  (8+3));// & 0x1f;
	clr->b = (pen >>   3);// & 0x1f;
}


// convert separate r,g,b biases (0..80..ff) to clr_t (-1f..0..1f)
INLINE void tint_to_clr(UINT8 r, UINT8 g, UINT8 b, clr_t *clr)
{
	clr->r	=	r>>2;
	clr->g	=	g>>2;
	clr->b	=	b>>2;
}

#if 0
INLINE void clamp_clr(clr_t *clr)
{
	if (clr->r > 0x1f) clr->r = 0x1f;
	if (clr->g > 0x1f) clr->g = 0x1f;
	if (clr->b > 0x1f) clr->b = 0x1f;
}
#endif

// clr_t to r5g5b5
INLINE UINT32 clr_to_pen(const clr_t *clr)
{
// --t- ---- rrrr r--- gggg g--- bbbb b---  format
	return (clr->r << (16+3)) | (clr->g << (8+3)) | (clr->b << 3);
}



// add clrs
INLINE void clr_add_with_clr_mul_fixed(clr_t *clr, const clr_t *clr0, const UINT8 mulfixed_val, const clr_t *mulfixed_clr0)
{
	clr->r = cavesh3_colrtable_add[clr0->r][cavesh3_colrtable[(mulfixed_clr0->r)][mulfixed_val]];
	clr->g = cavesh3_colrtable_add[clr0->g][cavesh3_colrtable[(mulfixed_clr0->g)][mulfixed_val]];
	clr->b = cavesh3_colrtable_add[clr0->b][cavesh3_colrtable[(mulfixed_clr0->b)][mulfixed_val]];
}

INLINE void clr_add_with_clr_mul_3param(clr_t *clr, const clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
{
	clr->r = cavesh3_colrtable_add[clr0->r][cavesh3_colrtable[(clr2->r)][(clr1->r)]];
	clr->g = cavesh3_colrtable_add[clr0->g][cavesh3_colrtable[(clr2->g)][(clr1->g)]];
	clr->b = cavesh3_colrtable_add[clr0->b][cavesh3_colrtable[(clr2->b)][(clr1->b)]];
}

INLINE void clr_add_with_clr_square(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
{
	clr->r = cavesh3_colrtable_add[clr0->r][cavesh3_colrtable[(clr1->r)][(clr1->r)]];
	clr->g = cavesh3_colrtable_add[clr0->r][cavesh3_colrtable[(clr1->g)][(clr1->g)]];
	clr->b = cavesh3_colrtable_add[clr0->r][cavesh3_colrtable[(clr1->b)][(clr1->b)]];
}

INLINE void clr_add_with_clr_mul_fixed_rev(clr_t *clr, const clr_t *clr0, const UINT8 val, const clr_t *clr1)
{
	clr->r =  cavesh3_colrtable_add[clr0->r][cavesh3_colrtable_rev[val][(clr1->r)]];
	clr->g =  cavesh3_colrtable_add[clr0->g][cavesh3_colrtable_rev[val][(clr1->g)]];
	clr->b =  cavesh3_colrtable_add[clr0->b][cavesh3_colrtable_rev[val][(clr1->b)]];
}

INLINE void clr_add_with_clr_mul_rev_3param(clr_t *clr, const clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
{
	clr->r =  cavesh3_colrtable_add[clr0->r][cavesh3_colrtable_rev[(clr2->r)][(clr1->r)]];
	clr->g =  cavesh3_colrtable_add[clr0->g][cavesh3_colrtable_rev[(clr2->g)][(clr1->g)]];
	clr->b =  cavesh3_colrtable_add[clr0->b][cavesh3_colrtable_rev[(clr2->b)][(clr1->b)]];
}

INLINE void clr_add_with_clr_mul_rev_square(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
{
	clr->r =  cavesh3_colrtable_add[clr0->r][cavesh3_colrtable_rev[(clr1->r)][(clr1->r)]];
	clr->g =  cavesh3_colrtable_add[clr0->g][cavesh3_colrtable_rev[(clr1->g)][(clr1->g)]];
	clr->b =  cavesh3_colrtable_add[clr0->b][cavesh3_colrtable_rev[(clr1->b)][(clr1->b)]];
}


INLINE void clr_add(clr_t *clr, const clr_t *clr0, const clr_t *clr1)
{
	// use pre-clamped lookup table
	clr->r =  cavesh3_colrtable_add[clr0->r][clr1->r];
	clr->g =  cavesh3_colrtable_add[clr0->g][clr1->g];
	clr->b =  cavesh3_colrtable_add[clr0->b][clr1->b];
}

// multiply clrs
INLINE void clr_mul(clr_t *clr0, const clr_t *clr1)
{
	clr0->r = cavesh3_colrtable[(clr0->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable[(clr0->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable[(clr0->b)][(clr1->b)];
}

INLINE void clr_square(clr_t *clr0, const clr_t *clr1)
{
	clr0->r = cavesh3_colrtable[(clr1->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable[(clr1->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable[(clr1->b)][(clr1->b)];
}

INLINE void clr_mul_3param(clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
{
	clr0->r = cavesh3_colrtable[(clr2->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable[(clr2->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable[(clr2->b)][(clr1->b)];
}

INLINE void clr_mul_rev(clr_t *clr0, const clr_t *clr1)
{
	clr0->r = cavesh3_colrtable_rev[(clr0->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable_rev[(clr0->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable_rev[(clr0->b)][(clr1->b)];
}

INLINE void clr_mul_rev_square(clr_t *clr0, const clr_t *clr1)
{
	clr0->r = cavesh3_colrtable_rev[(clr1->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable_rev[(clr1->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable_rev[(clr1->b)][(clr1->b)];
}


INLINE void clr_mul_rev_3param(clr_t *clr0, const clr_t *clr1, const clr_t *clr2)
{
	clr0->r = cavesh3_colrtable_rev[(clr2->r)][(clr1->r)];
	clr0->g = cavesh3_colrtable_rev[(clr2->g)][(clr1->g)];
	clr0->b = cavesh3_colrtable_rev[(clr2->b)][(clr1->b)];
}

INLINE void clr_mul_fixed(clr_t *clr, const UINT8 val, const clr_t *clr0)
{
	clr->r = cavesh3_colrtable[val][(clr0->r)];
	clr->g = cavesh3_colrtable[val][(clr0->g)];
	clr->b = cavesh3_colrtable[val][(clr0->b)];
}

INLINE void clr_mul_fixed_rev(clr_t *clr, const UINT8 val, const clr_t *clr0)
{
	clr->r = cavesh3_colrtable_rev[val][(clr0->r)];
	clr->g = cavesh3_colrtable_rev[val][(clr0->g)];
	clr->b = cavesh3_colrtable_rev[val][(clr0->b)];
}

INLINE void clr_copy(clr_t *clr, const clr_t *clr0)
{
	clr->r = clr0->r;
	clr->g = clr0->g;
	clr->b = clr0->b;
}



// (1|s|d) * s_factor * s + (1|s|d) * d_factor * d
// 0: +alpha
// 1: +source
// 2: +dest
// 3: *
// 4: -alpha
// 5: -source
// 6: -dest
// 7: *



static UINT32 cavesh_gfx_addr;
static UINT32 cavesh_gfx_scroll_0_x, cavesh_gfx_scroll_0_y;
static UINT32 cavesh_gfx_scroll_1_x, cavesh_gfx_scroll_1_y;

static UINT32 cavesh_gfx_addr_shadowcopy;
static UINT32 cavesh_gfx_scroll_0_x_shadowcopy, cavesh_gfx_scroll_0_y_shadowcopy;
static UINT32 cavesh_gfx_scroll_1_x_shadowcopy, cavesh_gfx_scroll_1_y_shadowcopy;

static UINT64 cave_blit_delay;


static int cavesh_gfx_size;
static bitmap_rgb32 *cavesh_bitmaps;
static rectangle cavesh_clip;

VIDEO_START_MEMBER(cavesh3_state,cavesh3)
{
	cavesh_gfx_size	= 0x2000 * 0x1000;
	cavesh_bitmaps = auto_bitmap_rgb32_alloc(machine(), 0x2000, 0x1000);
	cavesh_clip = cavesh_bitmaps->cliprect();
}



#define TRANSPARENT 1

#define FUNCNAME draw_sprite_notint_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT
#define TRANSPARENT 0

#define FUNCNAME draw_sprite_notint_opaque_plain
#include "csh3blit.inc"
#undef FUNCNAME

#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_opaque_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_opaque_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_opaque_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_opaque_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_opaque_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_opaque_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_opaque_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_opaque_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT

// flipped X cases

#define FLIPX 1
#define TRANSPARENT 1

#define FUNCNAME draw_sprite_notint_flipx_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT
#define TRANSPARENT 0

#define FUNCNAME draw_sprite_notint_flipx_opaque_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_notint_flipx_opaque_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT
#undef FLIPX

// really simple cases

#define REALLY_SIMPLE

#define TRANSPARENT 1
#define FUNCNAME draw_sprite_notint_simple
#include "csh3blit.inc"
#undef FUNCNAME

#define FLIPX
#define FUNCNAME draw_sprite_notint_flipx_simple
#include "csh3blit.inc"
#undef FUNCNAME
#undef FLIPX
#undef TRANSPARENT

#define TRANSPARENT 0
#define FUNCNAME draw_sprite_notint_opaque_simple
#include "csh3blit.inc"
#undef FUNCNAME

#define FLIPX
#define FUNCNAME draw_sprite_notint_flipx_opaque_simple
#include "csh3blit.inc"
#undef FUNCNAME
#undef FLIPX
#undef TRANSPARENT
#undef REALLY_SIMPLE


// TINT

#define TINT


#define TRANSPARENT 1

#define FUNCNAME draw_sprite_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT


#define TRANSPARENT 0

#define FUNCNAME draw_sprite_opaque_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_opaque_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_opaque_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_opaque_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_opaque_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_opaque_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_opaque_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_opaque_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_opaque_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT

// flipped X cases

#define FLIPX 1
#define TRANSPARENT 1

#define FUNCNAME draw_sprite_flipx_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT
#define TRANSPARENT 0

#define FUNCNAME draw_sprite_flipx_opaque_plain
#include "csh3blit.inc"
#undef FUNCNAME


#define BLENDED

#define _SMODE 0
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s0_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s1_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s2_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s3_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s4_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s5_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s6_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 0
#define FUNCNAME draw_sprite_flipx_opaque_s7_d0
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s0_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s1_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s2_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s3_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s4_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s5_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s6_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 1
#define FUNCNAME draw_sprite_flipx_opaque_s7_d1
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s0_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s1_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s2_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s3_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s4_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s5_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s6_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 2
#define FUNCNAME draw_sprite_flipx_opaque_s7_d2
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s0_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s1_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s2_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s3_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s4_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s5_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s6_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 3
#define FUNCNAME draw_sprite_flipx_opaque_s7_d3
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s0_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s1_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s2_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s3_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s4_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s5_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s6_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 4
#define FUNCNAME draw_sprite_flipx_opaque_s7_d4
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s0_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s1_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s2_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s3_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s4_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s5_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s6_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 5
#define FUNCNAME draw_sprite_flipx_opaque_s7_d5
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s0_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s1_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s2_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s3_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s4_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s5_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s6_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 6
#define FUNCNAME draw_sprite_flipx_opaque_s7_d6
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE


#define _SMODE 0
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s0_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 1
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s1_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 2
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s2_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 3
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s3_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 4
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s4_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 5
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s5_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 6
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s6_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#define _SMODE 7
#define _DMODE 7
#define FUNCNAME draw_sprite_flipx_opaque_s7_d7
#include "csh3blit.inc"
#undef FUNCNAME
#undef _SMODE
#undef _DMODE

#undef BLENDED

#undef TRANSPARENT
#undef FLIPX

#undef TINT


INLINE UINT16 READ_NEXT_WORD(offs_t *addr)
{
	UINT16 data =  cavesh3_ram16_copy[((*addr&(0x7fffff))>>1)^3]; // this probably needs to be made endian safe tho
	*addr += 2;

	return data;
}

INLINE UINT16 COPY_NEXT_WORD(address_space &space, offs_t *addr)
{
	UINT16 data =  cavesh3_ram16[((*addr&(0x7fffff))>>1)^3]; // this probably needs to be made endian safe tho
	cavesh3_ram16_copy[((*addr&(0x7fffff))>>1)^3] = data;
	*addr += 2;

	return data;
}


INLINE void cavesh_gfx_upload_shadow_copy(address_space &space, offs_t *addr)
{
	UINT32 x,y, dimx,dimy;
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);

	dimx = (COPY_NEXT_WORD(space, addr) & 0x1fff) + 1;
	dimy = (COPY_NEXT_WORD(space, addr) & 0x0fff) + 1;

	for (y = 0; y < dimy; y++)
		for (x = 0; x < dimx; x++)
			COPY_NEXT_WORD(space, addr);
}

INLINE void cavesh_gfx_upload(offs_t *addr)
{
	UINT32 x,y,dst_x_start,dst_y_start, dimx,dimy;
	UINT32 *dst;

	// 0x20000000
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	// 0x99999999
	READ_NEXT_WORD(addr);
	READ_NEXT_WORD(addr);

	dst_x_start = READ_NEXT_WORD(addr);
	dst_y_start = READ_NEXT_WORD(addr);

	// dst_p = 0;
	dst_x_start &= 0x1fff;
	dst_y_start &= 0x0fff;

	dimx = (READ_NEXT_WORD(addr) & 0x1fff) + 1;
	dimy = (READ_NEXT_WORD(addr) & 0x0fff) + 1;

	for (y = 0; y < dimy; y++)
	{
		dst = &cavesh_bitmaps->pix32(dst_y_start + y);
		dst += dst_x_start;

		for (x = 0; x < dimx; x++)
		{
			UINT16 pendat = READ_NEXT_WORD(addr);
			// real hw would upload the gfxword directly, but our VRAM is 32-bit, so convert it.
			*dst++ = ((pendat&0x8000)<<14) | ((pendat&0x7c00)<<9) | ((pendat&0x03e0)<<6) | ((pendat&0x001f)<<3);  // --t- ---- rrrr r--- gggg g--- bbbb b---  format

		}
	}
}

#define draw_params cavesh_bitmaps, &cavesh_clip, &cavesh_bitmaps->pix32(0,0), src_x,src_y, x,y, dimx,dimy, flipy, s_alpha, d_alpha, &tint_clr

typedef const void (*caveblitfunction)(bitmap_rgb32 *,
					 const rectangle *,
					 UINT32 *, /* gfx */
					 int , /* src_x */
					 int , /* src_y */
					 int , /* dst_x_start */
					 int , /* dst_y_start */
					 int , /* dimx */
					 int , /* dimy */
					 int , /* flipy */
					 const UINT8 , /* s_alpha */
					 const UINT8 , /* d_alpha */
					 clr_t * );

caveblitfunction cave_blit_funcs[] =
{
	draw_sprite_s0_d0, draw_sprite_s1_d0, draw_sprite_s2_d0, draw_sprite_s3_d0, draw_sprite_s4_d0, draw_sprite_s5_d0, draw_sprite_s6_d0, draw_sprite_s7_d0,
	draw_sprite_s0_d1, draw_sprite_s1_d1, draw_sprite_s2_d1, draw_sprite_s3_d1, draw_sprite_s4_d1, draw_sprite_s5_d1, draw_sprite_s6_d1, draw_sprite_s7_d1,
	draw_sprite_s0_d2, draw_sprite_s1_d2, draw_sprite_s2_d2, draw_sprite_s3_d2, draw_sprite_s4_d2, draw_sprite_s5_d2, draw_sprite_s6_d2, draw_sprite_s7_d2,
	draw_sprite_s0_d3, draw_sprite_s1_d3, draw_sprite_s2_d3, draw_sprite_s3_d3, draw_sprite_s4_d3, draw_sprite_s5_d3, draw_sprite_s6_d3, draw_sprite_s7_d3,
	draw_sprite_s0_d4, draw_sprite_s1_d4, draw_sprite_s2_d4, draw_sprite_s3_d4, draw_sprite_s4_d4, draw_sprite_s5_d4, draw_sprite_s6_d4, draw_sprite_s7_d4,
	draw_sprite_s0_d5, draw_sprite_s1_d5, draw_sprite_s2_d5, draw_sprite_s3_d5, draw_sprite_s4_d5, draw_sprite_s5_d5, draw_sprite_s6_d5, draw_sprite_s7_d5,
	draw_sprite_s0_d6, draw_sprite_s1_d6, draw_sprite_s2_d6, draw_sprite_s3_d6, draw_sprite_s4_d6, draw_sprite_s5_d6, draw_sprite_s6_d6, draw_sprite_s7_d6,
	draw_sprite_s0_d7, draw_sprite_s1_d7, draw_sprite_s2_d7, draw_sprite_s3_d7, draw_sprite_s4_d7, draw_sprite_s5_d7, draw_sprite_s6_d7, draw_sprite_s7_d7,
};

caveblitfunction cave_opaque_blit_funcs[] =
{
	draw_sprite_opaque_s0_d0, draw_sprite_opaque_s1_d0, draw_sprite_opaque_s2_d0, draw_sprite_opaque_s3_d0, draw_sprite_opaque_s4_d0, draw_sprite_opaque_s5_d0, draw_sprite_opaque_s6_d0, draw_sprite_opaque_s7_d0,
	draw_sprite_opaque_s0_d1, draw_sprite_opaque_s1_d1, draw_sprite_opaque_s2_d1, draw_sprite_opaque_s3_d1, draw_sprite_opaque_s4_d1, draw_sprite_opaque_s5_d1, draw_sprite_opaque_s6_d1, draw_sprite_opaque_s7_d1,
	draw_sprite_opaque_s0_d2, draw_sprite_opaque_s1_d2, draw_sprite_opaque_s2_d2, draw_sprite_opaque_s3_d2, draw_sprite_opaque_s4_d2, draw_sprite_opaque_s5_d2, draw_sprite_opaque_s6_d2, draw_sprite_opaque_s7_d2,
	draw_sprite_opaque_s0_d3, draw_sprite_opaque_s1_d3, draw_sprite_opaque_s2_d3, draw_sprite_opaque_s3_d3, draw_sprite_opaque_s4_d3, draw_sprite_opaque_s5_d3, draw_sprite_opaque_s6_d3, draw_sprite_opaque_s7_d3,
	draw_sprite_opaque_s0_d4, draw_sprite_opaque_s1_d4, draw_sprite_opaque_s2_d4, draw_sprite_opaque_s3_d4, draw_sprite_opaque_s4_d4, draw_sprite_opaque_s5_d4, draw_sprite_opaque_s6_d4, draw_sprite_opaque_s7_d4,
	draw_sprite_opaque_s0_d5, draw_sprite_opaque_s1_d5, draw_sprite_opaque_s2_d5, draw_sprite_opaque_s3_d5, draw_sprite_opaque_s4_d5, draw_sprite_opaque_s5_d5, draw_sprite_opaque_s6_d5, draw_sprite_opaque_s7_d5,
	draw_sprite_opaque_s0_d6, draw_sprite_opaque_s1_d6, draw_sprite_opaque_s2_d6, draw_sprite_opaque_s3_d6, draw_sprite_opaque_s4_d6, draw_sprite_opaque_s5_d6, draw_sprite_opaque_s6_d6, draw_sprite_opaque_s7_d6,
	draw_sprite_opaque_s0_d7, draw_sprite_opaque_s1_d7, draw_sprite_opaque_s2_d7, draw_sprite_opaque_s3_d7, draw_sprite_opaque_s4_d7, draw_sprite_opaque_s5_d7, draw_sprite_opaque_s6_d7, draw_sprite_opaque_s7_d7,
};

caveblitfunction cave_flipx_blit_funcs[] =
{
	draw_sprite_flipx_s0_d0, draw_sprite_flipx_s1_d0, draw_sprite_flipx_s2_d0, draw_sprite_flipx_s3_d0, draw_sprite_flipx_s4_d0, draw_sprite_flipx_s5_d0, draw_sprite_flipx_s6_d0, draw_sprite_flipx_s7_d0,
	draw_sprite_flipx_s0_d1, draw_sprite_flipx_s1_d1, draw_sprite_flipx_s2_d1, draw_sprite_flipx_s3_d1, draw_sprite_flipx_s4_d1, draw_sprite_flipx_s5_d1, draw_sprite_flipx_s6_d1, draw_sprite_flipx_s7_d1,
	draw_sprite_flipx_s0_d2, draw_sprite_flipx_s1_d2, draw_sprite_flipx_s2_d2, draw_sprite_flipx_s3_d2, draw_sprite_flipx_s4_d2, draw_sprite_flipx_s5_d2, draw_sprite_flipx_s6_d2, draw_sprite_flipx_s7_d2,
	draw_sprite_flipx_s0_d3, draw_sprite_flipx_s1_d3, draw_sprite_flipx_s2_d3, draw_sprite_flipx_s3_d3, draw_sprite_flipx_s4_d3, draw_sprite_flipx_s5_d3, draw_sprite_flipx_s6_d3, draw_sprite_flipx_s7_d3,
	draw_sprite_flipx_s0_d4, draw_sprite_flipx_s1_d4, draw_sprite_flipx_s2_d4, draw_sprite_flipx_s3_d4, draw_sprite_flipx_s4_d4, draw_sprite_flipx_s5_d4, draw_sprite_flipx_s6_d4, draw_sprite_flipx_s7_d4,
	draw_sprite_flipx_s0_d5, draw_sprite_flipx_s1_d5, draw_sprite_flipx_s2_d5, draw_sprite_flipx_s3_d5, draw_sprite_flipx_s4_d5, draw_sprite_flipx_s5_d5, draw_sprite_flipx_s6_d5, draw_sprite_flipx_s7_d5,
	draw_sprite_flipx_s0_d6, draw_sprite_flipx_s1_d6, draw_sprite_flipx_s2_d6, draw_sprite_flipx_s3_d6, draw_sprite_flipx_s4_d6, draw_sprite_flipx_s5_d6, draw_sprite_flipx_s6_d6, draw_sprite_flipx_s7_d6,
	draw_sprite_flipx_s0_d7, draw_sprite_flipx_s1_d7, draw_sprite_flipx_s2_d7, draw_sprite_flipx_s3_d7, draw_sprite_flipx_s4_d7, draw_sprite_flipx_s5_d7, draw_sprite_flipx_s6_d7, draw_sprite_flipx_s7_d7,
};

caveblitfunction cave_flipx_opaque_blit_funcs[] =
{
	draw_sprite_flipx_opaque_s0_d0, draw_sprite_flipx_opaque_s1_d0, draw_sprite_flipx_opaque_s2_d0, draw_sprite_flipx_opaque_s3_d0, draw_sprite_flipx_opaque_s4_d0, draw_sprite_flipx_opaque_s5_d0, draw_sprite_flipx_opaque_s6_d0, draw_sprite_flipx_opaque_s7_d0,
	draw_sprite_flipx_opaque_s0_d1, draw_sprite_flipx_opaque_s1_d1, draw_sprite_flipx_opaque_s2_d1, draw_sprite_flipx_opaque_s3_d1, draw_sprite_flipx_opaque_s4_d1, draw_sprite_flipx_opaque_s5_d1, draw_sprite_flipx_opaque_s6_d1, draw_sprite_flipx_opaque_s7_d1,
	draw_sprite_flipx_opaque_s0_d2, draw_sprite_flipx_opaque_s1_d2, draw_sprite_flipx_opaque_s2_d2, draw_sprite_flipx_opaque_s3_d2, draw_sprite_flipx_opaque_s4_d2, draw_sprite_flipx_opaque_s5_d2, draw_sprite_flipx_opaque_s6_d2, draw_sprite_flipx_opaque_s7_d2,
	draw_sprite_flipx_opaque_s0_d3, draw_sprite_flipx_opaque_s1_d3, draw_sprite_flipx_opaque_s2_d3, draw_sprite_flipx_opaque_s3_d3, draw_sprite_flipx_opaque_s4_d3, draw_sprite_flipx_opaque_s5_d3, draw_sprite_flipx_opaque_s6_d3, draw_sprite_flipx_opaque_s7_d3,
	draw_sprite_flipx_opaque_s0_d4, draw_sprite_flipx_opaque_s1_d4, draw_sprite_flipx_opaque_s2_d4, draw_sprite_flipx_opaque_s3_d4, draw_sprite_flipx_opaque_s4_d4, draw_sprite_flipx_opaque_s5_d4, draw_sprite_flipx_opaque_s6_d4, draw_sprite_flipx_opaque_s7_d4,
	draw_sprite_flipx_opaque_s0_d5, draw_sprite_flipx_opaque_s1_d5, draw_sprite_flipx_opaque_s2_d5, draw_sprite_flipx_opaque_s3_d5, draw_sprite_flipx_opaque_s4_d5, draw_sprite_flipx_opaque_s5_d5, draw_sprite_flipx_opaque_s6_d5, draw_sprite_flipx_opaque_s7_d5,
	draw_sprite_flipx_opaque_s0_d6, draw_sprite_flipx_opaque_s1_d6, draw_sprite_flipx_opaque_s2_d6, draw_sprite_flipx_opaque_s3_d6, draw_sprite_flipx_opaque_s4_d6, draw_sprite_flipx_opaque_s5_d6, draw_sprite_flipx_opaque_s6_d6, draw_sprite_flipx_opaque_s7_d6,
	draw_sprite_flipx_opaque_s0_d7, draw_sprite_flipx_opaque_s1_d7, draw_sprite_flipx_opaque_s2_d7, draw_sprite_flipx_opaque_s3_d7, draw_sprite_flipx_opaque_s4_d7, draw_sprite_flipx_opaque_s5_d7, draw_sprite_flipx_opaque_s6_d7, draw_sprite_flipx_opaque_s7_d7,
};



caveblitfunction cave_notint_blit_funcs[] =
{
	draw_sprite_notint_s0_d0, draw_sprite_notint_s1_d0, draw_sprite_notint_s2_d0, draw_sprite_notint_s3_d0, draw_sprite_notint_s4_d0, draw_sprite_notint_s5_d0, draw_sprite_notint_s6_d0, draw_sprite_notint_s7_d0,
	draw_sprite_notint_s0_d1, draw_sprite_notint_s1_d1, draw_sprite_notint_s2_d1, draw_sprite_notint_s3_d1, draw_sprite_notint_s4_d1, draw_sprite_notint_s5_d1, draw_sprite_notint_s6_d1, draw_sprite_notint_s7_d1,
	draw_sprite_notint_s0_d2, draw_sprite_notint_s1_d2, draw_sprite_notint_s2_d2, draw_sprite_notint_s3_d2, draw_sprite_notint_s4_d2, draw_sprite_notint_s5_d2, draw_sprite_notint_s6_d2, draw_sprite_notint_s7_d2,
	draw_sprite_notint_s0_d3, draw_sprite_notint_s1_d3, draw_sprite_notint_s2_d3, draw_sprite_notint_s3_d3, draw_sprite_notint_s4_d3, draw_sprite_notint_s5_d3, draw_sprite_notint_s6_d3, draw_sprite_notint_s7_d3,
	draw_sprite_notint_s0_d4, draw_sprite_notint_s1_d4, draw_sprite_notint_s2_d4, draw_sprite_notint_s3_d4, draw_sprite_notint_s4_d4, draw_sprite_notint_s5_d4, draw_sprite_notint_s6_d4, draw_sprite_notint_s7_d4,
	draw_sprite_notint_s0_d5, draw_sprite_notint_s1_d5, draw_sprite_notint_s2_d5, draw_sprite_notint_s3_d5, draw_sprite_notint_s4_d5, draw_sprite_notint_s5_d5, draw_sprite_notint_s6_d5, draw_sprite_notint_s7_d5,
	draw_sprite_notint_s0_d6, draw_sprite_notint_s1_d6, draw_sprite_notint_s2_d6, draw_sprite_notint_s3_d6, draw_sprite_notint_s4_d6, draw_sprite_notint_s5_d6, draw_sprite_notint_s6_d6, draw_sprite_notint_s7_d6,
	draw_sprite_notint_s0_d7, draw_sprite_notint_s1_d7, draw_sprite_notint_s2_d7, draw_sprite_notint_s3_d7, draw_sprite_notint_s4_d7, draw_sprite_notint_s5_d7, draw_sprite_notint_s6_d7, draw_sprite_notint_s7_d7,
};

caveblitfunction cave_notint_opaque_blit_funcs[] =
{
	draw_sprite_notint_opaque_s0_d0, draw_sprite_notint_opaque_s1_d0, draw_sprite_notint_opaque_s2_d0, draw_sprite_notint_opaque_s3_d0, draw_sprite_notint_opaque_s4_d0, draw_sprite_notint_opaque_s5_d0, draw_sprite_notint_opaque_s6_d0, draw_sprite_notint_opaque_s7_d0,
	draw_sprite_notint_opaque_s0_d1, draw_sprite_notint_opaque_s1_d1, draw_sprite_notint_opaque_s2_d1, draw_sprite_notint_opaque_s3_d1, draw_sprite_notint_opaque_s4_d1, draw_sprite_notint_opaque_s5_d1, draw_sprite_notint_opaque_s6_d1, draw_sprite_notint_opaque_s7_d1,
	draw_sprite_notint_opaque_s0_d2, draw_sprite_notint_opaque_s1_d2, draw_sprite_notint_opaque_s2_d2, draw_sprite_notint_opaque_s3_d2, draw_sprite_notint_opaque_s4_d2, draw_sprite_notint_opaque_s5_d2, draw_sprite_notint_opaque_s6_d2, draw_sprite_notint_opaque_s7_d2,
	draw_sprite_notint_opaque_s0_d3, draw_sprite_notint_opaque_s1_d3, draw_sprite_notint_opaque_s2_d3, draw_sprite_notint_opaque_s3_d3, draw_sprite_notint_opaque_s4_d3, draw_sprite_notint_opaque_s5_d3, draw_sprite_notint_opaque_s6_d3, draw_sprite_notint_opaque_s7_d3,
	draw_sprite_notint_opaque_s0_d4, draw_sprite_notint_opaque_s1_d4, draw_sprite_notint_opaque_s2_d4, draw_sprite_notint_opaque_s3_d4, draw_sprite_notint_opaque_s4_d4, draw_sprite_notint_opaque_s5_d4, draw_sprite_notint_opaque_s6_d4, draw_sprite_notint_opaque_s7_d4,
	draw_sprite_notint_opaque_s0_d5, draw_sprite_notint_opaque_s1_d5, draw_sprite_notint_opaque_s2_d5, draw_sprite_notint_opaque_s3_d5, draw_sprite_notint_opaque_s4_d5, draw_sprite_notint_opaque_s5_d5, draw_sprite_notint_opaque_s6_d5, draw_sprite_notint_opaque_s7_d5,
	draw_sprite_notint_opaque_s0_d6, draw_sprite_notint_opaque_s1_d6, draw_sprite_notint_opaque_s2_d6, draw_sprite_notint_opaque_s3_d6, draw_sprite_notint_opaque_s4_d6, draw_sprite_notint_opaque_s5_d6, draw_sprite_notint_opaque_s6_d6, draw_sprite_notint_opaque_s7_d6,
	draw_sprite_notint_opaque_s0_d7, draw_sprite_notint_opaque_s1_d7, draw_sprite_notint_opaque_s2_d7, draw_sprite_notint_opaque_s3_d7, draw_sprite_notint_opaque_s4_d7, draw_sprite_notint_opaque_s5_d7, draw_sprite_notint_opaque_s6_d7, draw_sprite_notint_opaque_s7_d7,
};

caveblitfunction cave_notint_flipx_blit_funcs[] =
{
	draw_sprite_notint_flipx_s0_d0, draw_sprite_notint_flipx_s1_d0, draw_sprite_notint_flipx_s2_d0, draw_sprite_notint_flipx_s3_d0, draw_sprite_notint_flipx_s4_d0, draw_sprite_notint_flipx_s5_d0, draw_sprite_notint_flipx_s6_d0, draw_sprite_notint_flipx_s7_d0,
	draw_sprite_notint_flipx_s0_d1, draw_sprite_notint_flipx_s1_d1, draw_sprite_notint_flipx_s2_d1, draw_sprite_notint_flipx_s3_d1, draw_sprite_notint_flipx_s4_d1, draw_sprite_notint_flipx_s5_d1, draw_sprite_notint_flipx_s6_d1, draw_sprite_notint_flipx_s7_d1,
	draw_sprite_notint_flipx_s0_d2, draw_sprite_notint_flipx_s1_d2, draw_sprite_notint_flipx_s2_d2, draw_sprite_notint_flipx_s3_d2, draw_sprite_notint_flipx_s4_d2, draw_sprite_notint_flipx_s5_d2, draw_sprite_notint_flipx_s6_d2, draw_sprite_notint_flipx_s7_d2,
	draw_sprite_notint_flipx_s0_d3, draw_sprite_notint_flipx_s1_d3, draw_sprite_notint_flipx_s2_d3, draw_sprite_notint_flipx_s3_d3, draw_sprite_notint_flipx_s4_d3, draw_sprite_notint_flipx_s5_d3, draw_sprite_notint_flipx_s6_d3, draw_sprite_notint_flipx_s7_d3,
	draw_sprite_notint_flipx_s0_d4, draw_sprite_notint_flipx_s1_d4, draw_sprite_notint_flipx_s2_d4, draw_sprite_notint_flipx_s3_d4, draw_sprite_notint_flipx_s4_d4, draw_sprite_notint_flipx_s5_d4, draw_sprite_notint_flipx_s6_d4, draw_sprite_notint_flipx_s7_d4,
	draw_sprite_notint_flipx_s0_d5, draw_sprite_notint_flipx_s1_d5, draw_sprite_notint_flipx_s2_d5, draw_sprite_notint_flipx_s3_d5, draw_sprite_notint_flipx_s4_d5, draw_sprite_notint_flipx_s5_d5, draw_sprite_notint_flipx_s6_d5, draw_sprite_notint_flipx_s7_d5,
	draw_sprite_notint_flipx_s0_d6, draw_sprite_notint_flipx_s1_d6, draw_sprite_notint_flipx_s2_d6, draw_sprite_notint_flipx_s3_d6, draw_sprite_notint_flipx_s4_d6, draw_sprite_notint_flipx_s5_d6, draw_sprite_notint_flipx_s6_d6, draw_sprite_notint_flipx_s7_d6,
	draw_sprite_notint_flipx_s0_d7, draw_sprite_notint_flipx_s1_d7, draw_sprite_notint_flipx_s2_d7, draw_sprite_notint_flipx_s3_d7, draw_sprite_notint_flipx_s4_d7, draw_sprite_notint_flipx_s5_d7, draw_sprite_notint_flipx_s6_d7, draw_sprite_notint_flipx_s7_d7,
};

caveblitfunction cave_notint_flipx_opaque_blit_funcs[] =
{
	draw_sprite_notint_flipx_opaque_s0_d0, draw_sprite_notint_flipx_opaque_s1_d0, draw_sprite_notint_flipx_opaque_s2_d0, draw_sprite_notint_flipx_opaque_s3_d0, draw_sprite_notint_flipx_opaque_s4_d0, draw_sprite_notint_flipx_opaque_s5_d0, draw_sprite_notint_flipx_opaque_s6_d0, draw_sprite_notint_flipx_opaque_s7_d0,
	draw_sprite_notint_flipx_opaque_s0_d1, draw_sprite_notint_flipx_opaque_s1_d1, draw_sprite_notint_flipx_opaque_s2_d1, draw_sprite_notint_flipx_opaque_s3_d1, draw_sprite_notint_flipx_opaque_s4_d1, draw_sprite_notint_flipx_opaque_s5_d1, draw_sprite_notint_flipx_opaque_s6_d1, draw_sprite_notint_flipx_opaque_s7_d1,
	draw_sprite_notint_flipx_opaque_s0_d2, draw_sprite_notint_flipx_opaque_s1_d2, draw_sprite_notint_flipx_opaque_s2_d2, draw_sprite_notint_flipx_opaque_s3_d2, draw_sprite_notint_flipx_opaque_s4_d2, draw_sprite_notint_flipx_opaque_s5_d2, draw_sprite_notint_flipx_opaque_s6_d2, draw_sprite_notint_flipx_opaque_s7_d2,
	draw_sprite_notint_flipx_opaque_s0_d3, draw_sprite_notint_flipx_opaque_s1_d3, draw_sprite_notint_flipx_opaque_s2_d3, draw_sprite_notint_flipx_opaque_s3_d3, draw_sprite_notint_flipx_opaque_s4_d3, draw_sprite_notint_flipx_opaque_s5_d3, draw_sprite_notint_flipx_opaque_s6_d3, draw_sprite_notint_flipx_opaque_s7_d3,
	draw_sprite_notint_flipx_opaque_s0_d4, draw_sprite_notint_flipx_opaque_s1_d4, draw_sprite_notint_flipx_opaque_s2_d4, draw_sprite_notint_flipx_opaque_s3_d4, draw_sprite_notint_flipx_opaque_s4_d4, draw_sprite_notint_flipx_opaque_s5_d4, draw_sprite_notint_flipx_opaque_s6_d4, draw_sprite_notint_flipx_opaque_s7_d4,
	draw_sprite_notint_flipx_opaque_s0_d5, draw_sprite_notint_flipx_opaque_s1_d5, draw_sprite_notint_flipx_opaque_s2_d5, draw_sprite_notint_flipx_opaque_s3_d5, draw_sprite_notint_flipx_opaque_s4_d5, draw_sprite_notint_flipx_opaque_s5_d5, draw_sprite_notint_flipx_opaque_s6_d5, draw_sprite_notint_flipx_opaque_s7_d5,
	draw_sprite_notint_flipx_opaque_s0_d6, draw_sprite_notint_flipx_opaque_s1_d6, draw_sprite_notint_flipx_opaque_s2_d6, draw_sprite_notint_flipx_opaque_s3_d6, draw_sprite_notint_flipx_opaque_s4_d6, draw_sprite_notint_flipx_opaque_s5_d6, draw_sprite_notint_flipx_opaque_s6_d6, draw_sprite_notint_flipx_opaque_s7_d6,
	draw_sprite_notint_flipx_opaque_s0_d7, draw_sprite_notint_flipx_opaque_s1_d7, draw_sprite_notint_flipx_opaque_s2_d7, draw_sprite_notint_flipx_opaque_s3_d7, draw_sprite_notint_flipx_opaque_s4_d7, draw_sprite_notint_flipx_opaque_s5_d7, draw_sprite_notint_flipx_opaque_s6_d7, draw_sprite_notint_flipx_opaque_s7_d7,
};



INLINE void cavesh_gfx_draw_shadow_copy(address_space &space, offs_t *addr, int cliptype)
{
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	UINT16 w = COPY_NEXT_WORD(space, addr);
	UINT16 h = COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);
	COPY_NEXT_WORD(space, addr);


	// todo, calcualte clipping.
	cave_blit_delay += w*h;
}



INLINE void cavesh_gfx_draw(offs_t *addr)
{
	int x, y, dimx,dimy, flipx, flipy;
	int trans, blend, s_mode, d_mode;
	clr_t tint_clr;
	int tinted = 0;

	UINT16 attr = READ_NEXT_WORD(addr);
	UINT16 alpha = READ_NEXT_WORD(addr);
	UINT16 src_x = READ_NEXT_WORD(addr);
	UINT16 src_y = READ_NEXT_WORD(addr);
	UINT16 dst_x_start = READ_NEXT_WORD(addr);
	UINT16 dst_y_start = READ_NEXT_WORD(addr);
	UINT16 w = READ_NEXT_WORD(addr);
	UINT16 h = READ_NEXT_WORD(addr);
	UINT16 tint_r = READ_NEXT_WORD(addr);
	UINT16 tint_gb = READ_NEXT_WORD(addr);

	// 0: +alpha
	// 1: +source
	// 2: +dest
	// 3: *
	// 4: -alpha
	// 5: -source
	// 6: -dest
	// 7: *

	d_mode =  attr & 0x0007;
	s_mode = (attr & 0x0070) >> 4;

	trans = attr & 0x0100;
	blend = attr & 0x0200;

	flipy =	attr & 0x0400;
	flipx =	attr & 0x0800;

	const UINT8 d_alpha = ((alpha & 0x00ff))>>3;
	const UINT8 s_alpha = ((alpha & 0xff00)>>8)>>3;

	src_x = src_x & 0x1fff;
	src_y = src_y & 0x0fff;


	x = (dst_x_start & 0x7fff) - (dst_x_start & 0x8000);
	y = (dst_y_start & 0x7fff) - (dst_y_start & 0x8000);

	dimx = (w & 0x1fff) + 1;
	dimy = (h & 0x0fff) + 1;

	// convert parameters to clr


	tint_to_clr(tint_r & 0x00ff, (tint_gb >>  8) & 0xff, tint_gb & 0xff, &tint_clr);

	/* interestingly this gets set to 0x20 for 'normal' not 0x1f */

	if (tint_clr.r!=0x20)
		tinted = 1;

	if (tint_clr.g!=0x20)
		tinted = 1;

	if (tint_clr.b!=0x20)
		tinted = 1;


	// surprisingly frequent, need to verify if it produces a worthwhile speedup tho.
	if ((s_mode==0 && s_alpha==0x1f) && (d_mode==4 && d_alpha==0x1f))
		blend = 0;

	if (tinted)
	{
		if (!flipx)
		{
			if (trans)
			{
				if (!blend)
					draw_sprite_plain(draw_params);
				else
					cave_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
			else
			{
				if (!blend)
					draw_sprite_opaque_plain(draw_params);
				else
					cave_opaque_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
		}
		else // flipx
		{
			if (trans)
			{
				if (!blend)
					draw_sprite_flipx_plain(draw_params);
				else
					cave_flipx_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
			else
			{
				if (!blend)
					draw_sprite_flipx_opaque_plain(draw_params);
				else
					cave_flipx_opaque_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
		}
	}
	else
	{

		if (blend==0 && tinted==0)
		{
			if (!flipx)
			{
				if (trans)
					draw_sprite_notint_simple(draw_params);
				else
					draw_sprite_notint_opaque_simple(draw_params);
			}
			else
			{
				if (trans)
					draw_sprite_notint_flipx_simple(draw_params);
				else
					draw_sprite_notint_flipx_opaque_simple(draw_params);

			}

			return;
		}


		if (!flipx)
		{
			if (trans)
			{
				if (!blend)
					draw_sprite_notint_plain(draw_params);
				else
					cave_notint_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
			else
			{
				if (!blend)
					draw_sprite_notint_opaque_plain(draw_params);
				else
					cave_notint_opaque_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
		}
		else // flipx
		{
			if (trans)
			{
				if (!blend)
					draw_sprite_notint_flipx_plain(draw_params);
				else
					cave_notint_flipx_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
			else
			{
				if (!blend)
					draw_sprite_notint_flipx_opaque_plain(draw_params);
				else
					cave_notint_flipx_opaque_blit_funcs[s_mode | (d_mode<<3)](draw_params);
			}
		}
	}
}


static void cavesh_gfx_create_shadow_copy(address_space &space)
{
	offs_t addr = cavesh_gfx_addr & 0x1fffffff;
	UINT16 cliptype = 0;

	cavesh_clip.set(cavesh_gfx_scroll_1_x_shadowcopy, cavesh_gfx_scroll_1_x_shadowcopy + 320-1, cavesh_gfx_scroll_1_y_shadowcopy,cavesh_gfx_scroll_1_y_shadowcopy + 240-1);

	while (1)
	{
		UINT16 data = COPY_NEXT_WORD(space, &addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = COPY_NEXT_WORD(space, &addr);

				cliptype = data ? 1 : 0;

				if (cliptype)
				{
					cavesh_clip.min_x = cavesh_gfx_scroll_1_x_shadowcopy;
					cavesh_clip.min_y = cavesh_gfx_scroll_1_y_shadowcopy;
					cavesh_clip.max_x = cavesh_clip.min_x + 320-1;
					cavesh_clip.max_y = cavesh_clip.min_y + 240-1;
				}
				else
				{
					cavesh_clip.min_x = 0;
					cavesh_clip.min_y = 0;
					cavesh_clip.max_x = 0x2000-1;
					cavesh_clip.max_y = 0x1000-1;
				}

				break;

			case 0x2000:
				addr -= 2;
				cavesh_gfx_upload_shadow_copy(space, &addr);
				break;

			case 0x1000:
				addr -= 2;
				cavesh_gfx_draw_shadow_copy(space, &addr, cliptype);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}

// Deathsmiles has bad text with wrong clip sizes, must clip to screen size.
static void cavesh_gfx_exec(void)
{
	UINT16 cliptype = 0;

	offs_t addr = cavesh_gfx_addr_shadowcopy & 0x1fffffff;

	cavesh_clip.min_x = cavesh_gfx_scroll_1_x_shadowcopy;
	cavesh_clip.min_y = cavesh_gfx_scroll_1_y_shadowcopy;
	cavesh_clip.max_x = cavesh_clip.min_x + 320-1;
	cavesh_clip.max_y = cavesh_clip.min_y + 240-1;

	while (1)
	{
		UINT16 data = READ_NEXT_WORD(&addr);

		switch( data & 0xf000 )
		{
			case 0x0000:
			case 0xf000:
				return;

			case 0xc000:
				data = READ_NEXT_WORD(&addr);
				cliptype = data ? 1 : 0;

				if (cliptype)
				{
					cavesh_clip.min_x = cavesh_gfx_scroll_1_x_shadowcopy;
					cavesh_clip.min_y = cavesh_gfx_scroll_1_y_shadowcopy;
					cavesh_clip.max_x = cavesh_clip.min_x + 320-1;
					cavesh_clip.max_y = cavesh_clip.min_y + 240-1;
				}
				else
				{
					cavesh_clip.min_x = 0;
					cavesh_clip.min_y = 0;
					cavesh_clip.max_x = 0x2000-1;
					cavesh_clip.max_y = 0x1000-1;
				}
				break;

			case 0x2000:
				addr -= 2;
				cavesh_gfx_upload(&addr);
				break;

			case 0x1000:
				addr -= 2;
				cavesh_gfx_draw(&addr);
				break;

			default:
				popmessage("GFX op = %04X", data);
				return;
		}
	}
}


static void *blit_request_callback(void *param, int threadid)
{
	cavesh_gfx_exec();
	return NULL;
}



READ32_MEMBER( cavesh3_state::cavesh_gfx_ready_r )
{
	// ideally we want a recompiler for the CPU before we attempt to do this
	// otherwise the games get stuck in more loops waiting for the blitter and we'd
	// have to add even more idle skips all over the place ;-)

	return 0x00000010;
}

WRITE32_MEMBER( cavesh3_state::cavesh_gfx_exec_w )
{
	if ( ACCESSING_BITS_0_7 )
	{
		if (data & 1)
		{
			if (this->blitter_request)
			{
				int result;
				do
				{
					result = osd_work_item_wait(this->blitter_request, 1000);
				} while (result==0);
				osd_work_item_release(this->blitter_request);
			}

			cave_blit_delay = 0;
			cavesh_gfx_create_shadow_copy(space); // create a copy of the blit list so we can safely thread it.

			if (cave_blit_delay)
			{
				this->blitter_busy = 1;
				this->cavesh3_blitter_delay_timer->adjust(attotime::from_nsec(cave_blit_delay*8)); // NOT accurate timing (currently ignored anyway)
			}

			cavesh_gfx_addr_shadowcopy = cavesh_gfx_addr;
			cavesh_gfx_scroll_0_x_shadowcopy =  cavesh_gfx_scroll_0_x;
			cavesh_gfx_scroll_0_y_shadowcopy = cavesh_gfx_scroll_0_y;
			cavesh_gfx_scroll_1_x_shadowcopy = cavesh_gfx_scroll_1_x;
			cavesh_gfx_scroll_1_y_shadowcopy = cavesh_gfx_scroll_1_y;
			this->blitter_request = osd_work_item_queue(this->queue, blit_request_callback, 0, 0);
		}
	}
}



static SCREEN_UPDATE_RGB32( cavesh3 )
{

	cavesh3_state *state = screen.machine().driver_data<cavesh3_state>();

	if (state->blitter_request)
	{
		int result;
		do
		{
			result = osd_work_item_wait(state->blitter_request, 1000);
		} while (result==0);
		osd_work_item_release(state->blitter_request);
	}


	int scroll_0_x, scroll_0_y;

	bitmap.fill(0, cliprect);

	scroll_0_x = -cavesh_gfx_scroll_0_x;
	scroll_0_y = -cavesh_gfx_scroll_0_y;

	copyscrollbitmap(bitmap, *cavesh_bitmaps, 1,&scroll_0_x, 1,&scroll_0_y, cliprect);

	return 0;
}


READ32_MEMBER( cavesh3_state::cavesh3_blitter_r )
{
	switch (offset*4)
	{
		case 0x10:
			return cavesh_gfx_ready_r(space,offset,mem_mask);

		case 0x24:
			return 0xffffffff;

		case 0x28:
			return 0xffffffff;

		case 0x50:
			return ioport("DSW")->read();

		default:
			break;

	}
	return 0;
}

WRITE32_MEMBER( cavesh3_state::cavesh3_blitter_w )
{
	switch (offset*4)
	{
		case 0x04:
			cavesh_gfx_exec_w(space,offset,data,mem_mask);
			break;

		case 0x08:
			COMBINE_DATA(&cavesh_gfx_addr);
			break;

		case 0x14:
			COMBINE_DATA(&cavesh_gfx_scroll_0_x);
			break;

		case 0x18:
			COMBINE_DATA(&cavesh_gfx_scroll_0_y);
			break;

		case 0x40:
			COMBINE_DATA(&cavesh_gfx_scroll_1_x);
			break;

		case 0x44:
			COMBINE_DATA(&cavesh_gfx_scroll_1_y);
			break;

	}
}

// FLASH

#define FLASH_PAGE_SIZE	(2048+64)

UINT8 flash_page_data[FLASH_PAGE_SIZE];

typedef enum { STATE_IDLE = 0, STATE_READ, STATE_READ_ID, STATE_READ_STATUS, STATE_BLOCK_ERASE, STATE_PAGE_PROGRAM } flash_state_t;
//static const char *flash_state_name[] = { "IDLE", "READ", "READ_ID", "READ_STATUS", "BLOCK ERASE", "PAGE PROGRAM" };

static flash_state_t flash_state;

static UINT8 flash_enab;

static UINT8 flash_cmd_seq;
static UINT32 flash_cmd_prev;

static UINT8 flash_addr_seq;
static UINT8 flash_read_seq;

static UINT16 flash_row, flash_col;
static UINT16 flash_page_addr;
static UINT16 flash_page_index;

static void flash_hard_reset(running_machine &machine)
{
	flash_state = STATE_READ;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_addr_seq = 0;
	flash_read_seq = 0;

	flash_row = 0;
	flash_col = 0;

	memset(flash_page_data, 0, FLASH_PAGE_SIZE);
	flash_page_addr = 0;
	flash_page_index = 0;
}

WRITE8_MEMBER( cavesh3_state::flash_enab_w )
{
	flash_enab = data;
}

static void flash_change_state(running_machine &machine, flash_state_t state)
{
	flash_state = state;

	flash_cmd_prev = -1;
	flash_cmd_seq = 0;

	flash_read_seq = 0;
	flash_addr_seq = 0;
}

WRITE8_MEMBER( cavesh3_state::flash_cmd_w )
{
	if (!flash_enab)
		return;

	if (flash_cmd_prev == -1)
	{
		flash_cmd_prev = data;

		switch (data)
		{
			case 0x00:	// READ
				flash_addr_seq = 0;
				break;

			case 0x60:  // BLOCK ERASE
				flash_addr_seq = 0;
				break;

			case 0x70:	// READ STATUS
				flash_change_state( machine(), STATE_READ_STATUS );
				break;

			case 0x80:	// PAGE / CACHE PROGRAM
				flash_addr_seq = 0;
				// this actually seems to be set with the next 2 writes?
				flash_page_addr = 0;
				break;

			case 0x90:	// READ ID
				flash_change_state( machine(), STATE_READ_ID );
				break;

			case 0xff:	// RESET
				flash_change_state( machine(), STATE_IDLE );
				break;

			default:
				break;
		}
	}
	else
	{
		switch (flash_cmd_prev)
		{
			case 0x00:	// READ
				if (data == 0x30)
				{
					memcpy(flash_page_data, flashregion + flash_row * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
					flash_page_addr = flash_col;
					flash_page_index = flash_row;

					flash_change_state( machine(), STATE_READ );
				}
				break;

			case 0x60: // BLOCK ERASE
				if (data==0xd0)
				{
					flash_change_state( machine(), STATE_BLOCK_ERASE );
					flashwritemap[flash_col] |= 1;
					memset(flashregion + flash_col * FLASH_PAGE_SIZE, 0xff, FLASH_PAGE_SIZE);
				}
				break;
			case 0x80:
				if (data==0x10)
				{
					flash_change_state( machine(), STATE_PAGE_PROGRAM );
					flashwritemap[flash_row] |= (memcmp(flashregion + flash_row * FLASH_PAGE_SIZE, flash_page_data, FLASH_PAGE_SIZE) != 0);
					memcpy(flashregion + flash_row * FLASH_PAGE_SIZE, flash_page_data, FLASH_PAGE_SIZE);

				}
				break;


			default:
				break;
		}
	}
}

WRITE8_MEMBER( cavesh3_state::flash_data_w ) // Deathsmiles
{
	if (!flash_enab)
		return;

	flash_page_data[flash_page_addr] = data;
	flash_page_addr++;
}

WRITE8_MEMBER( cavesh3_state::flash_addr_w )
{
	if (!flash_enab)
		return;

	switch( flash_addr_seq++ )
	{
		case 0:
			flash_col = (flash_col & 0xff00) | data;
			break;

		case 1:
			flash_col = (flash_col & 0x00ff) | (data << 8);
			break;

		case 2:
			flash_row = (flash_row & 0xff00) | data;
			break;

		case 3:
			flash_row = (flash_row & 0x00ff) | (data << 8);
			flash_addr_seq = 0;
			break;

		default:
			break;
	}
}

READ8_MEMBER( cavesh3_state::flash_io_r )
{
	UINT8 data = 0x00;

	if (!flash_enab)
		return 0xff;

	switch (flash_state)
	{
		case STATE_READ_ID:
			switch( flash_read_seq++ )
			{
				case 0:
					data = 0xEC;	// Manufacturer
					break;

				case 1:
					data = 0xF1;	// Device
					break;

				case 2:
					data = 0x00;	// XX
					break;

				case 3:
					data = 0x15;	// Flags
					flash_read_seq = 0;
					break;

				default:
					break;
			}
			break;

		case STATE_READ:
			if (flash_page_addr > FLASH_PAGE_SIZE-1)
				flash_page_addr = FLASH_PAGE_SIZE-1;
			data = flash_page_data[flash_page_addr++];
			break;

		case STATE_READ_STATUS:
			data = 0xe0;
			break;

		default:
			break;
	}

	return data;
}

/*READ8_MEMBER( cavesh3_state::flash_ready_r )
{
	return 1;
}*/

// FLASH interface

READ64_MEMBER( cavesh3_state::ibara_flash_port_e_r )
{
	//return ((flash_ready_r(space, offset) ? 0x20 : 0x00)) | 0xdf;
	//return (0x20 | 0xdf);
	return 0xff;
}


READ8_MEMBER( cavesh3_state::ibara_flash_io_r )
{
	switch (offset)
	{
		default:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			return 0xff;
		case 0x00:
			return flash_io_r(space,offset);
	}
}

WRITE8_MEMBER( cavesh3_state::ibara_flash_io_w )
{
	switch (offset)
	{
		default:
		case 0x03:
			break;

		case 0x00:
			flash_data_w(space, offset, data);
			break;

		case 0x01:
			flash_cmd_w(space, offset, data);
			break;

		case 0x2:
			flash_addr_w(space, offset, data);
			break;
	}
}



// ibarablk uses the rtc to render the clock in the first attract demo
// if this code returns bad values it has gfx corruption.  the ibarablka set doesn't do this?!
READ8_MEMBER( cavesh3_state::serial_rtc_eeprom_r )
{
	rtc9701_device* dev = machine().device<rtc9701_device>("eeprom");

	switch (offset)
	{
		case 1:
			return 0xfe | dev->read_bit();

		default:
			return 0;
	}
}




WRITE8_MEMBER( cavesh3_state::serial_rtc_eeprom_w )
{
	switch (offset)
	{
		case 0x01:
			ioport("EEPROMOUT")->write(data, 0xff);
			break;

		case 0x03:
			flash_enab_w(space,offset,data);
			return;

		default:
			break;
	}
}



/*WRITE64_MEMBER( cavesh3_state::cavesh3_nop_write )
{

}*/


static ADDRESS_MAP_START( cavesh3_map, AS_PROGRAM, 64, cavesh3_state )
	//AM_RANGE(0x00000000, 0x003fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITE( cavesh3_nop_write ) // mmmbanc writes here on startup for some reason..
	AM_RANGE(0x00000000, 0x003fffff) AM_ROM AM_REGION("maincpu", 0) AM_WRITENOP // mmmbanc writes here on startup for some reason..


	/*       0x04000000, 0x07ffffff  SH3 Internal Regs (including ports) */

	AM_RANGE(0x0c000000, 0x0cffffff) AM_RAM AM_SHARE("mainram")  //AM_BASE_LEGACY(&cavesh3_ram)// work RAM

	AM_RANGE(0x10000000, 0x10000007) AM_READWRITE8(ibara_flash_io_r, ibara_flash_io_w, U64(0xffffffffffffffff))
//	AM_RANGE(0x10400000, 0x10400007) AM_DEVREADWRITE8("ymz770", ymz770_device, read, write, U64(0xffffffffffffffff))
	AM_RANGE(0x10400000, 0x10400007) AM_DEVWRITE8("ymz770", ymz770_device, write, U64(0xffffffffffffffff))
	AM_RANGE(0x10C00000, 0x10C00007) AM_READWRITE8(serial_rtc_eeprom_r, serial_rtc_eeprom_w, U64(0xffffffffffffffff))
	AM_RANGE(0x18000000, 0x18000057) AM_READWRITE32(cavesh3_blitter_r, cavesh3_blitter_w, U64(0xffffffffffffffff))

	AM_RANGE(0xf0000000, 0xf0ffffff) AM_RAM // mem mapped cache (sh3 internal?)
	/*       0xffffe000, 0xffffffff  SH3 Internal Regs 2 */
ADDRESS_MAP_END

READ64_MEMBER( cavesh3_state::ibara_fpga_r )
{
	return 0xff;
}

/*WRITE64_MEMBER( cavesh3_state::ibara_fpga_w )
{
}*/


static ADDRESS_MAP_START( cavesh3_port, AS_IO, 64, cavesh3_state )
	AM_RANGE(SH3_PORT_C, SH3_PORT_C+7) AM_READ_PORT("PORT_C")
	AM_RANGE(SH3_PORT_D, SH3_PORT_D+7) AM_READ_PORT("PORT_D")
	AM_RANGE(SH3_PORT_E, SH3_PORT_E+7) AM_READ( ibara_flash_port_e_r )
	AM_RANGE(SH3_PORT_F, SH3_PORT_F+7) AM_READ_PORT("PORT_F")
	AM_RANGE(SH3_PORT_L, SH3_PORT_L+7) AM_READ_PORT("PORT_L")
	//AM_RANGE(SH3_PORT_J, SH3_PORT_J+7) AM_READWRITE( ibara_fpga_r, ibara_fpga_w )
	//AM_RANGE(SH3_PORT_J, SH3_PORT_J+7) AM_READWRITE( ibara_fpga_r, AM_WRITENOP )
	AM_RANGE(SH3_PORT_J, SH3_PORT_J+7) AM_READ( ibara_fpga_r )

ADDRESS_MAP_END


static INPUT_PORTS_START( cavesh3 )
	PORT_START("DSW")		// 18000050.l (18000050.b + 3 i.e. MSB + 3, is shown as DIPSW)
	PORT_DIPNAME(    0x00000002, 0x00000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00000002, DEF_STR( On ) )
	PORT_SERVICE(    0x00000001, IP_ACTIVE_HIGH )

	PORT_START("PORT_C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Test button copied here
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1  )	// IMPLEMENT COIN ERROR!
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PORT_D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(1)

	PORT_START("PORT_F")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE2 )	// Test Push Button
	PORT_BIT( 0xfd, IP_ACTIVE_LOW,  IPT_UNKNOWN )


	PORT_START("PORT_L")	// 4000134.b, 4000136.b
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4        ) PORT_PLAYER(2)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, set_clock_line)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", rtc9701_device, set_cs_line)

	PORT_START( "EEPROMIN" )

INPUT_PORTS_END



#define CAVE_CPU_CLOCK 12800000 * 8

// none of this is verified for cave sh3
// (the sh3 is different to the sh4 anyway, should be changed)
static const struct sh4_config sh4cpu_config = {
	0, // md2 (clock divders)
	0, // md1 (clock divders)
	0, // md0 (clock divders)
	0,
	0,
	0,
	1,
	1, // md7 (master?)
	0,
	CAVE_CPU_CLOCK
};

// 1166666 pixels per frame
// 1 frame is 0.01666666666666666666666666666667 seconds
// 1 frame is 16.666666666666666666666666666667 milliseconds
// 1 frame is 16666666.666666666666666666666667 nanoseconds



INTERRUPT_GEN_MEMBER(cavesh3_state::cavesh3_interrupt)
{
	device.execute().set_input_line(2, HOLD_LINE);
}


TIMER_CALLBACK_MEMBER(cavesh3_state::cavesh3_blitter_delay_callback)
{
	blitter_busy = 0;
}

MACHINE_START_MEMBER(cavesh3_state,cavesh3)
{
	size_t size = memregion( "game" )->bytes();
	cavesh3_ram = reinterpret_cast<UINT64 *>(memshare( "mainram" )->ptr());

	flashwritemap = auto_alloc_array(machine(), UINT8, size / FLASH_PAGE_SIZE);
	memset(flashwritemap, 0, size / FLASH_PAGE_SIZE);

	cavesh3_ram16_copy = auto_alloc_array(machine(), UINT16, 0x7ffffff);

	cavesh3_blitter_delay_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cavesh3_state::cavesh3_blitter_delay_callback),this));
	cavesh3_blitter_delay_timer->adjust(attotime::never);


	queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_HIGH_FREQ);
}

MACHINE_RESET_MEMBER(cavesh3_state,cavesh3)
{
	flash_enab = 0;
	flash_hard_reset(machine());
	cavesh3_ram16 = (UINT16*)cavesh3_ram;

	flashregion = memregion("game")->base();


	// cache table to avoid divides in blit code, also pre-clamped
	int x,y;
	for (y=0;y<0x40;y++)
	{
		for (x=0;x<0x20;x++)
		{
			cavesh3_colrtable[x][y] = (x*y) / 0x1f;
			if (cavesh3_colrtable[x][y]>0x1f) cavesh3_colrtable[x][y] = 0x1f;

			cavesh3_colrtable_rev[x^0x1f][y] = (x*y) / 0x1f;
			if (cavesh3_colrtable_rev[x^0x1f][y]>0x1f) cavesh3_colrtable_rev[x^0x1f][y] = 0x1f;
		}
	}

	// preclamped add table
	for (y=0;y<0x20;y++)
	{
		for (x=0;x<0x20;x++)
		{
			cavesh3_colrtable_add[x][y] = (x+y);
			if (cavesh3_colrtable_add[x][y]>0x1f) cavesh3_colrtable_add[x][y] = 0x1f;
		}
	}

	blitter_busy = 0;
}


static NVRAM_HANDLER( cavesh3 )
{
	/* Yes we have to crawl through the entire ~128MB flash because some games
       (eg. Deathsmiles) save data there on top of to the actual EEPROM */
	UINT8 *region = machine.root_device().memregion( "game" )->base();
	size_t size = machine.root_device().memregion( "game" )->bytes();
	cavesh3_state *state = machine.driver_data<cavesh3_state>();
	if (size % FLASH_PAGE_SIZE) return; // region size must be multiple of flash page size
	size /= FLASH_PAGE_SIZE;

	if (read_or_write)
	{
		UINT32 page = 0;
		while (page < size)
		{
			if (state->flashwritemap[page])
			{
				file->write(&page, 4);
				file->write(region + page * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
			}
			page++;
		}
		file->write(&page, 4);
	}
	else
	{
		if (file)
		{
			UINT32 page;
			file->read(&page, 4);
			while (page < size)
			{
				state->flashwritemap[page] = 1;
				file->read(region + page * FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
				file->read(&page, 4);
			}
		}
	}
}


static MACHINE_CONFIG_START( cavesh3, cavesh3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH3BE, CAVE_CPU_CLOCK)
	MCFG_CPU_CONFIG(sh4cpu_config)
	MCFG_CPU_PROGRAM_MAP(cavesh3_map)
	MCFG_CPU_IO_MAP(cavesh3_port)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cavesh3_state, cavesh3_interrupt)

	MCFG_RTC9701_ADD("eeprom")
	MCFG_NVRAM_HANDLER(cavesh3)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(0x200, 0x200)
	MCFG_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xf0-1)

	MCFG_PALETTE_LENGTH(0x10000)


	MCFG_SCREEN_UPDATE_STATIC(cavesh3)
	MCFG_MACHINE_START_OVERRIDE(cavesh3_state,cavesh3)
	MCFG_MACHINE_RESET_OVERRIDE(cavesh3_state,cavesh3)

	MCFG_VIDEO_START_OVERRIDE(cavesh3_state,cavesh3)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_YMZ770_ADD("ymz770", 16384000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/**************************************************

All roms are flash roms with no labels, so keep the
 version numbers attached to the roms that differ
 - roms which differ have also been prefixed with
   the MAME set names to aid readability and prevent
   accidental misloading of sets with the wrong
   CRCs which causes issues with the speedups.

**************************************************/

ROM_START( mushisam )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushisam_u4", 0x000000, 0x200000, CRC(0b5b30b2) SHA1(35fd1bb1561c30b311b4325bc8f4628f2fccd20b) ) /* (2004/10/12 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushisam_u2", 0x000000, 0x8400000, CRC(b1f826dc) SHA1(c287bd9f571d0df03d7fcbcf3c57c74ce564ab05) ) /* (2004/10/12 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( mushisama )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("mushisama_u4", 0x000000, 0x200000, CRC(9f1c7f51) SHA1(f82ae72ec03687904ca7516887080be92365a5f3) ) /* (2004/10/12 MASTER VER) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("mushisama_u2", 0x000000, 0x8400000, CRC(2cd13810) SHA1(40e45e201b60e63a060b68d4cc767eb64cfb99c2) ) /* (2004/10/12 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(138e2050) SHA1(9e86489a4e65af5efb5495adf6d4b3e01d5b2816) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(e3d05c9f) SHA1(130c3d62317da1729c85bd178bd51500edd73ada) )
ROM_END

ROM_START( espgal2 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(09c908bb) SHA1(7d6031fd3542b3e1d296ff218feb40502fd78694) ) /* (2005/11/14 MASTER VER) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(222f58c7) SHA1(d47a5085a1debd9cb8c61d88cd39e4f5036d1797) ) /* (2005/11/14 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(b9a10c22) SHA1(4561f95c6018c9716077224bfe9660e61fb84681) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c76b1ec4) SHA1(b98a53d41a995d968e0432ed824b0b06d93dcea8) )
ROM_END

ROM_START( mushitam )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u4", 0x000000, 0x200000, CRC(4a23e6c8) SHA1(d44c287bb88e6d413a8d35d75bc1b4928ad52cdf) ) /* (2005/09/09 MASTER VER) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("u2", 0x000000, 0x8400000, CRC(3f93ff82) SHA1(6f6c250aa7134016ffb288d056bc937ea311f538) ) /* (2005/09/09 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(701a912a) SHA1(85c198946fb693d99928ea2595c84ba4d9dc8157) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(6feeb9a1) SHA1(992711c80e660c32f97b343c2ce8184fddd7364e) )
ROM_END

ROM_START( futari15 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("futari15_u4", 0x000000, 0x200000, CRC(e8c5f128) SHA1(45fb8066fdbecb83fdc2e14555c460d0c652cd5f) ) /* (2006/12/8.MAST VER. 1.54.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("futari15_u2", 0x000000, 0x8400000, CRC(b9eae1fc) SHA1(410f8e7cfcbfd271b41fb4f8d049a13a3191a1f9) ) /* (2006/12/8.MAST VER. 1.54.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari15a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("futari15a_u4", 0x000000, 0x200000, CRC(a609cf89) SHA1(56752fae9f42fa852af8ee2eae79e25ec7f17953) ) /* (2006/12/8 MAST VER 1.54) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD("futari15a_u2", 0x000000, 0x8400000, CRC(b9d815f9) SHA1(6b6f668b0bbb087ffac65e4f0d8bd9d5b28eeb28) ) /* (2006/12/8 MAST VER 1.54) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP("u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP("u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futari10 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "futari10_u4", 0x000000, 0x200000, CRC(b127dca7) SHA1(e1f518bc72fc1cdf69aefa89eafa4edaf4e84778) ) /* (2006/10/23 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "futari10_u2", 0x000000, 0x8400000, CRC(78ffcd0c) SHA1(0e2937edec15ce3f5741b72ebd3bbaaefffb556e) ) /* (2006/10/23 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( futariblk )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "futariblk_u4", 0x000000, 0x200000, CRC(6db13c62) SHA1(6a53ce7f70b754936ccbb3a4674d4b2f03979644) ) /* (2007/12/11 BLACK LABEL VER) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "futariblk_u2", 0x000000, 0x8400000, CRC(08c6fd62) SHA1(e1fc386b2b0e41906c724287cbf82304297e0150) ) /* (2007/12/11 BLACK LABEL VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(39f1e1f4) SHA1(53d12f59a56df35c705408c76e6e02118da656f1) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(c631a766) SHA1(8bb6934a2f5b8a9841c3dcf85192b1743773dd8b) )
ROM_END

ROM_START( ibara )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(8e6c155d) SHA1(38ac2107dc7824836e2b4e04c7180d5ae43c9b79) ) /* (2005/03/22 MASTER VER..) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(55840976) SHA1(4982bdce84f9603adfed7a618f18bc80359ab81e) ) /* (2005/03/22 MASTER VER..) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(ee5e585d) SHA1(7eeba4ee693060e927f8c46b16e39227c6a62392) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(f0aa3cb6) SHA1(f9d137cd879e718811b2d21a0af2a9c6b7dca2f9) )
ROM_END

ROM_START( ibarablk ) /* Title screen shows (c) 2005 despite the 2006 "master" date */
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ibarablk_u4", 0x000000, 0x200000, CRC(ee1f1f77) SHA1(ac276f3955aa4dde2544af4912819a7ae6bcf8dd) ) /* (2006/02/06. MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ibarablk_u2", 0x000000, 0x8400000, CRC(5e46be44) SHA1(bed5f1bf452f2cac58747ecabec3c4392566a3a7) ) /* (2006/02/06. MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( ibarablka ) /* Title screen shows (c) 2005 despite the 2006 "master" date */
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "ibarablka_u4", 0x000000, 0x200000, CRC(a9d43839) SHA1(507696e616608c05893c7ac2814b3365e9cb0720) ) /* (2006/02/06 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "ibarablka_u2", 0x000000, 0x8400000, CRC(33400d96) SHA1(09c22b5431ac3726bf88c56efd970f56793f825a) ) /* (2006/02/06 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a436bb22) SHA1(0556e771cc02638bf8814315ba671c2d442594f1) ) /* (2006/02/06 MASTER VER.) */
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(d11ab6b6) SHA1(2132191cbe847e2560423e4545c969f21f8ff825) ) /* (2006/02/06 MASTER VER.) */
ROM_END

ROM_START( deathsml )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(1a7b98bf) SHA1(07798a4a846e5802756396b34df47d106895c1f1) ) /* (2007/10/09 MASTER VER) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d45b0698) SHA1(7077b9445f5ed4749c7f683191ccd312180fac38) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(aab718c8) SHA1(0e636c46d06151abd6f73232bc479dafcafe5327) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(83881d84) SHA1(6e2294b247dfcbf0ced155dc45c706f29052775d) )
ROM_END

ROM_START( mmpork )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x200000, CRC(d06cfa42) SHA1(5707feb4b3e5265daf5926f38c38612b24106f1f) ) /* (2007/ 4/17 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(1ee961b8) SHA1(81a2eba704ac1cf7fc44fa7c6a3f50e3570c104f) ) /* (2007/ 4/17 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4a4b36df) SHA1(5db5ce6fa47e5ca3263d4bd19315890c6d29df66) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(ce83d07b) SHA1(a5947467c8f5b7c4b0ad8e32df2ee29b787e355f) )
ROM_END

ROM_START( mmmbanc )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x0000, 0x200000, CRC(5589d8c6) SHA1(43fbdb0effe2bc0e7135698757b6ee50200aecde) ) /* (2007/06/05 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(f3b50c30) SHA1(962327798081b292b2d3fd3b7845c0197f9f2d8a) ) /* (2007/06/05 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4caaa1bf) SHA1(9b92c13eac05601da4d9bb3eb727c156974e9f0c) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(8e3a51ba) SHA1(e34cf9acb13c3d8ca6cd1306b060b1d429872abd) )
ROM_END

ROM_START( pinkswts )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pinkswts_u4", 0x0000, 0x200000, CRC(5d812c9e) SHA1(db821ec3892fd150513749d64a8b60bf147f3275) ) /* (2006/04/06 MASTER VER....) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pinkswts_u2", 0x000000, 0x8400000, CRC(92d3243a) SHA1(e9d20c62f642fb2f62ef83ed5caeee6b3f67fef9) ) /* (2006/04/06 MASTER VER....) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsa )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsa_u4", 0x0000, 0x200000, CRC(ee3339b2) SHA1(995988d370731a7074b49ce8752525dadf06a954) ) /* (2006/04/06 MASTER VER...) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pnkswtsa_u2", 0x000000, 0x8400000, CRC(829a862e) SHA1(8c0ee2a0eb33b68869252fd68aed74820a904287) ) /* (2006/04/06 MASTER VER...) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsb )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsb_u4", 0x0000, 0x200000, CRC(68bcc009) SHA1(2fef544b93c61161a37365f868b431d8262e4b21) ) /* (2006/04/06 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pnkswtsb_u2", 0x000000, 0x8400000, CRC(a5666ed9) SHA1(682e06c84990225bc6bb0c9f38b5f46c4e36b430) ) /* (2006/04/06 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( pinkswtsx )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "pnkswtsx_u4", 0x0000, 0x200000, CRC(8fe05bf0) SHA1(5cd45ac0e74690787c83d3fb383a65ed7cd47104) ) /* (2006/04/06 MASTER VER.) */
	ROM_RELOAD(0x200000,0x200000)

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "pnkswtsx_u2", 0x000000, 0x8400000, CRC(91e4deb2) SHA1(893cb10d6f805df7cb4a1bb709a3ea6de147b7e9) ) /* (2006/04/06 MASTER VER.) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(4b82d250) SHA1(ee98dbc3f791efb6d58f3945bcb2044667ae7978) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(e93f0627) SHA1(6f5ec0ade87f7fc42a58a8f125557a4d1f3f187d) )
ROM_END

ROM_START( dsmbl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(77fc5ad1) SHA1(afe044fc16e9494143c876879b033caccd08cf22) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d6b85b7a) SHA1(4674f6ad07f6a03904ca3d05060816b8fe061add) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(a9536a6a) SHA1(4b9dcaf6803b1fcfdf73ae9daabc4508fec71631) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(3b673326) SHA1(1ae847eb4e752fef1d72081d82344f0ad0537c31) )
ROM_END

ROM_START( ddpdfk )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(9976d699) SHA1(9dfe9d1daf6f638cafce8cdc5230209e2bcb7522) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(84a51a4f) SHA1(291a6279c0746d2eb8630e7d6d886043f0cfdd94) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(27032cde) SHA1(5b58d0140d72b91db4e763ca4af93060d36ac74d) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(a6178c2c) SHA1(7742ad1de2e4b0d0206ad11d467ea35da36352df) )
ROM_END

ROM_START( ddpdfk10 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u4", 0x000000, 0x400000, CRC(a3d650b2) SHA1(46a7551760e18c2cecd372c3f4be16f6600efc2c) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x8400000, "game", ROMREGION_ERASEFF)
	ROM_LOAD( "u2", 0x000000, 0x8400000, CRC(d349cb2a) SHA1(c364c36b69b93f8f62390f185d044f51056669ff) ) /* (2007/10/09 MASTER VER) */

	ROM_REGION( 0x800000, "ymz770", ROMREGION_ERASEFF)
	ROM_LOAD16_WORD_SWAP( "u23", 0x000000, 0x400000, CRC(27032cde) SHA1(5b58d0140d72b91db4e763ca4af93060d36ac74d) )
	ROM_LOAD16_WORD_SWAP( "u24", 0x400000, 0x400000, CRC(a6178c2c) SHA1(7742ad1de2e4b0d0206ad11d467ea35da36352df) )
ROM_END

READ64_MEMBER(cavesh3_state::mushisam_speedup_r)
{
	int pc = space.device().safe_pc();
	if ( pc == 0xc04a0aa ) space.device().execute().spin_until_time(attotime::from_usec(10)); // mushisam
	else if (pc == 0xc04a0da) space.device().execute().spin_until_time(attotime::from_usec(10)); // mushitam
	return cavesh3_ram[0x0022f0/8];
}

DRIVER_INIT_MEMBER( cavesh3_state,mushisam )
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc0022f0, 0xc0022f7, read64_delegate(FUNC(cavesh3_state::mushisam_speedup_r),this));
}

READ64_MEMBER(cavesh3_state::mushisama_speedup_r)
{
	if ( space.device().safe_pc()== 0xc04a2aa ) space.device().execute().spin_until_time(attotime::from_usec(10)); // mushisam
	return cavesh3_ram[0x00024d8/8];
}

DRIVER_INIT_MEMBER( cavesh3_state,mushisama )
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc0024d8, 0xc0024df, read64_delegate(FUNC(cavesh3_state::mushisama_speedup_r),this));
}

READ64_MEMBER(cavesh3_state::espgal2_speedup_r)
{
	int pc = space.device().safe_pc();

	if ( pc == 0xc05177a ) space.device().execute().spin_until_time(attotime::from_usec(10)); // espgal2
	if ( pc == 0xc05176a ) space.device().execute().spin_until_time(attotime::from_usec(10)); // futari15 / futari15a / futari10 / futariblk / ibarablk / ibarablka / mmpork / mmmbanc
	if ( pc == 0xc0519a2 ) space.device().execute().spin_until_time(attotime::from_usec(10)); // deathsml
	if ( pc == 0xc1d1346 ) space.device().execute().spin_until_time(attotime::from_usec(10)); // dsmbl
	return cavesh3_ram[0x002310/8];
}

DRIVER_INIT_MEMBER( cavesh3_state,espgal2 )
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc002310, 0xc002317, read64_delegate(FUNC(cavesh3_state::espgal2_speedup_r),this));
}

READ64_MEMBER(cavesh3_state::ddpdfk_speedup_r)
{
	space.device().execute().spin_until_time(attotime::from_usec(10));
	return cavesh3_ram[0x002310/8];
}

DRIVER_INIT_MEMBER( cavesh3_state,ddpdfk )
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc002310, 0xc002317, read64_delegate(FUNC(cavesh3_state::ddpdfk_speedup_r),this));
}


GAME( 2004, mushisam,   0,        cavesh3, cavesh3, cavesh3_state, mushisam,  ROT270, "Cave", "Mushihime-Sama (2004/10/12 MASTER VER.)",                        0 )
GAME( 2004, mushisama,  mushisam, cavesh3, cavesh3, cavesh3_state, mushisama, ROT270, "Cave", "Mushihime-Sama (2004/10/12 MASTER VER)",                         0 )
GAME( 2005, espgal2,    0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Espgaluda II (2005/11/14 MASTER VER)",                           0 )
GAME( 2005, ibara,      0,        cavesh3, cavesh3, cavesh3_state, mushisam,  ROT270, "Cave", "Ibara (2005/03/22 MASTER VER..)",                                0 )
GAME( 2006, ibarablk,   0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Ibara Kuro Black Label (2006/02/06. MASTER VER.)",               0 )
GAME( 2006, ibarablka,  ibarablk, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Ibara Kuro Black Label (2006/02/06 MASTER VER.)",                0 )
GAME( 2005, mushitam,   0,        cavesh3, cavesh3, cavesh3_state, mushisam,  ROT0,   "Cave", "Puzzle! Mushihime-Tama (2005/09/09 MASTER VER)",                 0 )
GAME( 2006, futari15,   0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Mushihime-Sama Futari Ver 1.5 (2006/12/8.MASTER VER. 1.54.)",    0 )
GAME( 2006, futari15a,  futari15, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Mushihime-Sama Futari Ver 1.5 (2006/12/8 MASTER VER 1.54)",      0 )
GAME( 2006, futari10,   futari15, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Mushihime-Sama Futari Ver 1.0 (2006/10/23 MASTER VER.)",         0 )
GAME( 2007, futariblk,  futari15, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Mushihime-Sama Futari Black Label (2007/12/11 BLACK LABEL VER)", 0 )
GAME( 2006, pinkswts,   0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER....)",        0 )
GAME( 2006, pinkswtsa,  pinkswts, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER...)",         0 )
GAME( 2006, pinkswtsb,  pinkswts, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Pink Sweets: Ibara Sorekara (2006/04/06 MASTER VER.)",           0 )
GAME( 2006, pinkswtsx,  pinkswts, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Pink Sweets: Ibara Sorekara (2006/xx/xx MASTER VER.)",           0 )
GAME( 2007, deathsml,   0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT0,   "Cave", "Deathsmiles (2007/10/09 MASTER VER)",                            0 )
GAME( 2007, mmpork,     0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT270, "Cave", "Muchi Muchi Pork! (2007/ 4/17 MASTER VER.)",                     0 )
GAME( 2007, mmmbanc,    0,        cavesh3, cavesh3, cavesh3_state, espgal2,   ROT0,   "Cave", "Medal Mahjong Moukari Bancho (2007/06/05 MASTER VER.)",          GAME_NOT_WORKING )
GAME( 2008, dsmbl,      deathsml, cavesh3, cavesh3, cavesh3_state, espgal2,   ROT0,   "Cave", "Deathsmiles Mega Black Label (2008/10/06 MEGABLACK LABEL VER)",  0 )
GAME( 2008, ddpdfk,     0,        cavesh3, cavesh3, cavesh3_state, ddpdfk,    ROT270, "Cave", "Do-Don-Pachi Dai-Fukkatsu 1.5 (2008/06/23 MASTER VER)",          0 )
GAME( 2008, ddpdfk10,   ddpdfk,   cavesh3, cavesh3, cavesh3_state, ddpdfk,    ROT270, "Cave", "Do-Don-Pachi Dai-Fukkatsu 1.0 (2008/05/16 MASTER VER)",          0 )

/*

Known versions of games on this hardware (* denotes undumped):

Mushihime-Sama
  "2004/10/12 MASTER VER"  - broken
  "2004/10/12 MASTER VER." - fixed 1
* "2004/10/12.MASTER VER." - fixed 2

Puzzle! Mushihime-Tama
  "2005/09/09 MASTER VER"

Espgaluda II
  "2005/11/14 MASTER VER"

Ibara
  "2005/03/22 MASTER VER.."

Ibara Kuro Black Label
  "2006/02/06 MASTER VER."
  "2006/02/06. MASTER VER."

Pink Sweets: Ibara Sorekara
  "2006/04/06 MASTER VER."
  "2006/04/06 MASTER VER..."
  "2006/04/06 MASTER VER...."
* "2006/05/18 MASTER VER."    - Removes infinite lives trick, fixes some bugs
* "2006/xx/xx MASTER VER"
  "2006/xx/xx MASTER VER."

Mushihime-Sama Futari Ver 1.0
* "2006/10/23 MASTER VER"  - Ultra unlockable
  "2006/10/23 MASTER VER." - Ultra unlockable
* "2006/10/23.MASTER VER." - Cannot unlock ultra

Mushihime-Sama Futari Ver 1.5
  "2006/12/8 MASTER VER 1.54"
  "2006/12/8.MASTER VER.1.54."

Mushihime-Sama Futari Black Label
  "2007/12/11 BLACK LABEL VER"
* "2009/11/17 INTERNATIONAL BL"  ("Another Ver" on title screen)

Muchi Muchi Pork!
  "2007/ 4/17 MASTER VER."
* "2007/ 4/17 MASTER VER.."

Medal Mahjong Moukari Bancho
  "2007/06/05 MASTER VER."

Deathsmiles
  "2007/10/09 MASTER VER"

Deathsmiles Mega Black Label
* "2008/10/06 MEGABLACK LABEL VER"

Do-Don-Pachi Dai-Fukkatsu Ver 1.0
* "2008/05/16 MASTER VER"

Do-Don-Pachi Dai-Fukkatsu Ver 1.5
* "2008/06/23 MASTER VER 1.5"

Do-Don-Pachi Dai-Fukkatsu Black Label
* "2010/1/18 BLACK LABEL"

Akai Katana
* "2010/ 8/13 MASTER VER."
*  Home/Limited version, unknown date line, different gameplay from regular version, doesn't accept coins - permanent freeplay

Mushihime-Sama Ver 1.5 Matsuri Version
* 2011/5/23 CAVEMATSURI VER 1.5


--- Titles below are known versions from Cave's fan festivals and are likely in house only versions. Dates are the Cave festival shown ---

Shown     Title                                                   Notes
-----------------------------------------------------------------------------------------------------------------------
12/28/06  Mushihime-Sama Ver 1.5 ~Blue Label~                     No Maniac mode.
12/28/06  Mushihime-Sama Futari Ver 1.5 ~Red Label~               3 PCBs were given away to top scorers.
12/28/06  Mushihime-Sama Futari Ver 1.01
12/30/07  Muchi Muchi Pork! Dai Cave Festival ~Special Version~   Includes Pink Sweets bosses at the end of each stage.
12/30/07  Deathsmiles: Dai Cave Festival ~Special Version~        Inclueds a rank 99 difficulty. Prototype for DSMBL.
08/15/08  Do-Don-Pachi Dai-Fukkatsu Ver 1.5 ~Special~             Power-style only.
10/20/10  Do-Don-Pachi Dai-Fukkatsu Ver 1.51

*/
