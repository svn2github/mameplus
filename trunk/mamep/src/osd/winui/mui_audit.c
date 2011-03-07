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

  mui_audit.c

  Audit dialog

***************************************************************************/

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>

// standard C headers
#include <stdio.h>
#include <tchar.h>

// MAME/MAMEUI headers
#include "winui.h"
#include "winutf8.h"
#include "strconv.h"
#include "audit.h"
#include "resource.h"
#include "mui_opts.h"
#include "mui_util.h"
#include "properties.h"
#include "emuopts.h"
#include "translate.h"


#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

/***************************************************************************
    function prototypes
 ***************************************************************************/

static DWORD WINAPI AuditThreadProc(LPVOID hDlg);
static INT_PTR CALLBACK AuditWindowProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static void ProcessNextRom(void);
static void ProcessNextSample(void);
static void CLIB_DECL DetailsPrintf(const WCHAR *fmt, ...);
static const WCHAR *StatusString(int iStatus);

/***************************************************************************
    Internal variables
 ***************************************************************************/

#define SAMPLES_NOT_USED    3
#define MAX_AUDITBOX_TEXT	0x7FFFFFFE

static volatile HWND hAudit;
static volatile int rom_index;
static volatile int roms_correct;
static volatile int roms_incorrect;
static volatile int sample_index;
static volatile int samples_correct;
static volatile int samples_incorrect;

static volatile BOOL bPaused = FALSE;
static volatile BOOL bCancel = FALSE;

/***************************************************************************
    External functions  
 ***************************************************************************/

void AuditDialog(HWND hParent)
{
	HMODULE hModule = NULL;
	rom_index         = 0;
	roms_correct      = 0;
	roms_incorrect    = 0;
	sample_index      = 0;
	samples_correct   = 0;
	samples_incorrect = 0;

	//RS use Riched32.dll
	hModule = LoadLibrary(TEXT("Riched32.dll"));
	if( hModule )
	{
		DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_AUDIT),hParent,AuditWindowProc);
		FreeLibrary( hModule );
		hModule = NULL;
	}
	else
	{
	    MessageBox(GetMainWindow(), _UIW(TEXT("Unable to Load Riched32.dll")),TEXT("Error"),
				   MB_OK | MB_ICONERROR);
	}
}

void InitGameAudit(int gameIndex)
{
	rom_index = gameIndex;
}

const WCHAR *GetAuditString(int audit_result)
{
	switch (audit_result)
	{
	case CORRECT :
	case BEST_AVAILABLE :
		return _UIW(TEXT("Yes"));

	case NOTFOUND :
	case INCORRECT :
		return _UIW(TEXT("No"));

	case UNKNOWN :
		return TEXT("?");

	default:
		dprintf("unknown audit value %i\n",audit_result);
	}

	return TEXT("?");
}

BOOL IsAuditResultKnown(int audit_result)
{
	return audit_result != UNKNOWN;
}

BOOL IsAuditResultYes(int audit_result)
{
	return audit_result == CORRECT || audit_result == BEST_AVAILABLE;
}

BOOL IsAuditResultNo(int audit_result)
{
	return audit_result == NOTFOUND || audit_result == INCORRECT;
}


/***************************************************************************
    Internal functions
 ***************************************************************************/

static void MameUIOutput(void *param, const char *format, va_list argptr)
{
	char buffer[512];
	vsnprintf(buffer, ARRAY_LENGTH(buffer), format, argptr);
	DetailsPrintf(TEXT("%s"), _UTF8Unicode(ConvertToWindowsNewlines(buffer)));
}

static int ProcessAuditResults(int game, audit_record *audit, int audit_records, BOOL isComplete)
{
	output_callback_func prevcb;
	void *prevparam;
	int res;

	mame_set_output_channel(OUTPUT_CHANNEL_INFO, (isComplete) ? MameUIOutput : mame_null_output_callback, NULL, &prevcb, &prevparam);
	res = audit_summary(drivers[game], audit_records, audit, TRUE);
	mame_set_output_channel(OUTPUT_CHANNEL_INFO, prevcb ? prevcb : mame_null_output_callback, prevparam, NULL, NULL);

	return res;
}

static BOOL RomsetNotExist(int game)
{
	const game_driver *drv;

	// skip non cpu or chd
	if (!DriverUsesRoms(game) || DriverIsHarddisk(game))
		return FALSE;

	// find the file
	for (drv = drivers[game]; drv != NULL; drv = driver_get_clone(drv))
	{
		file_error filerr;
		astring *fname;

		// open the file if we can
		fname = astring_assemble_2(astring_alloc(), drv->name, ".zip");
		emu_file file = emu_file(*(MameUIGlobal()), SEARCHPATH_ROM, OPEN_FLAG_READ);
		filerr = file.open(astring_c(fname));
		astring_free(fname);
		if (filerr == FILERR_NONE)
		{
			file.close();
			return FALSE;
		}

#if 0 // enable it will decrease audit speed
		mame_path *path;

		// open the folder if we can
		fname = astring_assemble_3(astring_alloc(), SEARCHPATH_ROM, PATH_SEPARATOR, drv->name);
		path = mame_openpath(mame_options(), astring_c(fname));
		astring_free(fname);
		if (path != NULL)
		{
			mame_closepath(path);
			return FALSE;
		}
#endif
	}

	return TRUE;
}

// Verifies the ROM set while calling SetRomAuditResults
int MameUIVerifyRomSet(int game, BOOL isComplete)
{
	audit_record *audit;
	int audit_records;
	int res;

	// mamep: if rom file doesn't exist, don't verify it
	if (!isComplete && RomsetNotExist(game))
	{
		res = NOTFOUND;
		SetRomAuditResults(game, res);
		return res;
	}

	// perform the audit
	audit_records = audit_images(MameUIGlobal(), drivers[game], AUDIT_VALIDATE_FAST, &audit);
	res = ProcessAuditResults(game, audit, audit_records, isComplete);
	if (audit_records > 0)
		global_free(audit);

	SetRomAuditResults(game, res);
	return res;
}

// Verifies the Sample set while calling SetSampleAuditResults	
int MameUIVerifySampleSet(int game, BOOL isComplete)
{
	audit_record *audit;
	int audit_records;
	int res;

	// perform the audit
	audit_records = audit_samples(MameUIGlobal(), drivers[game], &audit);
	res = ProcessAuditResults(game, audit, audit_records, isComplete);
	if (audit_records > 0)
		global_free(audit);

	SetSampleAuditResults(game, res);
	return res;
}

static DWORD WINAPI AuditThreadProc(LPVOID hDlg)
{
	WCHAR buffer[200];

	while (!bCancel)
	{
		if (!bPaused)
		{
			if (rom_index != -1)
			{
				WCHAR *descw = driversw[rom_index]->description; swprintf(buffer, _UIW(TEXT("Checking Game %s - %s")), 
					driversw[rom_index]->name, UseLangList() ? _LSTW(descw) : descw);
				SetWindowText((HWND)hDlg, buffer);
				ProcessNextRom();
			}
			else
			{
				if (sample_index != -1)
				{
					WCHAR *descw = driversw[sample_index]->description; swprintf(buffer, _UIW(TEXT("Checking Game %s - %s")),
						driversw[sample_index]->name, UseLangList() ? _LSTW(descw) : descw);
					SetWindowText((HWND)hDlg, buffer);
					ProcessNextSample();
				}
				else
				{
					SetWindowText((HWND)hDlg, _UIW(TEXT("File Audit")));
					EnableWindow(GetDlgItem((HWND)hDlg, IDPAUSE), FALSE);
					ExitThread(1);
				}
			}
		}
	}
	return 0;
}

static INT_PTR CALLBACK AuditWindowProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread;
	static DWORD dwThreadID;
	DWORD dwExitCode;
	HWND hEdit;

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, TRUE);

		hAudit = hDlg;
		//RS 20030613 Set Bkg of RichEdit Ctrl
		hEdit = GetDlgItem(hAudit, IDC_AUDIT_DETAILS);
		if (hEdit != NULL)
		{
			SendMessage( hEdit, EM_SETBKGNDCOLOR, FALSE, GetSysColor(COLOR_BTNFACE) );
			// MSH - Set to max
			SendMessage( hEdit, EM_SETLIMITTEXT, MAX_AUDITBOX_TEXT, 0 );

		}
		SendDlgItemMessage(hDlg, IDC_ROMS_PROGRESS,    PBM_SETRANGE, 0, MAKELPARAM(0, GetNumGames()));
		SendDlgItemMessage(hDlg, IDC_SAMPLES_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, GetNumGames()));
		bPaused = FALSE;
		bCancel = FALSE;
		rom_index = 0;
		hThread = CreateThread(NULL, 0, AuditThreadProc, hDlg, 0, &dwThreadID);
		return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
            bPaused = FALSE;
			if (hThread)
			{
				bCancel = TRUE;
				if (GetExitCodeThread(hThread, &dwExitCode) && (dwExitCode == STILL_ACTIVE))
				{
					PostMessage(hDlg, WM_COMMAND, wParam, lParam);
					return 1;
				}
				CloseHandle(hThread);
			}
			EndDialog(hDlg,0);
			break;

		case IDPAUSE:
			if (bPaused)
			{
				SendDlgItemMessage(hAudit, IDPAUSE, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)_UIW(TEXT("&Pause")));
				bPaused = FALSE;
			}
			else
			{
				SendDlgItemMessage(hAudit, IDPAUSE, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)(_UIW(TEXT("&Continue"))));
				bPaused = TRUE;
			}
			break;
		}
		return 1;
	}
	return 0;
}

/* Callback for the Audit property sheet */
INT_PTR CALLBACK GameAuditDialogProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, TRUE);

#ifdef TREE_SHEET
		if (GetShowTreeSheet())
		{
			ModifyPropertySheetForTreeSheet(hDlg);
		}
#endif /* TREE_SHEET */

		FlushFileCaches();
		hAudit = hDlg;
		Static_SetText(GetDlgItem(hDlg, IDC_PROP_TITLE), GameInfoTitle(OPTIONS_GAME, rom_index));
		SetTimer(hDlg, 0, 1, NULL);
		return 1;

	case WM_TIMER:
		KillTimer(hDlg, 0);
		{
			int iStatus;
			LPCWSTR lpStatus;

			iStatus = MameUIVerifyRomSet(rom_index, TRUE);
			lpStatus = DriverUsesRoms(rom_index) ? StatusString(iStatus) : _UIW(TEXT("None required"));
			SetWindowText(GetDlgItem(hDlg, IDC_PROP_ROMS), lpStatus);

			iStatus = MameUIVerifySampleSet(rom_index, TRUE);
			lpStatus = DriverUsesSamples(rom_index) ? StatusString(iStatus) : _UIW(TEXT("None required"));
			SetWindowText(GetDlgItem(hDlg, IDC_PROP_SAMPLES), lpStatus);
		}
		ShowWindow(hDlg, SW_SHOW);
		break;
	}
	return 0;
}

static void ProcessNextRom()
{
	int retval;
	TCHAR buffer[200];

	retval = MameUIVerifyRomSet(rom_index, TRUE);
	switch (retval)
	{
	case BEST_AVAILABLE: /* correct, incorrect or separate count? */
	case CORRECT:
		roms_correct++;
		_stprintf(buffer, TEXT("%i"), roms_correct);
		SendDlgItemMessage(hAudit, IDC_ROMS_CORRECT, WM_SETTEXT, 0, (LPARAM)buffer);
		_stprintf(buffer, TEXT("%i"), roms_correct + roms_incorrect);
		SendDlgItemMessage(hAudit, IDC_ROMS_TOTAL, WM_SETTEXT, 0, (LPARAM)buffer);
		break;

	case NOTFOUND:
		break;

	case INCORRECT:
		roms_incorrect++;
		_stprintf(buffer, TEXT("%i"), roms_incorrect);
		SendDlgItemMessage(hAudit, IDC_ROMS_INCORRECT, WM_SETTEXT, 0, (LPARAM)buffer);
		_stprintf(buffer, TEXT("%i"), roms_correct + roms_incorrect);
		SendDlgItemMessage(hAudit, IDC_ROMS_TOTAL, WM_SETTEXT, 0, (LPARAM)buffer);
		break;
	}

	rom_index++;
	SendDlgItemMessage(hAudit, IDC_ROMS_PROGRESS, PBM_SETPOS, rom_index, 0);

	if (rom_index == GetNumGames())
	{
		sample_index = 0;
		rom_index = -1;
	}
}

static void ProcessNextSample()
{
	int  retval;
	TCHAR buffer[200];
	
	retval = MameUIVerifySampleSet(sample_index, TRUE);
	
	switch (retval)
	{
	case CORRECT:
		if (DriverUsesSamples(sample_index))
		{
			samples_correct++;
			_stprintf(buffer, TEXT("%i"), samples_correct);
			SendDlgItemMessage(hAudit, IDC_SAMPLES_CORRECT, WM_SETTEXT, 0, (LPARAM)buffer);
			_stprintf(buffer, TEXT("%i"), samples_correct + samples_incorrect);
			SendDlgItemMessage(hAudit, IDC_SAMPLES_TOTAL, WM_SETTEXT, 0, (LPARAM)buffer);
			break;
		}

	case NOTFOUND:
		break;
			
	case INCORRECT:
		samples_incorrect++;
		_stprintf(buffer, TEXT("%i"), samples_incorrect);
		SendDlgItemMessage(hAudit, IDC_SAMPLES_INCORRECT, WM_SETTEXT, 0, (LPARAM)buffer);
		_stprintf(buffer, TEXT("%i"), samples_correct + samples_incorrect);
		SendDlgItemMessage(hAudit, IDC_SAMPLES_TOTAL, WM_SETTEXT, 0, (LPARAM)buffer);
		
		break;
	}

	sample_index++;
	SendDlgItemMessage(hAudit, IDC_SAMPLES_PROGRESS, PBM_SETPOS, sample_index, 0);
	
	if (sample_index == GetNumGames())
	{
		DetailsPrintf(_UIW(TEXT("Audit complete.\n")));
		SendDlgItemMessage(hAudit, IDCANCEL, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)_UIW(TEXT("&Close")));
		sample_index = -1;
	}
}

static void CLIB_DECL DetailsPrintf(const WCHAR *fmt, ...)
{
	HWND	hEdit;
	va_list marker;
	WCHAR	buffer[2000];
	//TCHAR*  t_s;
	int textLength;

	//RS 20030613 Different Ids for Property Page and Dialog
	// so see which one's currently instantiated
	hEdit = GetDlgItem(hAudit, IDC_AUDIT_DETAILS);
	if (hEdit ==  NULL)
		hEdit = GetDlgItem(hAudit, IDC_AUDIT_DETAILS_PROP);
	
	if (hEdit == NULL)
	{
		dprintf("audit detailsprintf() can't find any audit control\n");
		return;
	}

	va_start(marker, fmt);
	
	vswprintf(buffer, fmt, marker);
	
	va_end(marker);

	//t_s = tstring_from_utf8(ConvertToWindowsNewlines(buffer));
	//if( !t_s || _tcscmp(TEXT(""), t_s) == 0)
	//	return;

	textLength = Edit_GetTextLength(hEdit);
	Edit_SetSel(hEdit, textLength, textLength);
	SendMessage( hEdit, EM_REPLACESEL, FALSE, (LPARAM)buffer);

	//osd_free(t_s);
}

static const WCHAR *StatusString(int iStatus)
{
	static const WCHAR *ptr;

	ptr = _UIW(TEXT("Unknown"));

	switch (iStatus)
	{
	case CORRECT:
		ptr = _UIW(TEXT("Passed"));
		break;
		
	case BEST_AVAILABLE:
		ptr = _UIW(TEXT("Best available"));
		break;
		
	case NOTFOUND:
		ptr = _UIW(TEXT("Not found"));
		break;
		
	case INCORRECT:
		ptr = _UIW(TEXT("Failed"));
		break;
	}

	return ptr;
}
