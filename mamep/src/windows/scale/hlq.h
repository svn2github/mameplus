UINT32 interp_RGB2YUV[65536];

static void interp_init(void)
{
	static int interp_inited = 0;
	int i, j, k, r, g, b, y, u, v;

	if (interp_inited) return;
	interp_inited = 1;

	for (i=0; i<32; i++) {
		for (j=0; j<64; j++) {
			for (k=0; k<32; k++) {
				r = i << 3;
				g = j << 2;
				b = k << 3;
				y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2*g -b) >> 3);
				interp_RGB2YUV[ (i << 11) + (j << 5) + k ] = (y << 16) + (u << 8) + v;
			}
		}
	}
}


void hq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq2x_15_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x_16_def(UINT16* dst0, UINT16* dst1, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq2x_32_def(UINT32* dst0, UINT32* dst1, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void hq3x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq3x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq3x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq3x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

#ifdef SUPPORT_4X_EFFECT
void hq4x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void hq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);

void lq4x_15_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq4x_16_def(UINT16* dst0, UINT16* dst1, UINT16* dst2, UINT16* dst3, const UINT16* src0, const UINT16* src1, const UINT16* src2, unsigned count);
void lq4x_32_def(UINT32* dst0, UINT32* dst1, UINT32* dst2, UINT32* dst3, const UINT32* src0, const UINT32* src1, const UINT32* src2, unsigned count);
#endif /* SUPPORT_4X_EFFECT */
