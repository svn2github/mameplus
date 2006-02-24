/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2001 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>

#include "MAME32.h"
#include "resource.h"
#include "bitmask.h"
#include "TreeView.h"
#include "M32Util.h"

#if 0 //#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef  TreeView_GetNextItem
#define TreeView_GetNextItem(w,i,c) (HTREEITEM)(LRESULT)SendMessage((w),TVM_GETNEXTITEM,c,(LPARAM)(HTREEITEM)(i))

#undef  PropSheet_GetTabControl
#define PropSheet_GetTabControl(d) (HWND)(LRESULT)SendMessage(d,PSM_GETTABCONTROL,0,0)

#undef  PropSheet_GetCurrentPageHwnd
#define PropSheet_GetCurrentPageHwnd(d) (HWND)(LRESULT)SendMessage(d,PSM_GETCURRENTPAGEHWND,0,0)
#endif /* defined(__GNUC__) */

#include "translate.h"

#define MENUA		(ID_CONTEXT_SHOW_FOLDER_START)
#define NUM_MENUA	((ID_LANGUAGE_ENGLISH_US + UI_LANG_MAX - 1) - MENUA + 1)
#define MENUB		IDS_UI_FILE
#define NUM_MENUB	(IDD_MAIN - MENUB + 1)

#define NUM_POPSTR	4

static struct {
	HMENU hMenu;
	UINT  uiString;
} popstr[NUM_POPSTR];

static char *MenuStrings[NUM_MENUA + NUM_MENUB];
static char *MenuHelpStrings[NUM_MENUA + NUM_MENUB];

static HFONT hTranslateFont = NULL;

static BOOL force_change_font = TRUE;
static UINT ansi_codepage; 

void GetTranslatedFont(LOGFONTA *logfont)
{
	ZeroMemory (logfont, sizeof *logfont);

	logfont->lfHeight         = -10;
	logfont->lfWidth          = 0;
	logfont->lfEscapement     = 0;
	logfont->lfOrientation    = 0;
	logfont->lfWeight         = FW_NORMAL;
	logfont->lfItalic         = FALSE;
	logfont->lfUnderline      = FALSE;
	logfont->lfStrikeOut      = FALSE;
	logfont->lfCharSet        = ANSI_CHARSET;
	logfont->lfOutPrecision   = OUT_DEFAULT_PRECIS;
	logfont->lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	logfont->lfQuality        = DEFAULT_QUALITY;
	logfont->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	switch (ansi_codepage)
	{
	case 932:
		logfont->lfCharSet = SHIFTJIS_CHARSET;
		logfont->lfHeight  = -12;
		break;
	case 936:
		logfont->lfCharSet = GB2312_CHARSET;
		logfont->lfHeight  = -12;
		break;
	case 949:
		logfont->lfCharSet = HANGEUL_CHARSET;
		logfont->lfHeight  = -12;
		break;
	case 950:
		logfont->lfCharSet = CHINESEBIG5_CHARSET;
		logfont->lfHeight  = -12;
		break;
	}

	lstrcpyA(logfont->lfFaceName, _UI("MS Sans Serif"));
}


int InitTranslator(int langcode)
{
	LOGFONTA logfontA;

	ansi_codepage = ui_lang_info[langcode].codepage;

	set_langcode(langcode);

	if (hTranslateFont != NULL)
		DeleteObject(hTranslateFont);

	GetTranslatedFont(&logfontA);
	hTranslateFont = TranslateCreateFont(&logfontA);

	return langcode;
}


static void TranslateMenuW(HMENU hMenu)
{
	HMENU        hSubMenu;
	int          i;

	for (i = 0; i < GetMenuItemCount(hMenu); i++)
	{
		MENUITEMINFO  mii;
		WCHAR         buffer[1024];
		int           id;
		char         *p;

		hSubMenu = GetSubMenu(hMenu, i);
		if (hSubMenu != NULL )
			TranslateMenuW(hSubMenu);

		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_ID | MIIM_DATA | MIIM_TYPE;
		mii.fType      = MFT_STRING;
		mii.dwTypeData = buffer;
		mii.cch        = sizeof(buffer) / sizeof(*buffer);
		*buffer        = '\0';

		if (!GetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii) || !mii.wID)
			continue;

		id = mii.wID - MENUA;
		if (id < 0 || id >= NUM_MENUA)
		{
			int j;

			for (j = 0; j < NUM_POPSTR; j++)
				if (hMenu == popstr[j].hMenu)
				{
					mii.wID = popstr[j].uiString + i;
					if (popstr[j].uiString >= MENUA)
					{
						id = mii.wID - MENUA;
						if (id >= NUM_MENUA)
							continue;
					}
					else
					{
						id = mii.wID - MENUB + NUM_MENUA;
						if (id >= NUM_MENUA + NUM_MENUB)
							continue;
					}
					break;
				}

			if (j == NUM_POPSTR)
				continue;
		}

		if (!MenuStrings[id])
		{
			MenuStrings[id] = strdup(_String(buffer));
			if (!MenuStrings[id])
				continue;
		}

		if (!MenuHelpStrings[id])
		{
			LoadString(GetModuleHandle(NULL), mii.wID, buffer, sizeof(buffer) / sizeof(*buffer));
			MenuHelpStrings[id] = strdup(_String(buffer));
		}

		p = _UI(MenuStrings[id]);
		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_TYPE;
		mii.fType     |= MFT_STRING;
		mii.dwTypeData = _Unicode(p);
		mii.cch        = lstrlen(mii.dwTypeData);

		SetMenuItemInfo(hMenu, i, MF_BYPOSITION, &mii);
	}
}


static void TranslateMenuA(HMENU hMenu)
{
	HMENU        hSubMenu;
	int          i;

	for (i = 0; i < GetMenuItemCount(hMenu); i++)
	{
		MENUITEMINFOA mii;
		char          buffer[1024];
		int           id;
		char         *p;

		hSubMenu = GetSubMenu(hMenu, i);
		if (hSubMenu != NULL )
			TranslateMenuA(hSubMenu);

		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_ID | MIIM_DATA | MIIM_TYPE;
		mii.fType      = MFT_STRING;
		mii.dwTypeData = buffer;
		mii.cch        = sizeof(buffer) / sizeof(*buffer);
		*buffer        = '\0';

		if (!GetMenuItemInfoA(hMenu, i, MF_BYPOSITION, &mii) || !mii.wID)
			continue;

		id = mii.wID - MENUA;
		if (id < 0 || id >= NUM_MENUA)
		{
			int j;

			for (j = 0; j < NUM_POPSTR; j++)
				if (hMenu == popstr[j].hMenu)
				{
					mii.wID = popstr[j].uiString + i;
					if (popstr[j].uiString >= MENUA)
					{
						id = mii.wID - MENUA;
						if (id >= NUM_MENUA)
							continue;
					}
					else
					{
						id = mii.wID - MENUB + NUM_MENUA;
						if (id >= NUM_MENUA + NUM_MENUB)
							continue;
					}
					break;
				}

			if (j == NUM_POPSTR)
				continue;
		}

		if (!MenuStrings[id])
		{
			MenuStrings[id] = strdup(buffer);
			if (!MenuStrings[id])
				continue;
		}

		if (!MenuHelpStrings[id])
		{
			LoadStringA(GetModuleHandle(NULL), mii.wID, buffer, sizeof(buffer) / sizeof(*buffer));
			MenuHelpStrings[id] = strdup(buffer);
		}

		p = _UI(MenuStrings[id]);
		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_TYPE;
		mii.fType      |= MFT_STRING;
		mii.dwTypeData = p;
		mii.cch        = strlen(mii.dwTypeData);

		SetMenuItemInfoA(hMenu, i, MF_BYPOSITION, &mii);
	}
}


void TranslateMenu(HMENU hMenu, int uiString)
{
	// Setup normal menu
	if (!uiString)
	{
		// main menu title
		popstr[0].hMenu    = hMenu;
		popstr[0].uiString = IDS_UI_FILE;

		// View
		popstr[1].hMenu    = GetSubMenu(hMenu, 1);
		popstr[1].uiString = IDS_UI_TOOLBAR;

		// Options
		popstr[2].hMenu    = GetSubMenu(hMenu, 2);
		popstr[2].uiString = IDS_UI_FONT;

		// reset context popup
		popstr[3].hMenu    = NULL;
		popstr[3].uiString = 0;
	}
	// Setup sub menu for context popup
	else
	{
		popstr[3].hMenu    = hMenu;
		popstr[3].uiString = uiString;
	}

	if (OnNT())
		TranslateMenuW(hMenu);
	else
		TranslateMenuA(hMenu);
}


char  *TranslateMenuHelp(HMENU hMenu, UINT nIndex, int popup)
{
	int          id;

	id = nIndex - MENUA;
	if (id < 0 || id >= NUM_MENUA)
	{
		int j;

		if (!popup)
			return NULL;

		for (j = 0; j < NUM_POPSTR; j++)
			if (hMenu == popstr[j].hMenu)
			{
				nIndex = popstr[j].uiString + nIndex;
				if (popstr[j].uiString >= MENUA)
				{
					id = nIndex - MENUA;
					if (id >= NUM_MENUA)
						return NULL;
				}
				else
				{
					id = nIndex - MENUB + NUM_MENUA;
					if (id >= NUM_MENUA + NUM_MENUB)
						return NULL;
				}
				break;
			}

		if (j == NUM_POPSTR)
			return NULL;
	}

	if (!MenuHelpStrings[id])
		return NULL;

	return _UI(MenuHelpStrings[id]);
}


void TranslateControl(HWND hControl)
{
	WCHAR  buffer[1024];
	char  *p, *str;

	GetWindowText(hControl, buffer, sizeof(buffer) / sizeof(*buffer));
	str = _String(buffer);
	p = _UI(str);
	if (strcmp(p, str))
	{
		SetWindowFont(hControl, hTranslateFont, TRUE);
		SetWindowText(hControl, _Unicode(p));
	}
	else if (force_change_font)
		SetWindowFont(hControl, hTranslateFont, TRUE);
}


static void TranslateTabControl(HWND hControl)
{
	int  i;

	SetWindowFont(hControl, hTranslateFont, TRUE);

	for (i = 0; i < TabCtrl_GetItemCount(hControl); i++)
	{
		TC_ITEM  tci;
		WCHAR    buffer[1024];
		char    *p, *str;

		tci.mask = TCIF_TEXT;
		tci.pszText = buffer;
		tci.cchTextMax = sizeof(buffer) /  sizeof(*buffer);
		TabCtrl_GetItem(hControl, i, &tci);

		str = _String(buffer);
		p = _UI(str);
		if (strcmp(p, str))
		{
			tci.mask = TCIF_TEXT;
			tci.pszText = _Unicode(p);
			TabCtrl_SetItem(hControl, i, &tci);
		}
	}
}


static void translate_richedit20(HWND hControl)
{
	LOGFONTA logfontA;
	CHARFORMAT2 cfm;

	GetTranslatedFont(&logfontA);

	cfm.cbSize = sizeof (cfm);
	cfm.dwMask = CFM_CHARSET | CFM_FACE;
	cfm.bCharSet = logfontA.lfCharSet;
	lstrcpy(cfm.szFaceName, _Unicode(logfontA.lfFaceName));

	SendMessage(hControl, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cfm);
}


static BOOL CALLBACK translate_dialog_items(HWND hControl, LPARAM lParam)
{
	char  buffer[1024];

	GetClassNameA(hControl, buffer, sizeof(buffer) / sizeof(*buffer));
	if (!strcmp(buffer, "Button") || !strcmp(buffer, "Static"))
		TranslateControl(hControl);
	else if (!strcmp(buffer, "SysTabControl32"))
		TranslateTabControl(hControl);
	else if (!strcmp(buffer, "Edit") || !strcmp(buffer, "SysListView32") || !strcmp(buffer, "ComboBox"))
		SetWindowFont(hControl, hTranslateFont, TRUE);
	// only works RichEdit20W on Win9x
	else if (!strcmp(buffer, "RichEdit20W"))
		translate_richedit20(hControl);
	else if (!strcmp(buffer,"msctls_updown32"))
		ShowWindow(hControl, SW_HIDE);

	return TRUE;
}


void TranslateDialog(HWND hDlg, LPARAM lParam, BOOL change_font)
{
	force_change_font = change_font;
	TranslateControl(hDlg);
	EnumChildWindows(hDlg, translate_dialog_items, lParam);
	force_change_font = TRUE;
}


static void translate_tree_folder_items(HWND hWnd, HTREEITEM hti)
{
	while (hti)
	{
		LPTREEFOLDER lpFolder;
		TVITEM tvi;

		memset(&tvi, 0, sizeof tvi);
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hti;
		if (!TreeView_GetItem(hWnd, &tvi))
			break;

		lpFolder = (LPTREEFOLDER)tvi.lParam;
		if (!lpFolder)
			break;

		if (lpFolder->m_lpOriginalTitle)
		{
			char *translated;
			char *p;

			translated = lang_message(lpFolder->m_nCategoryID, lpFolder->m_lpOriginalTitle);

			p = strdup(translated);
			if (p)
			{
				free(lpFolder->m_lpTitle);
				lpFolder->m_lpTitle = p;
			}

			memset(&tvi, 0, sizeof tvi);
			tvi.mask = TVIF_HANDLE | TVIF_TEXT;
			tvi.hItem = hti;
			tvi.pszText = _Unicode(lpFolder->m_lpTitle);
			TreeView_SetItem(hWnd, &tvi);
		}

		translate_tree_folder_items(hWnd, TreeView_GetNextItem(hWnd, hti, TVGN_CHILD));

		hti = TreeView_GetNextItem(hWnd, hti, TVGN_NEXT);
	}
}


void TranslateTreeFolders(HWND hWnd)
{
	translate_tree_folder_items(hWnd, TreeView_GetNextItem(hWnd, 0, TVGN_ROOT));
}


/*
        Unicode Handlers
 */

#define TEMP_STRING_POOL_ENTRIES 16

LPWSTR _Unicode(const char *s)
{
	static LPWSTR temp_string_pool[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_alloc_len[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_index;
	size_t len;

	temp_string_pool_index %= TEMP_STRING_POOL_ENTRIES;

	len = MultiByteToWideChar(ansi_codepage, 0, s, -1, NULL, 0);
	if (!len)
		return NULL;

	if (len >= temp_string_pool_alloc_len[temp_string_pool_index])
	{
		if (temp_string_pool[temp_string_pool_index])
			free(temp_string_pool[temp_string_pool_index]);

		temp_string_pool[temp_string_pool_index] = malloc((len + 1) * sizeof(*temp_string_pool[temp_string_pool_index]));
		if (!temp_string_pool[temp_string_pool_index])
		{
			temp_string_pool_alloc_len[temp_string_pool_index] = 0;
			return NULL;
		}
		temp_string_pool_alloc_len[temp_string_pool_index] = len + 1;
	}
	if (!MultiByteToWideChar(ansi_codepage, 0, s, -1, temp_string_pool[temp_string_pool_index], temp_string_pool_alloc_len[temp_string_pool_index]))
		return NULL;

	return temp_string_pool[temp_string_pool_index++];
}

LPWSTR _UTF8Unicode(const char *s)
{
	static LPWSTR temp_string_pool[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_alloc_len[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_index;
	size_t len;

	temp_string_pool_index %= TEMP_STRING_POOL_ENTRIES;

	len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if (!len)
		return NULL;

	if (len >= temp_string_pool_alloc_len[temp_string_pool_index])
	{
		if (temp_string_pool[temp_string_pool_index])
			free(temp_string_pool[temp_string_pool_index]);

		temp_string_pool[temp_string_pool_index] = malloc((len + 1) * sizeof(*temp_string_pool[temp_string_pool_index]));
		if (!temp_string_pool[temp_string_pool_index])
		{
			temp_string_pool_alloc_len[temp_string_pool_index] = 0;
			return NULL;
		}
		temp_string_pool_alloc_len[temp_string_pool_index] = len + 1;
	}
	if (!MultiByteToWideChar(CP_UTF8, 0, s, -1, temp_string_pool[temp_string_pool_index], temp_string_pool_alloc_len[temp_string_pool_index]))
		return NULL;

	return temp_string_pool[temp_string_pool_index++];
}

char *_String(const LPWSTR ws)
{
	static char *temp_string_pool[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_alloc_len[TEMP_STRING_POOL_ENTRIES];
	static int temp_string_pool_index;
	size_t len;

	temp_string_pool_index %= TEMP_STRING_POOL_ENTRIES;

	len = WideCharToMultiByte(ansi_codepage, 0, ws, -1, 0, 0, NULL, NULL);

	if (!len)
		return NULL;

	if (len >= temp_string_pool_alloc_len[temp_string_pool_index])
	{
		if (temp_string_pool[temp_string_pool_index])
			free(temp_string_pool[temp_string_pool_index]);

		temp_string_pool[temp_string_pool_index] = malloc((len + 1) * sizeof(*temp_string_pool[temp_string_pool_index]));
		if (!temp_string_pool[temp_string_pool_index])
		{
			temp_string_pool_alloc_len[temp_string_pool_index] = 0;
			return NULL;
		}
		temp_string_pool_alloc_len[temp_string_pool_index] = len + 1;
	}
	if (!WideCharToMultiByte(ansi_codepage, 0, ws, -1, temp_string_pool[temp_string_pool_index], len, NULL, NULL))
		return NULL;

	return temp_string_pool[temp_string_pool_index++];
}

HFONT TranslateCreateFont(const LOGFONTA *lpLfA)
{
	LOGFONTW lfW;

	memcpy(&lfW, lpLfA, sizeof lfW);

	lstrcpyW(lfW.lfFaceName, _Unicode(lpLfA->lfFaceName));

	return CreateFontIndirectW(&lfW);
}

LRESULT StatusBarSetTextW(HWND hStatusBar,WPARAM id,LPCWSTR str)
{
	return (LRESULT)SendMessageW(hStatusBar, SB_SETTEXTW, id, (LPARAM)(void *)str);
}

LRESULT StatusBarSetTextA(HWND hStatusBar,WPARAM id,LPCSTR str)
{
	return StatusBarSetTextW(hStatusBar, id, _Unicode(str));
}


/*
 *      override WIN32API
 */

void ListView_GetItemTextA(HWND hwndCtl, int nIndex, int isitem, LPSTR lpch, int cchMax)
{
	static LPWSTR buf;
	char *p;

	if (buf)
		free(buf);
	buf = malloc((cchMax + 1) * sizeof (WCHAR));
	buf[0] = '\0';

	ListView_GetItemText(hwndCtl, nIndex, isitem, buf, cchMax);

	p = _String(buf);
	strncpy(lpch, p, cchMax);
}

int ComboBox_AddStringA(HWND hwndCtl, LPCSTR lpsz)
{
	return ComboBox_AddStringW(hwndCtl, (LPCWSTR)_Unicode(lpsz));
}

int ComboBox_InsertStringA(HWND hwndCtl, int nIndex, LPCSTR lpsz)
{
	return ComboBox_InsertStringW(hwndCtl, nIndex, (LPCWSTR)_Unicode(lpsz));
}

int ComboBox_FindStringA(HWND hwndCtl, int indexStart, LPCSTR lpszFind)
{
	return ComboBox_FindStringW(hwndCtl, indexStart, (LPCWSTR)_Unicode(lpszFind));
}

int ComboBox_GetLBTextA(HWND hwndCtl, int nIndex, LPSTR lpszBuffer)
{
	int ret;
	static LPWSTR buf;

	if (buf)
		free(buf);
	buf = malloc((ComboBox_GetLBTextLenW(hwndCtl, nIndex) + 1) * sizeof (WCHAR));

	ret = ComboBox_GetLBTextW(hwndCtl, nIndex, buf);
	if (!ret)
		return ret;

	strcpy(lpszBuffer, _String(buf));
	return ret;
}

int ComboBox_GetTextA(HWND hwndCtl, LPSTR lpch, int cchMax)
{
	char *p;
	int   ret;
	static LPWSTR buf;

	if (buf)
		free(buf);
	buf = malloc((cchMax + 1) * sizeof (WCHAR));

	ret = GetWindowTextW(hwndCtl, buf, cchMax);
	buf[cchMax] = '\0';

	if (!ret)
		return ret;

	p = _String(buf);
	strncpy(lpch, p, cchMax);

	return ret;
}

BOOL ComboBox_SetTextA(HWND hwndCtl, LPCSTR lpsz)
{
	return SetWindowTextW(hwndCtl, _Unicode(lpsz));

}
