/***************************************************************************

  M.A.M.E.32  -  Multiple Arcade Machine Emulator for Win32
  Win32 Portions Copyright (C) 1997-2003 Michael Soderstrom and Chris Kirmse

  This file is part of MAME32, and may only be used, modified and
  distributed under the terms of the MAME license, in "readme.txt".
  By continuing to use, modify or distribute this file you indicate
  that you have read the license and understand and accept it fully.

 ***************************************************************************/

#ifndef PROPERTIES_H
#define PROPERTIES_H

/* Get title string to display in the top of the property page,
 * Called also in Audit32.c
 */
LPWSTR GameInfoTitle(int nIndex);

enum
{
	SOURCE_GLOBAL = 0,
	SOURCE_VECTOR,
	SOURCE_FOLDER,
	SOURCE_GAME,
	SOURCE_MAX
};

/* Called in win32ui.c to create the property page */
void    InitPropertyPage(HINSTANCE hInst, HWND hwnd, int game_num, HICON hIcon, const char *folder);

#define PROPERTIES_PAGE 0
#define AUDIT_PAGE      1   

void    InitPropertyPageToPage(HINSTANCE hInst, HWND hwnd, int game_num, HICON hIcon, int start_page, const char *folder);
void    InitDefaultPropertyPage(HINSTANCE hInst, HWND hWnd);

/* Get Help ID array for WM_HELP and WM_CONTEXTMENU */
DWORD   GetHelpIDs(void);

/* Get Game status text string */
LPWSTR GameInfoStatus(int driver_index, BOOL bRomStatus);

/* Property sheet info for layout.c */
typedef struct
{
	BOOL bOnDefaultPage;
	BOOL (*pfnFilterProc)(void);
	DWORD dwDlgID;
	DLGPROC pfnDlgProc;
} PROPERTYSHEETINFO;

/* in layout.c */
extern const PROPERTYSHEETINFO g_propSheets[];


BOOL PropSheetFilter_Vector(void);
BOOL PropSheetFilter_BIOS(void);

INT_PTR CALLBACK GamePropertiesDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GameOptionsProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

void PropertiesInit(void);

#endif
