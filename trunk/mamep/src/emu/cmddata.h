/***************************************************************************

    cmddata.h

    Command List datafile engine

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

****************************************************************************

    Token parsing by Neil Bradley
    Modifications and higher-level functions by John Butler

***************************************************************************/

#pragma once

#ifndef __CMDDATA_H__
#define __CMDDATA_H__

extern void datafile_init(running_machine &machine, emu_options *options);
extern void datafile_exit(void);

#ifdef CMD_LIST
extern int load_driver_command_ex(const game_driver *drv, char *buffer, int bufsize, const int menu_sel);
extern UINT8 command_sub_menu(const game_driver *drv, const char *menu_item[]);
#endif /* CMD_LIST */

#endif	/* __CMDDATA_H__ */
