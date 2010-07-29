/***************************************************************************

    ips.h

    International Patching System.

    This is an unofficial version based on MAME.
    Please do not send any reports from this build to the MAME team.

***************************************************************************/

#ifndef IPS_H
#define IPS_H

int open_ips_entry(running_machine *machine, const char *patch_name, rom_load_data *romdata, const rom_entry *romp);
int close_ips_entry(rom_load_data *romdata);
void *assign_ips_patch(const rom_entry *romp);
void apply_ips_patch(void *patch, UINT8 *buffer, int length);

#endif
