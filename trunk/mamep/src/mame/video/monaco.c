/* Monaco GP video hardware simulation */

#include "driver.h"
#include "deprecat.h"
#include "video/generic.h"
#include "monaco.h"

#define plot_pixel(bitmap,x,y,col)	do { *BITMAP_ADDR16(bitmap, y, x) = col; } while (0)

struct monaco_gfx monaco_gfx;

static gfx_element *led_font;

static void build_led_font(void)
{
	static const UINT8 fontdata[10 * 8] =
	{
		0x70,0x88,0x88,0x88,0x88,0x88,0x70,0x00,
		0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x00,
		0x70,0x88,0x08,0x10,0x20,0x40,0xf8,0x00,
		0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00,
		0x10,0x30,0x50,0x90,0xf8,0x10,0x10,0x00,
		0xf8,0x80,0xf0,0x08,0x08,0x88,0x70,0x00,
		0x70,0x80,0xf0,0x88,0x88,0x88,0x70,0x00,
		0xf8,0x08,0x08,0x10,0x20,0x20,0x20,0x00,
		0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00,
		0x70,0x88,0x88,0x88,0x78,0x08,0x70,0x00
	};
	static const gfx_layout layout =
	{
		8,6,
		10,
		1,
		{ 0 },
		{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
		{ 0, 1, 2, 3, 4, 5 },
		8*8
	};

	led_font = allocgfx(&layout);
	if (!led_font)
		fatalerror("Fatal error: could not allocate memory for decimal font!");

	decodegfx(led_font, fontdata, 0, led_font->total_elements);

	led_font->color_base = 0;
	led_font->total_colors = 1;
}

static void draw_computer( mame_bitmap *bitmap )
{
	const rectangle *clip = &Machine->screen[0].visarea;
	int i;

	for( i=0; i<NUM_COMPUTER_CARS; i++ )
	{
		drawgfx(
			bitmap, Machine->gfx[GFX_COMPUTER],
			monaco_gfx.computer_car[i].tile,
			monaco_gfx.computer_car[i].color,
			0,0,
			monaco_gfx.computer_car[i].x,
			monaco_gfx.computer_car[i].y,
			clip,
			TRANSPARENCY_PEN, 0 );
	}

	drawgfx(
		bitmap, Machine->gfx[GFX_RESCUE_CAR],
		monaco_gfx.rescue_tile,
		0, /* color */
		0,0,
		monaco_gfx.rescue_x,
		monaco_gfx.rescue_y,
		clip,TRANSPARENCY_PEN,0 );
}

static void draw_pool( mame_bitmap *bitmap )
{
	drawgfx(
		bitmap, Machine->gfx[GFX_POOL],
		0,0, /* tile,color */
		0,0, /* flip */
		monaco_gfx.pool_x,
		monaco_gfx.pool_y,
		&Machine->screen[0].visarea,
		TRANSPARENCY_PEN,0 );
}

static void draw_player( mame_bitmap *bitmap )
{
	const rectangle *clip = &Machine->screen[0].visarea;
	int gfx;
	int tile;

	switch( monaco_gfx.player_splash )
	{
	case 0:
		drawgfx( bitmap, Machine->gfx[GFX_SPRAY],
			2,0,0,0,
			monaco_gfx.player_x,
			monaco_gfx.player_y+32-8,
			clip,TRANSPARENCY_PEN,0);
		drawgfx( bitmap, Machine->gfx[GFX_SPRAY],
			0,0,0,0,
			monaco_gfx.player_x,
			monaco_gfx.player_y-32+8,
			clip,TRANSPARENCY_PEN,0);
		break;
	case 1:
		drawgfx( bitmap, Machine->gfx[GFX_SPRAY],
			3,0,0,0,
			monaco_gfx.player_x,
			monaco_gfx.player_y+32-8,
			clip,TRANSPARENCY_PEN,0);
		drawgfx( bitmap, Machine->gfx[GFX_SPRAY],
			1,0,0,0,
			monaco_gfx.player_x,
			monaco_gfx.player_y-32+8,
			clip,TRANSPARENCY_PEN,0);
		break;
	}

	tile = monaco_gfx.player_tile;
	gfx = 0;
	if( tile>=0 )
	{
		switch( tile )
		{
		case 0:
		case 1: gfx = GFX_PLAYER; break;

		case 2:
		case 3: gfx = GFX_SPINOUT1; tile -= 2; break;

		case 4:
		case 5: gfx = GFX_SPINOUT2; tile -= 4; break;

		case 6:
		case 7:
		case 8:
		case 9: gfx = GFX_EXPLOSION; tile -= 6; break;

		case 10:
		case 11: gfx = GFX_SHAKE; tile -= 10; break;
		}

		drawgfx( bitmap, Machine->gfx[gfx],
			tile,0,
			0,0,
			monaco_gfx.player_x,
			monaco_gfx.player_y,
			clip,TRANSPARENCY_PEN,0);
	}
}

/*****************************************************************/

static void draw_strip( mame_bitmap *bitmap, int sy, int x0, int x1, int xpos, int pen )
{
	int sx;
	if( x0<xpos ) x0 = xpos;
	if( x1>xpos+PAGE_SIZE ) x1 = xpos+PAGE_SIZE;
	if( x0<0 ) x0 = 0;
	if( x1>SCREEN_WIDTH ) x1 = SCREEN_WIDTH;
	for( sx=x0; sx<x1; sx++ ) plot_pixel( bitmap,sx,sy,pen );
}

static void DrawSmoothZone( mame_bitmap *bitmap, int xpos )
{
	const rectangle *clip = &Machine->screen[0].visarea;

	const UINT8 data[14] =
	{
		GFX_GRASS,GFX_GRASS,GFX_GRASS,
		GFX_TREE,GFX_GRASS,GFX_TREE,
		GFX_GRASS,GFX_HOUSE,GFX_GRASS,
		GFX_SHRUB,GFX_SHRUB,
		GFX_GRASS,GFX_GRASS,GFX_GRASS
	};
	int top_inset = monaco_gfx.top_inset;
	int bottom_inset = monaco_gfx.bottom_inset;
	int i;

	draw_strip( bitmap, top_inset, xpos, xpos+PAGE_SIZE, xpos, Machine->pens[YELLOW_PEN] );
	draw_strip( bitmap, SCREEN_HEIGHT-1 - bottom_inset, xpos, xpos+PAGE_SIZE, xpos, Machine->pens[YELLOW_PEN] );

	for( i=0; i<14; i++ )
	{
		int code = data[i];
		const gfx_element *gfx = Machine->gfx[code];
		const gfx_element *belt = Machine->gfx[(code==GFX_HOUSE)?GFX_DUMMY:GFX_BELT];
		int j;

		for( j=0; j<3; j++ )
		{
			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos, monaco_gfx.top_inset-32-16-j*32,
				clip, TRANSPARENCY_NONE,0 );

			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos,SCREEN_HEIGHT-monaco_gfx.bottom_inset+j*32+16-8,
				clip, TRANSPARENCY_NONE,0 );
		}
		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos, monaco_gfx.top_inset-16,
			clip, TRANSPARENCY_NONE,0 );

		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos,SCREEN_HEIGHT-monaco_gfx.bottom_inset,
			clip, TRANSPARENCY_NONE,0 );

		xpos += 32;
	}
}

static void DrawSlipZone( mame_bitmap *bitmap, int xpos )
{
	const rectangle *clip = &Machine->screen[0].visarea;

	const UINT8 data[14] =
	{
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,GFX_HOUSE,
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,/* start  */GFX_SHRUB,GFX_SHRUB
	};

	int top_inset = monaco_gfx.top_inset;
	int bottom_inset = monaco_gfx.bottom_inset;
	int i;

	draw_strip( bitmap, top_inset, xpos, xpos+PAGE_SIZE, xpos, Machine->pens[YELLOW_PEN] );
	draw_strip( bitmap, SCREEN_HEIGHT-1 - bottom_inset, xpos, xpos+PAGE_SIZE, xpos, Machine->pens[YELLOW_PEN] );

	for( i=0; i<14; i++ ){
		int code = data[i];
		const gfx_element *gfx = Machine->gfx[code];
		const gfx_element *belt = Machine->gfx[(code==GFX_HOUSE)?GFX_DUMMY:GFX_BELT];
		int j;

		for( j=0; j<3; j++ ){
			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos, top_inset-32-j*32-16,
				clip, TRANSPARENCY_NONE,0 );

			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos,SCREEN_HEIGHT-bottom_inset+j*32+16-8,
				clip, TRANSPARENCY_NONE,0 );
		}

		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos, top_inset-16,
			clip, TRANSPARENCY_NONE,0 );

		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos,SCREEN_HEIGHT-bottom_inset,
			clip, TRANSPARENCY_NONE,0 );

		xpos += 32;
	}
}

static void DrawGravelZone( mame_bitmap *bitmap, int xpos ){
	const rectangle *clip = &Machine->screen[0].visarea;

	const UINT8 data[14] = {
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,GFX_HOUSE,
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB,
		GFX_SHRUB,GFX_SHRUB,GFX_SHRUB
	};
	int top_inset = monaco_gfx.top_inset;
	int bottom_inset = monaco_gfx.bottom_inset;
	int i;
	int xpos0 = xpos;

	for( i=0; i<14; i++ )
	{
		int code = data[i];
		const gfx_element *gfx = Machine->gfx[code];
		const gfx_element *belt = Machine->gfx[(code==GFX_HOUSE)?GFX_DUMMY:GFX_BELT];
		int j;
		int ypos;

		/* draw gravel */
		if( data[i]!=GFX_HOUSE )
		{
			ypos = SCREEN_HEIGHT-bottom_inset-32;
			drawgfx( bitmap, Machine->gfx[GFX_BELT],
				1,1, /* number, color */
				0,0, /* no flip */
				xpos, ypos+32-8,
				clip, TRANSPARENCY_NONE,0 );
			ypos-=24;
			while( ypos>0 ){
				drawgfx( bitmap, Machine->gfx[GFX_BELT],
					1,1, /* number, color */
					0,0, /* no flip */
					xpos, ypos,
					clip, TRANSPARENCY_NONE,0 );
				ypos -= 16;
			}
		}

		for( j=0; j<3; j++ ){
			ypos = SCREEN_HEIGHT-bottom_inset+16+j*32-8;
			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos, ypos,
				clip, TRANSPARENCY_NONE,0 );
		}

		for( j=0; j<3; j++ ){
			ypos = top_inset-32-16-j*32;
			drawgfx( bitmap, gfx,
				0,0, /* number, color */
				0,0, /* no flip */
				xpos, ypos,
				clip, TRANSPARENCY_NONE,0 );
		}

		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos, top_inset-16,
			clip, TRANSPARENCY_NONE,0 );

		drawgfx( bitmap, belt,
			0,0, /* number, color */
			0,0, /* no flip */
			xpos,SCREEN_HEIGHT-bottom_inset,
			clip, TRANSPARENCY_NONE,0 );


		xpos += 32;
	}
	draw_strip( bitmap, top_inset, xpos0, xpos0+PAGE_SIZE, xpos0, Machine->pens[YELLOW_PEN] );
	draw_strip( bitmap, SCREEN_HEIGHT-1 - bottom_inset, xpos0, xpos0+PAGE_SIZE, xpos0, Machine->pens[YELLOW_PEN] );
}

static void DrawBridgeZone( mame_bitmap *bitmap, int xpos )
{
	const rectangle *clip = &Machine->screen[0].visarea;
	const gfx_element *gfx1 = Machine->gfx[GFX_BRIDGE1];
	const gfx_element *gfx2 = Machine->gfx[GFX_BRIDGE2];

	int i;
	for( i=0; i<14; i++ )
	{
		int j;
		const gfx_element *gfx = (i==0)?gfx2:gfx1;
		for( j=0; j<7; j++ )
		{
			int flip;
			for( flip=0; flip<=1; flip++ )
			{
				int ypos = flip?(SCREEN_HEIGHT-16-j*16)+8:j*16-8;
				if( j<5 )
				{ /* water */
					drawgfx( bitmap, gfx1,
						0,0, /* number, color */
						0,flip,
						xpos, ypos,
						clip, TRANSPARENCY_NONE,0 );
				}
				else
				{ /* edge of bridge */
					drawgfx( bitmap, gfx,
						j-5,0, /* number, color */
						0,flip,
						xpos, ypos,
						clip, TRANSPARENCY_NONE,0 );
				}
			}
		}
		xpos += 32;
	}
}

static void DrawTunnelZone( mame_bitmap *bitmap, int xpos )
{
	int pen = Machine->pens[YELLOW_PEN];
	int top_inset = monaco_gfx.top_inset;
	int bottom_inset = monaco_gfx.bottom_inset;
	draw_strip( bitmap, top_inset, xpos, xpos+PAGE_SIZE, xpos, pen );
	draw_strip( bitmap, SCREEN_HEIGHT-1 - bottom_inset, xpos, xpos+PAGE_SIZE, xpos, pen );
}

static void DrawTunnelWall( mame_bitmap *bitmap, int xpos )
{
	const rectangle *clip = &Machine->screen[0].visarea;
	rectangle clip2 = Machine->screen[0].visarea;
	const gfx_element *gfx = Machine->gfx[GFX_TUNNEL];
	int top_inset = monaco_gfx.top_inset - 16;
	int bottom_inset = monaco_gfx.bottom_inset - 16;
	int i;

	clip2.min_y = SCREEN_HEIGHT-bottom_inset;

	for( i=0; i<14; i++ )
	{
		int j;
		for( j=0; j<2; j++ )
		{
			drawgfx( bitmap, gfx,
				1,0, /* number, color */
				0,0, /* no flip */
				xpos, top_inset-32-j*32,
				clip, TRANSPARENCY_PEN,0 );

			drawgfx( bitmap, gfx,
				1,0, /* number, color */
				0,0, /* no flip */
				xpos, SCREEN_HEIGHT-bottom_inset+j*32 - 8,
				&clip2, TRANSPARENCY_PEN,0 );
		}
		xpos += 32;
	}
}

static void draw_light_helper( mame_bitmap *bitmap, int xpos )
{
	int pen = Machine->pens[BLACK_PEN];
	const unsigned char *source = memory_region( REGION_GFX1 );
	int x0 = monaco_gfx.player_x-128;
	int y0 = monaco_gfx.player_y-48;
	int sy;
	for( sy=0; sy<SCREEN_HEIGHT; sy++ )
	{
		int i = sy-y0;
		if( i<0 || i>=128 )
		{
			draw_strip( bitmap, sy, xpos, xpos+PAGE_SIZE, xpos, pen );
		}
		else
		{
			int left = x0+source[i];
			int right = x0+source[i+128];
			draw_strip( bitmap, sy, xpos, left, xpos, pen );
			draw_strip( bitmap, sy, right, xpos+PAGE_SIZE, xpos, pen );
		}
	}
}

static void draw_wall( mame_bitmap *bitmap )
{
	if( monaco_gfx.right_page == PAGE_TUNNEL )
	{
		DrawTunnelWall( bitmap, SCREEN_WIDTH-14*32+monaco_gfx.scroll );
	}

	if( monaco_gfx.left_page == PAGE_TUNNEL )
	{
		DrawTunnelWall( bitmap, SCREEN_WIDTH-14*32*2+monaco_gfx.scroll );
	}
}

static void draw_light( mame_bitmap *bitmap )
{
	if( monaco_gfx.right_page == PAGE_TUNNEL )
	{
		draw_light_helper( bitmap, SCREEN_WIDTH-14*32+monaco_gfx.scroll );
	}

	if( monaco_gfx.left_page == PAGE_TUNNEL )
	{
		draw_light_helper( bitmap, SCREEN_WIDTH-14*32*2+monaco_gfx.scroll );
	}
}

static void draw_page( mame_bitmap *bitmap, int which, int xpos )
{
	rectangle r;
	r.min_x = xpos;
	r.max_x = xpos+PAGE_SIZE-1;
	r.min_y = 0;
	r.max_y = SCREEN_HEIGHT-1;
	fillbitmap( bitmap,Machine->pens[(which!=PAGE_SLIP)?GREY_PEN:CYAN_PEN],&r );
	switch( which )
	{
	case PAGE_SMOOTH:
		DrawSmoothZone( bitmap, xpos );
		break;

	case PAGE_SLIP:
		DrawSlipZone( bitmap, xpos );
		break;

	case PAGE_TUNNEL:
		DrawTunnelZone( bitmap, xpos );
		break;

	case PAGE_GRAVEL:
		DrawGravelZone( bitmap, xpos );
		break;

	case PAGE_BRIDGE:
		DrawBridgeZone( bitmap, xpos );
		break;
	}
}

static void draw_background( mame_bitmap *bitmap )
{
	draw_page(
		bitmap,
		monaco_gfx.right_page,
		SCREEN_WIDTH-14*32+monaco_gfx.scroll );

	draw_page(
		bitmap,
		monaco_gfx.left_page,
		SCREEN_WIDTH-14*32*2+monaco_gfx.scroll );
}

static void draw_text( mame_bitmap *bitmap )
{
	const rectangle *clip = &Machine->screen[0].visarea;
	int sx = (SCREEN_WIDTH-8)/2;
	int sy = (SCREEN_HEIGHT-128)/2;

	if( monaco_gfx.left_text>0 )
	{
		drawgfxzoom( bitmap, Machine->gfx[GFX_TEXT],
			monaco_gfx.left_text,0,
			0,0, /* flip */
			sx-96,sy,
			clip,TRANSPARENCY_PEN,0,
			1<<16, 2<<16
		);
	}

	if( monaco_gfx.right_text>0 )
	{
		drawgfxzoom( bitmap, Machine->gfx[GFX_TEXT],
			monaco_gfx.right_text,1,
			0,0, /* flip */
			SCREEN_WIDTH-32,sy,
			clip,TRANSPARENCY_PEN,0,
			1<<16, 2<<16
		);
	}
}

static void draw_leds( mame_bitmap *bitmap )
{
	int i, data;

	data = monaco_gfx.led_score;
	for( i=3; i>=0; i-- )
	{
		drawgfx( bitmap, led_font,
			data%10,0, /* number, color */
			0,1, /* no flip */
			0,SCREEN_HEIGHT-6-i*6,
			NULL, TRANSPARENCY_NONE,0 );
		data = data/10;
	}

	data = monaco_gfx.led_time;
	for( i=1; i>=0; i-- )
	{
		drawgfx( bitmap, led_font,
			data%10,0, /* number, color */
			0,1, /* no flip */
			12,SCREEN_HEIGHT-6-i*6,
			NULL, TRANSPARENCY_NONE,0 );
		data = data/10;
	}
}

void draw_signal( mame_bitmap *bitmap )
{
	if( monaco_gfx.bSignalVisible )
	{
		drawgfx(
			bitmap, Machine->gfx[GFX_SIGNAL],
			0,0, /* number, color */
			0,0, /* no flip */
			32,(SCREEN_HEIGHT - 32)/2,
			NULL, TRANSPARENCY_NONE,0 );
	}
}

VIDEO_UPDATE( monaco )
{
	draw_background( bitmap );
	draw_pool( bitmap );
	draw_computer( bitmap );
	draw_light( bitmap );
	draw_wall( bitmap );
	draw_text( bitmap );
	draw_player( bitmap );
	draw_signal( bitmap );
	draw_leds( bitmap );
	return 0;
}

VIDEO_START( monaco )
{
	int i;

	palette_set_color_rgb( machine, 0, 0x00,0x00,0x00 ); /* black (tire) */
	palette_set_color_rgb( machine, 1, 0xff,0xff,0xff ); /* white (trim) */
	/* computer car */
	for( i=0; i<NUM_COMPUTER_CAR_TYPES; i++ )
	{
		const unsigned char clut[3*NUM_COMPUTER_CAR_TYPES] =
		{
			0x00,0xff,0x00, /* green car */
			0xff,0xff,0x00, /* yellow car */
			0x00,0xff,0xff, /* cyan car */
			0xff,0x00,0xff, /* purple car */
			0x00,0x00,0xff  /* blue car */
		};

		palette_set_color_rgb( machine, 16*i+0x8, 0x00,0x00,0x00 ); /* black (tire) */
		palette_set_color_rgb( machine, 16*i+0xa, 0xff,0xff,0xff ); /* white (trim) */
		palette_set_color_rgb( machine, 16*i+0xc, clut[i*3+0],clut[i*3+1],clut[i*3+2] );
	}
	/* rescue car */
	palette_set_color_rgb( machine, 0x50+0x8, 0x00,0x00,0x00 ); // black
	palette_set_color_rgb( machine, 0x50+0x9, 0xff,0xff,0x00 ); // yellow
	palette_set_color_rgb( machine, 0x50+0xa, 0xff,0xff,0xff ); // white
	palette_set_color_rgb( machine, 0x50+0xc, 0xff,0x00,0x00 ); // red (light)
	/* house */
	palette_set_color_rgb( machine, 0x60+0x0, 0x00,0x00,0x00 ); // ground
	palette_set_color_rgb( machine, 0x60+0x9, 0xff,0x00,0x00 ); // right roof
	palette_set_color_rgb( machine, 0x60+0xa, 0xff,0x00,231 ); // left roof
	palette_set_color_rgb( machine, 0x60+0xb, 0x00,198,255 ); // front
	palette_set_color_rgb( machine, 0x60+0xc, 0xff,0xff,0xff ); // trim
	/* water */
	palette_set_color_rgb( machine, 0x70, 0x00,0x00,0xff );//l.blue
	palette_set_color_rgb( machine, 0x71, 0xff,0x00,0x00 );//red
	palette_set_color_rgb( machine, 0x72, 0xff,0xff,0x00 );//yellow
	palette_set_color_rgb( machine, 0x73, 0x00,0x00,0x00 );//black
	palette_set_color_rgb( machine, 0x74, 0x00,0x00,0x00 );//N/A
	palette_set_color_rgb( machine, 0x75, 0xff,0xff,0xff );//white
	palette_set_color_rgb( machine, 0x76, 0x9f,0x9f,0x9f );//grey (road)
	palette_set_color_rgb( machine, 0x77, 0x00,0x00,0x9f );//d.blue
	/* player car */
	palette_set_color_rgb( machine, 0x80, 0x00,0x00,0x00 );
	palette_set_color_rgb( machine, 0x81, 0xff,0x00,0x00 ); // red (car body)
	palette_set_color_rgb( machine, 0x82, 0xff,0xff,0xff ); // white (trim)
	palette_set_color_rgb( machine, 0x83, 0x00,0x00,0x00 ); // black (tire)
	/* tree */
	palette_set_color_rgb( machine, 0x84, 0xff,0xff,0x00 ); // yellow
	palette_set_color_rgb( machine, 0x85, 0x00,0x00,0x00 ); // dark green
	palette_set_color_rgb( machine, 0x86, 0x00,165,0x00 ); // light green
	palette_set_color_rgb( machine, 0x87, 0x00,0x00,0x00 ); // black?
	/* shrub */
	palette_set_color_rgb( machine, 0x88, 0x00,0x00,0x00 );
	palette_set_color_rgb( machine, 0x89, 0x00,0xff,0x00 );
	palette_set_color_rgb( machine, 0x8a, 0x00,0x00,0xff );
	palette_set_color_rgb( machine, 0x8b, 0xff,0xff,0xff );
	/* grass */
	palette_set_color_rgb( machine, 0x8c, 0x00,0x00,0x00 );
	palette_set_color_rgb( machine, 0x8d, 0x00,0x9f,0x00 ); // dark green
	palette_set_color_rgb( machine, 0x8e, 0x00,0x00,0x00 );
	palette_set_color_rgb( machine, 0x8f, 0x00,0x00,0x9f ); // dark blue
	palette_set_color_rgb( machine, 0x90, 0x00,0x00,0x00 ); // black
	palette_set_color_rgb( machine, 0x91, 0x00,0xff,0xff ); // cyan
	palette_set_color_rgb( machine, 0x92, 0x00,0x00,0x00 ); // grey
	palette_set_color_rgb( machine, 0x93, 0xff,0x00,0x00 ); // red
	palette_set_color_rgb( machine, 0x94, 0x00,0x00,0x00 ); // black
	palette_set_color_rgb( machine, 0x95, 255,215,0 ); // yellow trim
	palette_set_color_rgb( machine, 0x96, 132,132,132 ); // grey (road)
// 0,198,255: wet road
// 255,215,0: yellow trim

	build_led_font();
}
