/***************************************************************************

  gba.c

  Driver file to handle emulation of the Nintendo Game Boy Advance.

  By R. Belmont & Ryan Holtz

  TODO:
  - Roz sprites
  - Roz BG support
  - Probably some ARM7 CPU bugs along the way

***************************************************************************/

#include "driver.h"
#include "state.h"
#include "video/generic.h"
#include "cpu/arm7/arm7core.h"
#include "devices/cartslot.h"
#include "includes/gba.h"
#include "includes/gb.h"
#include "sound/dac.h"

#define VERBOSE_LEVEL	(2)
#define DISABLE_ROZ	(0)


static emu_timer *dma_timer[4], *tmr_timer[4], *irq_timer;

static UINT32 gba_sram[0x10000/4];
static UINT8 gba_eeprom[0x2000];
static UINT32 gba_flash64k[0x10000/4];
static UINT16 mosaic_offset[16][4096];

INLINE void verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		cpuintrf_push_context(0);
		logerror( "%08x: %s", activecpu_get_pc(), buf );
		cpuintrf_pop_context();
	}
}

static UINT32 *pram;
static UINT32 *vram;
static UINT32 *oam;

static UINT32 timer_clks[4] = { 16777216, 16777216/64, 16777216/256, 16777216/1024 };

static UINT32 dma_regs[16];
static UINT32 dma_src[4], dma_dst[4], dma_cnt[4], dma_srcadd[4], dma_dstadd[4];
static UINT32 timer_regs[4];
static UINT16 timer_reload[4];

typedef struct
{
	UINT16 DISPCNT,	GRNSWAP, DISPSTAT;
	UINT16 BG0CNT, BG1CNT, BG2CNT, BG3CNT;
	UINT16 BG0HOFS, BG0VOFS, BG1HOFS, BG1VOFS, BG2HOFS, BG2VOFS, BG3HOFS, BG3VOFS;
	UINT16 BG2PA, BG2PB, BG2PC, BG2PD, BG2X, BG2Y, BG3PA, BG3PB, BG3PC, BG3PD, BG3X, BG3Y;
	UINT16 WIN0H, WIN1H, WIN0V, WIN1V, WININ, WINOUT;
	UINT16 MOSAIC;
	UINT16 BLDCNT;
	UINT16 BLDALPHA;
	UINT16 BLDY;
	UINT8  SOUNDCNT_X;
	UINT16 SOUNDCNT_H;
	UINT16 SOUNDBIAS;
	UINT16 SIOMULTI0, SIOMULTI1, SIOMULTI2, SIOMULTI3;
	UINT16 SIOCNT, SIODATA8;
	UINT16 KEYCNT;
	UINT16 RCNT;
	UINT16 JOYCNT;
	UINT32 JOY_RECV, JOY_TRANS;
	UINT16 JOYSTAT;
	UINT16 IR, IE, IF, IME;
	UINT16 WAITCNT;
	UINT8  POSTFLG;
	UINT8  HALTCNT;
} GBARegsT;

static GBARegsT gba;
static UINT8 *nvptr;
static UINT32 nvsize = 0;

static void gba_machine_stop(running_machine *machine)
{
	// only do this if the cart loader detected some form of backup
	if (nvsize > 0)
	{
		image_battery_save(image_from_devtype_and_index(IO_CARTSLOT, 0), nvptr, nvsize);
	}
}

static PALETTE_INIT( gba )
{
	UINT8 r, g, b;
	for( b = 0; b < 32; b++ )
	{
		for( g = 0; g < 32; g++ )
		{
			for( r = 0; r < 32; r++ )
			{
				palette_set_color_rgb( machine, ( b << 10 ) | ( g << 5 ) | r, pal5bit(r), pal5bit(g), pal5bit(b) );
			}
		}
	}
}

static UINT8 temp;

void draw_4bpp_tile(UINT16 *scanline, UINT32 vrambase, UINT16 tilenum, int scnx, int tiley, int vflip, int hflip, int pal, int mosX)
{
	int pixx, yofs;
	UINT8 *pvram = (UINT8 *)&vram[vrambase>>2], pixel;
	UINT16 *ppram = (UINT16 *)pram;

	if (vflip)
	{
		yofs = 28-(tiley*4);
	}
	else
	{
		yofs = tiley*4;
	}

	if (hflip)
	{
		for (pixx = 0; pixx < 8; pixx+= 2)
		{
			pixel = pvram[(tilenum*32)+(3-(pixx/2))+yofs];

			if (pixel & 0xf0)
			{
				scanline[scnx+pixx] = ppram[(pixel>>4)+pal]&0x7fff;
			}
			if (pixel & 0x0f)
			{
				scanline[scnx+pixx+1] = ppram[(pixel&0xf)+pal]&0x7fff;
			}
		}
	}
	else
	{
		for (pixx = 0; pixx < 8; pixx+= 2)
		{
			pixel = pvram[(tilenum*32)+(pixx/2)+yofs];

			if (pixel & 0x0f)
			{
				scanline[scnx+pixx] = ppram[(pixel&0xf)+pal]&0x7fff;
			}
			if (pixel & 0xf0)
			{
				scanline[scnx+pixx+1] = ppram[(pixel>>4)+pal]&0x7fff;
			}
		}
	}
}

void draw_8bpp_tile(UINT16 *scanline, UINT32 vrambase, UINT16 tilenum, int scnx, int tiley, int vflip, int hflip, int pal, int mosX)
{
	int pixx, yofs;
	UINT8 *pvram = (UINT8 *)&vram[vrambase>>2], pixel;
	UINT16 *ppram = (UINT16 *)pram;

	if (vflip)
	{
		yofs = 56-(tiley*8);
	}
	else
	{
		yofs = tiley*8;
	}

	if (hflip)
	{
		for (pixx = 0; pixx < 8; pixx++)
		{
			pixel = pvram[(tilenum*32)+(7-pixx)+yofs];

			if (pixel)
			{
				scanline[scnx+pixx] = ppram[pixel+pal]&0x7fff;
			}
		}
	}
	else
	{
		for (pixx = 0; pixx < 8; pixx++)
		{
			pixel = pvram[(tilenum*32)+pixx+yofs];

			if (pixel)
			{
				scanline[scnx+pixx] = ppram[pixel+pal]&0x7fff;
			}
		}
	}
}

void draw_roz_scanline(UINT16 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, INT32 X, INT32 Y, INT32 PA, INT32 PB, INT32 PC, INT32 PD, int priority)
{
#if !DISABLE_ROZ
	UINT32 base, mapbase, size, ovr;
	INT32 sizes[4] = { 128, 256, 512, 1024 };
	INT32 cx, cy, psx, psy, x, px, py;
	UINT8 *mvram = (UINT8 *)vram, tile;
	UINT16 *ppram = (UINT16 *)pram;
	UINT16 pixel;

	if ((gba.DISPCNT & enablemask) && ((ctrl & 3) == priority))
	{
		base = ((ctrl>>2)&3) * 0x4000;		// VRAM base of tiles
		mapbase = ((ctrl>>8)&0x1f) * 0x800;	// VRAM base of map
		size = ((ctrl>>14) & 3);		// size of map in submaps
		ovr = ((ctrl >>13) & 1);

		// sign extend roz parameters
		if (X & 0x08000000) X |= 0xf0000000;
		if (Y & 0x08000000) Y |= 0xf0000000;
		if (PA & 0x8000) PA |= 0xffff0000;
		if (PB & 0x8000) PB |= 0xffff0000;
		if (PC & 0x8000) PC |= 0xffff0000;
		if (PD & 0x8000) PD |= 0xffff0000;

		cx = cy = sizes[size] / 2;

//		psx = ((PA * (X - cx)) & ~63) + ((PB * (Y - cy)) & ~63) + ((PB * ypos) & ~63) + (cx << 8); //pixel start x
//		psy = ((PC * (X - cx)) & ~63) + ((PD * (Y - cy)) & ~63) + ((PD * ypos) & ~63) + (cy << 8); //pixel start y		
		psx = ((PA * (X - cx))) + ((PB * (Y - cy))) + ((PB * ypos)) + (cx << 8); //pixel start x
		psy = ((PC * (X - cx))) + ((PD * (Y - cy))) + ((PD * ypos)) + (cy << 8); //pixel start y		


		/*

 x2 = A*(x1-x0) + B*(y1-y0) + x0
 y2 = C*(x1-x0) + D*(y1-y0) + y0

 x2-x0 = A*(x1-x0) + B*(y1-y0)
 y2-y0 = C*(x1-x0) + D*(y1-y0)
		*/

		for (x = 0; x < 240; x++)
		{
			px = psx + (PA * x); //pixel x position
			py = psy + (PC * x); //pixel y position

			// mask 'floating-point' bits (low 8 bits)
			px >>= 8;
			py >>= 8;

			// do infinite repeat for now
			px %= sizes[size];
			py %= sizes[size];

			// px and py are now offsets into the tilemap
			tile = mvram[mapbase + (px/8) + ((py/8)*sizes[size]/8)];

			// get the pixel
		     	pixel = mvram[(tile*64) + (px%8) + ((py%8)*8)];

			// plot it
			if (pixel)
			{
				scanline[x] = ppram[pixel]&0x7fff;
			}
		}		
	}
#endif
}

void draw_bg_scanline(UINT16 *scanline, int ypos, UINT32 enablemask, UINT32 ctrl, UINT32 hofs, UINT32 vofs, int priority)
{
	UINT32 base, mapbase, size, mode;
	UINT16 *mvram = (UINT16 *)vram, tile;
	int mx, my, tx, mosX, mosY;

	if (ctrl & 0x40)	// mosaic
	{
		mosX = gba.MOSAIC & 0xf;
		mosY = (gba.MOSAIC>>4) & 0xf;

		ypos = mosaic_offset[mosY][ypos];
	}
	else
	{
		mosX = mosY = 0;
	}

	if ((gba.DISPCNT & enablemask) && ((ctrl & 3) == priority))
	{
		base = ((ctrl>>2)&3) * 0x4000;		// VRAM base of tiles
		mapbase = ((ctrl>>8)&0x1f) * 0x800;	// VRAM base of map
		size = ((ctrl>>14) & 3);		// size of map in submaps
		mode = (gba.BG0CNT & 0x80) ? 1 : 0;	// 1 for 8bpp, 0 for 4bpp

		mx = hofs/8;		// X tile offset
		my = (ypos + vofs) / 8;	// Y tile offset

		switch (size)
		{
			case 0:	// 32x32
				for (tx = 0; tx < 31; tx++)
				{
					tile = mvram[(mapbase>>1) + ((tx + mx) % 32) + ((my % 32) * 32)];

					if (mode)
					{
						draw_8bpp_tile(scanline, base, (tile&0x3ff)<<1, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, 0, mosX);
					}
					else
					{
						draw_4bpp_tile(scanline, base, tile&0x3ff, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, ((tile&0xf000)>>8), mosX);
					}
				}
				break;

			case 1:	// 64x32
				for (tx = 0; tx < 31; tx++)
				{
					int mapxofs = ((tx + mx) % 64);

					if (mapxofs >= 32)
					{
						mapxofs %= 32;
						mapxofs += (32*32);	// offset to reach the second page of the tilemap
					}

					tile = mvram[(mapbase>>1) + mapxofs + ((my % 32) * 32)];

					if (mode)
					{
						draw_8bpp_tile(scanline, base, (tile&0x3ff)<<1, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, 0, mosX);
					}
					else
					{
						draw_4bpp_tile(scanline, base, tile&0x3ff, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, ((tile&0xf000)>>8), mosX);
					}
				}
				break;

			case 2:	// 32x64
				for (tx = 0; tx < 31; tx++)
				{
					int mapyofs = (my % 64);

					if (mapyofs >= 32)
					{
						mapyofs %= 32;
						mapyofs += 32;
					}

					tile = mvram[(mapbase>>1) + ((tx + mx) % 32) + (mapyofs * 32)];

					if (mode)
					{
						draw_8bpp_tile(scanline, base, (tile&0x3ff)<<1, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, 0, mosX);
					}
					else
					{
						draw_4bpp_tile(scanline, base, tile&0x3ff, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, ((tile&0xf000)>>8), mosX);
					}
				}
				break;

			case 3: // 64x64
			#if 0
				if (1)
				{
					int mapyofs = (my % 64);

					if (mapyofs >= 32)
					{
						mapyofs %= 32;
						mapyofs += 64;
					}

					printf("%d yofs %d (ypos %d vofs %d my %d) tileofs %x\n", video_screen_get_vpos(machine->primary_screen), mapyofs, ypos, vofs, my, (mapbase>>1) + (mapyofs * 32));
				}
			#endif

				for (tx = 0; tx < 31; tx++)
				{
					int mapxofs = ((tx + mx) % 64);
					int mapyofs = (my % 64);

					if (mapyofs >= 32)
					{
						mapyofs %= 32;
						mapyofs += 64;
					}

					if (mapxofs >= 32)
					{
						mapxofs %= 32;
						mapxofs += (32*32);	// offset to reach the second/fourth page of the tilemap
					}

					tile = mvram[(mapbase>>1) + mapxofs + (mapyofs * 32)];

					if (mode)
					{
						draw_8bpp_tile(scanline, base, (tile&0x3ff)<<1, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, 0, mosX);
					}
					else
					{
						draw_4bpp_tile(scanline, base, tile&0x3ff, (tx*8)-(hofs % 8), (ypos + vofs) % 8, (tile&0x800)?1:0, (tile&0x400)?1:0, ((tile&0xf000)>>8), mosX);
					}
				}
				break;
		}

	}
}

void draw_mode3_scanline(UINT16 *scanline, int y)
{
	int x;
	UINT16 *pvram = (UINT16 *)vram;

	for (x = 0; x < 240; x++)
	{
		scanline[x] = pvram[(240*y)+x]&0x7fff;
	}
}

void draw_mode4_scanline(UINT16 *scanline, int y)
{
	UINT32 base;
	int x;
	UINT8 *pvram = (UINT8 *)vram, pixel;
	UINT16 *ppram = (UINT16 *)pram;

	if (gba.DISPCNT & DISPCNT_FRAMESEL)
	{
		base = 0xa000;
	}
	else
	{
		base = 0;
	}

	for (x = 0; x < 240; x++)
	{
		pixel = pvram[base+(240*y)+x];
		if (pixel > 0)
		{
			scanline[x] = ppram[pixel]&0x7fff;
		}
	}
}

void draw_mode5_scanline(UINT16 *scanline, int y)
{
	UINT32 base;
	int x;
	UINT16 *pvram = (UINT16 *)vram;

	if (gba.DISPCNT & DISPCNT_FRAMESEL)
	{
		base = 0xa000>>1;
	}
	else
	{
		base = 0;
	}

	for (x = 0; x < 160; x++)
	{
		scanline[x] = pvram[base+(160*y)+x];
	}
}

static void draw_oam(UINT16 *scanline, int y, int priority)
{
	INT16 oamindex;
	UINT8 tilex, tiley;
	UINT32 tilebytebase, tileindex, tiledrawindex;
	UINT8 width, height;
	UINT16 *poam = (UINT16 *)oam;

	if( gba.DISPCNT & DISPCNT_OBJ_EN )
	{
		for( oamindex = 127; oamindex >= 0; oamindex-- )
		{
			UINT16 attr0, attr1, attr2;

			attr0 = poam[(4*oamindex)+0];
			attr1 = poam[(4*oamindex)+1];
			attr2 = poam[(4*oamindex)+2];

			if (((attr0 & OBJ_MODE) != OBJ_MODE_WINDOW) && (((attr2 >> 10) & 3) == priority))
			{
				width = 0;
				height = 0;
				switch (attr0 & OBJ_SHAPE )
				{
					case OBJ_SHAPE_SQR:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 2;
								height = 2;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 8;
								break;
						}
						break;
					case OBJ_SHAPE_HORIZ:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 2;
								height = 1;
								break;
							case OBJ_SIZE_16:
								width = 4;
								height = 1;
								break;
							case OBJ_SIZE_32:
								width = 4;
								height = 2;
								break;
							case OBJ_SIZE_64:
								width = 8;
								height = 4;
								break;
						}
						break;
					case OBJ_SHAPE_VERT:
						switch(attr1 & OBJ_SIZE )
						{
							case OBJ_SIZE_8:
								width = 1;
								height = 2;
								break;
							case OBJ_SIZE_16:
								width = 1;
								height = 4;
								break;
							case OBJ_SIZE_32:
								width = 2;
								height = 4;
								break;
							case OBJ_SIZE_64:
								width = 4;
								height = 8;
								break;
						}
						break;
					default:
						width = 0;
						height = 0;
						verboselog( 0, "OAM error: Trying to draw OBJ with OBJ_SHAPE = 3!\n" );
						break;
				}

				tiledrawindex = tileindex = (attr2 & OBJ_TILENUM);
				tilebytebase = 0x10000;	// the index doesn't change in the higher modes, we just ignore sprites that are out of range

 				if (attr0 & OBJ_USEROZ)
				{
				 	// don't draw ROZ sprites for now
//					printf("OAM: ROZ\n");
				}
				else
				{
					INT16 sx, sy;
					int vflip, hflip;

					if (attr0 & OBJ_DISABLE)
					{
						continue;
					}

					sx = (attr1 & OBJ_X_COORD);
					sy = (attr0 & OBJ_Y_COORD);
					vflip = (attr1 & OBJ_VFLIP) ? 1 : 0;
					hflip = (attr1 & OBJ_HFLIP) ? 1 : 0;

					if (sy > 160)
					{
						sy = 0 - (255-sy);
					}

					if (sx > 256)
					{
						sx = 0 - (511-sx);
					}

					// on this scanline?
					if ((y >= sy) && (y < (sy+(height*8))))
					{
						// Y = scanline we're drawing
						// SY = starting Y position of sprite
						if (vflip)
						{
							tiley = (sy+(height*8)-1) - y;
						}
						else
						{
							tiley = y - sy;
						}

						if (( gba.DISPCNT & DISPCNT_VRAM_MAP ) == DISPCNT_VRAM_MAP_2D)
						{
							tiledrawindex = (tileindex + ((tiley/8) * 32));
						}
						else
						{
							if ((attr0 & OBJ_PALMODE) == OBJ_PALMODE_16)
							{
								tiledrawindex = (tileindex + ((tiley/8) * width));
							}
							else
							{
								tiledrawindex = (tileindex + ((tiley/8) * (width*2)));
							}
						}

						if (hflip)
						{
							tiledrawindex += (width-1);
							for (tilex = 0; tilex < width; tilex++)
							{
								if ((attr0 & OBJ_PALMODE) == OBJ_PALMODE_16)
								{
									draw_4bpp_tile(scanline, tilebytebase, tiledrawindex, sx+(tilex*8), tiley % 8, 0, hflip, 256+((attr2&0xf000)>>8), 0);
								}
								else
								{
									draw_8bpp_tile(scanline, tilebytebase, tiledrawindex, sx+(tilex*8), tiley % 8, 0, hflip, 256, 0);
									tiledrawindex++;
								}
								tiledrawindex--;
							}
						}
						else
						{
							for (tilex = 0; tilex < width; tilex++)
							{
								if ((attr0 & OBJ_PALMODE) == OBJ_PALMODE_16)
								{
									draw_4bpp_tile(scanline, tilebytebase, tiledrawindex, sx+(tilex*8), tiley % 8, 0, hflip, 256+((attr2&0xf000)>>8), 0);
								}
								else
								{
									draw_8bpp_tile(scanline, tilebytebase, tiledrawindex, sx+(tilex*8), tiley % 8, 0, hflip, 256, 0);
									tiledrawindex++;
								}
								tiledrawindex++;
							}
						}
					}
				}
			}
		}
	}
}

void gba_draw_scanline(int y)
{
	UINT16 *ppram = (UINT16 *)pram, *scanline;
	int prio;
	bitmap_t *bitmap = tmpbitmap;
	int i;
	static UINT16 xferscan[240+2048];	// up to 1024 pixels of slop on either side to allow easier clip handling

	scanline = BITMAP_ADDR16(bitmap, y, 0);

	// forced blank
	if (gba.DISPCNT & DISPCNT_BLANK)
	{
		// forced blank is white
		for (i = 0; i < 240; i++)
		{
			scanline[i] = 0x7fff;
		}
		return;
	}

	// BG color
	for (i = 0; i < 240; i++)
	{
		xferscan[1024+i] = ppram[0]&0x7fff;
	}

//	printf("mode %d\n", (gba.DISPCNT & 7));

	switch (gba.DISPCNT & 7)
	{
		case 0:
			for (prio = 3; prio >= 0; prio--)
			{
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG3_EN, gba.BG3CNT, gba.BG3HOFS, gba.BG3VOFS, prio);
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG2_EN, gba.BG2CNT, gba.BG2HOFS, gba.BG2VOFS, prio);
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG1_EN, gba.BG1CNT, gba.BG1HOFS, gba.BG1VOFS, prio);
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG0_EN, gba.BG0CNT, gba.BG0HOFS, gba.BG0VOFS, prio);
				draw_oam(&xferscan[1024], y, prio);
			}
			break;

		case 1:
			for (prio = 3; prio >= 0; prio--)
			{
				draw_roz_scanline(&xferscan[1024], y, DISPCNT_BG2_EN, gba.BG2CNT, gba.BG2X, gba.BG2Y, gba.BG2PA, gba.BG2PB, gba.BG2PC, gba.BG2PD, prio);
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG1_EN, gba.BG1CNT, gba.BG1HOFS, gba.BG1VOFS, prio);
				draw_bg_scanline(&xferscan[1024], y, DISPCNT_BG0_EN, gba.BG0CNT, gba.BG0HOFS, gba.BG0VOFS, prio);
				draw_oam(&xferscan[1024], y, prio);
			}
			break;

		case 2:
			for (prio = 3; prio >= 0; prio--)
			{
				draw_roz_scanline(&xferscan[1024], y, DISPCNT_BG3_EN, gba.BG3CNT, gba.BG3X, gba.BG3Y, gba.BG3PA, gba.BG3PB, gba.BG3PC, gba.BG3PD, prio);
				draw_roz_scanline(&xferscan[1024], y, DISPCNT_BG2_EN, gba.BG2CNT, gba.BG2X, gba.BG2Y, gba.BG2PA, gba.BG2PB, gba.BG2PC, gba.BG2PD, prio);
				draw_oam(&xferscan[1024], y, prio);
			}
			break;

		case 3:
			draw_mode3_scanline(&xferscan[1024], y);
			draw_oam(&xferscan[1024], y, 3);
			draw_oam(&xferscan[1024], y, 2);
			draw_oam(&xferscan[1024], y, 1);
			draw_oam(&xferscan[1024], y, 0);
			break;

		case 4:
			draw_mode4_scanline(&xferscan[1024], y);
			draw_oam(&xferscan[1024], y, 3);
			draw_oam(&xferscan[1024], y, 2);
			draw_oam(&xferscan[1024], y, 1);
			draw_oam(&xferscan[1024], y, 0);
			break;

		case 5:
			draw_mode5_scanline(&xferscan[1024], y);
			draw_oam(&xferscan[1024], y, 3);
			draw_oam(&xferscan[1024], y, 2);
			draw_oam(&xferscan[1024], y, 1);
			draw_oam(&xferscan[1024], y, 0);
			break;
	}


	// copy the working scanline to the real one
	memcpy(scanline, &xferscan[1024], 240*2);

	return;
}

static void dma_exec(running_machine *machine, FPTR ch);

static void gba_request_irq(running_machine *machine, UINT32 int_type)
{
	// is this specific interrupt enabled?
	int_type &= gba.IE;
	if (int_type != 0)
	{
		// set flag for later recovery
		gba.IF |= int_type;

		// master enable?
		if (gba.IME & 1)
		{
//			printf("IRQ %04x\n", int_type);
			cpunum_set_input_line(machine, 0, ARM7_IRQ_LINE, PULSE_LINE);
		}
	}
}

TIMER_CALLBACK( dma_complete )
{
	int ctrl;
	FPTR ch;
	static UINT32 ch_int[4] = { INT_DMA0, INT_DMA1, INT_DMA2, INT_DMA3 };

	ch = param;

//	printf("dma complete: ch %d\n", ch);

	timer_adjust_oneshot(dma_timer[ch], attotime_never, 0);

	ctrl = dma_regs[(ch*3)+2] >> 16;

	// IRQ
	if (ctrl & 0x4000)
	{
		gba_request_irq(machine, ch_int[ch]);
	}

	// if we're supposed to repeat, don't clear "active" and then the next vbl/hbl will retrigger us
	// always clear active for immediate DMAs though
	if (!((ctrl>>9) & 1) || ((ctrl & 0x3000) == 0))
	{
//		printf("clear active for ch %d\n", ch);
		dma_regs[(ch*3)+2] &= ~0x80000000;	// clear "active" bit
	}
	else
	{
		// if repeat, reload the count
		if ((ctrl>>9) & 1)
		{
			dma_cnt[ch] = dma_regs[(ch*3)+2]&0xffff;

			// if increment & reload mode, reload the destination
			if (((ctrl>>5)&3) == 3)
			{
				dma_dst[ch] = dma_regs[(ch*3)+1];
			}
		}
	}
}

static void dma_exec(running_machine *machine, FPTR ch)
{
	int i, cnt;
	int ctrl;
	int srcadd, dstadd;
	UINT32 src, dst;

	cpuintrf_push_context(0);

	src = dma_src[ch];
	dst = dma_dst[ch];
	ctrl = dma_regs[(ch*3)+2] >> 16;
	srcadd = dma_srcadd[ch];
 	dstadd = dma_dstadd[ch];

	cnt = dma_cnt[ch];
	if (!cnt)
	{
		if (ch == 3)
		{
			cnt = 0x10000;
		}
		else
		{
			cnt = 0x4000;
		}
	}

	// override special parameters
	if ((ctrl & 0x3000) == 0x3000)		// special xfer mode
	{
		switch (ch)
		{
			case 1:			// Ch 1&2 are for audio DMA
			case 2:
				dstadd = 2;	// don't increment destination
				cnt = 4;	// always transfer 4 32-bit words
				ctrl |= 0x400;	// always 32-bit
				break;

			case 3:
				printf("Unsupported DMA 3 special mode\n");
				break;
		}
	}
	else
	{
//		if (dst >= 0x6000000 && dst <= 0x6017fff)
//			printf("DMA exec: ch %d from %08x to %08x, mode %04x, count %04x (PC %x) (%s)\n", (int)ch, src, dst, ctrl, cnt, activecpu_get_pc(), ((ctrl>>10) & 1) ? "32" : "16");
	}

	for (i = 0; i < cnt; i++)
	{
		if ((ctrl>>10) & 1)
		{
			src &= 0xfffffffc;
			dst &= 0xfffffffc;

			// 32-bit
			program_write_dword(dst, program_read_dword(src));
			switch (dstadd)
			{
				case 0:	// increment
					dst += 4;
					break;
				case 1:	// decrement
					dst -= 4;
					break;
				case 2:	// don't move
					break;
				case 3:	// increment and reload
					dst += 4;
					break;
			}
			switch (srcadd)
			{
				case 0:	// increment
					src += 4;
					break;
				case 1:	// decrement
					src -= 4;
					break;
				case 2:	// don't move
					break;
				case 3:	// not used
					printf("DMA: Bad srcadd 3!\n");
					src += 4;
					break;
			}
		}
		else
		{
			src &= 0xfffffffe;
			dst &= 0xfffffffe;

		 	// 16-bit
			program_write_word(dst, program_read_word(src));
			switch (dstadd)
			{
				case 0:	// increment
					dst += 2;
					break;
				case 1:	// decrement
					dst -= 2;
					break;
				case 2:	// don't move
					break;
				case 3:	// increment and reload
					dst += 2;
					break;
			}
			switch (srcadd)
			{
				case 0:	// increment
					src += 2;
					break;
				case 1:	// decrement
					src -= 2;
					break;
				case 2:	// don't move
					break;
				case 3:	// not used
					printf("DMA: Bad srcadd 3!\n");
					break;
			}
		}
	}

	dma_src[ch] = src;
	dma_dst[ch] = dst;

//	printf("settng DMA timer %d for %d cycs (tmr %x)\n", ch, cnt, (UINT32)dma_timer[ch]);
//	timer_adjust_oneshot(dma_timer[ch], ATTOTIME_IN_CYCLES(0, cnt), ch);
	dma_complete(machine, NULL, ch);

	cpuintrf_pop_context();
}

static int fifo_a_ptr, fifo_b_ptr, fifo_a_in, fifo_b_in;
static UINT8 fifo_a[20], fifo_b[20];

static void audio_tick(running_machine *machine, int ref)
{
	if (!(gba.SOUNDCNT_X & 0x80))
	{
		return;
	}

	if (!ref)
	{
		if (fifo_a_ptr != fifo_a_in)
		{
			if (fifo_a_ptr == 17)
			{
				fifo_a_ptr = 0;
			}

			if (gba.SOUNDCNT_H & 0x100)
			{
				dac_signed_data_w(0, fifo_a[fifo_a_ptr]^0x80);
			}
			if (gba.SOUNDCNT_H & 0x200)
			{
				dac_signed_data_w(1, fifo_a[fifo_a_ptr]^0x80);
			}
			fifo_a_ptr++;
		}

		// fifo empty?
		if (fifo_a_ptr == fifo_a_in)
		{
			// is a DMA set up to feed us?
			if ((dma_regs[(1*3)+1] == 0x40000a0) && ((dma_regs[(1*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 1 it is
				dma_exec(machine, 1);
			}
			if ((dma_regs[(2*3)+1] == 0x40000a0) && ((dma_regs[(2*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 2 it is
				dma_exec(machine, 2);
			}
		}
	}
	else
	{
		if (fifo_b_ptr != fifo_b_in)
		{
			if (fifo_b_ptr == 17)
			{
				fifo_b_ptr = 0;
			}

			if (gba.SOUNDCNT_H & 0x1000)
			{
				dac_signed_data_w(2, fifo_b[fifo_b_ptr]^0x80);
			}
			if (gba.SOUNDCNT_H & 0x2000)
			{
				dac_signed_data_w(3, fifo_b[fifo_b_ptr]^0x80);
			}
			fifo_b_ptr++;
		}

		if (fifo_b_ptr == fifo_b_in)
		{
			// is a DMA set up to feed us?
			if ((dma_regs[(1*3)+1] == 0x40000a4) && ((dma_regs[(1*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 1 it is
				dma_exec(machine, 1);
			}
			if ((dma_regs[(2*3)+1] == 0x40000a4) && ((dma_regs[(2*3)+2] & 0x30000000) == 0x30000000))
			{
				// channel 2 it is
				dma_exec(machine, 2);
			}
		}
	}
}

static TIMER_CALLBACK(timer_expire)
{
	static UINT32 tmr_ints[4] = { INT_TM0_OVERFLOW, INT_TM1_OVERFLOW, INT_TM2_OVERFLOW, INT_TM3_OVERFLOW };
	FPTR tmr = (FPTR) param;

//	printf("Timer %d expired, SOUNDCNT_H %04x\n", tmr, gba.SOUNDCNT_H);

	// check if timers 0 or 1 are feeding directsound
	if (tmr == 0)
	{
		if ((gba.SOUNDCNT_H & 0x400) == 0)
		{
			audio_tick(machine, 0);
		}

		if ((gba.SOUNDCNT_H & 0x4000) == 0)
		{
			audio_tick(machine, 1);
		}
	}

	if (tmr == 1)
	{
		if ((gba.SOUNDCNT_H & 0x400) == 0x400)
		{
			audio_tick(machine, 0);
		}

		if ((gba.SOUNDCNT_H & 0x4000) == 0x4000)
		{
			audio_tick(machine, 1);
		}
	}

	// Handle count-up timing
	switch (tmr)
	{
	case 0:
	        if (timer_regs[1] & 0x40000)
	        {
		        timer_regs[1] = ( ( timer_regs[1] & 0x0000ffff ) + 1 ) & 0x0000ffff;
		        if( ( timer_regs[1] & 0x0000ffff ) == 0 )
		        {
		                timer_regs[1] |= timer_reload[1];
		                if( ( timer_regs[1] & 0x400000 ) && ( gba.IME != 0 ) )
		                {
			                gba_request_irq( machine, tmr_ints[1] );
		                }
		                if( ( timer_regs[2] & 0x40000 ) )
		                {
			                timer_regs[2] = ( ( timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff;
			                if( ( timer_regs[2] & 0x0000ffff ) == 0 )
			                {
			                        timer_regs[2] |= timer_reload[2];
			                        if( ( timer_regs[2] & 0x400000 ) && ( gba.IME != 0 ) )
			                        {
				                        gba_request_irq( machine, tmr_ints[2] );
			                        }
			                        if( ( timer_regs[3] & 0x40000 ) )
			                        {
				                        timer_regs[3] = ( ( timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff;
				                        if( ( timer_regs[3] & 0x0000ffff ) == 0 )
				                        {
				                                timer_regs[3] |= timer_reload[3];
				                                if( ( timer_regs[3] & 0x400000 ) && ( gba.IME != 0 ) )
				                                {
					                                gba_request_irq( machine, tmr_ints[3] );
				                                }
				                        }
			                        }
			                }
		                }
		        }
	        }
	        break;
	case 1:
	        if (timer_regs[2] & 0x40000)
	        {
		        timer_regs[2] = ( ( timer_regs[2] & 0x0000ffff ) + 1 ) & 0x0000ffff;
		        if( ( timer_regs[2] & 0x0000ffff ) == 0 )
		        {
		                timer_regs[2] |= timer_reload[2];
		                if( ( timer_regs[2] & 0x400000 ) && ( gba.IME != 0 ) )
		                {
			                gba_request_irq( machine, tmr_ints[2] );
		                }
		                if( ( timer_regs[3] & 0x40000 ) )
		                {
		        	        timer_regs[3] = ( ( timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff;
			                if( ( timer_regs[3] & 0x0000ffff ) == 0 )
			                {
			                        timer_regs[2] |= timer_reload[2];
			                        if( ( timer_regs[3] & 0x400000 ) && ( gba.IME != 0 ) )
			                        {
				                        gba_request_irq( machine, tmr_ints[3] );
			                        }
			                }
		                }
		        }
	        }
	        break;
	case 2:
	        if (timer_regs[3] & 0x40000)
	        {
		        timer_regs[3] = ( ( timer_regs[3] & 0x0000ffff ) + 1 ) & 0x0000ffff;
		        if( ( timer_regs[3] & 0x0000ffff ) == 0 )
		        {
		                timer_regs[3] |= timer_reload[3];
		                if( ( timer_regs[3] & 0x400000 ) && ( gba.IME != 0 ) )
		                {
			                gba_request_irq( machine, tmr_ints[3] );
		                }
		        }
	        }
	        break;
	}

	// are we supposed to IRQ?
	if ((timer_regs[tmr] & 0x400000) && (gba.IME != 0))
	{
		gba_request_irq(machine, tmr_ints[tmr]);
	}
}

TIMER_CALLBACK(handle_irq)
{
	gba_request_irq(machine, gba.IF);

	timer_adjust_oneshot(irq_timer, attotime_never, 0);
}

static READ32_HANDLER( gba_io_r )
{
	UINT32 retval = 0;
	switch( offset )
	{
		case 0x0000/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: DISPCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.DISPCNT );
				retval |= gba.DISPCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: Green Swap (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.GRNSWAP );
				retval |= gba.GRNSWAP << 16;
			}
			break;
		case 0x0004/4:
			retval = gba.DISPSTAT | (video_screen_get_vpos(machine->primary_screen)<<16);
			break;
		case 0x0008/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG0CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.BG0CNT );
				retval |= gba.BG0CNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG1CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.BG1CNT );
				retval |= gba.BG1CNT << 16;
			}
			break;
		case 0x000c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.BG2CNT );
				retval |= gba.BG2CNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3CNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.BG3CNT );
				retval |= gba.BG3CNT << 16;
			}
			break;
		case 0x0010/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG0HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG0VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0014/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG1HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG1VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0018/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG2VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x001c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG3HOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3VOFS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0020/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2PA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG2PB (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0024/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2PC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG2PD (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0028/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2X_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG2X_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x002c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG2Y_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG2Y_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0030/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG3PA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3PB (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0034/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG3PC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3PD (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0038/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG3X_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3X_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x003c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BG3Y_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BG3Y_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0040/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: WIN0H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: WIN1H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0044/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: WIN0V (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: WIN1V (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0048/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: WININ (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.WININ );
				retval |= gba.WININ;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: WINOUT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.WINOUT );
				retval |= gba.WINOUT << 16;
			}
			break;
		case 0x004c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: MOSAIC (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0050/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BLDCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.BLDCNT );
				retval |= gba.BLDCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: BLDALPHA (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0054/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: BLDY (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0058/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x005c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), 0 );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0060/4:
			retval = gb_sound_r(machine, 0) | gb_sound_r(machine, 1)<<16 | gb_sound_r(machine, 2)<<24;
			break;
		case 0x0064/4:
			retval = gb_sound_r(machine, 3) | gb_sound_r(machine, 4)<<8;
			break;
		case 0x0068/4:
			retval = gb_sound_r(machine, 6) | gb_sound_r(machine, 7)<<8;
			break;
		case 0x006c/4:
			retval = gb_sound_r(machine, 8) | gb_sound_r(machine, 9)<<8;
			break;
		case 0x0070/4:
			retval = gb_sound_r(machine, 0xa) | gb_sound_r(machine, 0xb)<<16 | gb_sound_r(machine, 0xc)<<24;
			break;
		case 0x0074/4:
			retval = gb_sound_r(machine, 0xd) | gb_sound_r(machine, 0xe)<<8;
			break;
		case 0x0078/4:
			retval = gb_sound_r(machine, 0x10) | gb_sound_r(machine, 0x11)<<8;
			break;
		case 0x007c/4:
			retval = gb_sound_r(machine, 0x12) | gb_sound_r(machine, 0x13)<<8;
			break;
		case 0x0080/4:
			retval = gb_sound_r(machine, 0x14) | gb_sound_r(machine, 0x15)<<8;
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: SOUNDCNT_H (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.SOUNDCNT_H );
				retval |= gba.SOUNDCNT_H << 16;
			}
			break;
		case 0x0084/4:
			retval = gb_sound_r(machine, 0x16);
			break;
		case 0x0088/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: SOUNDBIAS (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.SOUNDBIAS );
				retval |= gba.SOUNDBIAS;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0090/4:
			retval = gb_wave_r(machine, 0) | gb_wave_r(machine, 1)<<8 | gb_wave_r(machine, 2)<<16 | gb_wave_r(machine, 3)<<24;
			break;
		case 0x0094/4:
			retval = gb_wave_r(machine, 4) | gb_wave_r(machine, 5)<<8 | gb_wave_r(machine, 6)<<16 | gb_wave_r(machine, 7)<<24;
			break;
		case 0x0098/4:
			retval = gb_wave_r(machine, 8) | gb_wave_r(machine, 9)<<8 | gb_wave_r(machine, 10)<<16 | gb_wave_r(machine, 11)<<24;
			break;
		case 0x009c/4:
			retval = gb_wave_r(machine, 12) | gb_wave_r(machine, 13)<<8 | gb_wave_r(machine, 14)<<16 | gb_wave_r(machine, 15)<<24;
			break;
		case 0x00a0/4:
		case 0x00a4/4:
			return 0;	// (does this actually do anything on real h/w?)
			break;
		case 0x00b0/4:
		case 0x00b4/4:
		case 0x00b8/4:
		case 0x00bc/4:
		case 0x00c0/4:
		case 0x00c4/4:
		case 0x00c8/4:
		case 0x00cc/4:
		case 0x00d0/4:
		case 0x00d4/4:
		case 0x00d8/4:
		case 0x00dc/4:
			{
				// no idea why here, but it matches VBA better
				if (((offset-0xb0/4) % 3) == 2)
				{
					return dma_regs[offset-(0xb0/4)] & 0xff000000;
				}

				return dma_regs[offset-(0xb0/4)];
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			return timer_regs[offset-(0x100/4)];
			break;
		case 0x0120/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: SIOMULTI0 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.SIOMULTI0 );
				retval |= gba.SIOMULTI0;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: SIOMULTI1 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.SIOMULTI1 );
				retval |= gba.SIOMULTI1 << 16;
			}
			break;
		case 0x0124/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: SIOMULTI2 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.SIOMULTI2 );
				retval |= gba.SIOMULTI2;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: SIOMULTI3 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.SIOMULTI3 );
				retval |= gba.SIOMULTI3 << 16;
			}
			break;
		case 0x0128/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: SIOCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.SIOCNT );
				retval |= gba.SIOCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: SIODATA8 (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.SIODATA8 );
				retval |= gba.SIODATA8 << 16;
			}
			break;
		case 0x0130/4:
			if( (mem_mask) & 0x0000ffff )	// KEYINPUT
			{
				return input_port_read(machine, "IN0");
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: KEYCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.KEYCNT );
				retval |= gba.KEYCNT << 16;
			}
			break;
		case 0x0134/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: RCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.RCNT );
				retval |= gba.RCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: IR (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0140/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: JOYCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.JOYCNT );
				retval |= gba.JOYCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0150/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: JOY_RECV_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.JOY_RECV & 0x0000ffff );
				retval |= gba.JOY_RECV & 0x0000ffff;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: JOY_RECV_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, ( gba.JOY_RECV & 0xffff0000 ) >> 16 );
				retval |= gba.JOY_RECV & 0xffff0000;
			}
			break;
		case 0x0154/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: JOY_TRANS_LSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.JOY_TRANS & 0x0000ffff );
				retval |= gba.JOY_TRANS & 0x0000ffff;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: JOY_TRANS_MSW (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, ( gba.JOY_TRANS & 0xffff0000 ) >> 16 );
				retval |= gba.JOY_TRANS & 0xffff0000;
			}
			break;
		case 0x0158/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: JOYSTAT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.JOYSTAT );
				retval |= gba.JOYSTAT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0200/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: IE (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.IE );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: IF (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, gba.IF );
			}

			retval = gba.IE | (gba.IF<<16);
			break;
		case 0x0204/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: WAITCNT (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.WAITCNT );
				retval |= gba.WAITCNT;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0208/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Read: IME (%08x) = %04x\n", 0x04000000 + ( offset << 2 ), gba.IME );
				retval |= gba.IME;
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Read: UNKNOWN (%08x) = %04x\n", 0x04000000 + ( offset << 2 ) + 2, 0 );
			}
			break;
		case 0x0300/4:
			retval = gba.HALTCNT << 8;
			break;
		default:
//			verboselog( 0, "Unknown GBA I/O register Read: %08x (%08x)\n", 0x04000000 + ( offset << 2 ), ~mem_mask );
			break;
	}
	return retval;
}

static WRITE32_HANDLER( gba_io_w )
{
	switch( offset )
	{
		case 0x0000/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: DISPCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.DISPCNT = ( gba.DISPCNT & ~mem_mask ) | ( data & mem_mask );
			}
			break;
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: Green Swap (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
				gba.GRNSWAP = ( gba.GRNSWAP & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0004/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: DISPSTAT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.DISPSTAT = ( gba.DISPSTAT & ~mem_mask ) | ( data & mem_mask ) ;
			}
			break;
		case 0x0008/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG0CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG0CNT = ( gba.BG0CNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG1CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG1CNT = ( gba.BG1CNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x000c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG2CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2CNT = ( gba.BG2CNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3CNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3CNT = ( gba.BG3CNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0010/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG0HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG0HOFS = ( gba.BG0HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG0VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG0VOFS = ( gba.BG0VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0014/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG1HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG1HOFS = ( gba.BG1HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG1VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG1VOFS = ( gba.BG1VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0018/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG2HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2HOFS = ( gba.BG2HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG2VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG2VOFS = ( gba.BG2VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x001c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG3HOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG3HOFS = ( gba.BG3HOFS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3VOFS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3VOFS = ( gba.BG3VOFS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0020/4:
			if( (mem_mask) & 0x0000ffff )
			{
				printf( "GBA IO Register Write: BG2PA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2PA = ( gba.BG2PA & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				printf( "GBA IO Register Write: BG2PB (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG2PB = ( gba.BG2PB & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0024/4:
			if( (mem_mask) & 0x0000ffff )
			{
				printf( "GBA IO Register Write: BG2PC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2PC = ( gba.BG2PC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				printf( "GBA IO Register Write: BG2PD (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG2PD = ( gba.BG2PD & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0028/4:
			if( (mem_mask) & 0x0000ffff )
			{
				printf( "GBA IO Register Write: BG2X_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2X = ( gba.BG2X & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				printf( "GBA IO Register Write: BG2X_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
		                gba.BG2X = ( gba.BG2X & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x002c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				printf( "GBA IO Register Write: BG2Y_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG2Y = ( gba.BG2Y & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				printf( "GBA IO Register Write: BG2Y_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                		gba.BG2Y = ( gba.BG2Y & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0030/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG3PA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG3PA = ( gba.BG3PA & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3PB (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3PB = ( gba.BG3PB & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0034/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG3PC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG3PC = ( gba.BG3PC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3PD (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3PD = ( gba.BG3PD & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0038/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG3X_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG3X = ( gba.BG3X & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3X_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3X = ( gba.BG3X & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x003c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BG3Y_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BG3Y = ( gba.BG3Y & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BG3Y_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BG3Y = ( gba.BG3Y & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0040/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: WIN0H (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.WIN0H = ( gba.WIN0H & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: WIN1H (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.WIN1H = ( gba.WIN1H & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0044/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: WIN0V (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.WIN0V = ( gba.WIN0V & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: WIN1V (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.WIN1V = ( gba.WIN1V & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0048/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: WININ (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.WININ = ( gba.WININ & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: WINOUT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.WINOUT = ( gba.WINOUT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x004c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: MOSAIC (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.MOSAIC = ( gba.MOSAIC & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0050/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BLDCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BLDCNT = ( gba.BLDCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: BLDALPHA (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
                gba.BLDALPHA = ( gba.BLDALPHA & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0054/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: BLDY (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.BLDY = ( gba.BLDY & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0058/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x005c/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0060/4:
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(machine, 0, data);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_sound_w(machine, 1, data>>16);	// SOUND1CNT_H
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_sound_w(machine, 2, data>>24);
			}
			break;
		case 0x0064/4:
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(machine, 3, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 4, data>>8);	// SOUND1CNT_H
			}
			break;
		case 0x0068/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 6, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 7, data>>8);
			}
			break;
		case 0x006c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 8, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 9, data>>8);
			}
			break;
		case 0x0070/4:	//SND3CNTL and H
			if( (mem_mask) & 0x000000ff )	// SOUNDCNTL
			{
				gb_sound_w(machine, 0xa, data);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_sound_w(machine, 0xb, data>>16);	// SOUND1CNT_H
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_sound_w(machine, 0xc, data>>24);
			}
			break;
		case 0x0074/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 0xd, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 0xe, data>>8);
			}
			break;
		case 0x0078/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 0x10, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 0x11, data>>8);
			}
			break;
		case 0x007c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 0x12, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 0x13, data>>8);
			}
			break;
		case 0x0080/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 0x14, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_sound_w(machine, 0x15, data>>8);
			}

			if ((mem_mask) & 0xffff0000)
			{
				data >>= 16;
				gba.SOUNDCNT_H = data;

				// DAC A reset?
				if (data & 0x0800)
				{
					fifo_a_ptr = 17;
					fifo_a_in = 17;
					dac_signed_data_w(0, 0x80);
					dac_signed_data_w(1, 0x80);
				}

				// DAC B reset?
				if (data & 0x8000)
				{
					fifo_b_ptr = 17;
					fifo_b_in = 17;
					dac_signed_data_w(2, 0x80);
					dac_signed_data_w(3, 0x80);
				}
			}
			break;
		case 0x0084/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_sound_w(machine, 0x16, data);
				if ((data & 0x80) && !(gba.SOUNDCNT_X & 0x80))
				{
					fifo_a_ptr = fifo_a_in = 17;
					fifo_b_ptr = fifo_b_in = 17;
					dac_signed_data_w(0, 0x80);
					dac_signed_data_w(1, 0x80);
					dac_signed_data_w(2, 0x80);
					dac_signed_data_w(3, 0x80);
				}
				gba.SOUNDCNT_X = data;
			}
			break;
		case 0x0088/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: SOUNDBIAS (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x0000ffff, ~mem_mask );
				gba.SOUNDBIAS = ( gba.SOUNDBIAS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		case 0x0090/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(machine, 0, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(machine, 1, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(machine, 2, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(machine, 3, data>>24);
			}
			break;
		case 0x0094/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(machine, 4, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(machine, 5, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(machine, 6, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(machine, 7, data>>24);
			}
			break;
		case 0x0098/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(machine, 8, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(machine, 9, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(machine, 0xa, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(machine, 0xb, data>>24);
			}
			break;
		case 0x009c/4:
			if( (mem_mask) & 0x000000ff )
			{
				gb_wave_w(machine, 0xc, data);
			}
			if( (mem_mask) & 0x0000ff00 )
			{
				gb_wave_w(machine, 0xd, data>>8);
			}
			if( (mem_mask) & 0x00ff0000 )
			{
				gb_wave_w(machine, 0xe, data>>16);
			}
			if( (mem_mask) & 0xff000000 )
			{
				gb_wave_w(machine, 0xf, data>>24);
			}
			break;
		case 0x00a0/4:
			fifo_a_in %= 17;
			fifo_a[fifo_a_in++] = (data)&0xff;
			fifo_a_in %= 17;
			fifo_a[fifo_a_in++] = (data>>8)&0xff;
			fifo_a_in %= 17;
			fifo_a[fifo_a_in++] = (data>>16)&0xff;
			fifo_a_in %= 17;
			fifo_a[fifo_a_in++] = (data>>24)&0xff;
			break;
		case 0x00a4/4:
			fifo_b_in %= 17;
			fifo_b[fifo_b_in++] = (data)&0xff;
			fifo_b_in %= 17;
			fifo_b[fifo_b_in++] = (data>>8)&0xff;
			fifo_b_in %= 17;
			fifo_b[fifo_b_in++] = (data>>16)&0xff;
			fifo_b_in %= 17;
			fifo_b[fifo_b_in++] = (data>>24)&0xff;
			break;
		case 0x00b0/4:
		case 0x00b4/4:
		case 0x00b8/4:

		case 0x00bc/4:
		case 0x00c0/4:
		case 0x00c4/4:

		case 0x00c8/4:
		case 0x00cc/4:
		case 0x00d0/4:

		case 0x00d4/4:
		case 0x00d8/4:
		case 0x00dc/4:
			{
				int ch;

				offset -= (0xb0/4);

				ch = offset / 3;

//				printf("%08x: DMA(%d): %x to reg %d (mask %08x)\n", activecpu_get_pc(), ch, data, offset%3, ~mem_mask);

				if (((offset % 3) == 2) && ((~mem_mask & 0xffff0000) == 0))
				{
					int ctrl = data>>16;

					// retrigger/restart on a rising edge.
					// also reload internal regs
					// (note: Metroid Fusion fails if we enforce the "rising edge" requirement...)
					if (ctrl & 0x8000) //&& !(dma_regs[offset] & 0x80000000))
					{
						dma_src[ch] = dma_regs[(ch*3)+0];
						dma_dst[ch] = dma_regs[(ch*3)+1];
						dma_srcadd[ch] = (ctrl>>7)&3;
						dma_dstadd[ch] = (ctrl>>5)&3;

                        COMBINE_DATA(&dma_regs[offset]);
                        dma_cnt[ch] = dma_regs[(ch*3)+2]&0xffff;

                        // immediate start
						if ((ctrl & 0x3000) == 0)
						{
							dma_exec(machine, ch);
							return;
						}
				 	}
				}

				COMBINE_DATA(&dma_regs[offset]);
			}
			break;
		case 0x0100/4:
		case 0x0104/4:
		case 0x0108/4:
		case 0x010c/4:
			{
				double rate, clocksel;

				offset -= (0x100/4);

				COMBINE_DATA(&timer_regs[offset]);

//				printf("%x to timer %d (mask %x PC %x)\n", data, offset, ~mem_mask, activecpu_get_pc());

				if (ACCESSING_BITS_0_15)
				{
				        timer_reload[offset] = ( timer_reload[offset] & ~mem_mask ) | ( ( data & 0x0000ffff ) & mem_mask );
				}

				// enabling this timer?
				if ((ACCESSING_BITS_16_31) && (data & 0x800000))
				{
					double final;

					rate = 0x10000 - (timer_regs[offset] & 0xffff);

					clocksel = timer_clks[(timer_regs[offset] >> 16) & 3];

					final = clocksel / rate;

//					printf("Enabling timer %d @ %f Hz\n", offset, final);

					// enable the timer
					if( !(data & 0x40000) ) // if we're not in Count-Up mode
					{
						timer_adjust_periodic(tmr_timer[offset], ATTOTIME_IN_HZ(final), offset, ATTOTIME_IN_HZ(final));
					}
				}
			}
			break;
		case 0x0120/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: SIOMULTI0 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.SIOMULTI0 = ( gba.SIOMULTI0 & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: SIOMULTI1 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.SIOMULTI1 = ( gba.SIOMULTI1 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0124/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: SIOMULTI2 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.SIOMULTI2 = ( gba.SIOMULTI2 & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: SIOMULTI3 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.SIOMULTI3 = ( gba.SIOMULTI3 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0128/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: SIOCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.SIOCNT = ( gba.SIOCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: SIODATA8 (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.SIODATA8 = ( gba.SIODATA8 & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0130/4:
			if( (mem_mask) & 0xffff0000 )
			{
//				printf("KEYCNT = %04x\n", data>>16);
				verboselog( 2, "GBA IO Register Write: KEYCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.KEYCNT = ( gba.KEYCNT & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0134/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: RCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.RCNT = ( gba.RCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: IR (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.IR = ( gba.IR & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0140/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: JOYCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.JOYCNT = ( gba.JOYCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0150/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: JOY_RECV_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.JOY_RECV = ( gba.JOY_RECV & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: JOY_RECV_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.JOY_RECV = ( gba.JOY_RECV & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0154/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: JOY_TRANS_LSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.JOY_TRANS = ( gba.JOY_TRANS & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: JOY_TRANS_MSW (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
                gba.JOY_TRANS = ( gba.JOY_TRANS & ( ~mem_mask >> 16 ) ) | ( ( data & mem_mask ) >> 16 );
			}
			break;
		case 0x0158/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: JOYSTAT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.JOYSTAT = ( gba.JOYSTAT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0200/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: IE (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.IE = ( gba.IE & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: IF (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ) + 2, ( data & mem_mask ) >> 16, ~mem_mask );
				gba.IF &= ~( ( data & mem_mask ) >> 16 );

				// if we still have interrupts, yank the IRQ line again
				if (gba.IF)
				{
					timer_adjust_oneshot(irq_timer, ATTOTIME_IN_CYCLES(0, 120), 0);
				}
			}
			break;
		case 0x0204/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 2, "GBA IO Register Write: WAITCNT (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.WAITCNT = ( gba.WAITCNT & ~mem_mask ) | ( data & mem_mask );
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0208/4:
			if( (mem_mask) & 0x0000ffff )
			{
				verboselog( 3, "GBA IO Register Write: IME (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), data & mem_mask, ~mem_mask );
				gba.IME = ( gba.IME & ~mem_mask ) | ( data & mem_mask );
				if (gba.IF)
				{
					timer_adjust_oneshot(irq_timer, attotime_zero, 0);
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 3, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & mem_mask ) >> 16, ~mem_mask );
			}
			break;
		case 0x0300/4:
			if( (mem_mask) & 0x0000ffff )
			{
				if( (mem_mask) & 0x000000ff )
				{
					verboselog( 2, "GBA IO Register Write: POSTFLG (%08x) = %02x (%08x)\n", 0x04000000 + ( offset << 2 ), data & 0x000000ff, ~mem_mask );
					gba.POSTFLG = data & 0x000000ff;
				}
				else
				{
					gba.HALTCNT = data & 0x000000ff;

					// either way, wait for an IRQ
					cpu_spinuntil_int();
				}
			}
			if( (mem_mask) & 0xffff0000 )
			{
				verboselog( 2, "GBA IO Register Write: UNKNOWN (%08x) = %04x (%08x)\n", 0x04000000 + ( offset << 2 ), ( data & 0xffff0000 ) >> 16, ~mem_mask );
			}
			break;
		default:
//			verboselog( 0, "Unknown GBA I/O register write: %08x = %08x (%08x)\n", 0x04000000 + ( offset << 2 ), data, ~mem_mask );
			break;
	}
}

static ADDRESS_MAP_START( gbadvance_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROMBANK(1)
	AM_RANGE(0x02000000, 0x0203ffff) AM_RAM AM_MIRROR(0xfc0000)
	AM_RANGE(0x03000000, 0x03007fff) AM_RAM AM_MIRROR(0xff8000)
	AM_RANGE(0x04000000, 0x040003ff) AM_READWRITE( gba_io_r, gba_io_w )
	AM_RANGE(0x05000000, 0x050003ff) AM_RAM AM_BASE(&pram)	// Palette RAM
	AM_RANGE(0x06000000, 0x06017fff) AM_RAM AM_BASE(&vram)	// VRAM
	AM_RANGE(0x07000000, 0x070003ff) AM_RAM AM_BASE(&oam)	// OAM
	AM_RANGE(0x08000000, 0x09ffffff) AM_ROM AM_REGION("cartridge", 0)	// cartridge ROM (mirror 0)
	AM_RANGE(0x0a000000, 0x0bffffff) AM_ROM AM_REGION("cartridge", 0)	// cartridge ROM (mirror 1)
	AM_RANGE(0x0c000000, 0x0cffffff) AM_ROM AM_REGION("cartridge", 0)	// final mirror
ADDRESS_MAP_END

INPUT_PORTS_START( gbadv )
	PORT_START("IN0")
	PORT_BIT( 0xfc00, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_UNUSED
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 R") PORT_PLAYER(1)	// R
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 L") PORT_PLAYER(1)	// L
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1)	// START
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("SELECT") PORT_PLAYER(1)	// SELECT
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B") PORT_PLAYER(1)	// B
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A") PORT_PLAYER(1)	// A
INPUT_PORTS_END

static UINT8 framecount;
static emu_timer *scan_timer, *hbl_timer;

TIMER_CALLBACK( perform_hbl )
{
	int ch, ctrl;

	// make the ARM7 current
	cpuintrf_push_context(0);

	gba_draw_scanline(video_screen_get_vpos(machine->primary_screen));

	// we are now in hblank
	gba.DISPSTAT |= DISPSTAT_HBL;
	if ((gba.DISPSTAT & DISPSTAT_HBL_IRQ_EN ) != 0)
	{
		gba_request_irq(machine, INT_HBL);
	}

	for (ch = 0; ch < 4; ch++)
	{
		ctrl = dma_regs[(ch*3)+2]>>16;

		// HBL-triggered DMA?
		if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x2000))
		{
			dma_exec(machine, ch);
		}
	}

	timer_adjust_oneshot(hbl_timer, attotime_never, 0);

	cpuintrf_pop_context();
}

TIMER_CALLBACK( perform_scan )
{
	int scanline;

	// make the ARM7 current
	cpuintrf_push_context(0);

	// no longer in HBL
	gba.DISPSTAT &= ~DISPSTAT_HBL;

	scanline = video_screen_get_vpos(machine->primary_screen);

	// check if VCNT is enabled
	if ((gba.DISPSTAT & DISPSTAT_VCNT_IRQ_EN) && (scanline < 255))
	{
		if (scanline == ((gba.DISPSTAT >> 8) & 0xff))
		{
			gba_request_irq(machine, INT_VCNT);
		}
	}

	// scanline zero means not in VBL anymore
	if (scanline == 0)
	{
		gba.DISPSTAT &= ~DISPSTAT_VBL;
	}
	else if (scanline == 160)	// scanline 160 means entering VBL
	{
		int ch, ctrl;

		gba.DISPSTAT |= DISPSTAT_VBL;
//		printf("DISPSTAT %x %x\n", gba.DISPSTAT, gba.DISPSTAT & DISPSTAT_VBL_IRQ_EN);
		if ((gba.DISPSTAT & DISPSTAT_VBL_IRQ_EN) != 0)
		{
			gba_request_irq(machine, INT_VBL);
			framecount++;
		}

		for (ch = 0; ch < 4; ch++)
		{
			ctrl = dma_regs[(ch*3)+2]>>16;

			// VBL-triggered DMA?
			if ((ctrl & 0x8000) && ((ctrl & 0x3000) == 0x1000))
			{
				dma_exec(machine, ch);
			}
		}
	}

	#if 0
	if( framecount > 200 && temp == 0 )
	{
		if( temp == 0 )
		{
			UINT8 charbuf[512];
			UINT32 byteindex;
			UINT32 lineindex;
			temp++;
			// Dump VRAM
			for( lineindex = 0; lineindex < 0x18000 / 4; lineindex += 4 )
			{
				sprintf( charbuf, "%08x: ", lineindex * 4 );
				for( byteindex = 0; byteindex < 4; byteindex++ )
				{
					sprintf( charbuf, "%s%02x %02x %02x %02x ", charbuf,
							 ( vram[lineindex + byteindex] >> 24 ) & 0x000000ff,
							 ( vram[lineindex + byteindex] >> 16 ) & 0x000000ff,
							 ( vram[lineindex + byteindex] >>  8 ) & 0x000000ff,
							 ( vram[lineindex + byteindex] >>  0 ) & 0x000000ff );
				}
				sprintf( charbuf, "%s\n", charbuf );
				verboselog( 0, "%s", charbuf );
			}
		}
	}
	#endif

	timer_adjust_oneshot(hbl_timer, video_screen_get_time_until_pos(machine->primary_screen, scanline, 240), 0);
	timer_adjust_oneshot(scan_timer, video_screen_get_time_until_pos(machine->primary_screen, ( scanline + 1 ) % 228, 0), 0);

	cpuintrf_pop_context();
}

static MACHINE_RESET( gba )
{
	memset(&gba, 0, sizeof(gba));
	gba.SOUNDBIAS = 0x0200;
	memset(timer_regs, 0, sizeof(timer_regs));
	memset(dma_regs, 0, sizeof(dma_regs));
	gba.SIOMULTI0 = 0xffff;
	gba.SIOMULTI1 = 0xffff;
	gba.SIOMULTI2 = 0xffff;
	gba.SIOMULTI3 = 0xffff;
	gba.KEYCNT = 0x03ff;
	gba.RCNT = 0x8000;
	gba.JOYSTAT = 0x0002;
	temp = 0;
	framecount = 0;

	timer_adjust_oneshot(scan_timer, video_screen_get_time_until_pos(machine->primary_screen, 1, 0), 0);
	timer_adjust_oneshot(hbl_timer, attotime_never, 0);
	timer_adjust_oneshot(dma_timer[0], attotime_never, 0);
	timer_adjust_oneshot(dma_timer[1], attotime_never, 1);
	timer_adjust_oneshot(dma_timer[2], attotime_never, 2);
	timer_adjust_oneshot(dma_timer[3], attotime_never, 3);

	fifo_a_ptr = fifo_b_ptr = 17;	// indicate empty
	fifo_a_in = fifo_b_in = 17;

	// and clear the DACs
	dac_signed_data_w(0, 0x80);
	dac_signed_data_w(1, 0x80);
	dac_signed_data_w(2, 0x80);
	dac_signed_data_w(3, 0x80);
}

static MACHINE_START( gba )
{
	int level, x;

	/* add a hook for battery save */
	add_exit_callback(machine, gba_machine_stop);

	/* create a timer to fire scanline functions */
	scan_timer = timer_alloc(perform_scan, 0);
	hbl_timer = timer_alloc(perform_hbl, 0);
	timer_adjust_oneshot(scan_timer, video_screen_get_time_until_pos(machine->primary_screen, 1, 0), 0);

	/* and one for each DMA channel */
	dma_timer[0] = timer_alloc(dma_complete, 0);
	dma_timer[1] = timer_alloc(dma_complete, 0);
	dma_timer[2] = timer_alloc(dma_complete, 0);
	dma_timer[3] = timer_alloc(dma_complete, 0);
	timer_adjust_oneshot(dma_timer[0], attotime_never, 0);
	timer_adjust_oneshot(dma_timer[1], attotime_never, 1);
	timer_adjust_oneshot(dma_timer[2], attotime_never, 2);
	timer_adjust_oneshot(dma_timer[3], attotime_never, 3);

	/* also one for each timer (heh) */
	tmr_timer[0] = timer_alloc(timer_expire, 0);
	tmr_timer[1] = timer_alloc(timer_expire, 0);
	tmr_timer[2] = timer_alloc(timer_expire, 0);
	tmr_timer[3] = timer_alloc(timer_expire, 0);
	timer_adjust_oneshot(tmr_timer[0], attotime_never, 0);
	timer_adjust_oneshot(tmr_timer[1], attotime_never, 1);
	timer_adjust_oneshot(tmr_timer[2], attotime_never, 2);
	timer_adjust_oneshot(tmr_timer[3], attotime_never, 3);

	/* and an IRQ handling timer */
	irq_timer = timer_alloc(handle_irq, 0);
	timer_adjust_oneshot(irq_timer, attotime_never, 0);

	/* generate a table to make mosaic fast */
	for (level = 0; level < 16; level++) 
	{
		for (x = 0; x < 4096; x++)
		{
			mosaic_offset[level][x] = (x / (level + 1)) * (level + 1);
		}
	}
}

// 8-bit GB audio
static custom_sound_interface gameboy_sound_interface =
{ gameboy_sh_start, 0, 0 };

static MACHINE_DRIVER_START( gbadv )
	MDRV_CPU_ADD("main", ARM7, 16777216)
	MDRV_CPU_PROGRAM_MAP(gbadvance_map,0)

	MDRV_MACHINE_START(gba)
	MDRV_MACHINE_RESET(gba)

	MDRV_SCREEN_ADD("main", RASTER)	// htot hst vwid vtot vst vis
	MDRV_SCREEN_RAW_PARAMS(16777216/4, 308, 0,  240, 228, 0,  160)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_PALETTE_LENGTH(32768)
	MDRV_PALETTE_INIT( gba )

	MDRV_VIDEO_START(generic_bitmapped)
	MDRV_VIDEO_UPDATE(generic_bitmapped)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")
	MDRV_SOUND_ADD("gblegacy", CUSTOM, 0)		// legacy GB sound
	MDRV_SOUND_CONFIG(gameboy_sound_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)
	MDRV_SOUND_ADD("direct A left", DAC, 0)			// GBA direct sound A left
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ADD("direct A right", DAC, 0)		// GBA direct sound A right
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
	MDRV_SOUND_ADD("direct B left", DAC, 0)			// GBA direct sound B left
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ADD("direct B right", DAC, 0)		// GBA direct sound B right
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
MACHINE_DRIVER_END

ROM_START( gba )
	ROM_REGION( 0x8000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "gba.bin", 0x000000, 0x004000, CRC(81977335) )

	/* cartridge region - 32 MBytes (128 Mbit) */
	ROM_REGION( 0x2000000, "cartridge", ROMREGION_ERASEFF )
ROM_END

static READ32_HANDLER( sram_r )
{
	return gba_sram[offset];
}

static WRITE32_HANDLER( sram_w )
{
	COMBINE_DATA(&gba_sram[offset]);
}

enum
{
    FLASH_IDLEBYTE0,
    FLASH_IDLEBYTE1,
    FLASH_IDLEBYTE2,
    FLASH_IDENT,
    FLASH_ERASEBYTE0,
    FLASH_ERASEBYTE1,
    FLASH_ERASEBYTE2,
    FLASH_ERASE_ALL,
    FLASH_ERASE_4K,
    FLASH_WRITE
};

static UINT8 flash64k_state = FLASH_IDLEBYTE0;
static UINT8 flash64k_page = 0;

static READ32_HANDLER( flash64k_r )
{
    switch( flash64k_state )
    {
        case FLASH_IDLEBYTE0:
        case FLASH_IDLEBYTE1:
        case FLASH_IDLEBYTE2:
        case FLASH_ERASEBYTE0:
        case FLASH_ERASEBYTE1:
        case FLASH_ERASEBYTE2:
            return gba_flash64k[offset];
        case FLASH_IDENT:
            if( offset == 0 )
            {
                return ( gba_flash64k[0] & 0xffff0000 ) | 0x00001b32;
            }
            else
            {
                return gba_flash64k[offset];
            }
            return 0x00001b32;
        case FLASH_ERASE_ALL:
        case FLASH_ERASE_4K:
            return gba_flash64k[offset];
    }
    return gba_flash64k[offset];
}

static WRITE32_HANDLER( flash64k_w )
{
    switch( flash64k_state )
    {
        case FLASH_IDLEBYTE0:
        case FLASH_ERASEBYTE0:
            if( offset == 0x5555/4 && ~mem_mask == 0xffff00ff )
            {
                if( ( data & mem_mask ) == 0x0000aa00 )
                {
                    if( flash64k_state == FLASH_IDLEBYTE0 )
                    {
                        flash64k_state = FLASH_IDLEBYTE1;
                    }
                    else if( flash64k_state == FLASH_ERASEBYTE0 )
                    {
                        flash64k_state = FLASH_ERASEBYTE1;
                    }
                }
            }
            break;
        case FLASH_IDLEBYTE1:
        case FLASH_ERASEBYTE1:
            if( offset == 0x2aaa/4 && ~mem_mask == 0xff00ffff )
            {
                if( ( data & mem_mask ) == 0x00550000 )
                {
                    if( flash64k_state == FLASH_IDLEBYTE1 )
                    {
                        flash64k_state = FLASH_IDLEBYTE2;
                    }
                    else if( flash64k_state == FLASH_ERASEBYTE1 )
                    {
                        flash64k_state = FLASH_ERASEBYTE2;
                    }
                }
            }
            break;
        case FLASH_IDLEBYTE2:
        case FLASH_ERASEBYTE2:
            if( offset == 0x5555/4 && ~mem_mask == 0xffff00ff )
            {
                if( flash64k_state == FLASH_IDLEBYTE2 )
                {
                    switch( ( data & mem_mask ) >> 8 )
                    {
                        case 0x80:
                            flash64k_state = FLASH_ERASEBYTE0;
                            break;
                        case 0x90:
                            flash64k_state = FLASH_IDENT;
                            break;
                        case 0xa0:
                            flash64k_state = FLASH_WRITE;
                            break;
                    }
                }
                else if( flash64k_state == FLASH_ERASEBYTE2 )
                {
                    if( ( data & mem_mask ) == 0x00001000 )
                    {
                        UINT32 flashWord;
                        for( flashWord = 0; flashWord < 0x10000/4; flashWord++ )
                        {
                            gba_flash64k[flashWord] = 0xffffffff;
                        }
                        flash64k_state = FLASH_ERASE_ALL;
                    }
                }
            }
            else if( ( offset & 0xffffc3ff ) == 0 && ( data & mem_mask ) == 0x00000030 )
            {
                UINT32 flashWord;
                flash64k_page = offset >> 10;
                for( flashWord = offset; flashWord < offset + 0x1000/4; flashWord++ )
                {
                    gba_flash64k[flashWord] = 0xffffffff;
                }
                flash64k_state = FLASH_ERASE_4K;
            }
            break;
        case FLASH_IDENT:
            // Hack; any sensibly-written game should follow up with the relevant read, which will reset the state to FLASH_IDLEBYTE0.
            flash64k_state = FLASH_IDLEBYTE0;
            flash64k_w( machine, offset, data, ~mem_mask );
            break;
        case FLASH_ERASE_4K:
            // Hack; any sensibly-written game should follow up with the relevant read, which will reset the state to FLASH_IDLEBYTE0.
            flash64k_state = FLASH_IDLEBYTE0;
            flash64k_w( machine, offset, data, ~mem_mask );
            break;
        case FLASH_ERASE_ALL:
            // Hack; any sensibly-written game should follow up with the relevant read, which will reset the state to FLASH_IDLEBYTE0.
            flash64k_state = FLASH_IDLEBYTE0;
            flash64k_w( machine, offset, data, ~mem_mask );
            break;
        case FLASH_WRITE:
            COMBINE_DATA(&gba_flash64k[offset]);
            flash64k_state = FLASH_IDLEBYTE0;
            break;
    }
}

enum
{
	EEP_IDLE,
	EEP_COMMAND,
	EEP_ADDR,
	EEP_AFTERADDR,
	EEP_READ,
	EEP_WRITE,
	EEP_AFTERWRITE,
    EEP_READFIRST
};

static int eeprom_state = EEP_IDLE, eeprom_command, eeprom_count, eeprom_addr, eeprom_bits;
static UINT8 eep_data;

static READ32_HANDLER( eeprom_r )
{
	UINT32 out;

	switch (eeprom_state)
	{
		case EEP_IDLE:
//			printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, eeprom_state, activecpu_get_pc(), 1);
			return 0x00010001;	// "ready"
			break;

        case EEP_READFIRST:
            eeprom_count--;

            if (!eeprom_count)
            {
                eeprom_count = 64;
                eeprom_bits = 0;
                eep_data = 0;
                eeprom_state = EEP_READ;
            }
            break;
		case EEP_READ:
			if ((eeprom_bits == 0) && (eeprom_count))
			{
				eep_data = gba_eeprom[eeprom_addr];
		       //		printf("EEPROM read @ %x = %x (%x)\n", eeprom_addr, eep_data, (eep_data & 0x80) ? 1 : 0);
				eeprom_addr++;
				eeprom_bits = 8;
			}

			out = (eep_data & 0x80) ? 1 : 0;
			out |= (out<<16);
			eep_data <<= 1;

			eeprom_bits--;
			eeprom_count--;

			if (!eeprom_count)
			{
				eeprom_state = EEP_IDLE;
			}

//			printf("out = %08x\n", out);
//			printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %08x\n", offset, ~mem_mask, eeprom_state, activecpu_get_pc(), out);
			return out;
			break;
	}
//	printf("eeprom_r: @ %x, mask %08x (state %d) (PC=%x) = %d\n", offset, ~mem_mask, eeprom_state, activecpu_get_pc(), 0);
	return 0;
}

static WRITE32_HANDLER( eeprom_w )
{
	if (~mem_mask == 0x0000ffff)
	{
		data >>= 16;
	}

//	printf("eeprom_w: %x @ %x (state %d) (PC=%x)\n", data, offset, eeprom_state, activecpu_get_pc());

	switch (eeprom_state)
	{
		case EEP_IDLE:
			if (data == 1)
			{
				eeprom_state++;
			}
			break;

		case EEP_COMMAND:
			if (data == 1)
			{
                eeprom_command = EEP_READFIRST;
			}
			else
			{
				eeprom_command = EEP_WRITE;
			}
			eeprom_state = EEP_ADDR;
			eeprom_count = 6;
			eeprom_addr = 0;
			break;

		case EEP_ADDR:
			eeprom_addr <<= 1;
			eeprom_addr |= (data & 1);
			eeprom_count--;
			if (!eeprom_count)
			{
				eeprom_addr *= 8;	// each address points to 8 bytes
                if (eeprom_command == EEP_READFIRST)
				{
					eeprom_state = EEP_AFTERADDR;
				}
				else
				{
					eeprom_count = 64;
					eeprom_bits = 8;
					eeprom_state = EEP_WRITE;
					eep_data = 0;
				}
			}
			break;

		case EEP_AFTERADDR:
			eeprom_state = eeprom_command;
			eeprom_count = 64;
			eeprom_bits = 0;
			eep_data = 0;
            if( eeprom_state == EEP_READFIRST )
            {
                eeprom_count = 4;
            }
			break;

		case EEP_WRITE:
			eep_data<<= 1;
			eep_data |= (data & 1);
			eeprom_bits--;
			eeprom_count--;

			if (eeprom_bits == 0)
			{
				printf("%08x: EEPROM: %02x to %x\n", activecpu_get_pc(), eep_data, eeprom_addr );
				gba_eeprom[eeprom_addr] = eep_data;
				eeprom_addr++;
				eep_data = 0;
				eeprom_bits = 8;
			}

			if (!eeprom_count)
			{
				eeprom_state = EEP_AFTERWRITE;
			}
			break;

		case EEP_AFTERWRITE:
			eeprom_state = EEP_IDLE;
			break;
	}
}

static DEVICE_IMAGE_LOAD( gba_cart )
{
	UINT8 *ROM = memory_region(image->machine, "cartridge");
	int i;

	nvsize = 0;
	nvptr = (UINT8 *)NULL;

	image_fread(image, ROM, image_length(image));

	for (i = 0; i < image_length(image); i++)
	{
		if (!memcmp(&ROM[i], "EEPROM_", 7))
		{
			nvptr = (UINT8 *)&gba_eeprom;
			nvsize = 0x2000;

			if (image_length(image) <= (16*1024*1024))
			{
				memory_install_read32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xd000000, 0xdffffff, 0, 0, eeprom_r);
				memory_install_write32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xd000000, 0xdffffff, 0, 0, eeprom_w);
			}
			else
			{
				memory_install_read32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xdffff00, 0xdffffff, 0, 0, eeprom_r);
				memory_install_write32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xdffff00, 0xdffffff, 0, 0, eeprom_w);
			}
			break;
		}
		else if (!memcmp(&ROM[i], "SRAM_", 5))
		{
			nvptr = (UINT8 *)&gba_sram;
			nvsize = 0x10000;

			memory_install_read32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xe000000, 0xe00ffff, 0, 0, sram_r);
			memory_install_write32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xe000000, 0xe00ffff, 0, 0, sram_w);
			break;
		}
		else if (!memcmp(&ROM[i], "FLASH1M_", 8))
		{
			printf("game has 1M FLASH\n");
			break;
		}
		else if (!memcmp(&ROM[i], "FLASH", 5))
		{
			nvptr = (UINT8 *)&gba_flash64k;
			nvsize = 0x10000;

			memory_install_read32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xe000000, 0xe007fff, 0, 0, flash64k_r);
			memory_install_write32_handler(image->machine, 0, ADDRESS_SPACE_PROGRAM, 0xe000000, 0xe007fff, 0, 0, flash64k_w);
			break;
		}
		else if (!memcmp(&ROM[i], "SIIRTC_V", 8))
		{
			printf("game has RTC\n");
			break;
		}
	}

	// if save media was found, reload it
	if (nvsize > 0)
	{
		image_battery_load(image, nvptr, nvsize);
	}

	// mirror the ROM
	switch (image_length(image))
	{
		case 2*1024*1024:
			memcpy(ROM+0x200000, ROM, 0x200000);
		// intentional fall-through
		case 4*1024*1024:
			memcpy(ROM+0x400000, ROM, 0x400000);
		// intentional fall-through
		case 8*1024*1024:
			memcpy(ROM+0x800000, ROM, 0x800000);
		// intentional fall-through
		case 16*1024*1024:
			memcpy(ROM+0x1000000, ROM, 0x1000000);
			break;
	}

	return INIT_PASS;
}

static void gba_cartslot_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* cartslot */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:	 	   			info->i = 1; break;
		case MESS_DEVINFO_INT_MUST_BE_LOADED:				info->i = 0; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_LOAD: 						info->load = device_load_gba_cart; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case MESS_DEVINFO_STR_FILE_EXTENSIONS:				strcpy(info->s = device_temp_str(), "gba,bin"); break;

		default:   							cartslot_device_getinfo(devclass, state, info); break;
	}
}

SYSTEM_CONFIG_START(gbadv)
	CONFIG_DEVICE(gba_cartslot_getinfo)
SYSTEM_CONFIG_END

/* this emulates the GBA's hardware protection: the BIOS returns only zeros when the PC is not in it,
   and some games verify that as a protection check (notably Metroid Fusion) */

static OPBASE_HANDLER( gba_setopbase )
{
	if (address > 0x4000)
	{
		memory_set_bankptr(1, memory_region(machine, "bios")+0x4000);
	}
	else
	{
		memory_set_bankptr(1, memory_region(machine, "bios"));
	}

	return address;
}

static DRIVER_INIT(gbadv)
{
	memory_set_opbase_handler(0, gba_setopbase);

	state_save_register_global_array(gba_sram);
	state_save_register_global_array(gba_eeprom);
	state_save_register_global_array(gba_flash64k);
	      
	state_save_register_global_array(dma_regs);
	state_save_register_global_array(dma_src);
	state_save_register_global_array(dma_dst);
	state_save_register_global_array(dma_cnt);
	state_save_register_global_array(dma_srcadd);
	state_save_register_global_array(dma_dstadd);
	      
	state_save_register_global_array(timer_regs);
	state_save_register_global_array(timer_reload);
	      
	state_save_register_item("DISPCNT", 0, gba.DISPCNT);
	state_save_register_item("GRNSWAP", 0, gba.GRNSWAP);
	state_save_register_item("DISPSTAT", 0, gba.DISPSTAT);
	state_save_register_item("BG0CNT", 0, gba.BG0CNT);
	state_save_register_item("BG1CNT", 0, gba.BG1CNT);
	state_save_register_item("BG2CNT", 0, gba.BG2CNT);
	state_save_register_item("BG3CNT", 0, gba.BG3CNT);
	state_save_register_item("BG0HOFS", 0, gba.BG0HOFS);
	state_save_register_item("BG0VOFS", 0, gba.BG0VOFS);
	state_save_register_item("BG1HOFS", 0, gba.BG1HOFS);
	state_save_register_item("BG1VOFS", 0, gba.BG1VOFS);
	state_save_register_item("BG2HOFS", 0, gba.BG2HOFS);
	state_save_register_item("BG2VOFS", 0, gba.BG2VOFS);
	state_save_register_item("BG3HOFS", 0, gba.BG3HOFS);
	state_save_register_item("BG3VOFS", 0, gba.BG3VOFS);
	state_save_register_item("BG2PA", 0, gba.BG2PA);
	state_save_register_item("BG2PB", 0, gba.BG2PB);
	state_save_register_item("BG2PC", 0, gba.BG2PC);
	state_save_register_item("BG2PD", 0, gba.BG2PD);
	state_save_register_item("BG3PA", 0, gba.BG3PA);
	state_save_register_item("BG3PB", 0, gba.BG3PB);
	state_save_register_item("BG3PC", 0, gba.BG3PC);
	state_save_register_item("BG3PD", 0, gba.BG3PD);
	state_save_register_item("BG2X", 0, gba.BG2X);
	state_save_register_item("BG2Y", 0, gba.BG2Y);
	state_save_register_item("BG3X", 0, gba.BG3X);
	state_save_register_item("BG3Y", 0, gba.BG3Y);
	state_save_register_item("WIN0H", 0, gba.WIN0H);
	state_save_register_item("WIN1H", 0, gba.WIN1H);
	state_save_register_item("WIN0V", 0, gba.WIN0V);
	state_save_register_item("WIN1V", 0, gba.WIN1V);
	state_save_register_item("WININ", 0, gba.WININ);
	state_save_register_item("WINOUT", 0, gba.WINOUT);
	state_save_register_item("MOSAIC", 0, gba.MOSAIC);
	state_save_register_item("BLDCNT", 0, gba.BLDCNT);
	state_save_register_item("BLDALPHA", 0, gba.BLDALPHA);
	state_save_register_item("BLDY", 0, gba.BLDY);
	state_save_register_item("SOUNDCNT_X", 0, gba.SOUNDCNT_X);
	state_save_register_item("SOUNDCNT_H", 0, gba.SOUNDCNT_H);
	state_save_register_item("SOUNDBIAS", 0, gba.SOUNDBIAS);
	state_save_register_item("SIOMULTI0", 0, gba.SIOMULTI0);
	state_save_register_item("SIOMULTI1", 0, gba.SIOMULTI1);
	state_save_register_item("SIOMULTI2", 0, gba.SIOMULTI2);
	state_save_register_item("SIOMULTI3", 0, gba.SIOMULTI3);
	state_save_register_item("SIOCNT", 0, gba.SIOCNT);
	state_save_register_item("SIODATA8", 0, gba.SIODATA8);
	state_save_register_item("KEYCNT", 0, gba.KEYCNT);
	state_save_register_item("RCNT", 0, gba.RCNT);
	state_save_register_item("JOYCNT", 0, gba.JOYCNT);
	state_save_register_item("JOY_RECV", 0, gba.JOY_RECV);
	state_save_register_item("JOY_TRANS", 0, gba.JOY_TRANS);
	state_save_register_item("JOYSTAT", 0, gba.JOYSTAT);
	state_save_register_item("IR", 0, gba.IR);
	state_save_register_item("IE", 0, gba.IE);
	state_save_register_item("IF", 0, gba.IF);
	state_save_register_item("IME", 0, gba.IME);
	state_save_register_item("WAITCNT", 0, gba.WAITCNT);
	state_save_register_item("POSTFLG", 0, gba.POSTFLG);
	state_save_register_item("HALTCNT", 0, gba.HALTCNT);

	state_save_register_global(fifo_a_ptr);
	state_save_register_global(fifo_b_ptr);
	state_save_register_global(fifo_a_in);
	state_save_register_global(fifo_b_in);
	state_save_register_global_array(fifo_a);
	state_save_register_global_array(fifo_b);
}

/*    YEAR  NAME PARENT COMPAT MACHINE INPUT INIT CONFIG COMPANY      FULLNAME */
CONS( 2001, gba, 0,     0,     gbadv,    gbadv,  gbadv, gbadv,   "Nintendo", "Game Boy Advance", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE )
