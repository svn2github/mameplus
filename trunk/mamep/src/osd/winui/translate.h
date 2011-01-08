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

  translate.h

  This is an unofficial version based on MAMEUI.
  Please do not send any reports from this build to the MAMEUI team.

***************************************************************************/

int      InitTranslator(int langcode);
HFONT    TranslateCreateFont(const LOGFONTW *);
void     TranslateMenu(HMENU hMenu, int uiString);
WCHAR   *TranslateMenuHelp(HMENU hMenu, UINT nIndex, int popup);
void     TranslateControl(HWND hControl);
void     TranslateDialog(HWND hDlg, LPARAM lParam, BOOL change_font);
void     TranslateTreeFolders(HWND hWnd);
void     GetTranslatedFont(LOGFONTW *);
LRESULT  StatusBarSetTextA(HWND,WPARAM,LPCSTR);
LRESULT  StatusBarSetTextW(HWND,WPARAM,LPCWSTR);

char   *_String(const WCHAR *ws);
LPWSTR  _Unicode(const char *s);
LPWSTR  _UTF8Unicode(const char *s);
void FreeTranslateBuffer(void);

void ListView_GetItemTextA(HWND hwndCtl, int nIndex, int isitem, LPSTR lpch, int cchMax);

int ComboBox_AddStringA(HWND hwndCtl, LPCSTR lpsz);
int ComboBox_InsertStringA(HWND hwndCtl, int nIndex, LPCSTR lpsz);
int ComboBox_FindStringA(HWND hwndCtl, int indexStart, LPCSTR lpszFind);
int ComboBox_GetLBTextA(HWND hwndCtl, int nIndex, LPSTR lpszBuffer);
int ComboBox_GetLBTextLenA(HWND hwndCtl, int nIndex);
int ComboBox_GetTextA(HWND hwndCtl, LPSTR lpch, int cchMax);
BOOL ComboBox_SetTextA(HWND hwndCtl, LPCSTR lpsz);

int ComboBox_AddStringW(HWND hwndCtl, LPWSTR lpsz);
int ComboBox_InsertStringW(HWND hwndCtl, int index, LPWSTR lpsz);
int ComboBox_FindStringW(HWND hwndCtl, int indexStart, LPWSTR lpszFind);
int ComboBox_GetLBTextW(HWND hwndCtl, int index, LPWSTR lpszBuffer);
int ComboBox_GetLBTextLenW(HWND hwndCtl, int index);

/* No internal conversion */
#define Edit_GetTextA(hwndCtl,lpch,cchMax)     GetWindowTextA((hwndCtl),(lpch),(cchMax))
#define Edit_SetTextA(hwndCtl,lpsz)            SetWindowTextA((hwndCtl),(lpsz))
#define Edit_GetTextLengthA(hwndCtl)           GetWindowTextLengthA(hwndCtl)
#define Edit_SetSelA(hwndCtl,ichStart,ichEnd)  ((void)SendMessageA((hwndCtl),EM_SETSEL,(ichStart),(ichEnd)))
#define Static_GetTextA(hwndCtl,lpch,cchMax)   GetWindowTextA((hwndCtl),(lpch),(cchMax))
#define Static_SetTextA(hwndCtl,lpsz)          SetWindowTextA((hwndCtl),(lpsz))

#undef GetModuleHandle
#define GetModuleHandle GetModuleHandleA

FILE *wfopen(const WCHAR *, const WCHAR *);

WCHAR *w_lang_message(int msgcat, const WCHAR *str);
//char *mb_lang_message(int msgcat, const char *str);
