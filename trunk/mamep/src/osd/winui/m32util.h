/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse
 
  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef M32UTIL_H
#define M32UTIL_H

extern void __cdecl ErrorMsg(const char* fmt, ...);
extern void __cdecl dprintf(const char* fmt, ...);
extern void __cdecl dwprintf(const WCHAR* fmt, ...);

extern UINT GetDepth(HWND hWnd);

extern BOOL OnNT(void);

/* Open a text file */
extern void DisplayTextFile(HWND hWnd, const WCHAR *cName);

#define PACKVERSION(major,minor) MAKELONG(minor,major)

/* Check for old version of comctl32.dll */
extern LONG GetCommonControlVersion(void);

extern LPWSTR MyStrStrI(LPCWSTR pStr, LPCWSTR pSrch);
extern char * ConvertToWindowsNewlines(const char *source);
extern const WCHAR * strlower(const WCHAR *s);

extern const WCHAR * GetDriverFilename(int nIndex);
extern const WCHAR * GetFilename(const WCHAR *filename);


BOOL DriverIsClone(int driver_index);
BOOL DriverIsBroken(int driver_index);
BOOL DriverIsHarddisk(int driver_index);
BOOL DriverIsConsole(int driver_index);
BOOL DriverIsStereo(int driver_index);
BOOL DriverIsVector(int driver_index);
BOOL DriverIsBios(int driver_index);
BOOL DriverHasOptionalBios(int driver_index);
int DriverBiosIndex(int driver_index);
int DriverSystemBiosIndex(int driver_index);
BOOL DriverUsesRoms(int driver_index);
BOOL DriverUsesSamples(int driver_index);
BOOL DriverUsesYM3812(int driver_index);
BOOL DriverSupportsSaveState(int driver_index);
BOOL DriverHasM68K(int driver_index);
int DriverParentIndex(int driver_index);

enum
{
		CONTROLLER_JOY2WAY, CONTROLLER_JOY4WAY, CONTROLLER_JOY8WAY, CONTROLLER_JOY16WAY,
		CONTROLLER_VJOY2WAY,
		CONTROLLER_DOUBLEJOY2WAY, CONTROLLER_DOUBLEJOY4WAY, CONTROLLER_DOUBLEJOY8WAY, CONTROLLER_DOUBLEJOY16WAY,
		CONTROLLER_VDOUBLEJOY2WAY,
		CONTROLLER_ADSTICK, CONTROLLER_PADDLE, CONTROLLER_DIAL, CONTROLLER_TRACKBALL, CONTROLLER_LIGHTGUN, CONTROLLER_PEDAL,
		CONTROLLER_MAX
};

int DriverNumPlayers(int driver_index);
int DriverNumButtons(int driver_index);
BOOL DriverUsesController(int driver_index, int type);

#define DriverUsesTrackball(driver_index)	DriverUsesController(driver_index, CONTROLLER_TRACKBALL)
#define DriverUsesLightGun(driver_index)	DriverUsesController(driver_index, CONTROLLER_LIGHTGUN)

#ifdef USE_IPS
int GetPatchCount(const WCHAR *game_name, const WCHAR *patch_name);
int GetPatchFilename(WCHAR *patch_name, const WCHAR *game_name, const int patch_index);
LPWSTR GetPatchDesc(const WCHAR *game_name, const WCHAR *patch_name);
#endif /* USE_IPS */

void FlushFileCaches(void);
void SetCorePathList(int file_type,const char *s);

void FreeIfAllocated(char **s);
void FreeIfAllocatedW(WCHAR **s);

HANDLE win_find_first_file_utf8(const char* filename, LPWIN32_FIND_DATAW findfiledata);

#ifdef TREE_SHEET
void CenterWindow(HWND hWnd);
#endif /* TREE_SHEET */

#endif /* MAME32UTIL_H */
