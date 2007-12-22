UINT32 RGBtoYUV[65536];
UINT32   LUT16to32[65536];
UINT8 *Buffer=0;

static void interp_init(void)
{
	static int interp_inited = 0;
	int i, j, k, r, g, b, y, u, v;

	if (interp_inited) return;
	interp_inited = 1;

	for (i=0; i<65536; i++)
		LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);

	for (i=0; i<32; i++) {
		for (j=0; j<64; j++) {
			for (k=0; k<32; k++) {
				r = i << 3;
				g = j << 2;
				b = k << 3;
				y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				RGBtoYUV[ (i << 11) + (j << 5) + k ] = (y << 16) + (u << 8) + v;
			}
		}
	}
	
	if(Buffer)
		free(Buffer);
	Buffer=0;
}

#ifndef ASM_HQ
void hq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void hq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#endif /*ASM_HQ*/
void hq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void hq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x4_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
//void lq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x3_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x3_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
void lq2x4_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
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
