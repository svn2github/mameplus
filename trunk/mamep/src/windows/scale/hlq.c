/* HQx C implementation */
//#undef INTERP_MMX

#include "interp.h"

#define PIXEL_BITS 15
#include "hlq2x.inc"

#define PIXEL_BITS 16
#include "hlq2x.inc"

#define PIXEL_BITS 32
#include "hlq2x.inc"

#define PIXEL_BITS 15
#include "hlq3x.inc"

#define PIXEL_BITS 16
#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

#ifdef SUPPORT_4X_EFFECT
#define PIXEL_BITS 15
#include "hlq4x.inc"

#define PIXEL_BITS 16
#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"
#endif /* SUPPORT_4X_EFFECT */


/* LQx C implementation */
#define PIXEL_LQ_FILTER

#define PIXEL_BITS 15
#include "hlq2x.inc"

#define PIXEL_BITS 16
#include "hlq2x.inc"

#define PIXEL_BITS 32
#include "hlq2x.inc"

#define PIXEL_BITS 15
#include "hlq3x.inc"

#define PIXEL_BITS 16
#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

#ifdef SUPPORT_4X_EFFECT
#define PIXEL_BITS 15
#include "hlq4x.inc"

#define PIXEL_BITS 16
#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"
#endif /* SUPPORT_4X_EFFECT */
