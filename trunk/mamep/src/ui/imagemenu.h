//******************************************
// Author:          Massimo Galbusera
// Email:           kkezmg@gmail.com
// Website:         http://www.winapizone.net
// File:            ImageMenu.h
// Version:         1.0
// Date:            7 February 2006
// Compiled on:     MinGW
// Compatible with: Window 98, ME, 2000, XP and 2003
//
// You are free to use/modify this code but leave this header intact.
// This file is public domain so you are free to use it any of
// your applications (Freeware, Shareware, Commercial). All I ask is
// that you let me know you're using this extension, so i can
// add a link to your program to my website.
//******************************************

#define IMIF_LOADFROMFILE               1
#define IMIF_LOADFROMRES                2
#define IMIF_ICON                       4
#define IMIF_BITMAP                     8
#define IMIF_NOIMAGE                    16

#define MPF_TITLE                       1
#define MPF_VERTICALTITLE               2
#define MPF_BKGND                       4
#define MPF_CUSTOMBKCOLOR               8
#define MPF_HORZGRADIENT                16
#define MPF_VERTGRADIENT                32

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif  /* __cplusplus */ 
#endif  /* ! EXTERN_C */

#if (defined __cplusplus)
typedef std::basic_string<TCHAR> tstring;
#endif

typedef struct tagMENUPROPS MENUPROPS;
struct tagMENUPROPS
{
    HMENU menuHandle;
    DWORD flags;
    //Title text props
#if (defined __cplusplus)
    tstring
#else
    LPTSTR
#endif
 	menuTitle;
    COLORREF textColor;
    //Title background colors
    COLORREF firstColor;
    COLORREF secondColor;
};

typedef struct tagIMITEM IMITEM;
struct tagIMITEM
{
    UINT mask;
    UINT itemID;
    LPTSTR imageStr;
    HINSTANCE hInst;
    HICON normalIcon;
    HBITMAP normalBitmap;
};

enum menuStyle
{
	MENU_STYLE_BASIC = 0,
	MENU_STYLE_GRAY,
	MENU_STYLE_OFFICE,
	MENU_STYLE_OFFICE2003,
	MENU_STYLE_OFFICE2007,
	MENU_STYLE_MAX
};

EXTERN_C BOOL ImageMenu_Create(HWND hwnd, HMENU hMenu, BOOL isMenuBar);
EXTERN_C BOOL ImageMenu_CreatePopup(HWND hwnd, HMENU hMenu);
EXTERN_C void ImageMenu_Remove(HMENU menu, int);

EXTERN_C BOOL ImageMenu_SetItemImage(IMITEM* imi);
EXTERN_C void ImageMenu_SetStyle(HWND hwnd, int newMenuStyle);

EXTERN_C void ImageMenu_SetMenuProps(MENUPROPS *mp);
EXTERN_C void ImageMenu_SetMenuTitleProps(HMENU menuHandle, LPTSTR title, BOOL isVerticalTitle, COLORREF textColor);
EXTERN_C void ImageMenu_SetMenuTitleBkProps(HMENU menuHandle, COLORREF firstColor, COLORREF secondColor, BOOL isGradient, BOOL isVerticalGradient);

#define ImageMenu_Supported()	OnNT()
