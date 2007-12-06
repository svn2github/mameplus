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
#include "driver.h"
#include "winuiopt.h"
#include "optionsms.h"
#include "emuopts.h"
#include "messopts.h"

#include "osd/windows/configms.h"
#include "winmain.h"

#include "strconv.h"
#include "opthndlr.h"


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
	{ WINGUIOPTION_SOFTWARE_COLUMN_WIDTHS,	"186, 230, 88, 84, 84, 68, 248, 248",	0,				NULL },
	{ WINGUIOPTION_SOFTWARE_COLUMN_ORDER,	"0,   1,    2,  3,  4,  5,   6,   7",	0,				NULL },
	{ WINGUIOPTION_SOFTWARE_COLUMN_SHOWN,	"1,   1,    1,  1,  1,  0,   0,   0",	0,				NULL },

	{ WINGUIOPTION_SOFTWARE_SORT_COLUMN,	"0",									0,				NULL },
	{ WINGUIOPTION_SOFTWARE_SORT_REVERSED,	"0",									OPTION_BOOLEAN,	NULL },

	{ WINGUIOPTION_SOFTWARE_TAB,			"0",									0,				NULL },
	{ WINGUIOPTION_SOFTWAREPATH,			"software",								0,				NULL },
	{ NULL }
};

static options_entry mess_driver_flag_opts[] =
{
	{ NULL, "", 0, NULL },
	{ NULL }
};

void MessSetupSettings(core_options *settings)
{
	options_add_entries(settings, mess_wingui_settings);
}

void MessSetupGameOptions(core_options *opt, int driver_index)
{
	static const options_entry this_entry[] =
	{
		{ OPTION_ADDED_DEVICE_OPTIONS,	"0",	OPTION_BOOLEAN | OPTION_INTERNAL,	"device-specific options have been added" },
		{ NULL }
	};

	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	options_add_entries(opt, this_entry);
	mess_add_device_options(opt, drivers[driver_index]);
}

void MessWriteGameOptions(void *p, core_file *inifile)
{
	options_output_ini_file(p, inifile);
}

void MessSetupGameVariables(core_options *settings, int driver_index)
{
	char buf[64];

	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	snprintf(buf, ARRAY_LENGTH(buf), "%s_extra_software", drivers[driver_index]->name);
	mess_driver_flag_opts[0].name = buf;

	options_add_entries(settings, mess_driver_flag_opts);
}

void SetMessColumnOrder(int order[])
{
	options_set_csv_int(get_winui_options(), WINGUIOPTION_SOFTWARE_COLUMN_ORDER, order, MESS_COLUMN_MAX, OPTION_PRIORITY_INI);
}

void GetMessColumnOrder(int order[])
{
	_options_get_csv_int(get_winui_options(), order, MESS_COLUMN_MAX, WINGUIOPTION_SOFTWARE_COLUMN_ORDER);
}

void SetMessColumnShown(int shown[])
{
	options_set_csv_int(get_winui_options(), WINGUIOPTION_SOFTWARE_COLUMN_SHOWN, shown, MESS_COLUMN_MAX, OPTION_PRIORITY_INI);
}

void GetMessColumnShown(int shown[])
{
	_options_get_csv_int(get_winui_options(), shown, MESS_COLUMN_MAX, WINGUIOPTION_SOFTWARE_COLUMN_SHOWN);
}

void SetMessColumnWidths(int width[])
{
	options_set_csv_int(get_winui_options(), WINGUIOPTION_SOFTWARE_COLUMN_WIDTHS, width, MESS_COLUMN_MAX, OPTION_PRIORITY_INI);
}

void GetMessColumnWidths(int width[])
{
	_options_get_csv_int(get_winui_options(), width, MESS_COLUMN_MAX, WINGUIOPTION_SOFTWARE_COLUMN_WIDTHS);
}

void SetMessSortColumn(int column)
{
	options_set_int(get_winui_options(), WINGUIOPTION_SOFTWARE_SORT_COLUMN, column, OPTION_PRIORITY_INI);
}

int GetMessSortColumn(void)
{
	return options_get_int(get_winui_options(), WINGUIOPTION_SOFTWARE_SORT_COLUMN);
}

void SetMessSortReverse(BOOL reverse)
{
	options_set_bool(get_winui_options(), WINGUIOPTION_SOFTWARE_SORT_REVERSED, reverse, OPTION_PRIORITY_INI);
}

BOOL GetMessSortReverse(void)
{
	return options_get_bool(get_winui_options(), WINGUIOPTION_SOFTWARE_SORT_REVERSED);
}

const WCHAR* GetSoftwareDirs(void)
{
	return options_get_wstring(get_winui_options(), WINGUIOPTION_SOFTWAREPATH);
}

void SetSoftwareDirs(const WCHAR* paths)
{
	options_set_wstring(get_winui_options(), WINGUIOPTION_SOFTWAREPATH, paths, OPTION_PRIORITY_INI);
}

void SetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst, const WCHAR *software)
{
	const char *opt_name = device_instancename(devclass, device_inst);
	options_type *o;
	core_options *opt;

	if (LOG_SOFTWARE)
	{
		dprintf("SetSelectedSoftware(): driver_index=%d (\'%s\') devclass=%p device_inst=%d software='%s'\n",
			driver_index, drivers[driver_index]->name, devclass, device_inst, software);
	}

	o = GetGameOptions(driver_index);
	opt = o->dynamic_opt;
	opt_name = device_instancename(devclass, device_inst);
	options_set_wstring(opt, opt_name, software, OPTION_PRIORITY_INI);
}

const WCHAR *GetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst)
{
	const char *opt_name = device_instancename(devclass, device_inst);
	const WCHAR *software;
	options_type *o;
	core_options *opt;

	o = GetGameOptions(driver_index);
	opt = o->dynamic_opt;
	opt_name = device_instancename(devclass, device_inst);
	software = options_get_wstring(opt, opt_name);
	return software ? software : L"";
}

void SetExtraSoftwarePaths(int driver_index, const WCHAR *extra_paths)
{
	char buf[64];

	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	snprintf(buf, ARRAY_LENGTH(buf), "%s_extra_software", drivers[driver_index]->name);
	options_set_wstring(get_winui_options(), buf, extra_paths, OPTION_PRIORITY_INI);
}

const WCHAR *GetExtraSoftwarePaths(int driver_index)
{
	const WCHAR *paths;
	char buf[64];

	assert(0 <= driver_index && driver_index < driver_list_get_count(drivers));

	snprintf(buf, ARRAY_LENGTH(buf), "%s_extra_software", drivers[driver_index]->name);
	paths = options_get_wstring(get_winui_options(), buf);

	return paths ? paths : L"";
}

void SetCurrentSoftwareTab(const char *shortname)
{
	options_set_string(get_winui_options(), WINGUIOPTION_SOFTWARE_TAB, shortname, OPTION_PRIORITY_INI);
}

const char *GetCurrentSoftwareTab(void)
{
	return options_get_string(get_winui_options(), WINGUIOPTION_SOFTWARE_TAB);
}
