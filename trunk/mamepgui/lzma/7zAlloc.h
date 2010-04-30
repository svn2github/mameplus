/* 7zAlloc.h -- Allocation functions
2008-10-04 : Igor Pavlov : Public domain */

#ifndef __7Z_ALLOC_H
#define __7Z_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void *SzAlloc(void *p, size_t size);
void SzFree(void *p, void *address);

void *SzAllocTemp(void *p, size_t size);
void SzFreeTemp(void *p, void *address);

#ifdef __cplusplus
}
#endif
	
#endif
