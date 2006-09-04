/* The video synchronization layout, derived from the schematics */
#define PONG_MAX_H	(256+128+64+4+2)
#define PONG_MAX_V	(256+4+1)
#define PONG_HBLANK (64+16)
#define PONG_VBLANK 16
#define PONG_HSYNC0 32
#define PONG_HSYNC1 64
#define PONG_VSYNC0 4
#define PONG_VSYNC1 8
#define PONG_FPS	60

VIDEO_START( pong );
VIDEO_UPDATE( pong );
INTERRUPT_GEN( pong_vh_scanline );
VIDEO_EOF( pong );


