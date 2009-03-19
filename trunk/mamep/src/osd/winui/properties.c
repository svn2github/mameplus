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

  Properties.c

    Properties Popup and Misc UI support routines.
    
    Created 8/29/98 by Mike Haaland (mhaaland@hypertech.com)

***************************************************************************/

/***************************************************************************

MSH - 20070809
--
Notes on properties and ini files, reset and reset to default. 
----------------------------------------------------------------------------
Each ini contains a complete option set.

Priority order for option sets (Lowest to Highest):

built-in defaults
program    ini (executable root filename ini)
debug      ini (if running a debug build)
vector     ini (really is vector.ini!)
driver     ini (source code root filename in which this driver is found)
granparent ini (grandparent, not sure these exist, but it is possible)
parent     ini (where parent is the name of the parent driver)
game       ini (where game is the driver name for this game)

To determine which option set to use, start at the top level (lowest
priority), and overlay all higher priority ini's until the desired level
is reached.

The 'default' option set is the next priority higher up the list from
the desired level. For the default (program.ini) level, it is also the
default.

When MAME is run, the desired level is game ini.

Expected Code behavior:
----------------------------------------------------------------------------
This approach requires 3 option sets, 'current', 'original' and 'default'.

'current': used to populate the property pages, and to initialize the
'original' set.

'original': used to evaluate if the 'Reset' button is enabled.
If 'current' matches 'original', the 'Reset' button is disabled,
otherwise it is enabled.

'default': used to evaluate if the 'Restore to Defaults' button is enabled.
If 'current' matches 'default', the 'Restore to Defaults' button is disabled,
otherwise it is enabled.

When editing any option set, the desired level is set to the one being
edited, the default set for that level, is the next lower priority set found.

Upon entering the properties dialog:
a) 'current' is initialized
b) 'original' is initialized by 'current'
c) 'default' is initialized
d) Populate Property pages with 'current'
e) 'Reset' and 'Restore to Defaults' buttons are evaluated.

After any change:
a) 'current' is updated
b) 'Reset' and 'Restore to Defaults' buttons are re-evaluated.

Reset:
a) 'current' is reinitialized to 'original'
b) Re-populate Property pages with 'current'
c) 'Reset' and 'Restore to Defaults' buttons are re-evaluated.

Restore to Defaults:
a) 'current' is reinitialized to 'default'
b) Re-populate Property pages with 'current'
b) 'Reset' and 'Restore to Defaults' buttons are re-evaluated.

Apply:
a) 'original' is reinitialized to 'current'
b) 'Reset' and 'Restore to defaults' are re-evaluated.
c) If they 'current' matches 'default', remove the ini from disk.
   Otherwise, write the ini to disk.

Cancel:
a) Exit the dialog.

OK:
a) If they 'current' matches 'default', remove the ini from disk.
   Otherwise, write the ini to disk.
b) Exit the dialog.


***************************************************************************/

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <ddraw.h>

// standard C headers
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
//#include <tchar.h>

// MAME/MAMEUI headers
#include "driver.h"
#include "info.h"
#include "audit.h"
#include "mui_audit.h"
#include "mui_opts.h"
#include "resource.h"
#include "dijoystick.h"     /* For DIJoystick avalibility. */
#include "mui_util.h"
#include "directdraw.h"
#include "properties.h"
#include "winui.h"
#include "datamap.h"
#include "help.h"
#include "winmain.h"
#include "strconv.h"
#include "winutf8.h"
#include "sound/2413intf.h"
#include "sound/3812intf.h"
#include "sound/samples.h"
#include "sound/vlm5030.h"
#ifdef USE_SCALE_EFFECTS
#include "osdscale.h"
#endif /* USE_SCALE_EFFECTS */
#include "bitmask.h"
#include "treeview.h"
#include "translate.h"
#include "datafile.h"

typedef HANDLE HTHEME;

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#ifdef MESS
// done like this until I figure out a better idea
#include "messopts.h"
#include "resourcems.h"
#include "propertiesms.h"
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

#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef  PropSheet_GetTabControl
#define PropSheet_GetTabControl(d) (HWND)(LRESULT)(int)SendMessage((d),PSM_GETTABCONTROL,0,0)
#endif /* defined(__GNUC__) */

#ifdef USE_IPS
//mamep: ignore ips setting
static core_options * load_options_ex(OPTIONS_TYPE opt_type, int game_num)
{
	core_options *opts = load_options(opt_type, game_num);

	options_set_string(opts, OPTION_IPS, "", OPTION_PRIORITY_CMDLINE);
	return opts;
}

//mamep: keep ips setting
static void save_options_ex(OPTIONS_TYPE opt_type, core_options *opts, int game_num)
{
	core_options *orig = load_options(opt_type, game_num);
	const char *ips = options_get_string(orig, OPTION_IPS);

	if (ips && *ips && !opts)
	{
		OPTIONS_TYPE type = opt_type;

		if (type > OPTIONS_GLOBAL)
			type--;

		opts = load_options(type, game_num);
	}

	if (opts)
		options_set_string(opts, OPTION_IPS, ips, OPTION_PRIORITY_CMDLINE);

	options_free(orig);
	save_options(opt_type, opts, game_num);
}

#define load_options	load_options_ex
#define save_options	save_options_ex
#endif /* USE_IPS */


/***************************************************************
 * Imported function prototypes
 ***************************************************************/

/**************************************************************
 * Local function prototypes
 **************************************************************/

//mamep: translate dialog
static int CALLBACK PropSheetCallbackProc(HWND hDlg, UINT Msg, LPARAM lParam);

static void SetStereoEnabled(HWND hWnd, int nIndex);
static void SetYM3812Enabled(HWND hWnd, int nIndex);
static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled);
static void InitializeOptions(HWND hDlg);
static void InitializeMisc(HWND hDlg);
static void OptOnHScroll(HWND hWnd, HWND hwndCtl, UINT code, int pos);
static void NumScreensSelectionChange(HWND hwnd);
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl);
static void InitializeSoundUI(HWND hwnd);
static void InitializeSkippingUI(HWND hwnd);
static void InitializeRotateUI(HWND hwnd);
static void UpdateSelectScreenUI(HWND hwnd);
static void InitializeSelectScreenUI(HWND hwnd);
static void InitializeD3DVersionUI(HWND hwnd);
static void InitializeVideoUI(HWND hwnd);
static void InitializeEffectUI(HWND hWnd);
static void InitializeBIOSUI(HWND hwnd);
static void InitializeControllerMappingUI(HWND hwnd);
#ifdef USE_SCALE_EFFECTS
static void InitializeScaleEffectUI(HWND hwnd);
#endif /* USE_SCALE_EFFECTS */
#ifdef JOYSTICK_ID
static void InitializeJoyidUI(HWND hWnd);
#endif /* JOYSTICK_ID */
#ifdef TREE_SHEET
static  void MovePropertySheetChildWindows(HWND hWnd, int nDx, int nDy);
static  HTREEITEM GetSheetPageTreeItem(int nPage);
#endif /* TREE_SHEET */
static void UpdateOptions(HWND hDlg, datamap *properties_datamap, core_options *opts);
static void UpdateProperties(HWND hDlg, datamap *properties_datamap, core_options *opts);
static void PropToOptions(HWND hWnd, core_options *o);
static void OptionsToProp(HWND hWnd, core_options *o);
static void SetPropEnabledControls(HWND hWnd);
#if 0 //mamep: use standard combobox
static BOOL SelectEffect(HWND hWnd);
static BOOL ResetEffect(HWND hWnd);
#endif
static BOOL SelectJoystickMap(HWND hWnd);
static BOOL ResetJoystickMap(HWND hWnd);
static BOOL SelectDebugscript(HWND hWnd);
static BOOL ResetDebugscript(HWND hWnd);

static void BuildDataMap(void);
static void ResetDataMap(HWND hWnd);

#if 1 //mamep
static BOOL IsControlOptionValue(HWND hDlg, HWND hwnd_ctrl, core_options *opts, core_options *ref);
#endif

static void UpdateBackgroundBrush(HWND hwndTab);
static HBRUSH hBkBrush;

/**************************************************************
 * Local private variables
 **************************************************************/

/* No longer used by the core, but we need it to predefine configurable screens for all games. */
#ifndef MAX_SCREENS
/* maximum number of screens for one game */
#define MAX_SCREENS					8
#endif

static core_options *pOrigOpts, *pDefaultOpts;
static BOOL orig_uses_defaults;
static core_options *pCurrentOpts = NULL;

//mamep: for coloring of changed elements
static core_options *pOptsGlobal;
static core_options *pOptsVector;
static core_options *pOptsSource;
static datamap *properties_datamap;

static int  g_nGame            = 0;
static int  g_nFolder          = 0;
static int  g_nFolderGame      = 0;
static int  g_nPropertyMode    = 0;
static BOOL g_bUseDefaults     = FALSE;
static BOOL g_bReset           = FALSE;
static BOOL g_bAutoAspect[MAX_SCREENS + 1] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
static BOOL  g_bAutoSnapSize = FALSE;
static HICON g_hIcon = NULL;

// mamep: enumerate all monitors on start up
static WCHAR *g_sMonitorDeviceString[MAX_SCREENS + 2];
static char *g_sMonitorDeviceName[MAX_SCREENS + 2];

#ifdef TREE_SHEET
#define SHEET_TREE_WIDTH 170
static int g_nFirstInitPropertySheet = 0;
static RECT rcTabCtrl;
static HWND hSheetTreeCtrl = NULL;
static HINSTANCE hSheetInstance = 0;
static WNDPROC pfnOldSheetProc = NULL;
static  BOOL bPageTreeSelChangedActive = FALSE;
#endif /* TREE_SHEET */

/* Property sheets */

#define HIGHLIGHT_COLOR RGB(0,196,0)
static HBRUSH highlight_brush = NULL;
static HBRUSH background_brush = NULL;
#define VECTOR_COLOR RGB( 190, 0, 0) //DARK RED
#define FOLDER_COLOR RGB( 0, 128, 0 ) // DARK GREEN
#define PARENT_COLOR RGB( 190, 128, 192 ) // PURPLE
#define GAME_COLOR RGB( 0, 128, 192 ) // DARK BLUE


BOOL PropSheetFilter_Vector(const machine_config *drv, const game_driver *gamedrv)
{
	return isDriverVector(drv);
}

/* Help IDs - moved to auto-generated helpids.c */
extern const DWORD dwHelpIDs[];

static struct ComboBoxVideo
{
	const WCHAR*	m_pText;
	const char*	m_pData;
} g_ComboBoxVideo[] = 
{
	{ TEXT("None"),                 "none"    },
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
	{ TEXT("Version 9"),  9   },
	{ TEXT("Version 8"),  8   },
};

#define NUMD3DVERSIONS ARRAY_LENGTH(g_ComboBoxD3DVersion)

static struct ComboBoxSelectScreen
{
	const WCHAR*	m_pText;
	const int	m_pData;
} g_ComboBoxSelectScreen[] = 
{
	{ TEXT("All screens"),      0    },
	{ TEXT("First screen"),     1    },
	{ TEXT("Second screen"),    2    },
	{ TEXT("Third screen"),     3    },
	{ TEXT("Fourth screen"),    4    },
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
	{ TEXT("None"),                  "none"      },
	{ TEXT("Keyboard"),              "keyboard"  },
	{ TEXT("Mouse"),                 "mouse"     },
	{ TEXT("Joystick"),              "joystick"  },
	{ TEXT("Lightgun"),              "lightgun"  },
};

#define NUMDEVICES ARRAY_LENGTH(g_ComboBoxDevice)

static struct ComboBoxSnapView
{
	const WCHAR*	m_pText;
	const char*		m_pData;
} g_ComboBoxSnapView[] = 
{
	{ TEXT("Internal"),	        "internal"    },
	{ TEXT("Auto"),		        "auto"        },
	{ TEXT("Standard"),         "standard"    }, 
	{ TEXT("Pixel Aspect"),     "pixel"       }, 
	{ TEXT("Cocktail"),         "cocktail"    },
};
#define NUMSNAPVIEW (sizeof(g_ComboBoxSnapView) / sizeof(g_ComboBoxSnapView[0]))

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
	{"decrypted",   IDC_DRV_DECRYPTED},
	{"console",     IDC_DRV_CONSOLE},
	{0}
};
#endif /* DRIVER_SWITCH */


/***************************************************************
 * Public functions
 ***************************************************************/

// mamep: enumerate all monitors on start up
void PropertiesInit(void)
{
	DISPLAY_DEVICEA dd;
	int iMonitors;
	int i;

	iMonitors = DirectDraw_GetNumDisplays(); // this gets the count of monitors attached
	if (iMonitors > MAX_SCREENS)
		iMonitors = MAX_SCREENS;

	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);

	g_sMonitorDeviceString[0] = _UIW(TEXT("Auto"));
	g_sMonitorDeviceName[0] = NULL;

	for (i = 0; i < iMonitors; i++)
	{
		g_sMonitorDeviceString[i + 1] = wstring_from_utf8(DirectDraw_GetDisplayName(i));
		g_sMonitorDeviceName[i + 1] = mame_strdup(DirectDraw_GetDisplayDriver(i));
	}

	g_sMonitorDeviceString[i + 1] = NULL;
	g_sMonitorDeviceName[i + 1] = NULL;
}

// Called by MESS, to avoid MESS specific hacks in properties.c
int PropertiesCurrentGame(HWND hDlg)
{
	return g_nGame;
}

DWORD_PTR GetHelpIDs(void)
{
	return (DWORD_PTR)dwHelpIDs;
}

// This function (and the code that use it) is a gross hack - but at least the vile
// and disgusting global variables are gone, making it less gross than what came before
static int GetSelectedScreen(HWND hWnd)
{
	int nSelectedScreen = 0;
	HWND hCtrl = GetDlgItem(hWnd, IDC_SCREENSELECT);
	if (hCtrl)
	{
		int nCurSel = ComboBox_GetCurSel(hCtrl);
		if (nCurSel != CB_ERR)
			nSelectedScreen = ComboBox_GetItemData(hCtrl, nCurSel);
	}

	if ((nSelectedScreen < 0) || (nSelectedScreen >= MAX_SCREENS + 1))
		nSelectedScreen = 0;
	return nSelectedScreen;

}

static PROPSHEETPAGE *CreatePropSheetPages(HINSTANCE hInst, BOOL bOnlyDefault,
	const game_driver *gamedrv, UINT *pnMaxPropSheets, BOOL isGame )
{
	PROPSHEETPAGE *pspages;
	int maxPropSheets;
	int possiblePropSheets;
	int i;

	i = ( isGame ) ? 0 : 2;

	for (; g_propSheets[i].pfnDlgProc; i++)
		;

	possiblePropSheets = (isGame) ? i + 1 : i - 1;

	pspages = malloc(sizeof(PROPSHEETPAGE) * possiblePropSheets);
	if (!pspages)
		return NULL;

	memset(pspages, 0, sizeof(PROPSHEETPAGE) * possiblePropSheets);

	maxPropSheets = 0;

	i = ( isGame ) ? 0 : 2;

	for (; g_propSheets[i].pfnDlgProc; i++)
	{
		if ((gamedrv != NULL) || g_propSheets[i].bOnDefaultPage)
		{
			machine_config *config = NULL;

			/* Allocate machine config */
			if (gamedrv != NULL)
			{
				config = machine_config_alloc(gamedrv->machine_config);
			}
			if (!gamedrv || !g_propSheets[i].pfnFilterProc || g_propSheets[i].pfnFilterProc(config, gamedrv))
			{
				pspages[maxPropSheets].dwSize      = sizeof(PROPSHEETPAGE);
				pspages[maxPropSheets].dwFlags     = 0;
				pspages[maxPropSheets].hInstance   = hInst;
				pspages[maxPropSheets].pszTemplate = MAKEINTRESOURCE(g_propSheets[i].dwDlgID);
				pspages[maxPropSheets].pfnCallback = NULL;
				pspages[maxPropSheets].lParam      = 0;
				pspages[maxPropSheets].pfnDlgProc  = g_propSheets[i].pfnDlgProc;
				maxPropSheets++;
			}
			if (config != NULL)
			{
				/* Free the structure */
				machine_config_free(config);
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

	// clear globals
	pCurrentOpts = NULL;

	g_nGame = GLOBAL_OPTIONS;

	/* Get default options to populate property sheets */
	pCurrentOpts = load_options(OPTIONS_GLOBAL, g_nGame); //GetDefaultOptions(-1, FALSE);
	pDefaultOpts = load_options(OPTIONS_GLOBAL, g_nGame);
	g_bUseDefaults = FALSE;
	/* Stash the result for comparing later */
	pOrigOpts = CreateGameOptions(OPTIONS_TYPE_GLOBAL);
	options_copy(pOrigOpts, pCurrentOpts);
	orig_uses_defaults = FALSE;
	g_bReset = FALSE;
	g_nPropertyMode = OPTIONS_GLOBAL;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(pshead));

	pspage = CreatePropSheetPages(hInst, TRUE, NULL, &pshead.nPages, FALSE);
	if (!pspage)
		return;

	/* Fill in the property sheet header */
	pshead.hwndParent   = hWnd;
	pshead.dwSize       = sizeof(PROPSHEETHEADER);
	pshead.dwFlags      = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	//mamep: translate dialog
	pshead.pfnCallback  = PropSheetCallbackProc;
	pshead.hInstance    = hInst;
	pshead.pszCaption   = _UIW(TEXT("Default Game"));
	pshead.nStartPage   = 0;
	pshead.pszIcon      = MAKEINTRESOURCE(IDI_MAMEUI_ICON);
	pshead.ppsp         = pspage;

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

/* Initilize the property pages for anything but the Default option set */
void InitPropertyPage(HINSTANCE hInst, HWND hWnd, HICON hIcon, OPTIONS_TYPE opt_type, int folder_id, int game_num)
{
	InitPropertyPageToPage(hInst, hWnd, hIcon, opt_type, folder_id, game_num, PROPERTIES_PAGE);
}

void InitPropertyPageToPage(HINSTANCE hInst, HWND hWnd, HICON hIcon, OPTIONS_TYPE opt_type, int folder_id, int game_num, int start_page )
{
	PROPSHEETHEADER pshead;
	PROPSHEETPAGE   *pspage;
	WCHAR*          w_description = NULL;
	OPTIONS_TYPE    default_type = opt_type;

	if (highlight_brush == NULL)
		highlight_brush = CreateSolidBrush(HIGHLIGHT_COLOR);

	if (background_brush == NULL)
		background_brush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	// Initialize the options

	// Load the current options, this will pickup the highest priority option set.
	pCurrentOpts = load_options(opt_type, game_num);

	// Load the default options, pickup the next lower options set than the current level.
	if (opt_type > OPTIONS_GLOBAL)
	{
		default_type -= 1;
	}
	pDefaultOpts = load_options(default_type, game_num);

	//mamep: for coloring of changed elements
	pOptsGlobal = load_options(OPTIONS_GLOBAL, game_num);
	pOptsVector = load_options(OPTIONS_VECTOR, game_num);
	pOptsSource = load_options(OPTIONS_SOURCE, game_num);

	// Copy current_options to original options
	pOrigOpts = CreateGameOptions(OPTIONS_TYPE_GLOBAL);
	options_copy(pOrigOpts, pCurrentOpts);

	// Copy icon to use for the property pages
	g_hIcon = CopyIcon(hIcon);

	// These MUST be valid, they are used as indicies
	g_nGame = game_num;
	g_nFolder = folder_id;

	// Keep track of OPTIONS_TYPE that was passed in.
	g_nPropertyMode = opt_type;

	// Evaluate if the current set uses the Default set
	g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
	g_bReset = FALSE;
	BuildDataMap();

	ZeroMemory(&pshead, sizeof(PROPSHEETHEADER));

	// Set the game to audio to this game 
	InitGameAudit(game_num);

	// Create the propery sheets
	if( OPTIONS_GAME == opt_type )
	{
		pspage = CreatePropSheetPages(hInst, FALSE, drivers[game_num], &pshead.nPages, TRUE);
	}
	else
	{
		//mamep: don't show Vector tab for raster properties
		pspage = CreatePropSheetPages(hInst, FALSE, drivers[game_num], &pshead.nPages, FALSE);
	}
	if (!pspage)
		return;


	// Get the description use as the dialog caption.
	switch( opt_type )
	{
	case OPTIONS_GAME:
		w_description = UseLangList() ? _LSTW(driversw[g_nGame]->description) : driversw[g_nGame]->modify_the;
		break;
	case OPTIONS_VECTOR:
	case OPTIONS_SOURCE:
		w_description = GetFolderByID(g_nFolder)->m_lpTitle;
		break;
	case OPTIONS_GLOBAL:
		w_description = _UIW(TEXT("Default Settings"));
		break;
	default:
		return;
	}
	// If we have no descrption, return.
	if( !w_description )
		return;

	/* Fill in the property sheet header */
	pshead.pszCaption = w_description;
	pshead.hwndParent = hWnd;
	pshead.dwSize     = sizeof(PROPSHEETHEADER);
	pshead.dwFlags    = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_PROPTITLE | PSH_USECALLBACK;
	//mamep: translate dialog
	pshead.pfnCallback= PropSheetCallbackProc;
	pshead.hInstance  = hInst;
	pshead.nStartPage = start_page;
	pshead.pszIcon    = MAKEINTRESOURCE(IDI_MAMEUI_ICON);
	pshead.ppsp       = pspage;

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

	//mamep: it doesn't allocate w_description from heap
	//free(w_description);
	free(pspage);
}


/*********************************************************************
 * Local Functions
 *********************************************************************/

/* Build CPU info string */
static LPCWSTR GameInfoCPU(UINT nIndex)
{
	static WCHAR buf[1024] = TEXT("");
	machine_config *config = machine_config_alloc(drivers[nIndex]->machine_config);
	const device_config *cpu;

	ZeroMemory(buf, sizeof(buf));

	cpu = device_list_class_first(config->devicelist, DEVICE_CLASS_CPU_CHIP);
	while (cpu != NULL)
	{
		if (cpu->clock >= 1000000)
		{
			swprintf(&buf[wcslen(buf)], TEXT("%s %d.%06d MHz"),
				 _Unicode(cpu_get_name(cpu)),
				cpu->clock / 1000000,
				cpu->clock % 1000000);
		}
		else
		{
			swprintf(&buf[wcslen(buf)], TEXT("%s %d.%03d kHz"),
				_Unicode(cpu_get_name(cpu)),
				cpu->clock / 1000,
				cpu->clock % 1000);
		}

		wcscat(buf, TEXT("\n"));

		cpu = device_list_class_next(cpu, DEVICE_CLASS_CPU_CHIP);
	}

	/* Free the structure */
	machine_config_free(config);

	return buf;
}

/* Build Sound system info string */
static LPCWSTR GameInfoSound(UINT nIndex)
{
	static WCHAR buf[1024];
	machine_config *config = machine_config_alloc(drivers[nIndex]->machine_config);
	const device_config *sound;

	buf[0] = 0;

	/* iterate over sound chips */
	sound = device_list_class_first(config->devicelist, DEVICE_CLASS_SOUND_CHIP);
	while(sound != NULL)
	{
		int clock,count;
		device_type sound_type_;

		sound_type_ = sound_get_type(sound);
		clock = sound->clock;

		count = 1;
		sound = device_list_class_next(sound, DEVICE_CLASS_SOUND_CHIP);

		/* Matching chips at the same clock are aggregated */
		while (sound != NULL
			&& sound_get_type(sound) == sound_type_
			&& sound->clock == clock)
		{
			count++;
			sound = device_list_class_next(sound, DEVICE_CLASS_SOUND_CHIP);
		}

		if (count > 1)
		{
			swprintf(&buf[wcslen(buf)], TEXT("%dx"), count);
		}

		swprintf(&buf[wcslen(buf)],TEXT("%s"),_Unicode(devtype_get_name(sound_type_)));

		if (clock)
		{
			if (clock >= 1000000)
			{
				swprintf(&buf[wcslen(buf)], TEXT(" %d.%06d MHz"),
					clock / 1000000,
					clock % 1000000);
			}
			else
			{
				swprintf(&buf[wcslen(buf)], TEXT(" %d.%03d kHz"),
						clock / 1000,
						clock % 1000);
			}
		}

		wcscat(buf, TEXT("\n"));
	}
	/* Free the structure */
	machine_config_free(config);

	return buf;
}

//mamep: display more info
/* Build Display info string */
static LPCWSTR GameInfoScreen(UINT nIndex)
{
	static WCHAR buf[1024];
	machine_config *config = machine_config_alloc(drivers[nIndex]->machine_config);
	const device_config *screen = video_screen_first(config);
	const screen_config *scrconfig = screen->inline_config;

	if (screen != NULL)
	{
		if (isDriverVector(config))
		{
			if (drivers[nIndex]->flags & ORIENTATION_SWAP_XY)
			{
				swprintf(buf, _UIW(TEXT("Vector (V) %f Hz (%d colors)")),
						scrconfig->refresh, config->total_colors);
			}
			else
			{
				swprintf(buf, _UIW(TEXT("Vector (H) %f Hz (%d colors)")),
						scrconfig->refresh, config->total_colors);
			}
		}
		else
		{
			if (drivers[nIndex]->flags & ORIENTATION_SWAP_XY)
			{
				swprintf(buf, _UIW(TEXT("%d x %d (V) %f Hz (%d colors)")),
						scrconfig->visarea.max_y - scrconfig->visarea.min_y + 1,
						scrconfig->visarea.max_x - scrconfig->visarea.min_x + 1,
						ATTOSECONDS_TO_HZ(scrconfig->refresh), config->total_colors);
			} else {
				swprintf(buf, _UIW(TEXT("%d x %d (H) %f Hz (%d colors)")),
						scrconfig->visarea.max_x - scrconfig->visarea.min_x + 1,
						scrconfig->visarea.max_y - scrconfig->visarea.min_y + 1,
						ATTOSECONDS_TO_HZ(scrconfig->refresh), config->total_colors);
			}
		}
	}
	/* Free the structure */
	machine_config_free(config);

	return buf;
}

#ifdef MISC_FOLDER
/* Build input information string */
static LPCWSTR GameInfoInput(int nIndex)
{
	static WCHAR buf[1024];
	static WCHAR control[1024];
	int nplayer = DriverNumPlayers(nIndex);
	int nbutton = DriverNumButtons(nIndex);
#if 0 // no space
	int ncoin = 0;
	const WCHAR *service = 0;
	const WCHAR *tilt = 0;
#endif // no space
	int i;

	control[0] = '\0';
	for (i = 0; i < CONTROLLER_MAX; i++)
	{
		if (DriverUsesController(nIndex, i))
		{
			static const WCHAR *name[CONTROLLER_MAX] =
			{
				TEXT("Joystick 2-Way"),
				TEXT("Joystick 4-Way"),
				TEXT("Joystick 8-Way"),
				TEXT("Joystick 16-Way"),
				TEXT("Joystick 2-Way Vertical"),
				TEXT("Double Joystick 2-Way"),
				TEXT("Double Joystick 4-Way"),
				TEXT("Double Joystick 8-Way"),
				TEXT("Double Joystick 16-Way"),
				TEXT("Double Joystick 2-Way Vertical"),
				TEXT("AD Stick"),
				TEXT("Paddle"),
				TEXT("Dial"),
				TEXT("Trackball"),
				TEXT("Lightgun"),
				TEXT("Pedal")
			};

#if 0	// no space
			if (control[0] != '\0')
				wcscat(control, TEXT(" "));
			wcscat(control, _UIW(name[i]));
#else
			wcscpy(control, _UIW(name[i]));
#endif
		}
	}

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

static LPCWSTR GameInfoSaveState(int driver_index)
{
	if (drivers[driver_index]->flags & GAME_SUPPORTS_SAVE)
		return _UIW(TEXT("Supported"));

	return _UIW(TEXT("Unsupported"));
}
#else /* MISC_FOLDER */
/* Build color information string */
static LPCWSTR GameInfoColors(UINT nIndex)
{
	static WCHAR buf[1024];
	machine_config *config = machine_config_alloc(drivers[nIndex]->machine_config);

	ZeroMemory(buf, sizeof(buf));
	swprintf(buf, _UIW(TEXT("%d colors ")), config->total_colors);
	machine_config_free(config);

	return buf;
}
#endif /* !MISC_FOLDER */

/* Build game status string */
LPWSTR GameInfoStatus(int driver_index, BOOL bRomStatus)
{
	static WCHAR buffer[1024];
	int audit_result = GetRomAuditResults(driver_index);
	memset(buffer,0,sizeof(char)*1024);
	if ( bRomStatus )
 	{
		if (IsAuditResultKnown(audit_result) == FALSE)
 		{
			wcscpy(buffer, _UIW(TEXT("Unknown")));
		}
		else if (IsAuditResultYes(audit_result))
		{
			if (DriverIsBroken(driver_index))
 			{
				wcscpy(buffer, _UIW(TEXT("Not working")));
				
				if (drivers[driver_index]->flags & GAME_UNEMULATED_PROTECTION)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game protection isn't fully emulated")));
				}
				if (drivers[driver_index]->flags & GAME_WRONG_COLORS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Colors are completely wrong")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_COLORS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Colors aren't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_GRAPHICS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Video emulation isn't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_NO_SOUND)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game lacks sound")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_SOUND)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Sound emulation isn't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_NO_COCKTAIL)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Screen flipping is not supported")));
				}
				if (drivers[driver_index]->flags & GAME_REQUIRES_ARTWORK)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game requires external artwork files")));
				}
 			}
			else
			{
				wcscpy(buffer, _UIW(TEXT("Working")));
				
				if (drivers[driver_index]->flags & GAME_UNEMULATED_PROTECTION)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game protection isn't fully emulated")));
				}
				if (drivers[driver_index]->flags & GAME_WRONG_COLORS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Colors are completely wrong")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_COLORS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Colors aren't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_GRAPHICS)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Video emulation isn't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_NO_SOUND)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game lacks sound")));
				}
				if (drivers[driver_index]->flags & GAME_IMPERFECT_SOUND)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Sound emulation isn't 100% accurate")));
				}
				if (drivers[driver_index]->flags & GAME_NO_COCKTAIL)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Screen flipping is not supported")));
				}
				if (drivers[driver_index]->flags & GAME_REQUIRES_ARTWORK)
				{
					if (*buffer != '\0')
						wcscat(buffer, TEXT("\r\n"));
					wcscat(buffer, _UIW(TEXT("Game requires external artwork files")));
				}
			}
		}
		else
		{
			// audit result is no
#if 0 //def MESS
			wcscpy(buffer, _UIW(TEXT("BIOS missing")));
#else
			wcscpy(buffer, _UIW(TEXT("ROMs missing")));
#endif
		}
	}
	else
	{
		//Just show the emulation flags
		if (DriverIsBroken(driver_index))
		{
			wcscpy(buffer, _UIW(TEXT("Not working")));
		}
		else
		{
			wcscpy(buffer, _UIW(TEXT("Working")));
		}	
		if (drivers[driver_index]->flags & GAME_UNEMULATED_PROTECTION)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Game protection isn't fully emulated")));
		}
		if (drivers[driver_index]->flags & GAME_WRONG_COLORS)
		{
		if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Colors are completely wrong")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_COLORS)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Colors aren't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_GRAPHICS)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Video emulation isn't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_NO_SOUND)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Game lacks sound")));
		}
		if (drivers[driver_index]->flags & GAME_IMPERFECT_SOUND)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Sound emulation isn't 100% accurate")));
		}
		if (drivers[driver_index]->flags & GAME_NO_COCKTAIL)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Screen flipping is not supported")));
		}
		if (drivers[driver_index]->flags & GAME_REQUIRES_ARTWORK)
		{
			if (*buffer != '\0')
				wcscat(buffer, TEXT("\r\n"));
			wcscat(buffer, _UIW(TEXT("Game requires external artwork files")));
		}
	}
	return buffer;
}

/* Build game manufacturer string */
static LPCWSTR GameInfoManufactured(UINT nIndex)
{
	static WCHAR buffer[1024];

	snwprintf(buffer, ARRAY_LENGTH(buffer), TEXT("%s %s"), driversw[nIndex]->year, UseLangList()? _MANUFACTW(driversw[nIndex]->manufacturer) : driversw[nIndex]->manufacturer);
	return buffer;
}

//mamep: display more info
/* Build Game title string */
LPWSTR GameInfoTitle(OPTIONS_TYPE opt_type, UINT nIndex)
{
	static WCHAR info[1024];
	static WCHAR desc[1024];

	if (OPTIONS_GLOBAL == opt_type)
		return _UIW(TEXT("Global game options\nDefault options used by all games"));

	if (OPTIONS_VECTOR == opt_type)
		return _UIW(TEXT("Global vector options\nCustom options used by all games in the Vector"));

	if (OPTIONS_SOURCE == opt_type)
	{
		LPTREEFOLDER folder = GetFolderByID(g_nFolder);

		swprintf(info, _UIW(TEXT("Global driver options\nCustom options used by all games in the %s")), folder->m_lpTitle);
		return info;
	}

	swprintf(desc, TEXT("%s [%s]"),
	        UseLangList() ? _LSTW(driversw[nIndex]->description) :
        	                driversw[nIndex]->modify_the,
		driversw[nIndex]->name);

	if (!DriverIsBios(nIndex))
		return desc;

	swprintf(info, _UIW(TEXT("Global BIOS options\nCustom options used by all games in the %s")), desc);
	return info;
}

/* Build game clone infromation string */
static LPCWSTR GameInfoCloneOf(UINT nIndex)
{
	static WCHAR buf[1024];
	int nParentIndex= -1;

	buf[0] = '\0';

	if (DriverIsClone(nIndex))
	{
		nParentIndex = GetParentIndex(drivers[nIndex]);
		swprintf(buf, TEXT("%s [%s]"),
			ConvertAmpersandString(UseLangList()?
				_LSTW(driversw[nParentIndex]->description):
				driversw[nParentIndex]->modify_the),
			driversw[nParentIndex]->name);
	}

	return buf;
}

static LPCWSTR GameInfoSource(UINT nIndex)
{
	return GetDriverFilename(nIndex);
}

//mamep: translate dialog
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

		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE),         GameInfoTitle(g_nPropertyMode, g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_MANUFACTURED),  GameInfoManufactured(g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_STATUS),        GameInfoStatus(g_nGame, FALSE));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_CPU),           GameInfoCPU(g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOUND),         GameInfoSound(g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_SCREEN),        GameInfoScreen(g_nGame));
#ifdef MISC_FOLDER
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_SAVESTATE),     GameInfoSaveState(g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_INPUT),         GameInfoInput(g_nGame));
#else /* MISC_FOLDER */
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_COLORS),        GameInfoColors(g_nGame));
#endif /* !MISC_FOLDER */
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_CLONEOF),       GameInfoCloneOf(g_nGame));
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_SOURCE),        GameInfoSource(g_nGame));

		if (DriverIsClone(g_nGame))
		{
			ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_SHOW);
		}
		else
		{
			ShowWindow(GetDlgItem(hDlg, IDC_PROP_CLONEOF_TEXT), SW_HIDE);
		}
		hWnd = PropSheet_GetTabControl(GetParent(hDlg));
		UpdateBackgroundBrush(hWnd);
		ShowWindow(hDlg, SW_SHOW);
		return 1;

	}
	return 0;
}

/* Handle all options property pages */
INT_PTR CALLBACK GameOptionsProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
//	RECT rc;
//	int nParentIndex = -1;
	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, TRUE);

#ifdef TREE_SHEET
		if (GetShowTreeSheet())
			ModifyPropertySheetForTreeSheet(hDlg);
#endif /* TREE_SHEET */

		/* Fill in the Game info at the top of the sheet */
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE), GameInfoTitle(g_nPropertyMode, g_nGame));
		InitializeOptions(hDlg);
		InitializeMisc(hDlg);

		UpdateProperties(hDlg, properties_datamap, pCurrentOpts);

		g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
		g_bReset = options_equal(pCurrentOpts, pOrigOpts) ? FALSE : TRUE;

		if (g_nGame == GLOBAL_OPTIONS)
			ShowWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), SW_HIDE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);

#ifdef DRIVER_SWITCH
		// if driver switch config is in MISC page
		{
			int i;
			for (i=0; drivers_table[i].name; i++)
				ShowWindow(GetDlgItem(hDlg, drivers_table[i].ctrl), (g_nGame == GLOBAL_OPTIONS) ? SW_SHOW : SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_DRV_TEXT), (g_nGame == GLOBAL_OPTIONS) ? SW_SHOW : SW_HIDE);
		}
#endif /* DRIVER_SWITCH */

		EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);
		ShowWindow(hDlg, SW_SHOW);
  
		return 1;

	case WM_HSCROLL:
		/* slider changed */
		HANDLE_WM_HSCROLL(hDlg, wParam, lParam, OptOnHScroll);
		//g_bUseDefaults = FALSE;
		//g_bReset = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), TRUE);
		PropSheet_Changed(GetParent(hDlg), hDlg);

		// make sure everything's copied over, to determine what's changed
		UpdateOptions(hDlg, properties_datamap, pCurrentOpts);

		// redraw it, it might be a new color now
		InvalidateRect((HWND)lParam,NULL,TRUE);

		break;

	case WM_COMMAND:
		{
			/* Below, 'changed' is used to signify the 'Apply'
			 * button should be enabled.
			 */
			WORD wID         = GET_WM_COMMAND_ID(wParam, lParam);
			HWND hWndCtrl    = GET_WM_COMMAND_HWND(wParam, lParam);
			WORD wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);
			BOOL changed     = FALSE;
			int nCurSelection = 0;
			TCHAR szClass[256];

			switch (wID)
			{
			case IDC_REFRESH:
				if (wNotifyCode == LBN_SELCHANGE)
				{
					RefreshSelectionChange(hDlg, hWndCtrl);
					changed = TRUE;
				}
				break;

			case IDC_ASPECT:
				nCurSelection = Button_GetCheck( GetDlgItem(hDlg, IDC_ASPECT));
				if( g_bAutoAspect[GetSelectedScreen(hDlg)] != nCurSelection )
				{
					changed = TRUE;
					g_bAutoAspect[GetSelectedScreen(hDlg)] = nCurSelection;
				}
				break;

			case IDC_SNAPSIZE:
				nCurSelection = Button_GetCheck( GetDlgItem(hDlg, IDC_SNAPSIZE));
				if( g_bAutoSnapSize != nCurSelection )
				{
					changed = TRUE;
					g_bAutoSnapSize = nCurSelection;
				}
				break;

#if 0 //mamep: use standard combobox
			case IDC_SELECT_EFFECT:
				changed = SelectEffect(hDlg);
				break;

			case IDC_RESET_EFFECT:
				changed = ResetEffect(hDlg);
				break;
#endif

			case IDC_SELECT_JOYSTICKMAP:
				changed = SelectJoystickMap(hDlg);
				break;

			case IDC_RESET_JOYSTICKMAP:
				changed = ResetJoystickMap(hDlg);
				break;

			case IDC_SELECT_DEBUGSCRIPT:
				changed = SelectDebugscript(hDlg);
				break;

			case IDC_RESET_DEBUGSCRIPT:
				changed = ResetDebugscript(hDlg);
				break;

			case IDC_PROP_RESET:
				if (wNotifyCode != BN_CLICKED)
					break;

				options_copy(pCurrentOpts, pOrigOpts);
				UpdateProperties(hDlg, properties_datamap, pCurrentOpts);

				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
				g_bReset = FALSE;
				PropSheet_UnChanged(GetParent(hDlg), hDlg);
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				break;

			case IDC_USE_DEFAULT:
				// Copy the pDefaultOpts into pCurrentOpts
				options_copy(pCurrentOpts, pDefaultOpts);
				// repopulate the controls with the new data
				UpdateProperties(hDlg, properties_datamap, pCurrentOpts);
				
				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
				// This evaluates properly
				g_bReset = options_equal(pCurrentOpts, pOrigOpts) ? FALSE : TRUE;
				// Enable/Dispable the Reset to Defaults button
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				// Tell the dialog to enable/disable the apply button.
				if (g_nGame != GLOBAL_OPTIONS)
				{
					if (g_bReset)
					{
						PropSheet_Changed(GetParent(hDlg), hDlg);
					}
					else
					{
						PropSheet_UnChanged(GetParent(hDlg), hDlg);
					}
				}
				break;

				// MSH 20070813 - Update all related controls
			case IDC_SCREENSELECT:
			case IDC_SCREEN:
				// NPW 3-Apr-2007:  Ugh I'm only perpetuating the vile hacks in this code
				if ((wNotifyCode == CBN_SELCHANGE) || (wNotifyCode == CBN_SELENDOK))
				{
					changed = datamap_read_control(properties_datamap, hDlg, pCurrentOpts, wID);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_SIZES);
					//MSH 20070814 - Hate to do this, but its either this, or update each individual
					// control on the SCREEN tab.
					UpdateProperties(hDlg, properties_datamap, pCurrentOpts);
					changed = TRUE;
					/*
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_SCREENSELECT);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_SCREEN);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_REFRESH);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_SIZES);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_VIEW);
					datamap_populate_control(properties_datamap, hDlg, pCurrentOpts, IDC_SWITCHRES);

					if (strcmp(options_get_string(pCurrentOpts, "screen0"), options_get_string(pOrigOpts, "screen0")) ||
						strcmp(options_get_string(pCurrentOpts, "screen1"), options_get_string(pOrigOpts, "screen1")) ||
						strcmp(options_get_string(pCurrentOpts, "screen2"), options_get_string(pOrigOpts, "screen2")) ||
						strcmp(options_get_string(pCurrentOpts, "screen3"), options_get_string(pOrigOpts, "screen3")))
					{
						changed = TRUE;
					}
					*/
				}
				break;
			default:
#if 0 //def MESS
				if (MessPropertiesCommand(hDlg, wNotifyCode, wID, &changed))
					break;
#endif // MESS

				// use default behavior; try to get the result out of the datamap if
				// appropriate
				GetClassName(hWndCtrl, szClass, ARRAY_LENGTH(szClass));
				if (!wcscmp(szClass, WC_COMBOBOX))
				{
					// combo box
					if ((wNotifyCode == CBN_SELCHANGE) || (wNotifyCode == CBN_SELENDOK))
					{
							changed = datamap_read_control(properties_datamap, hDlg, pCurrentOpts, wID);
					}
				}
				else if (!wcscmp(szClass, WC_BUTTON) && (GetWindowLong(hWndCtrl, GWL_STYLE) & BS_CHECKBOX))
				{
					// check box
					changed = datamap_read_control(properties_datamap, hDlg, pCurrentOpts, wID);
				}
				break;
			}

			if (changed == TRUE)
			{
				// make sure everything's copied over, to determine what's changed
				UpdateOptions(hDlg, properties_datamap, pCurrentOpts);
				// enable the apply button
				PropSheet_Changed(GetParent(hDlg), hDlg);
				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
				g_bReset = options_equal(pCurrentOpts, pOrigOpts) ? FALSE : TRUE;
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
			}

			// If we are closing, pCurrentOpts may be null
			if (NULL != pCurrentOpts)
			{
				// make sure everything's copied over, to determine what's changed
				UpdateOptions(hDlg, properties_datamap, pCurrentOpts);
				SetPropEnabledControls(hDlg);
				// redraw it, it might be a new color now
				if (GetDlgItem(hDlg,wID))
					InvalidateRect(GetDlgItem(hDlg,wID),NULL,FALSE);
			}
		}
		break;

	case WM_NOTIFY:
		{
			// Set to true if we are exiting the properites dialog
			BOOL bClosing = ((LPPSHNOTIFY) lParam)->lParam; 

			switch (((NMHDR *) lParam)->code)
			{
				//We'll need to use a CheckState Table 
				//Because this one gets called for all kinds of other things too, and not only if a check is set
			case PSN_SETACTIVE:
				/* Initialize the controls. */
				UpdateProperties(hDlg, properties_datamap, pCurrentOpts);
				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
				g_bReset = options_equal(pCurrentOpts, pOrigOpts) ? FALSE : TRUE;

				// Sync RESET TO DEFAULTS buttons.
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				break;

			case PSN_APPLY:
				// Handle more than one PSN_APPLY, since this proc handles more tha one
				// property sheet and can be called multiple times when it's time to exit,
				// and we may have already freed the core_options.
				if (bClosing)
				{
					if (NULL == pCurrentOpts)
						return TRUE;
				}

				// Read the datamap
				UpdateOptions(hDlg, properties_datamap, pCurrentOpts);

				// Copy current options to orignal options.
				options_copy(pOrigOpts, pCurrentOpts);
				
				// Repopulate the controls?  WTF?  We just read them, they should be fine.
				UpdateProperties(hDlg, properties_datamap, pCurrentOpts);

				// Determine button states.
				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);
				g_bReset = FALSE;

				orig_uses_defaults = g_bUseDefaults;

				// Sync RESET and RESET TO DEFAULTS buttons.
				EnableWindow(GetDlgItem(hDlg, IDC_USE_DEFAULT), (g_bUseDefaults) ? FALSE : TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

				// Save or remove the current options
				save_options(g_nPropertyMode, (g_bUseDefaults) ? NULL : pCurrentOpts, g_nGame);

				// Disable apply button
				PropSheet_UnChanged(GetParent(hDlg), hDlg);
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

				// If we a closing, free the core_options
				if (bClosing)
				{
					if (pCurrentOpts) options_free(pCurrentOpts);
					if (pOrigOpts)    options_free(pOrigOpts);
					if (pDefaultOpts) options_free(pDefaultOpts);
					pCurrentOpts = pOrigOpts = pDefaultOpts = NULL;

					//mamep: for coloring of changed elements
					if (pOptsGlobal) options_free(pOptsGlobal);
					if (pOptsVector) options_free(pOptsVector);
					if (pOptsSource) options_free(pOptsSource);
					pOptsGlobal = pOptsVector = pOptsSource = NULL;
				}
				return TRUE;

			case PSN_KILLACTIVE:
				/* Save Changes to the options here. */
				UpdateOptions(hDlg, properties_datamap, pCurrentOpts);
				// Determine button states.
				g_bUseDefaults = options_equal(pCurrentOpts, pDefaultOpts);

				ResetDataMap(hDlg);
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
				return 1;  

			case PSN_RESET:
				// Reset to the original values. Disregard changes
				options_copy(pCurrentOpts, pOrigOpts);
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
				break;

			case PSN_HELP:
				// User wants help for this property page
				break;
			}
		}
		break;
#if 1 //mamep
	/* FIXME - Not sure what to do here. */
	case WM_CTLCOLORSTATIC :
	case WM_CTLCOLOREDIT :
	{
		RECT rc;

		//Set the Coloring of the elements
		if (GetWindowLong((HWND)lParam, GWL_ID) < 0)
			return 0;

		if (g_nPropertyMode == OPTIONS_GLOBAL)
		{
			//Normal Black case
			SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
		}
		else if (IsControlOptionValue(hDlg, (HWND)lParam, pCurrentOpts, pOptsGlobal))
		{
			//Normal Black case
			SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
		}
		else if (IsControlOptionValue(hDlg,(HWND)lParam, pCurrentOpts, pOptsVector) && DriverIsVector(g_nGame))
		{
			SetTextColor((HDC)wParam,VECTOR_COLOR);
		}
		else if (IsControlOptionValue(hDlg,(HWND)lParam, pCurrentOpts, pOptsSource))
		{
			SetTextColor((HDC)wParam,FOLDER_COLOR);
		}
		else if (IsControlOptionValue(hDlg,(HWND)lParam, pCurrentOpts, pDefaultOpts))
		{
			SetTextColor((HDC)wParam,PARENT_COLOR);
		}
		else if (IsControlOptionValue(hDlg,(HWND)lParam, pCurrentOpts, pOrigOpts))
		{
			SetTextColor((HDC)wParam,GAME_COLOR);
		}
		else
		{
			switch (g_nPropertyMode)
			{
				case OPTIONS_GAME:
					SetTextColor((HDC)wParam,GAME_COLOR);
					break;
				case OPTIONS_SOURCE:
					SetTextColor((HDC)wParam,FOLDER_COLOR);
					break;
				case OPTIONS_VECTOR:
					SetTextColor((HDC)wParam,VECTOR_COLOR);
					break;
				default:
					case OPTIONS_GLOBAL:
					SetTextColor((HDC)wParam,COLOR_WINDOWTEXT);
					break;
			}
		}
		if (Msg == WM_CTLCOLORSTATIC)
		{
			if (SafeIsAppThemed())
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
			{
				SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
			}
		}
		else
			SetBkColor((HDC)wParam, RGB(255,255,255));
		UnrealizeObject(background_brush);
		return (DWORD)background_brush;
			break;
	}
#endif // 0 Not sure what to do here

	case WM_HELP:
		/* User clicked the ? from the upper right on a control */
		HelpFunction(((LPHELPINFO)lParam)->hItemHandle, MAMEUICONTEXTHELP, HH_TP_HELP_WM_HELP, GetHelpIDs());
		break;

	case WM_CONTEXTMENU: 
		HelpFunction((HWND)wParam, MAMEUICONTEXTHELP, HH_TP_HELP_CONTEXTMENU, GetHelpIDs());
		break; 

	}
	EnableWindow(GetDlgItem(hDlg, IDC_PROP_RESET), g_bReset);

	return 0;
}

#ifdef TREE_SHEET
static LRESULT CALLBACK NewSheetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;

	switch (Msg)
	{
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
		}
		break;

	case WM_DESTROY:
		if (hSheetTreeCtrl != NULL)
		{
			DestroyWindow(hSheetTreeCtrl);
			hSheetTreeCtrl = NULL;
		}

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

void ModifyPropertySheetForTreeSheet(HWND hPageDlg)
{
	HWND      hWnd, hTabWnd;
	DWORD     tabStyle;
	int       nPage, nPageCount;
	RECT      rectSheet, rectTree;
	HTREEITEM hItem;
	LONG_PTR  prevProc;

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
	rectSheet.right += SHEET_TREE_WIDTH;
	SetWindowPos(hWnd, HWND_TOP,
			-1, -1,
			rectSheet.right - rectSheet.left, rectSheet.bottom - rectSheet.top - 20,
			SWP_NOZORDER | SWP_NOMOVE);
	CenterWindow(hWnd);

	MovePropertySheetChildWindows(hWnd, SHEET_TREE_WIDTH, - 20);

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

	{
		LPWSTR wstr = wcsdup(TEXT(""));
		TCITEM item;

		item.mask    = TCIF_TEXT;
		item.iImage  = 0;
		item.lParam  = 0;
		item.pszText = wstr;

		SendMessage(hTempTab, TCM_INSERTITEM, 0, (LPARAM)&item);

		free(wstr);
	}

	DestroyWindow(hTempTab);

	rectTree.left   = rcTabCtrl.left + 7;
	rectTree.top    = rcTabCtrl.top  + 7;
	rectTree.right  = rcTabCtrl.left + SHEET_TREE_WIDTH;
	rectTree.bottom = rcTabCtrl.bottom + 10;

	hSheetTreeCtrl = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY,
							TEXT("SysTreeView32"), TEXT("PageTree"),
							WS_TABSTOP | WS_CHILD | WS_VISIBLE | TVS_SHOWSELALWAYS | TVS_LINESATROOT | TVS_NONEVENHEIGHT | TVS_FULLROWSELECT,
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
		lpTvItem = &tvis.item;
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


static void AspectSetOptionName(datamap *map, HWND dialog, HWND control, char *buffer, size_t buffer_size)
{
	int nSelectedScreen = GetSelectedScreen(dialog);

	if (nSelectedScreen > 0)
		snprintf(buffer, buffer_size, "aspect%d", nSelectedScreen - 1);
	else
		strcpy(buffer, WINOPTION_ASPECT);
}

/* Read controls that are not handled in the DataMap */
static void PropToOptions(HWND hWnd, core_options *o)
{
	HWND hCtrl;
	HWND hCtrl2;
	HWND hCtrl3;

	// It appears these are here to clear the global struct we are removing!
	// FIXME
	//if (g_nGame > -1)
	//	SetGameUsesDefaults(g_nGame,g_bUseDefaults);

	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	hCtrl3 = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl && hCtrl2 && hCtrl3)
	{
		char aspect_option[32];
		AspectSetOptionName(NULL, hWnd, NULL, aspect_option, ARRAY_LENGTH(aspect_option));

		if (Button_GetCheck(hCtrl3))
		{
			options_set_string(o, aspect_option, "auto", OPTION_PRIORITY_CMDLINE);
		}
		else
		{
			int n = 0;
			int d = 0;
			TCHAR buffer[200];
			char buffer2[200];

			Edit_GetText(hCtrl,buffer,sizeof(buffer));
			swscanf(buffer,TEXT("%d"),&n);

			Edit_GetText(hCtrl2,buffer,sizeof(buffer));
			swscanf(buffer,TEXT("%d"),&d);

			if (n == 0 || d == 0)
			{
				n = 4;
				d = 3;
			}

			snprintf(buffer2,sizeof(buffer2),"%d:%d",n,d);
			options_set_string(o, aspect_option, buffer2, OPTION_PRIORITY_CMDLINE);
		}
	}
	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_SNAPSIZEWIDTH);
	hCtrl2 = GetDlgItem(hWnd, IDC_SNAPSIZEHEIGHT);
	hCtrl3 = GetDlgItem(hWnd, IDC_SNAPSIZE);
	if (hCtrl && hCtrl2 && hCtrl3)
	{
		if (Button_GetCheck(hCtrl3))
		{
			options_set_string(o, OPTION_SNAPSIZE, "auto", OPTION_PRIORITY_CMDLINE);
		}
		else
		{
			int width = 0;
			int height = 0;
			TCHAR buffer[200];
			char buffer2[200];

			Edit_GetText(hCtrl,buffer,sizeof(buffer));
			swscanf(buffer,TEXT("%d"),&width);

			Edit_GetText(hCtrl2,buffer,sizeof(buffer));
			swscanf(buffer,TEXT("%d"),&height);

			if (width == 0 || height == 0)
			{
				width = 640;
				height = 480;
			}

			snprintf(buffer2,sizeof(buffer2),"%dx%d",width,height);
			options_set_string(o, OPTION_SNAPSIZE, buffer2, OPTION_PRIORITY_CMDLINE);
		}
	}
}

/* Update options from the dialog */
static void UpdateOptions(HWND hDlg, datamap *properties_datamap, core_options *opts)
{
	/* These are always called together, so make one conveniece function. */
	datamap_read_all_controls(properties_datamap, hDlg, opts);
	PropToOptions(hDlg, opts);
}

/* Update the dialog from the options */
static void UpdateProperties(HWND hDlg, datamap *properties_datamap, core_options *opts)
{
	/* These are always called together, so make one conviniece function. */
	datamap_populate_all_controls(properties_datamap, hDlg, opts);
	OptionsToProp(hDlg, opts);
	SetPropEnabledControls(hDlg);
}

/* Populate controls that are not handled in the DataMap */
static void OptionsToProp(HWND hWnd, core_options* o)
{
	HWND hCtrl;
	HWND hCtrl2;
	WCHAR buf[100];
	int  n = 0;
	int  d = 0;
	int  width = 0;
	int  height = 0;

	/* video */

#if 0 //mamep
	/* Setup refresh list based on depth. */
	datamap_update_control(properties_datamap, hWnd, pCurrentOpts, IDC_REFRESH);
	/* Setup Select screen*/
	UpdateSelectScreenUI(hWnd );

	hCtrl = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl)
	{
		Button_SetCheck(hCtrl, g_bAutoAspect[GetSelectedScreen(hWnd)] )	;
	}
#endif

	hCtrl = GetDlgItem(hWnd, IDC_SNAPSIZE);
	if (hCtrl)
	{
		Button_SetCheck(hCtrl, g_bAutoSnapSize );
	}

	/* Bios select list */
	hCtrl = GetDlgItem(hWnd, IDC_BIOS);
	if (hCtrl)
	{
		const char* cBuffer;
		int i = 0;
		int iCount = ComboBox_GetCount( hCtrl );
		for( i=0; i< iCount; i++ )
		{
			cBuffer = (const char*)ComboBox_GetItemData( hCtrl, i );
			if (strcmp(cBuffer, options_get_string(pCurrentOpts, OPTION_BIOS) ) == 0)
			{
				(void)ComboBox_SetCurSel(hCtrl, i);
				break;
			}

		}
	}

#ifdef DRIVER_SWITCH
	{
		char *temp = mame_strdup(options_get_string(o, OPTION_DRIVER_CONFIG));
		UINT32 enabled = 0;
		int i;

		if (temp)
		{
			int i;

			char *p = strtok(temp, ",");

			while (p)
			{
				char *s = core_strtrim(p);	//get individual driver name
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

	/* Setup refresh list based on depth. */
	datamap_update_control(properties_datamap, hWnd, pCurrentOpts, IDC_REFRESH);
	/* Setup Select screen*/
	UpdateSelectScreenUI(hWnd );

	hCtrl = GetDlgItem(hWnd, IDC_ASPECT);
	if (hCtrl)
	{
		char aspect_option[32];
		AspectSetOptionName(NULL, hWnd, NULL, aspect_option, ARRAY_LENGTH(aspect_option));
		if( strcmp(options_get_string(o, aspect_option), "auto") == 0)
		{
			Button_SetCheck(hCtrl, TRUE);
			g_bAutoAspect[GetSelectedScreen(hWnd)] = TRUE;
		}
		else
		{
			Button_SetCheck(hCtrl, FALSE);
			g_bAutoAspect[GetSelectedScreen(hWnd)] = FALSE;
		}
	}

	/* aspect ratio */
	hCtrl  = GetDlgItem(hWnd, IDC_ASPECTRATION);
	hCtrl2 = GetDlgItem(hWnd, IDC_ASPECTRATIOD);
	if (hCtrl && hCtrl2)
	{
		char aspect_option[32];
		AspectSetOptionName(NULL, hWnd, NULL, aspect_option, ARRAY_LENGTH(aspect_option));

		n = 0;
		d = 0;
		if (options_get_string(o, aspect_option))
		{
			if (sscanf(options_get_string(o, aspect_option), "%d:%d", &n, &d) == 2 && n != 0 && d != 0)
			{
				swprintf(buf, TEXT("%d"), n);
				Edit_SetText(hCtrl, buf);
				swprintf(buf, TEXT("%d"), d);
				Edit_SetText(hCtrl2, buf);
			}
			else
			{
				Edit_SetText(hCtrl,  TEXT("4"));
				Edit_SetText(hCtrl2, TEXT("3"));
			}
		}
		else
		{
			Edit_SetText(hCtrl,  TEXT("4"));
			Edit_SetText(hCtrl2, TEXT("3"));
		}
	}
#if 0 //mamep: use standard combobox
	hCtrl = GetDlgItem(hWnd, IDC_EFFECT);
	if (hCtrl) {
		const char* effect = options_get_string(o, WINOPTION_EFFECT);
		if (effect == NULL) {
			effect = "none";
			options_set_string(o, WINOPTION_EFFECT, effect, OPTION_PRIORITY_CMDLINE);
		}
		win_set_window_text_utf8(hCtrl, effect);
	}
#endif
	hCtrl = GetDlgItem(hWnd, IDC_SNAPSIZE);
	if (hCtrl)
	{
		if( strcmp(options_get_string(o, OPTION_SNAPSIZE), "auto") == 0)
		{
			Button_SetCheck(hCtrl, TRUE);
			g_bAutoSnapSize = TRUE;
		}
		else
		{
			Button_SetCheck(hCtrl, FALSE);
			g_bAutoSnapSize = FALSE;
		}
	}
	/* snapshot size */
	hCtrl  = GetDlgItem(hWnd, IDC_SNAPSIZEWIDTH);
	hCtrl2 = GetDlgItem(hWnd, IDC_SNAPSIZEHEIGHT);
	if (hCtrl && hCtrl2)
	{
		n = 0;
		d = 0;
		if (options_get_string(o, OPTION_SNAPSIZE))
		{
			if (sscanf(options_get_string(o, OPTION_SNAPSIZE), "%dx%d", &width, &height) == 2 && width != 0 && height != 0)
			{
				swprintf(buf, TEXT("%d"), width);
				Edit_SetText(hCtrl, buf);
				swprintf(buf, TEXT("%d"), height);
				Edit_SetText(hCtrl2, buf);
			}
			else
			{
				Edit_SetText(hCtrl,  TEXT("640"));
				Edit_SetText(hCtrl2, TEXT("480"));
			}
		}
		else
		{
			Edit_SetText(hCtrl,  TEXT("640"));
			Edit_SetText(hCtrl2, TEXT("480"));
		}
	}
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
	//mamep: gdi is ok
	//BOOL multimon = (DirectDraw_GetNumDisplays() >= 2);
	int joystick_attached = 0;
	int in_window = 0;

	nIndex = g_nGame;

	d3d = !mame_stricmp(options_get_string(pCurrentOpts, WINOPTION_VIDEO), "d3d");
	//mamep: ddraw is FALSE if d3d is selected
	ddraw = !d3d && !mame_stricmp(options_get_string(pCurrentOpts, WINOPTION_VIDEO), "ddraw");
	gdi = !d3d && !ddraw;

	in_window = options_get_bool(pCurrentOpts, WINOPTION_WINDOW);
	Button_SetCheck(GetDlgItem(hWnd, IDC_ASPECT), g_bAutoAspect[GetSelectedScreen(hWnd)] );

	//mamep: control -maximize option
	EnableWindow(GetDlgItem(hWnd, IDC_MAXIMIZE),               in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_WAITVSYNC),              !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_TRIPLE_BUFFER),          !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALE),               !gdi);
	//mamep: added IDC_PRESCALETEXT
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALETEXT),           !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_PRESCALEDISP),           !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_HWSTRETCH),              ddraw && DirectDraw_HasHWStretch());
	//mamep: disable -switchres option if window or gdi
	EnableWindow(GetDlgItem(hWnd, IDC_SWITCHRES),              !in_window && !gdi);
	//mamep: disable -syncrefresh option if gdi
	EnableWindow(GetDlgItem(hWnd, IDC_SYNCREFRESH),            !gdi);
	//mamep: always enable refresh rate
	//EnableWindow(GetDlgItem(hWnd, IDC_REFRESH),                !in_window && !gdi);
	//EnableWindow(GetDlgItem(hWnd, IDC_REFRESHTEXT),            !in_window && !gdi);
	//mamep: disable -full_screen_gamma option if gdi
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMA),                !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMATEXT),            !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSGAMMADISP),            !in_window && !gdi);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESS),           !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESSTEXT),       !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSBRIGHTNESSDISP),       !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRAST),             !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRASTTEXT),         !in_window);
	EnableWindow(GetDlgItem(hWnd, IDC_FSCONTRASTDISP),         !in_window);

	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOTEXT),        !g_bAutoAspect[GetSelectedScreen(hWnd)]);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATION),           !g_bAutoAspect[GetSelectedScreen(hWnd)]);
	EnableWindow(GetDlgItem(hWnd, IDC_ASPECTRATIOD),           !g_bAutoAspect[GetSelectedScreen(hWnd)]);

//	EnableWindow(GetDlgItem(hWnd, IDC_SNAPSIZETEXT), !g_bAutoSnapSize);
	EnableWindow(GetDlgItem(hWnd, IDC_SNAPSIZEHEIGHT), !g_bAutoSnapSize);
	EnableWindow(GetDlgItem(hWnd, IDC_SNAPSIZEWIDTH), !g_bAutoSnapSize);
	EnableWindow(GetDlgItem(hWnd, IDC_SNAPSIZEX), !g_bAutoSnapSize);


	EnableWindow(GetDlgItem(hWnd, IDC_D3D_FILTER),             d3d);
	EnableWindow(GetDlgItem(hWnd, IDC_D3D_VERSION),            d3d);

#if 0 //mamep: gdi is ok
	//Switchres and D3D or ddraw enable the per screen parameters

	EnableWindow(GetDlgItem(hWnd, IDC_NUMSCREENS),            (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_NUMSCREENSDISP),        (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_SCREENSELECT),          (ddraw || d3d) && multimon);
	EnableWindow(GetDlgItem(hWnd, IDC_SCREENSELECTTEXT),      (ddraw || d3d) && multimon);
#endif

	/* Joystick options */
	joystick_attached = DIJoystick.Available();

	Button_Enable(GetDlgItem(hWnd,IDC_JOYSTICK),               joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JDZTEXT),                joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JDZDISP),                joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JDZ),                    joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSATTEXT),               joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSATDISP),               joystick_attached);
	EnableWindow(GetDlgItem(hWnd, IDC_JSAT),                   joystick_attached);

#ifdef JOYSTICK_ID
	if (joystick_attached)
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

	/* Mouse options */
	useart = Button_GetCheck(GetDlgItem(hWnd, IDC_USE_MOUSE));
	//mamep: handle -multimouse option
	EnableWindow(GetDlgItem(hWnd, IDC_MULTIMOUSE),             useart);

	/* Trackball / Mouse options */
	if (nIndex <= -1 || DriverUsesTrackball(nIndex) || DriverUsesLightGun(nIndex))
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),TRUE);
	else
		Button_Enable(GetDlgItem(hWnd,IDC_USE_MOUSE),FALSE);

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
			Button_Enable(GetDlgItem(hWnd,IDC_DUAL_LIGHTGUN),use_lightgun || mouse);
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
	if (g_nPropertyMode == OPTIONS_GAME)
	{
		BOOL has_bios = DriverHasOptionalBIOS(nIndex);

		ShowWindow(GetDlgItem(hWnd, IDC_BIOS), has_bios ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_BIOSTEXT), has_bios ? SW_SHOW : SW_HIDE);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd, IDC_BIOS), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_BIOSTEXT), SW_HIDE);
	}

	if (g_nPropertyMode != OPTIONS_GAME || DriverSupportsSaveState(nIndex))
	{
		Button_Enable(GetDlgItem(hWnd,IDC_ENABLE_AUTOSAVE),TRUE);
	}
	else
	{
		Button_Enable(GetDlgItem(hWnd,IDC_ENABLE_AUTOSAVE),FALSE);
	}

}

//============================================================
//  CONTROL HELPER FUNCTIONS FOR DATA EXCHANGE
//============================================================

static BOOL RotateReadControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	int selected_index = ComboBox_GetCurSel(control);
	int original_selection = 0;

	// Figure out what the original selection value is
	if (options_get_bool(opts, OPTION_ROR) && !options_get_bool(opts, OPTION_ROL))
		original_selection = 1;
	else if (!options_get_bool(opts, OPTION_ROR) && options_get_bool(opts, OPTION_ROL))
		original_selection = 2;
	else if (!options_get_bool(opts, OPTION_ROTATE))
		original_selection = 3;
	else if (options_get_bool(opts, OPTION_AUTOROR))
		original_selection = 4;
	else if (options_get_bool(opts, OPTION_AUTOROL))
		original_selection = 5;

	// Any work to do?  If so, make the changes and return TRUE.
	if (selected_index != original_selection)
	{
		// Set the options based on the new selection.
		options_set_bool(opts, OPTION_ROR,		selected_index == 1, OPTION_PRIORITY_CMDLINE);
		options_set_bool(opts, OPTION_ROL,		selected_index == 2, OPTION_PRIORITY_CMDLINE);
		options_set_bool(opts, OPTION_ROTATE,	selected_index != 3, OPTION_PRIORITY_CMDLINE);
		options_set_bool(opts, OPTION_AUTOROR,	selected_index == 4, OPTION_PRIORITY_CMDLINE);
		options_set_bool(opts, OPTION_AUTOROL,	selected_index == 5, OPTION_PRIORITY_CMDLINE);
		return TRUE;
	}

	// No changes
	return FALSE;
}



static BOOL RotatePopulateControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	int selected_index = 0;
	if (options_get_bool(opts, OPTION_ROR) && !options_get_bool(opts, OPTION_ROL))
		selected_index = 1;
	else if (!options_get_bool(opts, OPTION_ROR) && options_get_bool(opts, OPTION_ROL))
		selected_index = 2;
	else if (!options_get_bool(opts, OPTION_ROTATE))
		selected_index = 3;
	else if (options_get_bool(opts, OPTION_AUTOROR))
		selected_index = 4;
	else if (options_get_bool(opts, OPTION_AUTOROL))
		selected_index = 5;

	(void)ComboBox_SetCurSel(control, selected_index);
	return FALSE;
}


//mamep: added ScreenSetOptionName
static void ScreenSetOptionName(datamap *map, HWND dialog, HWND control, char *buffer, size_t buffer_size)
{
	int nSelectedScreen = GetSelectedScreen(dialog);

	if (nSelectedScreen > 0)
		snprintf(buffer, buffer_size, "screen%d", nSelectedScreen - 1);
	else
		strcpy(buffer, WINOPTION_SCREEN);
}


static BOOL ScreenReadControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	char screen_option_name[32];
	const char *screen_option_value;
	int screen_option_index;

	screen_option_index = ComboBox_GetCurSel(control);
	screen_option_value = (const char *) ComboBox_GetItemData(control, screen_option_index);

	if (screen_option_value == NULL || *screen_option_value == '\0')
		screen_option_value = "auto";

	ScreenSetOptionName(map, dialog, control, screen_option_name, ARRAY_LENGTH(screen_option_name));
	options_set_string(opts, screen_option_name, screen_option_value, OPTION_PRIORITY_CMDLINE);
	return FALSE;
}



// mamep: enumerate all monitors on start up
static BOOL ScreenPopulateControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	int i = 0;
	int nSelection = 0;
	char screen_option[32];
	const char * option = 0;

	/* Remove all items in the list. */
	(void)ComboBox_ResetContent(control);

	ScreenSetOptionName(map, dialog, control, screen_option, ARRAY_LENGTH(screen_option));
	option = options_get_string(opts, screen_option);

	for (i = 0; g_sMonitorDeviceString[i]; i++)
	{
		(void)ComboBox_InsertString(control, i, g_sMonitorDeviceString[i]);
		(void)ComboBox_SetItemData(control, i, g_sMonitorDeviceName[i]);

		if (!option)
		{
			if (!g_sMonitorDeviceName[i])
				nSelection = i;
		}
		else if (g_sMonitorDeviceName[i] && strcmp(option, g_sMonitorDeviceName[i]) == 0)
			nSelection = i;
	}
	(void)ComboBox_SetCurSel(control, nSelection);
	return FALSE;
}



static void ViewSetOptionName(datamap *map, HWND dialog, HWND control, char *buffer, size_t buffer_size)
{
	int nSelectedScreen = GetSelectedScreen(dialog);

	if (nSelectedScreen > 0)
		snprintf(buffer, buffer_size, "view%d", nSelectedScreen - 1);
	else
		strcpy(buffer, WINOPTION_VIEW);
}

static BOOL ViewPopulateControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	int i;
	int selected_index = 0;
	char view_option[32];
	const char *view;

	// determine the view option value
	ViewSetOptionName(map, dialog, control, view_option, ARRAY_LENGTH(view_option));
	view = options_get_string(opts, view_option);

	(void)ComboBox_ResetContent(control);
	for (i = 0; i < NUMVIEW; i++)
	{
		(void)ComboBox_InsertString(control, i, _UIW(g_ComboBoxView[i].m_pText));
		(void)ComboBox_SetItemData(control, i, g_ComboBoxView[i].m_pData);

		if (!strcmp(view, g_ComboBoxView[i].m_pData))
			selected_index = i;
	}
	(void)ComboBox_SetCurSel(control, selected_index);
	return FALSE;
}

static BOOL SnapViewPopulateControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	int i;
	int selected_index = 0;
	const char *snapview;

	// determine the snapview option value
	snapview = options_get_string(opts, OPTION_SNAPVIEW);

	(void)ComboBox_ResetContent(control);
	for (i = 0; i < NUMSNAPVIEW; i++)
	{
		(void)ComboBox_InsertString(control, i, _UIW(g_ComboBoxSnapView[i].m_pText));
		(void)ComboBox_SetItemData(control, i, g_ComboBoxSnapView[i].m_pData);

		if (!strcmp(snapview, g_ComboBoxSnapView[i].m_pData))
			selected_index = i;
	}
	(void)ComboBox_SetCurSel(control, selected_index);
	return FALSE;
}


static BOOL DefaultInputPopulateControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	WCHAR *ext;
	WCHAR root[256];
	WCHAR path[256];
	int selected = 0;
	int index = 0;
	LPCWSTR w_ctrlr_option = 0;
	LPWSTR buf = 0;
	const char *ctrlr_option;
	//TCHAR* t_ctrldir;

	// determine the ctrlr option
	ctrlr_option = options_get_string(opts, OPTION_CTRLR);
	if( ctrlr_option != NULL )
	{
		buf = wstring_from_utf8(ctrlr_option);
		if( !buf )
			return FALSE;
		w_ctrlr_option = buf;
	}
	else
	{
		w_ctrlr_option = TEXT("");
	}

	// reset the controllers dropdown
	(void)ComboBox_ResetContent(control);
	(void)ComboBox_InsertString(control, index, _UIW(TEXT("None")));
	(void)ComboBox_SetItemData(control, index, "");
	index++;

	//t_ctrldir = tstring_from_utf8(GetCtrlrDir());
	//if( !t_ctrldir )
	//{
	//	if( buf )
	//		free(buf);
	//	return FALSE;
	//}

	swprintf (path, TEXT("%s\\*.*"), GetCtrlrDir());

	//free(t_ctrldir);
	
	hFind = FindFirstFileW(path, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			// copy the filename
			wcscpy (root,FindFileData.cFileName);

			// find the extension
			ext = wcsrchr (root,'.');
			if (ext)
			{
				// check if it's a cfg file
				if (wcscmp (ext, TEXT(".cfg")) == 0)
				{
					// and strip off the extension
					*ext = 0;

					// set the option?
					if (!wcscmp(root, w_ctrlr_option))
						selected = index;

					// add it as an option
					(void)ComboBox_InsertString(control, index, root);
					(void)ComboBox_SetItemData(control, index, (void*)utf8_from_wstring(root));	// FIXME - leaks memory!
					index++;
				}
			}
		}
		while (FindNextFileW (hFind, &FindFileData) != 0);
		
		FindClose (hFind);
	}

	(void)ComboBox_SetCurSel(control, selected);
	
	if( buf )
		free(buf);

	return FALSE;
}



static void ResolutionSetOptionName(datamap *map, HWND dialog, HWND control, char *buffer, size_t buffer_size)
{
	int nSelectedScreen = GetSelectedScreen(dialog);

	if (nSelectedScreen > 0)
		snprintf(buffer, buffer_size, "resolution%d", nSelectedScreen - 1);
	else
		strcpy(buffer, WINOPTION_RESOLUTION);
}


static BOOL ResolutionReadControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	HWND refresh_control = GetDlgItem(dialog, IDC_REFRESH);
	HWND sizes_control = GetDlgItem(dialog, IDC_SIZES);
	int refresh_index, refresh_value, width, height;
	char option_value[256];
	TCHAR buffer[256];

	if (refresh_control && sizes_control)
	{
		(void)ComboBox_GetText(sizes_control, buffer, ARRAY_LENGTH(buffer) - 1);
		if (swscanf(buffer, TEXT("%d x %d"), &width, &height) == 2)
		{
			refresh_index = ComboBox_GetCurSel(refresh_control);
			refresh_value = ComboBox_GetItemData(refresh_control, refresh_index);
			snprintf(option_value, ARRAY_LENGTH(option_value), "%dx%d@%d", width, height, refresh_value);
		}
		else
		{
			snprintf(option_value, ARRAY_LENGTH(option_value), "auto");
		}
		options_set_string(opts, option_name, option_value, OPTION_PRIORITY_CMDLINE);
	}
	return FALSE;
}



static BOOL ResolutionPopulateControl(datamap *map, HWND dialog, HWND control_, core_options *opts, const char *option_name)
{
	HWND sizes_control = GetDlgItem(dialog, IDC_SIZES);
	HWND refresh_control = GetDlgItem(dialog, IDC_REFRESH);
	int width, height, refresh;
	const char *option_value;
	int sizes_index = 0;
	int refresh_index = 0;
	int sizes_selection = 0;
	int refresh_selection = 0;
	char screen_option[32];
	const char *screen;
	//LPTSTR t_screen;
	WCHAR buf[16];
	int i;
	DEVMODEA devmode;

	if (sizes_control && refresh_control)
	{
		// determine the resolution
		option_value = options_get_string(opts, option_name);
		if (sscanf(option_value, "%dx%d@%d", &width, &height, &refresh) != 3)
		{
			width = 0;
			height = 0;
			refresh = 0;
		}

		// reset sizes control
		(void)ComboBox_ResetContent(sizes_control);
		(void)ComboBox_InsertString(sizes_control, sizes_index, _UIW(TEXT("Auto")));
		(void)ComboBox_SetItemData(sizes_control, sizes_index, 0);
		sizes_index++;

		// reset refresh control
		(void)ComboBox_ResetContent(refresh_control);
		(void)ComboBox_InsertString(refresh_control, refresh_index, _UIW(TEXT("Auto")));
		(void)ComboBox_SetItemData(refresh_control, refresh_index, 0);
		refresh_index++;

		// determine which screen we're using
		ScreenSetOptionName(map, dialog, NULL, screen_option, ARRAY_LENGTH(screen_option));
		screen = options_get_string(opts, screen_option);
		//t_screen = tstring_from_utf8(screen);
		if (!screen || !*screen || mame_stricmp(screen, "auto") == 0)
			screen = NULL;

		// retrieve screen information
		devmode.dmSize = sizeof(devmode);
		for (i = 0; EnumDisplaySettingsA(screen, i, &devmode); i++)
		{
			if ((devmode.dmBitsPerPel == 32 ) // Only 32 bit depth supported by core
				&&(devmode.dmDisplayFrequency == refresh || refresh == 0))
			{
				snwprintf(buf, ARRAY_LENGTH(buf), TEXT("%li x %li"),
					devmode.dmPelsWidth, devmode.dmPelsHeight);

				if (ComboBox_FindString(sizes_control, 0, buf) == CB_ERR)
				{
					(void)ComboBox_InsertString(sizes_control, sizes_index, buf);

					if ((width == devmode.dmPelsWidth) && (height == devmode.dmPelsHeight))
						sizes_selection = sizes_index;
					sizes_index++;

				}
			}
			if (devmode.dmDisplayFrequency >= 10 ) 
			{
				// I have some devmode "vga" which specifes 1 Hz, which is probably bogus, so we filter it out

				snwprintf(buf, ARRAY_LENGTH(buf), TEXT("%li Hz"), devmode.dmDisplayFrequency);

				if (ComboBox_FindString(refresh_control, 0, buf) == CB_ERR)
				{
					(void)ComboBox_InsertString(refresh_control, refresh_index, buf);
					(void)ComboBox_SetItemData(refresh_control, refresh_index, devmode.dmDisplayFrequency);

					if (refresh == devmode.dmDisplayFrequency)
						refresh_selection = refresh_index;

					refresh_index++;
				}
			}
		}
		//free(t_screen);

		(void)ComboBox_SetCurSel(sizes_control, sizes_selection);
		(void)ComboBox_SetCurSel(refresh_control, refresh_selection);
	}
	return FALSE;
}

#ifdef DRIVER_SWITCH
static BOOL DriverConfigReadControl(datamap *map, HWND dialog, HWND control, core_options *opts, const char *option_name)
{
	const char *option;
	char buf[256];
	BOOL all = TRUE;
	BOOL found = FALSE;
	int i;

	buf[0] = '\0';

	for (i = 0; i < ARRAY_LENGTH(drivers_table); i++)
	{
		HWND hCtrl = GetDlgItem(dialog, drivers_table[i].ctrl);

		if (hCtrl)
		{
			found = TRUE;

			if (Button_GetCheck(hCtrl))
			{
				if (*buf)
					strcat(buf, ",");

				strcat(buf, drivers_table[i].name);
			}
			else
				all = FALSE;
		}
	}

	if (!found)
		return FALSE;

	if (all)
		strcpy(buf, "all");

	option = options_get_string(opts, OPTION_DRIVER_CONFIG);
	if (mame_stricmp(option, buf) == 0)
		return FALSE;

	options_set_string(opts, OPTION_DRIVER_CONFIG, buf, OPTION_PRIORITY_CMDLINE);

	return TRUE;
}
#endif /* DRIVER_SWITCH */



//============================================================

/************************************************************
 * DataMap initializers
 ************************************************************/

/* Initialize local helper variables */
static void ResetDataMap(HWND hWnd)
{
	char screen_option[32];
	const char *screen_option_value;

	ScreenSetOptionName(NULL, hWnd, NULL, screen_option, ARRAY_LENGTH(screen_option));

	screen_option_value = options_get_string(pCurrentOpts, screen_option);
	if (screen_option_value == NULL || *screen_option_value == '\0')
	{
		options_set_string(pCurrentOpts, screen_option, "auto", OPTION_PRIORITY_CMDLINE);
	}
}


/* Build the control mapping by adding all needed information to the DataMap */
static void BuildDataMap(void)
{
	properties_datamap = datamap_create();

	// core state options
	datamap_add(properties_datamap, IDC_ENABLE_AUTOSAVE,		DM_BOOL,	OPTION_AUTOSAVE);
	datamap_add(properties_datamap, IDC_SNAPVIEW,				DM_STRING,	OPTION_SNAPVIEW);
	datamap_add(properties_datamap, IDC_SNAPSIZEWIDTH,			DM_STRING,	NULL);
	datamap_add(properties_datamap, IDC_SNAPSIZEHEIGHT,			DM_STRING,	NULL);

	// core performance options
	datamap_add(properties_datamap, IDC_AUTOFRAMESKIP,			DM_BOOL,	OPTION_AUTOFRAMESKIP);
	datamap_add(properties_datamap, IDC_FRAMESKIP,				DM_INT,		OPTION_FRAMESKIP);
	datamap_add(properties_datamap, IDC_SECONDSTORUN,			DM_INT,		OPTION_SECONDS_TO_RUN);
	datamap_add(properties_datamap, IDC_SECONDSTORUNDISP,		DM_INT,		OPTION_SECONDS_TO_RUN);
	datamap_add(properties_datamap, IDC_THROTTLE,				DM_BOOL,	OPTION_THROTTLE);
	datamap_add(properties_datamap, IDC_SLEEP,					DM_BOOL,	OPTION_SLEEP);
	datamap_add(properties_datamap, IDC_SPEED,				    DM_FLOAT,	OPTION_SPEED);
	datamap_add(properties_datamap, IDC_SPEEDDISP,				DM_FLOAT,	OPTION_SPEED);
	datamap_add(properties_datamap, IDC_REFRESHSPEED,			DM_BOOL,	OPTION_REFRESHSPEED);

	// core retation options
	datamap_add(properties_datamap, IDC_ROTATE,					DM_INT,		NULL);
	// ror, rol, autoror, autorol handled by callback
	datamap_add(properties_datamap, IDC_FLIPX,					DM_BOOL,	OPTION_FLIPX);
	datamap_add(properties_datamap, IDC_FLIPY,					DM_BOOL,	OPTION_FLIPY);

	// core artwork options
	datamap_add(properties_datamap, IDC_ARTWORK_CROP,			DM_BOOL,	OPTION_ARTWORK_CROP);
	datamap_add(properties_datamap, IDC_BACKDROPS,				DM_BOOL,	OPTION_USE_BACKDROPS);
	datamap_add(properties_datamap, IDC_OVERLAYS,				DM_BOOL,	OPTION_USE_OVERLAYS);
	datamap_add(properties_datamap, IDC_BEZELS,					DM_BOOL,	OPTION_USE_BEZELS);

	// core screen options
	datamap_add(properties_datamap, IDC_BRIGHTCORRECT,			DM_FLOAT,	OPTION_BRIGHTNESS);
	datamap_add(properties_datamap, IDC_BRIGHTCORRECTDISP,		DM_FLOAT,	OPTION_BRIGHTNESS);
	datamap_add(properties_datamap, IDC_CONTRAST,				DM_FLOAT,	OPTION_CONTRAST);
	datamap_add(properties_datamap, IDC_CONTRASTDISP,			DM_FLOAT,	OPTION_CONTRAST);
	datamap_add(properties_datamap, IDC_GAMMA,					DM_FLOAT,	OPTION_GAMMA);
	datamap_add(properties_datamap, IDC_GAMMADISP,				DM_FLOAT,	OPTION_GAMMA);
	datamap_add(properties_datamap, IDC_PAUSEBRIGHT,			DM_FLOAT,	OPTION_PAUSE_BRIGHTNESS);
	datamap_add(properties_datamap, IDC_PAUSEBRIGHTDISP,		DM_FLOAT,	OPTION_PAUSE_BRIGHTNESS);

	// core vector options
	datamap_add(properties_datamap, IDC_ANTIALIAS,				DM_BOOL,	OPTION_ANTIALIAS);
	datamap_add(properties_datamap, IDC_BEAM,					DM_FLOAT,	OPTION_BEAM);
	datamap_add(properties_datamap, IDC_BEAMDISP,				DM_FLOAT,	OPTION_BEAM);
	datamap_add(properties_datamap, IDC_FLICKER,				DM_FLOAT,	OPTION_FLICKER);
	datamap_add(properties_datamap, IDC_FLICKERDISP,			DM_FLOAT,	OPTION_FLICKER);

	// core sound options
	datamap_add(properties_datamap, IDC_SAMPLERATE,				DM_INT,		OPTION_SAMPLERATE);
	datamap_add(properties_datamap, IDC_SAMPLES,				DM_BOOL,	OPTION_SAMPLES);
	datamap_add(properties_datamap, IDC_USE_SOUND,				DM_BOOL,	OPTION_SOUND);
	datamap_add(properties_datamap, IDC_VOLUME,					DM_INT,		OPTION_VOLUME);
	datamap_add(properties_datamap, IDC_VOLUMEDISP,				DM_INT,		OPTION_VOLUME);
#ifdef USE_VOLUME_AUTO_ADJUST
	datamap_add(properties_datamap, IDC_VOLUME_ADJUST,			DM_BOOL,	OPTION_VOLUME_ADJUST);
#endif /* USE_VOLUME_AUTO_ADJUST */

	// core input options 
	datamap_add(properties_datamap, IDC_COINLOCKOUT,			DM_BOOL,	OPTION_COIN_LOCKOUT);
	datamap_add(properties_datamap, IDC_DEFAULT_INPUT,			DM_STRING,	OPTION_CTRLR);
	datamap_add(properties_datamap, IDC_USE_MOUSE,				DM_BOOL,	OPTION_MOUSE);
	datamap_add(properties_datamap, IDC_JOYSTICK,				DM_BOOL,	OPTION_JOYSTICK);
	datamap_add(properties_datamap, IDC_LIGHTGUN,				DM_BOOL,	OPTION_LIGHTGUN);
	datamap_add(properties_datamap, IDC_STEADYKEY,				DM_BOOL,	OPTION_STEADYKEY);
	datamap_add(properties_datamap, IDC_MULTIKEYBOARD,			DM_BOOL,	OPTION_MULTIKEYBOARD);
	datamap_add(properties_datamap, IDC_MULTIMOUSE,				DM_BOOL,	OPTION_MULTIMOUSE);
	datamap_add(properties_datamap, IDC_RELOAD,					DM_BOOL,	OPTION_OFFSCREEN_RELOAD);

	datamap_add(properties_datamap, IDC_JDZ,					DM_FLOAT,	OPTION_JOYSTICK_DEADZONE);
	datamap_add(properties_datamap, IDC_JDZDISP,				DM_FLOAT,	OPTION_JOYSTICK_DEADZONE);
	datamap_add(properties_datamap, IDC_JSAT,					DM_FLOAT,	OPTION_JOYSTICK_SATURATION);
	datamap_add(properties_datamap, IDC_JSATDISP,				DM_FLOAT,	OPTION_JOYSTICK_SATURATION);
	datamap_add(properties_datamap, IDC_JOYSTICKMAP,			DM_STRING,	OPTION_JOYSTICK_MAP);

	// core input automatic enable options
	datamap_add(properties_datamap, IDC_PADDLE,					DM_STRING,	OPTION_PADDLE_DEVICE);
	datamap_add(properties_datamap, IDC_ADSTICK,				DM_STRING,	OPTION_ADSTICK_DEVICE);
	datamap_add(properties_datamap, IDC_PEDAL,					DM_STRING,	OPTION_PEDAL_DEVICE);
	datamap_add(properties_datamap, IDC_DIAL,					DM_STRING,	OPTION_DIAL_DEVICE);
	datamap_add(properties_datamap, IDC_TRACKBALL,				DM_STRING,	OPTION_TRACKBALL_DEVICE);
	datamap_add(properties_datamap, IDC_LIGHTGUNDEVICE,			DM_STRING,	OPTION_LIGHTGUN_DEVICE);
	datamap_add(properties_datamap, IDC_POSITIONAL,					DM_STRING,	OPTION_POSITIONAL_DEVICE);
	datamap_add(properties_datamap, IDC_MOUSE,					DM_STRING,	OPTION_MOUSE_DEVICE);

	// windows debugging options
	datamap_add(properties_datamap, IDC_OSLOG,					DM_BOOL,	WINOPTION_OSLOG);
	// watchdog missing

	// core debugging options
	datamap_add(properties_datamap, IDC_LOG,					DM_BOOL,	OPTION_LOG);
	datamap_add(properties_datamap, IDC_DEBUG,					DM_BOOL,	OPTION_DEBUG);
	datamap_add(properties_datamap, IDC_VERBOSE,				DM_BOOL,	OPTION_VERBOSE);
	datamap_add(properties_datamap, IDC_UPDATEINPAUSE,			DM_BOOL,	OPTION_UPDATEINPAUSE);
	datamap_add(properties_datamap, IDC_DEBUGSCRIPT,			DM_STRING,	OPTION_DEBUGSCRIPT);

	// core misc options
	datamap_add(properties_datamap, IDC_BIOS,					DM_STRING,	OPTION_BIOS);
	datamap_add(properties_datamap, IDC_CHEAT,					DM_BOOL,	OPTION_CHEAT);
	datamap_add(properties_datamap, IDC_SKIP_GAME_INFO,			DM_BOOL,	OPTION_SKIP_GAMEINFO);
	datamap_add(properties_datamap, IDC_CONFIRM_QUIT,			DM_BOOL,	OPTION_CONFIRM_QUIT);
#ifdef AUTO_PAUSE_PLAYBACK
	datamap_add(properties_datamap, IDC_AUTO_PAUSE_PLAYBACK,		DM_BOOL,	OPTION_AUTO_PAUSE_PLAYBACK);
#endif /* AUTO_PAUSE_PLAYBACK */
#ifdef TRANS_UI
	datamap_add(properties_datamap, IDC_TRANSPARENCY,			DM_INT,  	OPTION_UI_TRANSPARENCY);
	datamap_add(properties_datamap, IDC_TRANSPARENCYDISP,			DM_INT,  	OPTION_UI_TRANSPARENCY);
#endif /* TRANS_UI */

	// windows performance options
	datamap_add(properties_datamap, IDC_HIGH_PRIORITY,			DM_INT,		WINOPTION_PRIORITY);
	datamap_add(properties_datamap, IDC_HIGH_PRIORITYTXT,		DM_INT,		WINOPTION_PRIORITY);
	datamap_add(properties_datamap, IDC_MULTITHREAD_RENDERING,	DM_BOOL,	WINOPTION_MULTITHREADING);

	// windows video options
	datamap_add(properties_datamap, IDC_VIDEO_MODE,				DM_STRING,	WINOPTION_VIDEO);
	datamap_add(properties_datamap, IDC_NUMSCREENS,				DM_INT,		WINOPTION_NUMSCREENS);
	datamap_add(properties_datamap, IDC_NUMSCREENSDISP,			DM_INT,		WINOPTION_NUMSCREENS);
	datamap_add(properties_datamap, IDC_WINDOWED,				DM_BOOL,	WINOPTION_WINDOW);
	datamap_add(properties_datamap, IDC_MAXIMIZE,				DM_BOOL,	WINOPTION_MAXIMIZE);
	datamap_add(properties_datamap, IDC_KEEPASPECT,				DM_BOOL,	WINOPTION_KEEPASPECT);
	datamap_add(properties_datamap, IDC_PRESCALE,				DM_INT,		WINOPTION_PRESCALE);
	datamap_add(properties_datamap, IDC_PRESCALEDISP,			DM_INT,		WINOPTION_PRESCALE);
	//mamep: use standard combobox
	datamap_add(properties_datamap, IDC_EFFECT,					DM_STRING,	WINOPTION_EFFECT);
	datamap_add(properties_datamap, IDC_WAITVSYNC,				DM_BOOL,	WINOPTION_WAITVSYNC);
	datamap_add(properties_datamap, IDC_SYNCREFRESH,			DM_BOOL,	WINOPTION_SYNCREFRESH);
#ifdef USE_SCALE_EFFECTS
	datamap_add(properties_datamap, IDC_SCALEEFFECT,			DM_STRING,	OPTION_SCALE_EFFECT);
#endif /* USE_SCALE_EFFECTS */

	// DirectDraw specific options
	datamap_add(properties_datamap, IDC_HWSTRETCH,				DM_BOOL,	WINOPTION_HWSTRETCH);

	// Direct3D specific options
	datamap_add(properties_datamap, IDC_D3D_VERSION,			DM_INT,		WINOPTION_D3DVERSION);
	datamap_add(properties_datamap, IDC_D3D_FILTER,				DM_BOOL,	WINOPTION_FILTER);

	// per window video options
	datamap_add(properties_datamap, IDC_SCREEN,					DM_STRING,	NULL);
	datamap_add(properties_datamap, IDC_SCREENSELECT,			DM_STRING,	NULL);
	datamap_add(properties_datamap, IDC_VIEW,					DM_STRING,	NULL);
	datamap_add(properties_datamap, IDC_ASPECTRATIOD,			DM_STRING,  NULL);
	datamap_add(properties_datamap, IDC_ASPECTRATION,			DM_STRING,  NULL);
	datamap_add(properties_datamap, IDC_REFRESH,				DM_STRING,  NULL);
	datamap_add(properties_datamap, IDC_SIZES,					DM_STRING,  NULL);

	// full screen options
	datamap_add(properties_datamap, IDC_TRIPLE_BUFFER,			DM_BOOL,	WINOPTION_TRIPLEBUFFER);
	datamap_add(properties_datamap, IDC_SWITCHRES,				DM_BOOL,	WINOPTION_SWITCHRES);
	datamap_add(properties_datamap, IDC_FSBRIGHTNESS,			DM_FLOAT,	WINOPTION_FULLSCREENBRIGHTNESS);
	datamap_add(properties_datamap, IDC_FSBRIGHTNESSDISP,		DM_FLOAT,	WINOPTION_FULLSCREENBRIGHTNESS);
	datamap_add(properties_datamap, IDC_FSCONTRAST,				DM_FLOAT,	WINOPTION_FULLLSCREENCONTRAST);
	datamap_add(properties_datamap, IDC_FSCONTRASTDISP,			DM_FLOAT,	WINOPTION_FULLLSCREENCONTRAST);
	datamap_add(properties_datamap, IDC_FSGAMMA,				DM_FLOAT,	WINOPTION_FULLSCREENGAMMA);
	datamap_add(properties_datamap, IDC_FSGAMMADISP,			DM_FLOAT,	WINOPTION_FULLSCREENGAMMA);

	// windows sound options
	datamap_add(properties_datamap, IDC_AUDIO_LATENCY,			DM_INT,		WINOPTION_AUDIO_LATENCY);
	datamap_add(properties_datamap, IDC_AUDIO_LATENCY_DISP,		DM_INT,		WINOPTION_AUDIO_LATENCY);

	// input device options
	datamap_add(properties_datamap, IDC_DUAL_LIGHTGUN,			DM_BOOL,	WINOPTION_DUAL_LIGHTGUN);
#ifdef JOYSTICK_ID
	datamap_add(properties_datamap, IDC_JOYID1,				DM_INT,		WINOPTION_JOYID1);
	datamap_add(properties_datamap, IDC_JOYID2,				DM_INT,		WINOPTION_JOYID2);
	datamap_add(properties_datamap, IDC_JOYID3,				DM_INT,		WINOPTION_JOYID3);
	datamap_add(properties_datamap, IDC_JOYID4,				DM_INT,		WINOPTION_JOYID4);
	datamap_add(properties_datamap, IDC_JOYID5,				DM_INT,		WINOPTION_JOYID5);
	datamap_add(properties_datamap, IDC_JOYID6,				DM_INT,		WINOPTION_JOYID6);
	datamap_add(properties_datamap, IDC_JOYID7,				DM_INT,		WINOPTION_JOYID7);
	datamap_add(properties_datamap, IDC_JOYID8,				DM_INT,		WINOPTION_JOYID8);
#endif /* JOYSTICK_ID */

#ifdef DRIVER_SWITCH
	{
		int i;

		for (i = 0; i < ARRAY_LENGTH(drivers_table); i++)
			datamap_add(properties_datamap, drivers_table[i].ctrl,	DM_STRING,	NULL);
	}
#endif /* DRIVER_SWITCH */

	// set up callbacks
	datamap_set_callback(properties_datamap, IDC_ROTATE,		DCT_READ_CONTROL,		RotateReadControl);
	datamap_set_callback(properties_datamap, IDC_ROTATE,		DCT_POPULATE_CONTROL,	RotatePopulateControl);
	datamap_set_callback(properties_datamap, IDC_SCREEN,		DCT_READ_CONTROL,		ScreenReadControl);
	datamap_set_callback(properties_datamap, IDC_SCREEN,		DCT_POPULATE_CONTROL,	ScreenPopulateControl);
	datamap_set_callback(properties_datamap, IDC_VIEW,			DCT_POPULATE_CONTROL,	ViewPopulateControl);
	datamap_set_callback(properties_datamap, IDC_REFRESH,		DCT_READ_CONTROL,		ResolutionReadControl);
	datamap_set_callback(properties_datamap, IDC_REFRESH,		DCT_POPULATE_CONTROL,	ResolutionPopulateControl);
	datamap_set_callback(properties_datamap, IDC_SIZES,			DCT_READ_CONTROL,		ResolutionReadControl);
	datamap_set_callback(properties_datamap, IDC_SIZES,			DCT_POPULATE_CONTROL,	ResolutionPopulateControl);
	datamap_set_callback(properties_datamap, IDC_DEFAULT_INPUT,	DCT_POPULATE_CONTROL,	DefaultInputPopulateControl);
	datamap_set_callback(properties_datamap, IDC_SNAPVIEW,		DCT_POPULATE_CONTROL,	SnapViewPopulateControl);

#ifdef DRIVER_SWITCH
	{
		int i;

		for (i = 0; i < ARRAY_LENGTH(drivers_table); i++)
			datamap_set_callback(properties_datamap, drivers_table[i].ctrl,	DCT_READ_CONTROL,	DriverConfigReadControl);
	}
#endif /* DRIVER_SWITCH */

	datamap_set_option_name_callback(properties_datamap, IDC_VIEW,		ViewSetOptionName);
	//missing population of views with per game defined additional views
	datamap_set_option_name_callback(properties_datamap, IDC_REFRESH,	ResolutionSetOptionName);
	datamap_set_option_name_callback(properties_datamap, IDC_SIZES,		ResolutionSetOptionName);


	// formats
	datamap_set_int_format(properties_datamap, IDC_VOLUMEDISP,			"%ddB");
	datamap_set_int_format(properties_datamap, IDC_AUDIO_LATENCY_DISP,	"%d/5");
	datamap_set_float_format(properties_datamap, IDC_BEAMDISP,			"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_FLICKERDISP,		"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_GAMMADISP,			"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_BRIGHTCORRECTDISP,	"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_CONTRASTDISP,		"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_PAUSEBRIGHTDISP,	"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_FSGAMMADISP,		"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_FSBRIGHTNESSDISP,	"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_FSCONTRASTDISP,	"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_JDZDISP,			"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_JSATDISP,			"%03.2f");
	datamap_set_float_format(properties_datamap, IDC_SPEEDDISP,			"%03.2f");

	// trackbar ranges
	datamap_set_trackbar_range(properties_datamap, IDC_PRESCALE,    1, 10, 1);
	datamap_set_trackbar_range(properties_datamap, IDC_JDZ,         0.00,  1.00, (float)0.05);
	datamap_set_trackbar_range(properties_datamap, IDC_JSAT,        0.00,  1.00, (float)0.05);
	datamap_set_trackbar_range(properties_datamap, IDC_SPEED,       0.00,  3.00, (float)0.01);
	datamap_set_trackbar_range(properties_datamap, IDC_BEAM,        (float)0.10, 10.00, (float)0.10);       
	datamap_set_trackbar_range(properties_datamap, IDC_VOLUME,      -32,  0, 1);
	datamap_set_trackbar_range(properties_datamap, IDC_SECONDSTORUN, 0,  60, 1);
#ifdef TRANS_UI
	datamap_set_trackbar_range(properties_datamap, IDC_TRANSPARENCY, 0, 255, 1);
#endif /* TRANS_UI */

#if 0 //def MESS
	// MESS specific stuff
	MessBuildDataMap(properties_datamap);
#endif // MESS
}

#if 1 //mamep
static BOOL IsControlOptionValue(HWND hDlg, HWND hwnd_ctrl, core_options *opts, core_options *ref)
{
	const char *option_name;
	const char *opts_value;
	const char *ref_value;

	if (!opts || !ref)
		return FALSE;

	option_name = datamap_get_contorl_option_name(properties_datamap, hDlg, hwnd_ctrl);
	if (option_name == NULL)
		return TRUE;

	opts_value = options_get_string(opts, option_name);
	ref_value = options_get_string(ref, option_name);

	if (opts_value == ref_value)
		return TRUE;
	if (!opts_value || !ref_value)
		return FALSE;

	return strcmp(opts_value, ref_value) == 0;
}
#endif

static void SetStereoEnabled(HWND hWnd, int nIndex)
{
	BOOL enabled = FALSE;
	HWND hCtrl;
	int num_speakers = 0;

	if ( nIndex > -1)
	{
		machine_config *config = machine_config_alloc(drivers[nIndex]->machine_config);
		num_speakers = numberOfSpeakers(config);
		machine_config_free(config);
	}

	hCtrl = GetDlgItem(hWnd, IDC_STEREO);
	if (hCtrl)
	{
		if (nIndex <= -1 || num_speakers == 2)
			enabled = TRUE;

		EnableWindow(hCtrl, enabled);
	}
}

static void SetYM3812Enabled(HWND hWnd, int nIndex)
{
	BOOL enabled;
	HWND hCtrl;
	machine_config *config = NULL;
	const device_config *sound;

	if (nIndex > -1)
	{
		config = machine_config_alloc(drivers[nIndex]->machine_config);
	}

	hCtrl = GetDlgItem(hWnd, IDC_USE_FM_YM3812);
	if (hCtrl)
	{
		enabled = FALSE;

		for (sound = device_list_class_first(config->devicelist, DEVICE_CLASS_SOUND_CHIP); sound != NULL;
			sound = device_list_class_next(sound, DEVICE_CLASS_SOUND_CHIP))
		{
			if (nIndex <= -1
#if HAS_YM3812
				||  sound->type == SOUND_YM3812
#endif
#if HAS_YM2413
				||  sound->type == SOUND_YM2413
#endif
			)
				enabled = TRUE;
		}
    
		EnableWindow(hCtrl, enabled);
	}
	if (nIndex > -1)
	{
		machine_config_free(config);
	}
}

static void SetSamplesEnabled(HWND hWnd, int nIndex, BOOL bSoundEnabled)
{
	machine_config *config = NULL;
#if (HAS_SAMPLES == 1) || (HAS_VLM5030 == 1)
	BOOL enabled = FALSE;
	HWND hCtrl;

	hCtrl = GetDlgItem(hWnd, IDC_SAMPLES);

	if ( nIndex > -1 ) {
		config = machine_config_alloc(drivers[nIndex]->machine_config);
	}
	
	if (hCtrl)
	{
		const device_config *sound;
		if (config != NULL)
		{
			for (sound = sound_first(config); sound != NULL; sound = sound_next(sound))
			{
				if (sound_get_type(sound) == SOUND_SAMPLES
#if HAS_VLM5030
					||  sound_get_type(sound) == SOUND_VLM5030
#endif
					)
				{
					enabled = TRUE;
				}
			}
		}

		enabled = enabled && bSoundEnabled;
		EnableWindow(hCtrl, enabled);
	}
#endif
	if (config != NULL)
	{
		machine_config_free(config);
	}
}

/* Moved here cause it's called in a few places */
static void InitializeOptions(HWND hDlg)
{
	InitializeSoundUI(hDlg);
	InitializeSkippingUI(hDlg);
	InitializeRotateUI(hDlg);
	InitializeSelectScreenUI(hDlg);
	InitializeEffectUI(hDlg);
	InitializeBIOSUI(hDlg);
	InitializeControllerMappingUI(hDlg);
	InitializeD3DVersionUI(hDlg);
	InitializeVideoUI(hDlg);
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
}

static void OptOnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	if (hwndCtl == GetDlgItem(hwnd, IDC_NUMSCREENS))
	{
		NumScreensSelectionChange(hwnd);
	}
}

/* Handle changes to the Numscreens slider */
static void NumScreensSelectionChange(HWND hwnd)
{
	//Also Update the ScreenSelect Combo with the new number of screens
	UpdateSelectScreenUI(hwnd );
}

/* Handle changes to the Refresh drop down */
static void RefreshSelectionChange(HWND hWnd, HWND hWndCtrl)
{
	int nCurSelection;

	nCurSelection = ComboBox_GetCurSel(hWndCtrl);
	if (nCurSelection != CB_ERR)
	{
		datamap_populate_control(properties_datamap, hWnd, pCurrentOpts, IDC_SIZES);
	}
}

/* Initialize the sound options */
static void InitializeSoundUI(HWND hwnd)
{
	HWND    hCtrl;
	int i = 0;

	hCtrl = GetDlgItem(hwnd, IDC_SAMPLERATE);
	if (hCtrl)
	{
		static int rates[] =
		{
			11025,
			22050,
			44100,
			48000
		};

		for (i = i; i < ARRAY_LENGTH(rates); i++)
		{
			WCHAR buf[8];

			swprintf(buf, TEXT("%d"), rates[i]);
			(void)ComboBox_AddString(hCtrl, buf);
			(void)ComboBox_SetItemData(hCtrl, i, rates[i]);
		}

		(void)ComboBox_SetCurSel(hCtrl, 3);
	}
}

/* Populate the Frame Skipping drop down */
static void InitializeSkippingUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_FRAMESKIP);
	int i = 0;

	if (hCtrl)
	{
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Draw every frame")));
		(void)ComboBox_SetItemData(hCtrl, i++, 0);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 1 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 1);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 2 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 2);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 3 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 3);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 4 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 4);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 5 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 5);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 6 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 6);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 7 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 7);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 8 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 8);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 9 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 9);
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Skip 10 of 10 frames")));
		(void)ComboBox_SetItemData(hCtrl, i++, 10);
	}
}

/* Populate the Rotate drop down */
static void InitializeRotateUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd, IDC_ROTATE);

	if (hCtrl)
	{
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Default")));             // 0
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Clockwise")));           // 1
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Anti-clockwise")));      // 2
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("None")));                // 3
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Auto clockwise")));      // 4
		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("Auto anti-clockwise"))); // 5
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
			(void)ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxVideo[i].m_pText));
			(void)ComboBox_SetItemData(hCtrl, i, g_ComboBoxVideo[i].m_pData);
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
			(void)ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxD3DVersion[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl, i, g_ComboBoxD3DVersion[i].m_pData);
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
		if ((curSel < 0) || (curSel >= NUMSELECTSCREEN))
			curSel = 0;
		(void)ComboBox_ResetContent(hCtrl );
		for (i = 0; i < NUMSELECTSCREEN && i < options_get_int(pCurrentOpts, WINOPTION_NUMSCREENS) + 1; i++)
		{
			(void)ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxSelectScreen[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl, i, g_ComboBoxSelectScreen[i].m_pData);
		}
		// Smaller Amount of screens was selected, so use 0
		if( i< curSel )
			(void)ComboBox_SetCurSel(hCtrl, 0 );
		else
			(void)ComboBox_SetCurSel(hCtrl, curSel );
	}
}

/* Populate the Select Screen drop down */
static void InitializeSelectScreenUI(HWND hwnd)
{
	UpdateSelectScreenUI(hwnd);
}

static void InitializeEffectUI(HWND hwnd)
{
	//mamep: use standard combobox
	HWND hCtrl = GetDlgItem(hwnd, IDC_EFFECT);
	int i = 0;

	if (hCtrl)
	{
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		WCHAR ext[MAX_PATH];
		WCHAR root[MAX_PATH];
		WCHAR path[MAX_PATH];

		(void)ComboBox_AddString(hCtrl, _UIW(TEXT("None")));
		(void)ComboBox_SetItemData(hCtrl, i++, "none");

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
					char *value;

					// and strip off the extension
					root[wcslen(root) - 4] = '\0';
					value = utf8_from_wstring(root);

					// add it as an option
					(void)ComboBox_AddStringW(hCtrl, root);
					(void)ComboBox_SetItemData(hCtrl, i++, value);
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
	HWND hCtrl3 = GetDlgItem(hwnd,IDC_MOUSE);
	HWND hCtrl4 = GetDlgItem(hwnd,IDC_DIAL);
	HWND hCtrl5 = GetDlgItem(hwnd,IDC_TRACKBALL);
	HWND hCtrl6 = GetDlgItem(hwnd,IDC_LIGHTGUNDEVICE);
	HWND hCtrl7 = GetDlgItem(hwnd,IDC_POSITIONAL);

	if (hCtrl)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl1)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl1, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl1, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl2)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl2, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl2, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl3)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl3, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl3, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl4)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl4, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl4, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl5)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl5, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl5, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl6)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl6, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl6, i, g_ComboBoxDevice[i].m_pData);
		}
	}
	if (hCtrl7)
	{
		for (i = 0; i < NUMDEVICES; i++)
		{
			(void)ComboBox_InsertString(hCtrl7, i, _UIW(g_ComboBoxDevice[i].m_pText));
			(void)ComboBox_SetItemData( hCtrl7, i, g_ComboBoxDevice[i].m_pData);
		}
	}
}


static void InitializeBIOSUI(HWND hwnd)
{
	HWND hCtrl = GetDlgItem(hwnd,IDC_BIOS);
	int i = 0;
	TCHAR* t_s;
	if (hCtrl)
	{
		const game_driver *gamedrv = drivers[g_nGame];
		const rom_entry *rom;

		if (g_nGame == GLOBAL_OPTIONS)
		{
			(void)ComboBox_InsertString(hCtrl, i, TEXT("None"));
			(void)ComboBox_SetItemData( hCtrl, i++, "");
			return;
		}
		if (g_nGame == FOLDER_OPTIONS) //Folder Options
		{
			gamedrv = drivers[g_nFolderGame];
			if (DriverHasOptionalBIOS(g_nFolderGame) == FALSE)
			{
				(void)ComboBox_InsertString(hCtrl, i, TEXT("None"));
				(void)ComboBox_SetItemData( hCtrl, i++, "");
				return;
			}
			(void)ComboBox_InsertString(hCtrl, i, _UIW(TEXT("Default")));
			(void)ComboBox_SetItemData( hCtrl, i++, "");

			if (gamedrv->rom != NULL)
			{
				for (rom = gamedrv->rom; !ROMENTRY_ISEND(rom); rom++)
				{
					if (ROMENTRY_ISSYSTEM_BIOS(rom))
					{
						const char *name = ROM_GETHASHDATA(rom);
						const char *biosname = ROM_GETNAME(rom);
						t_s = tstring_from_utf8(name);
						if( !t_s )
							return;
						(void)ComboBox_InsertString(hCtrl, i, win_tstring_strdup(t_s));
						(void)ComboBox_SetItemData( hCtrl, i++, biosname);
						free(t_s);
					}
				}
			}
			return;
		}

		if (DriverHasOptionalBIOS(g_nGame) == FALSE)
		{
			(void)ComboBox_InsertString(hCtrl, i, TEXT("None"));
			(void)ComboBox_SetItemData( hCtrl, i++, "");
			return;
		}
		(void)ComboBox_InsertString(hCtrl, i, _UIW(TEXT("Default")));
		(void)ComboBox_SetItemData( hCtrl, i++, "");

		if (gamedrv->rom != NULL)
		{
			for (rom = gamedrv->rom; !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom))
				{
					const char *name = ROM_GETHASHDATA(rom);
					const char *biosname = ROM_GETNAME(rom);
					t_s = tstring_from_utf8(name);
					if( !t_s )
						return;
					(void)ComboBox_InsertString(hCtrl, i, win_tstring_strdup(t_s));
					(void)ComboBox_SetItemData( hCtrl, i++, biosname);
					free(t_s);
				}
			}
		}
	}
}

#if 0 //mamep: use standard combobox
static BOOL SelectEffect(HWND hWnd)
{
	char filename[MAX_PATH];
	BOOL changed = FALSE;

	*filename = 0;
	if (CommonFileDialog(GetOpenFileName, filename, FILETYPE_EFFECT_FILES))
	{
		//strip Path and extension
		char buff[MAX_PATH];
		int i, j = 0, k = 0, l = 0;
		for(i=0; i<strlen(filename); i++ )
		{
			if( filename[i] == '\\' )
				j = i;
			if( filename[i] == '.' )
				k = i;
		}
		for(i=j+1;i<k;i++)
		{
			buff[l++] = filename[i];
		}
		buff[l] = '\0';

		if (strcmp(buff, options_get_string(pCurrentOpts, WINOPTION_EFFECT)))
		{
			HWND control = GetDlgItem(hWnd, IDC_EFFECT);
			options_set_string(pCurrentOpts, WINOPTION_EFFECT, buff, OPTION_PRIORITY_CMDLINE);
			win_set_window_text_utf8(control, buff);
			// datamap_populate_control(properties_datamap, hWnd, pCurrentOpts, IDC_EFFECT);
			changed = TRUE;
		}
	}
	return changed;
}

static BOOL ResetEffect(HWND hWnd)
{
	BOOL changed = FALSE;
	const char *new_value = "none";

	if (strcmp(new_value, options_get_string(pCurrentOpts, WINOPTION_EFFECT)))
	{
		HWND control = GetDlgItem(hWnd, IDC_EFFECT);
		options_set_string(pCurrentOpts, WINOPTION_EFFECT, new_value, OPTION_PRIORITY_CMDLINE);
		win_set_window_text_utf8(control, new_value);
		// datamap_populate_control(properties_datamap, hWnd, pCurrentOpts, IDC_EFFECT);
		changed = TRUE;
	}
	return changed;
}
#endif //mamep: use standard combobox

#ifdef JOYSTICK_ID
/* Populate the Joystick ID drop down */
static void InitializeJoyidUI(HWND hWnd)
{
	HWND hCtrl;
	int i, j, num;

	if (DIJoystick.Available() == FALSE)
		return;

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
				(void)ComboBox_AddString(hCtrl, buf);
				(void)ComboBox_SetItemData(hCtrl, j, j);
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

		for (i = 0; scale_name(i); i++)
		{
			const char *value = scale_name(i);
			(void)ComboBox_AddString(hCtrl,_UIW(_Unicode(scale_desc(i))));
			(void)ComboBox_SetItemData(hCtrl, i, value);
		}
	}
}
#endif /* USE_SCALE_EFFECTS */


static BOOL SelectJoystickMap(HWND hWnd)
{
	WCHAR filename[MAX_PATH];
	BOOL changed = FALSE;

	*filename = 0;
	if (CommonFileDialog(GetOpenFileName, filename, FILETYPE_JOYMAP_FILES))
	{
		if (wcscmp(filename, options_get_wstring(pCurrentOpts, OPTION_JOYSTICK_MAP)))
		{
			HWND control = GetDlgItem(hWnd, IDC_JOYSTICKMAP);
			options_set_wstring(pCurrentOpts, OPTION_JOYSTICK_MAP, filename, OPTION_PRIORITY_CMDLINE);
			SetWindowTextW(control, filename);
			changed = TRUE;
		}
	}
	return changed;
}

static BOOL ResetJoystickMap(HWND hWnd)
{
	BOOL changed = FALSE;
	const WCHAR *new_value = TEXT("auto");

	if (wcscmp(new_value, options_get_wstring(pCurrentOpts, OPTION_JOYSTICK_MAP)))
	{
		HWND control = GetDlgItem(hWnd, IDC_JOYSTICKMAP);
		options_set_wstring(pCurrentOpts, OPTION_JOYSTICK_MAP, new_value, OPTION_PRIORITY_CMDLINE);
		SetWindowTextW(control, new_value);
		changed = TRUE;
	}
	return changed;
}

static BOOL SelectDebugscript(HWND hWnd)
{
	WCHAR filename[MAX_PATH];
	BOOL changed = FALSE;

	*filename = 0;
	if (CommonFileDialog(GetOpenFileName, filename, FILETYPE_DEBUGSCRIPT_FILES))
	{
		if (wcscmp(filename, options_get_wstring(pCurrentOpts, OPTION_DEBUGSCRIPT)))
		{
			HWND control = GetDlgItem(hWnd, IDC_DEBUGSCRIPT);
			options_set_wstring(pCurrentOpts, OPTION_DEBUGSCRIPT, filename, OPTION_PRIORITY_CMDLINE);
			SetWindowTextW(control, filename);
			changed = TRUE;
		}
	}
	return changed;
}

static BOOL ResetDebugscript(HWND hWnd)
{
	BOOL changed = FALSE;
	const WCHAR *new_value = TEXT("");

	if (wcscmp(new_value, options_get_wstring(pCurrentOpts, OPTION_DEBUGSCRIPT)))
	{
		HWND control = GetDlgItem(hWnd, IDC_DEBUGSCRIPT);
		options_set_wstring(pCurrentOpts, OPTION_DEBUGSCRIPT, new_value, OPTION_PRIORITY_CMDLINE);
		SetWindowTextW(control, new_value);
		changed = TRUE;
	}
	return changed;
}

void UpdateBackgroundBrush(HWND hwndTab)
{
    // Destroy old brush
    if (hBkBrush)
        DeleteBrush(hBkBrush);

	hBkBrush = NULL;

	// Only do this if the theme is active
	if (SafeIsAppThemed())
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
        DeleteBitmap(hBmp);
        DeleteDC(hDCMem);
        ReleaseDC(hwndTab, hDC);
	}
}


/* End of source file */
