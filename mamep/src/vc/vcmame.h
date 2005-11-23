//---------------------------------------------------------------------------
// vcmame.h
//
// Header file used to compile M.A.M.E. with Visual C++
//---------------------------------------------------------------------------

#ifndef __VCMAME_H_
#define __VCMAME_H_
#pragma once

//---------------------------------------------------------------------------
// NOTE:
//
// All of the global #defines and library declarations have been moved into
// the project itself.  Although harder to maintain, it requires many less
// modifications to the M.A.M.E. source code that way
//---------------------------------------------------------------------------

// The IDE project manager can't seem to handle "INLINE=static __inline" as
// part of the project settings without screwing up, so the project declares
// "INLINE=__inline", and we overwrite it here with the proper expansion

#ifdef INLINE
#undef INLINE
#endif

//#define INLINE static __forceinline
#define INLINE static __inline

#include "windows\winprefix.h"		// <-- PI and M_PI

//---------------------------------------------------------------------------
// Compiler Warnings

#pragma warning(disable:592)		// "variable is used before its value is set"
#pragma warning(disable:4018)		// "signed/unsigned mismatch"
#pragma warning(disable:4022)		// "pointer mismatch for actual parameter"
#pragma warning(disable:4090)		// "different 'const' qualifiers"
#pragma warning(disable:4142)		// "benign redefinition of type"
#pragma warning(disable:4146)		// "unary minus operator applied to unsigned type"
#pragma warning(disable:4244)		// "possible loss of data"
#pragma warning(disable:4305)		// "truncation from 'type' to 'type'
#pragma warning(disable:4550)		// "expression evaluates .. missing an argument list"
#pragma warning(disable:4552)		// "operator has no effect"
#pragma warning(disable:4761)		// "integral size mismatch in argument"

//---------------------------------------------------------------------------
// M68000 Core Selection
//
// NOTE : If you change anything in this section, you must "Rebuild All"

//#define A68K0						// Assembler-based M68000 CPU Core
//#define A68K2						// Assembler-based M68020 CPU Core

//---------------------------------------------------------------------------
// HAS_CPU Declarations
//
// NOTE : If you change anything in this section, you must "Rebuild All"
#define HAS_Z80 1
#define HAS_Z180 1
#define HAS_8080 1
#define HAS_8085A 1
#define HAS_M6502 1
#define HAS_M65C02 1
//#define HAS_M65SC02 1
//#define HAS_M65CE02 1
//#define HAS_M6509 1
#define HAS_M6510 1
//#define HAS_M6510T 1
//#define HAS_M7501 1
//#define HAS_M8502 1
#define HAS_N2A03 1
#define HAS_DECO16 1
//#define HAS_M4510 1
#define HAS_H6280 1
#define HAS_I86 1
//#define HAS_I88 1
#define HAS_I186 1
//#define HAS_I188 1
//#define HAS_I286 1
#define HAS_V20 1
#define HAS_V30 1
#define HAS_V33 1
#define HAS_V60 1
#define HAS_V70 1
#define HAS_I8035 1
#define HAS_I8039 1
#define HAS_I8048 1
#define HAS_N7751 1
#define HAS_I8X41 1
#define HAS_M6800 1
#define HAS_M6801 1
#define HAS_M6802 1
#define HAS_M6803 1
#define HAS_M6808 1
#define HAS_HD63701 1
#define HAS_NSC8105 1
#define HAS_M6805 1
#define HAS_M68705 1
#define HAS_HD63705 1
#define HAS_HD6309 1
#define HAS_M6809 1
#define HAS_M6809E 1
#define HAS_KONAMI 1
#define HAS_M68000 1
#define HAS_M68010 1
#define HAS_M68EC020 1
#define HAS_M68020 1
#define HAS_T11 1
#define HAS_S2650 1
#define HAS_TMS34010 1
#define HAS_TMS34020 1
//#define HAS_TMS9900 1
//#define HAS_TMS9940 1
#define HAS_TMS9980 1
//#define HAS_TMS9985 1
//#define HAS_TMS9989 1
#define HAS_TMS9995 1
//#define HAS_TMS99105A 1
//#define HAS_TMS99110A 1
#define HAS_Z8000 1
#define HAS_TMS32010 1
#define HAS_TMS32025 1
#define HAS_TMS32026 1
#define HAS_TMS32031 1
#define HAS_CCPU 1
#define HAS_ADSP2100 1
#define HAS_ADSP2101 1
#define HAS_ADSP2104 1
#define HAS_ADSP2105 1
#define HAS_ADSP2115 1
#define HAS_PSXCPU 1
#define HAS_ASAP 1
#define HAS_UPD7810 1
#define HAS_UPD7807 1
#define HAS_ARM 1
#define HAS_JAGUAR 1
#define HAS_R3000 1
#define HAS_R4600 1
#define HAS_R5000 1
#define HAS_SH2 1
#define HAS_DSP32C 1
//#define HAS_PIC16C54 1
#define HAS_PIC16C55 1
//#define HAS_PIC16C56 1
#define HAS_PIC16C57 1
//#define HAS_PIC16C58 1
#define HAS_G65816 1
#define HAS_SPC700 1
//#define HAS_E132XS 1

//---------------------------------------------------------------------------
// HAS_SOUND Declarations
//
// NOTE : If you change anything in this section, you must "Rebuild All"
#define HAS_CUSTOM 1
#define HAS_SAMPLES 1
#define HAS_DAC 1
#define HAS_DMADAC 1
#define HAS_DISCRETE 1
#define HAS_AY8910 1
#define HAS_YM2203 1
//enable only one of the following two
//#define HAS_YM2151 1
#define HAS_YM2151_ALT 1
#define HAS_YM2608 1
#define HAS_YM2610 1
#define HAS_YM2610B 1
#define HAS_YM2612 1
#define HAS_YM3438 1
#define HAS_YM2413 1
#define HAS_YM3812 1
#define HAS_YMZ280B 1
#define HAS_YM3526 1
#define HAS_Y8950 1
#define HAS_SN76477 1
#define HAS_SN76496 1
#define HAS_POKEY 1
#define HAS_TIA 1
#define HAS_NES 1
#define HAS_ASTROCADE 1
#define HAS_NAMCO 1
#define HAS_NAMCO_15XX 1
#define HAS_NAMCO_CUS30 1
#define HAS_NAMCO_52XX 1
#define HAS_NAMCO_54XX 1
#define HAS_NAMCO_63701X 1
#define HAS_NAMCONA 1
#define HAS_TMS36XX 1
#define HAS_TMS5110 1
#define HAS_TMS5220 1
#define HAS_VLM5030 1
#define HAS_ADPCM 1
#define HAS_OKIM6295 1
#define HAS_MSM5205 1
#define HAS_MSM5232 1
#define HAS_UPD7759 1
#define HAS_HC55516 1
#define HAS_K005289 1
#define HAS_K007232 1
#define HAS_K051649 1
#define HAS_K053260 1
#define HAS_K054539 1
#define HAS_SEGAPCM 1
#define HAS_RF5C68 1
#define HAS_CEM3394 1
#define HAS_C140 1
#define HAS_QSOUND 1
#define HAS_SAA1099 1
#define HAS_IREMGA20 1
#define HAS_ES5505 1
#define HAS_ES5506 1
#define HAS_BSMT2000 1
#define HAS_YMF262 1
#define HAS_YMF278B 1
#define HAS_GAELCO_CG1V 1
#define HAS_GAELCO_GAE1 1
#define HAS_X1_010 1
#define HAS_MULTIPCM 1
#define HAS_C6280 1
#define HAS_SP0250 1
#define HAS_SCSP 1
#define HAS_YMF271 1
#define HAS_PSXSPU 1

//---------------------------------------------------------------------------

// Turn off some less commonly used defines in tiny builds
#ifdef TINY_COMPILE
#include <vctiny.h>
#endif

#endif		// __VCMAME_H_
