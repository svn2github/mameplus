/* HQx C implementation */
//#undef INTERP_MMX

#include "interp.h"

#define PIXEL_BITS 15
#include "hq2x.inc"

#define PIXEL_BITS 16
#include "hq2x.inc"

#define PIXEL_BITS 32
#include "hq2x.inc"

#define PIXEL_BITS 15
#include "hq3x.inc"

#define PIXEL_BITS 16
#include "hq3x.inc"

#define PIXEL_BITS 32
#include "hq3x.inc"

#ifdef SUPPORT_4X_EFFECT
#define PIXEL_BITS 15
#include "hq4x.inc"

#define PIXEL_BITS 16
#include "hq4x.inc"

#define PIXEL_BITS 32
#include "hq4x.inc"
#endif /* SUPPORT_4X_EFFECT */


/* LQx C implementation */
#define PIXEL_LQ_FILTER

#define PIXEL_BITS 15
#include "hq2x.inc"

#define PIXEL_BITS 16
#include "hq2x.inc"

#define PIXEL_BITS 32
#include "hq2x.inc"

#define PIXEL_BITS 15
#include "hq3x.inc"

#define PIXEL_BITS 16
#include "hq3x.inc"

#define PIXEL_BITS 32
#include "hq3x.inc"

#ifdef SUPPORT_4X_EFFECT
#define PIXEL_BITS 15
#include "hq4x.inc"

#define PIXEL_BITS 16
#include "hq4x.inc"

#define PIXEL_BITS 32
#include "hq4x.inc"
#endif /* SUPPORT_4X_EFFECT */
