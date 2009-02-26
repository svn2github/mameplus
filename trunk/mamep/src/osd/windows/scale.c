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

// defines
enum
{
	SCALE_EFFECT_NONE = 0,
	SCALE_EFFECT_SCANLINESTV,
	SCALE_EFFECT_SCALE2X,
//	SCALE_EFFECT_SUPERSCALE,
	SCALE_EFFECT_SUPERSCALE75,
	SCALE_EFFECT_SCALE3X,
	SCALE_EFFECT_2XSAI,
	SCALE_EFFECT_SUPER2XSAI,
	SCALE_EFFECT_SUPEREAGLE,
	SCALE_EFFECT_EAGLE,
	SCALE_EFFECT_2XPM,
	SCALE_EFFECT_HQ2X,
	SCALE_EFFECT_HQ2XS,
	SCALE_EFFECT_HQ3X,
//	SCALE_EFFECT_HQ3XS,
	SCALE_EFFECT_LAST
};

/* fixme: No longer used by the core */
#ifndef MAX_SCREENS
/* maximum number of screens for one game */
#define MAX_SCREENS					8
#endif
#define MAX_SCALE_BANK				(MAX_SCREENS * 2)


//============================================================
//	GLOBAL VARIABLES
//============================================================

struct _scale_effect scale_effect;

UINT32 RGBtoYUV[65536];
UINT32 LUT16to32[65536];

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
	"scanlinestv",
	"scale2x",
//	"superscale",
	"superscale75",
	"scale3x",
	"2xsai",
	"super2xsai",
	"supereagle",
	"eagle",
	"2xpm",
	"hq2x",
	"hq2xs",
	"hq3x",
//	"hq3xs",
	NULL
};

static const char *str_desc[] =
{
	"None",
	"Scanlines TV",
	"Scale2x",           // by AdvMAME v2.2
//	"SuperScale",        // by ElSemi, same as Scale2x
	"SuperScale w/ Scanlines", // by ElSemi
	"Scale3x",           // by AdvMAME v2.2
	"2xSaI",             // by Kreed v0.59
	"Super 2xSaI",       // by Kreed v0.59
	"Super Eagle",       // by Kreed v0.59
	"Eagle",             // by Dirk Stevens v0.41a
	"2xPM",              // by Pablo Medina v0.2
	"HQ2x",             //
	"HQ2xS",             //
	"HQ3x",      // by Maxim Stepin
//	"HQ3xS",
	NULL
};



//============================================================
//	prototypes
//============================================================

static void scale_allocate_local_buffer(int width, int height, int bank);

#ifndef PTR64
// functions from scale2x
static int scale_perform_scale2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth, int bank);
static void (*scale_scale2x_line_16)(UINT16 *dst0, UINT16 *dst1, const UINT16 *src0, const UINT16 *src1, const UINT16 *src2, unsigned count);
static void (*scale_scale2x_line_32)(UINT32 *dst0, UINT32 *dst1, const UINT32 *src0, const UINT32 *src1, const UINT32 *src2, unsigned count);
#endif /* PTR64 */

static int scale_perform_scale3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

#ifndef PTR64
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
#endif /* PTR64 */

// functions from AdvMAME
void scale2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void scale2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void scale2x_16_mmx(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void scale2x_32_mmx(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void scale3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void scale3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void hq2x_32_def(UINT32*, UINT32*, UINT32*, UINT32*, UINT32*, int);
void hq3x_32_def(UINT32*, UINT32*, UINT32*, UINT32*, UINT32*, UINT32*, int);
static int scale_perform_hq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);
static int scale_perform_hq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth);

// functions from vba-rerecording
void hqxx_init(unsigned bits_per_pixel);
void hq2xS(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);
void hq2xS32(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);
//void hq3xS(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);
//void hq3xS32(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);
void ScanlinesTV(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);

// functions from SNES9x
//void InitLUTs(void);
//void RenderHQ2X(unsigned char*, unsigned int, unsigned char*, unsigned int, int, int, int);

// functions from Kega Fusion
void _2xpm_555(void *SrcPtr, void *DstPtr, unsigned long SrcPitch, unsigned long DstPitch, unsigned long SrcW, unsigned long SrcH, int depth);

void hq2x_16_555(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);
void hq3x_16_555(unsigned short *in,unsigned short *out,unsigned int width,unsigned int height,unsigned int pitch,unsigned int xpitch);

#undef INTERP_RGB16_TABLE
static void interp_init(void)
{
	static int interp_inited = 0;
	int i, j, k, r, g, b, y, u, v;

	if (interp_inited) return;
	interp_inited = 1;

#ifdef INTERP_RGB16_TABLE
	for (i = 0; i < 65536; i++)
	{
		UINT8 r = (i & 0xF800) >> 11;
		UINT8 g = (i & 0x07E0) >> 5;
		UINT8 b = i & 0x001F;

		r = (r << 3) | (r >> 2);
		g = (g << 2) | (g >> 4);
		b = (b << 3) | (b >> 2);
		LUT16to32[i] = (r << 16) | (g << 8) | b;
	}

	for (i = 0; i < 32; i++)
		for (j = 0; j < 64; j++)
			for (k = 0; k < 32; k++)
			{
				r = (i << 3) | (i >> 2);
				g = (j << 2) | (j >> 4);
				b = (k << 3) | (k >> 2);
				y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				RGBtoYUV[ (i << 11) + (j << 5) + k ] = (y << 16) + (u << 8) + v;
			}

#else /* INTERP_RGB16_TABLE */
	memset(LUT16to32 + 0x8000, 0, sizeof (*LUT16to32) * 0x8000);
	memset(RGBtoYUV + 0x8000, 0, sizeof (*RGBtoYUV) * 0x8000);

	for (i=0; i < 0x8000; i++)
	{
		UINT32 col = ((i & 0x7C00) << 9) + ((i & 0x03E0) << 6) + ((i & 0x001F) << 3);
		LUT16to32[i] = col | ((col >> 5) & 0x070707);
	}

	for (i = 0; i < 32; i++)
		for (j = 0; j < 32; j++)
			for (k = 0; k < 32; k++)
			{
				r = (i << 3) | (i >> 2);
				g = (j << 3) | (j >> 2);
				b = (k << 3) | (k >> 2);
				y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				RGBtoYUV[ (i << 10) + (j << 5) + k ] = (y << 16) + (u << 8) + v;
			}
#endif /* INTERP_RGB16_TABLE */
}


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
#ifndef PTR64
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
#endif /* PTR64 */

//============================================================
//	scale_init
//============================================================

int scale_init(void)
{
	static char name[64];

#ifndef PTR64
	UINT32 features = x86_get_features();
	use_mmx = (features & (1 << 23));
#endif /* PTR64 */

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
		case SCALE_EFFECT_SCANLINESTV:
		{
			sprintf(name, "Scanlines TV");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		case SCALE_EFFECT_SCALE2X:
		{
			sprintf(name, "Scale2x (%s)", use_mmx ? "mmx optimised" : "non-mmx version");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}
		case SCALE_EFFECT_SCALE3X:
		{
			sprintf(name, "Scale3x (non-mmx version)");
			scale_effect.xsize = scale_effect.ysize = 3;
			break;
		}
#ifndef PTR64
		case SCALE_EFFECT_EAGLE:
		{
			sprintf(name, "Eagle (mmx optimised)");

			if (!use_mmx)
				return 1;

			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

//		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
		{
			/*
			if (scale_effect.effect == SCALE_EFFECT_SUPERSCALE)
			{
				sprintf(name, "SuperScale (mmx optimised)");
				superscale_line_func = superscale_line;
			}
			else
			*/
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
#endif /* PTR64 */
		case SCALE_EFFECT_2XPM:
		{
			sprintf(name, "2xPM");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_HQ2X:
		case SCALE_EFFECT_HQ2XS:
		{
//			InitLUTs();
			sprintf(name, "HQ2x");
			scale_effect.xsize = scale_effect.ysize = 2;
			break;
		}

		case SCALE_EFFECT_HQ3X:
//		case SCALE_EFFECT_HQ3XS:
		{
//			hqxx_init(32);
			sprintf(name, "HQ3x");
			scale_effect.xsize = scale_effect.ysize = 3;
			break;
		}

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
		case SCALE_EFFECT_SCALE3X:
			if (depth == 15 || depth == 16 || depth == 32)
				return 0;
			else
				return 1;

//		case SCALE_EFFECT_SUPERSCALE:
		case SCALE_EFFECT_SUPERSCALE75:
		case SCALE_EFFECT_EAGLE:
		case SCALE_EFFECT_2XSAI:
		case SCALE_EFFECT_SUPER2XSAI:
		case SCALE_EFFECT_SUPEREAGLE:
			if (use_mmx && (depth == 15 || depth == 16))
				return 0;
			else
				return 1;

		case SCALE_EFFECT_SCANLINESTV://fixme for 32
		case SCALE_EFFECT_2XPM:
			if (depth == 15)
				return 0;
			else
				return 1;

		case SCALE_EFFECT_HQ2X:
		case SCALE_EFFECT_HQ2XS:
		case SCALE_EFFECT_HQ3X:
//		case SCALE_EFFECT_HQ3XS:
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
		
		case SCALE_EFFECT_SCANLINESTV:
			ScanlinesTV((unsigned char*)src, (unsigned int)src_pitch, NULL, (unsigned char*)dst, (unsigned int)dst_pitch, width, height);
			return 0;

#ifndef PTR64
		case SCALE_EFFECT_SCALE2X:
			return scale_perform_scale2x(src, dst, src_pitch, dst_pitch, width, height, depth, bank);
#endif /* PTR64 */

		case SCALE_EFFECT_SCALE3X:
			return scale_perform_scale3x(src, dst, src_pitch, dst_pitch, width, height, depth);
#ifndef PTR64

//		case SCALE_EFFECT_SUPERSCALE:
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
#endif /* PTR64 */

		case SCALE_EFFECT_2XPM:
			if (depth != 15)
				return 1;

			_2xpm_555(src, dst, (unsigned long)src_pitch, (unsigned long)dst_pitch, (unsigned long)width, (unsigned long)height, depth);
			return 0;

		case SCALE_EFFECT_HQ2X:
			return scale_perform_hq2x(src, dst, src_pitch, dst_pitch, width, height, depth);

		case SCALE_EFFECT_HQ2XS:
			hq2xS32((unsigned char*)src, (unsigned int)src_pitch, NULL, (unsigned char*)dst, (unsigned int)dst_pitch, width, height);
			return 0;

		case SCALE_EFFECT_HQ3X:
			return scale_perform_hq3x(src, dst, src_pitch, dst_pitch, width, height, depth);

//		case SCALE_EFFECT_HQ3XS:
//			hq3xS32((unsigned char*)src, (unsigned int)src_pitch, NULL, (unsigned char*)dst, (unsigned int)dst_pitch, width, height);
//			return 0;

		default:
			return 1;
	}
}

//============================================================
//	scale_emms
//============================================================

INLINE void scale_emms(void)
{
#ifdef USE_MMX_INTERP_SCALE
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
#endif /* USE_MMX_INTERP_SCALE */
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
#ifndef PTR64
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
#endif /* PTR64 */

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


#ifndef PTR64
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
#endif /* PTR64 */


#ifndef PTR64
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
#endif /* PTR64 */


//============================================================
//	scale_perform_hq2x
//============================================================

static int scale_perform_hq2x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();

	if (depth == 15)
	{
		hq2x_16_555((unsigned short *)src, (unsigned short *)dst, width, height, dst_pitch, src_pitch >> 1);
	}
	else
	{
		hq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 2; y < height; y++)
		{
			dst += 2 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			hq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}

		dst += 2 * dst_pitch;
		src_prev = src_curr;
		src_curr = src_next;

		hq2x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
	}

	scale_emms();

	return 0;
}

//============================================================
//	scale_perform_hq3x
//============================================================

static int scale_perform_hq3x(UINT8 *src, UINT8 *dst, int src_pitch, int dst_pitch, int width, int height, int depth)
{
	int y;
	UINT8 *src_prev = src;
	UINT8 *src_curr = src;
	UINT8 *src_next = src + src_pitch;

	interp_init();
	
	if (depth == 15)
	{
		hq3x_16_555((unsigned short *)src, (unsigned short *)dst, width, height, dst_pitch, src_pitch >> 1);
	}
	else
	{
		hq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);

		for (y = 3; y < height; y++)
		{
			dst += 3 * dst_pitch;
			src_prev = src_curr;
			src_curr = src_next;
			src_next += src_pitch;

			hq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
		}

		dst += 3 * dst_pitch;
		src_prev = src_curr;
		src_curr = src_next;

		hq3x_32_def((UINT32 *)dst, (UINT32 *)(dst + dst_pitch), (UINT32 *)(dst + 2 * dst_pitch), (UINT32 *)src_prev, (UINT32 *)src_curr, (UINT32 *)src_next, width);
	}

	scale_emms();

	return 0;
}
