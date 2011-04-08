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

  translate.c

  This is an unofficial version based on MAMEUI.
  Please do not send any reports from this build to the MAMEUI team.

***************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>

// MAMEUI headers
#include "winui.h"
#include "resource.h"
#include "bitmask.h"
#include "TreeView.h"
#include "mui_util.h"
#include "mui_opts.h"
#include "translate.h"

#if 0 //#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef  TreeView_GetNextItem
#define TreeView_GetNextItem(w,i,c) (HTREEITEM)(LRESULT)SendMessage((w),TVM_GETNEXTITEM,c,(LPARAM)(HTREEITEM)(i))

#undef  PropSheet_GetTabControl
#define PropSheet_GetTabControl(d) (HWND)(LRESULT)SendMessage(d,PSM_GETTABCONTROL,0,0)

#undef  PropSheet_GetCurrentPageHwnd
#define PropSheet_GetCurrentPageHwnd(d) (HWND)(LRESULT)SendMessage(d,PSM_GETCURRENTPAGEHWND,0,0)
#endif /* defined(__GNUC__) */

#define MENUA		(ID_CONTEXT_SHOW_FOLDER_START)
#define NUM_MENUA	((ID_LANGUAGE_ENGLISH_US + UI_LANG_MAX - 1) - MENUA + 1)
#define MENUB		IDS_UI_FILE
#define NUM_MENUB	(IDD_MAIN - MENUB + 1)

#define NUM_POPSTR	4

static struct {
	HMENU hMenu;
	UINT  uiString;
} popstr[NUM_POPSTR];

static WCHAR *MenuStrings[NUM_MENUA + NUM_MENUB];
static WCHAR *MenuHelpStrings[NUM_MENUA + NUM_MENUB];

static HFONT hTranslateFont = NULL;

static BOOL force_change_font = TRUE;
static UINT ansi_codepage; 

void GetTranslatedFont(LOGFONTW *logfont)
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

	wcscpy(logfont->lfFaceName, _UIW(TEXT("MS Sans Serif")));
}


int InitTranslator(int langcode)
{
	LOGFONTW logfont;

	ansi_codepage = ui_lang_info[langcode].codepage;

	lang_set_langcode(MameUIGlobal(), langcode);

	if (hTranslateFont != NULL)
		DeleteObject(hTranslateFont);

	GetTranslatedFont(&logfont);
	hTranslateFont = TranslateCreateFont(&logfont);

	return langcode;
}


static void TranslateMenuRecurse(HMENU hMenu)
{
	BOOL         isMenuBarItem = (hMenu == popstr[0].hMenu);
	HMENU        hSubMenu;
	MENUITEMINFO mii;
	int          i;

	for (i = GetMenuItemCount(hMenu) - 1; i >= 0; i--)
	{
		WCHAR         buffer[1024];
		int           id;
		WCHAR        *p;

		hSubMenu = GetSubMenu(hMenu, i);
		if (hSubMenu != NULL )
			TranslateMenuRecurse(hSubMenu);

		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_ID | MIIM_STRING | MIIM_FTYPE;
		mii.dwTypeData = buffer;
		mii.cch        = ARRAY_LENGTH(buffer);
		*buffer        = '\0';

		if (!GetMenuItemInfo(hMenu, i, TRUE, &mii) || !mii.wID)
			continue;

		if (mii.fType & MFT_SEPARATOR)
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
			MenuStrings[id] = win_tstring_strdup(buffer);
			if (!MenuStrings[id])
				continue;
		}

		if (!MenuHelpStrings[id])
		{
			LoadString(GetModuleHandle(NULL), mii.wID, buffer, ARRAY_LENGTH(buffer));
			MenuHelpStrings[id] = win_tstring_strdup(buffer);
		}

		p = _UIW(MenuStrings[id]);

		if (isMenuBarItem)
			ModifyMenu(hMenu, i, MF_BYPOSITION, mii.wID, p);

		mii.cbSize     = sizeof(MENUITEMINFO);
		mii.fMask      = MIIM_STRING | MIIM_FTYPE;
		mii.dwTypeData = p;
		mii.cch        = wcslen(mii.dwTypeData);

		SetMenuItemInfo(hMenu, i, TRUE, &mii);
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
		popstr[1].uiString = IDS_VIEW_TOOLBAR;

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

	TranslateMenuRecurse(hMenu);
}


WCHAR *TranslateMenuHelp(HMENU hMenu, UINT nIndex, int popup)
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

	return _UIW(MenuHelpStrings[id]);
}


void TranslateControl(HWND hControl)
{
	WCHAR  buffer[1024];
	WCHAR *p;

	GetWindowText(hControl, buffer, ARRAY_LENGTH(buffer));
	p = _UIW(buffer);
	if (p != buffer)
	{
		SetWindowFont(hControl, hTranslateFont, TRUE);
		SetWindowText(hControl, p);
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
		WCHAR    *p;
		HRESULT res;

		tci.mask = TCIF_TEXT;
		tci.pszText = buffer;
		tci.cchTextMax = ARRAY_LENGTH(buffer);
		res = TabCtrl_GetItem(hControl, i, &tci);

		p = _UIW(buffer);
		if (p != buffer)
		{
			tci.mask = TCIF_TEXT;
			tci.pszText = p;
			res = TabCtrl_SetItem(hControl, i, &tci);
		}
	}
}


static BOOL CALLBACK translate_dialog_items(HWND hControl, LPARAM lParam)
{
	char  buffer[1024];

	GetClassNameA(hControl, buffer, ARRAY_LENGTH(buffer));
	if (!strcmp(buffer, "Button") || !strcmp(buffer, "Static"))
		TranslateControl(hControl);
	else if (!strcmp(buffer, "SysTabControl32"))
		TranslateTabControl(hControl);
	else if (!strcmp(buffer, "Edit") || !strcmp(buffer, "SysListView32") || !strcmp(buffer, "ComboBox"))
		SetWindowFont(hControl, hTranslateFont, TRUE);
//	else if (!strcmp(buffer,"msctls_updown32"))
//		ShowWindow(hControl, SW_HIDE);

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
			WCHAR *translated;
			WCHAR *p;
			HRESULT res;

			translated = w_lang_message(lpFolder->m_nCategoryID, lpFolder->m_lpOriginalTitle);

			p = win_tstring_strdup(translated);
			if (p)
			{
				osd_free(lpFolder->m_lpTitle);
				lpFolder->m_lpTitle = p;
			}

			memset(&tvi, 0, sizeof tvi);
			tvi.mask = TVIF_HANDLE | TVIF_TEXT;
			tvi.hItem = hti;
			tvi.pszText = lpFolder->m_lpTitle;
			res = TreeView_SetItem(hWnd, &tvi);
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

#define TMP_STRING_POOL_ENTRIES 32

static bool translate_firsttime = true;
static LPWSTR tmp_unicode_string_pool[TMP_STRING_POOL_ENTRIES];
static int tmp_unicode_string_pool_alloc_len[TMP_STRING_POOL_ENTRIES];
static LPWSTR tmp_utf8_string_pool[TMP_STRING_POOL_ENTRIES];
static int tmp_utf8_string_pool_alloc_len[TMP_STRING_POOL_ENTRIES];
static char *tmp_str_string_pool[TMP_STRING_POOL_ENTRIES];
static int tmp_str_string_pool_alloc_len[TMP_STRING_POOL_ENTRIES];

static void InitializeTranslateBuffer(void)
{
	translate_firsttime = false;

	for (int i = 0; i < TMP_STRING_POOL_ENTRIES; i++)
	{
		tmp_unicode_string_pool[i] = NULL;
		tmp_unicode_string_pool_alloc_len[i] = 0;
		tmp_utf8_string_pool[i] = NULL;
		tmp_utf8_string_pool_alloc_len[i] = 0;
		tmp_str_string_pool[i] = NULL;
		tmp_str_string_pool_alloc_len[i] = 0;
	}
}

void FreeTranslateBuffer(void)
{
	//Not initialized
	if (translate_firsttime) return;

	for (int i = 0; i < TMP_STRING_POOL_ENTRIES; i++)
	{
		if (tmp_unicode_string_pool[i])
			free(tmp_unicode_string_pool[i]);
		if (tmp_utf8_string_pool[i])
			free(tmp_utf8_string_pool[i]);
		if (tmp_str_string_pool[i])
			free(tmp_str_string_pool[i]);
	}

}

LPWSTR _Unicode(const char *s)
{
	static int tmp_string_pool_index = 0;
	size_t len;

	if (translate_firsttime) InitializeTranslateBuffer();

	tmp_string_pool_index %= TMP_STRING_POOL_ENTRIES;

	len = MultiByteToWideChar(ansi_codepage, 0, s, -1, NULL, 0);
	if (!len)
		return NULL;

	if (len >= tmp_unicode_string_pool_alloc_len[tmp_string_pool_index])
	{
		if (tmp_unicode_string_pool[tmp_string_pool_index])
			free(tmp_unicode_string_pool[tmp_string_pool_index]);

		tmp_unicode_string_pool[tmp_string_pool_index] = (WCHAR *)malloc((len + 1) * sizeof(*tmp_unicode_string_pool[tmp_string_pool_index]));
		if (!tmp_unicode_string_pool[tmp_string_pool_index])
		{
			tmp_unicode_string_pool_alloc_len[tmp_string_pool_index] = 0;
			return NULL;
		}
		tmp_unicode_string_pool_alloc_len[tmp_string_pool_index] = len + 1;
	}
	if (!MultiByteToWideChar(ansi_codepage, 0, s, -1, tmp_unicode_string_pool[tmp_string_pool_index], tmp_unicode_string_pool_alloc_len[tmp_string_pool_index]))
		return NULL;

	return tmp_unicode_string_pool[tmp_string_pool_index++];
}

LPWSTR _UTF8Unicode(const char *s)
{
	static int tmp_string_pool_index = 0;
	size_t len;

	if (translate_firsttime) InitializeTranslateBuffer();

	tmp_string_pool_index %= TMP_STRING_POOL_ENTRIES;

	len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if (!len)
		return NULL;

	if (len >= tmp_utf8_string_pool_alloc_len[tmp_string_pool_index])
	{
		if (tmp_utf8_string_pool[tmp_string_pool_index])
			free(tmp_utf8_string_pool[tmp_string_pool_index]);

		tmp_utf8_string_pool[tmp_string_pool_index] = (WCHAR *)malloc((len + 1) * sizeof(*tmp_utf8_string_pool[tmp_string_pool_index]));
		if (!tmp_utf8_string_pool[tmp_string_pool_index])
		{
			tmp_utf8_string_pool_alloc_len[tmp_string_pool_index] = 0;
			return NULL;
		}
		tmp_utf8_string_pool_alloc_len[tmp_string_pool_index] = len + 1;
	}
	if (!MultiByteToWideChar(CP_UTF8, 0, s, -1, tmp_utf8_string_pool[tmp_string_pool_index], tmp_utf8_string_pool_alloc_len[tmp_string_pool_index]))
		return NULL;

	return tmp_utf8_string_pool[tmp_string_pool_index++];
}

char *_String(const WCHAR *ws)
{
	static int tmp_string_pool_index = 0;
	size_t len;

	if (translate_firsttime) InitializeTranslateBuffer();

	tmp_string_pool_index %= TMP_STRING_POOL_ENTRIES;

	len = WideCharToMultiByte(ansi_codepage, 0, ws, -1, 0, 0, NULL, NULL);

	if (!len)
		return NULL;

	if (len >= tmp_str_string_pool_alloc_len[tmp_string_pool_index])
	{
		if (tmp_str_string_pool[tmp_string_pool_index])
			free(tmp_str_string_pool[tmp_string_pool_index]);

		tmp_str_string_pool[tmp_string_pool_index] = (char *)malloc((len + 1) * sizeof(*tmp_str_string_pool[tmp_string_pool_index]));
		if (!tmp_str_string_pool[tmp_string_pool_index])
		{
			tmp_str_string_pool_alloc_len[tmp_string_pool_index] = 0;
			return NULL;
		}
		tmp_str_string_pool_alloc_len[tmp_string_pool_index] = len + 1;
	}
	if (!WideCharToMultiByte(ansi_codepage, 0, ws, -1, tmp_str_string_pool[tmp_string_pool_index], len, NULL, NULL))
		return NULL;

	return tmp_str_string_pool[tmp_string_pool_index++];
}

HFONT TranslateCreateFont(const LOGFONTW *lpLf)
{
	return CreateFontIndirectW(lpLf);
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
	buf = (WCHAR *)malloc((cchMax + 1) * sizeof (*buf));
	buf[0] = '\0';

	ListView_GetItemText(hwndCtl, nIndex, isitem, buf, cchMax);

	p = _String(buf);
	strncpy(lpch, p, cchMax);
}

int ComboBox_AddStringA(HWND hwndCtl, LPCSTR lpsz)
{
	return ComboBox_AddStringW(hwndCtl, _Unicode(lpsz));
}

int ComboBox_InsertStringA(HWND hwndCtl, int nIndex, LPCSTR lpsz)
{
	return ComboBox_InsertStringW(hwndCtl, nIndex, _Unicode(lpsz));
}

int ComboBox_FindStringA(HWND hwndCtl, int indexStart, LPCSTR lpszFind)
{
	return ComboBox_FindStringW(hwndCtl, indexStart, _Unicode(lpszFind));
}

int ComboBox_GetLBTextA(HWND hwndCtl, int nIndex, LPSTR lpszBuffer)
{
	int ret;
	static LPWSTR buf;

	if (buf)
		free(buf);
	buf = (WCHAR *)malloc((ComboBox_GetLBTextLenW(hwndCtl, nIndex) + 1) * sizeof (*buf));

	ret = ComboBox_GetLBTextW(hwndCtl, nIndex, buf);
	if (!ret)
		return ret;

	strcpy(lpszBuffer, _String(buf));
	return ret;
}

int ComboBox_GetLBTextLenA(HWND hwndCtl, int nIndex)
{
	static WCHAR *buf;

	if (buf)
		free(buf);
	buf = (WCHAR *)malloc((ComboBox_GetLBTextLenW(hwndCtl, nIndex) + 1) * sizeof (*buf));

	ComboBox_GetLBTextW(hwndCtl, nIndex, buf);
	return strlen(_String(buf));
}

int ComboBox_GetTextA(HWND hwndCtl, LPSTR lpch, int cchMax)
{
	char *p;
	int   ret;
	static LPWSTR buf;

	if (buf)
		free(buf);
	buf = (WCHAR *)malloc((cchMax + 1) * sizeof (*buf));

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

int ComboBox_AddStringW(HWND hwndCtl, LPWSTR lpsz)
{
	DWORD result;

	result = SendMessageW(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpsz);
	return (int)result;
}

int ComboBox_InsertStringW(HWND hwndCtl, int index, LPWSTR lpsz)
{
	DWORD result;

	result = SendMessageW(hwndCtl, CB_INSERTSTRING, index, (LPARAM)lpsz);
	return (int)result;
}

int ComboBox_FindStringW(HWND hwndCtl, int indexStart, LPWSTR lpszFind)
{
	DWORD result;

	result = SendMessageW(hwndCtl, CB_FINDSTRING, indexStart, (LPARAM)lpszFind);
	return (int)result;
}

int ComboBox_GetLBTextW(HWND hwndCtl, int index, LPWSTR lpszBuffer)
{
	DWORD result;

	result = SendMessageW(hwndCtl, CB_GETLBTEXT, index, (LPARAM)lpszBuffer);
	return (int)result;
}

	DWORD result;

int ComboBox_GetLBTextLenW(HWND hwndCtl, int index)
{
	DWORD result;

	result = SendMessageW(hwndCtl, CB_GETLBTEXTLEN, index, 0);
	return (int)result;
}



#undef _wfopen
FILE *wfopen(const WCHAR *fname, const WCHAR *mode)
{
	return _wfopen(fname, mode);
}


static int wcmp(const void *p1, const void *p2)
{
	return wcscmp((const wchar_t *)p1, (const wchar_t *)p2);
}

WCHAR *w_lang_message(int msgcat, const WCHAR *str)
{
	return (WCHAR *)lang_messagew(msgcat, str, wcmp);
}


#if 0 //temporary to keep compatibility

#undef malloc
#undef realloc
#undef free

struct mb_msg
{
	WCHAR *wstr;
	char *mbstr;
};

static int mb_msg_num;
static int mb_msg_size;
static struct mb_msg *mb_msg_index;

static int mb_msg_cmp(const void *p1, const void *p2)
{
	return ((struct mb_msg *)p1)->wstr - ((struct mb_msg *)p2)->wstr;
}

char *mb_lang_message(int msgcat, const char *str)
{
	struct mb_msg *p;
	struct mb_msg temp;
	WCHAR *wid;
	WCHAR *wstr;

	wid = _Unicode(str);
	wstr = (WCHAR *)lang_messagew(msgcat, wid, wcmp);

	if (wid == wstr)
		return (char *)str;
dprintf("mb_lang_message: %s", str);

	if (mb_msg_index == NULL)
	{
		mb_msg_size = 1024;
		mb_msg_index = (mb_msg *)malloc(mb_msg_size * sizeof (*mb_msg_index));
		mb_msg_index[0].wstr = wstr;
		mb_msg_index[0].mbstr = mame_strdup(_String(wstr));
		mb_msg_num = 1;

		return mb_msg_index[0].mbstr;
	}

	temp.wstr = wstr;
	p = (struct mb_msg *)bsearch(&temp, mb_msg_index, mb_msg_num, sizeof (*mb_msg_index), mb_msg_cmp);
	if (p)
		return p->mbstr;

	if (mb_msg_num == mb_msg_size)
	{
		mb_msg_size += 1024;
		mb_msg_index = (mb_msg *)realloc(mb_msg_index, mb_msg_size * sizeof (*mb_msg_index));
	}

	temp.mbstr = mame_strdup(_String(wstr));
	mb_msg_index[mb_msg_num++] = temp;

	qsort(mb_msg_index, mb_msg_num, sizeof (*mb_msg_index), mb_msg_cmp);

	return temp.mbstr;
}
#endif
