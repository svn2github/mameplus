#if 0
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <wingdi.h>
#include <winuser.h>
#include <tchar.h>

#include "m32util.h"
#include "winuiopt.h"
#include "optionsms.h"
#include "emuopts.h"
#include "driver.h"
#endif
#include "messopts.h"

#ifndef	MESS_OPTION_TYPE_DEFINE
#define MESS_OPTION_TYPE_DEFINE \
	int      mess_column_shown[MESS_COLUMN_MAX]; \
	int      mess_column_widths[MESS_COLUMN_MAX]; \
	int      mess_column_order[MESS_COLUMN_MAX]; \
	int      mess_sort_column; \
	BOOL     mess_sort_reversed; \
	char*    current_software_tab; \
	WCHAR*   softwarepath; \

#else /* MESS_OPTION_TYPE_DEFINE */
#include "osd/windows/configms.h"
#include "winmain.h"

#define WINGUIOPTION_SOFTWARE_COLUMN_SHOWN		"mess_column_shown"
#define WINGUIOPTION_SOFTWARE_COLUMN_WIDTHS		"mess_column_widths"
#define WINGUIOPTION_SOFTWARE_COLUMN_ORDER		"mess_column_order"
#define WINGUIOPTION_SOFTWARE_SORT_REVERSED		"mess_sort_reversed"
#define WINGUIOPTION_SOFTWARE_SORT_COLUMN		"mess_sort_column"
#define WINGUIOPTION_SOFTWARE_TAB				"current_software_tab"
#define WINGUIOPTION_SOFTWAREPATH				"softwarepath"

#define LOG_SOFTWARE	0

static const options_entry mess_wingui_settings[] =
{
	{ NULL,                          NULL,                         OPTION_HEADER,     "MESS SPECIFIC OPTIONS"},
	{ WINGUIOPTION_SOFTWARE_COLUMN_WIDTHS,	"186, 230, 88, 84, 84, 68, 248, 248",	0,		"Software column width" },
	{ WINGUIOPTION_SOFTWARE_COLUMN_ORDER,	"0,   1,    2,  3,  4,  5,   6,   7",	0,		"Software column order" },
	{ WINGUIOPTION_SOFTWARE_COLUMN_SHOWN,	"1,   1,    1,  1,  1,  0,   0,   0",	0,		"Software column shown" },

	{ WINGUIOPTION_SOFTWARE_SORT_COLUMN,	"0",					0,		"Software sort column" },
	{ WINGUIOPTION_SOFTWARE_SORT_REVERSED,	"0",					OPTION_BOOLEAN,	"Software reverse sort" },

	{ WINGUIOPTION_SOFTWARE_TAB,		"0",					0,		"Software tab" },
	{ WINGUIOPTION_SOFTWAREPATH,		"software",				0,		"Software path" },
	{ NULL }
};

void MessSetupSettings(core_options *settings)
{
	options_add_entries(settings, mess_wingui_settings);
}

void MessSetupGameOptions(core_options *opts, int driver_index)
{
	options_add_entries(opts, mess_core_options);
	options_add_entries(opts, mess_win_options);

	if (driver_index >= 0)
	{
		mess_add_device_options(opts, drivers[driver_index]);
	}
}

void SetMessColumnOrder(int order[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		settings.mess_column_order[i] = order[i];
}

void GetMessColumnOrder(int order[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		order[i] = settings.mess_column_order[i];
}

void SetMessColumnShown(int shown[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		settings.mess_column_shown[i] = shown[i];
}

void GetMessColumnShown(int shown[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		shown[i] = settings.mess_column_shown[i];
}

void SetMessColumnWidths(int width[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		settings.mess_column_widths[i] = width[i];
}

void GetMessColumnWidths(int width[])
{
	int i;

	for (i = 0; i < MESS_COLUMN_MAX; i++)
		width[i] = settings.mess_column_widths[i];
}

void SetMessSortColumn(int column)
{
	settings.mess_sort_column = column;
}

int GetMessSortColumn(void)
{
	return settings.mess_sort_column;
}

void SetMessSortReverse(BOOL reverse)
{
	settings.mess_sort_reversed = reverse;
}

BOOL GetMessSortReverse(void)
{
	return settings.mess_sort_reversed;
}

const WCHAR* GetSoftwareDirs(void)
{
	return settings.softwarepath;
}

void SetSoftwareDirs(const WCHAR* paths)
{
	FreeIfAllocatedW(&settings.softwarepath);

	if (paths != NULL)
		settings.softwarepath = wcsdup(paths);
}

const WCHAR* GetHashDirs(void)
{
	return settings.hashpath;
}

void SetHashDirs(const WCHAR* dir)
{
	FreeIfAllocatedW(&settings.hashpath);

	if (dir != NULL)
		settings.hashpath = wcsdup(dir);
}

#if 0
void SetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst, const char *software)
{
	const char *opt_name = device_instancename(devclass, device_inst);
	core_options *o;

	if (LOG_SOFTWARE)
	{
		dprintf("SetSelectedSoftware(): driver_index=%d (\'%s\') devclass=%p device_inst=%d software='%s'\n",
			driver_index, drivers[driver_index]->name, devclass, device_inst, software);
	}

	o = load_options(OPTIONS_GAME, driver_index);
	opt_name = device_instancename(devclass, device_inst);
	options_set_string(o, opt_name, software, OPTION_PRIORITY_CMDLINE);
	save_options(OPTIONS_GAME, o, driver_index);
	options_free(o);
}

const char *GetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst)
{
	const char *opt_name = device_instancename(devclass, device_inst);
	const char *software;
	core_options *o;

	o = load_options(OPTIONS_GAME, driver_index);
	opt_name = device_instancename(devclass, device_inst);
	software = options_get_string(o, opt_name);
	return software ? software : "";
}
#endif

void SetExtraSoftwarePaths(int driver_index, const char *extra_paths)
{
	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	FreeIfAllocated(&driver_variables[driver_index].extra_software);
	if (extra_paths)
		driver_variables[driver_index].extra_software = mame_strdup(extra_paths);
}

const char *GetExtraSoftwarePaths(int driver_index)
{
	const char *paths;

	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	paths = driver_variables[driver_index].extra_software;
	return paths ? paths : "";
}

void SetCurrentSoftwareTab(const char *shortname)
{
	FreeIfAllocated(&settings.current_software_tab);
	if (shortname != NULL)
		settings.current_software_tab = mame_strdup(shortname);
}

const char *GetCurrentSoftwareTab(void)
{
	return settings.current_software_tab;
}
#endif /* MESS_OPTION_TYPE_DEFINE */
