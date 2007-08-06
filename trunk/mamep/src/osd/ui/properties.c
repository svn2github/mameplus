//mamep: mame32 v113u3
/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/
 
/***************************************************************************

  Properties.c

    Properties Popup and Misc UI support routines.
    
    Created 8/29/98 by Mike Haaland (mhaaland@hypertech.com)

***************************************************************************/
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#define NONAMELESSUNION 1
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#undef NONAMELESSUNION
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "MAME32.h"	// include this first
#include "driver.h"
#ifdef USE_SCALE_EFFECTS
#include "osdscale.h"
#endif /* USE_SCALE_EFFECTS */
#include "info.h"
#include "audit.h"
#include "audit32.h"
#include "bitmask.h"
#include "winuiopt.h"
#include "file.h"
#include "resource.h"
#include "DIJoystick.h"     /* For DIJoystick avalibility. */
#include "m32util.h"
#include "directdraw.h"
#include "properties.h"
#include "treeview.h"
#include "translate.h"

#include "screenshot.h"
#include "DataMap.h"
#include "help.h"
#include "resource.hm"
#include "datafile.h"

typedef HANDLE HTHEME;

#ifdef UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifdef MESS
// done like this until I figure out a better idea
#include "ui/resourcems.h"
#include "ui/propertiesms.h"
#endif

// missing win32 api defines
#ifndef TBCD_TICS
#define TBCD_TICS 1
#endif
#ifndef TBCD_THUMB
#define TBCD_THUMB 2
#endif
#ifndef TBCD_CHANNEL
#define TBCD_CHANNEL 3
#endif

#ifndef DISPLAY_DEVICE_MIRRORING_DRIVER
#define DISPLAY_DEVICE_MIRRORING_DRIVER    0x00000008
#endif

#define FOLDER_OPTIONS	-2
#define GLOBAL_OPTIONS	-1

#define IS_GLOBAL	(g_nGame == GLOBAL_OPTIONS)
#define IS_FOLDER	(g_pFolder)
#define IS_GAME		(g_nGame > -1)

#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef  PropSheet_GetTabControl
#define PropSheet_GetTabControl(d) (HWND)(LRESULT)(int)SendMessage((d),PSM_GETTABCONTROL,0,0)
#endif /* defined(__GNUC__) */

/***************************************************************
 * Imported function prototypes
 ***************************************************************/

/**************************************************************
 * Local function prototypes
 **************************************************************/

static int CALLBACK PropSheetCallbackProc(HWND hDlg, UINT Msg, LPARAM lParam);
static void SetStereoEnabled(HWND hWnd, int nIndex);
static void SetYM3812Enabled(HWND hWnd, int nIndex);
static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled);
static void InitializeOptions(HWND hDlg);
static void InitializeMisc(HWND hDlg);
static void OptOnHScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos);
static void BeamSelectionChange(HWND hwnd);
static void NumScreensSelectionChange(HWND hwnd);
static void FlickerSelectionChange(HWND hwnd);
static void PrescaleSelectionChange(HWND hwnd);
static void GammaSelectionChange(HWND hwnd);
static void BrightCorrectSelectionChange(HWND hwnd);
static void ContrastSelectionChange(HWND hwnd);
static void PauseBrightSelectionChange(HWND hwnd);
static void FullScreenGammaSelectionChange(HWND hwnd);
static void FullScreenBrightnessSelectionChange(HWND hwnd);
static void FullScreenContrastSelectionChange(HWND hwnd);
static void JDZSelectionChange(HWND hwnd);
static void JSATSelectionChange(HWND hwnd);
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl);
static void VolumeSelectionChange(HWND hwnd);
static void AudioLatencySelectionChange(HWND hwnd);
static void ThreadPrioritySelectionChange(HWND hwnd);
static void UpdateDisplayModeUI(HWND hwnd, DWORD dwRefresh);
static void InitializeDisplayModeUI(HWND hwnd);
static void InitializeSoundUI(HWND hwnd);
static void InitializeSkippingUI(HWND hwnd);
static void InitializeRotateUI(HWND hwnd);
static void UpdateScreenUI(HWND hwnd);
static void InitializeScreenUI(HWND hwnd);
static void UpdateSelectScreenUI(HWND hwnd);
static void InitializeSelectScreenUI(HWND hwnd);
static void InitializeD3DVersionUI(HWND hwnd);
static void InitializeVideoUI(HWND hwnd);
static void InitializeViewUI(HWND hwnd);
static void InitializeRefreshUI(HWND hwnd);
static void UpdateRefreshUI(HWND hwnd);
static void InitializeDefaultInputUI(HWND hWnd);
static void InitializeEffectUI(HWND hWnd);
static void InitializeBIOSUI(HWND hwnd);
static void InitializeDefaultBIOSUI(HWND hwnd);
static void InitializeControllerMappingUI(HWND hwnd);
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
static void InitializeM68kCoreUI(HWND hwnd);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef USE_SCALE_EFFECTS
static void InitializeScaleEffectUI(HWND hwnd);
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
static void InitializeJoyidUI(HWND hWnd);
#endif /* JOYSTICK_ID */
#ifdef TRANS_UI
static void TransparencySelectionChange(HWND hwnd);
#endif /* TRANS_UI */
#ifdef TREE_SHEET
static  void MovePropertySheetChildWindows(HWND hWnd, int nDx, int nDy);
static  HTREEITEM GetSheetPageTreeItem(int nPage);
static  int GetSheetPageTreeCurSelText(LPWSTR lpszText, int iBufSize);
#endif /* TREE_SHEET */
static void PropToOptions(HWND hWnd, options_type *o);
static void OptionsToProp(HWND hWnd, options_type *o);
static void SetPropEnabledControls(HWND hWnd);

static void BuildDataMap(void);
static void ResetDataMap(void);

static BOOL IsControlOptionValue(HWND hDlg,HWND hwnd_ctrl, options_type *opts);

static void UpdateBackgroundBrush(HWND hwndTab);
HBRUSH hBkBrush;
BOOL bThemeActive;

/**************************************************************
 * Local private variables
 **************************************************************/

static options_type  origGameOpts;
static BOOL orig_uses_defaults;
static options_type* pGameOpts = NULL;
static int g_biosinfo = -1;
static int  default_bios_index[MAX_SYSTEM_BIOS];
#ifdef JOYSTICK_ID
static int g_nJoyid[8];
#endif /* JOYSTICK_ID */
static char *g_sMonitorDeviceString[MAX_SCREENS + 2];
static char *g_sMonitorDeviceName[MAX_SCREENS + 2];

static int  g_nGame            = 0;
static const WCHAR *g_pFolder  = NULL;
static int  g_nPropertyMode    = 0;
static BOOL g_bInternalSet     = FALSE;
static BOOL g_bUseDefaults     = FALSE;
static BOOL g_bReset           = FALSE;
static int  g_nSampleRateIndex = 0;
static int  g_nVolumeIndex     = 0;
static int  g_nPriorityIndex   = 0;
static int  g_nGammaIndex      = 0;
static int  g_nContrastIndex   = 0;
static int  g_nBrightIndex = 0;
static int  g_nPauseBrightIndex = 0;
static int  g_nBeamIndex       = 0;
static int  g_nFlickerIndex    = 0;
static int  g_nRotateIndex     = 0;
static int  g_nScreenIndex     = 0;
static int  g_nViewIndex     = 0;
static int  g_nSelectScreenIndex     = 0;
static int  g_nInputIndex      = 0;
static int  g_nPrescaleIndex   = 0;
static int  g_nFullScreenGammaIndex = 0;
static int  g_nFullScreenBrightnessIndex = 0;
static int  g_nFullScreenContrastIndex = 0;
static int  g_nEffectIndex     = 0;
static int  g_nBiosIndex       = 0;
static int  g_nJDZIndex        = 0;
static int  g_nJSATIndex        = 0;
static int  g_nPaddleIndex = 0;
static int  g_nADStickIndex = 0;
static int  g_nPedalIndex = 0;
static int  g_nDialIndex = 0;
static int  g_nTrackballIndex = 0;
static int  g_nLightgunIndex = 0;
static int  g_nPositionalIndex = 0;
static int  g_nMouseIndex = 0;
static int  g_nVideoIndex = 0;
static int  g_nD3DVersionIndex = 0;
static BOOL  g_bAutoAspect[MAX_SCREENS] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
#ifdef USE_SCALE_EFFECTS
static int  g_nScaleEffectIndex= 0;
#endif /* USE_SCALE_EFFECTS */
#ifdef TRANS_UI
static int  g_nUITransparencyIndex = 0;
#endif /* TRANS_UI */

#ifdef TREE_SHEET
#define SHEET_TREE_WIDTH 180
static int g_nFirstInitPropertySheet = 0;
static RECT rcTabCtrl;
static RECT rcTabCaption;
static RECT rcSheetSnap;
static HBITMAP hSheetBitmap = NULL;
static BOOL bUseScreenShot = FALSE;
static int nCaptionHeight;
static HWND hSheetTreeCtrl = NULL;
static HINSTANCE hSheetInstance = 0;
static WNDPROC pfnOldSheetProc = NULL;
static  BOOL bPageTreeSelChangedActive = FALSE;
#endif /* TREE_SHEET */

static HICON g_hIcon = NULL;

/* Property sheets */

#define HIGHLIGHT_COLOR RGB(0,196,0)
HBRUSH highlight_brush = NULL;
HBRUSH background_brush = NULL;
#define VECTOR_COLOR RGB( 190, 0, 0) //DARK RED
#define FOLDER_COLOR RGB( 0, 128, 0 ) // DARK GREEN
#define PARENT_COLOR RGB( 190, 128, 192 ) // PURPLE
#define GAME_COLOR RGB( 0, 128, 192 ) // DARK BLUE

BOOL PropSheetFilter_Vector(void)
{
	if (IS_GLOBAL)
	{
#if 1
		int i;

		for (i = 0; drivers[i]; i++)
			if (DriverIsVector(i))
				return 1;
#endif

		return 0;
	}

	if (IS_FOLDER)
	{
		if (g_nPropertyMode == SOURCE_VECTOR)
			return 1;

		return FolderHasVector(g_pFolder);
	}

	return DriverIsVector(g_nGame);
}

BOOL PropSheetFilter_BIOS(void)
{
#ifdef DRIVER_SWITCH
	// driver switch config is in bios page
	return IS_GLOBAL;
#else /* DRIVER_SWITCH */
	if (IS_GLOBAL)
		return (GetSystemBiosInfo(0) != NULL);

	return 0;
#endif /* DRIVER_SWITCH */
}

/* Help IDs */
static DWORD dwHelpIDs[] =
{

	IDC_JDZ,				HIDC_JDZ,
	IDC_ANTIALIAS,          HIDC_ANTIALIAS,
	IDC_ARTWORK_CROP,		HIDC_ARTWORK_CROP,
	IDC_ASPECTRATIOD,       HIDC_ASPECTRATIOD,
	IDC_ASPECTRATION,       HIDC_ASPECTRATION,
	IDC_AUTOFRAMESKIP,      HIDC_AUTOFRAMESKIP,
	IDC_BACKDROPS,			HIDC_BACKDROPS,
	IDC_BEAM,               HIDC_BEAM,
	IDC_BEZELS,				HIDC_BEZELS,
	IDC_FSGAMMA,		    HIDC_FSGAMMA,
	IDC_BRIGHTCORRECT,      HIDC_BRIGHTCORRECT,
	IDC_BROADCAST,			HIDC_BROADCAST,
	IDC_RANDOM_BG,          HIDC_RANDOM_BG,
	IDC_CHEAT,              HIDC_CHEAT,
	IDC_DEFAULT_INPUT,      HIDC_DEFAULT_INPUT,
	IDC_FILTER_CLONES,      HIDC_FILTER_CLONES,
	IDC_FILTER_EDIT,        HIDC_FILTER_EDIT,
	IDC_FILTER_NONWORKING,  HIDC_FILTER_NONWORKING,
	IDC_FILTER_ORIGINALS,   HIDC_FILTER_ORIGINALS,
	IDC_FILTER_RASTER,      HIDC_FILTER_RASTER,
	IDC_FILTER_UNAVAILABLE, HIDC_FILTER_UNAVAILABLE,
	IDC_FILTER_VECTOR,      HIDC_FILTER_VECTOR,
	IDC_FILTER_WORKING,     HIDC_FILTER_WORKING,
	IDC_FLICKER,            HIDC_FLICKER,
	IDC_FLIPX,              HIDC_FLIPX,
	IDC_FLIPY,              HIDC_FLIPY,
	IDC_FRAMESKIP,          HIDC_FRAMESKIP,
	IDC_GAMMA,              HIDC_GAMMA,
	IDC_HISTORY,            HIDC_HISTORY,
	IDC_HWSTRETCH,          HIDC_HWSTRETCH,
	IDC_JOYSTICK,           HIDC_JOYSTICK,
	IDC_KEEPASPECT,         HIDC_KEEPASPECT,
//	IDC_LANGUAGECHECK,      HIDC_LANGUAGECHECK,
//	IDC_LANGUAGEEDIT,       HIDC_LANGUAGEEDIT,
	IDC_LOG,                HIDC_LOG,
	IDC_SLEEP,				HIDC_SLEEP,
	IDC_MAXIMIZE,           HIDC_MAXIMIZE,
	IDC_OVERLAYS,			HIDC_OVERLAYS,
	IDC_PROP_RESET,         HIDC_PROP_RESET,
	IDC_REFRESH,            HIDC_REFRESH,
	IDC_RESET_DEFAULT,      HIDC_RESET_DEFAULT,
	IDC_RESET_FILTERS,      HIDC_RESET_FILTERS,
	IDC_RESET_GAMES,        HIDC_RESET_GAMES,
	IDC_RESET_UI,           HIDC_RESET_UI,
	IDC_ROTATE,             HIDC_ROTATE,
	IDC_SAMPLERATE,         HIDC_SAMPLERATE,
	IDC_SAMPLES,            HIDC_SAMPLES,
	IDC_SIZES,              HIDC_SIZES,
	IDC_START_GAME_CHECK,   HIDC_START_GAME_CHECK,
	IDC_SWITCHRES,          HIDC_SWITCHRES,
	IDC_SYNCREFRESH,        HIDC_SYNCREFRESH,
	IDC_THROTTLE,           HIDC_THROTTLE,
	IDC_TRIPLE_BUFFER,      HIDC_TRIPLE_BUFFER,
	IDC_USE_DEFAULT,        HIDC_USE_DEFAULT,
	IDC_USE_MOUSE,          HIDC_USE_MOUSE,
	IDC_USE_SOUND,          HIDC_USE_SOUND,
	IDC_VOLUME,             HIDC_VOLUME,
	IDC_WAITVSYNC,          HIDC_WAITVSYNC,
	IDC_WINDOWED,           HIDC_WINDOWED,
	IDC_PAUSEBRIGHT,        HIDC_PAUSEBRIGHT,
	IDC_LIGHTGUN,           HIDC_LIGHTGUN,
	IDC_DUAL_LIGHTGUN,      HIDC_DUAL_LIGHTGUN,
	IDC_RELOAD,             HIDC_RELOAD,
	IDC_STEADYKEY,          HIDC_STEADYKEY,
	IDC_JOY_GUI,            HIDC_JOY_GUI,
	IDC_RANDOM_BG,          HIDC_RANDOM_BG,
	IDC_SKIP_GAME_INFO,     HIDC_SKIP_GAME_INFO,
	IDC_HIGH_PRIORITY,      HIDC_HIGH_PRIORITY,
	IDC_D3D_FILTER,         HIDC_D3D_FILTER,
	IDC_AUDIO_LATENCY,      HIDC_AUDIO_LATENCY,
	IDC_BIOS,               HIDC_BIOS,
	IDC_STRETCH_SCREENSHOT_LARGER, HIDC_STRETCH_SCREENSHOT_LARGER,
	IDC_SCREEN,             HIDC_SCREEN,
	IDC_PADDLE,				HIDC_PADDLE,
	IDC_ADSTICK,			HIDC_ADSTICK,
	IDC_PEDAL,				HIDC_PEDAL,
	IDC_DIAL,				HIDC_DIAL,
	IDC_TRACKBALL,			HIDC_TRACKBALL,
	IDC_LIGHTGUNDEVICE,		HIDC_LIGHTGUNDEVICE,
	IDC_ENABLE_AUTOSAVE,    HIDC_ENABLE_AUTOSAVE,
	IDC_MULTITHREAD_RENDERING,    HIDC_MULTITHREAD_RENDERING,
	IDC_JSAT,				HIDC_JSAT,
	0,                      0
};

static struct ComboBoxVideo
{
	const WCHAR*	m_pText;
	const char*	m_pData;
} g_ComboBoxVideo[] = 
{
	{ TEXT("GDI"),                  "gdi"    },
	{ TEXT("DirectDraw"),           "ddraw"  },
	{ TEXT("Direct3D"),             "d3d"    },
};
#define NUMVIDEO ARRAY_LENGTH(g_ComboBoxVideo)

static struct ComboBoxD3DVersion
{
	const WCHAR*	m_pText;
	const int	m_pData;
} g_ComboBoxD3DVersion[] = 
{
	{ TEXT("Version 9"),           9   },
	{ TEXT("Version 8"),           8   },
};

#define NUMD3DVERSIONS ARRAY_LENGTH(g_ComboBoxD3DVersion)

static struct ComboBoxSelectScreen
{
	const WCHAR*	m_pText;
	const int	m_pData;
} g_ComboBoxSelectScreen[] = 
{
	{ TEXT("First screen"),         0    },
	{ TEXT("Second screen"),        1    },
	{ TEXT("Third screen"),         2    },
	{ TEXT("Fourth screen"),        3    },
};
#define NUMSELECTSCREEN ARRAY_LENGTH(g_ComboBoxSelectScreen)

static struct ComboBoxView
{
	const WCHAR*	m_pText;
	const char*	m_pData;
} g_ComboBoxView[] = 
{
	{ TEXT("Auto"),		          "auto"        },
	{ TEXT("Standard"),         "standard"    }, 
	{ TEXT("Pixel Aspect"),     "pixel"       }, 
	{ TEXT("Cocktail"),         "cocktail"    },
};
#define NUMVIEW ARRAY_LENGTH(g_ComboBoxView)

static struct ComboBoxDevices
{
	const WCHAR*	m_pText;
	const char*	m_pData;
} g_ComboBoxDevice[] = 
{
	{ TEXT("Keyboard"),              "keyboard"  },
	{ TEXT("Mouse"),                 "mouse"     },
	{ TEXT("Joystick"),              "joystick"  },
	{ TEXT("Lightgun"),              "lightgun"  },
};

#define NUMDEVICES ARRAY_LENGTH(g_ComboBoxDevice)

#ifdef DRIVER_SWITCH
static const struct
{
	const char *name;
	const UINT ctrl;
} drivers_table[] =
{
	{"mame",        IDC_DRV_MAME},
	{"plus",        IDC_DRV_PLUS},
	{"homebrew",    IDC_DRV_HOMEBREW},
	{"neod",        IDC_DRV_NEOD},
#ifndef NEOCPSMAME
	{"noncpu",      IDC_DRV_NONCPU},
	{"hazemd",      IDC_DRV_HAZEMD},
#endif /* NEOCPSMAME */
	{0}
};
#endif /* DRIVER_SWITCH */

/***************************************************************
 * Public functions
 ***************************************************************/

typedef HTHEME (WINAPI *OpenThemeProc)(HWND hwnd, LPCWSTR pszClassList);

HMODULE hThemes;
OpenThemeProc fnOpenTheme;
FARPROC fnIsThemed;

void PropertiesInit(void)
{
	hThemes = LoadLibraryA("uxtheme.dll");

	if (hThemes)
	{
		fnIsThemed = GetProcAddress(hThemes,"IsAppThemed");
	}
	bThemeActive = FALSE;

	// mamep: enumerate all monitors on start up
	{
		DISPLAY_DEVICEA dd;
		int iMonitors;
		int i;

		iMonitors = DirectDraw_GetNumDisplays(); // this gets the count of monitors attached
		if (iMonitors > MAX_SCREENS)
			iMonitors = MAX_SCREENS;

		ZeroMemory(&dd, sizeof(dd));
		dd.cb = sizeof(dd);

		g_sMonitorDeviceString[0] = NULL;
		g_sMonitorDeviceName[0] = strdup("auto");

		for (i = 0; i < iMonitors; i++)
		{
			g_sMonitorDeviceString[i + 1] = strdup(DirectDraw_GetDisplayName(i));
			g_sMonitorDeviceName[i + 1] = strdup(DirectDraw_GetDisplayDriver(i));
		}

		g_sMonitorDeviceString[i + 1] = NULL;
		g_sMonitorDeviceName[i + 1] = NULL;
	}
}

DWORD GetHelpIDs(void)
{
	return (DWORD) (LPSTR) dwHelpIDs;
}

static PROPSHEETPAGE *CreatePropSheetPages(HINSTANCE hInst, BOOL bOnlyDefault,
	UINT *pnMaxPropSheets )
{
	PROPSHEETPAGE *pspages;
	int maxPropSheets;
	int possiblePropSheets;
	int i;

	if (IS_FOLDER)
		i = 2;
	else
		i = 0;

	for (; g_propSheets[i].pfnDlgProc; i++)
		;

	if (IS_FOLDER)
		possiblePropSheets = i - 1;
	else
		possiblePropSheets = i + 1;

	pspages = malloc(sizeof(PROPSHEETPAGE) * possiblePropSheets);
	if (!pspages)
		return NULL;
	memset(pspages, 0, sizeof(PROPSHEETPAGE) * possiblePropSheets);

	maxPropSheets = 0;
	if (IS_FOLDER)
		i = 2;
	else
		i = 0;

	for (; g_propSheets[i].pfnDlgProc; i++)
	{
		if (!bOnlyDefault || g_propSheets[i].bOnDefaultPage)
		{
			if (!g_propSheets[i].pfnFilterProc || g_propSheets[i].pfnFilterProc())
			{
				pspages[maxPropSheets].dwSize                     = sizeof(PROPSHEETPAGE);
				pspages[maxPropSheets].dwFlags                    = 0;
				pspages[maxPropSheets].hInstance                  = hInst;
				pspages[maxPropSheets].DUMMYUNIONNAME.pszTemplate = MAKEINTRESOURCE(g_propSheets[i].dwDlgID);
				pspages[maxPropSheets].pfnCallback                = NULL;
				pspages[maxPropSheets].lParam                     = 0;
				pspages[maxPropSheets].pfnDlgProc                 = g_propSheets[i].pfnDlgProc;
				maxPropSheets++;
			}
		}
	}
	
	if (pnMaxPropSheets)
		*pnMaxPropSheets = maxPropSheets;

	return pspages;
}

void InitDefaultPropertyPage(HINSTANCE hInst, HWND hWnd)
{
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE   *pspage;

	g_nGame = GLOBAL_OPTIONS;
	g_pFolder = NULL;
	g_nPropertyMode = SOURCE_GLOBAL;

	/* Get default options to populate property sheets */
	pGameOpts = GetDefaultOptions();
	g_bUseDefaults = FALSE;
	/* Stash the result for comparing later */
	FreeGameOptions(&origGameOpts);
	CopyGameOptions(pGameOpts,&origGameOpts);
	orig_uses_defaults = FALSE;
	g_bReset = FALSE;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(pshead));

	pspage = CreatePropSheetPages(hInst, TRUE, &pshead.nPages);
	if (!pspage)
		return;

	/* Fill in the property sheet header */
	pshead.hwndParent                 = hWnd;
	pshead.dwSize                     = sizeof(PROPSHEETHEADER);
	pshead.dwFlags                    = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	pshead.pfnCallback                = PropSheetCallbackProc;
	pshead.hInstance                  = hInst;
	pshead.pszCaption                 = _UIW(TEXT("Default Game"));
	pshead.DUMMYUNIONNAME2.nStartPage = 0;
	pshead.DUMMYUNIONNAME.pszIcon     = MAKEINTRESOURCE(IDI_MAME32_ICON);
	pshead.DUMMYUNIONNAME3.ppsp       = pspage;

#ifdef TREE_SHEET
	if (GetShowTreeSheet())
	{
		g_nFirstInitPropertySheet = 1;
		hSheetInstance = hInst;
	}
#endif /* TREE_SHEET */

	/* Create the Property sheet and display it */
	if (PropertySheet(&pshead) == -1)
	{
		WCHAR temp[100];
		DWORD dwError = GetLastError();

		swprintf(temp, _UIW(TEXT("Propery Sheet Error %d %X")), (int)dwError, (int)dwError);
		MessageBox(0, temp, _UIW(TEXT("Error")), IDOK);
	}

	free(pspage);
}

void InitPropertyPage(HINSTANCE hInst, HWND hWnd, int game_num, HICON hIcon, const WCHAR *folder)
{
	InitPropertyPageToPage(hInst, hWnd, game_num, hIcon, PROPERTIES_PAGE, folder);
}

void InitPropertyPageToPage(HINSTANCE hInst, HWND hWnd, int game_num, HICON hIcon, int start_page, const WCHAR *folder)
{
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE   *pspage;

	if (highlight_brush == NULL)
		highlight_brush = CreateSolidBrush(HIGHLIGHT_COLOR);

	if (background_brush == NULL)
		background_brush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	g_hIcon = CopyIcon(hIcon);
	g_pFolder = folder;
	InitGameAudit(game_num);

	if (IS_FOLDER)
		g_nGame = FOLDER_OPTIONS;
	else
		g_nGame = game_num;

	if (IS_GAME)
	{
		pGameOpts = GetGameOptions(game_num);
		g_bUseDefaults = GetGameUsesDefaults(game_num);
		/* Stash the result for comparing later */
		FreeGameOptions(&origGameOpts);
		CopyGameOptions(pGameOpts,&origGameOpts);
		g_nPropertyMode = SOURCE_GAME;
	}
	else
	{
		pGameOpts = GetFolderOptions(g_pFolder);
		g_bUseDefaults = GetFolderUsesDefaults(g_pFolder);
		if (!wcscmp(g_pFolder, TEXT("Vector")))
			g_nPropertyMode = SOURCE_VECTOR;
		else
			g_nPropertyMode = SOURCE_FOLDER;
		/* Stash the result for comparing later */
		FreeGameOptions(&origGameOpts);
		CopyGameOptions(pGameOpts,&origGameOpts);
	}
	orig_uses_defaults = g_bUseDefaults;
	g_bReset = FALSE;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(PROPSHEETHEADER));

	pspage = CreatePropSheetPages(hInst, FALSE, &pshead.nPages);
	if (!pspage)
		return;

	/* Fill in the property sheet header */
	pshead.hwndParent                 = hWnd;
	pshead.dwSize                     = sizeof(PROPSHEETHEADER);
	pshead.dwFlags                    = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	pshead.pfnCallback                = PropSheetCallbackProc;
	pshead.hInstance                  = hInst;
	if (folder)
	//if (IS_FOLDER)
		pshead.pszCaption             = _UIW(folder);
	else
		pshead.pszCaption             = driversw[g_nGame]->name;
	pshead.DUMMYUNIONNAME2.nStartPage = start_page;
	pshead.DUMMYUNIONNAME.pszIcon     = MAKEINTRESOURCE(IDI_MAME32_ICON);
	pshead.DUMMYUNIONNAME3.ppsp       = pspage;

#ifdef TREE_SHEET
	if (GetShowTreeSheet())
	{
		g_nFirstInitPropertySheet = 1;
		hSheetInstance = hInst;
	}
#endif /* TREE_SHEET */

	/* Create the Property sheet and display it */
	if (PropertySheet(&pshead) == -1)
	{
		WCHAR temp[100];
		DWORD dwError = GetLastError();
		swprintf(temp, _UIW(TEXT("Propery Sheet Error %d %X")), (int)dwError, (int)dwError);
		MessageBox(0, temp, _UIW(TEXT("Error")), IDOK);
	}

	free(pspage);
}

/*********************************************************************
 * Local Functions
 *********************************************************************/

/* Build CPU info string */
static LPCWSTR GameInfoCPU(int nIndex)
{
	int i;
	static WCHAR buf[1024];
	machine_config drv;
	expand_machine_driver(drivers[nIndex]->drv, &drv);

	buf[0] = '\0';

	i = 0;
	while (i < MAX_CPU && drv.cpu[i].cpu_type)
	{
		if (drv.cpu[i].cpu_clock >= 1000000)
			swprintf(&buf[wcslen(buf)], TEXT("%s %d.%06d MHz"),
					_Unicode(cputype_name(drv.cpu[i].cpu_type)),
					drv.cpu[i].cpu_clock / 1000000,
					drv.cpu[i].cpu_clock % 1000000);
		else
			swprintf(&buf[wcslen(buf)], TEXT("%s %d.%03d kHz"),
					_Unicode(cputype_name(drv.cpu[i].cpu_type)),
					drv.cpu[i].cpu_clock / 1000,
					drv.cpu[i].cpu_clock % 1000);

		wcscat(buf, TEXT("\n"));

		i++;
	}

	return buf;
}

/* Build Sound system info string */
static LPCWSTR GameInfoSound(int nIndex)
{
	int i;
	static WCHAR buf[1024];
	machine_config drv;
	expand_machine_driver(drivers[nIndex]->drv,&drv);

	buf[0] = '\0';

	i = 0;
	while (i < MAX_SOUND && drv.sound[i].sound_type)
	{
		int clock,sound_type,count;

		sound_type = drv.sound[i].sound_type;
		clock = drv.sound[i].clock;

		count = 1;
		i++;

		while (i < MAX_SOUND
				&& drv.sound[i].sound_type == sound_type
				&& drv.sound[i].clock == clock)
		{
			count++;
			i++;
		}

		if (count > 1)
			swprintf(&buf[wcslen(buf)], TEXT("%dx"), count);

		wcscpy(&buf[wcslen(buf)], _Unicode(sndtype_name(sound_type)));

		if (clock)
		{
			if (clock >= 1000000)
				swprintf(&buf[wcslen(buf)], TEXT(" %d.%06d MHz"),
						clock / 1000000,
						clock % 1000000);
			else
				swprintf(&buf[wcslen(buf)], TEXT(" %d.%03d kHz"),
						clock / 1000,
						clock % 1000);
		}

		wcscat(buf, TEXT("\n"));
	}

	return buf;
}

/* Build Display info string */
static LPCWSTR GameInfoScreen(int nIndex)
{
	static WCHAR buf[1024];
	machine_config drv;

	expand_machine_driver(drivers[nIndex]->drv, &drv);

	if (drv.video_attributes & VIDEO_TYPE_VECTOR)
	{
		if (drivers[nIndex]->flags & ORIENTATION_SWAP_XY)
		{
			swprintf(buf, _UIW(TEXT("Vector (V) %f Hz (%d colors)")),
				drv.screen[0].defstate.refresh, drv.total_colors);
		}
		else
		{
			swprintf(buf, _UIW(TEXT("Vector (H) %f Hz (%d colors)")),
				drv.screen[0].defstate.refresh, drv.total_colors);
		}
	}
	else
	{
		if (drivers[nIndex]->flags & ORIENTATION_SWAP_XY)
		{
			swprintf(buf, _UIW(TEXT("%d x %d (V) %f Hz (%d colors)")),
				drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1,
				drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1,
				SUBSECONDS_TO_HZ(drv.screen[0].defstate.refresh), drv.total_colors);
		}
		else
		{
			swprintf(buf, _UIW(TEXT("%d x %d (H) %f Hz (%d colors)")),
				drv.screen[0].defstate.visarea.max_x - drv.screen[0].defstate.visarea.min_x + 1,
				drv.screen[0].defstate.visarea.max_y - drv.screen[0].defstate.visarea.min_y + 1,
				SUBSECONDS_TO_HZ(drv.screen[0].defstate.refresh), drv.total_colors);
		}
	}
	return buf;
}

#ifdef MISC_FOLDER
/* Build input information string */
static LPCWSTR GameInfoInput(int nIndex)
{
	static WCHAR buf[1024];
	const input_port_entry* input;
	int nplayer = 0;
	const WCHAR *control = 0;
	int nbutton = 0;
#if 0 // no space
	int ncoin = 0;
	const WCHAR *service = 0;
	const WCHAR *tilt = 0;
#endif // no space

	begin_resource_tracking();

	input = input_port_allocate(drivers[nIndex]->ipt, NULL);

	while (input->type != IPT_END)
	{
		if (nplayer < input->player+1)
			nplayer = input->player+1;

		switch (input->type)
		{
			case IPT_JOYSTICK_LEFT:
			case IPT_JOYSTICK_RIGHT:

				/* if control not defined, start it off as horizontal 2-way */
				if (!control)
					control = _UIW(TEXT("Joystick 2-Way"));
				else if (wcscmp(control, _UIW(TEXT("Joystick 2-Way"))) == 0)
					;
				/* if already defined as vertical, make it 4 or 8 way */
				else if (wcscmp(control, _UIW(TEXT("Joystick 2-Way Vertical"))) == 0)
				{
					if (input->way == 4)
						control = _UIW(TEXT("Joystick 4-Way"));
					else
					{
						if (input->way == 16)
							control = _UIW(TEXT("Joystick 16-Way"));
						else
							control = _UIW(TEXT("Joystick 8-Way"));
					}
				}
				break;

			case IPT_JOYSTICK_UP:
			case IPT_JOYSTICK_DOWN:

				/* if control not defined, start it off as vertical 2-way */
				if (!control)
					control = _UIW(TEXT("Joystick 2-Way Vertical"));
				else if (wcscmp(control, _UIW(TEXT("Joystick 2-Way Vertical"))) == 0)
					;
				/* if already defined as horiz, make it 4 or 8way */
				else if (wcscmp(control, _UIW(TEXT("Joystick 2-Way")))==0)
				{
					if (input->way == 4)
						control = _UIW(TEXT("Joystick 4-Way"));
					else
					{
						if (input->way == 16)
							control = _UIW(TEXT("Joystick 16-Way"));
						else
							control = _UIW(TEXT("Joystick 8-Way"));
					}
				}
				break;

			case IPT_JOYSTICKRIGHT_UP:
			case IPT_JOYSTICKRIGHT_DOWN:
			case IPT_JOYSTICKLEFT_UP:
			case IPT_JOYSTICKLEFT_DOWN:

				/* if control not defined, start it off as vertical 2way */
				if (!control)
					control = _UIW(TEXT("Double Joystick 2-Way Vertical"));
				else if (wcscmp(control, _UIW(TEXT("Double Joystick 2-Way Vertical"))) == 0)
					;
				/* if already defined as horiz, make it 4 or 8 way */
				else if (wcscmp(control, _UIW(TEXT("Double Joystick 2-Way"))) == 0)
				{
					if (input->way == 4)
						control = _UIW(TEXT("Double Joystick 4-Way"));
					else
					{
						if (input->way == 16)
							control = _UIW(TEXT("Double Joystick 16-Way"));
						else
							control = _UIW(TEXT("Double Joystick 8-Way"));
					}
				}
				break;

			case IPT_JOYSTICKRIGHT_LEFT:
			case IPT_JOYSTICKRIGHT_RIGHT:
			case IPT_JOYSTICKLEFT_LEFT:
			case IPT_JOYSTICKLEFT_RIGHT:

				/* if control not defined, start it off as horiz 2-way */
				if (!control)
					control = _UIW(TEXT("Double Joystick 2-Way"));
				else if (wcscmp(control, _UIW(TEXT("Double Joystick 2-Way"))) == 0)
					;
				/* if already defined as vertical, make it 4 or 8 way */
				else if (wcscmp(control, _UIW(TEXT("Double Joystick 2-Way Vertical"))) == 0)
				{
					if (input->way == 4)
						control = _UIW(TEXT("Double Joystick 4-Way"));
					else
					{
						if (input->way == 16)
							control = _UIW(TEXT("Double Joystick 16-Way"));
						else
							control = _UIW(TEXT("Double Joystick 8-Way"));
					}
				}
				break;

			case IPT_BUTTON1:
				if (nbutton<1) nbutton = 1;
				break;
			case IPT_BUTTON2:
				if (nbutton<2) nbutton = 2;
				break;
			case IPT_BUTTON3:
				if (nbutton<3) nbutton = 3;
				break;
			case IPT_BUTTON4:
				if (nbutton<4) nbutton = 4;
				break;
			case IPT_BUTTON5:
				if (nbutton<5) nbutton = 5;
				break;
			case IPT_BUTTON6:
				if (nbutton<6) nbutton = 6;
				break;
			case IPT_BUTTON7:
				if (nbutton<7) nbutton = 7;
				break;
			case IPT_BUTTON8:
				if (nbutton<8) nbutton = 8;
				break;
			case IPT_BUTTON9:
				if (nbutton<9) nbutton = 9;
				break;
			case IPT_BUTTON10:
				if (nbutton<10) nbutton = 10;
				break;

			case IPT_PADDLE:
				control = _UIW(TEXT("Paddle"));
				break;
			case IPT_DIAL:
				control = _UIW(TEXT("Dial"));
				break;
			case IPT_TRACKBALL_X:
			case IPT_TRACKBALL_Y:
				control = _UIW(TEXT("Trackball"));
				break;
			case IPT_AD_STICK_X:
			case IPT_AD_STICK_Y:
				control = _UIW(TEXT("AD Stick"));
				break;
			case IPT_LIGHTGUN_X:
			case IPT_LIGHTGUN_Y:
				control = _UIW(TEXT("Lightgun"));
				break;
#if 0 // no space
			case IPT_COIN1:
				if (ncoin < 1) ncoin = 1;
				break;
			case IPT_COIN2:
				if (ncoin < 2) ncoin = 2;
				break;
			case IPT_COIN3:
				if (ncoin < 3) ncoin = 3;
				break;
			case IPT_COIN4:
				if (ncoin < 4) ncoin = 4;
				break;
			case IPT_COIN5:
				if (ncoin < 5) ncoin = 5;
				break;
			case IPT_COIN6:
				if (ncoin < 6) ncoin = 6;
				break;
			case IPT_COIN7:
				if (ncoin < 7) ncoin = 7;
				break;
			case IPT_COIN8:
				if (ncoin < 8) ncoin = 8;
				break;
			case IPT_SERVICE :
				service = "yes";
				break;
			case IPT_TILT :
				tilt = "yes";
				break;
#endif // no space
		}
		++input;
	}

	end_resource_tracking();

	if (control == NULL) control = TEXT("");

	if (nplayer<1)
		wcscpy(buf, _UIW(TEXT("Unknown")));
	else
	if ((nbutton<1) && (nplayer>1))
		swprintf(buf, _UIW(TEXT("%s (%d players)")), control, nplayer);
	else
	if (nbutton<1)
		swprintf(buf, _UIW(TEXT("%s (%d player)")), control, nplayer);
	else
	if ((nplayer>1) && (nbutton>1))
		swprintf(buf, _UIW(TEXT("%s (%d players, %d buttons)")), control, nplayer, nbutton);
	else
	if (nplayer>1)
		swprintf(buf, _UIW(TEXT("%s (%d players, %d button)")), control, nplayer, nbutton);
	else
	if (nbutton>1)
		swprintf(buf, _UIW(TEXT("%s (%d player, %d buttons)")), control, nplayer, nbutton);
	else
		swprintf(buf, _UIW(TEXT("%s (%d player, %d button)")), control, nplayer, nbutton);

	return buf;
}
#else /* MISC_FOLDER */
/* Build color information string */
static LPCWSTR GameInfoColors(int nIndex)
{
	static WCHAR buf[1024];
	machine_config drv;
	expand_machine_driver(drivers[nIndex]->drv, &drv);

	ZeroMemory(buf, sizeof(buf));
	swprintf(buf, _UIW(TEXT("%d colors ")), drv.total_colors);

	return buf;
}
#endif /* !MISC_FOLDER */

/* Build game status string */
LPWSTR GameInfoStatus(int driver_index, BOOL bRomStatus)
{
	static WCHAR buffer[1024];
	int audit_result = GetRomAuditResults(driver_index);

	buffer[0] = '\0';

	if (bRomStatus && IsAuditResultKnown(audit_result) == FALSE)
	{
		wcscpy(buffer, _UIW(TEXT("Unknown")));
	}

	else if (!bRomStatus || IsAuditResultYes(audit_result))
	{
		if (DriverIsBroken(driver_index))
			wcscpy(buffer, _UIW(TEXT("Not working")));
		else
			wcscpy(buffer, _UIW(TEXT("Working")));

		//the Flags are checked in the order of "noticability"
		//1) visible deficiencies
		//2) audible deficiencies
		//3) other deficiencies
		if (drivers[driver_index]->flags & GAME_UNEMULATED_PROTECTION)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Game protection isn't fully emulated")));
		}
		if (drivers[driver_index]->flags & GAME_WRONG_COLORS)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Colors are completely wrong")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_COLORS)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Colors aren't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_GRAPHICS)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Video emulation isn't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_NO_SOUND)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Game lacks sound")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_SOUND)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Sound emulation isn't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_NO_COCKTAIL)
		{
			wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Screen flipping is not supported")));
		}
	}
	else
	{
			// audit result is no
#ifdef MESS
		return _UIW(TEXT("BIOS missing"));
#else
		return _UIW(TEXT("ROMs missing"));
#endif
	}

	return buffer;
}

/* Build game manufacturer string */
static LPCWSTR GameInfoManufactured(int nIndex)
{
	static WCHAR buffer[1024];

	snwprintf(buffer, ARRAY_LENGTH(buffer), TEXT("%s %s"), driversw[nIndex]->year, UseLangList()? _MANUFACTW(driversw[nIndex]->manufacturer) : driversw[nIndex]->manufacturer);
	return buffer;
}

/* Build Game title string */
LPWSTR GameInfoTitle(int nIndex)
{
	const WCHAR *folder = g_pFolder;
	static WCHAR desc[1024];
	static WCHAR info[1024];

	if (nIndex == GLOBAL_OPTIONS)
		return _UIW(TEXT("Global game options\nDefault options used by all games"));

	if (g_nPropertyMode == SOURCE_VECTOR)
		return _UIW(TEXT("Global vector options\nCustom options used by all games in the Vector"));

	if (nIndex != FOLDER_OPTIONS)
	{
		swprintf(desc, TEXT("%s [%s]"),
		        UseLangList() ? _LSTW(driversw[nIndex]->description) :
		                        driversw[nIndex]->modify_the,
			driversw[nIndex]->name);

		folder = GetUnifiedFolder(nIndex);
		if (!folder)
			return desc;
	}

	if (nIndex != FOLDER_OPTIONS)
		swprintf(info, _UIW(TEXT("%s\nThis is also global driver options in the %s")), desc, folder);
	else
		swprintf(info, _UIW(TEXT("Global driver options\nCustom options used by all games in the %s")), folder);

	return info;
}

/* Build game clone infromation string */
static LPCWSTR GameInfoCloneOf(int nIndex)
{
	static WCHAR buf[1024];
	int nParentIndex= -1;

	buf[0] = '\0';

	if (DriverIsClone(nIndex))
	{
		if ((nParentIndex = GetParentIndex(drivers[nIndex])) >= 0)
			swprintf(buf, TEXT("%s [%s]"),
					ConvertAmpersandString(UseLangList()?
						_LSTW(driversw[nParentIndex]->description):
						driversw[nParentIndex]->modify_the),
					driversw[nParentIndex]->name);
	}

	return buf;
}

static LPCWSTR GameInfoSaveState(int driver_index)
{
	if (drivers[driver_index]->flags & GAME_SUPPORTS_SAVE)
		return _UIW(TEXT("Supported"));

	return _UIW(TEXT("Unsupported"));
}

static LPCWSTR GameInfoSource(int nIndex)
{
	return GetDriverFilename(nIndex);
}

#ifdef TREE_SHEET
static void UpdateSheetCaption(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC         hDC;
	HRGN        hRgn;
	HBRUSH      hBrush;
	RECT        rect, rc;
	int         i, iWidth;
	BYTE        bR, bG, bB, bSR, bSG, bSB, bER, bEG, bEB;
	DWORD       dwLColor, dwRColor;
	WCHAR       szText[256];

	// Gradation color
	dwLColor = GetSysColor(COLOR_ACTIVECAPTION);
	dwRColor = GetSysColor(COLOR_GRADIENTACTIVECAPTION);
	bSR = GetRValue(dwLColor); bSG = GetGValue(dwLColor); bSB = GetBValue(dwLColor);
	bER = GetRValue(dwRColor); bEG = GetGValue(dwRColor); bEB = GetBValue(dwRColor);

	memcpy(&rect, &rcTabCaption, sizeof(RECT));

	iWidth = rect.right - rect.left;
	if (iWidth == 0)
		return;

	BeginPaint (hWnd, &ps);
	hDC = ps.hdc;

	hRgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
	SelectClipRgn(hDC, hRgn);

	rc.left = rect.left;
	rc.top = rect.top;
	rc.right = rect.left + 1;
	rc.bottom = rect.bottom;

	for (i = 0; i < iWidth; i++)
	{
		bR = bSR + ((bER - bSR) * i) / iWidth;
		bG = bSG + ((bEG - bSG) * i) / iWidth;
		bB = bSB + ((bEB - bSB) * i) / iWidth;

		hBrush = CreateSolidBrush(RGB(bR,bG,bB));

		FillRect(hDC, &rc, hBrush);
		DeleteObject(hBrush);

		rc.left++;
		rc.right++;
	}

	i = GetSheetPageTreeCurSelText(szText, ARRAY_LENGTH(szText));
	if (i > 0)
	{
		HFONT hFontCaption, hOldFont;

		hFontCaption = CreateFont(14, 0,				// height, width
								0, 						// angle of escapement
								0,						// base-line orientation angle
								FW_BOLD,				// font weight
								0, 0, 0, 				// italic, underline, strikeout
								DEFAULT_CHARSET,		// character set identifier
								OUT_DEFAULT_PRECIS,		// output precision
								CLIP_DEFAULT_PRECIS,	// clipping precision
								ANTIALIASED_QUALITY,	// output quality
								FF_DONTCARE,			// pitch and family
								(LPTSTR)TEXT("Tahoma"));		// typeface name

		if (hFontCaption)
		{
			hOldFont = (HFONT)SelectObject(hDC, hFontCaption);

			SetTextColor(hDC, GetSysColor(COLOR_CAPTIONTEXT));
			SetBkMode(hDC, TRANSPARENT);

			memcpy(&rc, &rect, sizeof(RECT));
			rc.left += 5;

			DrawText(hDC, (LPCTSTR)szText, wcslen((LPTSTR)szText), &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

			SelectObject(hDC, hOldFont);
			DeleteObject(hFontCaption);
		}
	}

	SelectClipRgn(hDC, NULL);
	DeleteObject(hRgn);

	memcpy(&rect, &rcSheetSnap, sizeof(RECT));
	// Snapshot region
	hRgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
	SelectClipRgn(hDC, hRgn);

	if (hSheetBitmap != NULL) 
	{
		HDC hMemDC;
		HBITMAP hOldBitmap;
		int iWidth, iHeight, iSnapWidth, iSnapHeight, iDrawWidth, iDrawHeight;

		if (bUseScreenShot == TRUE)
		{
			iSnapWidth = GetScreenShotWidth();
			iSnapHeight = GetScreenShotHeight();
		}
		else
		{
			BITMAP bmpInfo;

			GetObject(hSheetBitmap, sizeof(BITMAP), &bmpInfo);
			iSnapWidth = bmpInfo.bmWidth;
			iSnapHeight = bmpInfo.bmHeight;
		}

		iWidth = rect.right - rect.left;
		iHeight = rect.bottom - rect.top;

		if (iWidth && iHeight)
		{
			int iXOffs, iYOffs;
			double dXRatio, dYRatio;

			dXRatio = (double)iWidth  / (double)iSnapWidth;
			dYRatio = (double)iHeight / (double)iSnapHeight;

			if (dXRatio > dYRatio)
			{
				iDrawWidth = (int)((iSnapWidth * dYRatio) + 0.5);
				iDrawHeight = (int)((iSnapHeight * dYRatio) + 0.5);
			}
			else
			{
				iDrawWidth = (int)((iSnapWidth * dXRatio) + 0.5);
				iDrawHeight = (int)((iSnapHeight * dXRatio) + 0.5);
			}

			iXOffs = (iWidth - iDrawWidth)  / 2;
			iYOffs = (iHeight - iDrawHeight) / 2;

			hMemDC = CreateCompatibleDC(hDC);

			hOldBitmap = SelectObject(hMemDC, hSheetBitmap);
	
			SetStretchBltMode(hDC, STRETCH_HALFTONE);
			StretchBlt(hDC,
					rect.left+iXOffs, rect.top+iYOffs,
					iDrawWidth, iDrawHeight,
					hMemDC, 0, 0,
					iSnapWidth, iSnapHeight, SRCCOPY);

			SelectObject(hMemDC, hOldBitmap);
			DeleteDC(hMemDC);
		}
	}
	else
	{
		hBrush = CreateSolidBrush(RGB(220,220,220));
		FillRect(hDC, &rect, hBrush);
		DeleteObject(hBrush);
	}

	SelectClipRgn(hDC, NULL);
	DeleteObject(hRgn);

	EndPaint (hWnd, &ps);

	return;
}

static LRESULT CALLBACK NewSheetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;

	switch (Msg)
	{
	case WM_PAINT:
		UpdateSheetCaption(hWnd);
		bHandled = TRUE;
		break;

	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->code)
		{
		case TVN_SELCHANGINGA:
		case TVN_SELCHANGINGW:
			if ((bPageTreeSelChangedActive == FALSE) && (g_nFirstInitPropertySheet == 0))
			{
				int nPage;
				TVITEM item;
				NMTREEVIEW* pTvn = (NMTREEVIEW*)lParam;

				bPageTreeSelChangedActive = TRUE;
				item.hItem = pTvn->itemNew.hItem;
				item.mask = TVIF_PARAM;
				SendMessage(hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);

				nPage = (int)item.lParam;
				if (nPage >= 0)
				{
					SendMessage(hWnd, PSM_SETCURSEL, nPage, 0);
				}

				bPageTreeSelChangedActive = FALSE;
				bHandled = TRUE;
			}
			break;
		case TVN_SELCHANGEDA:
		case TVN_SELCHANGEDW:
			InvalidateRect(hWnd, &rcTabCaption, FALSE);
			bHandled = TRUE;
			break;
		}
		break;

	case WM_DESTROY:
		if (hSheetTreeCtrl != NULL)
		{
			DestroyWindow(hSheetTreeCtrl);
			hSheetTreeCtrl = NULL;
		}

		if (hSheetBitmap != NULL)
		{
			if (bUseScreenShot == FALSE)
				DeleteObject(hSheetBitmap);
			hSheetBitmap = NULL;
		}
		bUseScreenShot = FALSE;

		if (pfnOldSheetProc)
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnOldSheetProc);
		break;
	}

	if ((bHandled == FALSE) && pfnOldSheetProc)
		return CallWindowProc(pfnOldSheetProc, hWnd, Msg, wParam, lParam);

	return 0;
}

static void AdjustChildWindows(HWND hWnd)
{
	WCHAR szClass[128];
	DWORD dwStyle;

	GetClassName(hWnd, szClass, ARRAY_LENGTH(szClass));
	if (!wcscmp(szClass, TEXT("Button")))
	{
		dwStyle = GetWindowLong(hWnd, GWL_STYLE);
		if (((dwStyle & BS_GROUPBOX) == BS_GROUPBOX) && (dwStyle & WS_TABSTOP))
		{
			SetWindowLong(hWnd, GWL_STYLE, (dwStyle & ~WS_TABSTOP));
		}
	}
}

static void AdjustPropertySheetChildWindows(HWND hWnd)
{
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	while (hChild)
	{
		hChild = GetNextWindow(hChild, GW_HWNDNEXT);
	}
}

static void MovePropertySheetChildWindows(HWND hWnd, int nDx, int nDy)
{
	HWND hChild = GetWindow(hWnd, GW_CHILD);
	RECT rcChild;

	while (hChild)
	{
		GetWindowRect(hChild, &rcChild);
		OffsetRect(&rcChild, nDx, nDy);

		ScreenToClient(hWnd, (LPPOINT)&rcChild);
		ScreenToClient(hWnd, ((LPPOINT)&rcChild)+1);

		AdjustChildWindows(hChild);

		MoveWindow(hChild, rcChild.left, rcChild.top,
				rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, TRUE);

		hChild = GetNextWindow(hChild, GW_HWNDNEXT);
	}
}

static HTREEITEM GetSheetPageTreeItem(int nPage)
{
	HTREEITEM hItem;
	TVITEM    item;
	int       nTreePage;

	if (hSheetTreeCtrl == NULL)
		return NULL;

	hItem = (HTREEITEM)(int)SendMessage(hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_ROOT, (LPARAM)NULL);
	while (hItem)
	{
		item.hItem = hItem;
		item.mask = TVIF_PARAM;
		SendMessage(hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);

		nTreePage = (int)item.lParam;

		if (nTreePage == nPage)
			return hItem;

		hItem = (HTREEITEM)(int)SendMessage(hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
	}

	return NULL;
}

static int GetSheetPageTreeCurSelText(LPWSTR lpszText, int iBufSize)
{
	HTREEITEM hItem;
	TVITEM item;

	lpszText[0] = 0;

	if (hSheetTreeCtrl == NULL)
		return -1;

	hItem = (HTREEITEM)(int)SendMessage(hSheetTreeCtrl, TVM_GETNEXTITEM, TVGN_CARET, 0);

	if (hItem == NULL)
		return -1;

	item.hItem      = hItem;
	item.mask  	    = TVIF_TEXT;
	item.pszText    = lpszText;
	item.cchTextMax = iBufSize;

	SendMessage(hSheetTreeCtrl, TVM_GETITEM, 0, (LPARAM)&item);

	return wcslen(lpszText);
}

void ModifyPropertySheetForTreeSheet(HWND hPageDlg)
{
	HWND      hWnd, hTabWnd;
	DWORD     tabStyle;
	int       i, nPage, nPageCount;
	RECT      rectSheet, rectTree;
	HTREEITEM hItem;
	LONG_PTR  prevProc;

	TCITEM item;
	HWND hTempTab;

	if (g_nFirstInitPropertySheet == 0)
	{
		AdjustPropertySheetChildWindows(hPageDlg);
		return;
	}

	hWnd = GetParent(hPageDlg);
	if (!hWnd)
		return;

	prevProc = GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	pfnOldSheetProc = (WNDPROC)prevProc;
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)NewSheetWndProc);

	hTabWnd = PropSheet_GetTabControl(hWnd);

	if (!hTabWnd)
		return;

	tabStyle = (GetWindowLong(hTabWnd, GWL_STYLE) & ~TCS_MULTILINE);
	SetWindowLong(hTabWnd, GWL_STYLE, tabStyle | TCS_SINGLELINE);

	ShowWindow(hTabWnd, SW_HIDE);
	EnableWindow(hTabWnd, FALSE);

	GetWindowRect(hTabWnd, &rcTabCtrl);
	ScreenToClient(hTabWnd, (LPPOINT)&rcTabCtrl);
	ScreenToClient(hTabWnd, ((LPPOINT)&rcTabCtrl)+1);

	GetWindowRect(hWnd, &rectSheet);
	rectSheet.right += SHEET_TREE_WIDTH + 5;
	SetWindowPos(hWnd, HWND_TOP,
			-1, -1,
			rectSheet.right - rectSheet.left, rectSheet.bottom - rectSheet.top,
			SWP_NOZORDER | SWP_NOMOVE);
	CenterWindow(hWnd);

	MovePropertySheetChildWindows(hWnd, SHEET_TREE_WIDTH+5, 0);

	if (hSheetTreeCtrl != NULL)
	{
		DestroyWindow(hSheetTreeCtrl);
		hSheetTreeCtrl = NULL;
	}

	memset(&rectTree, 0, sizeof(rectTree));

	hTempTab = CreateWindowEx(0, TEXT("SysTabControl32"), NULL,
						WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
						rectTree.left, rectTree.top,
						rectTree.right - rectTree.left, rectTree.bottom - rectTree.top,
						hWnd, (HMENU)0x1234, hSheetInstance, NULL);

	item.mask    = TCIF_TEXT;
	item.iImage  = 0;
	item.lParam  = 0;
	item.pszText = TEXT("");
	SendMessage(hTempTab, TCM_INSERTITEM, 0, (LPARAM)&item);

	SendMessage(hTempTab, TCM_GETITEMRECT, 0, (LPARAM)&rcTabCaption);
	nCaptionHeight = (rcTabCaption.bottom - rcTabCaption.top);

	rcTabCaption.left   = rcTabCtrl.left + SHEET_TREE_WIDTH + 5;
	rcTabCaption.top    = 4;
	rcTabCaption.right  = rcTabCaption.left + (rcTabCtrl.right - rcTabCtrl.left);
	rcTabCaption.bottom = rcTabCaption.top + nCaptionHeight;

	DestroyWindow(hTempTab);

	i = (int)((SHEET_TREE_WIDTH * 3) / 4 + 0.5);

	rcSheetSnap.left   = rcTabCtrl.left + 4;
	rcSheetSnap.top    = (rcTabCtrl.bottom - i);
	rcSheetSnap.right  = rcTabCtrl.left + SHEET_TREE_WIDTH;
	rcSheetSnap.bottom = rcTabCtrl.bottom;

	if ((g_nGame == GLOBAL_OPTIONS) || (g_nGame == FOLDER_OPTIONS))
	{
		hSheetBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ABOUT));
		bUseScreenShot = FALSE;
	}
	else
	{
		if (!ScreenShotLoaded())
			LoadScreenShot(g_nGame, NULL, TAB_SCREENSHOT);

		if (ScreenShotLoaded())
		{
			hSheetBitmap = GetScreenShotHandle();
			bUseScreenShot = TRUE;
		}
		else
		{
			hSheetBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ABOUT));
			bUseScreenShot = FALSE;
		}
	}

	rectTree.left   = rcTabCtrl.left + 4;
	rectTree.top    = rcTabCtrl.top  + 5;
	rectTree.right  = rcTabCtrl.left + SHEET_TREE_WIDTH;
	rectTree.bottom = (rcTabCtrl.bottom - i) - 5;

	hSheetTreeCtrl = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
							TEXT("SysTreeView32"), TEXT("PageTree"),
							WS_TABSTOP | WS_CHILD | WS_VISIBLE | TVS_SHOWSELALWAYS | TVS_TRACKSELECT | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
							rectTree.left, rectTree.top,
							rectTree.right - rectTree.left, rectTree.bottom - rectTree.top,
							hWnd, (HMENU)0x7EEE, hSheetInstance, NULL);

	if (hSheetTreeCtrl == NULL)
	{
		WCHAR temp[100];
		DWORD dwError = GetLastError();
		swprintf(temp, _UIW(TEXT("PropertySheet TreeCtrl Creation Error %d %X")), (int)dwError, (int)dwError);
		MessageBox(hWnd, temp, _UIW(TEXT("Error")), IDOK);
	}

	SendMessage(hSheetTreeCtrl, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);

	nPageCount = SendMessage(hTabWnd, TCM_GETITEMCOUNT, 0, 0L);

	for (nPage = 0; nPage < nPageCount; ++nPage)
	{
		WCHAR          szText[256];
		TCITEM         ti;
		TVINSERTSTRUCT tvis;
		LPTVITEM       lpTvItem;

		// Get title and image of the page
		memset(&ti, 0, sizeof(TCITEM));
		ti.mask       = TCIF_TEXT|TCIF_IMAGE;
		ti.cchTextMax = ARRAY_LENGTH(szText);
		ti.pszText    = szText;

		SendMessage(hTabWnd, TCM_GETITEM, nPage, (LPARAM)&ti);

#if (_WIN32_IE >= 0x0400)
		lpTvItem = &tvis.DUMMYUNIONNAME.item;
#else
		lpTvItem = &tvis.item;
#endif
		// Create an item in the tree for the page
		tvis.hParent             = TVI_ROOT;
		tvis.hInsertAfter        = TVI_LAST;
		lpTvItem->mask           = TVIF_TEXT;
		lpTvItem->pszText        = szText;
		lpTvItem->iImage         = 0;
		lpTvItem->iSelectedImage = 0;
		lpTvItem->state          = 0;
		lpTvItem->stateMask      = 0;
		lpTvItem->lParam         = (LPARAM)NULL;

		// insert Item
		hItem = (HTREEITEM)(int)SendMessage(hSheetTreeCtrl, TVM_INSERTITEM, 0, (LPARAM)&tvis);

		if (hItem)
		{
			TVITEM item;

			item.hItem          = hItem;
			item.mask           = TVIF_PARAM;
			item.pszText        = NULL;
			item.iImage         = 0;
			item.iSelectedImage = 0;
			item.state          = 0;
			item.stateMask      = 0;
			item.lParam         = nPage;

			SendMessage(hSheetTreeCtrl, TVM_SETITEM, 0, (LPARAM)&item);
		}
	}

	nPage = SendMessage(hTabWnd, TCM_GETCURSEL, 0, 0);
	if (nPage != -1)
	{
		hItem = GetSheetPageTreeItem(nPage);
		if (hItem)
			SendMessage(hSheetTreeCtrl, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
	}

	g_nFirstInitPropertySheet = 0;
}
#endif /* TREE_SHEET */

static int CALLBACK PropSheetCallbackProc(HWND hDlg, UINT Msg, LPARAM lParam)
{
	switch (Msg)
	{
	case PSCB_INITIALIZED:
		TranslateDialog(hDlg, lParam, FALSE);
		break;
	}
	return 0;
}

/* Handle the information property page */
INT_PTR CALLBACK GamePropertiesDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd;
	switch (Msg)
	{
	case WM_INITDIALOG:
		if (g_hIcon)
			SendDlgItemMessage(hDlg, IDC_GAME_ICON, STM_SETICON, (WPARAM) g_hIcon, 0);

		TranslateDialog(hDlg, lParam, TRUE);

#ifdef TREE_SHEET
		if (GetShowTreeSheet())
			ModifyPropertySheetForTreeSheet(hDlg);
#endif /* TREE_SHEET */

#if defined(USE_SINGLELINE_TABCONTROL)
		{
			HWND hWnd = PropSheet_GetTabControl(GetParent(hDlg));
			DWORD tabStyle = (GetWindowLong(hWnd,GWL_STYLE) & ~TCS_MULTILINE);
			SetWindowLong(hWnd,GWL_STYLE,tabStyle | TCS_SINGLELINE);
		}
#endif

		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE),         GameInfoTitle(g_nGame));

		if (IS_GAME)
		{
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_MANUFACTURED),  GameInfoManufactured(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_STATUS),        GameInfoStatus(g_nGame, FALSE));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_CPU),           GameInfoCPU(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOUND),         GameInfoSound(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SCREEN),        GameInfoScreen(g_nGame));
#ifdef MISC_FOLDER
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_INPUT),         GameInfoInput(g_nGame));
#else /* MISC_FOLDER */
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_COLORS),        GameInfoColors(g_nGame));
#endif /* !MISC_FOLDER */
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_CLONEOF),       GameInfoCloneOf(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SAVESTATE),     GameInfoSaveState(g_nGame));
			Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOURCE),        GameInfoSource(g_nGame));

			if (DriverIsClone(g_nGame))
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_SHOW);
			}
			else
			{
				ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_HIDE);
			}
		}
		hWnd = PropSheet_GetTabControl(GetParent(hDlg));
		UpdateBackgroundBrush(hWnd);
		ShowWindow(hDlg, SW_SHOW);
		return 1;

	}
	return 0;
}

static INT_PTR HandleGameOptionsMessage(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	/* Below, 'changed' is used to signify the 'Apply'
	 * button should be enabled.
	 */
	WORD wID         = GET_WM_COMMAND_ID(wParam, lParam);
	HWND hWndCtrl    = GET_WM_COMMAND_HWND(wParam, lParam);
	WORD wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);
	BOOL changed     = FALSE;
	int nCurSelection = 0;

	switch (wID)
	{
	case IDC_SIZES:
	case IDC_FRAMESKIP:
	case IDC_DEFAULT_INPUT:
	case IDC_ROTATE:
	case IDC_VIEW:
	case IDC_SAMPLERATE:
	case IDC_VIDEO_MODE:
		if (wNotifyCode == CBN_SELCHANGE)
		{
			changed = TRUE;
		}
		break;
	case IDC_SCREEN:
	case IDC_M68K_CORE:
	case IDC_EFFECT:
#ifdef USE_SCALE_EFFECTS
	case IDC_SCALEEFFECT:
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
	case IDC_JOYID1:
	case IDC_JOYID2:
	case IDC_JOYID3:
	case IDC_JOYID4:
	case IDC_JOYID5:
	case IDC_JOYID6:
	case IDC_JOYID7:
	case IDC_JOYID8:
#endif /* JOYSTICK_ID */
	case IDC_BIOS :
	case IDC_BIOS1 :
	case IDC_BIOS2 :
	case IDC_BIOS3 :
	case IDC_BIOS4 :
	case IDC_BIOS5 :
	case IDC_BIOS6 :
	case IDC_BIOS7 :
	case IDC_BIOS8 :
	case IDC_BIOS9 :
	case IDC_BIOS10 :
	case IDC_BIOS11 :
	case IDC_BIOS12 :
	case IDC_BIOS13 :
	case IDC_BIOS14 :
	case IDC_BIOS15 :
	case IDC_BIOS16 :
		if (wNotifyCode == CBN_SELCHANGE)
		{
			changed = TRUE;
			OptionsToProp( hDlg, pGameOpts );
		}
		break;
	case IDC_SCREENSELECT:
		if (wNotifyCode == CBN_SELCHANGE)
		{
			//First save the changes for this screen index
			PropToOptions( hDlg, pGameOpts );
			nCurSelection = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_SCREENSELECT));
			if (nCurSelection != CB_ERR)
				g_nSelectScreenIndex = ComboBox_GetItemData(GetDlgItem(hDlg,IDC_SCREENSELECT), nCurSelection);
			//changed = TRUE; /* just select which to configure, not changed params */
			//Load settings for new Index
			OptionsToProp( hDlg, pGameOpts );
		}
		break;
	case IDC_WINDOWED:
		changed = ReadControl(hDlg, wID);
		break;

	case IDC_D3D_VERSION:
		if (wNotifyCode == CBN_SELCHANGE)
		{
			changed = TRUE;
		}
		break;

	case IDC_PADDLE:
	case IDC_ADSTICK:
	case IDC_PEDAL:
	case IDC_DIAL:
	case IDC_TRACKBALL:
	case IDC_LIGHTGUNDEVICE:
	case IDC_POSITIONALDEVICE:
	case IDC_MOUSEDEVICE:
		if (wNotifyCode == CBN_SELCHANGE)
		{
			changed = TRUE;
		}
		break;

	case IDC_REFRESH:
		if (wNotifyCode == LBN_SELCHANGE)
		{
			RefreshSelectionChange(hDlg, hWndCtrl);
			changed = TRUE;
		}
		break;

	case IDC_ASPECTRATION:
	case IDC_ASPECTRATIOD:
		if (wNotifyCode == EN_CHANGE)
		{
			if (g_bInternalSet == FALSE)
				changed = TRUE;
		}
		break;

	case IDC_TRIPLE_BUFFER:
		changed = ReadControl(hDlg, wID);
		break;

	case IDC_ASPECT:
		nCurSelection = Button_GetCheck( GetDlgItem(hDlg, IDC_ASPECT));
		if( g_bAutoAspect[g_nSelectScreenIndex] != nCurSelection )
		{
			changed = TRUE;
			g_bAutoAspect[g_nSelectScreenIndex] = nCurSelection;
		}
		break;

	case IDC_PROP_RESET:
		if (wNotifyCode != BN_CLICKED)
			break;

		FreeGameOptions(pGameOpts);
		CopyGameOptions(&origGameOpts,pGameOpts);
		if (IS_GAME)
			SetGameUsesDefaults(g_nGame,orig_uses_defaults);
		else if (IS_FOLDER)
			SetFolderUsesDefaults(g_pFolder,orig_uses_defaults);

		BuildDataMap();
		PopulateControls(hDlg);
		OptionsToProp(hDlg, pGameOpts);
		SetPropEnabledControls(hDlg);
		g_bReset = FALSE;
		PropSheet_UnChanged(GetParent(hDlg), hDlg);
		g_bUseDefaults = orig_uses_defaults;
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
		break;

	case IDC_USE_DEFAULT:
		if (!IS_GLOBAL)
		{
			if (IS_GAME)
			{
				SetGameUsesDefaults(g_nGame,TRUE);

				pGameOpts = GetGameOptions(g_nGame);
				g_bUseDefaults = GetGameUsesDefaults(g_nGame);
			}
			else
			{
				SetFolderUsesDefaults(g_pFolder,TRUE);

				pGameOpts = GetFolderOptions(g_pFolder);
				g_bUseDefaults = GetFolderUsesDefaults(g_pFolder);
			}

			BuildDataMap();
			PopulateControls(hDlg);
			OptionsToProp(hDlg, pGameOpts);
			SetPropEnabledControls(hDlg);
			if (orig_uses_defaults != g_bUseDefaults)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);
				g_bReset = TRUE;
			}
			else
			{
				if (IS_GAME)
				{
						PropSheet_UnChanged(GetParent(hDlg), hDlg);
						g_bReset = FALSE;
				}
				else
				{
					if (strcmp(origGameOpts.bios, pGameOpts->bios))
					{
						PropSheet_Changed(GetParent(hDlg), hDlg);
						g_bReset = TRUE;
					}
					else
					{
						PropSheet_UnChanged(GetParent(hDlg), hDlg);
						g_bReset = FALSE;
					}
				}
			}
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
		}
		break;

	default:
#ifdef MESS
		if (MessPropertiesCommand(g_nGame, hDlg, wNotifyCode, wID, &changed))
#endif
		if (wNotifyCode == BN_CLICKED)
			changed = TRUE;
		break;
	}
	if (changed == TRUE)
	{
		// enable the apply button
		if (IS_GAME)
		{
			SetGameUsesDefaults(g_nGame,FALSE);
			g_bUseDefaults = FALSE;
		}
		else if (IS_FOLDER)
		{
			if (wID != IDC_BIOS)
			{
				SetFolderUsesDefaults(g_pFolder,FALSE);
				g_bUseDefaults = FALSE;
			}
		}
		PropSheet_Changed(GetParent(hDlg), hDlg);
		g_bReset = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
	}
	SetPropEnabledControls(hDlg);

	// make sure everything's copied over, to determine what's changed
	PropToOptions(hDlg, pGameOpts);
	ReadControls(hDlg);

	// redraw it, it might be a new color now
	if (GetDlgItem(hDlg,wID))
		InvalidateRect(GetDlgItem(hDlg,wID),NULL,FALSE);

	EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

	return 0;
}

static INT_PTR HandleGameOptionsNotify(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (((NMHDR *) lParam)->code)
	{
	//We'll need to use a CheckState Table 
	//Because this one gets called for all kinds of other things too, and not only if a check is set
	case LVN_ITEMCHANGED: 
		break;
	case PSN_SETACTIVE:
		/* Initialize the controls. */
		PopulateControls(hDlg);
		OptionsToProp(hDlg, pGameOpts);
		SetPropEnabledControls(hDlg);
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
		break;

	case PSN_APPLY:
		/* Save and apply the options here */
		PropToOptions(hDlg, pGameOpts);
		ReadControls(hDlg);
		if (IS_GLOBAL)
			pGameOpts = GetDefaultOptions();
		else if (IS_FOLDER)
		{
			SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);
			orig_uses_defaults = g_bUseDefaults;
			pGameOpts = GetFolderOptions(g_pFolder);
		}
		else
		{
			SetGameUsesDefaults(g_nGame,g_bUseDefaults);
			orig_uses_defaults = g_bUseDefaults;
			pGameOpts = GetGameOptions(g_nGame);
		}

		FreeGameOptions(&origGameOpts);
		CopyGameOptions(pGameOpts,&origGameOpts);

		BuildDataMap();
		PopulateControls(hDlg);
		OptionsToProp(hDlg, pGameOpts);
		SetPropEnabledControls(hDlg);
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
		g_bReset = FALSE;
		PropSheet_UnChanged(GetParent(hDlg), hDlg);
		SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
		return PSNRET_NOERROR;

	case PSN_KILLACTIVE:
		/* Save Changes to the options here. */
		PropToOptions(hDlg, pGameOpts);
		ReadControls(hDlg);
		ResetDataMap();
		if (IS_GAME)
			SetGameUsesDefaults(g_nGame,g_bUseDefaults);
		else if (IS_FOLDER)
			SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);
		SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
		return 1;  

	case PSN_RESET:
		// Reset to the original values. Disregard changes
		FreeGameOptions(pGameOpts);
		CopyGameOptions(&origGameOpts,pGameOpts);
		if (IS_GAME)
			SetGameUsesDefaults(g_nGame,orig_uses_defaults);
		else if (IS_FOLDER)
			SetFolderUsesDefaults(g_pFolder,orig_uses_defaults);
		SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
		break;

	case PSN_HELP:
		// User wants help for this property page
		break;
	}

	EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

	return 0;
}

static INT_PTR HandleGameOptionsCtlColor(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	//Set the Coloring of the elements
	if( GetControlID(hDlg,(HWND)lParam) < 0)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);
		return 0;
	}

	if (IsControlOptionValue(hDlg,(HWND)lParam, GetDefaultOptions() ) )
		//Normal Black case
		SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
	else if (((IS_GAME && DriverIsVector(g_nGame)) || (IS_FOLDER && FolderHasVector(g_pFolder))) && IsControlOptionValue(hDlg,(HWND)lParam, GetVectorOptions() ) )
		SetTextColor((HDC)wParam,VECTOR_COLOR);
	else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, GetSourceOptions(g_nGame) ) )
		SetTextColor((HDC)wParam,FOLDER_COLOR);
	else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, GetParentOptions(g_nGame) ) )
		SetTextColor((HDC)wParam,PARENT_COLOR);
	else if (IS_GAME && IsControlOptionValue(hDlg,(HWND)lParam, &origGameOpts) )
		SetTextColor((HDC)wParam,GAME_COLOR);
	else
	{
		switch ( g_nPropertyMode )
		{
		case SOURCE_GAME:
			SetTextColor((HDC)wParam,GAME_COLOR);
			break;
		case SOURCE_FOLDER:
			SetTextColor((HDC)wParam,FOLDER_COLOR);
			break;
		case SOURCE_VECTOR:
			SetTextColor((HDC)wParam,VECTOR_COLOR);
			break;
		default:
		case SOURCE_GLOBAL:
			SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
			break;
		}
	}
	if( Msg == WM_CTLCOLORSTATIC )
	{

	//	SetBkColor((HDC)wParam,GetSysColor(COLOR_3DFACE) );
		if( hThemes )
		{
		    if( fnIsThemed && fnIsThemed() )
		    {
			    HWND hWnd = PropSheet_GetTabControl(GetParent(hDlg));
			    // Set the background mode to transparent
			    SetBkMode((HDC)wParam, TRANSPARENT);
    
			    // Get the controls window dimensions
			    GetWindowRect((HWND)lParam, &rc);
    
			    // Map the coordinates to coordinates with the upper left corner of dialog control as base
			    MapWindowPoints(NULL, hWnd, (LPPOINT)(&rc), 2);
    
			    // Adjust the position of the brush for this control (else we see the top left of the brush as background)
			    SetBrushOrgEx((HDC)wParam, -rc.left, -rc.top, NULL);
    
			    // Return the brush
			    return (INT_PTR)(hBkBrush);
		    }
		    else
			    SetBkColor((HDC) wParam,GetSysColor(COLOR_3DFACE) );
		}
		else
			SetBkColor((HDC) wParam,GetSysColor(COLOR_3DFACE) );
	}
	else
		SetBkColor((HDC)wParam,RGB(255,255,255) );
	UnrealizeObject(background_brush);
	return (DWORD)background_brush;
}

/* Handle all options property pages */
INT_PTR CALLBACK GameOptionsProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, TRUE);

#ifdef TREE_SHEET
		if (GetShowTreeSheet())
			ModifyPropertySheetForTreeSheet(hDlg);
#endif /* TREE_SHEET */

		/* Fill in the Game info at the top of the sheet */
		g_nSelectScreenIndex = 0; // Start out wth screen 0
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE), GameInfoTitle(g_nGame));
		InitializeOptions(hDlg);
		InitializeMisc(hDlg);

		PopulateControls(hDlg);
		OptionsToProp(hDlg, pGameOpts);
		SetPropEnabledControls(hDlg);
		if (IS_GLOBAL)
			ShowWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), SW_HIDE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);

#ifdef DRIVER_SWITCH
		{
			int i;

			for (i = 0; drivers_table[i].name; i++)
				ShowWindow(GetDlgItem(hDlg, drivers_table[i].ctrl), IS_GLOBAL ? SW_SHOW : SW_HIDE);

			ShowWindow(GetDlgItem(hDlg, IDC_DRV_TEXT), IS_GLOBAL ? SW_SHOW : SW_HIDE);
		}
#endif /* DRIVER_SWITCH */

		EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);
		ShowWindow(hDlg, SW_SHOW);

		return 1;

	case WM_HSCROLL:
		/* slider changed */
		HANDLE_WM_HSCROLL(hDlg, wParam, lParam, OptOnHScroll);
		g_bUseDefaults = FALSE;
		g_bReset = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), TRUE);
		PropSheet_Changed(GetParent(hDlg), hDlg);

		// make sure everything's copied over, to determine what's changed
		PropToOptions(hDlg,pGameOpts);
		ReadControls(hDlg);
		// redraw it, it might be a new color now
		InvalidateRect((HWND)lParam,NULL,TRUE);

		break;

	case WM_COMMAND:
		return HandleGameOptionsMessage(hDlg, Msg, wParam, lParam);

	case WM_NOTIFY:
		return HandleGameOptionsNotify(hDlg, Msg, wParam, lParam);

	case WM_CTLCOLORSTATIC :
	case WM_CTLCOLOREDIT :
		return HandleGameOptionsCtlColor(hDlg, Msg, wParam, lParam);

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, TEXT(MAME32CONTEXTHELP), HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU: 
		HelpFunction((HWND)wParam, TEXT(MAME32CONTEXTHELP), HH_TP_HELP_CONTEXTMENU, GetHelpIDs());
		break; 

	}
	EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

	return 0;
}

/* Read controls that are not handled in the DataMap */
static void PropToOptions(HWND hWnd, options_type *o)
{
	HWND hCtrl = NULL;
	HWND hCtrl2;
	HWND hCtrl3;
	int  nIndex;

	if (IS_GAME)
		SetGameUsesDefaults(g_nGame,g_bUseDefaults);
	else
	if (IS_FOLDER)
		SetFolderUsesDefaults(g_pFolder,g_bUseDefaults);

#ifdef DRIVER_SWITCH
	if (IS_GLOBAL)
	{
		char buffer[200];
		int all_enable = 1;
		int i;

		buffer[0] = '\0';

		for (i = 0; drivers_table[i].name; i++)
		{
			hCtrl = GetDlgItem(hWnd, drivers_table[i].ctrl);
			if (hCtrl && Button_GetCheck(hCtrl))
			{
				if (buffer[0])
					strcat(buffer, ",");
				strcat(buffer, drivers_table[i].name);
			}
			else
				all_enable = 0;
		}

		if (buffer[0])
		{
			FreeIfAllocated(&o->driver_config);

			if (all_enable)
				o->driver_config = strdup("all");
			else
				o->driver_config = strdup(buffer);
		}
	}
#endif /* DRIVER_SWITCH */

	/* resolution size */
	hCtrl = GetDlgItem(hWnd, IDC_SIZES);
	if (hCtrl)
	{
		char buffer[200];

		/* Screen size control */
		nIndex = ComboBox_GetCurSel(hCtrl);

		if (nIndex == 0)
			sprintf(buffer, "%dx%d", 0, 0); // auto
		else
		{
			int w, h;

			ComboBox_GetTextA(hCtrl, buffer, ARRAY_LENGTH(buffer)-1);
			if (sscanf(buffer, "%d x %d", &w, &h) == 2)
			{
				sprintf(buffer, "%dx%d", w, h);
			}
			else
			{
				sprintf(buffer, "%dx%d", 0, 0); // auto
			}
		}

		/* refresh */
		hCtrl = GetDlgItem(hWnd, IDC_REFRESH);
		if (hCtrl)
		{
			nIndex = ComboBox_GetCurSel(hCtrl);
			sprintf(buffer + strlen(buffer), "@%ld", ComboBox_GetItemData(hCtrl, nIndex));
		}

		if (strcmp(buffer,"0x0@0") == 0)
			sprintf(buffer,"auto");
		FreeIfAllocated(&o->resolutions[g_nSelectScreenIndex]);
		o->resolutions[g_nSelectScreenIndex] = mame_strdup(buffer);
	}

	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	hCtrl3 = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl && hCtrl2 && hCtrl3)
	{
		BOOL bAutoAspect = Button_GetCheck(hCtrl3);
		if( bAutoAspect )
		{
			FreeIfAllocated(&o->aspects[g_nSelectScreenIndex]);
			o->aspects[g_nSelectScreenIndex] = mame_strdup("auto");
		}
		else
		{
			int n = 0;
			int d = 0;
			char buffer[200];

			Edit_GetTextA(hCtrl, buffer, ARRAY_LENGTH(buffer));
			sscanf(buffer,"%d",&n);

			Edit_GetTextA(hCtrl2, buffer, ARRAY_LENGTH(buffer));
			sscanf(buffer,"%d",&d);

			if (n == 0 || d == 0)
			{
				n = 4;
				d = 3;
			}

			snprintf(buffer, ARRAY_LENGTH(buffer), "%d:%d", n, d);
			FreeIfAllocated(&o->aspects[g_nSelectScreenIndex]);
			o->aspects[g_nSelectScreenIndex] = mame_strdup(buffer);
		}
	}
#ifdef MESS
	MessPropToOptions(g_nGame, hWnd, o);
#endif
}

/* Populate controls that are not handled in the DataMap */
static void OptionsToProp(HWND hWnd, options_type* o)
{
	HWND hCtrl;
	HWND hCtrl2;
	char buf[100];
	int  h = 0;
	int  w = 0;
	int  n = 0;
	int  d = 0;
	int  r = 0;	//fixme: r should be stored in options_type* o?

	g_bInternalSet = TRUE;

#ifdef DRIVER_SWITCH
	{
		char *temp = mame_strdup(o->driver_config);
		UINT32 enabled = 0;
		int i;

		if (temp)
		{
			int i;

			char *p = strtok(temp, ",");

			while (p)
			{
				char *s = mame_strtrim(p);	//get individual driver name
				if (s[0])
				{
					if (mame_stricmp(s, "all") == 0)
					{
						enabled = (UINT32)-1;
						break;
					}

					for (i = 0; drivers_table[i].name; i++)
						if (mame_stricmp(s, drivers_table[i].name) == 0)
						{
							enabled |= 1 << i;
							break;
						}

					if (!drivers_table[i].name)
						dwprintf(_WINDOWSW(TEXT("Illegal value for %s = %s\n")), TEXT(OPTION_DRIVER_CONFIG), _Unicode(s));
				}
				free(s);

				p = strtok(NULL, ",");
			}

			free(temp);
		}

		if (enabled == 0)
			enabled = 1;	// default to mamedrivers

		for (i = 0; drivers_table[i].name; i++)
			Button_SetCheck(GetDlgItem(hWnd, drivers_table[i].ctrl), enabled & (1 << i));
	}
#endif /* DRIVER_SWITCH */

	/* video */

	/* get desired resolution */
	if (mame_stricmp(o->resolutions[g_nSelectScreenIndex], "auto") == 0)
	{
		w = h = r = 0;
	}
	else
	if (sscanf(o->resolutions[g_nSelectScreenIndex], "%dx%d@%d", &w, &h, &r) < 2)
	{
		w = h = r = 0;
	}
	/* Setup Screen*/
	UpdateScreenUI(hWnd );
	/* Setup sizes list based on depth. */
	UpdateDisplayModeUI(hWnd, r);
	/* Setup refresh list based on depth. */
	UpdateRefreshUI(hWnd );
	/* Setup Select screen*/
	UpdateSelectScreenUI(hWnd );

	hCtrl = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl)
	{
		Button_SetCheck(hCtrl, g_bAutoAspect[g_nSelectScreenIndex] );
	}
	/* Screen size drop down list */
	hCtrl = GetDlgItem(hWnd, IDC_SIZES);
	if (hCtrl)
	{
		if (w == 0 && h == 0)
		{
			/* default to auto */
			ComboBox_SetCurSel(hCtrl, 0);
		}
		else
		{
			/* Select the mode in the list. */
			int nSelection = 0;
			int nCount = 0;

			/* Get the number of items in the control */
			nCount = ComboBox_GetCount(hCtrl);

			while (0 < nCount--)
			{
				int nWidth, nHeight;

				/* Get the screen size */
				ComboBox_GetLBTextA(hCtrl, nCount, buf);

				if (sscanf(buf, "%d x %d", &nWidth, &nHeight) == 2)
				{
					/* If we match, set nSelection to the right value */
					if (w == nWidth
					&&  h == nHeight)
					{
						nSelection = nCount;
						break;
					}
				}
			}
			ComboBox_SetCurSel(hCtrl, nSelection);
		}
	}

	/* Screen refresh list */
	hCtrl = GetDlgItem(hWnd, IDC_REFRESH);
	if (hCtrl)
	{
		if (r == 0)
		{
			/* default to auto */
			ComboBox_SetCurSel(hCtrl, 0);
		}
		else
		{
			/* Select the mode in the list. */
			int nSelection = 0;
			int nCount = 0;

			/* Get the number of items in the control */
			nCount = ComboBox_GetCount(hCtrl);

			while (0 < nCount--)
			{
				int nRefresh;

				/* Get the screen Refresh */
				nRefresh = ComboBox_GetItemData(hCtrl, nCount);

				/* If we match, set nSelection to the right value */
				if (r == nRefresh)
				{
					nSelection = nCount;
					break;
				}
			}
			ComboBox_SetCurSel(hCtrl, nSelection);
		}
	}
	/* Screen select list */
	hCtrl = GetDlgItem(hWnd, IDC_SCREENSELECT);
	if (hCtrl)
	{
		ComboBox_SetCurSel(hCtrl, g_nSelectScreenIndex);
	}
	/* View select list */
	hCtrl = GetDlgItem(hWnd, IDC_VIEW);
	if (hCtrl)
	{
		int nCount = 0;
		nCount = ComboBox_GetCount(hCtrl);
    
		while (0 < nCount--)
		{
        
			/* Get the view name */
			const char* ptr = (const char*)ComboBox_GetItemData(hCtrl, nCount);
        
			/* If we match, set nSelection to the right value */
			if (strcmp (o->views[g_nSelectScreenIndex], ptr ) == 0)
				break;
		}
		ComboBox_SetCurSel(hCtrl, nCount);
	}

	hCtrl = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl)
	{
		if( strcmp( o->aspects[g_nSelectScreenIndex], "auto") == 0)
		{
			Button_SetCheck(hCtrl, TRUE);
			g_bAutoAspect[g_nSelectScreenIndex] = TRUE;
		}
		else
		{
			Button_SetCheck(hCtrl, FALSE);
			g_bAutoAspect[g_nSelectScreenIndex] = FALSE;
		}
	}
	hCtrl = GetDlgItem(hWnd, IDC_FSGAMMADISP);
	if (hCtrl)
	{
		snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", o->full_screen_gamma);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_FSBRIGHTNESSDISP);
	if (hCtrl)
	{
		snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", o->full_screen_brightness);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_FSCONTRASTDISP);
	if (hCtrl)
	{
		snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", o->full_screen_contrast);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_NUMSCREENSDISP);
	if (hCtrl)
	{
		snprintf(buf, ARRAY_LENGTH(buf), "%d", o->numscreens);
		Static_SetTextA(hCtrl, buf);
	}


	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	if (hCtrl && hCtrl2)
	{
		n = 0;
		d = 0;
		if(o->aspects[g_nSelectScreenIndex])
		{
			if (sscanf(o->aspects[g_nSelectScreenIndex], "%d:%d", &n, &d) == 2 && n != 0 && d != 0)
			{
				sprintf(buf, "%d", n);
				Edit_SetTextA(hCtrl, buf);
				sprintf(buf, "%d", d);
				Edit_SetTextA(hCtrl2, buf);
			}
			else
			{
				Edit_SetTextA(hCtrl,  "4");
				Edit_SetTextA(hCtrl2, "3");
			}
		}
		else
		{
			Edit_SetTextA(hCtrl,  "4");
			Edit_SetTextA(hCtrl2, "3");
		}
	}

	/* core video */
	hCtrl = GetDlgItem(hWnd, IDC_GAMMADISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->gamma);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_CONTRASTDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->contrast);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_BRIGHTCORRECTDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->brightness);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_PAUSEBRIGHTDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->pause_brightness);
		Static_SetTextA(hCtrl, buf);
	}

	/* Input */
	hCtrl = GetDlgItem(hWnd, IDC_JDZDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->joy_deadzone);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_JSATDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->joy_saturation);
		Static_SetTextA(hCtrl, buf);
	}

#ifdef TRANS_UI
	hCtrl = GetDlgItem(hWnd, IDC_TRANSPARENCYDISP);
	if (hCtrl)
	{
		sprintf(buf, "%d", o->ui_transparency);
		Static_SetTextA(hCtrl, buf);
	}
#endif /* TRANS_UI */

	/* thread priority */
	hCtrl = GetDlgItem(hWnd, IDC_HIGH_PRIORITYTXT);
	if (hCtrl)
	{
		sprintf(buf, "%d", o->priority);
		Static_SetTextA(hCtrl, buf);
	}

	/* vector */
	hCtrl = GetDlgItem(hWnd, IDC_BEAMDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->beam);
		Static_SetTextA(hCtrl, buf);
	}

	hCtrl = GetDlgItem(hWnd, IDC_FLICKERDISP);
	if (hCtrl)
	{
		sprintf(buf, "%03.2f", o->flicker);
		Static_SetTextA(hCtrl, buf);
	}

	/* sound */
	hCtrl = GetDlgItem(hWnd, IDC_VOLUMEDISP);
	if (hCtrl)
	{
		sprintf(buf, "%ddB", o->volume);
		Static_SetTextA(hCtrl, buf);
	}
	AudioLatencySelectionChange(hWnd);

	PrescaleSelectionChange(hWnd);

	ThreadPrioritySelectionChange(hWnd);

	g_bInternalSet = FALSE;

	g_nInputIndex = 0;
	hCtrl = GetDlgItem(hWnd, IDC_DEFAULT_INPUT);
	if (hCtrl)
	{
		int nCount;

		/* Get the number of items in the control */
		nCount = ComboBox_GetCount(hCtrl);

		if (o->ctrlr)
		{
			while (0 < nCount--)
			{
				ComboBox_GetLBTextA(hCtrl, nCount, buf);

				if (mame_stricmp (buf,o->ctrlr) == 0)
				{
					g_nInputIndex = nCount;
				}
			}

			ComboBox_SetCurSel(hCtrl, g_nInputIndex);
		}
	}

	g_nEffectIndex = 0;
	hCtrl = GetDlgItem(hWnd, IDC_EFFECT);	
	if (hCtrl)
	{
		int nCount;

		/* Get the number of items in the control */
		nCount = ComboBox_GetCount(hCtrl);

		if (o->effect)
		{
			while (0 < nCount--)
			{
				ComboBox_GetLBTextA(hCtrl, nCount, buf);

				if (mame_stricmp (buf,o->effect) == 0)
				{
					g_nEffectIndex = nCount;
				}
			}

			ComboBox_SetCurSel(hCtrl, g_nEffectIndex);
		}
	}

#ifdef MESS
	MessOptionsToProp(g_nGame, hWnd, o);
#endif
}

/* Adjust controls - tune them to the currently selected game */
static void SetPropEnabledControls(HWND hWnd)
{
	HWND hCtrl;
	int  nIndex;
	int  sound;
	BOOL ddraw = FALSE;
	BOOL d3d = FALSE;
	BOOL gdi = FALSE;
	BOOL useart = TRUE;
//	BOOL multimon = (DirectDraw_GetNumDisplays() >= 2);
	int joystick_attached = 0;
	int in_window = 0;

#ifdef MESS
	MessSetPropEnabledControls(hWnd, pGameOpts);
#endif

	nIndex = g_nGame;

	if( ! mame_stricmp(pGameOpts->video, "d3d" ) )
	{
		d3d = TRUE;
		ddraw = FALSE;
		gdi = FALSE;
	}else if ( ! mame_stricmp(pGameOpts->video, "ddraw" ) )
	{
		d3d = FALSE;
		ddraw = TRUE;
		gdi = FALSE;
	}else
	{
		d3d = FALSE;
		ddraw = FALSE;
		gdi = TRUE;
	}

	// check windows mode is enabled
	hCtrl = GetDlgItem(hWnd, IDC_WINDOWED);
	if (hCtrl)
		in_window = Button_GetCheck(hCtrl);
	else
		in_window = pGameOpts->window;

	Button_SetCheck(GetDlgItem(hWnd, IDC_ASPECT), g_bAutoAspect[g_nSelectScreenIndex] );

	EnableWindow(GetDlgItem(hWnd, IDC_MAXIMIZE),               in_window);

	EnableWindow(GetDlgItem(hWnd, IDC_WAITVSYNC),              !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_TRIPLE_BUFFER),          !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALE),               !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALEDISP),           !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALETEXT),           !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_HWSTRETCH),              ddraw && DirectDraw_HasHWStretch());
	EnableWindow(GetDlgItem(hWnd, IDC_SWITCHRES),              !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_SYNCREFRESH),            !gdi);
//	EnableWindow(GetDlgItem(hWnd, IDC_REFRESH),                !in_window && !gdi);
//	EnableWindow(GetDlgItem(hWnd, IDC_REFRESHTEXT),            !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMA),                !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMATEXT),            !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMADISP),            !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESS),           !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESSTEXT),       !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESSDISP),       !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRAST),             !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRASTTEXT),         !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRASTDISP),         !in_window);

	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOTEXT),        !g_bAutoAspect[g_nSelectScreenIndex]);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATION),           !g_bAutoAspect[g_nSelectScreenIndex]);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOD),           !g_bAutoAspect[g_nSelectScreenIndex]);

	EnableWindow(GetDlgItem(hWnd, IDC_D3D_FILTER),             d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_D3D_VERSION),            d3d);

	EnableWindow(GetDlgItem(hWnd, IDC_D3D_TEXT),               d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_DDRAW_TEXT),             ddraw);

//mamep: gdi is ok
/*
	//Switchres and D3D or ddraw enable the per screen parameters

	EnableWindow(GetDlgItem(hWnd, IDC_NUMSCREENS),                 (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_NUMSCREENSDISP),             (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_SCREENSELECT),               (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_SCREENSELECTTEXT),           (ddraw || d3d) && multimon);
*/
#ifdef TRANS_UI
	hCtrl = GetDlgItem(hWnd, IDC_TRANSUI);
	useart = Button_GetCheck(hCtrl);

	EnableWindow(GetDlgItem(hWnd, IDC_TRANSPARENCY),           useart);
	EnableWindow(GetDlgItem(hWnd, IDC_TRANSPARENCYDISP),       useart);
#endif /* TRANS_UI */

	/* Joystick options */
	hCtrl = GetDlgItem(hWnd, IDC_JOYSTICK);

	if (hCtrl)
		joystick_attached = Button_GetCheck(hCtrl);
	else
		joystick_attached = pGameOpts->joystick;

	EnableWindow(GetDlgItem(hWnd, IDC_JDZTEXT),                joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JDZDISP),                joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JDZ),                    joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSATTEXT),               joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSATDISP),               joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSAT),                   joystick_attached);
#ifdef JOYSTICK_ID
	if (joystick_attached && DIJoystick.Available())
	{
		int  i;

		EnableWindow(GetDlgItem(hWnd, IDC_JOYIDTEXT),  TRUE);

		for (i = 0; i < 8; i++)
		{
			if (i < DIJoystick_GetNumPhysicalJoysticks())
			{
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1 + i),     TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT + i), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1 + i),     FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT + i), FALSE);
			}
		}
	}
	else
	{
		EnableWindow(GetDlgItem(hWnd, IDC_JOYIDTEXT),  FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID1),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID2),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID3),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID4),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID5),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID6),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID7),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID8),     FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID1TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID2TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID3TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID4TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID5TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID6TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID7TEXT), FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_JOYID8TEXT), FALSE);
	}
#endif /* JOYSTICK_ID */

	/* Trackball / Mouse options */
	if (nIndex <= -1 || DriverUsesTrackball(nIndex) || DriverUsesLightGun(nIndex))
	{
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),TRUE);
	}
	else
	{
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),FALSE);
	}

	if (nIndex <= -1 || DriverUsesLightGun(nIndex))
	{
		// on WinXP the Lightgun and Dual Lightgun switches are no longer supported use mouse instead
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;
		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			bOsVersionInfoEx = GetVersionEx ( (OSVERSIONINFO *) &osvi);
		}

		if( bOsVersionInfoEx && (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion >= 5) )
		{
			BOOL use_lightgun;
			BOOL mouse;
			//XP and above...
			Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), TRUE);
			Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE), TRUE);
			use_lightgun = Button_GetCheck(GetDlgItem(hWnd,IDC_LIGHTGUN));
			mouse = Button_GetCheck(GetDlgItem(hWnd,IDC_USE_MOUSE));
			Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), !mouse);
			Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN),use_lightgun && !mouse);
			Button_Enable(GetDlgItem(hWnd,IDC_RELOAD),use_lightgun || mouse);
		}
		else
		{
			BOOL use_lightgun;
			// Older than XP 
			Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), TRUE);
			use_lightgun = Button_GetCheck(GetDlgItem(hWnd,IDC_LIGHTGUN));
			Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN),use_lightgun);
			Button_Enable(GetDlgItem(hWnd,IDC_RELOAD),use_lightgun);
		}
	}
	else
	{
		Button_Enable(GetDlgItem(hWnd,IDC_LIGHTGUN), FALSE);
		Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN), FALSE);
		Button_Enable(GetDlgItem(hWnd,IDC_RELOAD), FALSE);
	}


	/* Sound options */
	hCtrl = GetDlgItem(hWnd, IDC_USE_SOUND);
	if (hCtrl)
	{
		sound = Button_GetCheck(hCtrl);
		ComboBox_Enable(GetDlgItem(hWnd, IDC_SAMPLERATE), (sound != 0));

		EnableWindow(GetDlgItem(hWnd,IDC_VOLUME),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_RATETEXT),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUMEDISP),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUMETEXT),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY_DISP),sound);
		EnableWindow(GetDlgItem(hWnd,IDC_AUDIO_LATENCY_TEXT),sound);
#ifdef USE_VOLUME_AUTO_ADJUST
		EnableWindow(GetDlgItem(hWnd,IDC_VOLUME_ADJUST),sound);
#endif /* USE_VOLUME_AUTO_ADJUST */
		SetSamplesEnabled(hWnd, nIndex, sound);
		SetStereoEnabled(hWnd, nIndex);
		SetYM3812Enabled(hWnd, nIndex);
	}

	if (Button_GetCheck(GetDlgItem(hWnd, IDC_AUTOFRAMESKIP)))
		EnableWindow(GetDlgItem(hWnd, IDC_FRAMESKIP), FALSE);
	else
		EnableWindow(GetDlgItem(hWnd, IDC_FRAMESKIP), TRUE);


	// misc

	if (g_biosinfo != -1)
	{
		ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT), SW_SHOW);
		ShowWindow(GetDlgItem(hWnd,IDC_BIOS), SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd,IDC_BIOS), SW_HIDE);
	}

	if (nIndex <= -1 || DriverSupportsSaveState(nIndex))
	{
		Button_Enable(GetDlgItem(hWnd,IDC_ENABLE_AUTOSAVE),TRUE);
	}
	else
	{
		Button_Enable(GetDlgItem(hWnd,IDC_ENABLE_AUTOSAVE),FALSE);
	}

#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
	{
		BOOL has_m68k = DriverHasM68K(nIndex);

		ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORE), has_m68k ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORETEXT), has_m68k ? SW_SHOW : SW_HIDE);
	}
#else /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
	ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORE), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, IDC_M68K_CORETEXT), SW_HIDE);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */

	// BIOS
	if (nIndex == GLOBAL_OPTIONS)
	{
		int i;

		for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		{
			int bios_driver = GetSystemBiosDriver(i);
			if (bios_driver != -1)
			{
				Static_SetText(GetDlgItem(hWnd, IDC_BIOSTEXT1 + i), driversw[bios_driver]->description);

				ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT1 + i), SW_SHOW);
				ShowWindow(GetDlgItem(hWnd,IDC_BIOS1 + i), SW_SHOW);
			}
			else
			{
				ShowWindow(GetDlgItem(hWnd,IDC_BIOSTEXT1 + i), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd,IDC_BIOS1 + i), SW_HIDE);
			}
		}
	}
}

/**************************************************************
 * Control Helper functions for data exchange
 **************************************************************/

static void AssignSampleRate(HWND hWnd)
{
	switch (g_nSampleRateIndex)
	{
		case 0:  pGameOpts->samplerate = 11025; break;
		case 1:  pGameOpts->samplerate = 22050; break;
		case 2:  pGameOpts->samplerate = 24000; break;
		case 3:
		default: pGameOpts->samplerate = 44100; break;
		case 4:  pGameOpts->samplerate = 48000; break;
	}
}

static void AssignVolume(HWND hWnd)
{
	pGameOpts->volume = g_nVolumeIndex - 32;
}

static void AssignPriority(HWND hWnd)
{
	pGameOpts->priority = g_nPriorityIndex - 15;
}

static void AssignBrightCorrect(HWND hWnd)
{
	/* "1.0", 0.5, 2.0 */
	pGameOpts->brightness = g_nBrightIndex / 20.0 + 0.1;
	
}

static void AssignPauseBright(HWND hWnd)
{
	/* "0.65", 0.5, 2.0 */
	pGameOpts->pause_brightness = g_nPauseBrightIndex / 20.0 + 0.5;
	
}

static void AssignGamma(HWND hWnd)
{
	pGameOpts->gamma = g_nGammaIndex / 20.0 + 0.1;
}

static void AssignContrast(HWND hWnd)
{
	pGameOpts->contrast = g_nContrastIndex / 20.0 + 0.1;
}

static void AssignFullScreenGamma(HWND hWnd)
{
	pGameOpts->full_screen_gamma = g_nFullScreenGammaIndex / 20.0 + 0.1;
}

static void AssignFullScreenBrightness(HWND hWnd)
{
	pGameOpts->full_screen_brightness = g_nFullScreenBrightnessIndex / 20.0 + 0.1;
}

static void AssignFullScreenContrast(HWND hWnd)
{
	pGameOpts->full_screen_contrast = g_nFullScreenContrastIndex / 20.0 + 0.1;
}

static void AssignBeam(HWND hWnd)
{
	pGameOpts->beam = g_nBeamIndex / 20.0 + 1.0;
}

static void AssignFlicker(HWND hWnd)
{
	pGameOpts->flicker = g_nFlickerIndex;
}

static void AssignJDZ(HWND hWnd)
{
	pGameOpts->joy_deadzone = g_nJDZIndex / 20.0;
}

static void AssignJSAT(HWND hWnd)
{
	pGameOpts->joy_saturation = g_nJSATIndex / 20.0;
}

static void AssignRotate(HWND hWnd)
{
	pGameOpts->ror = 0;
	pGameOpts->rol = 0;
	pGameOpts->rotate = 1;
	pGameOpts->autoror = 0;
	pGameOpts->autorol = 0;

	switch (g_nRotateIndex)
	{
	case 1:  pGameOpts->ror = 1; break;
	case 2:  pGameOpts->rol = 1; break;
	case 3 : pGameOpts->rotate = 0; break;
	case 4 : pGameOpts->autoror = 1; break;
	case 5 : pGameOpts->autorol = 1; break;
		default: break;
	}
}

static void AssignScreen(HWND hWnd)
{
	int nIndex = CB_ERR;

	if (ComboBox_GetCount(hWnd) > 0)
		nIndex = ComboBox_GetCurSel(hWnd);

	if (nIndex == CB_ERR)
		nIndex = 0;

	FreeIfAllocated(&pGameOpts->screens[g_nSelectScreenIndex]);
	pGameOpts->screens[g_nSelectScreenIndex] = mame_strdup(g_sMonitorDeviceName[nIndex]);
}

static void AssignView(HWND hWnd)
{
	const char* ptr = NULL;
	int nIndex = CB_ERR;
	if( ComboBox_GetCount(hWnd) > 0 )
	{
		nIndex = ComboBox_GetCurSel(hWnd);
		if( nIndex != CB_ERR )
			ptr = (const char*)ComboBox_GetItemData(hWnd, nIndex);
	
	}

	FreeIfAllocated(&pGameOpts->views[g_nSelectScreenIndex]);
	if( ptr != NULL )
		pGameOpts->views[g_nSelectScreenIndex] = mame_strdup(ptr);
	else
		//default to auto
		pGameOpts->views[g_nSelectScreenIndex]= mame_strdup("auto");
}


static void AssignInput(HWND hWnd)
{
	int new_length;

	FreeIfAllocated(&pGameOpts->ctrlr);

	new_length = ComboBox_GetLBTextLenA(hWnd,g_nInputIndex);
	if (new_length == CB_ERR)
	{
		dprintf("error getting text len");
		pGameOpts->ctrlr = strdup("Standard");
		return;
	}
	pGameOpts->ctrlr = (char *)malloc(new_length + 1);
	ComboBox_GetLBTextA(hWnd, g_nInputIndex, pGameOpts->ctrlr);
	if (wcscmp(_Unicode(pGameOpts->ctrlr), _UIW(TEXT("Standard"))) == 0)
	{
		FreeIfAllocated(&pGameOpts->ctrlr);
		pGameOpts->ctrlr = mame_strdup("Standard");
	}

}

static void AssignVideo(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nVideoIndex);

	FreeIfAllocated(&pGameOpts->video);
	if (ptr != NULL)
		pGameOpts->video = mame_strdup(ptr);
}



static void AssignD3DVersion(HWND hWnd)
	{
	const int ptr = (int)ComboBox_GetItemData(hWnd, g_nD3DVersionIndex);
	pGameOpts->d3dversion = ptr;
}


static const char *GetBIOSName(int bios_driver, int nIndex)
{
	const rom_entry *rom;

	for (rom = drivers[bios_driver]->rom; !ROMENTRY_ISEND(rom); rom++)
	{
		if (ROMENTRY_ISSYSTEM_BIOS(rom) && (ROM_GETBIOSFLAGS(rom) - 1 == nIndex))
			return ROM_GETHASHDATA(rom);
	}

	return NULL;
}

static void AssignBios(HWND hWnd)
{
	FreeIfAllocated(&pGameOpts->bios);

	if (g_biosinfo != -1 && g_nBiosIndex)
		pGameOpts->bios = strdup(GetBIOSName(g_biosinfo, g_nBiosIndex));
	else
		pGameOpts->bios = mame_strdup(BIOS_DEFAULT);
}

#ifdef JOYSTICK_ID
#define AssignJoyid(n)	\
static void AssignJoyid##n(HWND hWnd) \
{ \
	int nIndex = CB_ERR; \
 \
	if (ComboBox_GetCount(hWnd) > 0) \
		nIndex = ComboBox_GetCurSel(hWnd); \
 \
	if (nIndex != CB_ERR) \
		pGameOpts->joyid##n = nIndex; \
}

AssignJoyid(1)
AssignJoyid(2)
AssignJoyid(3)
AssignJoyid(4)
AssignJoyid(5)
AssignJoyid(6)
AssignJoyid(7)
AssignJoyid(8)
#endif /* JOYSTICK_ID */

static void AssignPaddle(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nPaddleIndex);
	FreeIfAllocated(&pGameOpts->paddle_device);
	if (ptr != NULL)
		pGameOpts->paddle_device = mame_strdup(ptr);
}

static void AssignADStick(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nADStickIndex);
	FreeIfAllocated(&pGameOpts->adstick_device);
	if (ptr != NULL)
		pGameOpts->adstick_device = mame_strdup(ptr);

}

static void AssignPedal(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nPedalIndex);
	FreeIfAllocated(&pGameOpts->pedal_device);
	if (ptr != NULL)
		pGameOpts->pedal_device = mame_strdup(ptr);
}

static void AssignDial(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nDialIndex);
	FreeIfAllocated(&pGameOpts->dial_device);
	if (ptr != NULL)
		pGameOpts->dial_device = mame_strdup(ptr);
}

static void AssignTrackball(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nTrackballIndex);
	FreeIfAllocated(&pGameOpts->trackball_device);
	if (ptr != NULL)
		pGameOpts->trackball_device = mame_strdup(ptr);
}

static void AssignLightgun(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nLightgunIndex);
	FreeIfAllocated(&pGameOpts->lightgun_device);
	if (ptr != NULL)
		pGameOpts->lightgun_device = mame_strdup(ptr);
}

static void AssignPositional(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nPositionalIndex);
	FreeIfAllocated(&pGameOpts->positional_device);
	if (ptr != NULL)
		pGameOpts->positional_device = mame_strdup(ptr);
}

static void AssignMouse(HWND hWnd)
{
	const char* ptr = (const char*)ComboBox_GetItemData(hWnd, g_nMouseIndex);
	FreeIfAllocated(&pGameOpts->mouse_device);
	if (ptr != NULL)
		pGameOpts->mouse_device = mame_strdup(ptr);
}

#define AssignDefaultBios(i) \
static void AssignDefaultBios##i(HWND hWnd) \
{ \
	int bios_driver = GetSystemBiosDriver(i); \
	if (bios_driver != -1) \
	{ \
		if (default_bios_index[i]) \
			SetDefaultBios(i, GetBIOSName(bios_driver, default_bios_index[i])); \
		else \
			SetDefaultBios(i, BIOS_DEFAULT); \
	} \
}

AssignDefaultBios(0)
AssignDefaultBios(1)
AssignDefaultBios(2)
AssignDefaultBios(3)
AssignDefaultBios(4)
AssignDefaultBios(5)
AssignDefaultBios(6)
AssignDefaultBios(7)
AssignDefaultBios(8)
AssignDefaultBios(9)
AssignDefaultBios(10)
AssignDefaultBios(11)
AssignDefaultBios(12)
AssignDefaultBios(13)
AssignDefaultBios(14)
AssignDefaultBios(15)

static void AssignEffect(HWND hWnd)
{
	int new_length;

	FreeIfAllocated(&pGameOpts->effect);

	new_length = ComboBox_GetLBTextLenA(hWnd,g_nEffectIndex);
	if (new_length == CB_ERR)
	{
		dprintf("error getting text len");
		return;
	}
	pGameOpts->effect = (char *)malloc(new_length + 1);
	ComboBox_GetLBTextA(hWnd, g_nEffectIndex, pGameOpts->effect);
	if (wcscmp(_Unicode(pGameOpts->effect), _UIW(TEXT("None"))) == 0)
	{
		FreeIfAllocated(&pGameOpts->effect);
		pGameOpts->effect = mame_strdup("none");
	}
}

#ifdef USE_SCALE_EFFECTS
static void AssignScaleEffect(HWND hWnd)
{
	FreeIfAllocated(&pGameOpts->scale_effect);
	pGameOpts->scale_effect = mame_strdup(scale_name(g_nScaleEffectIndex));
}
#endif /* USE_SCALE_EFFECTS */

#ifdef TRANS_UI
static void AssignUI_TRANSPARENCY(HWND hWnd)
{
	pGameOpts->ui_transparency = g_nUITransparencyIndex;
}
#endif /* TRANS_UI */


/************************************************************
 * DataMap initializers
 ************************************************************/

/* Initialize local helper variables */
static void ResetDataMap(void)
{
	int i;
	// add the 0.001 to make sure it truncates properly to the integer
	// (we don't want 35.99999999 to be cut down to 35 because of floating point error)
	g_nPrescaleIndex = pGameOpts->prescale;
	g_nGammaIndex           = (int)((pGameOpts->gamma            - 0.1) * 20.0 + 0.001);
	g_nFullScreenGammaIndex = (int)((pGameOpts->full_screen_gamma -0.1)  * 20.0 + 0.001);
	g_nFullScreenBrightnessIndex= (int)((pGameOpts->full_screen_brightness - 0.1) * 20.0 + 0.001);
	g_nFullScreenContrastIndex = (int)((pGameOpts->full_screen_contrast   - 0.1) * 20.0 + 0.001);
	g_nBrightIndex   = (int)((pGameOpts->brightness       - 0.1) * 20.0 + 0.001);
	g_nContrastIndex	= (int)((pGameOpts->contrast         - 0.1) * 20.0 + 0.001);
	g_nPauseBrightIndex     = (int)((pGameOpts->pause_brightness - 0.5) * 20.0 + 0.001);
	g_nBeamIndex            = (int)((pGameOpts->beam             - 1.0) * 20.0 + 0.001);
	g_nFlickerIndex         = (int)( pGameOpts->flicker);
	g_nJDZIndex             = (int)( pGameOpts->joy_deadzone            * 20.0 + 0.001);
	g_nJSATIndex            = (int)( pGameOpts->joy_saturation          * 20.0 + 0.001);
#ifdef TRANS_UI
	g_nUITransparencyIndex  = (int)( pGameOpts->ui_transparency);
#endif /* TRANS_UI */

	// if no controller type was specified or it was standard
	if (pGameOpts->ctrlr == NULL || mame_stricmp(pGameOpts->ctrlr,"Standard") == 0)
	{
		FreeIfAllocated(&pGameOpts->ctrlr);
		pGameOpts->ctrlr = mame_strdup("Standard");
	}
	g_nViewIndex = 0;
	//TODO HOW DO VIEWS work, where do I get the input for the combo from ???	
	for (i = 0; i < NUMVIDEO; i++)
	{
		if( pGameOpts->views[g_nSelectScreenIndex] != NULL )
		{
			if (!mame_stricmp(pGameOpts->views[g_nSelectScreenIndex], ""))
				g_nViewIndex = i;
		}
	}	
	g_nScreenIndex = 0;
	if (pGameOpts->screens[g_nSelectScreenIndex] == NULL
		|| (mame_stricmp(pGameOpts->screens[g_nSelectScreenIndex],"") == 0 )
		|| (mame_stricmp(pGameOpts->screens[g_nSelectScreenIndex],"auto") == 0 ) )
	{
		FreeIfAllocated(&pGameOpts->screens[g_nSelectScreenIndex]);
		pGameOpts->screens[g_nSelectScreenIndex] = mame_strdup("auto");
		g_nScreenIndex = 0;
	}
	else
	{
		//get the selected Index

		g_nScreenIndex = 0;
		// To account for "Auto" on first index
		for (i = 1; g_sMonitorDeviceString[i]; i++)
			if (mame_stricmp(pGameOpts->screens[g_nSelectScreenIndex], g_sMonitorDeviceString[i]) == 0)
			{
				g_nScreenIndex = i;
				break;
			}
	}

	g_nRotateIndex = 0;
	if (pGameOpts->ror == TRUE && pGameOpts->rol == FALSE)
		g_nRotateIndex = 1;
	if (pGameOpts->ror == FALSE && pGameOpts->rol == TRUE)
		g_nRotateIndex = 2;
	if (!pGameOpts->rotate)
		g_nRotateIndex = 3;
	if (pGameOpts->autoror)
		g_nRotateIndex = 4;
	if (pGameOpts->autorol)
		g_nRotateIndex = 5;

	g_nVolumeIndex = pGameOpts->volume + 32;
	g_nPriorityIndex = pGameOpts->priority + 15;
	switch (pGameOpts->samplerate)
	{
		case 11025:  g_nSampleRateIndex = 0; break;
		case 22050:  g_nSampleRateIndex = 1; break;
		case 24000:  g_nSampleRateIndex = 2; break;
		case 48000:  g_nSampleRateIndex = 4; break;
		default:
		case 44100:  g_nSampleRateIndex = 3; break;
	}

	if (pGameOpts->effect == NULL || mame_stricmp(pGameOpts->effect,"none") == 0)
	{
		FreeIfAllocated(&pGameOpts->effect);
		pGameOpts->effect = mame_strdup("none");
	}

	g_biosinfo = -1;
	if (IS_GAME && DriverHasOptionalBios(g_nGame))
		g_biosinfo = DriverBiosIndex(g_nGame);
	else if (IS_FOLDER)
	{
		char *name = _String(g_pFolder);
		int len = strlen(name) - 2;
		if (!stricmp(name + len, ".c"))
		{
			int n;

			name[len] = '\0';

			n = GetGameNameIndex(name);
			if (DriverIsBios(n))
				g_biosinfo = n;
		}
	}

	g_nBiosIndex = 0;
	if (g_biosinfo != -1)
	{
		set_core_bios(pGameOpts->bios);
		g_nBiosIndex = determine_bios_rom(drivers[g_biosinfo]->rom) - 1;
		set_core_bios(NULL);
	}

	if (IS_GLOBAL)
	{
		for (i = 0; i < MAX_SYSTEM_BIOS; i++)
		{
			int bios_driver = GetSystemBiosDriver(i);
			if (bios_driver != -1)
			{
				char *name = strdup(GetDefaultBios(i));

				set_core_bios(name);
				default_bios_index[i] = determine_bios_rom(drivers[bios_driver]->rom) - 1;
				set_core_bios(NULL);
				free(name);
			}
		}
	}

#ifdef JOYSTICK_ID
	g_nJoyid[0] = pGameOpts->joyid1;
	g_nJoyid[1] = pGameOpts->joyid2;
	g_nJoyid[2] = pGameOpts->joyid3;
	g_nJoyid[3] = pGameOpts->joyid4;
	g_nJoyid[4] = pGameOpts->joyid5;
	g_nJoyid[5] = pGameOpts->joyid6;
	g_nJoyid[6] = pGameOpts->joyid7;
	g_nJoyid[7] = pGameOpts->joyid8;
#endif /* JOYSTICK_ID */

	g_nVideoIndex = 0;
	for (i = 0; i < NUMVIDEO; i++)
	{
		if (!mame_stricmp(pGameOpts->video, g_ComboBoxVideo[i].m_pData))
			g_nVideoIndex = i;
	}
	g_nViewIndex = 0;
	for (i = 0; i < NUMVIEW; i++)
	{
		if (!mame_stricmp(pGameOpts->views[g_nSelectScreenIndex], g_ComboBoxView[i].m_pData))
			g_nViewIndex = i;
	}
	g_nD3DVersionIndex = 0;
	for (i = 0; i < NUMD3DVERSIONS; i++)
	{
		if (pGameOpts->d3dversion == g_ComboBoxD3DVersion[i].m_pData ) 
			g_nD3DVersionIndex = i;
	}

#ifdef USE_SCALE_EFFECTS
	g_nScaleEffectIndex = 0;
	if (pGameOpts->scale_effect)
		for (i = 0; scale_name(i); i++)
		{
			if (!mame_stricmp(pGameOpts->scale_effect, scale_name(i)))
				g_nScaleEffectIndex = i;
		}
#endif /* USE_SCALE_EFFECTS */

	g_nPaddleIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->paddle_device, g_ComboBoxDevice[i].m_pData))
			g_nPaddleIndex = i;
	}
	g_nADStickIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->adstick_device, g_ComboBoxDevice[i].m_pData))
			g_nADStickIndex = i;
	}
	g_nPedalIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->pedal_device, g_ComboBoxDevice[i].m_pData))
			g_nPedalIndex = i;
	}
	g_nDialIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->dial_device, g_ComboBoxDevice[i].m_pData))
			g_nDialIndex = i;
	}
	g_nTrackballIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->trackball_device, g_ComboBoxDevice[i].m_pData))
			g_nTrackballIndex = i;
	}
	g_nLightgunIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->lightgun_device, g_ComboBoxDevice[i].m_pData))
			g_nLightgunIndex = i;
	}
	g_nPositionalIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->positional_device, g_ComboBoxDevice[i].m_pData))
			g_nPositionalIndex = i;
	}
	g_nMouseIndex = 0;
	for (i = 0; i < NUMDEVICES; i++)
	{
		if (!mame_stricmp(pGameOpts->mouse_device, g_ComboBoxDevice[i].m_pData))
			g_nMouseIndex = i;
	}

}

/* Build the control mapping by adding all needed information to the DataMap */
static void BuildDataMap(void)
{
	InitDataMap();


	ResetDataMap();
	/* video */
	DataMapAdd(IDC_D3D_VERSION,   DM_INT,  CT_COMBOBOX, &g_nD3DVersionIndex,       DM_INT,    &pGameOpts->d3dversion,  0, 0, AssignD3DVersion);
	DataMapAdd(IDC_VIDEO_MODE,    DM_INT,  CT_COMBOBOX, &g_nVideoIndex,            DM_STRING, &pGameOpts->video,       0, 0, AssignVideo);
	DataMapAdd(IDC_PRESCALE,      DM_INT,  CT_SLIDER,   &pGameOpts->prescale,      DM_INT, &pGameOpts->prescale,       0, 0, 0);
	DataMapAdd(IDC_PRESCALEDISP,  DM_NONE, CT_NONE,     NULL,                      DM_INT, &pGameOpts->prescale,       0, 0, 0);
	DataMapAdd(IDC_NUMSCREENS,    DM_INT,  CT_SLIDER,   &pGameOpts->numscreens,    DM_INT, &pGameOpts->numscreens,     0, 0, 0);
	DataMapAdd(IDC_NUMSCREENSDISP,DM_NONE, CT_NONE,     NULL,                      DM_INT, &pGameOpts->numscreens,     0, 0, 0);
	DataMapAdd(IDC_AUTOFRAMESKIP, DM_BOOL, CT_BUTTON,   &pGameOpts->autoframeskip, DM_BOOL, &pGameOpts->autoframeskip, 0, 0, 0);
	DataMapAdd(IDC_FRAMESKIP,     DM_INT,  CT_COMBOBOX, &pGameOpts->frameskip,     DM_INT,  &pGameOpts->frameskip,     0, 0, 0);
	DataMapAdd(IDC_WAITVSYNC,     DM_BOOL, CT_BUTTON,   &pGameOpts->waitvsync,     DM_BOOL, &pGameOpts->waitvsync,     0, 0, 0);
	DataMapAdd(IDC_TRIPLE_BUFFER, DM_BOOL, CT_BUTTON,   &pGameOpts->triplebuffer,  DM_BOOL, &pGameOpts->triplebuffer,  0, 0, 0);
	DataMapAdd(IDC_WINDOWED,      DM_BOOL, CT_BUTTON,   &pGameOpts->window,        DM_BOOL, &pGameOpts->window,        0, 0, 0);
	DataMapAdd(IDC_HWSTRETCH,     DM_BOOL, CT_BUTTON,   &pGameOpts->hwstretch,     DM_BOOL,   &pGameOpts->hwstretch,       0, 0, 0);
	DataMapAdd(IDC_SWITCHRES,     DM_BOOL, CT_BUTTON,   &pGameOpts->switchres,     DM_BOOL, &pGameOpts->switchres,     0, 0, 0);
	DataMapAdd(IDC_MAXIMIZE,      DM_BOOL, CT_BUTTON,   &pGameOpts->maximize,      DM_BOOL, &pGameOpts->maximize,      0, 0, 0);
	DataMapAdd(IDC_KEEPASPECT,    DM_BOOL, CT_BUTTON,   &pGameOpts->keepaspect,    DM_BOOL, &pGameOpts->keepaspect,    0, 0, 0);
	DataMapAdd(IDC_SYNCREFRESH,   DM_BOOL, CT_BUTTON,   &pGameOpts->syncrefresh,   DM_BOOL, &pGameOpts->syncrefresh,   0, 0, 0);
	DataMapAdd(IDC_THROTTLE,      DM_BOOL, CT_BUTTON,   &pGameOpts->throttle,      DM_BOOL, &pGameOpts->throttle,      0, 0, 0);
	DataMapAdd(IDC_FSGAMMA,       DM_INT,  CT_SLIDER,   &g_nFullScreenGammaIndex,  DM_FLOAT, &pGameOpts->full_screen_gamma, 0, 0, AssignFullScreenGamma);
	DataMapAdd(IDC_FSGAMMADISP,   DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->full_screen_gamma, 0, 0, 0);
	DataMapAdd(IDC_FSBRIGHTNESS,  DM_INT,  CT_SLIDER,   &g_nFullScreenBrightnessIndex,DM_FLOAT,&pGameOpts->full_screen_brightness, 0, 0, AssignFullScreenBrightness);
	DataMapAdd(IDC_FSBRIGHTNESSDISP,DM_NONE,CT_NONE,    NULL,                      DM_FLOAT, &pGameOpts->full_screen_brightness,   0, 0, 0);
	DataMapAdd(IDC_FSCONTRAST,    DM_INT,  CT_SLIDER,   &g_nFullScreenContrastIndex,DM_FLOAT,&pGameOpts->full_screen_contrast, 0, 0, AssignFullScreenContrast);
	DataMapAdd(IDC_FSCONTRASTDISP,DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->full_screen_contrast, 0, 0, 0);
	/* pGameOpts->frames_to_display */
	DataMapAdd(IDC_EFFECT,        DM_INT,  CT_COMBOBOX, &g_nEffectIndex,           DM_STRING, &pGameOpts->effect,      0, 0, AssignEffect);
	DataMapAdd(IDC_ASPECTRATIOD,  DM_NONE, CT_NONE,     &pGameOpts->aspects[g_nSelectScreenIndex],     DM_STRING, &pGameOpts->aspects[g_nSelectScreenIndex],     0, 0, 0);
	DataMapAdd(IDC_ASPECTRATION,  DM_NONE, CT_NONE,     &pGameOpts->aspects[g_nSelectScreenIndex],     DM_STRING, &pGameOpts->aspects[g_nSelectScreenIndex],     0, 0, 0);
	DataMapAdd(IDC_SIZES,         DM_NONE, CT_NONE,     &pGameOpts->resolutions[g_nSelectScreenIndex], DM_STRING, &pGameOpts->resolutions[g_nSelectScreenIndex], 0, 0, 0);
	DataMapAdd(IDC_REFRESH,       DM_NONE, CT_NONE,     &pGameOpts->resolutions[g_nSelectScreenIndex], DM_STRING, &pGameOpts->resolutions[g_nSelectScreenIndex], 0, 0, 0);
#ifdef USE_SCALE_EFFECTS
	DataMapAdd(IDC_SCALEEFFECT,   DM_INT,  CT_COMBOBOX, &g_nScaleEffectIndex,      DM_STRING, &pGameOpts->scale_effect,0, 0, AssignScaleEffect);
#endif /* USE_SCALE_EFFECTS */

	// direct3d
	DataMapAdd(IDC_D3D_FILTER,    DM_BOOL, CT_BUTTON,   &pGameOpts->filter,        DM_BOOL,   &pGameOpts->filter,          0, 0, 0);

	/* input */
	DataMapAdd(IDC_DEFAULT_INPUT, DM_INT,  CT_COMBOBOX, &g_nInputIndex,            DM_STRING, &pGameOpts->ctrlr,           0, 0, AssignInput);
	DataMapAdd(IDC_USE_MOUSE,     DM_BOOL, CT_BUTTON,   &pGameOpts->mouse,         DM_BOOL,   &pGameOpts->mouse,           0, 0, 0);   
	DataMapAdd(IDC_JOYSTICK,      DM_BOOL, CT_BUTTON,   &pGameOpts->joystick,      DM_BOOL,   &pGameOpts->joystick,        0, 0, 0);
	DataMapAdd(IDC_JDZ,           DM_INT,  CT_SLIDER,   &g_nJDZIndex,              DM_FLOAT,  &pGameOpts->joy_deadzone,    0, 0, AssignJDZ);
	DataMapAdd(IDC_JDZDISP,       DM_NONE, CT_NONE,     NULL,                      DM_FLOAT,  &pGameOpts->joy_deadzone,    0, 0, 0);
	DataMapAdd(IDC_JSAT,          DM_INT,  CT_SLIDER,   &g_nJSATIndex,             DM_FLOAT,  &pGameOpts->joy_saturation,  0, 0, AssignJSAT);
	DataMapAdd(IDC_JSATDISP,      DM_NONE, CT_NONE,     NULL,                      DM_FLOAT,  &pGameOpts->joy_saturation,  0, 0, 0);
	DataMapAdd(IDC_MULTIKEYBOARD, DM_BOOL, CT_BUTTON,   &pGameOpts->multikeyboard, DM_BOOL,   &pGameOpts->multikeyboard,   0, 0, 0);
	DataMapAdd(IDC_MULTIMOUSE,    DM_BOOL, CT_BUTTON,   &pGameOpts->multimouse,    DM_BOOL,   &pGameOpts->multimouse,      0, 0, 0);
	DataMapAdd(IDC_STEADYKEY,     DM_BOOL, CT_BUTTON,   &pGameOpts->steadykey,     DM_BOOL,   &pGameOpts->steadykey,       0, 0, 0);
	DataMapAdd(IDC_LIGHTGUN,      DM_BOOL, CT_BUTTON,   &pGameOpts->lightgun,      DM_BOOL,   &pGameOpts->lightgun,        0, 0, 0);
	DataMapAdd(IDC_DUAL_LIGHTGUN, DM_BOOL, CT_BUTTON,   &pGameOpts->dual_lightgun, DM_BOOL,   &pGameOpts->dual_lightgun,   0, 0, 0);
	DataMapAdd(IDC_RELOAD,        DM_BOOL, CT_BUTTON,   &pGameOpts->offscreen_reload,DM_BOOL, &pGameOpts->offscreen_reload,0, 0, 0);
#ifdef JOYSTICK_ID
	DataMapAdd(IDC_JOYID1,        DM_INT,  CT_COMBOBOX, &g_nJoyid[0],              DM_INT, &pGameOpts->joyid1,             0, 0, AssignJoyid1);
	DataMapAdd(IDC_JOYID2,        DM_INT,  CT_COMBOBOX, &g_nJoyid[1],              DM_INT, &pGameOpts->joyid2,             0, 0, AssignJoyid2);
	DataMapAdd(IDC_JOYID3,        DM_INT,  CT_COMBOBOX, &g_nJoyid[2],              DM_INT, &pGameOpts->joyid3,             0, 0, AssignJoyid3);
	DataMapAdd(IDC_JOYID4,        DM_INT,  CT_COMBOBOX, &g_nJoyid[3],              DM_INT, &pGameOpts->joyid4,             0, 0, AssignJoyid4);
	DataMapAdd(IDC_JOYID5,        DM_INT,  CT_COMBOBOX, &g_nJoyid[4],              DM_INT, &pGameOpts->joyid5,             0, 0, AssignJoyid5);
	DataMapAdd(IDC_JOYID6,        DM_INT,  CT_COMBOBOX, &g_nJoyid[5],              DM_INT, &pGameOpts->joyid6,             0, 0, AssignJoyid6);
	DataMapAdd(IDC_JOYID7,        DM_INT,  CT_COMBOBOX, &g_nJoyid[6],              DM_INT, &pGameOpts->joyid7,             0, 0, AssignJoyid7);
	DataMapAdd(IDC_JOYID8,        DM_INT,  CT_COMBOBOX, &g_nJoyid[7],              DM_INT, &pGameOpts->joyid8,             0, 0, AssignJoyid8);
#endif /* JOYSTICK_ID */
	/*Controller mapping*/
	DataMapAdd(IDC_PADDLE,        DM_INT, CT_COMBOBOX,  &g_nPaddleIndex,           DM_STRING,&pGameOpts->paddle_device,    0, 0, AssignPaddle);
	DataMapAdd(IDC_ADSTICK,       DM_INT, CT_COMBOBOX,  &g_nADStickIndex,          DM_STRING,&pGameOpts->adstick_device,   0, 0, AssignADStick);
	DataMapAdd(IDC_PEDAL,         DM_INT, CT_COMBOBOX,  &g_nPedalIndex,            DM_STRING,&pGameOpts->pedal_device,     0, 0, AssignPedal);
	DataMapAdd(IDC_DIAL,          DM_INT, CT_COMBOBOX,  &g_nDialIndex,             DM_STRING,&pGameOpts->dial_device,      0, 0, AssignDial);
	DataMapAdd(IDC_TRACKBALL,     DM_INT, CT_COMBOBOX,  &g_nTrackballIndex,        DM_STRING,&pGameOpts->trackball_device, 0, 0, AssignTrackball);
	DataMapAdd(IDC_LIGHTGUNDEVICE,DM_INT, CT_COMBOBOX,  &g_nLightgunIndex,         DM_STRING,&pGameOpts->lightgun_device,  0, 0, AssignLightgun);
	DataMapAdd(IDC_POSITIONALDEVICE,DM_INT, CT_COMBOBOX,&g_nPositionalIndex,       DM_STRING,&pGameOpts->positional_device,0, 0, AssignPositional);
	DataMapAdd(IDC_MOUSEDEVICE,   DM_INT, CT_COMBOBOX,  &g_nMouseIndex,            DM_STRING,&pGameOpts->mouse_device,     0, 0, AssignMouse);


	/* core video */
	DataMapAdd(IDC_BRIGHTCORRECT,    DM_INT,  CT_SLIDER,   &g_nBrightIndex,        DM_FLOAT, &pGameOpts->brightness,       0, 0, AssignBrightCorrect);
	DataMapAdd(IDC_BRIGHTCORRECTDISP,DM_NONE, CT_NONE,     NULL,                   DM_FLOAT, &pGameOpts->brightness,       0, 0, 0);
	DataMapAdd(IDC_PAUSEBRIGHT,      DM_INT,  CT_SLIDER,   &g_nPauseBrightIndex,   DM_FLOAT, &pGameOpts->pause_brightness, 0, 0, AssignPauseBright);
	DataMapAdd(IDC_PAUSEBRIGHTDISP,  DM_NONE, CT_NONE,     NULL,                   DM_FLOAT, &pGameOpts->pause_brightness, 0, 0, 0);
	DataMapAdd(IDC_ROTATE,           DM_INT,  CT_COMBOBOX, &g_nRotateIndex,        DM_INT,   &pGameOpts->ror,              0, 0, AssignRotate);
	DataMapAdd(IDC_FLIPX,            DM_BOOL, CT_BUTTON,   &pGameOpts->flipx,      DM_BOOL,  &pGameOpts->flipx,            0, 0, 0);
	DataMapAdd(IDC_FLIPY,            DM_BOOL, CT_BUTTON,   &pGameOpts->flipy,      DM_BOOL,  &pGameOpts->flipy,            0, 0, 0);
	DataMapAdd(IDC_SCREEN,           DM_INT,  CT_COMBOBOX, &g_nScreenIndex,        DM_STRING,&pGameOpts->screens[g_nSelectScreenIndex], 0, 0, AssignScreen);
	DataMapAdd(IDC_VIEW,             DM_INT,  CT_COMBOBOX, &g_nViewIndex,          DM_STRING,&pGameOpts->views[g_nSelectScreenIndex],   0, 0, AssignView);
	/* debugres */
	DataMapAdd(IDC_GAMMA,         DM_INT,  CT_SLIDER,   &g_nGammaIndex,            DM_FLOAT, &pGameOpts->gamma,            0, 0, AssignGamma);
	DataMapAdd(IDC_GAMMADISP,     DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->gamma,            0, 0, 0);
	DataMapAdd(IDC_CONTRAST,      DM_INT,  CT_SLIDER,   &g_nContrastIndex,         DM_FLOAT, &pGameOpts->contrast,         0, 0, AssignContrast);
	DataMapAdd(IDC_CONTRASTDISP,  DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->contrast,         0, 0, 0);

	/* vector */
	DataMapAdd(IDC_ANTIALIAS,     DM_BOOL, CT_BUTTON,   &pGameOpts->antialias,     DM_BOOL,  &pGameOpts->antialias,        0, 0, 0);
	DataMapAdd(IDC_BEAM,          DM_INT,  CT_SLIDER,   &g_nBeamIndex,             DM_FLOAT, &pGameOpts->beam,             0, 0, AssignBeam);
	DataMapAdd(IDC_BEAMDISP,      DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->beam,             0, 0, 0);
	DataMapAdd(IDC_FLICKER,       DM_INT,  CT_SLIDER,   &g_nFlickerIndex,          DM_FLOAT, &pGameOpts->flicker,          0, 0, AssignFlicker);
	DataMapAdd(IDC_FLICKERDISP,   DM_NONE, CT_NONE,     NULL,                      DM_FLOAT, &pGameOpts->flicker,          0, 0, 0);

	/* sound */
	DataMapAdd(IDC_SAMPLERATE,    DM_INT,  CT_COMBOBOX, &g_nSampleRateIndex,       DM_INT,  &pGameOpts->samplerate,    0, 0, AssignSampleRate);
	DataMapAdd(IDC_SAMPLES,       DM_BOOL, CT_BUTTON,   &pGameOpts->samples,       DM_BOOL, &pGameOpts->samples,       0, 0, 0);
	DataMapAdd(IDC_USE_SOUND,     DM_BOOL, CT_BUTTON,   &pGameOpts->sound,         DM_BOOL, &pGameOpts->sound,         0, 0, 0);
	DataMapAdd(IDC_VOLUME,        DM_INT,  CT_SLIDER,   &g_nVolumeIndex,           DM_INT,  &pGameOpts->volume,        0, 0, AssignVolume);
	DataMapAdd(IDC_VOLUMEDISP,    DM_NONE, CT_NONE,     NULL,                      DM_INT,  &pGameOpts->volume,        0, 0, 0);
#ifdef USE_VOLUME_AUTO_ADJUST
	DataMapAdd(IDC_VOLUME_ADJUST, DM_BOOL, CT_BUTTON,   &pGameOpts->volume_adjust, DM_BOOL, &pGameOpts->volume_adjust, 0, 0, 0);
#endif /* USE_VOLUME_AUTO_ADJUST */
	DataMapAdd(IDC_AUDIO_LATENCY, DM_INT,  CT_SLIDER,   &pGameOpts->audio_latency, DM_INT,  &pGameOpts->audio_latency, 0, 0, 0);
	DataMapAdd(IDC_AUDIO_LATENCY_DISP, DM_NONE,  CT_NONE,   NULL, DM_INT, &pGameOpts->audio_latency, 0, 0, 0);

	/* misc artwork options */
	DataMapAdd(IDC_BACKDROPS,     DM_BOOL, CT_BUTTON,   &pGameOpts->use_backdrops, DM_BOOL, &pGameOpts->use_backdrops,     0, 0, 0);
	DataMapAdd(IDC_OVERLAYS,      DM_BOOL, CT_BUTTON,   &pGameOpts->use_overlays,  DM_BOOL, &pGameOpts->use_overlays,      0, 0, 0);
	DataMapAdd(IDC_BEZELS,        DM_BOOL, CT_BUTTON,   &pGameOpts->use_bezels,    DM_BOOL, &pGameOpts->use_bezels,        0, 0, 0);
	DataMapAdd(IDC_ARTWORK_CROP,  DM_BOOL, CT_BUTTON,   &pGameOpts->artwork_crop,  DM_BOOL, &pGameOpts->artwork_crop,      0, 0, 0);

	/* misc */
	DataMapAdd(IDC_CHEAT,         DM_BOOL, CT_BUTTON,   &pGameOpts->cheat,         DM_BOOL, &pGameOpts->cheat,         0, 0, 0);
/*	DataMapAdd(IDC_DEBUG,       DM_BOOL, CT_BUTTON,   &pGameOpts->mame_debug,    DM_BOOL, &pGameOpts->mame_debug,    0, 0, 0); */
	DataMapAdd(IDC_LOG,           DM_BOOL, CT_BUTTON,   &pGameOpts->log,      DM_BOOL, &pGameOpts->log,      0, 0, 0);
	DataMapAdd(IDC_SLEEP,         DM_BOOL, CT_BUTTON,   &pGameOpts->sleep,         DM_BOOL, &pGameOpts->sleep,         0, 0, 0);
	DataMapAdd(IDC_HIGH_PRIORITY, DM_INT,  CT_SLIDER,   &g_nPriorityIndex,         DM_INT,  &pGameOpts->priority,      0, 0, AssignPriority);
	DataMapAdd(IDC_HIGH_PRIORITYTXT,DM_NONE,CT_NONE,    NULL,                      DM_INT,  &pGameOpts->priority,      0, 0, 0);
	DataMapAdd(IDC_SKIP_GAME_INFO,DM_BOOL,CT_BUTTON,    &pGameOpts->skip_gameinfo, DM_BOOL, &pGameOpts->skip_gameinfo, 0, 0, 0);
#ifdef DRIVER_SWITCH
	{
	int i;
	for (i=0; drivers_table[i].name; i++)
		DataMapAdd(drivers_table[i].ctrl,      DM_NONE, CT_NONE,     &pGameOpts->driver_config, DM_STRING,&pGameOpts->driver_config, 0, 0, 0);
	}
#endif /* DRIVER_SWITCH */
	DataMapAdd(IDC_BIOS,          DM_INT,  CT_COMBOBOX, &g_nBiosIndex,             DM_STRING, &pGameOpts->bios,        0, 0, AssignBios);
	DataMapAdd(IDC_ENABLE_AUTOSAVE, DM_BOOL, CT_BUTTON, &pGameOpts->autosave,      DM_BOOL, &pGameOpts->autosave,      0, 0, 0);
	DataMapAdd(IDC_MULTITHREAD_RENDERING, DM_BOOL, CT_BUTTON, &pGameOpts->multithreading, DM_BOOL, &pGameOpts->multithreading, 0, 0, 0);
	DataMapAdd(IDC_CONFIRM_QUIT,  DM_BOOL, CT_BUTTON,   &pGameOpts->confirm_quit,  DM_BOOL, &pGameOpts->confirm_quit,  0, 0, 0);
#ifdef AUTO_PAUSE_PLAYBACK
	DataMapAdd(IDC_AUTO_PAUSE_PLAYBACK,  DM_BOOL, CT_BUTTON,   &pGameOpts->auto_pause_playback,  DM_BOOL, &pGameOpts->auto_pause_playback,  0, 0, 0);
#endif /* AUTO_PAUSE_PLAYBACK */
#ifdef TRANS_UI
	DataMapAdd(IDC_TRANSUI,       DM_BOOL, CT_BUTTON,   &pGameOpts->use_trans_ui,   DM_BOOL, &pGameOpts->use_trans_ui,   0, 0, 0);
	DataMapAdd(IDC_TRANSPARENCY,  DM_INT,  CT_SLIDER,   &g_nUITransparencyIndex,   DM_INT,  &pGameOpts->ui_transparency, 0, 0, AssignUI_TRANSPARENCY);
	DataMapAdd(IDC_TRANSPARENCYDISP, DM_NONE,  CT_NONE,   NULL, DM_INT,  &pGameOpts->ui_transparency, 0, 0, 0);
#endif /* TRANS_UI */
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	DataMapAdd(IDC_M68K_CORE,     DM_INT,  CT_COMBOBOX, &pGameOpts->m68k_core,     DM_INT,  &pGameOpts->m68k_core,     0, 0, 0);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */

#define DATAMAPADD_BIOS(n) \
	if (GetSystemBiosDriver(n) != -1) \
	{ \
		DataMapAdd(IDC_BIOS1+n, DM_INT, CT_COMBOBOX, \
			&default_bios_index[n], DM_NONE, NULL, 0, 0, \
			AssignDefaultBios##n); \
	}

	/* BIOS */
	if (IS_GLOBAL)
	{
		DATAMAPADD_BIOS(0)
		DATAMAPADD_BIOS(1)
		DATAMAPADD_BIOS(2)
		DATAMAPADD_BIOS(3)
		DATAMAPADD_BIOS(4)
		DATAMAPADD_BIOS(5)
		DATAMAPADD_BIOS(6)
		DATAMAPADD_BIOS(7)
		DATAMAPADD_BIOS(8)
		DATAMAPADD_BIOS(9)
		DATAMAPADD_BIOS(10)
		DATAMAPADD_BIOS(11)
		DATAMAPADD_BIOS(12)
		DATAMAPADD_BIOS(13)
		DATAMAPADD_BIOS(14)
		DATAMAPADD_BIOS(15)
	}

#ifdef MESS
	DataMapAdd(IDC_USE_NEW_UI,    DM_BOOL, CT_BUTTON,   &pGameOpts->mess.use_new_ui,DM_BOOL, &pGameOpts->mess.use_new_ui, 0, 0, 0);
#endif

}

BOOL IsControlOptionValue(HWND hDlg,HWND hwnd_ctrl, options_type *opts )
{
	int control_id = GetControlID(hDlg,hwnd_ctrl);

	// certain controls we need to handle specially
	switch (control_id)
	{
	case IDC_ASPECTRATION :
	{
		int n1=0, n2=0;

		sscanf(pGameOpts->aspects[g_nSelectScreenIndex],"%i",&n1);
		sscanf(opts->aspects[g_nSelectScreenIndex],"%i",&n2);

		return n1 == n2;
	}
	case IDC_ASPECTRATIOD :
	{
		int temp, d1=0, d2=0;

		sscanf(pGameOpts->aspects[g_nSelectScreenIndex],"%i:%i",&temp,&d1);
		sscanf(opts->aspects[g_nSelectScreenIndex],"%i:%i",&temp,&d2);

		return d1 == d2;
	}
	case IDC_SIZES:
	{
		int x1=0,y1=0,x2=0,y2=0;

		if (strcmp(pGameOpts->resolutions[g_nSelectScreenIndex],"auto") == 0 &&
			strcmp(opts->resolutions[g_nSelectScreenIndex],"auto") == 0)
			return TRUE;

			sscanf(pGameOpts->resolutions[g_nSelectScreenIndex],"%d x %d",&x1,&y1);
			sscanf(opts->resolutions[g_nSelectScreenIndex],"%d x %d",&x2,&y2);

		return x1 == x2 && y1 == y2;
	}
	case IDC_ROTATE :
	{
		ReadControl(hDlg,control_id);
	
		return pGameOpts->ror == opts->ror &&
			pGameOpts->rol == opts->rol;

	}
	}
	// most options we can compare using data in the data map
	if (IsControlDifferent(hDlg,hwnd_ctrl,pGameOpts,opts))
		return FALSE;

	return TRUE;
}


static void SetStereoEnabled(HWND hWnd, int nIndex)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverIsStereo(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_STEREO);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		EnableWindow(hCtrl, enabled);
	}
}

static void SetYM3812Enabled(HWND hWnd, int nIndex)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverUsesYM3812(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_USE_FM_YM3812);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		EnableWindow(hCtrl, enabled);
	}
}

static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled)
{
	BOOL enabled = FALSE;
	HWND hCtrl;

	if (nIndex != GLOBAL_OPTIONS && nIndex != FOLDER_OPTIONS)
		enabled = DriverUsesSamples(nIndex);

	hCtrl = GetDlgItem(hWnd, IDC_SAMPLES);
	if (hCtrl)
	{
		if (nIndex == GLOBAL_OPTIONS || nIndex == FOLDER_OPTIONS)
			enabled = TRUE;

		enabled = enabled && bSoundEnabled;
		EnableWindow(hCtrl, enabled);
	}
}

/* Moved here cause it's called in a few places */
static void InitializeOptions(HWND hDlg)
{
	InitializeRefreshUI(hDlg);
	InitializeDisplayModeUI(hDlg);
	InitializeSoundUI(hDlg);
	InitializeSkippingUI(hDlg);
	InitializeRotateUI(hDlg);
	InitializeScreenUI(hDlg);
	InitializeSelectScreenUI(hDlg);
	InitializeDefaultInputUI(hDlg);
	InitializeEffectUI(hDlg);
	InitializeBIOSUI(hDlg);
	InitializeDefaultBIOSUI(hDlg);
	InitializeControllerMappingUI(hDlg);
	InitializeD3DVersionUI(hDlg);
	InitializeVideoUI(hDlg);
	InitializeViewUI(hDlg);
#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
	InitializeM68kCoreUI(hDlg);
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */
#ifdef USE_SCALE_EFFECTS
	InitializeScaleEffectUI(hDlg);
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
	InitializeJoyidUI(hDlg);
#endif /* JOYSTICK_ID */
}

/* Moved here because it is called in several places */
static void InitializeMisc(HWND hDlg)
{
	Button_Enable(GetDlgItem(hDlg, IDC_JOYSTICK), DIJoystick.Available());

	SendDlgItemMessage(hDlg, IDC_GAMMA, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 58)); /* [0.10, 3.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_NUMSCREENS, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(1, 4)); /* [1, 8] in 1 increments, core says upto 8 is supported, but params can only be specified for 4 */

	SendDlgItemMessage(hDlg, IDC_CONTRAST, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 38)); /* [0.10, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_BRIGHTCORRECT, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 38)); /* [0.10, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_PAUSEBRIGHT, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 30)); /* [0.50, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_FSGAMMA, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 58)); /* [0.10, 3.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_FSBRIGHTNESS, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 38)); /* [0.10, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_FSCONTRAST, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 38)); /* [0.10, 2.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_JDZ, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 20)); /* [0.00, 1.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_JSAT, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 20)); /* [0.00, 1.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_FLICKER, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 100)); /* [0.0, 100.0] in 1.0 increments */

	SendDlgItemMessage(hDlg, IDC_BEAM, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 300)); /* [1.00, 16.00] in .05 increments */

	SendDlgItemMessage(hDlg, IDC_VOLUME, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 32)); /* [-32, 0] */
	SendDlgItemMessage(hDlg, IDC_AUDIO_LATENCY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(1, 4)); // [1, 4]
	SendDlgItemMessage(hDlg, IDC_PRESCALE, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(1, 10)); // [1, 10] //10 enough ?
#ifdef TRANS_UI
	SendDlgItemMessage(hDlg, IDC_TRANSPARENCY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 255)); /* [0, 255] in 1.0 increments */
#endif /* TRANS_UI */
	SendDlgItemMessage(hDlg, IDC_HIGH_PRIORITY, TBM_SETRANGE,
				(WPARAM)FALSE,
				(LPARAM)MAKELONG(0, 16)); // [-15, 1]
}

static void OptOnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	if (hwndCtl == GetDlgItem(hwnd, IDC_FLICKER))
	{
		FlickerSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_GAMMA))
	{
		GammaSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_BRIGHTCORRECT))
	{
		BrightCorrectSelectionChange(hwnd);
	}
	if (hwndCtl == GetDlgItem(hwnd, IDC_CONTRAST))
	{
		ContrastSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_PAUSEBRIGHT))
	{
		PauseBrightSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_FSGAMMA))
	{
		FullScreenGammaSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_FSBRIGHTNESS))
	{
		FullScreenBrightnessSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_FSCONTRAST))
	{
		FullScreenContrastSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_BEAM))
	{
		BeamSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_NUMSCREENS))
	{
		NumScreensSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_FLICKER))
	{
		FlickerSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_VOLUME))
	{
		VolumeSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_JDZ))
	{
		JDZSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_JSAT))
	{
		JSATSelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_AUDIO_LATENCY))
	{
		AudioLatencySelectionChange(hwnd);
	}
#ifdef TRANS_UI
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_TRANSPARENCY))
	{
		TransparencySelectionChange(hwnd);
	}
#endif /* TRANS_UI */
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_HIGH_PRIORITY))
	{
		ThreadPrioritySelectionChange(hwnd);
	}
	else
	if (hwndCtl == GetDlgItem(hwnd, IDC_PRESCALE))
	{
		PrescaleSelectionChange(hwnd);
	}


}

/* Handle changes to the Beam slider */
static void BeamSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dBeam;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_BEAM, TBM_GETPOS, 0, 0);

	dBeam = nValue / 20.0 + 1.0;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dBeam);
	Static_SetTextA(GetDlgItem(hwnd, IDC_BEAMDISP), buf);
}

/* Handle changes to the Numscreens slider */
static void NumScreensSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	int iNumScreens;
	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_NUMSCREENS, TBM_GETPOS, 0, 0);

	iNumScreens = nValue;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%d", iNumScreens);
	Static_SetTextA(GetDlgItem(hwnd, IDC_NUMSCREENSDISP), buf);
	//Also Update the ScreenSelect Combo with the new number of screens
	UpdateSelectScreenUI(hwnd );

}

/* Handle changes to the Flicker slider */
static void FlickerSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dFlicker;
	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_FLICKER, TBM_GETPOS, 0, 0);

	dFlicker = nValue;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dFlicker);
	Static_SetTextA(GetDlgItem(hwnd, IDC_FLICKERDISP), buf);
}

/* Handle changes to the Gamma slider */
static void GammaSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dGamma;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_GAMMA, TBM_GETPOS, 0, 0);

	dGamma = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dGamma);
	Static_SetTextA(GetDlgItem(hwnd,	IDC_GAMMADISP), buf);
}

/* Handle changes to the Brightness Correction slider */
static void BrightCorrectSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_BRIGHTCORRECT, TBM_GETPOS, 0, 0);

	dValue = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_BRIGHTCORRECTDISP), buf);
}

/* Handle changes to the Contrast slider */
static void ContrastSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dContrast;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_CONTRAST, TBM_GETPOS, 0, 0);

	dContrast = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dContrast);
	Static_SetTextA(GetDlgItem(hwnd, IDC_CONTRASTDISP), buf);
}



/* Handle changes to the Pause Brightness slider */
static void PauseBrightSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_PAUSEBRIGHT, TBM_GETPOS, 0, 0);

	dValue = nValue / 20.0 + 0.5;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_PAUSEBRIGHTDISP), buf);
}

/* Handle changes to the Fullscreen Gamma slider */
static void FullScreenGammaSelectionChange(HWND hwnd)
{
	char   buf[100];
	int    nValue;
	double dGamma;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_FSGAMMA, TBM_GETPOS, 0, 0);

	dGamma = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf),"%03.2f", dGamma);
	Static_SetTextA(GetDlgItem(hwnd, IDC_FSGAMMADISP), buf);
}

/* Handle changes to the Fullscreen Brightness slider */
static void FullScreenBrightnessSelectionChange(HWND hwnd)
{
	char   buf[100];
	int    nValue;
	double dBrightness;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_FSBRIGHTNESS, TBM_GETPOS, 0, 0);

	dBrightness = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf),"%03.2f", dBrightness);
	Static_SetTextA(GetDlgItem(hwnd, IDC_FSBRIGHTNESSDISP), buf);
}

/* Handle changes to the Fullscreen Contrast slider */
static void FullScreenContrastSelectionChange(HWND hwnd)
{
	char   buf[100];
	int    nValue;
	double dContrast;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_FSCONTRAST, TBM_GETPOS, 0, 0);

	dContrast = nValue / 20.0 + 0.1;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf),"%03.2f", dContrast);
	Static_SetTextA(GetDlgItem(hwnd, IDC_FSCONTRASTDISP), buf);
}

/* Handle changes to the Joystick deadzone slider */
static void JDZSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dJDZ;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_JDZ, TBM_GETPOS, 0, 0);

	dJDZ = nValue / 20.0;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dJDZ);
	Static_SetTextA(GetDlgItem(hwnd, IDC_JDZDISP), buf);
}

/* Handle changes to the Joystick saturation slider */
static void JSATSelectionChange(HWND hwnd)
{
	char   buf[100];
	UINT   nValue;
	double dJSAT;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_JSAT, TBM_GETPOS, 0, 0);

	dJSAT = nValue / 20.0;

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%03.2f", dJSAT);
	Static_SetTextA(GetDlgItem(hwnd, IDC_JSATDISP), buf);
}

/* Handle changes to the Refresh drop down */
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl)
{
	int nCurSelection;

	nCurSelection = ComboBox_GetCurSel(hWndCtrl);
	if (nCurSelection != CB_ERR)
	{
		int nRefresh  = 0;

		nRefresh = ComboBox_GetItemData(hWndCtrl, nCurSelection);

		UpdateDisplayModeUI(hWnd, nRefresh);
	}
}

/* Handle changes to the Volume slider */
static void VolumeSelectionChange(HWND hwnd)
{
	char buf[100];
	int  nValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_VOLUME, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buf, ARRAY_LENGTH(buf), "%ddB", nValue - 32);
	Static_SetTextA(GetDlgItem(hwnd, IDC_VOLUMEDISP), buf);
}

static void AudioLatencySelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendDlgItemMessage(hwnd,IDC_AUDIO_LATENCY, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer, ARRAY_LENGTH(buffer),"%i/5 ~ %i/5", value, value + 1);
	Static_SetTextA(GetDlgItem(hwnd,IDC_AUDIO_LATENCY_DISP),buffer);
}

static void PrescaleSelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendDlgItemMessage(hwnd,IDC_PRESCALE, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer, ARRAY_LENGTH(buffer),"%d",value);
	Static_SetTextA(GetDlgItem(hwnd,IDC_PRESCALEDISP),buffer);

}

static void ThreadPrioritySelectionChange(HWND hwnd)
{
	char buffer[100];
	int value;

	// Get the current value of the control
	value = SendDlgItemMessage(hwnd,IDC_HIGH_PRIORITY, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	snprintf(buffer, ARRAY_LENGTH(buffer),"%i",value-15);
	Static_SetTextA(GetDlgItem(hwnd,IDC_HIGH_PRIORITYTXT),buffer);

}

/* Adjust possible choices in the Screen Size drop down */
static void UpdateDisplayModeUI(HWND hwnd, DWORD dwRefresh)
{
	int                   i;
	char                  buf[100];
	int                   nPick;
	int                   nCount = 0;
	int                   nSelection = 0;
	DWORD                 w = 0, h = 0;
	HWND                  hCtrl = GetDlgItem(hwnd, IDC_SIZES);
	DEVMODEA devmode;
	int                   iCurScreen;
	const char           *devicename;

	if (!hCtrl)
		return;

	/* Find out what is currently selected if anything. */
	nPick = ComboBox_GetCurSel(hCtrl);
	if (nPick != 0 && nPick != CB_ERR)
	{
		ComboBox_GetTextA(GetDlgItem(hwnd, IDC_SIZES), buf, 100);
		if (sscanf(buf, "%lu x %lu", &w, &h) != 2)
		{
			w = 0;
			h = 0;
		}
	}

	/* Remove all items in the list. */
	ComboBox_ResetContent(hCtrl);

	ComboBox_AddString(hCtrl, _UIW(TEXT("Auto")));
	//retrieve the screen Infos
	devmode.dmSize = sizeof(devmode);

	/* get monitor name to enumerate */
	iCurScreen = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SCREEN));
	if (iCurScreen == CB_ERR)
		iCurScreen = 0;
	if (iCurScreen)
		devicename = g_sMonitorDeviceName[iCurScreen];
	else
		devicename = NULL;	/* auto */

	for (i = 0; EnumDisplaySettingsA(devicename, i, &devmode); i++)
	{
		if ((devmode.dmBitsPerPel == 32 ) // Only 32 bit depth supported by core
		&&  (devmode.dmDisplayFrequency == dwRefresh || dwRefresh == 0))
		{
			sprintf(buf, "%li x %li", devmode.dmPelsWidth,
			                          devmode.dmPelsHeight);

			if (ComboBox_FindStringA(hCtrl, 0, buf) == CB_ERR)
			{
				ComboBox_AddStringA(hCtrl, buf);
				nCount++;

				if (w == devmode.dmPelsWidth
				&&  h == devmode.dmPelsHeight)
					nSelection = nCount;
			}
		}
	}
	ComboBox_SetCurSel(hCtrl, nSelection);
}

/* Initialize the Display options to auto mode */
static void InitializeDisplayModeUI(HWND hwnd)
{
	UpdateDisplayModeUI(hwnd, 0);
}

/* Initialize the sound options */
static void InitializeSoundUI(HWND hwnd)
{
	HWND    hCtrl;

	hCtrl = GetDlgItem(hwnd, IDC_SAMPLERATE);
	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "11025");
		ComboBox_AddStringA(hCtrl, "22050");
		ComboBox_AddStringA(hCtrl, "24000");
		ComboBox_AddStringA(hCtrl, "44100");
		ComboBox_AddStringA(hCtrl, "48000");
		ComboBox_SetCurSel(hCtrl, 3);
	}
}

/* Populate the Frame Skipping drop down */
static void InitializeSkippingUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_FRAMESKIP);

	if (hCtrl)
	{
		ComboBox_AddString(hCtrl, _UIW(TEXT("Draw every frame")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 1 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 2 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 3 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 4 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 5 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 6 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 7 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 8 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 9 of 10 frames")));
		ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 10 of 10 frames")));
	}
}

/* Populate the Rotate drop down */
static void InitializeRotateUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_ROTATE);

	if (hCtrl)
	{
		ComboBox_AddString(hCtrl, _UIW(TEXT("Default")));             // 0
		ComboBox_AddString(hCtrl, _UIW(TEXT("Clockwise")));           // 1
		ComboBox_AddString(hCtrl, _UIW(TEXT("Anti-clockwise")));      // 2
		ComboBox_AddString(hCtrl, _UIW(TEXT("None")));                // 3
		ComboBox_AddString(hCtrl, _UIW(TEXT("Auto clockwise")));      // 4
		ComboBox_AddString(hCtrl, _UIW(TEXT("Auto anti-clockwise"))); // 5
	}
}

/* Populate the Video Mode drop down */
static void InitializeVideoUI(HWND hwnd)
{
	HWND    hCtrl;

	hCtrl = GetDlgItem(hwnd, IDC_VIDEO_MODE);
	if (hCtrl)
	{
		int i;
		for (i = 0; i < NUMVIDEO; i++)
		{
			ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxVideo[i].m_pText));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxVideo[i].m_pData);
		}
	}
}

static void InitializeViewUI(HWND hwnd)
{
	HWND    hCtrl;

	hCtrl = GetDlgItem(hwnd, IDC_VIEW);
	if (hCtrl)
	{
		int i;
		for (i = 0; i < NUMVIEW; i++)
		{
			ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxView[i].m_pText));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxView[i].m_pData);
		}
	}
}


/* Populate the D3D Version drop down */
static void InitializeD3DVersionUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_D3D_VERSION);
	if (hCtrl)
	{
		int i;
		for (i = 0; i < NUMD3DVERSIONS; i++)
		{
			ComboBox_InsertString(hCtrl, i, g_ComboBoxD3DVersion[i].m_pText);
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxD3DVersion[i].m_pData);
		}
	}
}

static void UpdateSelectScreenUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_SCREENSELECT);
	if (hCtrl)
	{
		int i, curSel;
		curSel = ComboBox_GetCurSel(hCtrl);
		ComboBox_ResetContent(hCtrl );
		for (i = 0; i < NUMSELECTSCREEN && i < pGameOpts->numscreens ; i++)
		{
			ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxSelectScreen[i].m_pText));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxSelectScreen[i].m_pData);
		}
		// Smaller AMount of screens was selected, so use 0
		if( i< curSel )
			ComboBox_SetCurSel(hCtrl, 0 );
		else
			ComboBox_SetCurSel(hCtrl, curSel );
	}
}

/* Populate the Select Screen drop down */
static void InitializeSelectScreenUI(HWND hwnd)
{
	UpdateSelectScreenUI(hwnd);
}

static void UpdateScreenUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_SCREEN);
	if (hCtrl)
	{
		int i= 0;
		int nSelection  = 0;

		/* Remove all items in the list. */
		ComboBox_ResetContent(hCtrl);
		ComboBox_InsertString(hCtrl, 0, _UIW(TEXT("Auto")));

		//we have to add 1 to account for the "auto" entry
		for (i = 1; g_sMonitorDeviceName[i]; i++)
		{
			ComboBox_InsertStringA(hCtrl, i, g_sMonitorDeviceString[i]); //mamep: use DeviceString
			if (strcmp(pGameOpts->screens[g_nSelectScreenIndex], g_sMonitorDeviceName[i]) == 0)
				nSelection = i;
		}
		ComboBox_SetCurSel(hCtrl, nSelection);
	}
}

/* Populate the Screen drop down */
static void InitializeScreenUI(HWND hwnd)
{
	UpdateScreenUI(hwnd );
}

/* Update the refresh drop down */
static void UpdateRefreshUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_REFRESH);

	if (hCtrl)
	{
		int nCount = 0;
		int i;
		DEVMODEA devmode;
		int iCurScreen;
		const char *devicename;

		/* Remove all items in the list. */
		ComboBox_ResetContent(hCtrl);

		ComboBox_AddString(hCtrl, _UIW(TEXT("Auto")));
		ComboBox_SetItemData(hCtrl, nCount++, 0);

		//retrieve the screen Infos
		devmode.dmSize = sizeof(devmode);

		/* get monitor name to enumerate */
		iCurScreen = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SCREEN));
		if (iCurScreen == CB_ERR)
			iCurScreen = 0;
		if (iCurScreen)
			devicename = g_sMonitorDeviceName[iCurScreen];
		else
			devicename = NULL;	/* auto */

		for (i = 0; EnumDisplaySettingsA(devicename, i, &devmode); i++)
		{
			if (devmode.dmDisplayFrequency >= 10 ) 
			{
				// I have some devmode "vga" which specifes 1 Hz, which is probably bogus, so we filter it out
				char buf[16];

				sprintf(buf, "%li", devmode.dmDisplayFrequency);

				if (ComboBox_FindStringA(hCtrl, 0, buf) == CB_ERR)
				{
					ComboBox_InsertStringA(hCtrl, nCount, buf);
					ComboBox_SetItemData(hCtrl, nCount++, devmode.dmDisplayFrequency);
				}
			}
		}
	}
}

/* Populate the refresh drop down */
static void InitializeRefreshUI(HWND hwnd)
{
	UpdateRefreshUI(hwnd);
}
/* Populate the Default Input drop down */
static void InitializeDefaultInputUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_DEFAULT_INPUT);

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;
	WCHAR ext[MAX_PATH];
	WCHAR root[MAX_PATH];
	WCHAR path[MAX_PATH];

	if (hCtrl)
	{
		ComboBox_AddString(hCtrl, _UIW(TEXT("Standard")));

		swprintf(path, TEXT("%s\\*.*"), GetCtrlrDir());

		hFind = FindFirstFileW(path, &FindFileData);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				// copy the filename
				wcscpy(root, FindFileData.cFileName);

				// find the extension
				_wsplitpath(FindFileData.cFileName, NULL, NULL, NULL, ext);

				// check if it's a cfg file
				if (wcsicmp(ext, TEXT(".cfg")) == 0)
				{
					// and strip off the extension
					root[wcslen(root) - 4] = '\0';

					if (wcsicmp(root, TEXT("Standard")) == 0)
						continue;

					// add it as an option
					ComboBox_AddStringW(hCtrl, root);
				}
			}
			while (FindNextFileW(hFind, &FindFileData) != 0);
			
			FindClose(hFind);
		}
	}
}

static void InitializeEffectUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_EFFECT);

	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;
	WCHAR ext[MAX_PATH];
	WCHAR root[MAX_PATH];
	WCHAR path[MAX_PATH];

	if (hCtrl)
	{
		ComboBox_AddString(hCtrl, _UIW(TEXT("None")));

		swprintf(path, TEXT("%s\\*.*"), GetArtDir());

		hFind = FindFirstFileW(path, &FindFileData);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				// copy the filename
				wcscpy(root, FindFileData.cFileName);

				// find the extension
				_wsplitpath(FindFileData.cFileName, NULL, NULL, NULL, ext);

				// check if it's a cfg file
				if (wcsicmp(ext, TEXT(".png")) == 0)
				{
					// and strip off the extension
					root[wcslen(root) - 4] = '\0';

					// add it as an option
					ComboBox_AddStringW(hCtrl, root);
				}
			}
			while (FindNextFileW(hFind, &FindFileData) != 0);
			
			FindClose(hFind);
		}
	}
}

static void InitializeControllerMappingUI(HWND hwnd)
{
	int i;
	HWND hCtrl = GetDlgItem(hwnd,IDC_PADDLE);
	HWND hCtrl1 = GetDlgItem(hwnd,IDC_ADSTICK);
	HWND hCtrl2 = GetDlgItem(hwnd,IDC_PEDAL);
	HWND hCtrl3 = GetDlgItem(hwnd,IDC_DIAL);
	HWND hCtrl4 = GetDlgItem(hwnd,IDC_TRACKBALL);
	HWND hCtrl5 = GetDlgItem(hwnd,IDC_LIGHTGUNDEVICE);
	HWND hCtrl6 = GetDlgItem(hwnd,IDC_POSITIONALDEVICE);
	HWND hCtrl7 = GetDlgItem(hwnd,IDC_MOUSEDEVICE);

	if (hCtrl)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl1)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl1, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl1, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl2)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl2, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl2, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl3)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl3, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl3, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl4)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl4, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl4, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl5)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl5, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl5, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl6)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl6, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl6, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl7)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			ComboBox_InsertString(hCtrl7, i, _UIW(g_ComboBoxDevice[i].m_pText));
			ComboBox_SetItemData( hCtrl7, i, g_ComboBoxDevice[i].m_pData);
		}
	}
}


static void InitializeBIOSCtrl(HWND hCtrl, int bios_driver)
{
	if (hCtrl && bios_driver != -1)
	{
		int i;

		for (i = 0; i < MAX_SYSTEM_BIOS_ENTRY; i++)
		{
			const rom_entry *rom;

			for (rom = drivers[bios_driver]->rom; !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom) && (ROM_GETBIOSFLAGS(rom) - 1 == i))
				{
					const char *name = ROM_GETHASHDATA(rom);
					const char *description = name + strlen(name) + 1;

					ComboBox_AddStringA(hCtrl, description);
					break;
				}
			}
		}
	}
}

static void InitializeBIOSUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_BIOS);

	if (hCtrl)
		InitializeBIOSCtrl(hCtrl, g_biosinfo);
}

static void InitializeDefaultBIOSUI(HWND hwnd)
{
	int n;

	for (n = 0; n < MAX_SYSTEM_BIOS; n++)
	{
		HWND hCtrl = GetDlgItem(hwnd, IDC_BIOS1 + n);

		if (hCtrl)
			InitializeBIOSCtrl(hCtrl, GetSystemBiosDriver(n));
	}
}


void UpdateBackgroundBrush(HWND hwndTab)
{
	// Check if the application is themed
    if (hThemes)
    {
	    if(fnIsThemed)
		    bThemeActive = fnIsThemed();
    }
	// Destroy old brush
	if (hBkBrush)
		DeleteObject(hBkBrush);

	hBkBrush = NULL;

	// Only do this if the theme is active
	if (bThemeActive)
	{
		RECT rc;
		HDC hDC, hDCMem;
		HBITMAP hBmp, hBmpOld;
		// Get tab control dimensions
		GetWindowRect( hwndTab, &rc);

		// Get the tab control DC
		hDC = GetDC(hwndTab);

		// Create a compatible DC
		hDCMem = CreateCompatibleDC(hDC);
		hBmp = CreateCompatibleBitmap(hDC, 
		                              rc.right - rc.left, rc.bottom - rc.top);
		hBmpOld = (HBITMAP)(SelectObject(hDCMem, hBmp));

		// Tell the tab control to paint in our DC
		SendMessage(hwndTab, WM_PRINTCLIENT, (WPARAM)(hDCMem), 
		            (LPARAM)(PRF_ERASEBKGND | PRF_CLIENT | PRF_NONCLIENT));

		// Create a pattern brush from the bitmap selected in our DC
		hBkBrush = CreatePatternBrush(hBmp);

		// Restore the bitmap
		SelectObject(hDCMem, hBmpOld);

		// Cleanup
		DeleteObject(hBmp);
		DeleteDC(hDCMem);
		ReleaseDC(hwndTab, hDC);
	}
}

#if (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040)
static void InitializeM68kCoreUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_M68K_CORE);

	if (hCtrl)
	{
		ComboBox_AddStringA(hCtrl, "C");
		ComboBox_AddStringA(hCtrl, "DRC");
		ComboBox_AddStringA(hCtrl, "ASM");
	}
}
#endif /* (HAS_M68000 || HAS_M68008 || HAS_M68010 || HAS_M68EC020 || HAS_M68020 || HAS_M68040) */

#ifdef JOYSTICK_ID
/* Populate the Joystick ID drop down */
static void InitializeJoyidUI(HWND hWnd)
{
	HWND hCtrl;
	int i, j, num;

	if (DIJoystick.Available() == FALSE)
		return;

	if (GetJoyGUI() == FALSE)
		DIJoystick.init();

	num = (DIJoystick_GetNumPhysicalJoysticks() < 8) ? DIJoystick_GetNumPhysicalJoysticks() : 8;

	for (i = 0; i < num; i++)
	{
		hCtrl = GetDlgItem(hWnd, IDC_JOYID1 + i);
		if (hCtrl)
		{
			for (j = 0; j < DIJoystick_GetNumPhysicalJoysticks(); j++)
			{
				WCHAR buf[256];

				swprintf(buf, _UIW(TEXT("ID:%d")), j + 1);
				ComboBox_AddString(hCtrl, buf);
			}
		}
	}
}
#endif /* JOYSTICK_ID */

#ifdef USE_SCALE_EFFECTS
/* Populate the scale effect drop down */
 static void InitializeScaleEffectUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_SCALEEFFECT);

	if (hCtrl)
	{
		int i;

		ComboBox_AddString(hCtrl,_UIW(TEXT("None")));

		for (i = 1; scale_name(i); i++)
			ComboBox_AddString(hCtrl,_UIW(_Unicode(scale_desc(i))));
	}
}
#endif /* USE_SCALE_EFFECTS */

#ifdef TRANS_UI
static void TransparencySelectionChange(HWND hwnd)
{
	char buf[100];
	int  nValue;

	/* Get the current value of the control */
	nValue = SendDlgItemMessage(hwnd, IDC_TRANSPARENCY, TBM_GETPOS, 0, 0);

	/* Set the static display to the new value */
	sprintf(buf, "%d", nValue);
	Static_SetTextA(GetDlgItem(hwnd, IDC_TRANSPARENCYDISP), buf);
}
#endif /* TRANS_UI */


/* End of source file */
