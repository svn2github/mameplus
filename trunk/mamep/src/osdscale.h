//============================================================
//
//	scale.c - scaling effects framework code
//
//============================================================

#ifndef __SCALE_EFFECTS__
#define __SCALE_EFFECTS__


//============================================================
//	GLOBAL VARIABLES
//============================================================

extern struct { int effect; int xsize; int ysize; const char *name; } scale_effect;



//============================================================
//	PROTOTYPES
//============================================================

int scale_init(void);
int scale_exit(void);
int scale_check(int depth);
int scale_perform_scale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int update);
int scale_decode(const char *arg);

#endif
