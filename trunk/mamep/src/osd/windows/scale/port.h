/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#ifndef _PORT_H_
#define _PORT_H_

#include <limits.h>

#ifndef STORM
#include <memory.h>
#include <string.h>
#else
#include <strings.h>
#include <clib/powerpc_protos.h>
#endif

#ifndef ACCEPT_SIZE_T
#ifdef __WIN32__
#define ACCEPT_SIZE_T int
#else
#define ACCEPT_SIZE_T unsigned int
#endif
#endif

#include <sys/types.h>

/* #define PIXEL_FORMAT RGB565 */
#define GFX_MULTI_FORMAT

#ifndef NOASM
//#define USE_X86_ASM
#endif

#ifdef __MACOSX__

	#ifdef _C
	#undef _C
	#endif

	#ifdef _D
	#undef _D
	#endif

	#define CHECK_SOUND()
	#define PIXEL_FORMAT RGB555
	#undef GFX_MULTI_FORMAT
	#undef USE_X86_ASM
	#undef _MAX_PATH

	#define SET_UI_COLOR(r,g,b)	SetInfoDlgColor(r,g,b)
	void SetInfoDlgColor(unsigned char, unsigned char, unsigned char);

#endif /* __MACOSX__ */

#ifndef snes9x_types_defined
#define snes9x_types_defined

typedef unsigned char bool8;

#ifdef HAVE_STDINT_H
#include <stdint.h>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef intptr_t pint;

#else /* Don't have stdint.h */

#ifdef PTR_NOT_INT
typedef long pint;
#else /* pointer is int */
typedef int pint;
#endif /* PTR_NOT_INT */

/* FIXME: Refactor this by moving out the BORLAND part and unifying typedefs */
#ifndef __WIN32__
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef unsigned int uint32;
# ifdef __GNUC__  /* long long is not part of ISO C++ */
__extension__
# endif
typedef long long int64;
typedef unsigned long long uint64;
#else /* __WIN32__ */

#include <windows.h>

# ifdef __BORLANDC__
#   include <systypes.h>
# else

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;

# ifndef WSAAPI
/* winsock2.h typedefs int32 as well. */
typedef long int32;

#   define PLAT_SOUND_BUFFER SoundBuffer
#   define RIGHTSHIFT_IS_SAR
# endif

# if defined(_MSC_VER) && (_MSC_VER == 1400) /* VC8.0 */
/* temporary solution for fatal error C1063 (cl.exe 14.00.50727.762) */
#   ifdef RIGHTSHIFT_IS_SAR
#     undef RIGHTSHIFT_IS_SAR
#   endif /* RIGHTSHIFT_IS_SAR */
#   define RIGHTSHIFT_INT8_IS_SAR
#   define RIGHTSHIFT_INT16_IS_SAR
#   define RIGHTSHIFT_INT32_IS_SAR
# endif /* VC8.0 */

typedef unsigned int uint32;

# endif /* __BORLANDC__ */

typedef __int64 int64;
typedef unsigned __int64 uint64;

#endif /* __WIN32__ */
#endif /* HAVE_STDINT_H */
#endif /* snes9x_types_defined */


typedef uint8  u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef int8   s8;
typedef int16  s16;
typedef int64  s64;

// from advmame
typedef unsigned short interp_uint16;
typedef unsigned interp_uint32;

typedef unsigned char scale2x_uint8;
typedef unsigned short scale2x_uint16;
typedef unsigned scale2x_uint32;

typedef unsigned char scale3x_uint8;
typedef unsigned short scale3x_uint16;
typedef unsigned scale3x_uint32;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef bool
#define bool BOOL
#endif

#endif

