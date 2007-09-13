/***************************************************************************

  M.A.M.E.32	-  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  M32Util.c

 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <assert.h>
#include <stdio.h>

#include "mame32.h"	// include this first
#include "unzip.h"
#include "bitmask.h"
#include "screenshot.h"
#include "sound/samples.h"
#include "m32util.h"
#include "translate.h"

#ifdef USE_IPS
#include "winuiopt.h"
#include "patch.h"
#endif /* USE_IPS */


/***************************************************************************
	function prototypes
 ***************************************************************************/

/***************************************************************************
	External variables
 ***************************************************************************/

/***************************************************************************
	Internal structures
 ***************************************************************************/
static struct DriversInfo
{
	BOOL isClone;
	BOOL isBroken;
	BOOL isHarddisk;
	BOOL hasOptionalBios;
	BOOL isStereo;
	BOOL isVector;
	BOOL usesRoms;
	BOOL usesSamples;
	BOOL usesYM3812;
	BOOL usesTrackball;
	BOOL usesLightGun;
	BOOL supportsSaveState;
	BOOL hasM68K;
	int parentIndex;
	int biosIndex;
} *drivers_info = NULL;


/***************************************************************************
	Internal variables
 ***************************************************************************/
#ifndef PATH_SEPARATOR
#ifdef _MSC_VER
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif
#endif


/***************************************************************************
	External functions
 ***************************************************************************/

/*
	ErrorMsg
*/
void __cdecl ErrorMsg(const char* fmt, ...)
{
	static FILE*	pFile = NULL;
	DWORD			dwWritten;
	char			buf[5000];
	char			buf2[5000];
	va_list 		va;

	va_start(va, fmt);

	vsprintf(buf, fmt, va);

	MessageBox(GetActiveWindow(), _Unicode(buf), TEXT_MAME32NAME, MB_OK | MB_ICONERROR);

	strcpy(buf2, MAME32NAME ": ");
	strcat(buf2,buf);
	strcat(buf2, "\n");

	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), _Unicode(buf2), strlen(buf2), &dwWritten, NULL);

	if (pFile == NULL)
		pFile = fopen("debug.txt", "wt");

	if (pFile != NULL)
	{
		fprintf(pFile, "%s", buf2);
		fflush(pFile);
	}

	va_end(va);
}

void __cdecl dprintf(const char* fmt, ...)
{
	char	buf[5000];
	va_list va;

	va_start(va, fmt);

	_vsnprintf(buf, ARRAY_LENGTH(buf), fmt, va);

	OutputDebugStringA(buf);

	va_end(va);
}

void __cdecl dwprintf(const WCHAR* fmt, ...)
{
	WCHAR	buf[5000];
	va_list va;

	va_start(va, fmt);

	_vsnwprintf(buf, ARRAY_LENGTH(buf), fmt, va);

	OutputDebugStringW(buf);

	va_end(va);
}

UINT GetDepth(HWND hWnd)
{
	UINT	nBPP;
	HDC 	hDC;

	hDC = GetDC(hWnd);
	
	nBPP = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

	ReleaseDC(hWnd, hDC);

	return nBPP;
}

BOOL OnNT(void)
{
	OSVERSIONINFO version_info;
	static int result = -1;

	if (result == -1)
	{
		version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&version_info);
		result = (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT);
	}

	return result;
}

/*
 * Return TRUE if comctl32.dll is version 4.71 or greater
 * otherwise return FALSE.
 */
LONG GetCommonControlVersion()
{
	HMODULE hModule = GetModuleHandleA("comctl32");

	if (hModule)
	{
		FARPROC lpfnICCE = GetProcAddress(hModule, "InitCommonControlsEx");

		if (NULL != lpfnICCE)
		{
			FARPROC lpfnDLLI = GetProcAddress(hModule, "DllInstall");

			if (NULL != lpfnDLLI) 
			{
				/* comctl 4.71 or greater */

				// see if we can find out exactly
				
				DLLGETVERSIONPROC pDllGetVersion;
				pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hModule, "DllGetVersion");

				/* Because some DLLs might not implement this function, you
				   must test for it explicitly. Depending on the particular 
				   DLL, the lack of a DllGetVersion function can be a useful
				   indicator of the version. */

				if(pDllGetVersion)
				{
					DLLVERSIONINFO dvi;
					HRESULT hr;

					ZeroMemory(&dvi, sizeof(dvi));
					dvi.cbSize = sizeof(dvi);

					hr = (*pDllGetVersion)(&dvi);

					if (SUCCEEDED(hr))
					{
						return PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
					}
				}
				return PACKVERSION(4,71);
			}
			return PACKVERSION(4,7);
		}
		return PACKVERSION(4,0);
	}
	/* DLL not found */
	return PACKVERSION(0,0);
}

void DisplayTextFile(HWND hWnd, const WCHAR *cName)
{
	HINSTANCE hErr;
	const WCHAR *msg = 0;

	hErr = ShellExecute(hWnd, NULL, cName, NULL, NULL, SW_SHOWNORMAL);
	if ((int)hErr > 32)
		return;

	switch((int)hErr)
	{
	case 0:
		msg = _UIW(TEXT("The operating system is out of memory or resources."));
		break;

	case ERROR_FILE_NOT_FOUND:
		msg = _UIW(TEXT("The specified file was not found.")); 
		break;

	case SE_ERR_NOASSOC :
		msg = _UIW(TEXT("There is no application associated with the given filename extension."));
		break;

	case SE_ERR_OOM :
		msg = _UIW(TEXT("There was not enough memory to complete the operation."));
		break;

	case SE_ERR_PNF :
		msg = _UIW(TEXT("The specified path was not found."));
		break;

	case SE_ERR_SHARE :
		msg = _UIW(TEXT("A sharing violation occurred."));
		break;

	default:
		msg = _UIW(TEXT("Unknown error."));
	}
 
	MessageBoxW(NULL, msg, cName, MB_OK); 
}

LPWSTR MyStrStrI(LPCWSTR pStr, LPCWSTR pSrch)
{
	int len = wcslen(pSrch);

	while (*pStr)
	{
		if (_wcsnicmp(pStr, pSrch, len) == 0)
			return (LPWSTR)pStr;

		pStr++;
	}

	return NULL;
}

char * ConvertToWindowsNewlines(const char *source)
{
	static char buf[100 * 1024];
	char *dest;

	dest = buf;
	while (*source != 0)
	{
		if (*source == '\n')
		{
			*dest++ = '\r';
			*dest++ = '\n';
		}
		else
			*dest++ = *source;
		source++;
	}
	*dest = 0;
	return buf;
}

const WCHAR * strlower(const WCHAR *s)
{
	static WCHAR buf[100 * 1024];

	wcscpy(buf, s);
	_wcslwr(buf);

	return buf;
}

/* Lop off path and extention from a source file name
 * This assumes their is a pathname passed to the function
 * like src\drivers\blah.c
 */
const WCHAR * GetFilename(const WCHAR *filename)
{
	const WCHAR *ptmp;

	for (ptmp = filename; *ptmp; ptmp++)
	{
		if (*ptmp == '\\')
			filename = ptmp + 1;
		else if (*ptmp == '/')
			filename = ptmp + 1;
	}

	return filename;
}

const WCHAR * GetDriverFilename(int nIndex)
{
	return GetFilename(driversw[nIndex]->source_file);
}

static struct DriversInfo* GetDriversInfo(int driver_index)
{
	if (drivers_info == NULL)
	{
		int ndriver, i;
		drivers_info = malloc(sizeof(struct DriversInfo) * GetNumGames());
		for (ndriver = 0; ndriver < GetNumGames(); ndriver++)
		{
			const game_driver *gamedrv = drivers[ndriver];
			int nParentIndex = GetParentRomSetIndex(gamedrv);
			struct DriversInfo *gameinfo = &drivers_info[ndriver];
			const rom_entry *region, *rom;
			machine_config drv;
			const input_port_entry *input_ports;
			int speakernum, num_speakers;
			gameinfo->isClone = (nParentIndex != -1);
			gameinfo->isBroken = ((gamedrv->flags & GAME_NOT_WORKING) != 0);
			gameinfo->supportsSaveState = ((gamedrv->flags & GAME_SUPPORTS_SAVE) != 0);
			gameinfo->isHarddisk = FALSE;
			for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
				if (ROMREGION_ISDISKDATA(region))
				{
					gameinfo->isHarddisk = TRUE;
					break;
				}
			expand_machine_driver(gamedrv->drv, &drv);

			num_speakers = 0;
			for (speakernum = 0; speakernum < MAX_SPEAKER; speakernum++)
				if (drv.speaker[speakernum].tag != NULL)
					num_speakers++;

			gameinfo->isStereo = (num_speakers > 1);
			gameinfo->isVector = ((drv.video_attributes & VIDEO_TYPE_VECTOR) != 0);
			gameinfo->usesRoms = FALSE;
			gameinfo->hasOptionalBios = FALSE;
			for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					gameinfo->usesRoms = TRUE;
					gameinfo->hasOptionalBios = (determine_bios_rom(get_core_options(), gamedrv->rom) != 0);
					break;
				}

			gameinfo->usesSamples = FALSE;
			gameinfo->usesYM3812 = FALSE;
			for (i = 0; drv.sound[i].sound_type && i < MAX_SOUND; i++)
			{
#if HAS_SAMPLES
				if (drv.sound[i].sound_type == SOUND_SAMPLES)
				{
					const char **samplenames;

					samplenames = ((struct Samplesinterface *)drv.sound[i].config)->samplenames;

					if (samplenames != NULL && samplenames[0] != NULL)
						gameinfo->usesSamples = TRUE;
				}
#endif
				if (0
#if HAS_YM3812
					|| drv.sound[i].sound_type == SOUND_YM3812
#endif
#if HAS_YM3526
					|| drv.sound[i].sound_type == SOUND_YM3526
#endif
#if HAS_YM2413
					|| drv.sound[i].sound_type == SOUND_YM2413
#endif
				)
					gameinfo->usesYM3812 = TRUE;
			}
			gameinfo->usesTrackball = FALSE;
			gameinfo->usesLightGun = FALSE;
			if (gamedrv->ipt != NULL)
			{
				begin_resource_tracking();
				input_ports = input_port_allocate(gamedrv->ipt, NULL);
				while (1)
				{
					UINT32 type;
					type = input_ports->type;
					if (type == IPT_END)
						break;
					if (type == IPT_TRACKBALL_X || type == IPT_TRACKBALL_Y)
						gameinfo->usesTrackball = TRUE;
					if (type == IPT_LIGHTGUN_X || type == IPT_LIGHTGUN_Y)
						gameinfo->usesLightGun = TRUE;
					input_ports++;
				}
				end_resource_tracking();
			}
			gameinfo->hasM68K = FALSE;
			for (i = 0; i < MAX_CPU; i++)
			{
				if (0
#if (HAS_M68000)
					|| drv.cpu[i].cpu_type == CPU_M68000
#endif
#if (HAS_M68008)
					|| drv.cpu[i].cpu_type == CPU_M68008
#endif
#if (HAS_M68010)
					|| drv.cpu[i].cpu_type == CPU_M68010
#endif
#if (HAS_M68EC020)
					|| drv.cpu[i].cpu_type == CPU_M68EC020
#endif
#if (HAS_M68020)
					|| drv.cpu[i].cpu_type == CPU_M68020
#endif
#if (HAS_M68040)
					|| drv.cpu[i].cpu_type == CPU_M68040
#endif
				)
					gameinfo->hasM68K = TRUE;
			}
			gameinfo->parentIndex = -1;
			if (gameinfo->isClone)
			{
				for (i = 0; i < GetNumGames(); i++)
				{
					if (nParentIndex == i)
					{
						gameinfo->parentIndex = i;
						break;
					}
				}
			}
			gameinfo->biosIndex = -1;
			if (DriverIsBios(ndriver))
				gameinfo->biosIndex = ndriver;
			else if (gameinfo->hasOptionalBios)
			{
				int parentIndex;

				if (gameinfo->isClone)
					parentIndex = gameinfo->parentIndex;
				else
					parentIndex = ndriver;

				while (1)
				{
					parentIndex = GetGameNameIndex(drivers[parentIndex]->parent);
					if (parentIndex == -1)
					{
						dprintf("bios for %s is not found", drivers[ndriver]->name);
						break;
					}

					if (DriverIsBios(parentIndex))
					{
						gameinfo->biosIndex = parentIndex;
						break;
					}
				}
			}
		}
	}
	return &drivers_info[driver_index];
}

BOOL DriverIsClone(int driver_index)
{
	return GetDriversInfo(driver_index)->isClone;
}

BOOL DriverIsBroken(int driver_index)
{
	return GetDriversInfo(driver_index)->isBroken;
}

BOOL DriverIsHarddisk(int driver_index)
{
	return GetDriversInfo(driver_index)->isHarddisk;
}

BOOL DriverIsBios(int driver_index)
{
	BOOL bBios = FALSE;
	if( !( (drivers[driver_index]->flags & GAME_IS_BIOS_ROOT ) == 0)   )
		bBios = TRUE;
	return bBios;
}

BOOL DriverHasOptionalBios(int driver_index)
{
	return GetDriversInfo(driver_index)->hasOptionalBios;
}

int DriverBiosIndex(int driver_index)
{
	return GetDriversInfo(driver_index)->biosIndex;
}

int DriverSystemBiosIndex(int driver_index)
{
	int i;

	for (i = 0; i < MAX_SYSTEM_BIOS; i++)
	{
		int bios_driver = GetSystemBiosDriver(i);
		if (bios_driver != -1 && bios_driver == DriverBiosIndex(driver_index))
			return i;
	}

	return -1;
}

BOOL DriverIsStereo(int driver_index)
{
	return GetDriversInfo(driver_index)->isStereo;
}

BOOL DriverIsVector(int driver_index)
{
	return GetDriversInfo(driver_index)->isVector;
}

BOOL DriverUsesRoms(int driver_index)
{
	return GetDriversInfo(driver_index)->usesRoms;
}

BOOL DriverUsesSamples(int driver_index)
{
	return GetDriversInfo(driver_index)->usesSamples;
}

BOOL DriverUsesYM3812(int driver_index)
{
	return GetDriversInfo(driver_index)->usesYM3812;
}

BOOL DriverUsesTrackball(int driver_index)
{
	return GetDriversInfo(driver_index)->usesTrackball;
}

BOOL DriverUsesLightGun(int driver_index)
{
	return GetDriversInfo(driver_index)->usesLightGun;
}

BOOL DriverSupportsSaveState(int driver_index)
{
	return GetDriversInfo(driver_index)->supportsSaveState;
}

BOOL DriverHasM68K(int driver_index)
{
	return GetDriversInfo(driver_index)->hasM68K;
}

int DriverParentIndex(int driver_index)
{
	return GetDriversInfo(driver_index)->parentIndex;
}

#ifdef USE_IPS
int GetPatchCount(const WCHAR *game_name, const WCHAR *patch_name)
{
	int Count = 0;

	if (game_name && patch_name)
	{
		WCHAR szFilename[MAX_PATH];
		WIN32_FIND_DATAW ffd;
		HANDLE hFile;

		swprintf(szFilename, TEXT("%s\\%s\\%s.dat"), GetPatchDir(), game_name, patch_name);
		hFile = FindFirstFileW(szFilename, &ffd);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			int Done = 0;

			while (!Done)
			{
				Count++;
				Done = !FindNextFileW(hFile, &ffd);
			}
			FindClose(hFile);
		}
	}
	return Count;
}

int GetPatchFilename(WCHAR *patch_name, const WCHAR *game_name, const int patch_index)
{
	WIN32_FIND_DATAW ffd;
	HANDLE hFile;
	WCHAR szFilename[MAX_PATH];

	swprintf(szFilename, TEXT("%s\\%s\\*.dat"), GetPatchDir(), game_name);
	hFile = FindFirstFileW(szFilename, &ffd);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		int Done = 0;
		int Count = 0;

		while (!Done )
		{
			if (Count == patch_index)
			{
				wcscpy(patch_name, ffd.cFileName);
				patch_name[wcslen(patch_name) - 4] = '\0';	// To trim the ext ".dat"
				break;
			}
			Count++;
			Done = !FindNextFileW(hFile, &ffd);
		}
		FindClose(hFile);
		return -1;
	}
	return 0;
}

static LPWSTR GetPatchDescByLangcode(FILE *fp, int langcode)
{
	LPWSTR result;
	char *desc = NULL;
	char langtag[8];

	sprintf(langtag, "[%s]", ui_lang_info[langcode].name);

	fseek(fp, 0, SEEK_SET);

	while (!feof(fp))
	{
		char s[4096];

		if (fgets(s, ARRAY_LENGTH(s), fp) != NULL)
		{
			if (strncmp(langtag, s, strlen(langtag)) != 0)
				continue;

			while (fgets(s, ARRAY_LENGTH(s), fp) != NULL)
			{
				char *p;

				if (*s == '[')
				{
					if (desc)
					{
						result = _UTF8Unicode(desc);
						free(desc);
						return result;
					}
					else
						return NULL;
				}

				for (p = s; *p; p++)
					if (*p == '\r' || *p == '\n')
					{
						*p = '\0';
						break;
					}

//				if (*s == '\0')
//					continue;

				if (desc)
				{
					char *p;
					int len = strlen(desc);

					len += strlen(s) + 2;
					p = malloc(len + 1);
					sprintf(p, "%s\r\n%s", desc, s);
					FreeIfAllocated(&desc);
					desc = p;
				}
				else
				{
					desc = mame_strdup(s);
				}
			}
		}
	}

	if (desc)
	{
		result = _UTF8Unicode(desc);
		free(desc);
		return result;
	}
	else
		return NULL;
}

LPWSTR GetPatchDesc(const WCHAR *game_name, const WCHAR *patch_name)
{
	FILE *fp;
	LPWSTR desc = NULL;
	WCHAR szFilename[MAX_PATH];

	swprintf(szFilename, TEXT("%s\\%s\\%s.dat"), GetPatchDir(), game_name, patch_name);

	if ((fp = wfopen(szFilename, TEXT("r"))) != NULL)
	{
		/* Get localized desc */
		desc = GetPatchDescByLangcode(fp, GetLangcode());

		/* Get English desc if localized version is not found */
		if (desc == NULL)
			desc = GetPatchDescByLangcode(fp, UI_LANG_EN_US);

		fclose(fp);
	}

	return desc;
}
#endif /* USE_IPS */


void FlushFileCaches(void)
{
	zip_file_cache_clear();
}

void FreeIfAllocated(char **s)
{
	if (*s)
		free(*s);
	*s = NULL;
}

void FreeIfAllocatedW(WCHAR **s)
{
	if (*s)
		free(*s);
	*s = NULL;
}

#ifdef TREE_SHEET
void CenterWindow(HWND hWnd)
{
	HWND hWndParent;
	RECT rcCenter, rcWnd;
	int iWndWidth, iWndHeight, iScrWidth, iScrHeight, xLeft, yTop;

	hWndParent = GetParent(hWnd);
	GetWindowRect(hWnd, &rcWnd);

	iWndWidth  = rcWnd.right - rcWnd.left;
	iWndHeight = rcWnd.bottom - rcWnd.top;

	if (hWndParent != NULL)
	{
		GetWindowRect(hWndParent, &rcCenter);
	}
	else
	{
		rcCenter.left = 0;
		rcCenter.top = 0;
		rcCenter.right = GetSystemMetrics(SM_CXFULLSCREEN);
		rcCenter.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	}

	iScrWidth  = rcCenter.right - rcCenter.left;
	iScrHeight = rcCenter.bottom - rcCenter.top;

	xLeft = rcCenter.left;
	yTop = rcCenter.top;

	if (iScrWidth > iWndWidth)
		xLeft += ((iScrWidth - iWndWidth) / 2);
	if (iScrHeight > iWndHeight)
		yTop += ((iScrHeight - iWndHeight) / 2);

	// map screen coordinates to child coordinates
	SetWindowPos(hWnd, HWND_TOP, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
#endif /* TREE_SHEET */

/***************************************************************************
	Internal functions
 ***************************************************************************/

