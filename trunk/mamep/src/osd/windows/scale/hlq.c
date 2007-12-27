#include "interp.h"

/* HQx C implementation */

#define PIXEL_BITS 15
#include "hlq2x.inc"

//#define PIXEL_BITS 16
//#include "hlq2x.inc"

#define PIXEL_BITS 32
#include "hlq2x.inc"

#define PIXEL_BITS 15
#include "hlq2x3.inc"

//#define PIXEL_BITS 16
//#include "hlq2x3.inc"

#define PIXEL_BITS 32
#include "hlq2x3.inc"

#define PIXEL_BITS 15
#include "hlq2x4.inc"

//#define PIXEL_BITS 16
//#include "hlq2x4.inc"

#define PIXEL_BITS 32
#include "hlq2x4.inc"

#define PIXEL_BITS 15
#include "hlq3x.inc"

//#define PIXEL_BITS 16
//#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

#ifdef USE_4X_SCALE
#define PIXEL_BITS 15
#include "hlq4x.inc"

//#define PIXEL_BITS 16
//#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"
#endif /* USE_4X_SCALE */


/* LQx C implementation */
#define PIXEL_LQ_FILTER

#define PIXEL_BITS 15
#include "hlq2x.inc"

//#define PIXEL_BITS 16
//#include "hlq2x.inc"

#define PIXEL_BITS 32
#include "hlq2x.inc"

#define PIXEL_BITS 15
#include "hlq2x3.inc"

//#define PIXEL_BITS 16
//#include "hlq2x3.inc"

#define PIXEL_BITS 32
#include "hlq2x3.inc"

#define PIXEL_BITS 15
#include "hlq2x4.inc"

//#define PIXEL_BITS 16
//#include "hlq2x4.inc"

#define PIXEL_BITS 32
#include "hlq2x4.inc"

#define PIXEL_BITS 15
#include "hlq3x.inc"

//#define PIXEL_BITS 16
//#include "hlq3x.inc"

#define PIXEL_BITS 32
#include "hlq3x.inc"

#ifdef USE_4X_SCALE
#define PIXEL_BITS 15
#include "hlq4x.inc"

//#define PIXEL_BITS 16
//#include "hlq4x.inc"

#define PIXEL_BITS 32
#include "hlq4x.inc"
#endif /* USE_4X_SCALE */
