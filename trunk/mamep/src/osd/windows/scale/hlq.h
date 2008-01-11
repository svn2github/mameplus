#undef INTERP_RGB16_TABLE

UINT32 RGBtoYUV[65536];
UINT32 LUT16to32[65536];

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

void hq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void hq2x3_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void hq2x4_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x4_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x3_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x4_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x4_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

//#ifndef ASM_HQ
void hq3x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
//#endif /*ASM_HQ*/

void lq3x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

#ifdef USE_4X_SCALE
void hq4x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq4x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#endif /* USE_4X_SCALE */
