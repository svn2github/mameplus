// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//============================================================

// mamep: Visual C++
#if defined(_MSC_VER) && _MSC_VER >= 1400
#define _CRT_NON_CONFORMING_SWPRINTFS 
#endif

// mamep: windows specific translations
#ifndef _WINDOWS
#define _WINDOWS(str)	lang_message(UI_MSG_OSD0, str)
#endif

#define _WIN32_WINNT 0x0501

#ifdef _MSC_VER
#include <assert.h>
#include <malloc.h>
#if _MSC_VER < 1800
#define alloca _alloca
#define round(x) floor((x) + 0.5)
#endif
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#if _MSC_VER < 1900
#define snprintf _snprintf
#else
#pragma warning (disable: 4091)
#pragma warning (disable: 4267)
#pragma warning (disable: 4456 4457 4458 4459)
#pragma warning (disable: 4463)
#pragma warning (disable: 4838)
#pragma warning (disable: 5025 5026 5027)
#define _CRT_STDIO_LEGACY_WIDE_SPECIFIERS
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca  __builtin_alloca
#endif
#define min(x,y) fmin(x,y)
#define max(x,y) fmax(x,y)
#endif

#define PATH_SEPARATOR      "\\"

#ifdef _MSC_VER
#define snprintf _snprintf

/* Turn off type mismatch warnings */
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
#pragma warning(disable:4799)
#pragma warning(disable:4819)
#endif /* _MSC_VER */
