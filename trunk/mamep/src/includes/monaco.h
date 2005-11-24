/* monaco.h */

#define kRESCUE_CAR_SPEED (1.75)

VIDEO_START( monaco );
VIDEO_UPDATE( monaco );

#define SCREEN_WIDTH	384 /* 12 car lengths */
#define SCREEN_HEIGHT	240 /* 15 car widths */

#define BRIDGE_YPOS	((SCREEN_HEIGHT-16)/2-8)
#define PAGE_SIZE		(14*32)

#define NUM_COMPUTER_CARS 4

extern struct monaco_gfx
{
	int bSignalVisible;
	int left_text;
	int right_text;

	int bExtendedPlay;

	int led_score;
	int led_time;

	int scroll;
	int left_page, right_page;
	int top_inset, bottom_inset;

	int player_x, player_y, player_tile, player_splash;

	int pool_x, pool_y;
	int rescue_x, rescue_y, rescue_tile;
	struct
	{
		int x,y,tile,color;
	} computer_car[NUM_COMPUTER_CARS];
} monaco_gfx;

enum
{
	GFX_UNKNOWN,GFX_EXPLOSION,GFX_PLAYER,GFX_SHAKE,GFX_SPINOUT1,GFX_SPINOUT2,
	GFX_SPRAY,GFX_COMPUTER,GFX_TEXT,
	GFX_TREE,GFX_GRASS,GFX_SHRUB,GFX_HOUSE,
	GFX_TUNNEL,GFX_POOL,GFX_BELT,
	GFX_RESCUE_CAR,GFX_SIGNAL,GFX_DUMMY,
	GFX_BRIDGE1,GFX_BRIDGE2
};

/* red */
#define TEXT_GAMEOVER		2
#define TEXT_EXTENDEDPLAY	0

/* blue */
#define TEXT_DEPOSITCOIN	3
#define TEXT_START			1

enum
{
	CYAN_PEN	= 0x91,
	BLACK_PEN	= 0x94,
	YELLOW_PEN	= 0x95,
	GREY_PEN	= 0x96
};

enum
{
	RESCUE_CLUT	= 0x50,
	HOUSE_CLUT	= 0x60,
	WATER_CLUT	= 0x70,
	PLAYER_CLUT	= 0x80,
	TREE_CLUT	= 0x84,
	SHRUB_CLUT	= 0x88,
	GRASS_CLUT	= 0x8c,
	CYAN_CLUT	= 0x90,
	RED_CLUT	= 0x92,
	YELLOW_CLUT	= 0x94
};

enum
{
	GREEN_CAR, YELLOW_CAR, /* slow */
	CYAN_CAR, PURPLE_CAR, /* medium */
	BLUE_CAR, /* fast */
	NUM_COMPUTER_CAR_TYPES
};

enum
{
	PAGE_SMOOTH,
	PAGE_SLIP,
	PAGE_TUNNEL,
	PAGE_GRAVEL,
	PAGE_BRIDGE
};
