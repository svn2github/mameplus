/* 7zDecode.h -- Decoding from 7z folder
2008-11-23 : Igor Pavlov : Public domain */

#ifndef __7Z_DECODE_H
#define __7Z_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "7zItem.h"

SRes SzDecode(const UInt64 *packSizes, const CSzFolder *folder,
    ILookInStream *stream, UInt64 startPos,
    Byte *outBuffer, size_t outSize, ISzAlloc *allocMain);

#ifdef __cplusplus
}
#endif

#endif
