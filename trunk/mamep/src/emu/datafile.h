/***************************************************************************

    datafile.h

    Controls execution of the core MAME system.

***************************************************************************/

#pragma once

#ifndef __DATAFILE_H__
#define __DATAFILE_H__

extern int load_driver_history(const game_driver *drv, char *buffer, int bufsize);
#ifdef STORY_DATAFILE
extern int load_driver_story(const game_driver *drv, char *buffer, int bufsize);
#endif /* STORY_DATAFILE */
extern int load_driver_mameinfo(const game_driver *drv, char *buffer, int bufsize);
extern int load_driver_drivinfo(const game_driver *drv, char *buffer, int bufsize);
extern int load_driver_statistics(char *buffer, int bufsize);

#ifdef CMD_LIST
extern int load_driver_command_ex(const game_driver *drv, char *buffer, int bufsize, const int menu_sel);
extern UINT8 command_sub_menu(const game_driver *drv, const char *menu_item[]);
#endif /* CMD_LIST */

#endif	/* __DATAFILE_H__ */
