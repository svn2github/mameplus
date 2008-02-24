/***************************************************************************

  M.A.M.E.UI  -  Multiple Arcade Machine Emulator with User Interface
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse,
  Copyright (C) 2003-2007 Chris Kirmse and the MAME32/MAMEUI team.

  This file is part of MAMEUI, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  mui_util.c

 ***************************************************************************/

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#define _UNICODE
#define UNICODE
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

// standard C headers
#include <assert.h>
#include <stdio.h>
#include <tchar.h>

// MAME/MAMEUI headers
#include "mameui.h"	// include this first
#include "unzip.h"
#include "sound/samples.h"
#include "winutf8.h"
#include "strconv.h"
#include "winui.h"
#include "mui_util.h"
#include "translate.h"

#ifdef USE_IPS
#include "mui_opts.h"
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
	BOOL hasOptionalBIOS;
	BOOL isStereo;
	BOOL isVector;
	BOOL usesRoms;
	BOOL usesSamples;
	BOOL usesYM3812;
	BOOL supportsSaveState;
	BOOL hasM68K;
	int numPlayers;
	int numButtons;
	BOOL usesController[CONTROLLER_MAX];
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

	MessageBox(GetActiveWindow(), _Unicode(buf), TEXT_MAMEUINAME, MB_OK | MB_ICONERROR);

	strcpy(buf2, MAMEUINAME ": ");
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
	char 	buf[5000];
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
	HMODULE hModule = GetModuleHandleW(TEXT("comctl32"));

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
	LPCWSTR	  msg = 0;

	hErr = ShellExecute(hWnd, NULL, cName, NULL, NULL, SW_SHOWNORMAL);
	if ((FPTR)hErr > 32) 
	{
		return;
	}

	switch((FPTR)hErr)
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
 
	MessageBox(NULL, msg, cName, MB_OK);
}

LPWSTR MyStrStrI(LPCWSTR pFirst, LPCWSTR pSrch)
{
	int len = wcslen(pSrch);

	while (*pFirst)
	{
		if (_wcsnicmp(pFirst, pSrch, len) == 0)
			return (LPWSTR)pFirst;

		pFirst++;
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

/* Lop off path and extention from a source file name
 * This assumes their is a pathname passed to the function
 * like src\drivers\blah.c
 */
const WCHAR * GetDriverFilename(int nIndex)
{
	const WCHAR *ptmp;

	const WCHAR *filename = driversw[nIndex]->source_file;

	for (ptmp = filename; *ptmp; ptmp++)
	{
		if (*ptmp == '\\')
			filename = ptmp + 1;
		else if (*ptmp == '/')
			filename = ptmp + 1;
	}

	return filename;
}

struct control_cache_t
{
	const input_port_token *ipt;
	int num;
};

static int cmp_ipt(const void *m1, const void *m2)
{
	struct control_cache_t *p1 = (struct control_cache_t *)m1;
	struct control_cache_t *p2 = (struct control_cache_t *)m2;

	return (int)p1->ipt - (int)p2->ipt;
}

static void UpdateController(void)
{
	struct control_cache_t *cache;
	const input_port_token *last_ipt = NULL;
	BOOL flags[CONTROLLER_MAX];
	int nGames = GetNumGames();
	int b = 0;
	int p = 0;
	int i;

	cache = malloc(sizeof (*cache) * nGames);
	if (cache == NULL)
		return;

	for (i = 0; i < nGames; i++)
	{
		cache[i].ipt = drivers[i]->ipt;
		cache[i].num = i;
	}
	qsort(cache, nGames, sizeof (*cache), cmp_ipt);

	for (i = 0; i < nGames; i++)
	{
		struct DriversInfo *gameinfo = &drivers_info[cache[i].num];

		if (!cache[i].ipt)
			continue;

		if (cache[i].ipt != last_ipt)
		{
			const input_port_entry *input;
			int w = CONTROLLER_JOY8WAY;
			BOOL lr = FALSE;
			BOOL ud = FALSE;
			BOOL dual = FALSE;

			last_ipt = cache[i].ipt;
			memset(flags, 0, sizeof flags);
			b = 0;
			p = 0;

			begin_resource_tracking();
			input = input_port_allocate(last_ipt, NULL);

			while (input->type != IPT_END)
			{
				int n;

				if (p < input->player + 1)
					p = input->player + 1;

				n = input->type - IPT_BUTTON1 + 1;
				if (n >= 1 && n <= MAX_NORMAL_BUTTONS && n > b)
				{
					b = n;
					continue;
				}

				switch (input->type)
				{
				case IPT_JOYSTICKRIGHT_LEFT:
				case IPT_JOYSTICKRIGHT_RIGHT:	
				case IPT_JOYSTICKLEFT_LEFT:
				case IPT_JOYSTICKLEFT_RIGHT:
					dual = TRUE;

				case IPT_JOYSTICK_LEFT:
				case IPT_JOYSTICK_RIGHT:
					lr = TRUE;

					if (input->way == 4)
						w = CONTROLLER_JOY4WAY;
					else if (input->way == 16)
						w = CONTROLLER_JOY16WAY;
					break;

				case IPT_JOYSTICKRIGHT_UP:
				case IPT_JOYSTICKRIGHT_DOWN:
				case IPT_JOYSTICKLEFT_UP:
				case IPT_JOYSTICKLEFT_DOWN:
					dual = TRUE;

				case IPT_JOYSTICK_UP:
				case IPT_JOYSTICK_DOWN:
					ud = TRUE;

					if (input->way == 4)
						w = CONTROLLER_JOY4WAY;
					else if (input->way == 16)
						w = CONTROLLER_JOY16WAY;
					break;

				case IPT_PADDLE:
					flags[CONTROLLER_PADDLE] = TRUE;
					break;

				case IPT_DIAL:
					flags[CONTROLLER_DIAL] = TRUE;
					break;

				case IPT_TRACKBALL_X:
				case IPT_TRACKBALL_Y:
					flags[CONTROLLER_TRACKBALL] = TRUE;
					break;

				case IPT_AD_STICK_X:
				case IPT_AD_STICK_Y:
					flags[CONTROLLER_ADSTICK] = TRUE;
					break;

				case IPT_LIGHTGUN_X:
				case IPT_LIGHTGUN_Y:
					flags[CONTROLLER_LIGHTGUN] = TRUE;
					break;
				case IPT_PEDAL:
					flags[CONTROLLER_PEDAL] = TRUE;
					break;
				}
				++input;
			}

			end_resource_tracking();

			if (lr || ud)
			{
				if (lr && !ud)
					w = CONTROLLER_JOY2WAY;
				else if (!lr && ud)
					w = CONTROLLER_VJOY2WAY;

				if (dual)
					w += CONTROLLER_DOUBLEJOY2WAY - CONTROLLER_JOY2WAY;

				flags[w] = TRUE;
			}
		}

		gameinfo->numPlayers = p;
		gameinfo->numButtons = b;

		memcpy(gameinfo->usesController, flags, sizeof gameinfo->usesController);
	}

	free(cache);
}

BOOL isDriverVector(const machine_config *config)
{
	const device_config *screen = video_screen_first(config);

	if (screen != NULL) {
		const screen_config *scrconfig = screen->inline_config;

		/* parse "vector.ini" for vector games */
		if (SCREEN_TYPE_VECTOR == scrconfig->type)
		{
			return TRUE;
		}
	}
	return FALSE;
}

int numberOfSpeakers(const machine_config *config)
{
	int has_sound = FALSE;
	int speakers = 0;
	int sndnum;

	/* see if we have any sound chips to report */
	for (sndnum = 0; sndnum < ARRAY_LENGTH(config->sound); sndnum++)
		if (config->sound[sndnum].type != SOUND_DUMMY)
		{
			has_sound = TRUE;
			break;
		}

	/* if we have sound, count the number of speakers */
	if (has_sound)
		for (speakers = 0; speakers < ARRAY_LENGTH(config->speaker); speakers++)
			if (config->speaker[speakers].tag == NULL)
				break;

	return speakers;
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
			machine_config *config;
			int num_speakers;

			/* Allocate machine config */
			config = machine_config_alloc(gamedrv->drv);

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
			// expand_machine_driver(gamedrv->drv, &drv);

			num_speakers = numberOfSpeakers(config);

			gameinfo->isStereo = (num_speakers > 1);
			gameinfo->isVector = isDriverVector(config); // ((drv.video_attributes & VIDEO_TYPE_VECTOR) != 0);
			gameinfo->usesRoms = FALSE;
			gameinfo->hasOptionalBIOS = FALSE;
			for (region = rom_first_region(gamedrv); region; region = rom_next_region(region))
				for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
				{
					gameinfo->usesRoms = TRUE; 
					gameinfo->hasOptionalBIOS = (determine_bios_rom(get_core_options(), gamedrv->rom) != 0);
					break; 
				}

			gameinfo->usesSamples = FALSE;
			gameinfo->usesYM3812 = FALSE;
			for (i = 0; config->sound[i].type && i < MAX_SOUND; i++)
			{
#if HAS_SAMPLES
				if (config->sound[i].type == SOUND_SAMPLES)
				{
					const char * const * samplenames = NULL;

					samplenames = ((struct Samplesinterface *)config->sound[i].config)->samplenames;

					if (samplenames != NULL && samplenames[0] != NULL)
						gameinfo->usesSamples = TRUE;
				}
#endif
				if (0
#if HAS_YM3812
					|| config->sound[i].type == SOUND_YM3812
#endif
#if HAS_YM3526
					|| config->sound[i].type == SOUND_YM3526
#endif
#if HAS_YM2413
					|| config->sound[i].type == SOUND_YM2413
#endif
				)
					gameinfo->usesYM3812 = TRUE;
			}

			gameinfo->numPlayers = 0;
			gameinfo->numButtons = 0;
			memset(gameinfo->usesController, 0, sizeof gameinfo->usesController);

			gameinfo->hasM68K = FALSE;
			for (i = 0; i < MAX_CPU; i++)
			{
				if (0
#if (HAS_M68000)
					|| config->cpu[i].type == CPU_M68000
#endif
#if (HAS_M68008)
					|| config->cpu[i].type == CPU_M68008
#endif
#if (HAS_M68010)
					|| config->cpu[i].type == CPU_M68010
#endif
#if (HAS_M68EC020)
					|| config->cpu[i].type == CPU_M68EC020
#endif
#if (HAS_M68020)
					|| config->cpu[i].type == CPU_M68020
#endif
#if (HAS_M68040)
					|| config->cpu[i].type == CPU_M68040
#endif
				)
					gameinfo->hasM68K = TRUE;
			}
			/* Free the structure */
			machine_config_free(config);

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
			else if (gameinfo->hasOptionalBIOS)
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

		UpdateController();
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

BOOL DriverIsConsole(int driver_index)
{
	return drivers[driver_index]->sysconfig_ctor != NULL;
}

BOOL DriverIsBios(int driver_index)
{
	BOOL bBios = FALSE;
	if( !( (drivers[driver_index]->flags & GAME_IS_BIOS_ROOT ) == 0)   )
		bBios = TRUE;
	return bBios;
}

BOOL DriverHasOptionalBIOS(int driver_index)
{
	return GetDriversInfo(driver_index)->hasOptionalBIOS;
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

int DriverNumPlayers(int driver_index)
{
	return GetDriversInfo(driver_index)->numPlayers;
}

int DriverNumButtons(int driver_index)
{
	return GetDriversInfo(driver_index)->numButtons;
}

BOOL DriverUsesController(int driver_index, int type)
{
	return GetDriversInfo(driver_index)->usesController[type];
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


//============================================================
//  win_tstring_strdup
//============================================================

TCHAR* win_tstring_strdup(LPCTSTR str)
{
	TCHAR *cpy = NULL;
	if (str != NULL)
	{
		cpy = malloc((_tcslen(str) + 1) * sizeof(TCHAR));
		if (cpy != NULL)
			_tcscpy(cpy, str);
	}
	return cpy;
}


//============================================================
//  win_get_current_directory_utf8
//============================================================

DWORD win_get_current_directory_utf8(DWORD bufferlength, char* buffer)
{
	DWORD result = 0;
	TCHAR* t_buffer = NULL;
	char* utf8_buffer = NULL;
	
	if( bufferlength > 0 ) {
		t_buffer = malloc((bufferlength * sizeof(TCHAR)) + 1);
		if( !t_buffer )
			return result;
	}
	
	result = GetCurrentDirectory(bufferlength, t_buffer);
	
	if( bufferlength > 0 ) {
		utf8_buffer = utf8_from_tstring(t_buffer);
		if( !utf8_buffer ) {
			free(t_buffer);
			return result;
		}
	}

	strncpy(buffer, utf8_buffer, bufferlength);

	if( utf8_buffer )
		free(utf8_buffer);
	
	if( t_buffer )
		free(t_buffer);
	
	return result;
}

//============================================================
//  win_find_first_file_utf8
//============================================================

HANDLE win_find_first_file_utf8(const char* filename, LPWIN32_FIND_DATA findfiledata)
{
	HANDLE result = 0;
	TCHAR* t_filename = tstring_from_utf8(filename);
	if( !t_filename )
		return result;
	
	result = FindFirstFile(t_filename, findfiledata);
	
	free(t_filename);

	return result;
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





