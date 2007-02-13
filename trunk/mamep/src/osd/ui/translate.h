int      InitTranslator(int langcode);
HFONT    TranslateCreateFont(const LOGFONTA*);
void     TranslateMenu(HMENU hMenu, int uiString);
char    *TranslateMenuHelp(HMENU hMenu, UINT nIndex, int popup);
void     TranslateControl(HWND hControl);
void     TranslateDialog(HWND hDlg, LPARAM lParam, BOOL change_font);
void     TranslateTreeFolders(HWND hWnd);
void     GetTranslatedFont(LOGFONTA *logfont);
LRESULT  StatusBarSetTextA(HWND,WPARAM,LPCSTR);
LRESULT  StatusBarSetTextW(HWND,WPARAM,LPCWSTR);

char   *_String(const LPWSTR ws);
LPWSTR  _Unicode(const char *s);
LPWSTR  _UTF8Unicode(const char *s);

void ListView_GetItemTextA(HWND hwndCtl, int nIndex, int isitem, LPSTR lpch, int cchMax);

int ComboBox_AddStringA(HWND hwndCtl, LPCSTR lpsz);
int ComboBox_InsertStringA(HWND hwndCtl, int nIndex, LPCSTR lpsz);
int ComboBox_FindStringA(HWND hwndCtl, int indexStart, LPCSTR lpszFind);
int ComboBox_GetLBTextA(HWND hwndCtl, int nIndex, LPSTR lpszBuffer);
int ComboBox_GetLBTextLenA(HWND hwndCtl, int nIndex);
int ComboBox_GetTextA(HWND hwndCtl, LPSTR lpch, int cchMax);
BOOL ComboBox_SetTextA(HWND hwndCtl, LPCSTR lpsz);

#define ComboBox_AddStringW(hwndCtl,lpsz)                       ((int)(DWORD)        SendMessageW(        (hwndCtl), CB_ADDSTRING,     0,                         (LPARAM)(LPWSTR)(lpsz)))
#define ComboBox_InsertStringW(hwndCtl,index,lpsz)              ((int)(DWORD)        SendMessageW(        (hwndCtl), CB_INSERTSTRING,  (WPARAM)(int)(index),      (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBox_FindStringW(hwndCtl,indexStart,lpszFind)       ((int)(DWORD)        SendMessageW(        (hwndCtl), CB_FINDSTRING,    (WPARAM)(int)(indexStart), (LPARAM)(LPWSTR)(lpszFind)))
#define ComboBox_GetLBTextW(hwndCtl,index,lpszBuffer)           ((int)(DWORD)        SendMessageW(        (hwndCtl), CB_GETLBTEXT,     (WPARAM)(int)(index),      (LPARAM)(LPWSTR)(lpszBuffer)))
#define ComboBox_GetLBTextLenW(hwndCtl,index)                   ((int)(DWORD)        SendMessageW(        (hwndCtl), CB_GETLBTEXTLEN,  (WPARAM)(int)(index),      0))

/* No internal conversion */
#define Edit_GetTextA(hwndCtl,lpch,cchMax)     GetWindowTextA((hwndCtl),(lpch),(cchMax))
#define Edit_SetTextA(hwndCtl,lpsz)            SetWindowTextA((hwndCtl),(lpsz))
#define Edit_GetTextLengthA(hwndCtl)           GetWindowTextLengthA(hwndCtl)
#define Edit_SetSelA(hwndCtl,ichStart,ichEnd)  ((void)SendMessageA((hwndCtl),EM_SETSEL,(ichStart),(ichEnd)))
#define Static_GetTextA(hwndCtl,lpch,cchMax)   GetWindowTextA((hwndCtl),(lpch),(cchMax))
#define Static_SetTextA(hwndCtl,lpsz)          SetWindowTextA((hwndCtl),(lpsz))

#undef GetModuleHandle
#define GetModuleHandle GetModuleHandleA
