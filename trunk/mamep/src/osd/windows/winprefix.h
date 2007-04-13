//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// mamep: for VC2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#define _CRT_NON_CONFORMING_SWPRINTFS 
#endif

#ifdef MALLOC_DEBUG
#include <stdlib.h>

// override malloc to track file/line
void* malloc_file_line(size_t size, const char *file, int line);
void* calloc_file_line(size_t size, size_t count, const char *FILE, int line);
void * realloc_file_line(void *memory, size_t size, const char *file, int line);

#undef malloc
#define malloc(x) malloc_file_line(x, __FILE__, __LINE__)
#undef calloc
#define calloc(x,y) calloc_file_line(x, y, __FILE__, __LINE__)
#undef realloc
#define realloc(x,y) realloc_file_line(x, y, __FILE__, __LINE__)
#endif

#ifdef _MSC_VER
void *__cdecl _alloca(size_t);
#define alloca _alloca
#endif

#ifdef __GNUC__
#define alloca	__builtin_alloca
#endif

#define PATH_SEPARATOR		"\\"

#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#ifdef NO_FORCEINLINE
#undef INLINE

//#define INLINE static __forceinline
#define INLINE static __inline
#endif /* NO_FORCEINLINE */

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
#endif /* _MSC_VER */
