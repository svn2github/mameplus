/***************************************************************************

    ips.c

    International Patching System.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef IPS_H
#define IPS_H

int open_ips_entry(const char *patch_name, rom_load_data *romdata, const rom_entry *romp);
int close_ips_entry(rom_load_data *romdata);
void *assign_ips_patch(const rom_entry *romp);
void apply_ips_patch(void *patch, UINT8 *buffer, int length);

#endif
