//============================================================
//
//	scale.c - scaling effects framework code
//
//============================================================


// MAME headers
#include "driver.h"
#ifdef USE_SCALE_EFFECTS
#include "osdscale.h"
#endif /* USE_SCALE_EFFECTS */

// scale2x
#include "scale/scale2x.h"
#include "scale/scale3x.h"
#include "scale/hlq.h"
#include "scale/2xpm.h"

// defines
#define SCALE_EFFECT_NONE			0
#define SCALE_EFFECT_SCALE2X		1
#define SCALE_EFFECT_SCALE2X3		2
#define SCALE_EFFECT_SCALE2X4		3
#define SCALE_EFFECT_SCALE3X		4
#define SCALE_EFFECT_SUPERSCALE		5
#define SCALE_EFFECT_SUPERSCALE75	6
#define SCALE_EFFECT_2XSAI			7
#define SCALE_EFFECT_SUPER2XSAI		8
#define SCALE_EFFECT_SUPEREAGLE		9
#define SCALE_EFFECT_EAGLE			10
#define SCALE_EFFECT_2XPM			11
#define SCALE_EFFECT_HQ2X			12
#define SCALE_EFFECT_HQ2X3			13
#define SCALE_EFFECT_HQ2X4			14
#define SCALE_EFFECT_LQ2X			15
#define SCALE_EFFECT_LQ2X3			16
#define SCALE_EFFECT_LQ2X4			17
#define SCALE_EFFECT_HQ3X			18
#define SCALE_EFFECT_LQ3X			19
#ifdef USE_4X_SCALE
#define SCALE_EFFECT_HQ4X			20
#define SCALE_EFFECT_LQ4X			21
#endif /* USE_4X_SCALE */

#define MAX_SCALE_BANK				(MAX_SCREENS * 2)


//============================================================
//	GLOBAL VARIABLES
//============================================================

struct _scale_effect scale_effect;



//============================================================
//	IMPORTS
//============================================================



//============================================================
//	LOCAL VARIABLES
//============================================================

static int use_mmx;

static UINT8 *scale_buffer[MAX_SCALE_BANK];

static int previous_depth[MAX_SCALE_BANK];
static int previous_width[MAX_SCALE_BANK];
static int previous_height[MAX_SCALE_BANK];

static const char *str_name[] =
{
	"none",
	"scale2x",
	"scale2x3",
	"scale2x4",
	"scale3x",
	"superscale",
	"superscale75",
	"2xsai",
	"super2xsai",
	"supereagle",
	"eagle",
	"2xpm",
	"hq2x",
	"hq2x3",
	"hq2x4",
	"lq2x",
	"lq2x3",
	"lq2x4",
	"hq3x",
	"lq3x",
#ifdef USE_4X_SCALE
	"hq4x",
	"lq4x",
#endif /* USE_4X_SCALE */
	NULL
};

static const char *str_desc[] =
{
	"None",
	"Scale2x v2.2 by AdvMAME",
	"Scale2x3 v2.2 by AdvMAME",
	"Scale2x4 v2.2 by AdvMAME",
	"Scale3x v2.2 by AdvMAME",
	"SuperScale by ElSemi",
	"SuperScale 75% SL by ElSemi",
	"2xSaI v0.59 by Kreed",
	"Super 2xSaI v0.59 by Kreed",
	"Super Eagle v0.59 by Kreed",
	"Eagle 0.41a by Dirk Stevens",
	"2xPM v0.2 by Pablo Medina",
	"HQ2x by Maxim Stepin",
	"HQ2x3 by AdvMAME",
	"HQ2x4 by AdvMAME",
	"LQ2x by AdvMAME",
	"LQ2x3 by AdvMAME",
	"LQ2x4 by AdvMAME",
	"HQ3x by Maxim Stepin",
	"LQ3x by AdvMAME",
#ifdef USE_4X_SCALE
	"HQ4x by Maxim Stepin",
	"LQ4x by AdvMAME",
#endif /* USE_4X_SCALE */
	NULL
};



//============================================================
//	prototypes
//============================================================

static void scale_allocate_local_buffer(int width, int height, int bank);

// functions from scale2x
static int scale_perform_scale2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank);
static void (*scale_scale2x_line_16)(UINT16 *dst0, UINT16 *dst1, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x_line_32)(UINT32 *dst0, UINT32 *dst1, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank);
static void (*scale_scale2x3_line_16)(UINT16 *dst0, UINT16 *dst1, UINT16 *dst2, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x3_line_32)(UINT32 *dst0, UINT32 *dst1, UINT32 *dst2, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank);
static void (*scale_scale2x4_line_16)(UINT16 *dst0, UINT16 *dst1, UINT16 *dst2, UINT16 *dst3, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x4_line_32)(UINT32 *dst0, UINT32 *dst1, UINT32 *dst2, UINT32 *dst3, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);

static int scale_perform_scale3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from superscale.asm
void superscale_line(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);
void superscale_line_75(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);

static void (*superscale_line_func)(UINT16 *src0, UINT16 *src1, UINT16 *src2, UINT16 *dst, UINT32 width, UINT64 *mask);

static int scale_perform_superscale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from eagle
void _eagle_mmx16(unsigned long *lb, unsigned long *lb2, short width,	unsigned long *screen_address1, unsigned long *screen_address2);

static int scale_perform_eagle(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from 2xsaimmx.asm
void  _2xSaISuperEagleLine(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  _2xSaILine(UINT8* srcPtr, UINT8* deltaPtr, UINT32 srcPitch, UINT32 width, UINT8* dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  _2xSaISuper2xSaILine(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
void  Init_2xSaIMMX(UINT32 BitFormat);

static void (*scale_2xsai_line)(UINT8 *srcPtr, UINT8 *deltaPtr, UINT32 srcPitch, UINT32 width, UINT8 *dstPtr, UINT32 dstPitch, UINT16 dstBlah);
static int scale_perform_2xsai(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank);

// functions from 2xpm
static int scale_perform_2xpm(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from hq2x/hq3x/hq4x/lq2x/lq3x/lq4x
static int scale_perform_hlq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x_15_def)(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x_32_def)(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#ifdef ASM_HQ
static int scale_perform_hq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
void hq2x_16(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);
void hq2x_32(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);
#endif /* ASM_HQ */

static int scale_perform_hlq2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x3_15_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x3_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

static int scale_perform_hlq2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq2x4_15_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq2x4_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

static int scale_perform_hlq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq3x_15_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq3x_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#ifdef ASM_HQ
static int scale_perform_hq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
void hq3x_16(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);
void hq3x_32(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);
#endif /* ASM_HQ */

#ifdef USE_4X_SCALE
static int scale_perform_hlq4x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static void (*scale_hlq4x_15_def)(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
static void (*scale_hlq4x_32_def)(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#endif /* USE_4X_SCALE */

//============================================================
//	scale_decode
//============================================================

int scale_decode(const char *arg)
{
	int i;

	for (i = 0; str_name[i]; i++)
		if (!strcmp(arg, str_name[i]))
		{
			scale_effect.effect = i;
			return 0;
		}

	return 0;
}

//============================================================
//	scale_decode
//============================================================

const char *scale_name(int effect)
{
	return str_name[effect];
}

//============================================================
//	scale_decode
//============================================================

const char *scale_desc(int effect)
{
	return str_desc[effect];
}

//============================================================
//	scale_exit
//============================================================

int scale_exit(void)
{
	int i;

	for (i = 0; i < MAX_SCALE_BANK; i++)
	{
		previous_depth[i] = previous_width[i] = previous_height[i] = 0;

		if (scale_buffer[i])
		{
			free(scale_buffer[i]);
			scale_buffer[i] = NULL;
		}
	}

	return 0;
}

//============================================================
//	x86_get_features
//============================================================

static UINT32 x86_get_features(void)
{
	UINT32 features = 0;
#ifdef _MSC_VER
	__asm
	{
		mov eax, 1
		xor ebx, ebx
		xor ecx, ecx
		xor edx, edx
		__asm _emit 0Fh __asm _emit 0A2h	// cpuid
		mov features, edx
	}
#else /* !_MSC_VER */
	__asm__
	(
		"pushl %%ebx         ; "
		"movl $1,%%eax       ; "
		"xorl %%ebx,%%ebx    ; "
		"xorl %%ecx,%%ecx    ; "
		"xorl %%edx,%%edx    ; "
		"cpuid               ; "
		"movl %%edx,%0       ; "
		"popl %%ebx          ; "
	: "=&a" (features)		/* result has to go in eax */
	: 				/* no inputs */
	: "%ecx", "%edx"	/* clobbers ebx, ecx and edx */
	);
#endif /* MSC_VER */
	return features;
}


//============================================================
//	scale_init
//============================================================

int scale_init(void)
{
	static char name[64];

	UINT32 features = x86_get_features();
	use_mmx = (features & (1 << 23));

	scale_exit();

	scale_effect.xsize = scale_effect.ysize = 1;
	sprintf(name, "none");
	scale_effect.name = name;

	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
		{
			break;
		}
		case SCALE_EFFECT_SCALE2X:
		{
			sprintf(name, "Scale2x (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		case SCALE_EFFECT_SCALE2X3:
		{
			sprintf(name, "Scale2x3 (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			break;
		}
		case SCALE_EFFECT_SCALE2X4:
		{
			sprintf(name, "Scale2x4 (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			break;
		}
		case SCALE_EFFECT_SCALE3X:
		{
			sprintf(name, "Scale3x (non-mmx version)");
			scale_effect.xsize = scale_effect.ysize = 3;
			break;
		}
		case SCALE_EFFECT_EAGLE:
		{
			sprintf(name, "Eagle (mmx optimised)");

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
		{
			if (scale_effect.effect == SCALE_EFFECT_SUPERSCALE)
			{
				sprintf(name, "SuperScale (mmx optimised)");
				superscale_line_func = superscale_line;
			}
			else
			{
				sprintf(name, "SuperScale75%% (mmx optimised)");
				superscale_line_func = superscale_line_75;
			}

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
		{
			if (scale_effect.effect == SCALE_EFFECT_2XSAI)
			{
				sprintf(name, "2xSaI (mmx optimised)");
				scale_2xsai_line = _2xSaILine;
			}
			else if (scale_effect.effect == SCALE_EFFECT_SUPER2XSAI)
			{
				sprintf(name, "Super 2xSaI (mmx optimised)");
				scale_2xsai_line = _2xSaISuper2xSaILine;
			}
			else
			{
				sprintf(name, "Super Eagle (mmx optimised)");
				scale_2xsai_line = _2xSaISuperEagleLine;
			}

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		
		case SCALE_EFFECT_2XPM:
		{
			sprintf(name, "2xPM");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		
		case SCALE_EFFECT_HQ2X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "HQ2x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "HQ2x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 2;
#ifdef ASM_HQ
			scale_hlq2x_15_def = NULL;
#else /* ASM_HQ */
			scale_hlq2x_15_def = hq2x_15_def;
#endif /* ASM_HQ */
			scale_hlq2x_32_def = hq2x_32_def;
			break;
		}
		case SCALE_EFFECT_HQ2X3:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "HQ2x3 (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "HQ2x3 (non-mmx version)");

			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			scale_hlq2x3_15_def = hq2x3_15_def;
			scale_hlq2x3_32_def = hq2x3_32_def;
			break;
		}
		case SCALE_EFFECT_HQ2X4:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "HQ2x4 (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "HQ2x4 (non-mmx version)");

			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			scale_hlq2x4_15_def = hq2x4_15_def;
			scale_hlq2x4_32_def = hq2x4_32_def;
			break;
		}
		case SCALE_EFFECT_LQ2X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "LQ2x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "LQ2x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 2;
			scale_hlq2x_15_def = lq2x_15_def;
			scale_hlq2x_32_def = lq2x_32_def;
			break;
		}
		case SCALE_EFFECT_LQ2X3:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "LQ2x3 (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "LQ2x3 (non-mmx version)");

			scale_effect.xsize = 2;
			scale_effect.ysize = 3;
			scale_hlq2x3_15_def = lq2x3_15_def;
			scale_hlq2x3_32_def = lq2x3_32_def;
			break;
		}
		case SCALE_EFFECT_LQ2X4:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "LQ2x4 (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "LQ2x4 (non-mmx version)");

			scale_effect.xsize = 2;
			scale_effect.ysize = 4;
			scale_hlq2x4_15_def = lq2x4_15_def;
			scale_hlq2x4_32_def = lq2x4_32_def;
			break;
		}
		case SCALE_EFFECT_HQ3X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "HQ3x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "HQ3x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 3;
#ifdef ASM_HQ
			scale_hlq3x_15_def = NULL;
#else /* ASM_HQ */
			scale_hlq3x_15_def = hq3x_15_def;
#endif /* ASM_HQ */
			scale_hlq3x_32_def = hq3x_32_def;
			break;
		}
		case SCALE_EFFECT_LQ3X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "LQ3x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "LQ3x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 3;
			scale_hlq3x_15_def = lq3x_15_def;
			scale_hlq3x_32_def = lq3x_32_def;
			break;
		}
#ifdef USE_4X_SCALE
		case SCALE_EFFECT_HQ4X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "HQ4x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "HQ4x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 4;
			scale_hlq4x_15_def = hq4x_15_def;
			scale_hlq4x_32_def = hq4x_32_def;
			break;
		}
		case SCALE_EFFECT_LQ4X:
		{
#ifdef USE_MMX_INTERP_SCALE
			if (use_mmx)
				sprintf(name, "LQ4x (mmx version)");
			else
#endif /* USE_MMX_INTERP_SCALE */
				sprintf(name, "LQ4x (non-mmx version)");

			scale_effect.xsize = scale_effect.ysize = 4;
			scale_hlq4x_15_def = lq4x_15_def;
			scale_hlq4x_32_def = lq4x_32_def;
			break;
		}
#endif /* USE_4X_SCALE */
		default:
		{
			return 1;
		}
	}

	return 0;
}



//============================================================
//	scale_check
//============================================================

int scale_check(int depth)
{
	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
			return 0;

		case SCALE_EFFECT_SCALE2X:
		case SCALE_EFFECT_SCALE2X3:
		case SCALE_EFFECT_SCALE2X4:
		case SCALE_EFFECT_SCALE3X:
			if (depth == 15 || depth == 16 || depth == 32)
				return 0;
			else
				return 1;

		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
		case SCALE_EFFECT_EAGLE:
		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
			if (use_mmx && (depth == 15 || depth == 16))
				return 0;
			else
				return 1;

		case SCALE_EFFECT_2XPM:
			if (depth == 15 || depth == 16)
				return 0;
			else
				return 1;

		case SCALE_EFFECT_HQ2X:
		case SCALE_EFFECT_HQ2X3:
		case SCALE_EFFECT_HQ2X4:
		case SCALE_EFFECT_LQ2X:
		case SCALE_EFFECT_LQ2X3:
		case SCALE_EFFECT_LQ2X4:
		case SCALE_EFFECT_HQ3X:
		case SCALE_EFFECT_LQ3X:
#ifdef USE_4X_SCALE
		case SCALE_EFFECT_HQ4X:
		case SCALE_EFFECT_LQ4X:
#endif /* USE_4X_SCALE */
#ifdef USE_MMX_INTERP_SCALE
			if (!use_mmx)
				return 1;
#endif /* USE_MMX_INTERP_SCALE */

			if (depth == 15 || depth == 32)
				return 0;
			else
				return 1;

		default:
			return 1;
	}
}



//============================================================
//	scale_perform_scale
//============================================================

int scale_perform_scale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int update, int bank)
{
	switch (scale_effect.effect)
	{
		case SCALE_EFFECT_NONE:
			return 0;
		case SCALE_EFFECT_SCALE2X:
			return scale_perform_scale2x(src, dst, src_pitch, dst_pitch, width, height, depth, bank);
		case SCALE_EFFECT_SCALE2X3:
			return scale_perform_scale2x3(src, dst, src_pitch, dst_pitch, width, height, depth, bank);
		case SCALE_EFFECT_SCALE2X4:
			return scale_perform_scale2x4(src, dst, src_pitch, dst_pitch, width, height, depth, bank);
		case SCALE_EFFECT_SCALE3X:
			return scale_perform_scale3x(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
			return scale_perform_superscale(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_EAGLE:
			return scale_perform_eagle(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
			if (update)
				scale_allocate_local_buffer(src_pitch, height, bank);
			return scale_perform_2xsai(src, dst, src_pitch, dst_pitch, width, height, depth, bank);
		case SCALE_EFFECT_2XPM:
			return scale_perform_2xpm(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X:
#ifdef ASM_HQ
			if (depth == 15)
				return scale_perform_hq2x(src, dst, src_pitch, dst_pitch, width, height, depth);
#endif /* ASM_HQ */
		case SCALE_EFFECT_LQ2X:
			return scale_perform_hlq2x(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X3:
		case SCALE_EFFECT_LQ2X3:
			return scale_perform_hlq2x3(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ2X4:
		case SCALE_EFFECT_LQ2X4:
			return scale_perform_hlq2x4(src, dst, src_pitch, dst_pitch, width, height, depth);
		case SCALE_EFFECT_HQ3X:
#ifdef ASM_HQ
			if (depth == 15)
				return scale_perform_hq3x(src, dst, src_pitch, dst_pitch, width, height, depth);
#endif /* ASM_HQ */
		case SCALE_EFFECT_LQ3X:
			return scale_perform_hlq3x(src, dst, src_pitch, dst_pitch, width, height, depth);
#ifdef USE_4X_SCALE
		case SCALE_EFFECT_HQ4X:
		case SCALE_EFFECT_LQ4X:
			return scale_perform_hlq4x(src, dst, src_pitch, dst_pitch, width, height, depth);
#endif /* USE_4X_SCALE */
		default:
			return 1;
	}
}


//============================================================
//	scale_emms
//============================================================

static inline void scale_emms(void)
{
	if (use_mmx)
	{
#ifdef __GNUC__
		__asm__ __volatile__ (
			"emms\n"
		);
#else
		__asm {
			emms;
		}
#endif
	}
}

//============================================================
//	scale_allocate_local_buffer
//============================================================

static void scale_allocate_local_buffer(int width, int height, int bank)
{
	if (scale_buffer[bank])
		free(scale_buffer[bank]);

	scale_buffer[bank] = malloc(width * height);

	previous_width[bank] = width;
	previous_height[bank] = height;
}

//============================================================
//	scale_perform_scale2x
//============================================================

static int scale_perform_scale2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 15 && depth != 16 && depth != 32)
		return 1;

	if (previous_depth[bank] != depth)
	{
		if (use_mmx)
		{
			scale_scale2x_line_16 = scale2x_16_mmx;
			scale_scale2x_line_32 = scale2x_32_mmx;
		}
		else
		{
			scale_scale2x_line_16 = scale2x_16_def;
			scale_scale2x_line_32 = scale2x_32_def;
		}

		previous_depth[bank] = depth;
	}

	if (depth == 15 || depth == 16)
//		scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 2 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_scale2x_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}



//============================================================
//	scale_perform_scale2x3
//============================================================

static int scale_perform_scale2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 15 && depth != 16 && depth != 32)
		return 1;

	if (previous_depth[bank] != depth)
	{
		if (use_mmx)
		{
			scale_scale2x3_line_16 = scale2x3_16_mmx;
			scale_scale2x3_line_32 = scale2x3_32_mmx;
		}
		else
		{
			scale_scale2x3_line_16 = scale2x3_16_def;
			scale_scale2x3_line_32 = scale2x3_32_def;
		}

		previous_depth[bank] = depth;
	}

	if (depth == 15 || depth == 16)
//		scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x3_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_scale2x3_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x3_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}


//============================================================
//	scale_perform_scale2x4
//============================================================

static int scale_perform_scale2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 15 && depth != 16 && depth != 32)
		return 1;

	if (previous_depth[bank] != depth)
	{
		if (use_mmx)
		{
			scale_scale2x4_line_16 = scale2x4_16_mmx;
			scale_scale2x4_line_32 = scale2x4_32_mmx;
		}
		else
		{
			scale_scale2x4_line_16 = scale2x4_16_def;
			scale_scale2x4_line_32 = scale2x4_32_def;
		}

		previous_depth[bank] = depth;
	}

	if (depth == 15 || depth == 16)
//		scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		scale2x4_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
//		scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		scale2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale_scale2x4_line_16((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_scale2x4_line_32((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	scale_emms();

	return 0;
}


//============================================================
//	scale_perform_scale3x
//============================================================

static int scale_perform_scale3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	if (depth != 15 && depth != 16 && depth != 32)
		return 1;

	if (depth == 15 || depth == 16)
		scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	if (depth == 15 || depth == 16)
	{
		for (y = 3; y < height; y++)
		{
			dst	+= 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;
			scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;
	if (depth == 15 || depth == 16)
		scale3x_16_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

	return 0;
}



//============================================================
//	scale_perform_superscale
//============================================================

static int scale_perform_superscale(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	UINT32 srcNextline = src_pitch >> 1;
	UINT16 *dst0=(UINT16 *)dst;
	UINT16 *dst1=(UINT16 *)(dst+dst_pitch);
	UINT16 *src0=(UINT16 *)(src-src_pitch);  //don't worry, there is extra space :)
	UINT16 *src1=(UINT16 *)src;
	UINT16 *src2=(UINT16 *)(src+src_pitch);
	UINT64 mask = 0x7BEF7BEF7BEF7BEFLL;
	int i;

	if (depth == 15)
		mask = 0x3DEF3DEF3DEF3DEFLL;

	for (i = 0; i < height; i++)
	{
		superscale_line(src0, src1, src2, dst0, width, &mask);
		superscale_line_func(src2, src1, src0, dst1, width, &mask);

		src0 = src1;
		src1 = src2;
		src2 += srcNextline;

		dst0 += dst_pitch;
		dst1 += dst_pitch;
	}
	scale_emms();
	
	return 0;
}



//============================================================
//	scale_perform_eagle
//============================================================

static int scale_perform_eagle(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;

	if ((depth != 15 && depth != 16) || !use_mmx)
		return 1;

	width *= 2;

	_eagle_mmx16((unsigned long *)src, (unsigned long *)src, width, (unsigned long *)dst, (unsigned long *)dst);
	dst += dst_pitch - 2;
	for (y = 0; y < height; y++, src += src_pitch, dst += 2 * dst_pitch)
		_eagle_mmx16((unsigned long *)src, (unsigned long *)(src + src_pitch), width, (unsigned long *)dst, (unsigned long *)(dst + dst_pitch));

	scale_emms();

	return 0;
}



//============================================================
//	scale_perform_2xpm
//============================================================

static int scale_perform_2xpm(UINT8 *src, UINT8 *dest, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	
	if (depth != 15 && depth != 16)
		return 1;

	if (depth == 15)
		_2xpm_15(src, dest, (unsigned long)src_pitch, (unsigned long)dst_pitch, (unsigned long)width, (unsigned long)height, depth);
	else
		_2xpm_16(src, dest, (unsigned long)src_pitch, (unsigned long)dst_pitch, (unsigned long)width, (unsigned long)height, depth);

	return 0;
}



//============================================================
//	scale_perform_2xsai
//============================================================

static int scale_perform_2xsai(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank)
{
	int y;
	UINT8 *buf;

	if ((depth != 15 && depth != 16) || !use_mmx)
		return 1;

	// see if we need to (re-)initialise
	if (previous_depth[bank] != depth || previous_width[bank] != src_pitch || previous_height[bank] != height + 3)
	{
		Init_2xSaIMMX(depth == 15 ? 555 : 565);

		scale_allocate_local_buffer(src_pitch, height + 3, bank);
		memset(scale_buffer[bank], 0xAA, src_pitch * (height + 3));

		previous_depth[bank] = depth;
	}

	if (scale_buffer[bank] == NULL)
		return 1;

	buf = scale_buffer[bank];

	// treat 2xSaI differently from the others to better align the blitters with scanlines
	if (scale_effect.effect == SCALE_EFFECT_2XSAI)
	{
		for (y = 1; y < height; y++, src += src_pitch, buf += src_pitch, dst += 2 * dst_pitch)
			scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
		scale_2xsai_line(src, buf, 0, width - 1, dst, dst_pitch, 0);
	}
	else
	{
		scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
		dst += dst_pitch;
		for (y = 0; y < height; y++, src += src_pitch, buf += src_pitch, dst += 2 * dst_pitch)
			scale_2xsai_line(src, buf, src_pitch, width - 1, dst, dst_pitch, 0);
	}

	return 0;
}



//============================================================
//	scale_perform_hq2x
//============================================================
#ifdef ASM_HQ
static int scale_perform_hq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	interp_init();

	// see if we need to (re-)initialise
	if (previous_width[0] != dst_pitch || previous_height[0] != height * 2)
	{
		scale_allocate_local_buffer(dst_pitch, height * 2, 0);

		previous_depth[0] = 0;
	}

	if (depth == 32)
	{
		UINT8 *ptr = dst;
		UINT8 *s = scale_buffer[0];
		int i;

		hq2x_32((unsigned short *)src, (unsigned short *)s, width, height, width * 4 * 2, src_pitch >> 1);

		for (i = 0; i < height * 2; i++)
		{
			memcpy(ptr, s, width * 4 * 2);
			s += width * 4 * 2;
			ptr += dst_pitch;
		}
	}
	else
	{
		UINT8 *ptr = dst;
		UINT8 *s = scale_buffer[0];
		int i;

		hq2x_16((unsigned short *)src, (unsigned short *)s, width, height, width * 2 * 2, src_pitch >> 1);

		for (i = 0; i < height * 2; i++)
		{
			memcpy(ptr, s, width * 2 * 2);
			s += width * 2 * 2;
			ptr += dst_pitch;
		}
	}

	scale_emms();

	return 0;
}
#endif /* ASM_HQ */

//============================================================
//	scale_perform_hlq2x
//============================================================

static int scale_perform_hlq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		scale_hlq2x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst	+= 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 2 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;

	if (depth == 15)
		scale_hlq2x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

#ifdef USE_MMX_INTERP_SCALE
	scale_emms();
#endif /* USE_MMX_INTERP_SCALE */

	return 0;
}

//============================================================
//	scale_perform_hlq2x3
//============================================================

static int scale_perform_hlq2x3(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		scale_hlq2x3_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst	+= 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x3_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;

	if (depth == 15)
		scale_hlq2x3_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x3_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

#ifdef USE_MMX_INTERP_SCALE
	scale_emms();
#endif /* USE_MMX_INTERP_SCALE */

	return 0;
}

//============================================================
//	scale_perform_hlq2x4
//============================================================

static int scale_perform_hlq2x4(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		scale_hlq2x4_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst	+= 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x4_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;

	if (depth == 15)
		scale_hlq2x4_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq2x4_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

#ifdef USE_MMX_INTERP_SCALE
	scale_emms();
#endif /* USE_MMX_INTERP_SCALE */

	return 0;
}

//============================================================
//	scale_perform_hq3x
//============================================================
#ifdef ASM_HQ
static int scale_perform_hq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	interp_init();

	// see if we need to (re-)initialise
	if (previous_width[0] != dst_pitch || previous_height[0] != height * 3)
	{
		scale_allocate_local_buffer(dst_pitch, height * 3, 0);

		previous_depth[0] = 0;
	}

	if (depth == 32)
	{
		UINT8 *ptr = dst;
		UINT8 *s = scale_buffer[0];
		int i;

		hq3x_32((unsigned short *)src, (unsigned short *)s, width, height, width * 4 * 3, src_pitch >> 1);

		for (i = 0; i < height * 3; i++)
		{
			memcpy(ptr, s, width * 4 * 3);
			s += width * 4 * 3;
			ptr += dst_pitch;
		}
	}
	else
	{
		UINT8 *ptr = dst;
		UINT8 *s = scale_buffer[0];
		int i;

		hq3x_16((unsigned short *)src, (unsigned short *)s, width, height, width * 2 * 3, src_pitch >> 1);

		for (i = 0; i < height * 3; i++)
		{
			memcpy(ptr, s, width * 2 * 3);
			s += width * 2 * 3;
			ptr += dst_pitch;
		}
	}

	scale_emms();

	return 0;
}
#endif /* ASM_HQ */

//============================================================
//	scale_perform_hlq3x
//============================================================

static int scale_perform_hlq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		scale_hlq3x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);

		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq3x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 3 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;

	if (depth == 15)
		scale_hlq3x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

#ifdef USE_MMX_INTERP_SCALE
	scale_emms();
#endif /* USE_MMX_INTERP_SCALE */

	return 0;
}

#ifdef USE_4X_SCALE
//============================================================
//	scale_perform_hlq4x
//============================================================

static int scale_perform_hlq4x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		scale_hlq4x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);

		for (y = 4; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq4x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
		}
	}
	else
	{
		scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 4; y < height; y++)
		{
			dst += 4 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}
	}

	dst += 4 * dst_pitch;
	src_prev = src_curr;
	src_curr = src_next;

	if (depth == 15)
		scale_hlq4x_15_def((UINT16 *)dst, (UINT16 *)(dst + dst_pitch), (UINT16 *)(dst + 2 * dst_pitch), (UINT16 *)(dst + 3 * dst_pitch), (UINT16 *)src_prev, (UINT16 *)src_curr, (UINT16 *)src_next, width);
	else
		scale_hlq4x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)(dst + 3 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

#ifdef USE_MMX_INTERP_SCALE
	scale_emms();
#endif /* USE_MMX_INTERP_SCALE */

	return 0;
}
#endif /* USE_4X_SCALE */
