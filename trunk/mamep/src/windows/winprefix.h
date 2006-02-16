//============================================================
//
//  winprefix.h - Win32 prefix file, included by ALL files
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifdef MALLOC_DEBUG
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <malloc.h>
#endif /* _MSC_VER */

// override malloc to track file/line
void* malloc_file_line(size_t size, const char *file, int line);
char* strdup_file_line(const char *s, const char *file, int line);
void* calloc_file_line(size_t size, size_t count, const char *FILE, int line);
void* realloc_file_line(void *memory, size_t size, const char *file, int line);

#undef malloc
#define malloc(x) malloc_file_line(x, __FILE__, __LINE__)
#undef strdup
#define strdup(x) strdup_file_line(x, __FILE__, __LINE__)
#undef calloc
#define calloc(x,y) calloc_file_line(x, y, __FILE__, __LINE__)
#undef realloc
#define realloc(x,y) realloc_file_line(x, y, __FILE__, __LINE__)
#endif

// missing math constants
#define PI				3.1415927
#define M_PI			PI
