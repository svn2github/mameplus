/******************************************************************************

    tiny.c

    driver.c substitute file for "tiny" MAME builds.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

******************************************************************************/

#include "driver.h"

extern const game_driver TINY_NAME;

#ifndef DRIVER_SWITCH
const game_driver * const drivers[] =
#else /* DRIVER_SWITCH */
const game_driver ** drivers = NULL;
const game_driver * const mamedrivers[] =
#endif /* DRIVER_SWITCH */
{
	TINY_POINTER,
	0	/* end of array */
};
