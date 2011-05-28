//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#define alloca _alloca
#define round(x) floor((x) + 0.5)
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#endif

#ifdef __GNUC__
#ifndef alloca
#define alloca	__builtin_alloca
#endif
#define min(x,y) fmin(x,y)
#define max(x,y) fmax(x,y)
#endif

#define PATH_SEPARATOR		"\\"

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
