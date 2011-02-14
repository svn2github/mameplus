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
#include <windows.h>
#include <shellapi.h>

// standard C headers
#include <assert.h>
#include <stdio.h>
#include <tchar.h>

#include "emu.h"

// MAME/MAMEUI headers
#include "unzip.h"
#include "sound/samples.h"
#include "winutf8.h"
#include "strconv.h"
#include "winui.h"
#include "mui_util.h"
#include "translate.h"

#ifdef USE_IPS
#include "mui_opts.h"
#include "ips.h"
#endif /* USE_IPS */

#include <shlwapi.h>

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
	int screenCount;
	BOOL isVector;
	BOOL usesRoms;
	BOOL usesSamples;
	BOOL usesTrackball;
	BOOL usesLightGun;
	BOOL usesMouse;
	BOOL supportsSaveState;
	BOOL isVertical;
	int numPlayers;
	int numButtons;
	BOOL usesController[CONTROLLER_MAX];
	int parentIndex;
	int biosIndex;
} *drivers_info = NULL;


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

	MessageBox(GetActiveWindow(), _Unicode(buf), TEXT(MAMEUINAME), MB_OK | MB_ICONERROR);

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

void DisplayTextFile(HWND hWnd, const char *cName)
{
	HINSTANCE hErr;
	LPCTSTR	  msg = 0;
	LPTSTR    tName;
	
	tName = tstring_from_utf8(cName);
	if( !tName )
		return;

	hErr = ShellExecute(hWnd, NULL, tName, NULL, NULL, SW_SHOWNORMAL);
	if ((FPTR)hErr > 32) 
	{
		osd_free(tName);
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
 
	MessageBox(NULL, msg, tName, MB_OK);
	
	osd_free(tName);
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
	static char buf[1024 * 1024];
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

BOOL isDriverVector(const machine_config *config)
{
	const screen_device_config *screen  = config->first_screen();

	if (screen != NULL) {
		// parse "vector.ini" for vector games 
		if (SCREEN_TYPE_VECTOR == screen->screen_type())
		{
			return TRUE;
		}
	}
	return FALSE;
}

int numberOfScreens(const machine_config *config)
{
	const screen_device_config *screen  = config->first_screen();
	int i=0;
	for (; screen != NULL; screen = screen->next_screen()) {
		i++;
	}
	return i;
}



struct control_cache_t
{
	input_port_token *ipt;
	int num;
};

static int cmp_ipt(const void *m1, const void *m2)
{
	struct control_cache_t *p1 = (struct control_cache_t *)m1;
	struct control_cache_t *p2 = (struct control_cache_t *)m2;

	return p1->ipt - p2->ipt;
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

	cache = (control_cache_t *)malloc(sizeof (*cache) * nGames);
	if (cache == NULL)
		return;

	for (i = 0; i < nGames; i++)
	{
		cache[i].ipt = (input_port_token *)drivers[i]->ipt;
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
			const input_port_config *port;
			ioport_list portlist;

			int w = CONTROLLER_JOY8WAY;
			BOOL lr = FALSE;
			BOOL ud = FALSE;
			BOOL dual = FALSE;

			last_ipt = cache[i].ipt;
			memset(flags, 0, sizeof flags);
			b = 0;
			p = 0;

			input_port_list_init(portlist, last_ipt, NULL, 0, FALSE);

			for (port = portlist.first(); port != NULL; port = port->next())
			{
				const input_field_config *field;
				for (field = port->fieldlist; field != NULL; field = field->next)
				{
				    int n;
    
				    if (p < field->player + 1)
					    p = field->player + 1;
    
				    n = field->type - IPT_BUTTON1 + 1;
				    if (n >= 1 && n <= MAX_NORMAL_BUTTONS && n > b)
				    {
					    b = n;
					    continue;
				    }
    
				    switch (field->type)
				    {
				    case IPT_JOYSTICKRIGHT_LEFT:
				    case IPT_JOYSTICKRIGHT_RIGHT:	
				    case IPT_JOYSTICKLEFT_LEFT:
				    case IPT_JOYSTICKLEFT_RIGHT:
					    dual = TRUE;
    
				    case IPT_JOYSTICK_LEFT:
				    case IPT_JOYSTICK_RIGHT:
					    lr = TRUE;
    
					    if (field->way == 4)
						    w = CONTROLLER_JOY4WAY;
					    else if (field->way == 16)
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
    
					    if (field->way == 4)
						    w = CONTROLLER_JOY4WAY;
					    else if (field->way == 16)
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
				}
			}
			//input_port_list_deinit(&portlist);

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

int numberOfSpeakers(const machine_config *config)
{
	return config->m_devicelist.count(SPEAKER);
}

static struct DriversInfo* GetDriversInfo(int driver_index)
{
	if (drivers_info == NULL)
	{
		int ndriver;
		drivers_info = (DriversInfo*)malloc(sizeof(struct DriversInfo) * GetNumGames());
		for (ndriver = 0; ndriver < GetNumGames(); ndriver++)
		{
			const game_driver *gamedrv = drivers[ndriver];
			struct DriversInfo *gameinfo = &drivers_info[ndriver];
			const rom_entry *region, *rom;
			machine_config config(*gamedrv);
			const rom_source *source;
			int num_speakers;

			gameinfo->isClone = (GetParentRomSetIndex(gamedrv) != -1);
			gameinfo->isBroken = ((gamedrv->flags & GAME_NOT_WORKING) != 0);
			gameinfo->supportsSaveState = ((gamedrv->flags & GAME_SUPPORTS_SAVE) != 0);
			gameinfo->isHarddisk = FALSE;
			gameinfo->isVertical = (gamedrv->flags & ORIENTATION_SWAP_XY) ? TRUE : FALSE;
			for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
			{
				for (region = rom_first_region(*source); region; region = rom_next_region(region))
				{
					if (ROMREGION_ISDISKDATA(region))
						gameinfo->isHarddisk = TRUE;
				}
			}
			gameinfo->hasOptionalBIOS = FALSE;
			if (gamedrv->rom != NULL)
			{
				for (rom = gamedrv->rom; !ROMENTRY_ISEND(rom); rom++)
				{
					if (ROMENTRY_ISSYSTEM_BIOS(rom))
					{
						gameinfo->hasOptionalBIOS = TRUE;
						break;
					}
				}
			}

			num_speakers = numberOfSpeakers(&config);

			gameinfo->isStereo = (num_speakers > 1);
			gameinfo->screenCount = numberOfScreens(&config);
			gameinfo->isVector = isDriverVector(&config); // ((drv.video_attributes & VIDEO_TYPE_VECTOR) != 0);
			gameinfo->usesRoms = FALSE;
			for (source = rom_first_source(config); source != NULL; source = rom_next_source(*source))
			{
				for (region = rom_first_region(*source); region; region = rom_next_region(region))
				{
					for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
					{
						gameinfo->usesRoms = TRUE; 
						break; 
					}
				}
			}
			gameinfo->usesSamples = FALSE;
			
			{
				const device_config_sound_interface *sound = NULL;
				const char * const * samplenames = NULL;
				for (bool gotone = config.m_devicelist.first(sound); gotone; gotone = sound->next(sound)) {
					if (sound->devconfig().type() == SAMPLES)
					{
						const samples_interface *intf = (const samples_interface *)sound->devconfig().static_config();
						samplenames = intf->samplenames;

						if (samplenames != 0 && samplenames[0] != 0)
						{
							gameinfo->usesSamples = TRUE;
							break;
						}			
					}				
				}
			}
			gameinfo->numPlayers = 0;
			gameinfo->numButtons = 0;
			memset(gameinfo->usesController, 0, sizeof gameinfo->usesController);

			gameinfo->parentIndex = -1;
			if (gameinfo->isClone)
			{
				int i;

				for (i = 0; i < GetNumGames(); i++)
				{
					if (GetParentRomSetIndex(gamedrv) == i)
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

			gameinfo->usesTrackball = FALSE;
			gameinfo->usesLightGun = FALSE;
			if (gamedrv->ipt != NULL)
			{
				const input_port_config *port;
				ioport_list portlist;
				
				input_port_list_init(portlist, gamedrv->ipt, NULL, 0, FALSE);
				
				for (port = portlist.first(); port != NULL; port = port->next())
				{
					const input_field_config *field;
					for (field = port->fieldlist; field != NULL; field = field->next)
 					{
						UINT32 type;
						type = field->type;
						if (type == IPT_END)
							break;
						if (type == IPT_DIAL || type == IPT_PADDLE || 
							type == IPT_TRACKBALL_X || type == IPT_TRACKBALL_Y ||
							type == IPT_AD_STICK_X || type == IPT_AD_STICK_Y)
							gameinfo->usesTrackball = TRUE;
						if (type == IPT_LIGHTGUN_X || type == IPT_LIGHTGUN_Y)
							gameinfo->usesLightGun = TRUE;
						if (type == IPT_MOUSE_X || type == IPT_MOUSE_Y)
							gameinfo->usesMouse = TRUE;
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

BOOL DriverIsBios(int driver_index)
{
	BOOL bBios = FALSE;
	if( !( (drivers[driver_index]->flags & GAME_IS_BIOS_ROOT ) == 0)   )
		bBios = TRUE;
	return bBios;
}

BOOL DriverIsMechanical(int driver_index)
{
	BOOL bMechanical = FALSE;
	if( !( (drivers[driver_index]->flags & GAME_MECHANICAL ) == 0)   )
		bMechanical = TRUE;
	return bMechanical;
}

BOOL DriverHasOptionalBIOS(int driver_index)
{
	return GetDriversInfo(driver_index)->hasOptionalBIOS;
}

BOOL DriverIsStereo(int driver_index)
{
	return GetDriversInfo(driver_index)->isStereo;
}

int DriverNumScreens(int driver_index)
{
	return GetDriversInfo(driver_index)->screenCount;
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

BOOL DriverUsesTrackball(int driver_index)
{
	return GetDriversInfo(driver_index)->usesTrackball;
}

BOOL DriverUsesLightGun(int driver_index)
{
	return GetDriversInfo(driver_index)->usesLightGun;
}

BOOL DriverUsesMouse(int driver_index)
{
	return GetDriversInfo(driver_index)->usesMouse;
}

BOOL DriverSupportsSaveState(int driver_index)
{
	return GetDriversInfo(driver_index)->supportsSaveState;
}

BOOL DriverIsVertical(int driver_index) {
	return GetDriversInfo(driver_index)->isVertical; 
}

BOOL DriverIsConsole(int driver_index)
{
#ifdef MAMEMESS
	return drivers[driver_index]->flags & (GAME_TYPE_CONSOLE|GAME_TYPE_COMPUTER);
#else /* MAMEMESS */
	return FALSE;
#endif /* MAMEMESS */
}

int DriverBiosIndex(int driver_index)
{
	return GetDriversInfo(driver_index)->biosIndex;
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

void FlushFileCaches(void)
{
	zip_file_cache_clear();
}

BOOL StringIsSuffixedBy(const char *s, const char *suffix)
{
	return (strlen(s) > strlen(suffix)) && (strcmp(s + strlen(s) - strlen(suffix), suffix) == 0);
}

void FreeIfAllocated(char **s)
{
	if (*s)
		osd_free(*s);
	*s = NULL;
}

void FreeIfAllocatedW(WCHAR **s)
{
	if (*s)
		osd_free(*s);
	*s = NULL;
}

/***************************************************************************
	Win32 wrappers
 ***************************************************************************/

BOOL SafeIsAppThemed(void)
{
	BOOL bResult = FALSE;
	HMODULE hThemes;
	BOOL (WINAPI *pfnIsAppThemed)(void);
	
	hThemes = LoadLibrary(TEXT("uxtheme.dll"));
	if (hThemes != NULL)
	{
		pfnIsAppThemed = (BOOL (WINAPI *)(void)) GetProcAddress(hThemes, "IsAppThemed");
		if (pfnIsAppThemed != NULL)
			bResult = pfnIsAppThemed();
		FreeLibrary(hThemes);
	}
	return bResult;

}


void GetSystemErrorMessage(DWORD dwErrorId, TCHAR **tErrorMessage)
{
	if( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorId, 0, (LPTSTR)tErrorMessage, 0, NULL) == 0 )
	{
		*tErrorMessage = (LPTSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(TCHAR));
		_tcscpy(*tErrorMessage, TEXT("Unknown Error"));
	}
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

		swprintf(szFilename, TEXT("%s\\%s\\%s.dat"), GetIPSDir(), game_name, patch_name);
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

	swprintf(szFilename, TEXT("%s\\%s\\*.dat"), GetIPSDir(), game_name);
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
						osd_free(desc);
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
					p = (char *)malloc(len + 1);
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
		osd_free(desc);
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

	swprintf(szFilename, TEXT("%s\\%s\\%s.dat"), GetIPSDir(), game_name, patch_name);

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
//  win_extract_icon_utf8
//============================================================

HICON win_extract_icon_utf8(HINSTANCE inst, const char* exefilename, UINT iconindex)
{
	HICON icon = 0;
	TCHAR* t_exefilename = tstring_from_utf8(exefilename);
	if( !t_exefilename )
		return icon;
	
	icon = ExtractIcon(inst, t_exefilename, iconindex);
	
	osd_free(t_exefilename);
	
	return icon;
}



//============================================================
//  win_tstring_strdup
//============================================================

TCHAR* win_tstring_strdup(LPCTSTR str)
{
	TCHAR *cpy = NULL;
	if (str != NULL)
	{
		cpy = (TCHAR*)osd_malloc((_tcslen(str) + 1) * sizeof(TCHAR));
		if (cpy != NULL)
			_tcscpy(cpy, str);
	}
	return cpy;
}

//============================================================
//  win_create_file_utf8
//============================================================

HANDLE win_create_file_utf8(const char* filename, DWORD desiredmode, DWORD sharemode, 
					   		LPSECURITY_ATTRIBUTES securityattributes, DWORD creationdisposition,
					   		DWORD flagsandattributes, HANDLE templatehandle)
{
	HANDLE result = 0;
	TCHAR* t_filename = tstring_from_utf8(filename);
	if( !t_filename )
		return result;
	
	result = CreateFile(t_filename, desiredmode, sharemode, securityattributes, creationdisposition,
						flagsandattributes, templatehandle);

	osd_free(t_filename);
						
	return result;
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
		t_buffer = (TCHAR*)malloc((bufferlength * sizeof(TCHAR)) + 1);
		if( !t_buffer )
			return result;
	}
	
	result = GetCurrentDirectory(bufferlength, t_buffer);
	
	if( bufferlength > 0 ) {
		utf8_buffer = utf8_from_tstring(t_buffer);
		if( !utf8_buffer ) {
			osd_free(t_buffer);
			return result;
		}
	}

	strncpy(buffer, utf8_buffer, bufferlength);

	if( utf8_buffer )
		osd_free(utf8_buffer);
	
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
	
	osd_free(t_filename);

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

