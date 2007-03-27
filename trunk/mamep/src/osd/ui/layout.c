/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

***************************************************************************/

/***************************************************************************

  layout.c

  MAME specific TreeView definitions (and maybe more in the future)

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdio.h>  /* for sprintf */
#include <stdlib.h> /* For malloc and free */
#include <string.h>

#include "mame32.h"
#include "bitmask.h"
#include "TreeView.h"
#include "M32Util.h"
#include "resource.h"
#include "directories.h"
#include "winuiopt.h"
#include "translate.h"
#include "splitters.h"
#include "help.h"
#include "audit32.h"
#include "screenshot.h"
#include "win32ui.h"
#include "properties.h"

static BOOL FilterAvailable(int driver_index);

FOLDERDATA g_folderData[] =
{
	{TEXT("All Games"),       "allgames",          FOLDER_ALLGAMES,     IDI_FOLDER,				0,             0,            NULL,                       NULL,              TRUE },
	{TEXT("Available"),       "available",         FOLDER_AVAILABLE,    IDI_FOLDER_AVAILABLE,     F_AVAILABLE,   F_UNAVAILABLE,NULL,                       FilterAvailable,              TRUE },
#ifdef SHOW_UNAVAILABLE_FOLDER
	{TEXT("Unavailable"),     "unavailable",       FOLDER_UNAVAILABLE,  IDI_FOLDER_UNAVAILABLE,	F_UNAVAILABLE, F_AVAILABLE,  NULL,                       FilterAvailable,              FALSE },
#endif
	{TEXT("Manufacturer"),    "manufacturer",      FOLDER_MANUFACTURER, IDI_FOLDER_MANUFACTURER,  0,             0,            CreateManufacturerFolders },
	{TEXT("Year"),            "year",              FOLDER_YEAR,         IDI_FOLDER_YEAR,          0,             0,            CreateYearFolders },
	{TEXT("Driver"),          "driver",            FOLDER_SOURCE,       IDI_FOLDER_SOURCE,        0,             0,            CreateSourceFolders },
#ifdef MISC_FOLDER
	{TEXT("BIOS"),            "bios",              FOLDER_BIOS,         IDI_BIOS,                 0,             0,            CreateBIOSFolders,          DriverIsBios,  TRUE },
#endif /* MISC_FOLDER */
	{TEXT("CPU"),             "cpu",               FOLDER_CPU,          IDI_CPU,                  0,             0,            CreateCPUFolders },
	{TEXT("Sound"),           "sound",             FOLDER_SND,          IDI_SND,                  0,             0,            CreateSoundFolders },
	{TEXT("Orientation"),     "orientation",       FOLDER_ORIENTATION,  IDI_FOLDER,               0,             0,            CreateOrientationFolders },
	{TEXT("Imperfect"),       "imperfect",         FOLDER_DEFICIENCY,   IDI_FOLDER,               0,             0,            CreateDeficiencyFolders },
	{TEXT("Dumping Status"),  "dumping",           FOLDER_DUMPING,      IDI_FOLDER,               0,             0,            CreateDumpingFolders },
	{TEXT("Working"),         "working",           FOLDER_WORKING,      IDI_WORKING,              F_WORKING,     F_NONWORKING, NULL,                       DriverIsBroken,    FALSE },
	{TEXT("Not Working"),     "nonworking",        FOLDER_NONWORKING,   IDI_NONWORKING,           F_NONWORKING,  F_WORKING,    NULL,                       DriverIsBroken,    TRUE },
	{TEXT("Originals"),       "originals",         FOLDER_ORIGINAL,     IDI_FOLDER,               F_ORIGINALS,   F_CLONES,     NULL,                       DriverIsClone,     FALSE },
	{TEXT("Clones"),          "clones",            FOLDER_CLONES,       IDI_FOLDER,               F_CLONES,      F_ORIGINALS,  NULL,                       DriverIsClone,     TRUE },
	{TEXT("Raster"),          "raster",            FOLDER_RASTER,       IDI_FOLDER,               F_RASTER,      F_VECTOR,     NULL,                       DriverIsVector,    FALSE },
	{TEXT("Vector"),          "vector",            FOLDER_VECTOR,       IDI_FOLDER,               F_VECTOR,      F_RASTER,     NULL,                       DriverIsVector,    TRUE },
#ifdef MISC_FOLDER
	{TEXT("Resolution"),      "resolution",        FOLDER_RESOLUTION,   IDI_FOLDER,               0,             0,            CreateResolutionFolders },
	{TEXT("FPS"),             "fps",               FOLDER_FPS,          IDI_FOLDER,               0,             0,            CreateFPSFolders },
	{TEXT("Save State"),      "savestate",         FOLDER_SAVESTATE,    IDI_FOLDER,               0,             0,            CreateSaveStateFolders },
	{TEXT("Control Type"),    "control",           FOLDER_CONTROL,      IDI_FOLDER,               0,             0,            CreateControlFolders },
#else /* MISC_FOLDER */
	{TEXT("Trackball"),       "trackball",         FOLDER_TRACKBALL,    IDI_FOLDER,               0,             0,            NULL,                       DriverUsesTrackball,	TRUE },
	{TEXT("Lightgun"),        "lightgun",          FOLDER_LIGHTGUN,     IDI_FOLDER,               0,             0,            NULL,                       DriverUsesLightGun,TRUE },
#endif /* !MISC_FOLDER */
	{TEXT("Stereo"),          "stereo",            FOLDER_STEREO,       IDI_SOUND,                0,             0,            NULL,                       DriverIsStereo,    TRUE },
	{TEXT("CHD"),             "harddisk",          FOLDER_HARDDISK,     IDI_HARDDISK,             0,             0,            NULL,                       DriverIsHarddisk,  TRUE },
	{TEXT("Samples"),         "samples",           FOLDER_SAMPLES,      IDI_FOLDER,               0,             0,            NULL,                       DriverUsesSamples,  TRUE },
#ifndef MISC_FOLDER
	{TEXT("Save State"),      "savestate",         FOLDER_SAVESTATE,    IDI_FOLDER,               0,             0,            NULL,                       DriverSupportsSaveState,  TRUE },
	{TEXT("BIOS"),            "bios",              FOLDER_BIOS,         IDI_FOLDER,               0,             0,            NULL,                       DriverIsBios,  TRUE },
#endif /* !MISC_FOLDER */
	{ NULL }
};

/* list of filter/control Id pairs */
FILTER_ITEM g_filterList[] =
{
	{ F_CLONES,       IDC_FILTER_CLONES,      DriverIsClone, TRUE },
	{ F_NONWORKING,   IDC_FILTER_NONWORKING,  DriverIsBroken, TRUE },
	{ F_UNAVAILABLE,  IDC_FILTER_UNAVAILABLE, FilterAvailable, FALSE },
	{ F_RASTER,       IDC_FILTER_RASTER,      DriverIsVector, FALSE },
	{ F_VECTOR,       IDC_FILTER_VECTOR,      DriverIsVector, TRUE },
	{ F_ORIGINALS,    IDC_FILTER_ORIGINALS,   DriverIsClone, FALSE },
	{ F_WORKING,      IDC_FILTER_WORKING,     DriverIsBroken, FALSE },
	{ F_AVAILABLE,    IDC_FILTER_AVAILABLE,   FilterAvailable, TRUE },
	{ 0 }
};

const DIRECTORYINFO g_directoryInfo[] =
{
	{ TEXT("ROMs"),                  GetRomDirs,          SetRomDirs,          TRUE,  DIRDLG_ROMS },
	{ TEXT("Samples"),               GetSampleDirs,       SetSampleDirs,       TRUE,  DIRDLG_SAMPLES },
	{ TEXT("Ini Files"),             GetIniDir,           SetIniDir,           FALSE, DIRDLG_INI },
	{ TEXT("Translation Files"),     GetTranslationDir,   SetTranslationDir,   FALSE, 0 },
	{ TEXT("Localized Files"),       GetLocalizedDir,     SetLocalizedDir,     FALSE, 0 },
#ifdef USE_IPS
	{ TEXT("IPS Files"),             GetPatchDir,         SetPatchDir,         FALSE, 0 },
#endif /* USE_IPS */
	{ TEXT("Config"),                GetCfgDir,           SetCfgDir,           FALSE, DIRDLG_CFG },
#ifdef USE_HISCORE
	{ TEXT("High Scores"),           GetHiDir,            SetHiDir,            FALSE, DIRDLG_HI },
#endif /* USE_HISCORE */
	{ TEXT("Snapshots"),             GetImgDirs,          SetImgDirs,          TRUE,  DIRDLG_IMG },
	{ TEXT("Input Files (*.inp)"),   GetInpDir,           SetInpDir,           FALSE, DIRDLG_INP },
	{ TEXT("State"),                 GetStateDir,         SetStateDir,         FALSE, 0 },
	{ TEXT("Artwork"),               GetArtDir,           SetArtDir,           FALSE, 0 },
	{ TEXT("Memory Card"),           GetMemcardDir,       SetMemcardDir,       FALSE, 0 },
	{ TEXT("Flyers"),                GetFlyerDirs,        SetFlyerDirs,        TRUE,  0 },
	{ TEXT("Cabinets"),              GetCabinetDirs,      SetCabinetDirs,      TRUE,  0 },
	{ TEXT("Marquees"),              GetMarqueeDirs,      SetMarqueeDirs,      TRUE,  0 },
	{ TEXT("Titles"),                GetTitlesDirs,       SetTitlesDirs,       TRUE,  0 },
	{ TEXT("Control Panels"),        GetControlPanelDirs, SetControlPanelDirs, TRUE,  0 },
	{ TEXT("NVRAM"),                 GetNvramDir,         SetNvramDir,         FALSE, 0 },
	{ TEXT("Controller Files"),      GetCtrlrDir,         SetCtrlrDir,         FALSE, DIRDLG_CTRLR },
	{ TEXT("Hard Drive Difference"), GetDiffDir,          SetDiffDir,          FALSE, 0 },
	{ TEXT("Icons"),                 GetIconsDirs,        SetIconsDirs,        TRUE,  0 },
	{ TEXT("Background Images"),     GetBgDir,            SetBgDir,            FALSE, 0 },
	{ TEXT("Comment Files"),         GetCommentDir,       SetCommentDir,       FALSE, DIRDLG_COMMENT },
	{ TEXT("External Folder List"),  GetFolderDir,        SetFolderDir,        FALSE, 0 },
#ifdef USE_VIEW_PCBINFO
	{ TEXT("PCB Info Files"),        GetPcbinfoDir,       SetPcbinfoDir,       FALSE, 0 },
#endif /* USE_VIEW_PCBINFO */
	{ NULL }
};

const SPLITTERINFO g_splitterInfo[] =
{
	{ 0.25,	IDC_SPLITTER,	IDC_TREE,	IDC_LIST,		AdjustSplitter1Rect },
	{ 0.5,	IDC_SPLITTER2,	IDC_LIST,	IDC_SSFRAME,		AdjustSplitter2Rect },
	{ -1 }
};

const MAMEHELPINFO g_helpInfo[] =
{
	{ ID_HELP_CONTENTS,     TRUE,	TEXT(MAME32HELP) TEXT("::/html/mame32_overview.htm") },
	{ ID_HELP_WHATS_NEW32,	TRUE,	TEXT(MAME32HELP) TEXT("::/html/mame32_changes.htm") },
	{ ID_HELP_TROUBLE,      TRUE,	TEXT(MAME32HELP) TEXT("::/html/mame32_support.htm") },
	{ ID_HELP_RELEASE,      FALSE,	TEXT("windows.txt") },
	{ ID_HELP_WHATS_NEW,	TRUE,	TEXT(MAME32HELP) TEXT("::/docs/whatsnew.txt") },
	{ -1 }
};

const PROPERTYSHEETINFO g_propSheets[] =
{
	{ FALSE,	NULL,					IDD_PROP_GAME,			GamePropertiesDialogProc },
	{ FALSE,	NULL,					IDD_PROP_AUDIT,			GameAuditDialogProc },
	{ TRUE,		NULL,					IDD_PROP_DISPLAY,		GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_ADVANCED,		GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_SCREEN,		GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_SOUND,			GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_INPUT,			GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_CONTROLLER,	GameOptionsProc },
	{ TRUE,		NULL,					IDD_PROP_MISC,			GameOptionsProc },
	{ TRUE, 	PropSheetFilter_Vector,	IDD_PROP_VECTOR,		GameOptionsProc },
	{ TRUE,		PropSheetFilter_BIOS,	IDD_PROP_BIOS,			GameOptionsProc },
	{ FALSE }
};

const ICONDATA g_iconData[] =
{
	{ IDI_WIN_NOROMS,			"noroms" },
	{ IDI_WIN_ROMS,				"roms" },
	{ IDI_WIN_UNKNOWN,			"unknown" },
	{ IDI_WIN_CLONE,			"clone" },
	{ IDI_WIN_REDX,				"warning" },
	{ 0 }
};

EXTFOLDER_TEMPLATE extFavorite =
{
	TEXT("Favorites"),
	"golden",
	"cust2"
};

/*
const char g_szPlayGameString[] = "&Play %s";
const char g_szGameCountString[] = "%d games";
const char g_szHistoryFileName[] = "history.dat";
const char g_szMameInfoFileName[] = "mameinfo.dat";
*/

static BOOL FilterAvailable(int driver_index)
{
	//mamep: pong does not use roms
	if (!DriverUsesRoms(driver_index))
		return TRUE;
	return IsAuditResultYes(GetRomAuditResults(driver_index));
}
