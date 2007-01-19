/*
Monaco GP Simulator v0.1
http://www.jps.net/camilty/monman.htm
Phil Stroffolino

To Do:
- abstract and externalize in-game parameters (speeds,accel,timing)
- fix distribution of computer car
- computer cars should avoid rescue car lane when present
- computer cars should move to center in bridge zone
- player lives not hooked up
- high score tracking
- extended play colors

General:
	Score Increases as a function of the player's speed.
	Speed of Terrain scrolling is independent.

Computer Cars:
	The bridge section is different from the others (besides all the cars going
	faster after so many points(6K and 8K).

Display Panel:
	The cockpit model displays TIME (2 digit), top 5 scores (4 digits each),
	your Score (4 digits), Players to Date (3 digits)(daily), and
	Ranking (3 digits) (this is final score ranking, not position during the game).

	The top 999 high scores are internally recorded in a RAM.
	The player's ranking is updated only after gamplay is finished (during the
	fanfare).

Timed Play:
	X000-X295	A-Smooth Zone
	X295-X450	B-Slip Zone
	X450-X795	A-Smooth Zone
	X795-X000	C-Tunnel

Extended Play:
	X000-X095	A-Smooth Zone
	X095-X150	D-Gravel Zone
	X150-X295	A-Smooth Zone
	X295-X475	B-Slip Zone
	X475-X595	A-Smooth Zone
	X595-X635	E-Bridge Zone
	X635-X795	A-Smooth Zone
	X795-X000	C-Tunnel Zone

Oil Slicks (pool):
	X185		(during zone A)
	X685		(during zone A)
	After 8000 points, two pools come out in succession
	(the second about 20-30 points from the first).

Rescue Car (firetruck)
	The rescue car only appears during Extended play.  Tolerance -50 (not+)
	X000
	X075
	X125
	X250
	X500

	The rescue car appears in a mid-right lane.  It does not move side to side,
	it just moves slowly up the screen until it goes off the screen.

	In extended play, the rescue car was observed to come out AT LEAST ONCE
	during each 1000 points cycle.  I have observed the rescue car coming
	out as much a 3 TIMES in one 1000 point cycle.
	I could not find a pattern in the time or count interval between the
	rescue cars.  However, 72% were between 750 and 1250 point intervals
	(most around 875) while the remaining 28% were 500 point intervals or
	lower.  Therfore, we can narrow down WHERE the rescue cars appear, but
	IF it is going to appear seems to have some randomness to it.


notes:
	when the player first appears (game start, after a spinout, after
	a crash), the player's car shows up on the extreme right of the
	screen and is totally invulnerable (even if over water on the
	bridge scene!).  The road doesn't scroll at all until you give the
	car some gas.  Note that once you start moving, you can never
	completely come to a stop.

	Once you move to left onto the center part of the
	road, normal collision behavior takes over.  You earn no points
	while in the invulnerable state.  The player can remain in this
	state forever during timed play (until time runs out).  In extended
	play, the player's car will explode after several seconds of driving.



In extended play, the rescue car was observed to come out AT LEAST ONCE
during each 1000 points cycle.  I have observed the rescue car coming out
as much a 3 TIMES in one 1000 point cycle.

I could not find a pattern in the time or count interval between the rescue
cars.  However,
72% were between 750 and 1250 point intervals (most around 875) while the
remaining 28% were 500 point intervals or lower.  Therfore, we can narrow
down WHERE the resuce cars appear, but IF it is going to appear seems to
have some randomness to it.

---------------------------------------------------------------------------

Board 96598-P is an oscillator board that generates the game sounds.
It is activated by outputs from Board Assy A (96577X).

Board Assy's A (96577X) and B (96578X) are the main guts of the gameplay
and contain the sprite roms.

Board Assy A
- accepts all of the game inputs
- sends the signals for the sounds to the Oscillator board
- outputs to the L.E.D. score display board
- directly interacts with Board Assy B
	Components:
		Start Timer Control
		Player Control (pedal, steering, shifter)
		My Car light control, explosion, and pool (oil slick) skid
		Other Car Control

Board Assy B
- outputs the video
- outputs to the L.E.D. score display board
- directly interacts with Board Assy A.
	Components:
		Display of Letter
		Road Movement Horizontal
		Road Movement Vertical (Right and Left)
		Background (sprite) coloring
		Road Object (tunnel, bridge, water) control
		Signal Sync
		Rescue Car, Player Car, Bridge--Video and Control

	(On the second set, boards A and B have different part numbers
	(97091X and 97092X) and two less roms.  These boards were made
	later then the first set.  I have interchanged these boards with
	each other, and they seem to be interchangable, the only
	difference seems to be the rom size, which only 2 of the roms
	utilize the full 1024 bytes (second set).)

Oscillator Board 96598
ROM ID		IC#
PRm-40		IC21			PRm-40		<both type 6331-1, 16 pin>
	engine (constant; pitch varies with speed)
	passing (two variations)
	bonus (chimes)
	fanfare
	explosion
	slip zone
	siren (rescue car)
*/

#include "driver.h"
//#include "vidhrdw/generic.h"
#include "sound/samples.h"
#include "monaco.h"

enum
{
	eSAMPLE_PASS1,
	eSAMPLE_PASS2,
	eSAMPLE_CRASH,
	eSAMPLE_ENGINE,
	eSAMPLE_EXTEND,
	eSAMPLE_FANFARE,
	eSAMPLE_SIREN,
	eSAMPLE_SLIP
};

#define DOWN	0x01
#define UP	0x02
#define RIGHT	0x04
#define LEFT	0x08
#define COIN	0x10
#define ACCEL1	0x20
#define ACCEL2	0x40
#define ACCEL3	0x80

#define IS_PRESSED(key)		(input_port_0_r(0) & key)

static struct
{
	enum
	{
		MODE_ATTRACT,
		/*	cars zoom up the screen (max 2 on the screen at a time)
		 *	player's car is not visible
		 *	"GAME OVER" is centered near the middle-top
		 *	"DEPOSIT COIN" is centered near the bottom
		 */

		MODE_START,

		MODE_INVULNERABLE,

		MODE_NORMAL,	/* driving */

		MODE_GRAVEL,

		MODE_SLIP1,		/* skidding */
		MODE_SLIP2,

		MODE_SPINOUT1,
		MODE_SPINOUT2,
		MODE_SPINOUT3,
		MODE_SPINOUT4,

		MODE_CRASH1,
		MODE_CRASH2,
		MODE_CRASH3,
		MODE_CRASH4
	} mode;

	int anim_timer;

	/* LED display */
	UINT16 total_play;
	UINT16 ranking;
	UINT32 score;
	UINT32 time;

	int bShaking;
	double speed;
	double player_ypos;

	double computer_speed;

	struct
	{
		double xpos,ypos,dy;
	} computer_car[NUM_COMPUTER_CARS];

	double rescue_xpos;
	double pool_xpos;

	double scroll, distance;

	int ticks;
	int page_current, page_next, page_next2;

	double track_bottom_inset, track_top_inset;
	double track_bottom_delta, track_top_delta;
} state;

enum
{
	COLLISION_CRASH,
	COLLISION_POOL
};

static int get_player_xpos( void ){
	return SCREEN_WIDTH-32-state.speed;
}

static void handle_collision( int sx, int sy, int width, int height, int type )
{
	int px, py;

	if( state.mode == MODE_NORMAL )
	{
		px = get_player_xpos();
		py = state.player_ypos + 8;

		if( px < sx + width &&
			sx < px + 32 &&
			py < sy + height &&
			sy < py + 16 )
		{
			sample_start( 0/*channel*/, eSAMPLE_CRASH, 0/*don't loop*/ );
			sample_set_volume( 0/*channel*/,100 );

			state.anim_timer = 0;
			if( type == COLLISION_CRASH )
			{
				if( state.time>0 )
				{
					state.mode = MODE_SPINOUT1;
				}
				else
				{
					state.mode = MODE_CRASH1;
				}
			}
			else if( type == COLLISION_POOL )
			{
				state.pool_xpos = 512;
				state.mode = MODE_SLIP1;
			}
		}
	}
}

/*****************************************************************/

static int read_coin( void )
{
	static int old_trigger;
	if( IS_PRESSED(COIN) )
	{
		old_trigger = 1;
	}
	else {
		if( old_trigger ){
			old_trigger = 0;
			return 1;
		}
		old_trigger = 0;
	}
	return 0;
}

static void update_player_speed( void )
{
	static int gear;
	int desired_speed;
	int accel;

	accel = 0;
	if( IS_PRESSED(ACCEL1) ) accel = 1;
	if( IS_PRESSED(ACCEL2) ) accel = 2;
	if( IS_PRESSED(ACCEL3) ) accel = 3;

	if( state.mode == MODE_START )
	{
		if( accel ) state.mode = MODE_INVULNERABLE;
		else return;
	}

	if( IS_PRESSED(UP) )
	{
		gear = 1;
	}
	else if( IS_PRESSED(DOWN) )
	{
		gear = 0;
	}

	/* min: 0; max: 6 */
	desired_speed = 20*(accel?(gear*3+accel):0);

	if( state.speed<=desired_speed )
	{
		state.speed += gear?0.25:0.5;
		if( state.speed>desired_speed ) state.speed = desired_speed;
	}
	else if( state.speed>desired_speed )
	{
		state.speed-= 2.0;
		if( state.speed<0 ) state.speed = 0;
	}
}

static double get_player_delta( void )
{
	if( state.mode == MODE_ATTRACT || state.mode == MODE_START ) return 0;
	return state.speed/16.0;
}

static void HandlePlayer( void )
{
	double dy = 0;
	int sy = state.player_ypos;
	update_player_speed();
	if( IS_PRESSED(RIGHT) && sy>0 ) dy-=1.0;
	if( IS_PRESSED(LEFT) && sy<SCREEN_HEIGHT-32 ) dy+=1.0;
	if( state.page_current == PAGE_SLIP ) dy = dy*0.5;
	state.player_ypos += dy;
}

/*****************************************************************/

static void update_computer_cars( void )
{
	int i;
	int delta = get_player_delta();
	for( i=0; i<NUM_COMPUTER_CARS; i++ )
	{
		int top_inset = state.track_top_inset;
		int bottom_inset = state.track_bottom_inset;
		int sx,sy;
		state.computer_car[i].xpos += delta-state.computer_speed;
		if( state.computer_car[i].xpos < -256 )
		{
			state.computer_car[i].xpos += SCREEN_WIDTH*2;
		}
		else if( state.computer_car[i].xpos > SCREEN_WIDTH + 256 )
		{
			state.computer_car[i].xpos -= SCREEN_WIDTH*2;
		}

		state.computer_car[i].ypos += state.computer_car[i].dy;

		sx = state.computer_car[i].xpos;
		sy = state.computer_car[i].ypos;

		if( ( state.computer_car[i].dy<0 && sy<top_inset ) ||
		    (	state.computer_car[i].dy>0 && sy+16>SCREEN_HEIGHT-bottom_inset ) )
		{
			state.computer_car[i].dy *= -1;
		}

		handle_collision( sx-32, sy, 32, 16, COLLISION_CRASH );
	}
}

#define DELTA_TIME (256.0/60)

static void HandleRescue( void )
{
	if( state.rescue_xpos > -64 )
	{
		state.rescue_xpos -= kRESCUE_CAR_SPEED;
		handle_collision(
			state.rescue_xpos,
			(SCREEN_HEIGHT-32)/2,
			64,
			32,
			COLLISION_CRASH );
	}
}

static void HandlePool( void )
{
	if( state.pool_xpos < SCREEN_WIDTH )
	{
		state.pool_xpos += get_player_delta();
		handle_collision(
			state.pool_xpos,
			(SCREEN_HEIGHT-32)/2,
			32,
			32,
			COLLISION_POOL );
	}
}

static void HandleEvents( void )
{
	int score = state.score/256;
	int event = (score)%1000;

	if( state.page_current == PAGE_SMOOTH &&
		(score==8000 || score == 8030 || event == 185 || event == 685)  )
	{
		state.pool_xpos = -32*256;
	}

	if( state.time <=0 )
	{
		if(	state.rescue_xpos<= -64 &&
			(rand()&3) == 0 &&
			(event == 0 || event == 75 || event == 125 || event == 250 || event == 500) )
		{
			state.rescue_xpos = SCREEN_WIDTH;
		}
	}

	if( state.time>0 )
	{
		/* Timed Play */
		if( event == 0 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 295 ) state.page_next2 = PAGE_SLIP;
		else if( event == 450 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 795 ) state.page_next2 = PAGE_TUNNEL;
	}
	else
	{
		/* Extended Play */
		if( event == 0 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 95 ) state.page_next2 = PAGE_GRAVEL;
		else if( event == 150 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 295 ) state.page_next2 = PAGE_SLIP;
		else if( event == 475 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 595 ) state.page_next2 = PAGE_BRIDGE;
		else if( event == 635 ) state.page_next2 = PAGE_SMOOTH;
		else if( event == 795 ) state.page_next2 = PAGE_TUNNEL;
	}
}

static INTERRUPT_GEN( monaco_interrupt )
{
	int i;
	UINT32 old_score = state.score;
	double delta = get_player_delta();

	HandleRescue();
	HandlePool();
	HandlePlayer();

	state.score += state.speed;
	/* cap the score at 9999 */
	if( state.score> 9999*256 ) state.score = 9999*256;

	if( state.time>0 )
	{
		if( state.mode != MODE_ATTRACT && state.mode != MODE_START )
		{
			if( state.time<DELTA_TIME ) state.time = 0;
			else state.time -= DELTA_TIME;
		}
	}

	if( state.score > old_score )
	{
		HandleEvents();
	}

	switch( state.mode )
	{
	case MODE_ATTRACT:
		state.time = 90*256;
		state.score = 0;
		state.pool_xpos = SCREEN_WIDTH;
		state.rescue_xpos = -64;

		if( read_coin() )
		{
			sample_start( 1/*channel*/, eSAMPLE_ENGINE, 1/*loop*/ );
			sample_set_volume( 1/*channel*/,100 );

			state.speed = 0;
			state.mode = MODE_START;
			state.anim_timer = 0;
			state.player_ypos = 0;//32;
		}
		break;

	case MODE_START:
		state.speed = 0;
		break;
	default:
		break;
	}

	state.scroll += delta;
	if( state.scroll>=14*32 )
	{
		state.scroll -= 14*32;
		state.page_current = state.page_next;
		state.page_next = state.page_next2;
	}

	state.ticks++; /* for animation */

	if(	state.page_next != PAGE_TUNNEL && state.page_current != PAGE_TUNNEL &&
		state.page_next != PAGE_BRIDGE && state.page_current != PAGE_BRIDGE )
	{
		/* delta: player speed */
		state.track_bottom_inset += state.track_bottom_delta*delta/8;
		state.track_top_inset += state.track_top_delta*delta/8;

		if( state.track_bottom_inset <= 32 )
		{
			state.track_bottom_delta = 1;
		}
		else if( state.track_bottom_inset >= 96 )
		{
			state.track_bottom_delta = -1;
		}

		if( state.track_top_inset <= 32 )
		{
			state.track_top_delta = 1;
		}
		else if( state.track_top_inset >= 64 )
		{
			state.track_top_delta = -1;
		}
	}

	switch( state.mode )
	{
	case MODE_ATTRACT:
		break;
	case MODE_START:
		if( state.speed )
		{
			state.mode = MODE_INVULNERABLE;
			state.anim_timer = 0;
		}
		break;

	case MODE_INVULNERABLE:
		state.anim_timer++;
		if( state.anim_timer > 60*2 )
		{
			state.mode = MODE_NORMAL;
			state.anim_timer = 0;
		}
		break;

	case MODE_NORMAL:
		break;

	case MODE_GRAVEL:
		break;

	case MODE_SLIP1:
	case MODE_SLIP2:
		state.anim_timer++;
		if( state.anim_timer>16 )
		{
			state.anim_timer = 0;
			if( state.mode!=MODE_SLIP2 )
			{
				state.mode++;
			}
			else
			{
				state.mode = MODE_NORMAL;
			}
		}
		break;

	case MODE_SPINOUT1:
	case MODE_SPINOUT2:
	case MODE_SPINOUT3:
	case MODE_SPINOUT4:
		if( state.player_ypos > 0 )
		{
			state.mode = ((state.anim_timer/2)&3)+MODE_SPINOUT1;
			state.anim_timer++;
			state.player_ypos-=2.0;
		}
		else
		{
			state.player_ypos = 0;
			state.mode = MODE_INVULNERABLE;
			state.anim_timer = 0;
		}
		if( state.speed > 0 )
		{
			state.speed-= 1.0;
			if( state.speed<0 ) state.speed = 0;
		}
		break;

	case MODE_CRASH1:
	case MODE_CRASH2:
	case MODE_CRASH3:
	case MODE_CRASH4:
		state.anim_timer++;
		if( state.anim_timer>8 )
		{
			state.anim_timer = 0;
			if( state.mode!=MODE_CRASH4 )
			{
				state.mode++;
			}
			else
			{
				state.mode = MODE_INVULNERABLE;
				state.player_ypos = 0;
			}
		}
		break;
	default:
		break;
	}

	if( state.mode == MODE_NORMAL )
	{
		if( state.player_ypos+16 > state.track_top_inset+8 &&
			state.player_ypos+16 < SCREEN_HEIGHT - (state.track_bottom_inset+8) )
		{
			state.bShaking = 0;
		}
		else if( state.player_ypos+16 > state.track_top_inset+8 - 16 &&
			state.player_ypos+16 < SCREEN_HEIGHT - (state.track_bottom_inset+8 - 16) )
		{
			/* brush roadside */
			state.bShaking = 1;
		}
		else
		{
			/* crash wall */
			state.mode = MODE_SPINOUT1;
			state.anim_timer = 0;
			state.bShaking = 1;
		}
	}
	else
	{
		state.bShaking = 0;
	}

	update_computer_cars();

	for( i=0; i<NUM_COMPUTER_CARS; i++ )
	{
		monaco_gfx.computer_car[i].x = state.computer_car[i].xpos - 32;
		monaco_gfx.computer_car[i].y = state.computer_car[i].ypos;
		monaco_gfx.computer_car[i].tile = (state.ticks&2)>>1;
		monaco_gfx.computer_car[i].color = i; /* hack */
	}
	monaco_gfx.rescue_x = state.rescue_xpos;
	monaco_gfx.rescue_y = BRIDGE_YPOS;
	monaco_gfx.rescue_tile = (state.ticks&2)>>1;
	monaco_gfx.pool_x = state.pool_xpos;
	monaco_gfx.pool_y = (SCREEN_HEIGHT-32)/2;
	monaco_gfx.top_inset = state.track_top_inset;
	monaco_gfx.bottom_inset = state.track_bottom_inset;
	monaco_gfx.bSignalVisible = (state.page_next == PAGE_BRIDGE || state.page_next == PAGE_BRIDGE );
	monaco_gfx.led_score = state.score/(256);
	monaco_gfx.led_time = state.time/(256);
	monaco_gfx.right_page = state.page_current;
	monaco_gfx.left_page = state.page_next;
	monaco_gfx.scroll = state.scroll;

	monaco_gfx.left_text = -1;
	monaco_gfx.right_text = -1;

	/* we stretch the text so that it is proportioned correctly
	** compared to the rest of the in-game graphics
	*/

	switch( state.mode ){
	case MODE_ATTRACT:
		monaco_gfx.left_text = TEXT_GAMEOVER;
		monaco_gfx.right_text = TEXT_DEPOSITCOIN;
		break;

	case MODE_START:
		monaco_gfx.right_text = TEXT_START;
		break;

	default:
		break;
	}

	monaco_gfx.player_x = get_player_xpos();
	monaco_gfx.player_y = state.player_ypos;
	monaco_gfx.player_tile = -1;
	monaco_gfx.player_splash = -1;
	switch( state.mode )
	{
	case MODE_ATTRACT:
		break;

	case MODE_SLIP1:
		monaco_gfx.player_splash = 0;
		monaco_gfx.player_tile = (state.ticks&2)>>1;
		break;

	case MODE_SLIP2:
		monaco_gfx.player_splash = 1;
		monaco_gfx.player_tile = (state.ticks&2)>>1;
		break;

	case MODE_SPINOUT1:
		monaco_gfx.player_tile = 2;
		break;

	case MODE_SPINOUT2:
		monaco_gfx.player_tile = 3;
		break;

	case MODE_SPINOUT3:
		monaco_gfx.player_tile = 4;
		break;
	case MODE_SPINOUT4:
		monaco_gfx.player_tile = 5;
		break;

	case MODE_CRASH1:
	case MODE_CRASH2:
	case MODE_CRASH3:
	case MODE_CRASH4:
		monaco_gfx.player_tile = state.mode-MODE_CRASH1 + 6;
		break;

	default:
		if( state.bShaking )
		{
			monaco_gfx.player_tile = 10+((state.ticks&4)>>2);
		}
		else
		{
			monaco_gfx.player_tile = (state.ticks&2)>>1;
		}
		break;
	}

	return;
}

/*********************************************/

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )/* fake */
	AM_RANGE(0x0000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )/* fake */
	AM_RANGE(0x0000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

INPUT_PORTS_START( monaco )
	PORT_START /* fake */
	PORT_BIT( DOWN,   IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( UP,     IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( RIGHT,  IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( LEFT,   IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( COIN,   IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( ACCEL1, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( ACCEL2, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( ACCEL3, IP_ACTIVE_HIGH, IPT_BUTTON3 )
INPUT_PORTS_END

/***************************************************************************/

static gfx_layout tree_layout =
{
	32,32,
	1, /* number of characters */
	2, /* number of bitplanes */
	{ 0,4 },
	{
		0x003,0x002,0x001,0x000,0x103,0x102,0x101,0x100,
		0x203,0x202,0x201,0x200,0x303,0x302,0x301,0x300,
		0x403,0x402,0x401,0x400,0x503,0x502,0x501,0x500,
		0x603,0x602,0x601,0x600,0x703,0x702,0x701,0x700
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8
	},
	0
};

static gfx_layout pool_layout =
{
	32,32,
	1, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x003,0x002,0x001,0x000,0x103,0x102,0x101,0x100,
		0x203,0x202,0x201,0x200,0x303,0x302,0x301,0x300,
		0x403,0x402,0x401,0x400,0x503,0x502,0x501,0x500,
		0x603,0x602,0x601,0x600,0x703,0x702,0x701,0x700
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8
	},
	4
};

static gfx_layout player_layout =
{
	32,32,
	2, /* number of characters */
	2, /* number of bitplanes */
	{ 0,4 },
	{
		0x003,0x002,0x001,0x000,0x103,0x102,0x101,0x100,
		0x203,0x202,0x201,0x200,0x303,0x302,0x301,0x300,
		0x403,0x402,0x401,0x400,0x503,0x502,0x501,0x500,
		0x603,0x602,0x601,0x600,0x703,0x702,0x701,0x700
	},
	{
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8,
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x800
};

static gfx_layout text_layout =
{
	16,64,
	4, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x007,0x006,0x005,0x004,0x003,0x002,0x001,0x000,
		0x207,0x206,0x205,0x204,0x203,0x202,0x201,0x200,
	},
	{
		0x100,0x108,0x110,0x118,0x120,0x128,0x130,0x138,
		0x140,0x148,0x150,0x158,0x160,0x168,0x170,0x178,
		0x180,0x188,0x190,0x198,0x1a0,0x1a8,0x1b0,0x1b8,
		0x1c0,0x1c8,0x1d0,0x1d8,0x1e0,0x1e8,0x1f0,0x1f8,
		0x000,0x008,0x010,0x018,0x020,0x028,0x030,0x038,
		0x040,0x048,0x050,0x058,0x060,0x068,0x070,0x078,
		0x080,0x088,0x090,0x098,0x0a0,0x0a8,0x0b0,0x0b8,
		0x0c0,0x0c8,0x0d0,0x0d8,0x0e0,0x0e8,0x0f0,0x0f8,
	},
	0x400
};

static gfx_layout house_layout =
{
	32,32,
	1, /* number of characters */
	4, /* number of bitplanes */
	{ 0,2,4,6 },
	{
		0x001,0x000,0x101,0x100,0x201,0x200,0x301,0x300,
		0x401,0x400,0x501,0x500,0x601,0x600,0x701,0x700,
		0x801,0x800,0x901,0x900,0xa01,0xa00,0xb01,0xb00,
		0xc01,0xc00,0xd01,0xd00,0xe01,0xe00,0xf01,0xf00
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8
	},
	4
};

static gfx_layout signal_layout =
{
	32,32,
	1, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x001,0x000,0x101,0x100,0x201,0x200,0x301,0x300,
		0x401,0x400,0x501,0x500,0x601,0x600,0x701,0x700,
		0x801,0x800,0x901,0x900,0xa01,0xa00,0xb01,0xb00,
		0xc01,0xc00,0xd01,0xd00,0xe01,0xe00,0xf01,0xf00
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8,
	},
	0
};

static gfx_layout dummy_layout =
{
	32,16,
	2, /* number of characters */
	2, /* number of bitplanes */
	{ 4,6 },
	{
		0x001,0x000,0x101,0x100,0x201,0x200,0x301,0x300,
		0x401,0x400,0x501,0x500,0x601,0x600,0x701,0x700,
		0x801,0x800,0x901,0x900,0xa01,0xa00,0xb01,0xb00,
		0xc01,0xc00,0xd01,0xd00,0xe01,0xe00,0xf01,0xf00
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x80
};

static gfx_layout computer_layout =
{
	32,16,
	2, /* number of characters */
	4, /* number of bitplanes */
	{ 0,2,4,6 },
	{
		0x001,0x000,0x081,0x080,0x101,0x100,0x181,0x180,
		0x201,0x200,0x281,0x280,0x301,0x300,0x381,0x380,
		0x401,0x400,0x481,0x480,0x501,0x500,0x581,0x580,
		0x601,0x600,0x681,0x680,0x701,0x700,0x781,0x780
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x800
};

static gfx_layout rescue_layout =
{
	64,16,
	2, /* number of characters */
	4, /* number of bitplanes */
	{ 0,2,4,6 },
	{
		0x001,0x001,0x000,0x000,0x081,0x081,0x080,0x080,
		0x101,0x101,0x100,0x100,0x181,0x181,0x180,0x180,
		0x201,0x201,0x200,0x200,0x281,0x281,0x280,0x280,
		0x301,0x301,0x300,0x300,0x381,0x381,0x380,0x380,
		0x401,0x401,0x400,0x400,0x481,0x481,0x480,0x480,
		0x501,0x501,0x500,0x500,0x581,0x581,0x580,0x580,
		0x601,0x601,0x600,0x600,0x681,0x681,0x680,0x680,
		0x701,0x701,0x700,0x700,0x781,0x781,0x780,0x780
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x800
};

static gfx_layout explode_layout =
{
	32,32,
	4, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x007,0x006,0x005,0x004,0x003,0x002,0x001,0x000,
		0x107,0x106,0x105,0x104,0x103,0x102,0x101,0x100,
		0x207,0x206,0x205,0x204,0x203,0x202,0x201,0x200,
		0x307,0x306,0x305,0x304,0x303,0x302,0x301,0x300,
	},
	{
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8,
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x400
};

static gfx_layout bridge_layout =
{
	32,16,
	2, /* number of characters */
	4, /* number of bitplanes */
	{ 0,1,2,3 },
	{
		0x004,0x000,0x104,0x100,0x204,0x200,0x304,0x300,
		0x404,0x400,0x504,0x500,0x604,0x600,0x704,0x700,
		0x804,0x800,0x904,0x900,0xa04,0xa00,0xb04,0xb00,
		0xc04,0xc00,0xd04,0xd00,0xe04,0xe00,0xf04,0xf00
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x80
};

static gfx_layout unknown_layout =
{ /* police car? */
	32,32,
	2, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x307,0x306,0x305,0x304,0x303,0x302,0x301,0x300,
		0x207,0x206,0x205,0x204,0x203,0x202,0x201,0x200,
		0x107,0x106,0x105,0x104,0x103,0x102,0x101,0x100,
		0x007,0x006,0x005,0x004,0x003,0x002,0x001,0x000,
	},
	{
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8,
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78
	},
	0x400
};


static gfx_layout tunnel_layout =
{
	32,32,
	2, /* number of characters */
	1, /* number of bitplanes */
	{ 0 },
	{
		0x003,0x002,0x001,0x000,0x103,0x102,0x101,0x100,
		0x203,0x202,0x201,0x200,0x303,0x302,0x301,0x300,
		0x403,0x402,0x401,0x400,0x503,0x502,0x501,0x500,
		0x603,0x602,0x601,0x600,0x703,0x702,0x701,0x700
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8
	},
	4
};

static gfx_layout belt_layout = {
	32,16,
	2, /* number of characters */
	1, /* number of bitplanes */
	{ 4 },
	{
		0x003,0x002,0x001,0x000,0x103,0x102,0x101,0x100,
		0x203,0x202,0x201,0x200,0x303,0x302,0x301,0x300,
		0x403,0x402,0x401,0x400,0x503,0x502,0x501,0x500,
		0x603,0x602,0x601,0x600,0x703,0x702,0x701,0x700
	},
	{
		0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,
		0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78,
		0x80,0x88,0x90,0x98,0xa0,0xa8,0xb0,0xb8,
		0xc0,0xc8,0xd0,0xd8,0xe0,0xe8,0xf0,0xf8
	},
	0x80
};

static gfx_decode gfxdecodeinfo[] = {
	{ REGION_GFX1, 0x0100, &unknown_layout,	0x00, 1 },			/* PR125: unused; police car? */
	{ REGION_GFX1, 0x0200, &explode_layout,	RED_CLUT, 2 },		/* PR126: explosion */
	{ REGION_GFX1, 0x0400, &player_layout,	PLAYER_CLUT, 1 },	/* PR127: player's car */
	{ REGION_GFX1, 0x0600, &player_layout,	PLAYER_CLUT, 1 },	/* PR128: swerving (rough road) */
	{ REGION_GFX1, 0x0800, &player_layout,	PLAYER_CLUT, 1 },	/* PR129: spinout1 */
	{ REGION_GFX1, 0x0a00, &player_layout,	PLAYER_CLUT, 1 },	/* PR130: spinout2 */
	{ REGION_GFX1, 0x0c00, &explode_layout,	CYAN_CLUT, 1 },		/* PR131: spray */
	{ REGION_GFX1, 0x0e00, &computer_layout,	0x00, 5 },		/* PR132: computer car A,B */
	{ REGION_GFX1, 0x1000, &text_layout,	CYAN_CLUT, 2 },		/* PR133: text */
	{ REGION_GFX1, 0x1200, &tree_layout,	TREE_CLUT, 1 },		/* PR134: tree */
	{ REGION_GFX1, 0x1300, &tree_layout,	TREE_CLUT, 1 },		/* PR134: grass */
	{ REGION_GFX1, 0x1400, &tree_layout,	SHRUB_CLUT, 1 },	/* PR135: shrub */
	{ REGION_GFX1, 0x1600, &house_layout,	HOUSE_CLUT, 1 },	/* PR136: house */
	{ REGION_GFX1, 0x1800, &tunnel_layout,	RED_CLUT, 1 },		/* PR137: tunnel */
	{ REGION_GFX1, 0x1900, &pool_layout,	CYAN_CLUT, 1 },		/* PR137: pool (oil slick) */
	{ REGION_GFX1, 0x1900, &belt_layout,	GRASS_CLUT, 2 },	/* PR137: red/green belt */
	{ REGION_GFX1, 0x1a00, &rescue_layout,	RESCUE_CLUT, 1 },	/* PR138: rescue car */
	{ REGION_GFX1, 0x1c00, &signal_layout,	YELLOW_CLUT, 1 },	/* PR139: bridge signal (yellow on black)*/
	{ REGION_GFX1, 0x1c00, &dummy_layout,	PLAYER_CLUT, 1 },	/* PR139: dummy car */
	{ REGION_GFX1, 0x1e00, &bridge_layout,	WATER_CLUT, 1 },	/* PR140: bridge-water */
	{ REGION_GFX1, 0x2000, &bridge_layout,	WATER_CLUT, 1 },	/* PR141: bridge-pillar */
	{ -1 }
};

static const char *monaco_sample_names[] =
{
	"*monaco",
	"6car1.wav","6car2.wav",
	"6crash.wav","6engine.wav",
	"6extend.wav","6fanfare.wav",
	"6siren.wav","6slip.wav",
	0
};

static struct Samplesinterface monaco_samples_interface =
{
	2,	/* number of channels */
	monaco_sample_names
};

static MACHINE_DRIVER_START( monaco )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 200) /* fake */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(monaco_interrupt,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)
	MDRV_SCREEN_VISIBLE_AREA(0, SCREEN_WIDTH-1, 0, SCREEN_HEIGHT-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(1024)
	MDRV_COLORTABLE_LENGTH(1024)

	MDRV_VIDEO_START(monaco)
	MDRV_VIDEO_UPDATE(monaco)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SAMPLES, 0)
	MDRV_SOUND_CONFIG(monaco_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/*****************************************************************/

ROM_START( monaco )
	ROM_REGION( 0x10000, REGION_CPU1, ROMREGION_ERASE00 )	/* fake */

	ROM_REGION( 0x3000, REGION_GFX1, 0 )
	ROM_LOAD( "pr125", 512*0,  512, CRC(7a66ed4c) SHA1(514e129c334a551b931c90b063b073a9b4bdffc3) ) /* light data */
	ROM_LOAD( "pr126", 512*1,  512, CRC(5d7a8f12) SHA1(b4f0d21b91a7cf7002f99c08788669c7c38be51d) ) /* explosion */
	ROM_LOAD( "pr127", 512*2,  512, CRC(8ffdc2f0) SHA1(05cc3330c067965b8b90b5d27119fe9f26580a13) ) /* car(2)main */
	ROM_LOAD( "pr128", 512*3,  512, CRC(dde29dea) SHA1(34c413edff991297471bd0bc193c4bd8ede4e468) ) /* car(2)rotated */
	ROM_LOAD( "pr129", 512*4,  512, CRC(7b18af26) SHA1(3d1ff2610813544c3b9b65182f081272a9537640) ) /* car(2)rotated */
	ROM_LOAD( "pr130", 512*5,  512, CRC(9ef1913b) SHA1(58830121781b8a13532eaf8ea13ec07f10522320) ) /* car(2) spinout */
	ROM_LOAD( "pr131", 512*6,  512, CRC(ff31eb01) SHA1(fd6bcd92c4bd919bb1a96ca97688d46cb310b39d) ) /* splash */
	ROM_LOAD( "pr132", 512*7,  512, CRC(6b8ad9bc) SHA1(be36e3b6b647d3a9565bc45903027c791dc889e5) ) /* car(2)(other) */
	ROM_LOAD( "pr133", 512*8,  512, CRC(d50641d9) SHA1(bf399e9830c88e4d8f8fb386305f54ef766946d9) ) /* text(4) */
	ROM_LOAD( "pr134", 512*9,  512, CRC(8ebd50bb) SHA1(98d51f503753d4d7191a09b509d26c1e049e981a) ) /* tree, grass */
	ROM_LOAD( "pr135", 512*10, 512, CRC(986eda32) SHA1(73fa539d4c83748952d9339985208520fec955f3) ) /* shrub */
	ROM_LOAD( "pr136", 512*11, 512, CRC(ecc5d1a2) SHA1(33bff7381785557a85e4c8bdd74679b59e0ed9d5) ) /* house */
	ROM_LOAD( "pr137", 512*12, 512, CRC(ddd9004e) SHA1(5229c34578e66d9c51a05439a516513946ba69ed) ) /* tunnel, oil slip */
	ROM_LOAD( "pr138", 512*13, 512, CRC(058e53cf) SHA1(7c3aaaca5a9e9ce3a3badd0dcc8360342673a397) ) /* firetruck */
	ROM_LOAD( "pr139", 512*14, 512, CRC(e8ba0794) SHA1(eadd7425134f26b1c126bbcd3d3dabf4b2e1fe70) ) /* car, bridge symbol */
	ROM_LOAD( "pr140", 512*15, 512, CRC(48e9971b) SHA1(c0c265cdc08727e3caaf49cdfe728a91c4c46ba2) ) /* bridge-water */
	ROM_LOAD( "pr141", 512*16, 512, CRC(99934236) SHA1(ec271f3e690d5c57ead9132b22b9b1b966e4d170) ) /* bridge-pillar */

	ROM_REGION( 32*3, REGION_PROMS, 0 )
	ROM_LOAD( "prm38",	0*32, 32, CRC(82dd0a0f) SHA1(3e7e475c3270853d70c1fe90a773172532b60cfb) )	/* acceleration related */
	ROM_LOAD( "prm39",	1*32, 32, CRC(6acfa0da) SHA1(1e56da4cdf71a095eac29878969b831babac222b) )	/* regulates opponent car speed */

//	ROM_LOAD( "prm-40",	2*32, 32, CRC(8030dac8) )
/*	PR40 is in the Fanfare sound circuit and seems to access the particular
 *	notes for the fanfare sound (so PR40 may contain timing and pointer info
 *	on the melody).  The switch (SW1) I mentioned before that helped in tuning
 *	the fanfare sound with the 6 pots seems to help in making the tuning of each
 *	pot for output of one of three audio frequencies (262, 330, 392 Hz),
 *	instead of having to tune to 6 different frequencies (a production/test
 *	equipment issue).
 *	In any case, if we get a good sample of this fanfare sound, we will not
 *	need to bother with this circuit or PR40.  As far a I have seen, the
 *	fanfare sound only comes up at the end of the game if you have a top five
 *	score and possibly when you plug in the game.
 */
ROM_END

static DRIVER_INIT( monaco )
{
	int i;
	const double dy_table[5] =
	{
		0.75,
		1.25,
		1.75,
		2.00,
		2.50
	};

	state.computer_speed = 52*16/256.0;
	state.track_bottom_inset = 32;
	state.track_top_inset = 32;
	state.track_bottom_delta = 1;
	state.track_top_delta = 1;

	/* computer car */
	for( i=0; i<NUM_COMPUTER_CARS; i++ )
	{
		state.computer_car[i].xpos = i*32*3 + ((i>1)?192:0);
		state.computer_car[i].ypos = (SCREEN_HEIGHT-16)/2;
		state.computer_car[i].dy = dy_table[i];
	}
}

/*          rom     parent  machine inp     init */
GAME( 1979, monaco, 0,      monaco, monaco, monaco,   ROT90, "Sega", "Monaco GP", 0 )
