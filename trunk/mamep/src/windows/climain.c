#include <windows.h>
#include <stdio.h>

#ifdef DONT_USE_DLL
#else /* DONT_USE_DLL */
#define SHAREDOBJ_IMPORT
#endif /* DONT_USE_DLL */

#include "osd_so.h"

#ifndef DONT_USE_DLL
int main(int argc, char *argv[])
{
	return utf8_main(argc, argv);
}
#endif /* !DONT_USE_DLL */
