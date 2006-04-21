#ifndef PIXEL_BITS
#error "PIXEL_BITS not defined"
#endif

#include "hlq.inc"

#ifdef PIXEL_LQ_FILTER
	#define PIXEL_PROC				_TOKEN3(lq2x3_, PIXEL_BITS, _def)
#else
	#define PIXEL_PROC				_TOKEN3(hq2x3_, PIXEL_BITS, _def)
#endif

void PIXEL_PROC(
	PIXEL_UINT* dst0, 
	PIXEL_UINT* dst1, 
	PIXEL_UINT* dst2, 
	const PIXEL_UINT* src0, 
	const PIXEL_UINT* src1, 
	const PIXEL_UINT* src2, 
	unsigned count)
{
	int pattern;
	PIXEL_COLOR c[9];
#ifdef PIXEL_INDEX
	PIXEL_INDEX s[9];
#endif

	unsigned i;
	unsigned last = count - 1;

	for(i = 0; i < count; i++) {
		if (i > 0) {
			PIXEL_COPY(0, 1);
			PIXEL_COPY(1, 2);
			PIXEL_COPY(3, 4);
			PIXEL_COPY(4, 5);
			PIXEL_COPY(6, 7);
			PIXEL_COPY(7, 8);
		}
		else {
			PIXEL_SRC(1, src0[0]);
			PIXEL_COPY(0, 1);
			PIXEL_SRC(4, src1[0]);
			PIXEL_COPY(3, 4);
			PIXEL_SRC(7, src2[0]);
			PIXEL_COPY(6, 7);
		}
		if (i < last) {
			PIXEL_SRC(2, src0[1]);
			PIXEL_SRC(5, src1[1]);
			PIXEL_SRC(8, src2[1]);
		} 
		else {
			PIXEL_COPY(2, 1);
			PIXEL_COPY(5, 4);
			PIXEL_COPY(8, 7);
		}

		pattern = 0;
		if (PIXEL_DIFF(4, 0)) pattern |= 1 << 0;
		if (PIXEL_DIFF(4, 1)) pattern |= 1 << 1;
		if (PIXEL_DIFF(4, 2)) pattern |= 1 << 2;
		if (PIXEL_DIFF(4, 3)) pattern |= 1 << 3;
		if (PIXEL_DIFF(4, 5)) pattern |= 1 << 4;
		if (PIXEL_DIFF(4, 6)) pattern |= 1 << 5;
		if (PIXEL_DIFF(4, 7)) pattern |= 1 << 6;
		if (PIXEL_DIFF(4, 8)) pattern |= 1 << 7;

		switch (pattern) {
#ifdef PIXEL_LQ_FILTER		
		#include "lq2x3.dat"
#else
		#include "hq2x3.dat"
#endif
		}

		src0 += 1;
		src1 += 1;
		src2 += 1;
		dst0 += 2;
		dst1 += 2;
		dst2 += 2;
	}
}

#undef PIXEL_PROC
#undef PIXEL_BITS
