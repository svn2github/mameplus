#ifndef OPTIONSMS_H
#define OPTIONSMS_H

#include "device.h"
#include "options.h"

enum
{
	MESS_COLUMN_IMAGES,
	MESS_COLUMN_GOODNAME,
	MESS_COLUMN_MANUFACTURER,
	MESS_COLUMN_YEAR,
	MESS_COLUMN_PLAYABLE,
	MESS_COLUMN_CRC,
	MESS_COLUMN_SHA1,
	MESS_COLUMN_MD5,
	MESS_COLUMN_MAX
};

void MessSetupSettings(core_options *settings);
void MessSetupGameVariables(core_options *settings, int driver_index);
void MessSetupGameOptions(core_options *opt, int driver_index);

void SetMessColumnWidths(int widths[]);
void GetMessColumnWidths(int widths[]);

void SetMessColumnOrder(int order[]);
void GetMessColumnOrder(int order[]);

void SetMessColumnShown(int shown[]);
void GetMessColumnShown(int shown[]);

void SetMessSortColumn(int column);
int  GetMessSortColumn(void);

void SetMessSortReverse(BOOL reverse);
BOOL GetMessSortReverse(void);

const WCHAR* GetSoftwareDirs(void);
void  SetSoftwareDirs(const WCHAR* paths);

void SetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst, const WCHAR *software);
const WCHAR *GetSelectedSoftware(int driver_index, const device_class *devclass, int device_inst);

void SetExtraSoftwarePaths(int driver_index, const WCHAR *extra_paths);
const WCHAR *GetExtraSoftwarePaths(int driver_index);

void SetCurrentSoftwareTab(const char *shortname);
const char *GetCurrentSoftwareTab(void);

#endif /* OPTIONSMS_H */

