//============================================================
//
//  osinline.h - Win32 inline functions
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __OSINLINE__
#define __OSINLINE__

#include "osd_cpu.h"


//============================================================
//  INLINE FUNCTIONS
//============================================================

#ifdef PTR64

#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
	return (int)(((INT64)x * (INT64)y) >> 32);
}

#elif defined(_MSC_VER)

#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
    int result;

    __asm {
        mov eax, x
        imul y
        mov result, edx
}

    return result;
}

#else

#define vec_mult _vec_mult
INLINE int _vec_mult(int x, int y)
{
	int result;
	__asm__ (
			"movl  %1    , %0    ; "
			"imull %2            ; "    /* do the multiply */
			"movl  %%edx , %%eax ; "
			:  "=&a" (result)           /* the result has to go in eax */
			:  "mr" (x),                /* x and y can be regs or mem */
			   "mr" (y)
			:  "%edx", "%cc"            /* clobbers edx and flags */
		);
	return result;
}

#endif /* PTR64 */


/*-------------------------------------------------
	TRANSPARENT UI MACROS
-------------------------------------------------*/

#ifndef USE_SAMPLE_MACORS_FOR_TRANSPARENT_UI
#define draw_transparent16_PALETTE(src,dst,y) \
{ \
	UINT8 r = ((UINT8 *)&adjusted_palette[*src])[2]; \
	UINT8 g = ((UINT8 *)&adjusted_palette[*src])[1]; \
	UINT8 b = ((UINT8 *)&adjusted_palette[*src])[0]; \
 \
	r = ui_transparent_background[0][y << 8 | r]; \
	g = ui_transparent_background[1][y << 8 | g]; \
	b = ui_transparent_background[2][y << 8 | b]; \
 \
	*dst = find_near_palette_by_index(get_colormap_index_by_rgb(r, g, b)); \
}

#define draw_transparent16_RGB15(src,dst,y) \
{ \
	UINT8 r = (*src >> 10); \
	UINT8 g = (*src >> 5); \
	UINT8 b = *src; \
 \
	r = ui_transparent_background[0][y << 8 | r]; \
	g = ui_transparent_background[1][y << 8 | g]; \
	b = ui_transparent_background[2][y << 8 | b]; \
 \
	*dst = r << 10 | g << 5 | b; \
}

#define draw_transparent32_RGB32(src,dst,y) \
{ \
	UINT8 r = ((UINT8 *)src)[2]; \
	UINT8 g = ((UINT8 *)src)[1]; \
	UINT8 b = ((UINT8 *)src)[0]; \
 \
	((UINT8 *)dst)[2] = ui_transparent_background[0][y << 8 | r]; \
	((UINT8 *)dst)[1] = ui_transparent_background[1][y << 8 | g]; \
	((UINT8 *)dst)[0] = ui_transparent_background[2][y << 8 | b]; \
}
#endif /* USE_SAMPLE_MACORS_FOR_TRANSPARENT_UI */
#endif /* __OSINLINE__ */
