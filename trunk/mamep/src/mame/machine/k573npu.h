// license:MAME
// copyright-holders:smf
/*
 * Konami 573 Network PCB Unit
 *
 */

#pragma once

#ifndef __K573NPU_H__
#define __K573NPU_H__

#include "emu.h"

extern const device_type KONAMI_573_NETWORK_PCB_UNIT;

class k573npu_device : public device_t
{
public:
	k573npu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start();

	virtual const rom_entry *device_rom_region() const;
};

#endif
