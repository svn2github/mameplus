#ifdef DONT_USE_DLL
#else /* DONT_USE_DLL */
#define SHAREDOBJ_IMPORT
#endif /* DONT_USE_DLL */

#include "osd_so.h"

#ifndef DONT_USE_DLL
#undef main

int main(int argc, char *argv[])
{
	return main_(argc, argv);
}
#endif /* !DONT_USE_DLL */
