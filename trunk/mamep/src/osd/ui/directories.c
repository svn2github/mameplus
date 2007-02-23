/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

/***************************************************************************

  Directories.c

***************************************************************************/

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <sys/stat.h>
#include <assert.h>

#include "MAME32.h"	// include this first
#include "screenshot.h"
#include "Directories.h"
#include "resource.h"
#include "m32util.h"
#include "translate.h"

#if defined(__GNUC__)
/* fix warning: cast does not match function type */
#undef ListView_GetEditControl
//#define ListView_GetEditControl(w) (HWND)(LRESULT)SendMessage((w),LVM_GETEDITCONTROL,0,0)
static HWND ListView_GetEditControl(HWND w)
{
	LRESULT hwnd;

	hwnd = SendMessage((w),LVM_GETEDITCONTROL,0,0);
	return (HWND)hwnd;
}
#endif /* defined(__GNUC__) */

#define MAX_DIRS 64

/***************************************************************************
    Internal structures
 ***************************************************************************/

typedef struct
{
	WCHAR   m_Directories[MAX_DIRS][MAX_PATH];
	int     m_NumDirectories;
	BOOL    m_bModified;
} tPath;

typedef union
{
	tPath	*m_Path;
	WCHAR	*m_Directory;
	void    *m_Ptr;
} tDirInfo;

/***************************************************************************
    Function prototypes
 ***************************************************************************/

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
#ifdef MESS
BOOL BrowseForDirectory(HWND hwnd, const WCHAR* pStartDir, WCHAR* pResult);
#else
static BOOL         BrowseForDirectory(HWND hwnd, const WCHAR* pStartDir, WCHAR* pResult);
#endif

static void     DirInfo_SetDir(tDirInfo *pInfo, int nType, int nItem, const WCHAR* pText);
static WCHAR*   DirInfo_Dir(tDirInfo *pInfo, int nType);
static WCHAR*   DirInfo_Path(tDirInfo *pInfo, int nType, int nItem);
static void     DirInfo_SetModified(tDirInfo *pInfo, int nType, BOOL bModified);
static BOOL     DirInfo_Modified(tDirInfo *pInfo, int nType);
static WCHAR*   FixSlash(WCHAR* s);

static void     UpdateDirectoryList(HWND hDlg);

static void     Directories_OnSelChange(HWND hDlg);
static BOOL     Directories_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
static void     Directories_OnDestroy(HWND hDlg);
static void     Directories_OnClose(HWND hDlg);
static void     Directories_OnOk(HWND hDlg);
static void     Directories_OnCancel(HWND hDlg);
static void     Directories_OnInsert(HWND hDlg);
static void     Directories_OnBrowse(HWND hDlg);
static void     Directories_OnDelete(HWND hDlg);
static BOOL     Directories_OnBeginLabelEditA(HWND hDlg, NMHDR* pNMHDR);
static BOOL     Directories_OnEndLabelEditA(HWND hDlg, NMHDR* pNMHDR);
static BOOL     Directories_OnBeginLabelEditW(HWND hDlg, NMHDR* pNMHDR);
static BOOL     Directories_OnEndLabelEditW(HWND hDlg, NMHDR* pNMHDR);
static void     Directories_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
static BOOL     Directories_OnNotify(HWND hDlg, int id, NMHDR* pNMHDR);

/***************************************************************************
    External variables
 ***************************************************************************/

/***************************************************************************
    Internal variables
 ***************************************************************************/

static tDirInfo *g_pDirInfo;

/***************************************************************************
    External function definitions
 ***************************************************************************/

INT_PTR CALLBACK DirectoriesDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (Msg)
	{
	case WM_INITDIALOG:
		TranslateDialog(hDlg, lParam, FALSE);

		return (BOOL)HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Directories_OnInitDialog);

	case WM_COMMAND:
		HANDLE_WM_COMMAND(hDlg, wParam, lParam, Directories_OnCommand);
		bReturn = TRUE;
		break;

	case WM_NOTIFY:
		return (BOOL)HANDLE_WM_NOTIFY(hDlg, wParam, lParam, Directories_OnNotify);
		break;

	case WM_CLOSE:
		HANDLE_WM_CLOSE(hDlg, wParam, lParam, Directories_OnClose);
		break;

	case WM_DESTROY:
		HANDLE_WM_DESTROY(hDlg, wParam, lParam, Directories_OnDestroy);
		break;

	default:
		bReturn = FALSE;
	}
	return bReturn;
}

/***************************************************************************
	Internal function definitions
 ***************************************************************************/

static BOOL IsMultiDir(int nType)
{
	return g_directoryInfo[nType].bMulti;
}

static void DirInfo_SetDir(tDirInfo *pInfo, int nType, int nItem, const WCHAR* pText)
{
	WCHAR *s;
	WCHAR *pOldText;

	if (IsMultiDir(nType))
	{
		assert(nItem >= 0);
		lstrcpy(DirInfo_Path(pInfo, nType, nItem), pText);
		DirInfo_SetModified(pInfo, nType, TRUE);
	}
	else
	{
		s = wcsdup(pText);
		if (!s)
			return;
		pOldText = pInfo[nType].m_Directory;
		if (pOldText)
			free((char *)pOldText);
		pInfo[nType].m_Directory = s;
	}
}

static WCHAR* DirInfo_Dir(tDirInfo *pInfo, int nType)
{
	assert(!IsMultiDir(nType));
	return pInfo[nType].m_Directory;
}

static WCHAR* DirInfo_Path(tDirInfo *pInfo, int nType, int nItem)
{
	return pInfo[nType].m_Path->m_Directories[nItem];
}

static void DirInfo_SetModified(tDirInfo *pInfo, int nType, BOOL bModified)
{
	assert(IsMultiDir(nType));
	pInfo[nType].m_Path->m_bModified = bModified;
}

static BOOL DirInfo_Modified(tDirInfo *pInfo, int nType)
{
	assert(IsMultiDir(nType));
	return pInfo[nType].m_Path->m_bModified;
}

#define DirInfo_NumDir(pInfo, path) \
	((pInfo)[(path)].m_Path->m_NumDirectories)

/* lop off trailing backslash if it exists */
static WCHAR* FixSlash(WCHAR* s)
{
	int len = 0;
	
	if (s)
		len = lstrlen(s);

	if (len && s[len - 1] == *PATH_SEPARATOR)
		s[len - 1] = '\0';

	return s;
}

static void UpdateDirectoryList(HWND hDlg)
{
	int 	i;
	int 	nType;
	LV_ITEM Item;
	HWND	hList  = GetDlgItem(hDlg, IDC_DIR_LIST);
	HWND	hCombo = GetDlgItem(hDlg, IDC_DIR_COMBO);

	/* Remove previous */

	ListView_DeleteAllItems(hList);

	/* Update list */

	memset(&Item, 0, sizeof(LV_ITEM));
	Item.mask = LVIF_TEXT;

	nType = ComboBox_GetCurSel(hCombo);
	if (IsMultiDir(nType))
	{
		Item.pszText = TEXT(DIRLIST_NEWENTRYTEXT);
		ListView_InsertItem(hList, &Item);
		for (i = DirInfo_NumDir(g_pDirInfo, nType) - 1; 0 <= i; i--)
		{
			Item.pszText = DirInfo_Path(g_pDirInfo, nType, i);
			ListView_InsertItem(hList, &Item);
		}
	}
	else
	{
		Item.pszText = DirInfo_Dir(g_pDirInfo, nType);
		ListView_InsertItem(hList, &Item);
	}

	/* select first one */

	ListView_SetItemState(hList, 0, LVIS_SELECTED, LVIS_SELECTED);
}

static void Directories_OnSelChange(HWND hDlg)
{
	UpdateDirectoryList(hDlg);
	
	if (ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO)) == 0
	||	ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO)) == 1)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_DIR_DELETE), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_DIR_INSERT), TRUE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, IDC_DIR_DELETE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_DIR_INSERT), FALSE);
	}
}

static BOOL Directories_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	RECT		rectClient;
	LVCOLUMN	LVCol;
	int 		i;
	int			nDirInfoCount;
	const WCHAR*	s;
	WCHAR *token;
	WCHAR buf[MAX_PATH * MAX_DIRS];

	/* count how many dirinfos there are */
	nDirInfoCount = 0;
	while(g_directoryInfo[nDirInfoCount].lpName)
		nDirInfoCount++;

	g_pDirInfo = (tDirInfo *) malloc(sizeof(tDirInfo) * nDirInfoCount);
	if (!g_pDirInfo) /* bummer */
		goto error;
	memset(g_pDirInfo, 0, sizeof(tDirInfo) * nDirInfoCount);

	for (i = nDirInfoCount - 1; i >= 0; i--)
	{
		ComboBox_InsertString(GetDlgItem(hDlg, IDC_DIR_COMBO), 0, _UIW(g_directoryInfo[i].lpName));
	}

	ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO), 0);

	GetClientRect(GetDlgItem(hDlg, IDC_DIR_LIST), &rectClient);

	memset(&LVCol, 0, sizeof(LVCOLUMN));
	LVCol.mask	  = LVCF_WIDTH;
	LVCol.cx	  = rectClient.right - rectClient.left - GetSystemMetrics(SM_CXHSCROLL);
	
	ListView_InsertColumn(GetDlgItem(hDlg, IDC_DIR_LIST), 0, &LVCol);

	/* Keep a temporary copy of the directory strings in g_pDirInfo. */
	for (i = 0; i < nDirInfoCount; i++)
	{
		s = g_directoryInfo[i].pfnGetTheseDirs();
		if (g_directoryInfo[i].bMulti)
		{
			/* Copy the string to our own buffer so that we can mutilate it */
			lstrcpy(buf, s);

			g_pDirInfo[i].m_Path = malloc(sizeof(tPath));
			if (!g_pDirInfo[i].m_Path)
				goto error;

			g_pDirInfo[i].m_Path->m_NumDirectories = 0;
			token = wcstok(buf, TEXT(";"));
			while ((DirInfo_NumDir(g_pDirInfo, i) < MAX_DIRS) && token)
			{
				lstrcpy(DirInfo_Path(g_pDirInfo, i, DirInfo_NumDir(g_pDirInfo, i)), token);
				DirInfo_NumDir(g_pDirInfo, i)++;
				token = wcstok(NULL, TEXT(";"));
			}
			DirInfo_SetModified(g_pDirInfo, i, FALSE);
		}
		else
		{
			DirInfo_SetDir(g_pDirInfo, i, -1, s);
		}
	}

	UpdateDirectoryList(hDlg);
	return TRUE;

error:
	Directories_OnDestroy(hDlg);
	EndDialog(hDlg, -1);
	return FALSE;
}

static void Directories_OnDestroy(HWND hDlg)
{
	int nDirInfoCount, i;

	if (g_pDirInfo)
	{
		/* count how many dirinfos there are */
		nDirInfoCount = 0;
		while(g_directoryInfo[nDirInfoCount].lpName)
			nDirInfoCount++;

		for (i = 0; i < nDirInfoCount; i++)
		{
			if (g_pDirInfo[i].m_Ptr)
				free(g_pDirInfo[i].m_Ptr);
		}
		free(g_pDirInfo);
		g_pDirInfo = NULL;
	}
}

static void Directories_OnClose(HWND hDlg)
{
	EndDialog(hDlg, IDCANCEL);
}

static int RetrieveDirList(int nDir, int nFlagResult, void (*SetTheseDirs)(const WCHAR *s))
{
	int i;
	int nResult = 0;
	int nPaths;
	WCHAR buf[MAX_PATH * MAX_DIRS];

	if (DirInfo_Modified(g_pDirInfo, nDir))
	{
		memset(buf, 0, sizeof(buf));
		nPaths = DirInfo_NumDir(g_pDirInfo, nDir);
		for (i = 0; i < nPaths; i++)
		{

			lstrcat(buf, FixSlash(DirInfo_Path(g_pDirInfo, nDir, i)));

			if (i < nPaths - 1)
				lstrcat(buf, TEXT(";"));
		}
		SetTheseDirs(buf);

		nResult |= nFlagResult;
    }
	return nResult;
}

static void Directories_OnOk(HWND hDlg)
{
	int i;
	int nResult = 0;
	WCHAR *s;

	for (i = 0; g_directoryInfo[i].lpName; i++)
	{
		if (g_directoryInfo[i].bMulti)
		{
			nResult |= RetrieveDirList(i, g_directoryInfo[i].nDirDlgFlags, g_directoryInfo[i].pfnSetTheseDirs);
		}
		else
		{
			s = FixSlash(DirInfo_Dir(g_pDirInfo, i));
			g_directoryInfo[i].pfnSetTheseDirs(s);
		}
	}
	EndDialog(hDlg, nResult);
}

static void Directories_OnCancel(HWND hDlg)
{
	EndDialog(hDlg, IDCANCEL);
}

static void Directories_OnInsert(HWND hDlg)
{
	int 	nItem;
	WCHAR	buf[MAX_PATH];
	HWND	hList;

	hList = GetDlgItem(hDlg, IDC_DIR_LIST);
	nItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (BrowseForDirectory(hDlg, NULL, buf) == TRUE)
	{
		int 	i;
		int 	nType;

		/* list was empty */
		if (nItem == -1)
			nItem = 0;

		nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
		if (IsMultiDir(nType))
		{
			if (MAX_DIRS <= DirInfo_NumDir(g_pDirInfo, nType))
				return;

			for (i = DirInfo_NumDir(g_pDirInfo, nType); nItem < i; i--)
				lstrcpy(DirInfo_Path(g_pDirInfo, nType, i),
					   DirInfo_Path(g_pDirInfo, nType, i - 1));

			lstrcpy(DirInfo_Path(g_pDirInfo, nType, nItem), buf);
			DirInfo_NumDir(g_pDirInfo, nType)++;
			DirInfo_SetModified(g_pDirInfo, nType, TRUE);
		}

		UpdateDirectoryList(hDlg);

		ListView_SetItemState(hList, nItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	}
}

static void Directories_OnBrowse(HWND hDlg)
{
	int 	nType;
	int 	nItem;
	WCHAR	inbuf[MAX_PATH];
	WCHAR	outbuf[MAX_PATH];
	HWND	hList;

	hList = GetDlgItem(hDlg, IDC_DIR_LIST);
	nItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (nItem == -1)
		return;

	nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
	if (IsMultiDir(nType))
	{
		/* Last item is placeholder for append */
		if (nItem == ListView_GetItemCount(hList) - 1)
		{
			Directories_OnInsert(hDlg); 
			return;
		}
	}

	ListView_GetItemText(hList, nItem, 0, inbuf, MAX_PATH);

	if (BrowseForDirectory(hDlg, inbuf, outbuf) == TRUE)
	{
		nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
		DirInfo_SetDir(g_pDirInfo, nType, nItem, outbuf);
		UpdateDirectoryList(hDlg);
	}
}

static void Directories_OnDelete(HWND hDlg)
{
	int 	nType;
	int 	nCount;
	int 	nSelect;
	int 	i;
	int 	nItem;
	HWND	hList = GetDlgItem(hDlg, IDC_DIR_LIST);

	nItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED | LVNI_ALL);

	if (nItem == -1)
		return;

	/* Don't delete "Append" placeholder. */
	if (nItem == ListView_GetItemCount(hList) - 1)
		return;

	nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
	if (IsMultiDir(nType))
	{
		for (i = nItem; i < DirInfo_NumDir(g_pDirInfo, nType) - 1; i++)
			lstrcpy(DirInfo_Path(g_pDirInfo, nType, i),
				   DirInfo_Path(g_pDirInfo, nType, i + 1));

		lstrcpy(DirInfo_Path(g_pDirInfo, nType, DirInfo_NumDir(g_pDirInfo, nType) - 1), TEXT(""));
		DirInfo_NumDir(g_pDirInfo, nType)--;

		DirInfo_SetModified(g_pDirInfo, nType, TRUE);
	}

	UpdateDirectoryList(hDlg);
	

	nCount = ListView_GetItemCount(hList);
	if (nCount <= 1)
		return;

	/* If the last item was removed, select the item above. */
	if (nItem == nCount - 1)
		nSelect = nCount - 2;
	else
		nSelect = nItem;

	ListView_SetItemState(hList, nSelect, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

static BOOL Directories_OnBeginLabelEditA(HWND hDlg, NMHDR* pNMHDR)
{
	int 		  nType;
	BOOL		  bResult = FALSE;
	NMLVDISPINFOA    *pDispInfo = (NMLVDISPINFOA*)pNMHDR;
	LVITEMA          *pItem = &pDispInfo->item;

	nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
	if (IsMultiDir(nType))
	{
		/* Last item is placeholder for append */
		if (pItem->iItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_DIR_LIST)) - 1)
		{
			HWND hEdit;

			if (MAX_DIRS <= DirInfo_NumDir(g_pDirInfo, nType))
				return TRUE; /* don't edit */

			hEdit = ListView_GetEditControl(GetDlgItem(hDlg, IDC_DIR_LIST));
			Edit_SetText(hEdit, TEXT(""));
		}
	}

	return bResult;
}

static BOOL Directories_OnEndLabelEditA(HWND hDlg, NMHDR* pNMHDR)
{
	BOOL		  bResult = FALSE;
	NMLVDISPINFOA    *pDispInfo = (NMLVDISPINFOA*)pNMHDR;
	LVITEMA          *pItem = &pDispInfo->item;

	if (pItem->pszText != NULL)
	{
		struct stat file_stat;

		/* Don't allow empty entries. */
		if (!strcmp(pItem->pszText, ""))
		{
			return FALSE;
		}

		/* Check validity of edited directory. */
		if (stat(pItem->pszText, &file_stat) == 0
		&&	(file_stat.st_mode & S_IFDIR))
		{
			bResult = TRUE;
		}
		else
		{
			if (MessageBox(NULL, _UIW(TEXT("Directory does not exist, continue anyway?")), TEXT_MAME32NAME, MB_OKCANCEL) == IDOK)
				bResult = TRUE;
		}
	}

	if (bResult == TRUE)
	{
		int nType;
		int i;

		nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
		if (IsMultiDir(nType))
		{
			/* Last item is placeholder for append */
			if (pItem->iItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_DIR_LIST)) - 1)
			{
				if (MAX_DIRS <= DirInfo_NumDir(g_pDirInfo, nType))
					return FALSE;

				for (i = DirInfo_NumDir(g_pDirInfo, nType); pItem->iItem < i; i--)
					lstrcpy(DirInfo_Path(g_pDirInfo, nType, i),
						   DirInfo_Path(g_pDirInfo, nType, i - 1));

				lstrcpy(DirInfo_Path(g_pDirInfo, nType, pItem->iItem), _Unicode(pItem->pszText));

				DirInfo_SetModified(g_pDirInfo, nType, TRUE);
				DirInfo_NumDir(g_pDirInfo, nType)++;
			}
			else
				DirInfo_SetDir(g_pDirInfo, nType, pItem->iItem, _Unicode(pItem->pszText));
		}
		else
		{
			DirInfo_SetDir(g_pDirInfo, nType, pItem->iItem, _Unicode(pItem->pszText));
		}

		UpdateDirectoryList(hDlg);
		ListView_SetItemState(GetDlgItem(hDlg, IDC_DIR_LIST), pItem->iItem,
							  LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	}

	return bResult;
}

static BOOL Directories_OnBeginLabelEditW(HWND hDlg, NMHDR* pNMHDR)
{
	int 		  nType;
	BOOL		  bResult = FALSE;
	NMLVDISPINFOW    *pDispInfo = (NMLVDISPINFOW*)pNMHDR;
	LVITEMW          *pItem = &pDispInfo->item;

	nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
	if (IsMultiDir(nType))
	{
		/* Last item is placeholder for append */
		if (pItem->iItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_DIR_LIST)) - 1)
		{
			HWND hEdit;

			if (MAX_DIRS <= DirInfo_NumDir(g_pDirInfo, nType))
				return TRUE; /* don't edit */

			hEdit = ListView_GetEditControl(GetDlgItem(hDlg, IDC_DIR_LIST));
			Edit_SetText(hEdit, TEXT(""));
		}
	}

	return bResult;
}

static BOOL Directories_OnEndLabelEditW(HWND hDlg, NMHDR* pNMHDR)
{
	BOOL		  bResult = FALSE;
	NMLVDISPINFOW    *pDispInfo = (NMLVDISPINFOW*)pNMHDR;
	LVITEMW          *pItem = &pDispInfo->item;

	if (pItem->pszText != NULL)
	{
		struct _stat file_stat;

		/* Don't allow empty entries. */
		if (!lstrcmp(pItem->pszText, TEXT("")))
		{
			return FALSE;
		}

		/* Check validity of edited directory. */
		if (_wstat(pItem->pszText, &file_stat) == 0
		&&	(file_stat.st_mode & S_IFDIR))
		{
			bResult = TRUE;
		}
		else
		{
			if (MessageBox(NULL, _UIW(TEXT("Directory does not exist, continue anyway?")), TEXT_MAME32NAME, MB_OKCANCEL) == IDOK)
				bResult = TRUE;
		}
	}

	if (bResult == TRUE)
	{
		int nType;
		int i;

		nType = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DIR_COMBO));
		if (IsMultiDir(nType))
		{
			/* Last item is placeholder for append */
			if (pItem->iItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_DIR_LIST)) - 1)
			{
				if (MAX_DIRS <= DirInfo_NumDir(g_pDirInfo, nType))
					return FALSE;

				for (i = DirInfo_NumDir(g_pDirInfo, nType); pItem->iItem < i; i--)
					lstrcpy(DirInfo_Path(g_pDirInfo, nType, i),
						   DirInfo_Path(g_pDirInfo, nType, i - 1));

				lstrcpy(DirInfo_Path(g_pDirInfo, nType, pItem->iItem), pItem->pszText);

				DirInfo_SetModified(g_pDirInfo, nType, TRUE);
				DirInfo_NumDir(g_pDirInfo, nType)++;
			}
			else
				DirInfo_SetDir(g_pDirInfo, nType, pItem->iItem, pItem->pszText);
		}
		else
		{
			DirInfo_SetDir(g_pDirInfo, nType, pItem->iItem, pItem->pszText);
		}

		UpdateDirectoryList(hDlg);
		ListView_SetItemState(GetDlgItem(hDlg, IDC_DIR_LIST), pItem->iItem,
							  LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	}

	return bResult;
}

static void Directories_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDOK:
		if (codeNotify == BN_CLICKED)
			Directories_OnOk(hDlg);
		break;

	case IDCANCEL:
		if (codeNotify == BN_CLICKED)
			Directories_OnCancel(hDlg);
		break;

	case IDC_DIR_BROWSE:
		if (codeNotify == BN_CLICKED)
			Directories_OnBrowse(hDlg);
		break;

	case IDC_DIR_INSERT:
		if (codeNotify == BN_CLICKED)
			Directories_OnInsert(hDlg);
		break;

	case IDC_DIR_DELETE:
		if (codeNotify == BN_CLICKED)
			Directories_OnDelete(hDlg);
		break;

	case IDC_DIR_COMBO:
		switch (codeNotify)
		{
		case CBN_SELCHANGE:
			Directories_OnSelChange(hDlg);
			break;
		}
		break;
	}
}

static BOOL Directories_OnNotify(HWND hDlg, int id, NMHDR* pNMHDR)
{
	switch (id)
	{
	case IDC_DIR_LIST:
		switch (pNMHDR->code)
		{
		case LVN_ENDLABELEDITA:
			return Directories_OnEndLabelEditA(hDlg, pNMHDR);

		case LVN_ENDLABELEDITW:
			return Directories_OnEndLabelEditW(hDlg, pNMHDR);

		case LVN_BEGINLABELEDITA:
			return Directories_OnBeginLabelEditA(hDlg, pNMHDR);

		case LVN_BEGINLABELEDITW:
			return Directories_OnBeginLabelEditW(hDlg, pNMHDR);
		}
	}
	return FALSE;
}

/**************************************************************************

	Use the shell to select a Directory.

 **************************************************************************/

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	/*
		Called just after the dialog is initialized
		Select the dir passed in BROWSEINFO.lParam
	*/
	if (uMsg == BFFM_INITIALIZED)
	{
		if ((const char*)lpData != NULL)
		{
			if (!OnNT())
				SendMessageA(hwnd, BFFM_SETSELECTIONA, TRUE, lpData);
			else
				SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, lpData);
		}
	}

	return 0;
}

static BOOL BrowseForDirectoryA(HWND hwnd, const WCHAR* pStartDir, WCHAR* pResult) 
{
	BOOL		bResult = FALSE;
	IMalloc*	piMalloc = 0;
	BROWSEINFOA	Info;
	ITEMIDLIST* pItemIDList = NULL;
	char		buf[MAX_PATH];
	char		startDir[MAX_PATH];
	
	if (!SUCCEEDED(SHGetMalloc(&piMalloc)))
		return FALSE;

	strcpy(startDir, _String(pStartDir));

	Info.hwndOwner		= hwnd;
	Info.pidlRoot		= NULL;
	Info.pszDisplayName = buf;
	Info.lpszTitle		= _String(_UIW(TEXT("Directory name:")));
	Info.ulFlags		= BIF_RETURNONLYFSDIRS;
	Info.lpfn			= BrowseCallbackProc;
	Info.lParam 		= (LPARAM)pStartDir;

	pItemIDList = SHBrowseForFolderA(&Info);

	if (pItemIDList != NULL)
	{
		if (SHGetPathFromIDListA(pItemIDList, buf) == TRUE)
		{
			wcsncpy(pResult, _Unicode(buf), MAX_PATH);
			bResult = TRUE;
		}
		IMalloc_Free(piMalloc, pItemIDList);
	}
	else
	{
		bResult = FALSE;
	}

	IMalloc_Release(piMalloc);
	return bResult;
}

static BOOL BrowseForDirectoryW(HWND hwnd, const WCHAR* pStartDir, WCHAR* pResult) 
{
	BOOL		bResult = FALSE;
	IMalloc*	piMalloc = 0;
	BROWSEINFOW	Info;
	ITEMIDLIST* pItemIDList = NULL;
	WCHAR		buf[MAX_PATH];
	WCHAR		startDir[MAX_PATH];
	
	if (!SUCCEEDED(SHGetMalloc(&piMalloc)))
		return FALSE;

	lstrcpy(startDir, pStartDir);

	Info.hwndOwner		= hwnd;
	Info.pidlRoot		= NULL;
	Info.pszDisplayName = buf;
	Info.lpszTitle		= _UIW(TEXT("Directory name:"));
	Info.ulFlags		= BIF_RETURNONLYFSDIRS;
	Info.lpfn			= BrowseCallbackProc;
	Info.lParam 		= (LPARAM)startDir;

	pItemIDList = SHBrowseForFolderW(&Info);

	if (pItemIDList != NULL)
	{
		if (SHGetPathFromIDListW(pItemIDList, buf) == TRUE)
		{
			wcsncpy(pResult, buf, MAX_PATH);
			bResult = TRUE;
		}
		IMalloc_Free(piMalloc, pItemIDList);
	}
	else
	{
		bResult = FALSE;
	}

	IMalloc_Release(piMalloc);
	return bResult;
}

BOOL BrowseForDirectory(HWND hwnd, const WCHAR* pStartDir, WCHAR* pResult)
{
	if (!OnNT())
		return BrowseForDirectoryA(hwnd, pStartDir, pResult);

	return BrowseForDirectoryW(hwnd, pStartDir, pResult);
}

/**************************************************************************/


