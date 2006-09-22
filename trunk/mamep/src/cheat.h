/*********************************************************************

    cheat.h

    Cheat system.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __CHEAT_H__
#define __CHEAT_H__

void cheat_init(running_machine *machine);

UINT32 cheat_menu(UINT32 state);

void cheat_display_watches(void);

#endif	/* __CHEAT_H__ */
