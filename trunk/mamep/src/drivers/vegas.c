/*************************************************************************

    Driver for Atari/Midway Vegas hardware games

    driver by Aaron Giles

    Games supported:
        * Gauntlet Legends [Atari, 200MHz, 8MB RAM, 2-TMU * 4MB]
        * Gauntlet Dark Legacy [Atari, 200MHz]
        * War: Final Assault [Midway, 250MHz, 2-TMU]
        * NBA on NBC
        * Tenth Degree/ Juko Threat
        * NBA Showtime Gold + NFL Blitz 2000 Gold

    Durango PCB (uses an RM7000 or RM5271 @ 250MHz):
        * Road Burners [Atari, 250MHz, 32MB RAM, 2-TMU * 4MB]
        * San Francisco Rush 2049 [Atari, RM7000 @ 250MHz, 32MB RAM, Voodoo3 w/16MB]
        * San Francisco Rush 2049 Tournament Edition (PIC ID = 348)
        * CART Fury

    Known bugs:
        * not working yet

***************************************************************************

    Interrupt summary:

                        __________
    UART clear-to-send |          |
    -------(0x2000)--->|          |
                       |          |
    UART data ready    |          |
    -------(0x1000)--->|          |                     __________
                       |          |   VSYNC            |          |
    Main-to-sound empty|  IOASIC  |   -------(0x20)--->|          |
    -------(0x0080)--->|          |                    |          |
                       |          |   Ethernet         |          |   SIO Summary
    Sound-to-main full |          |   -------(0x10)--->|   SIO    |--------------+
    -------(0x0040)--->|          |                    |          |              |
                       |          |                    |          |              |
    Sound FIFO empty   |          |   IOASIC Summary   |          |              |
    -------(0x0008)--->|          |----------(0x04)--->|          |              |
                       |__________|                    |__________|              |
                                                                                 |
 +-------------------------------------------------------------------------------+
 |
 |                      __________                      __________
 |  IDE Controller     |          |                    |          |
 |  -------(0x0800)--->|          |                    |          |
 |                     |          |----------(IRQ5)--->|          |
 |  SIO Summary        |          |                    |          |
 +---------(0x0400)--->|          |----------(IRQ4)--->|          |
                       |          |                    |          |
    Timer 2            |          |----------(IRQ3)--->|   CPU    |
    -------(0x0040)--->|   NILE   |                    |          |
                       |          |----------(IRQ2)--->|          |
    Timer 3            |          |                    |          |
    -------(0x0020)--->|          |----------(IRQ1)--->|          |
                       |          |                    |          |
    UART Transmit      |          |----------(IRQ0)--->|          |
    -------(0x0010)--->|          |                    |__________|
                       |__________|

**************************************************************************/

#include "driver.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "sndhrdw/dcs.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "machine/smc91c9x.h"
#include "vidhrdw/voodoo.h"
#include <time.h>


/*************************************
 *
 *  Debugging constants
 *
 *************************************/

#define LOG_NILE			(0)
#define LOG_NILE_IRQS		(0)
#define LOG_PCI				(0)
#define LOG_TIMERS			(0)
#define LOG_TIMEKEEPER		(0)
#define LOG_SIO				(0)
#define LOG_DYNAMIC			(1)
#define PRINTF_SERIAL		(0)



/*************************************
 *
 *  Core constants
 *
 *************************************/

#define SYSTEM_CLOCK		100000000
#define TIMER_CLOCK			TIME_IN_HZ(SYSTEM_CLOCK)

#define MAX_DYNAMIC_ADDRESSES	32



/*************************************
 *
 *  NILE constants
 *
 *************************************/

#define DMA_SECS_PER_BYTE	(TIME_IN_HZ(100000000))

/* NILE 4 registers 0x000-0x0ff */
#define NREG_SDRAM0			(0x000/4)
#define NREG_SDRAM1			(0x008/4)
#define NREG_DCS2			(0x010/4)	/* SIO misc */
#define NREG_DCS3			(0x018/4)	/* ADC */
#define NREG_DCS4			(0x020/4)	/* CMOS */
#define NREG_DCS5			(0x028/4)	/* SIO */
#define NREG_DCS6			(0x030/4)	/* IOASIC */
#define NREG_DCS7			(0x038/4)	/* ethernet */
#define NREG_DCS8			(0x040/4)
#define NREG_PCIW0			(0x060/4)
#define NREG_PCIW1			(0x068/4)
#define NREG_INTCS			(0x070/4)
#define NREG_BOOTCS			(0x078/4)
#define NREG_CPUSTAT		(0x080/4)
#define NREG_INTCTRL		(0x088/4)
#define NREG_INTSTAT0  		(0x090/4)
#define NREG_INTSTAT1		(0x098/4)
#define NREG_INTCLR			(0x0A0/4)
#define NREG_INTPPES		(0x0A8/4)
#define NREG_PCIERR			(0x0B8/4)
#define NREG_MEMCTRL		(0x0C0/4)
#define NREG_ACSTIME		(0x0C8/4)
#define NREG_CHKERR			(0x0D0/4)
#define NREG_PCICTRL		(0x0E0/4)
#define NREG_PCIARB			(0x0E8/4)
#define NREG_PCIINIT0		(0x0F0/4)
#define NREG_PCIINIT1		(0x0F8/4)

/* NILE 4 registers 0x100-0x1ff */
#define NREG_LCNFG			(0x100/4)
#define NREG_LCST2			(0x110/4)
#define NREG_LCST3			(0x118/4)
#define NREG_LCST4			(0x120/4)
#define NREG_LCST5			(0x128/4)
#define NREG_LCST6			(0x130/4)
#define NREG_LCST7			(0x138/4)
#define NREG_LCST8			(0x140/4)
#define NREG_DCSFN			(0x150/4)
#define NREG_DCSIO			(0x158/4)
#define NREG_BCST			(0x178/4)
#define NREG_DMACTRL0		(0x180/4)
#define NREG_DMASRCA0		(0x188/4)
#define NREG_DMADESA0		(0x190/4)
#define NREG_DMACTRL1		(0x198/4)
#define NREG_DMASRCA1		(0x1A0/4)
#define NREG_DMADESA1		(0x1A8/4)
#define NREG_T0CTRL			(0x1C0/4)
#define NREG_T0CNTR			(0x1C8/4)
#define NREG_T1CTRL			(0x1D0/4)
#define NREG_T1CNTR			(0x1D8/4)
#define NREG_T2CTRL			(0x1E0/4)
#define NREG_T2CNTR			(0x1E8/4)
#define NREG_T3CTRL			(0x1F0/4)
#define NREG_T3CNTR			(0x1F8/4)

/* NILE 4 registers 0x200-0x2ff */
#define NREG_VID			(0x200/4)
#define NREG_DID			(0x202/4)
#define NREG_PCICMD			(0x204/4)
#define NREG_PCISTS			(0x206/4)
#define NREG_REVID			(0x208/4)
#define NREG_CLASS			(0x209/4)
#define NREG_CLSIZ			(0x20C/4)
#define NREG_MLTIM			(0x20D/4)
#define NREG_HTYPE			(0x20E/4)
#define NREG_BIST			(0x20F/4)
#define NREG_BARC			(0x210/4)
#define NREG_BAR0			(0x218/4)
#define NREG_BAR1			(0x220/4)
#define NREG_CIS			(0x228/4)
#define NREG_SSVID			(0x22C/4)
#define NREG_SSID			(0x22E/4)
#define NREG_ROM			(0x230/4)
#define NREG_INTLIN			(0x23C/4)
#define NREG_INTPIN			(0x23D/4)
#define NREG_MINGNT			(0x23E/4)
#define NREG_MAXLAT			(0x23F/4)
#define NREG_BAR2			(0x240/4)
#define NREG_BAR3			(0x248/4)
#define NREG_BAR4			(0x250/4)
#define NREG_BAR5			(0x258/4)
#define NREG_BAR6			(0x260/4)
#define NREG_BAR7			(0x268/4)
#define NREG_BAR8			(0x270/4)
#define NREG_BARB			(0x278/4)

/* NILE 4 registers 0x300-0x3ff */
#define NREG_UARTRBR		(0x300/4)
#define NREG_UARTTHR		(0x300/4)
#define NREG_UARTIER		(0x308/4)
#define NREG_UARTDLL		(0x300/4)
#define NREG_UARTDLM		(0x308/4)
#define NREG_UARTIIR		(0x310/4)
#define NREG_UARTFCR		(0x310/4)
#define NREG_UARTLCR		(0x318/4)
#define NREG_UARTMCR		(0x320/4)
#define NREG_UARTLSR		(0x328/4)
#define NREG_UARTMSR		(0x330/4)
#define NREG_UARTSCR		(0x338/4)

/* NILE 4 interrupts */
#define NINT_CPCE			(0)
#define NINT_CNTD			(1)
#define NINT_MCE			(2)
#define NINT_DMA			(3)
#define NINT_UART			(4)
#define NINT_WDOG			(5)
#define NINT_GPT			(6)
#define NINT_LBRTD			(7)
#define NINT_INTA			(8)
#define NINT_INTB			(9)
#define NINT_INTC			(10)
#define NINT_INTD			(11)
#define NINT_INTE			(12)
#define NINT_RESV			(13)
#define NINT_PCIS			(14)
#define NINT_PCIE			(15)



/*************************************
 *
 *  Local variables
 *
 *************************************/

static UINT32 *rambase, *rombase;
static size_t ramsize;

static UINT32 *nile_regs;
static UINT16 nile_irq_state;
static UINT16 ide_irq_state;

static UINT32 pci_bridge_regs[0x40];
static UINT32 pci_ide_regs[0x40];
static UINT32 pci_3dfx_regs[0x40];

static void *timer[4];

static UINT8 vblank_state;

static UINT8 sio_data[4];
static UINT8 sio_irq_clear;
static UINT8 sio_irq_enable;
static UINT8 sio_irq_state;
static UINT8 sio_led_state;

static UINT8 pending_analog_read;

static UINT8 cmos_unlocked;

static UINT32 *timekeeper_nvram;
static size_t timekeeper_nvram_size;

static UINT8 voodoo_type;
static UINT8 has_dcs3;

static int dynamic_count;
static struct dynamic_address
{
	offs_t			start;
	offs_t			end;
	read32_handler	read;
	write32_handler	write;
	const char *	rdname;
	const char *	wrname;
} dynamic[MAX_DYNAMIC_ADDRESSES];




/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void timer_callback(int param);
static void ide_interrupt(int state);
static void remap_dynamic_addresses(void);



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_INIT( vegas )
{
	/* set the fastest DRC options, but strict verification */
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_DRC_OPTIONS, MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY + MIPS3DRC_FLUSH_PC);

	/* configure fast RAM regions for DRC */
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_SELECT, 0);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_START, 0x00000000);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_END, ramsize - 1);
	cpunum_set_info_ptr(0, CPUINFO_PTR_MIPS3_FASTRAM_BASE, rambase);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_READONLY, 0);

	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_SELECT, 1);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_START, 0x1fc00000);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_END, 0x1fc7ffff);
	cpunum_set_info_ptr(0, CPUINFO_PTR_MIPS3_FASTRAM_BASE, rombase);
	cpunum_set_info_int(0, CPUINFO_INT_MIPS3_FASTRAM_READONLY, 1);

	/* allocate timers for the NILE */
	timer[0] = timer_alloc(NULL);
	timer[1] = timer_alloc(NULL);
	timer[2] = timer_alloc(timer_callback);
	timer[3] = timer_alloc(timer_callback);

	/* reset dynamic addressing */
	memset(nile_regs, 0, 0x1000);
	memset(pci_ide_regs, 0, sizeof(pci_ide_regs));
	memset(pci_3dfx_regs, 0, sizeof(pci_3dfx_regs));

	/* reset the DCS system if we have one */
	if (mame_find_cpu_index("dcs2") != -1 || mame_find_cpu_index("dcs3") != -1)
	{
		dcs_reset_w(1);
		dcs_reset_w(0);
	}

	/* reset subsystems */
	ide_controller_reset(0);
	voodoo_reset();
	smc91c94_reset();

	/* initialize IRQ states */
	ide_irq_state = 0;
	nile_irq_state = 0;
	sio_irq_state = 0;

	/* find out what type of voodoo we have */
	voodoo_type = voodoo_get_type();
	has_dcs3 = (mame_find_cpu_index("dcs3") != -1);
}



/*************************************
 *
 *  Timekeeper access
 *
 *************************************/

static WRITE32_HANDLER( cmos_unlock_w )
{
	cmos_unlocked = 1;
}


static WRITE32_HANDLER( timekeeper_w )
{
	if (cmos_unlocked)
	{
		COMBINE_DATA(&timekeeper_nvram[offset]);
		if (offset*4 >= 0x7ff0)
			if (LOG_TIMEKEEPER) logerror("timekeeper_w(%04X & %08X) = %08X\n", offset*4, ~mem_mask, data);
		cmos_unlocked = 0;
	}
	else
		logerror("%08X:timekeeper_w(%04X,%08X & %08X) without CMOS unlocked\n", activecpu_get_pc(), offset, data, ~mem_mask);
}


INLINE UINT8 make_bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}


static READ32_HANDLER( timekeeper_r )
{
	UINT32 result = timekeeper_nvram[offset];

	/* upper bytes are a realtime clock */
	if ((offset*4) >= 0x7ff0)
	{
		/* get the time */
		struct tm *exptime;
		time_t curtime;
		time(&curtime);
		exptime = localtime(&curtime);

		/* return portions thereof */
		switch (offset*4)
		{
			case 0x7ff0:
				result &= 0x00ff0000;
				result |= make_bcd((1900 + exptime->tm_year) / 100) << 8;
				break;
			case 0x7ff4:
				break;
			case 0x7ff8:
				result &= 0x000000ff;
				result |= make_bcd(exptime->tm_sec) << 8;
				result |= make_bcd(exptime->tm_min) << 16;
				result |= make_bcd(exptime->tm_hour) << 24;
				break;
			case 0x7ffc:
				result = exptime->tm_wday + 1;
				result |= 0x40;		/* frequency test */
				result |= make_bcd(exptime->tm_mday) << 8;
				result |= make_bcd(exptime->tm_mon + 1) << 16;
				result |= make_bcd(exptime->tm_year % 100) << 24;
				break;
		}
	}
	return result;
}


static NVRAM_HANDLER( timekeeper_save )
{
	if (read_or_write)
		mame_fwrite(file, timekeeper_nvram, timekeeper_nvram_size);
	else if (file)
		mame_fread(file, timekeeper_nvram, timekeeper_nvram_size);
	else
		memset(timekeeper_nvram, 0xff, timekeeper_nvram_size);
}



/*************************************
 *
 *  PCI bridge accesses
 *
 *************************************/

static READ32_HANDLER( pci_bridge_r )
{
	UINT32 result = pci_bridge_regs[offset];

	switch (offset)
	{
		case 0x00:		/* ID register: 0x005a = VRC 5074, 0x1033 = NEC */
			result = 0x005a1033;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI bridge read: reg %d = %08X\n", activecpu_get_pc(), offset, result);
	return result;
}


static WRITE32_HANDLER( pci_bridge_w )
{
	pci_bridge_regs[offset] = data;
	if (LOG_PCI)
		logerror("%06X:PCI bridge write: reg %d = %08X\n", activecpu_get_pc(), offset, data);
}



/*************************************
 *
 *  PCI IDE accesses
 *
 *************************************/

static READ32_HANDLER( pci_ide_r )
{
	UINT32 result = pci_ide_regs[offset];

	switch (offset)
	{
		case 0x00:		/* ID register: 0x0646 = 646 EIDE controller, 0x1095 = CMD */
			result = 0x06461095;
			break;

		case 0x14:		/* interrupt pending */
			result &= 0xffffff00;
			if (ide_irq_state)
				result |= 4;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI IDE read: reg %d = %08X\n", activecpu_get_pc(), offset, result);
	return result;
}


static WRITE32_HANDLER( pci_ide_w )
{
	pci_ide_regs[offset] = data;

	switch (offset)
	{
		case 0x04:		/* address register */
			pci_ide_regs[offset] &= 0xfffffff0;
			remap_dynamic_addresses();
			break;

		case 0x05:		/* address register */
			pci_ide_regs[offset] &= 0xfffffffc;
			remap_dynamic_addresses();
			break;

		case 0x08:		/* address register */
			pci_ide_regs[offset] &= 0xfffffff0;
			remap_dynamic_addresses();
			break;

		case 0x14:		/* interrupt pending */
			if (data & 4)
				ide_interrupt(0);
			break;
	}
	if (LOG_PCI)
		logerror("%06X:PCI IDE write: reg %d = %08X\n", activecpu_get_pc(), offset, data);
}



/*************************************
 *
 *  PCI 3dfx accesses
 *
 *************************************/

static READ32_HANDLER( pci_3dfx_r )
{
	UINT32 result = pci_3dfx_regs[offset];

	switch (offset)
	{
		case 0x00:		/* ID register: 0x0002 = SST-2, 0x121a = 3dfx */
			if (voodoo_type == 2)
				result = 0x0002121a;
			else
				result = 0x0003121a;
			break;

		case 0x02:		/* revision ID register */
			result = 0x00000002;
			break;

		case 0x10:		/* fab ID register?? */
			result = 0x00044000;
			break;

		case 0x15:		/* ???? -- gauntleg want's 0s in the bits below */
			result &= 0xf000ffff;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI 3dfx read: reg %d = %08X\n", activecpu_get_pc(), offset, result);
	return result;
}


static WRITE32_HANDLER( pci_3dfx_w )
{
	pci_3dfx_regs[offset] = data;

	switch (offset)
	{
		case 0x04:		/* address register */
			if (voodoo_type == 2)
				pci_3dfx_regs[offset] &= 0xff000000;
			else
				pci_3dfx_regs[offset] &= 0xfe000000;
			remap_dynamic_addresses();
			break;

		case 0x05:		/* address register */
			if (voodoo_type == 3)
				pci_3dfx_regs[offset] &= 0xfe000000;
			remap_dynamic_addresses();
			break;

		case 0x06:		/* I/O register */
			if (voodoo_type == 3)
				pci_3dfx_regs[offset] &= 0xffffff00;
			remap_dynamic_addresses();
			break;

		case 0x0c:		/* romBaseAddr register */
			if (voodoo_type == 3)
				pci_3dfx_regs[offset] &= 0xffff0000;
			remap_dynamic_addresses();
			break;

		case 0x10:		/* initEnable register */
			voodoo_set_init_enable(data);
			break;

	}
	if (LOG_PCI)
		logerror("%06X:PCI 3dfx write: reg %d = %08X\n", activecpu_get_pc(), offset, data);
}



/*************************************
 *
 *  nile timers & interrupts
 *
 *************************************/

static void update_nile_irqs(void)
{
	UINT32 intctll = nile_regs[NREG_INTCTRL+0];
	UINT32 intctlh = nile_regs[NREG_INTCTRL+1];
	UINT8 irq[6];
	int i;

	/* check for UART transmit IRQ enable and synthsize one */
	if (nile_regs[NREG_UARTIER] & 2)
		nile_irq_state |= 0x0010;
	else
		nile_irq_state &= ~0x0010;

	irq[0] = irq[1] = irq[2] = irq[3] = irq[4] = irq[5] = 0;
	nile_regs[NREG_INTSTAT0+0] = 0;
	nile_regs[NREG_INTSTAT0+1] = 0;
	nile_regs[NREG_INTSTAT1+0] = 0;
	nile_regs[NREG_INTSTAT1+1] = 0;

	/* handle the lower interrupts */
	for (i = 0; i < 8; i++)
		if (nile_irq_state & (1 << i))
			if ((intctll >> (4*i+3)) & 1)
			{
				int vector = (intctll >> (4*i)) & 7;
				if (vector < 6)
				{
					irq[vector] = 1;
					nile_regs[NREG_INTSTAT0 + vector/2] |= 1 << (i + 16*(vector&1));
				}
			}

	/* handle the upper interrupts */
	for (i = 0; i < 8; i++)
		if (nile_irq_state & (1 << (i+8)))
			if ((intctlh >> (4*i+3)) & 1)
			{
				int vector = (intctlh >> (4*i)) & 7;
				if (vector < 6)
				{
					irq[vector] = 1;
					nile_regs[NREG_INTSTAT0 + vector/2] |= 1 << (i + 8 + 16*(vector&1));
				}
			}

	/* push out the state */
	if (LOG_NILE_IRQS) logerror("NILE IRQs:");
	for (i = 0; i < 6; i++)
	{
		if (irq[i])
		{
			if (LOG_NILE_IRQS) logerror(" 1");
			cpunum_set_input_line(0, MIPS3_IRQ0 + i, ASSERT_LINE);
		}
		else
		{
			if (LOG_NILE_IRQS) logerror(" 0");
			cpunum_set_input_line(0, MIPS3_IRQ0 + i, CLEAR_LINE);
		}
	}
	if (LOG_NILE_IRQS) logerror("\n");
}


static void timer_callback(int which)
{
	UINT32 *regs = &nile_regs[NREG_T0CTRL + which * 4];
	if (LOG_TIMERS) logerror("timer %d fired\n", which);

	/* adjust the timer to fire again */
	{
		double period = TIMER_CLOCK;
		if (regs[1] & 2)
			logerror("Unexpected value: timer %d is prescaled\n", which);
		timer_adjust(timer[which], period * (regs[0] + 1), which, TIME_NEVER);
	}

	/* trigger the interrupt */
	if (which == 2)
		nile_irq_state |= 1 << 6;
	if (which == 3)
		nile_irq_state |= 1 << 5;

	update_nile_irqs();
}



/*************************************
 *
 *  Nile system controller
 *
 *************************************/

static READ32_HANDLER( nile_r )
{
	UINT32 result = nile_regs[offset];
	int logit = 1, which;

	switch (offset)
	{
		case NREG_CPUSTAT+0:	/* CPU status */
		case NREG_CPUSTAT+1:	/* CPU status */
			if (LOG_NILE) logerror("%08X:NILE READ: CPU status(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTCTRL+0:	/* Interrupt control */
		case NREG_INTCTRL+1:	/* Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt control(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTSTAT0+0:	/* Interrupt status 0 */
		case NREG_INTSTAT0+1:	/* Interrupt status 0 */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt status 0(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTSTAT1+0:	/* Interrupt status 1 */
		case NREG_INTSTAT1+1:	/* Interrupt status 1 */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt status 1/enable(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTCLR+0:		/* Interrupt clear */
		case NREG_INTCLR+1:		/* Interrupt clear */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt clear(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTPPES+0:	/* PCI Interrupt control */
		case NREG_INTPPES+1:	/* PCI Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE READ: PCI interrupt control(%03X) = %08X\n", activecpu_get_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_PCIERR+0:		/* PCI error */
		case NREG_PCIERR+1:		/* PCI error */
		case NREG_PCICTRL+0:	/* PCI control */
		case NREG_PCICTRL+1:	/* PCI arbiter */
		case NREG_PCIINIT0+0:	/* PCI master */
		case NREG_PCIINIT0+1:	/* PCI master */
		case NREG_PCIINIT1+0:	/* PCI master */
		case NREG_PCIINIT1+1:	/* PCI master */
			logit = 0;
			break;

		case NREG_T0CNTR:		/* SDRAM timer control (counter) */
		case NREG_T1CNTR:		/* bus timeout timer control (counter) */
		case NREG_T2CNTR:		/* general purpose timer control (counter) */
		case NREG_T3CNTR:		/* watchdog timer control (counter) */
			which = (offset - NREG_T0CTRL) / 4;
			if (nile_regs[offset - 1] & 1)
			{
				double period = TIMER_CLOCK;
				if (nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				result = nile_regs[offset + 1] = timer_timeleft(timer[which]) / period;
			}

			if (LOG_TIMERS) logerror("%08X:NILE READ: timer %d counter(%03X) = %08X\n", activecpu_get_pc(), which, offset*4, result);
			logit = 0;
			break;

		case NREG_UARTIIR:			/* serial port interrupt ID */
			if (nile_regs[NREG_UARTIER] & 2)
				result = 0x02;			/* transmitter buffer IRQ pending */
			else
				result = 0x01;			/* no IRQ pending */
			break;

		case NREG_UARTLSR:			/* serial port line status */
			result = 0x60;
			logit = 0;
			break;

		case NREG_VID:
		case NREG_PCICMD:
		case NREG_REVID:
		case NREG_CLSIZ:
		case NREG_BARC:
		case NREG_BAR0:
		case NREG_BAR1:
		case NREG_CIS:
		case NREG_SSVID:
		case NREG_ROM:
		case NREG_INTLIN:
		case NREG_BAR2:
		case NREG_BAR3:
		case NREG_BAR4:
		case NREG_BAR5:
		case NREG_BAR6:
		case NREG_BAR7:
		case NREG_BAR8:
		case NREG_BARB:
			result = pci_bridge_r(offset & 0x3f, mem_mask);
			break;

	}

	if (LOG_NILE && logit)
		logerror("%06X:nile read from offset %03X = %08X\n", activecpu_get_pc(), offset*4, result);
	return result;
}


static WRITE32_HANDLER( nile_w )
{
	UINT32 olddata = nile_regs[offset];
	int logit = 1, which;

	COMBINE_DATA(&nile_regs[offset]);

	switch (offset)
	{
		case NREG_CPUSTAT+0:	/* CPU status */
		case NREG_CPUSTAT+1:	/* CPU status */
			if (LOG_NILE) logerror("%08X:NILE WRITE: CPU status(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			break;

		case NREG_INTCTRL+0:	/* Interrupt control */
		case NREG_INTCTRL+1:	/* Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt control(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTSTAT0+0:	/* Interrupt status 0 */
		case NREG_INTSTAT0+1:	/* Interrupt status 0 */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt status 0(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTSTAT1+0:	/* Interrupt status 1 */
		case NREG_INTSTAT1+1:	/* Interrupt status 1 */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt status 1/enable(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTCLR+0:		/* Interrupt clear */
		case NREG_INTCLR+1:		/* Interrupt clear */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt clear(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			nile_irq_state &= ~nile_regs[offset];
			update_nile_irqs();
			break;

		case NREG_INTPPES+0:	/* PCI Interrupt control */
		case NREG_INTPPES+1:	/* PCI Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE WRITE: PCI interrupt control(%03X) = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
			logit = 0;
			break;

		case NREG_PCIERR+0:		/* PCI error */
		case NREG_PCIERR+1:		/* PCI error */
		case NREG_PCICTRL+0:	/* PCI control */
		case NREG_PCICTRL+1:	/* PCI arbiter */
		case NREG_PCIINIT0+0:	/* PCI master */
		case NREG_PCIINIT0+1:	/* PCI master */
		case NREG_PCIINIT1+1:	/* PCI master */
			logit = 0;
			break;

		case NREG_PCIINIT1+0:	/* PCI master */
			if (((olddata & 0xe) == 0xa) != ((nile_regs[offset] & 0xe) == 0xa))
				remap_dynamic_addresses();
			logit = 0;
			break;

		case NREG_T0CTRL+1:		/* SDRAM timer control (control bits) */
		case NREG_T1CTRL+1:		/* bus timeout timer control (control bits) */
		case NREG_T2CTRL+1:		/* general purpose timer control (control bits) */
		case NREG_T3CTRL+1:		/* watchdog timer control (control bits) */
			which = (offset - NREG_T0CTRL) / 4;
			if (LOG_NILE) logerror("%08X:NILE WRITE: timer %d control(%03X) = %08X & %08X\n", activecpu_get_pc(), which, offset*4, data, ~mem_mask);
			logit = 0;

			/* timer just enabled? */
			if (!(olddata & 1) && (nile_regs[offset] & 1))
			{
				double period = TIMER_CLOCK;
				if (nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				timer_adjust(timer[which], period + period * nile_regs[offset + 1], which, TIME_NEVER);
				if (LOG_TIMERS) logerror("Starting timer %d at a rate of %d Hz\n", which, (int)(1.0 / (period + period * nile_regs[offset + 1])));
			}

			/* timer disabled? */
			else if ((olddata & 1) && !(nile_regs[offset] & 1))
			{
				double period = TIMER_CLOCK;
				if (nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				nile_regs[offset + 1] = timer_timeleft(timer[which]) / period;
				timer_adjust(timer[which], TIME_NEVER, which, TIME_NEVER);
			}
			break;

		case NREG_T0CNTR:		/* SDRAM timer control (counter) */
		case NREG_T1CNTR:		/* bus timeout timer control (counter) */
		case NREG_T2CNTR:		/* general purpose timer control (counter) */
		case NREG_T3CNTR:		/* watchdog timer control (counter) */
			which = (offset - NREG_T0CTRL) / 4;
			if (LOG_TIMERS) logerror("%08X:NILE WRITE: timer %d counter(%03X) = %08X & %08X\n", activecpu_get_pc(), which, offset*4, data, ~mem_mask);
			logit = 0;

			if (nile_regs[offset - 1] & 1)
			{
				double period = TIMER_CLOCK;
				if (nile_regs[offset - 1] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				period *= nile_regs[offset];
				timer_adjust(timer[which], period, which, TIME_NEVER);
			}
			break;

		case NREG_UARTTHR:		/* serial port output */
			if (PRINTF_SERIAL) printf("%c", data & 0xff);
			logit = 0;
			break;
		case NREG_UARTIER:		/* serial interrupt enable */
			update_nile_irqs();
			break;

		case NREG_VID:
		case NREG_PCICMD:
		case NREG_REVID:
		case NREG_CLSIZ:
		case NREG_BARC:
		case NREG_BAR0:
		case NREG_BAR1:
		case NREG_CIS:
		case NREG_SSVID:
		case NREG_ROM:
		case NREG_INTLIN:
		case NREG_BAR2:
		case NREG_BAR3:
		case NREG_BAR4:
		case NREG_BAR5:
		case NREG_BAR6:
		case NREG_BAR7:
		case NREG_BAR8:
		case NREG_BARB:
			pci_bridge_w(offset & 0x3f, data, mem_mask);
			break;

		case NREG_DCS2:
		case NREG_DCS3:
		case NREG_DCS4:
		case NREG_DCS5:
		case NREG_DCS6:
		case NREG_DCS7:
		case NREG_DCS8:
		case NREG_PCIW0:
		case NREG_PCIW1:
			remap_dynamic_addresses();
			break;
	}

	if (LOG_NILE && logit)
		logerror("%06X:nile write to offset %03X = %08X & %08X\n", activecpu_get_pc(), offset*4, data, ~mem_mask);
}



/*************************************
 *
 *  IDE interrupts
 *
 *************************************/

static void ide_interrupt(int state)
{
	ide_irq_state = state;
	if (state)
		nile_irq_state |= 0x800;
	else
		nile_irq_state &= ~0x800;
	update_nile_irqs();
}

static struct ide_interface ide_intf =
{
	ide_interrupt
};



/*************************************
 *
 *  SIO interrupts
 *
 *************************************/

static void update_sio_irqs(void)
{
	if (sio_irq_state & sio_irq_enable)
		nile_irq_state |= 0x400;
	else
		nile_irq_state &= ~0x400;
	update_nile_irqs();
}


static void vblank_assert(int state)
{
	if (!vblank_state && state)
	{
		sio_irq_state |= 0x20;
		update_sio_irqs();
	}
	vblank_state = state;

	/* if we have stalled DMA, restart */
//  if (dma_pending_on_vblank[0]) { cpuintrf_push_context(0); perform_dma(0); cpuintrf_pop_context(); }
//  if (dma_pending_on_vblank[1]) { cpuintrf_push_context(0); perform_dma(1); cpuintrf_pop_context(); }
//  if (dma_pending_on_vblank[2]) { cpuintrf_push_context(0); perform_dma(2); cpuintrf_pop_context(); }
//  if (dma_pending_on_vblank[3]) { cpuintrf_push_context(0); perform_dma(3); cpuintrf_pop_context(); }
}


static void ioasic_irq(int state)
{
	if (state)
		sio_irq_state |= 0x04;
	else
		sio_irq_state &= ~0x04;
	update_sio_irqs();
}


static void ethernet_interrupt(int state)
{
	if (state)
		sio_irq_state |= 0x10;
	else
		sio_irq_state &= ~0x10;
	update_sio_irqs();
}


static READ32_HANDLER( sio_irq_clear_r )
{
	return sio_irq_clear;
}


static WRITE32_HANDLER( sio_irq_clear_w )
{
	if (!(mem_mask & 0x000000ff))
	{
		sio_irq_clear = data;

		/* bit 0x01 seems to be used to reset the IOASIC */
		if (!(data & 0x01))
		{
			midway_ioasic_reset();
			dcs_reset_w(data & 0x01);
		}

		/* they toggle bit 0x08 low to reset the VBLANK */
		if (!(data & 0x08))
		{
			sio_irq_state &= ~0x20;
			update_sio_irqs();
		}
	}
}


static READ32_HANDLER( sio_irq_enable_r )
{
	return sio_irq_enable;
}


static WRITE32_HANDLER( sio_irq_enable_w )
{
	if (!(mem_mask & 0x000000ff))
	{
		sio_irq_enable = data;
		update_sio_irqs();
	}
}


static READ32_HANDLER( sio_irq_status_r )
{
	return sio_irq_state;
}


static WRITE32_HANDLER( sio_led_w )
{
	if (!(mem_mask & 0x000000ff))
		sio_led_state = data;
}


static READ32_HANDLER( sio_led_r )
{
	return sio_led_state;
}



/*************************************
 *
 *  SIO FPGA accesses
 *
 *************************************/

static WRITE32_HANDLER( sio_w )
{
	if (!(mem_mask & 0x000000ff)) offset += 0;
	if (!(mem_mask & 0x0000ff00)) offset += 1;
	if (!(mem_mask & 0x00ff0000)) offset += 2;
	if (!(mem_mask & 0xff000000)) offset += 3;
	if (LOG_SIO && offset != 0)
		logerror("%08X:sio write to offset %X = %02X\n", activecpu_get_pc(), offset, data >> (offset*8));
	if (offset < 4)
		sio_data[offset] = data >> (offset*8);
	if (offset == 1)
		sio_data[2] = (sio_data[2] & ~0x02) | ((sio_data[1] & 0x01) << 1) | (sio_data[1] & 0x01);
}


static READ32_HANDLER( sio_r )
{
	UINT32 result = 0;
	if (!(mem_mask & 0x000000ff)) offset += 0;
	if (!(mem_mask & 0x0000ff00)) offset += 1;
	if (!(mem_mask & 0x00ff0000)) offset += 2;
	if (!(mem_mask & 0xff000000)) offset += 3;
	if (offset < 4)
		result = sio_data[0] | (sio_data[1] << 8) | (sio_data[2] << 16) | (sio_data[3] << 24);
	if (LOG_SIO && offset != 2)
		logerror("%08X:sio read from offset %X = %02X\n", activecpu_get_pc(), offset, result >> (offset*8));
	return result;
}



/*************************************
 *
 *  Analog input handling
 *
 *************************************/

static READ32_HANDLER( analog_port_r )
{
	return pending_analog_read;
}


static WRITE32_HANDLER( analog_port_w )
{
	if (data < 8 || data > 15)
		logerror("%08X:Unexpected analog port select = %08X\n", activecpu_get_pc(), data);
	pending_analog_read = readinputport(4 + (data & 7));
}



/*************************************
 *
 *  Misc accesses
 *
 *************************************/

static WRITE32_HANDLER( vegas_watchdog_w )
{
	activecpu_eat_cycles(100);
}


static WRITE32_HANDLER( asic_fifo_w )
{
	midway_ioasic_fifo_w(data);
}


static READ32_HANDLER( ide_main_r )
{
	return ide_controller32_0_r(0x1f0/4 + offset, mem_mask);
}


static WRITE32_HANDLER( ide_main_w )
{
	ide_controller32_0_w(0x1f0/4 + offset, data, mem_mask);
}


static READ32_HANDLER( ide_alt_r )
{
	return ide_controller32_0_r(0x3f4/4 + offset, mem_mask);
}


static WRITE32_HANDLER( ide_alt_w )
{
	ide_controller32_0_w(0x3f4/4 + offset, data, mem_mask);
}


static READ32_HANDLER( ethernet_r )
{
	UINT32 result = 0;
	if ((mem_mask & 0x0000ffff) != 0x0000ffff)
		result |= smc91c94_r(offset * 2 + 0, mem_mask);
	if ((mem_mask & 0xffff0000) != 0xffff0000)
		result |= smc91c94_r(offset * 2 + 1, mem_mask >> 16) << 16;
	return result;
}


static WRITE32_HANDLER( ethernet_w )
{
	if ((mem_mask & 0x0000ffff) != 0x0000ffff)
		smc91c94_w(offset * 2 + 0, data, mem_mask);
	if ((mem_mask & 0xffff0000) != 0xffff0000)
		smc91c94_w(offset * 2 + 1, data >> 16, mem_mask >> 16);
}


static WRITE32_HANDLER( sound_dma_addr_w )
{
	cpuintrf_push_context(1);
	adsp2181_idma_addr_w(data);
	cpuintrf_pop_context();
}


static WRITE32_HANDLER( sound_dma_data_w )
{
	cpuintrf_push_context(1);
	adsp2181_idma_data_w(data);
	cpuintrf_pop_context();
}



/*************************************
 *
 *  Dynamic addressing
 *
 *************************************/

#define add_dynamic_address(s,e,r,w)	_add_dynamic_address(s,e,r,w,#r,#w)

INLINE void _add_dynamic_address(offs_t start, offs_t end, read32_handler read, write32_handler write, const char *rdname, const char *wrname)
{
	dynamic[dynamic_count].start = start;
	dynamic[dynamic_count].end = end;
	dynamic[dynamic_count].read = read;
	dynamic[dynamic_count].write = write;
	dynamic[dynamic_count].rdname = rdname;
	dynamic[dynamic_count].wrname = wrname;
	dynamic_count++;
}


static void remap_dynamic_addresses(void)
{
	offs_t base;
	int addr;

	/* unmap everything we know about */
	for (addr = 0; addr < dynamic_count; addr++)
	{
		memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, dynamic[addr].start, dynamic[addr].end, 0, 0, MRA32_NOP);
		memory_install_write32_handler(0, ADDRESS_SPACE_PROGRAM, dynamic[addr].start, dynamic[addr].end, 0, 0, MWA32_NOP);
	}

	/* the build the list of stuff */
	dynamic_count = 0;

	/* DCS2 */
	base = nile_regs[NREG_DCS2] & 0x1fffff00;
	if (base >= ramsize)
	{
		add_dynamic_address(base + 0x0000, base + 0x0003, sio_irq_clear_r, sio_irq_clear_w);
		add_dynamic_address(base + 0x1000, base + 0x1003, sio_irq_enable_r, sio_irq_enable_w);
		add_dynamic_address(base + 0x2000, base + 0x2003, sio_irq_status_r, NULL);
		add_dynamic_address(base + 0x4000, base + 0x4003, sio_led_r, sio_led_w);
		add_dynamic_address(base + 0x5000, base + 0x5007, MRA32_NOP, NULL);
		add_dynamic_address(base + 0x6000, base + 0x6003, NULL, cmos_unlock_w);
		add_dynamic_address(base + 0x7000, base + 0x7003, NULL, vegas_watchdog_w);
	}

	/* DCS3 */
	base = nile_regs[NREG_DCS3] & 0x1fffff00;
	if (base >= ramsize)
		add_dynamic_address(base + 0x0000, base + 0x0003, analog_port_r, analog_port_w);

	/* DCS4 */
	base = nile_regs[NREG_DCS4] & 0x1fffff00;
	if (base >= ramsize)
		add_dynamic_address(base + 0x0000, base + 0x7fff, timekeeper_r, timekeeper_w);

	/* DCS5 */
	base = nile_regs[NREG_DCS5] & 0x1fffff00;
	if (base >= ramsize)
		add_dynamic_address(base + 0x0000, base + 0x0003, sio_r, sio_w);

	/* DCS6 */
	base = nile_regs[NREG_DCS6] & 0x1fffff00;
	if (base >= ramsize)
	{
		add_dynamic_address(base + 0x0000, base + 0x003f, midway_ioasic_packed_r, midway_ioasic_packed_w);
		add_dynamic_address(base + 0x1000, base + 0x1003, NULL, asic_fifo_w);
		if (has_dcs3)
		{
			add_dynamic_address(base + 0x5000, base + 0x5003, NULL, sound_dma_addr_w);
			add_dynamic_address(base + 0x7000, base + 0x7003, NULL, sound_dma_data_w);
		}
	}

	/* DCS7 */
	base = nile_regs[NREG_DCS7] & 0x1fffff00;
	if (base >= ramsize)
		add_dynamic_address(base + 0x1000, base + 0x100f, ethernet_r, ethernet_w);

	/* PCI config space */
	if ((nile_regs[NREG_PCIINIT1] & 0xe) == 0xa)
	{
		base = nile_regs[NREG_PCIW1] & 0x1fffff00;
		if (base >= ramsize)
		{
			add_dynamic_address(base + (1 << (21 + 4)) + 0x0000, base + (1 << (21 + 4)) + 0x00ff, pci_3dfx_r, pci_3dfx_w);
			add_dynamic_address(base + (1 << (21 + 5)) + 0x0000, base + (1 << (21 + 5)) + 0x00ff, pci_ide_r, pci_ide_w);
		}
	}

	/* PCI real space */
	else
	{
		/* IDE controller */
		base = pci_ide_regs[0x04] & 0xfffffff0;
		if (base >= ramsize && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x000f, ide_main_r, ide_main_w);

		base = pci_ide_regs[0x05] & 0xfffffffc;
		if (base >= ramsize && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x0003, ide_alt_r, ide_alt_w);

		base = pci_ide_regs[0x08] & 0xfffffff0;
		if (base >= ramsize && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x0007, ide_bus_master32_0_r, ide_bus_master32_0_w);

		/* 3dfx card */
		base = pci_3dfx_regs[0x04] & 0xfffffff0;
		if (base >= ramsize && base < 0x20000000)
		{
			if (voodoo_type == 2)
			{
				add_dynamic_address(base + 0x000000, base + 0x3fffff, voodoo_regs_r, voodoo2_regs_w);
				add_dynamic_address(base + 0x400000, base + 0x7fffff, voodoo_framebuf_r, voodoo_framebuf_w);
				add_dynamic_address(base + 0x800000, base + 0xffffff, MRA32_NOP, voodoo_textureram_w);
			}
			else
			{
				add_dynamic_address(base + 0x0000000, base + 0x007ffff, voodoo3_ioreg_r, voodoo3_ioreg_w);
				add_dynamic_address(base + 0x0080000, base + 0x00fffff, voodoo3_cmdagp_r, voodoo3_cmdagp_w);
				add_dynamic_address(base + 0x0100000, base + 0x01fffff, voodoo3_2d_r, voodoo3_2d_w);
				add_dynamic_address(base + 0x0200000, base + 0x05fffff, voodoo_regs_r, voodoo2_regs_w);
				add_dynamic_address(base + 0x0600000, base + 0x09fffff, MRA32_NOP, voodoo_textureram_w);
				add_dynamic_address(base + 0x0c00000, base + 0x0ffffff, voodoo3_yuv_r, voodoo3_yuv_w);
				add_dynamic_address(base + 0x1000000, base + 0x1ffffff, voodoo_framebuf_r, voodoo_framebuf_w);
			}
		}

		if (voodoo_type >= 3)
		{
			base = pci_3dfx_regs[0x05] & 0xfffffff0;
			if (base >= ramsize && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x1ffffff, voodoo_framebuf_r, voodoo_framebuf_w);

			base = pci_3dfx_regs[0x06] & 0xfffffff0;
			if (base >= ramsize && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x00000ff, voodoo3_ioreg_r, voodoo3_ioreg_w);

			base = pci_3dfx_regs[0x0c] & 0xffff0000;
			if (base >= ramsize && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x000ffff, voodoo3_rom_r, NULL);
		}
	}

	/* now remap everything */
	if (LOG_DYNAMIC) logerror("remap_dynamic_addresses:\n");
	for (addr = 0; addr < dynamic_count; addr++)
	{
		if (LOG_DYNAMIC) logerror("  installing: %08X-%08X %s,%s\n", 0xa0000000+dynamic[addr].start, 0xa0000000+dynamic[addr].end, dynamic[addr].rdname, dynamic[addr].wrname);
		if (dynamic[addr].read)
			memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, dynamic[addr].start, dynamic[addr].end, 0, 0, dynamic[addr].read);
		if (dynamic[addr].write)
			memory_install_write32_handler(0, ADDRESS_SPACE_PROGRAM, dynamic[addr].start, dynamic[addr].end, 0, 0, dynamic[addr].write);
	}

	if (LOG_DYNAMIC)
	{
		static int count = 0;
		++count;
		ui_popup("Remaps = %d", count);
	}
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( vegas_map_8mb, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_BASE(&rambase) AM_SIZE(&ramsize)
	AM_RANGE(0x1fa00000, 0x1fa00fff) AM_READWRITE(nile_r, nile_w) AM_BASE(&nile_regs)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION(REGION_USER1, 0) AM_BASE(&rombase)
ADDRESS_MAP_END


static ADDRESS_MAP_START( vegas_map_32mb, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM AM_BASE(&rambase) AM_SIZE(&ramsize)
	AM_RANGE(0x1fa00000, 0x1fa00fff) AM_READWRITE(nile_r, nile_w) AM_BASE(&nile_regs)
	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION(REGION_USER1, 0) AM_BASE(&rombase)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

INPUT_PORTS_START( gauntleg )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_BIT( 0x003e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0780, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0800, 0x0800, "SIO Rev" )
	PORT_DIPSETTING(      0x0800, "1 or later")
	PORT_DIPSETTING(      0x0000, "0")
	PORT_DIPNAME( 0x1000, 0x1000, "Harness" )
	PORT_DIPSETTING(      0x1000, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "Midway" )
	PORT_DIPNAME( 0x2000, 0x2000, "Joysticks" )
	PORT_DIPSETTING(      0x2000, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( gauntdl )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4 or later" )
	PORT_DIPSETTING(      0x0000, "3 or earlier" )
	PORT_DIPNAME( 0x0004, 0x0004, "DRAM" )
	PORT_DIPSETTING(      0x0004, "8MB" )
	PORT_DIPSETTING(      0x0000, "32MB" )
	PORT_BIT( 0x0038, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0780, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0800, 0x0800, "SIO Rev" )
	PORT_DIPSETTING(      0x0800, "1 or later")
	PORT_DIPSETTING(      0x0000, "0")
	PORT_DIPNAME( 0x1000, 0x1000, "Harness" )
	PORT_DIPSETTING(      0x1000, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "Midway" )
	PORT_DIPNAME( 0x2000, 0x2000, "Joysticks" )
	PORT_DIPSETTING(      0x2000, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( tenthdeg )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0xc000, 0xc000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	/* P1 jab */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	/* P1 strong */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	/* P1 short */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	/* P2 jab */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	/* P2 strong */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	/* P2 short */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* P1 roundhouse */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	/* P1 fierce */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)	/* P1 forward */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)	/* P2 roundhouse */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)	/* P2 forward */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)	/* P2 fierce */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( warfa )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "?" )
	PORT_BIT( 0x003c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x3f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


INPUT_PORTS_START( roadburn )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "?" )
	PORT_BIT( 0x003c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x3f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


INPUT_PORTS_START( nbashowt )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ))
	PORT_DIPSETTING(      0x000e, "Mode 1" )
	PORT_DIPSETTING(      0x0008, "Mode 2" )
	PORT_DIPSETTING(      0x0009, "Mode 3" )
	PORT_DIPSETTING(      0x0002, "Mode 4" )
	PORT_DIPSETTING(      0x000c, "Mode ECA" )
//  PORT_DIPSETTING(      0x0004, "Not Used 1" )        /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0008, "Not Used 2" )        /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0030, 0x0030, "Curency Type" )
	PORT_DIPSETTING(      0x0030, DEF_STR( USA ))
	PORT_DIPSETTING(      0x0020, DEF_STR( French ))
	PORT_DIPSETTING(      0x0010, DEF_STR( German ))
//  PORT_DIPSETTING(      0x0000, "Not Used" )      /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ))	/* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0040, "0" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0080, 0x0080, "Power Up Test Loop" )
	PORT_DIPSETTING(      0x0080, "One Time" )
	PORT_DIPSETTING(      0x0000, "Continuous" )
	PORT_DIPNAME( 0x0100, 0x0100, "Joysticks" )
	PORT_DIPSETTING(      0x0100, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0x0600, 0x0200, "Graphics Mode" )
	PORT_DIPSETTING(      0x0200, "512x385 @ 25KHz" )
	PORT_DIPSETTING(      0x0400, "512x256 @ 15KHz" )
//  PORT_DIPSETTING(      0x0600, "0" )         /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0000, "3" )         /* Marked as Unused in the manual */
	PORT_DIPNAME( 0x1800, 0x1800, "Graphics Speed" )
	PORT_DIPSETTING(      0x0000, "45 MHz" )
	PORT_DIPSETTING(      0x0800, "47 MHz" )
	PORT_DIPSETTING(      0x1000, "49 MHz" )
	PORT_DIPSETTING(      0x1800, "51 MHz" )
	PORT_DIPNAME( 0x2000, 0x0000, "Bill Validator" )
	PORT_DIPSETTING(      0x2000, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, "One" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ))
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ))
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( vegas )
	PORT_START	    /* DIPs */
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) /* Test switch */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	/* Bill */

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	/* 3d cam */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static struct mips3_config config =
{
	16384,			/* code cache size */
	16384,			/* data cache size */
	SYSTEM_CLOCK	/* system clock rate */
};

MACHINE_DRIVER_START( vegascore )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", R5000LE, SYSTEM_CLOCK*2)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(vegas_map_8mb,0)

	MDRV_FRAMES_PER_SECOND(57)
	MDRV_VBLANK_DURATION(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_INIT(vegas)
	MDRV_NVRAM_HANDLER(timekeeper_save)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_VISIBLE_AREA(0, 639, 0, 479)
	MDRV_PALETTE_LENGTH(65536)

	MDRV_VIDEO_START(voodoo2_2x4mb)
	MDRV_VIDEO_STOP(voodoo)
	MDRV_VIDEO_UPDATE(voodoo)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( vegas )
	MDRV_IMPORT_FROM(vegascore)
	MDRV_IMPORT_FROM(dcs2_audio_2104)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( vegas250 )
	MDRV_IMPORT_FROM(vegascore)
	MDRV_CPU_REPLACE("main", R5000LE, SYSTEM_CLOCK*2.5)
	MDRV_IMPORT_FROM(dcs2_audio_2104)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( vegas32m )
	MDRV_IMPORT_FROM(vegascore)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(vegas_map_32mb,0)
	MDRV_IMPORT_FROM(dcs3_audio)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( vegasban )
	MDRV_IMPORT_FROM(vegas32m)
	MDRV_VIDEO_START(voodoo3_1x4mb)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( vegasv3 )
	MDRV_IMPORT_FROM(vegas32m)
	MDRV_CPU_REPLACE("main", RM7000LE, SYSTEM_CLOCK*2.5)
	MDRV_VIDEO_START(voodoo3_2x4mb)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( gauntleg )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "legend15.bin", 0x000000, 0x80000, CRC(a8372d70) SHA1(d8cd4fd4d7007ee38bb58b5a818d0f83043d5a48) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "gauntleg", 0, MD5(2d814e48ef612fa387df671466824c97) SHA1(2e01dc43186578384207dc0b630efbc46427dd46) )
ROM_END


ROM_START( gauntl12 )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "legend12.bin", 0x000000, 0x80000, CRC(34674c5f) SHA1(92ec1779f3ab32944cbd953b6e1889503a57794b) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "gauntl12", 0, MD5(52e4b984ae722da56ff6cf623e296a32) SHA1(bd01508ffd8fee497c8a75eacddff816147fd8c2) )
ROM_END


ROM_START( gauntdl )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "gauntdl.bin", 0x000000, 0x80000, CRC(3d631518) SHA1(d7f5a3bc109a19c9c7a711d607ff87e11868b536) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "gauntdl", 0, MD5(002cbaf85806be0dbd21949bf8ce83c2) SHA1(5e73de969a2d6896bcd01d075370520e063e8cc5) )
ROM_END


ROM_START( warfa )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "warboot.v19", 0x000000, 0x80000, CRC(b0c095cd) SHA1(d3b8cccdca83f0ecb49aa7993864cfdaa4e5c6f0) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "warfa", 0, MD5(01035f301d84d665f7c4bf7e3554c516) SHA1(9ce1d4a3115329b9b6fb6482b11b22b52a7fef79) )
ROM_END


ROM_START( tenthdeg )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "tenthdeg.snd", 0x000000, 0x8000, CRC(1c75c1c1) SHA1(02ac1419b0fd4acc3f39676e7dce879e926d998b) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "tenthdeg.bio", 0x000000, 0x80000, CRC(1cd2191b) SHA1(a40c48f3d6a9e2760cec809a79a35abe762da9ce) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "tenthdeg", 0, MD5(be653883b640f540945e9c8ab8f72463) SHA1(5ba31d22c0fa29897b45e01c4f1afed8b906f500) )
ROM_END


ROM_START( roadburn )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, NO_DUMP CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "rbmain.bin", 0x000000, 0x80000, CRC(060e1aa8) SHA1(2a1027d209f87249fe143500e721dfde7fb5f3bc) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "roadburn", 0, MD5(ce4710671f4266389e7d71f1fc0da81d) SHA1(1c971c9ed573d178d9f318ccd88d305d8146de2d) )
ROM_END


ROM_START( nbashowt )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "nbau27.100", 0x000000, 0x80000, CRC(ff5d620d) SHA1(8f07567929f40a2269a42495dfa9dd5edef688fe) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "nbashowt", 0, MD5(2dab719f8f0fdeb8ac1db3844ed8c1e4) SHA1(bf60b8f74647dc911f78f364a72ef0301ae0167a) )
ROM_END


ROM_START( nbanfl )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "u27nflnba.bin", 0x000000, 0x80000, CRC(6a9bd382) SHA1(18b942df6af86ea944c24166dbe88148334eaff9) )
//  ROM_LOAD( "bootnflnba.bin", 0x000000, 0x80000, CRC(3def7053) SHA1(8f07567929f40a2269a42495dfa9dd5edef688fe) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "nbanfl", 0, MD5(9e3748957c672f6d7a1e464546f46b15) SHA1(4256c7487a55fd0d0e4241f595cc886d4402fd7d) )
ROM_END


ROM_START( cartfury )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "bootu27", 0x000000, 0x80000, CRC(c44550a2) SHA1(ad30f1c3382ff2f5902a4cbacbb1f0c4e37f42f9) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "cartfury", 0, MD5(d8e9d2616f8d70155f1068f884aa39e5) SHA1(98597d79ea25c0e74a575ba636abccc68fd5d301) )
ROM_END


ROM_START( sfru2049 )
	ROM_REGION16_LE( 0x410000, REGION_SOUND1, 0 )	/* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, NO_DUMP CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	ROM_REGION32_LE( 0x80000, REGION_USER1, 0 )
	ROM_LOAD( "u27a.dat", 0x000000, 0x80000, CRC(174ba8fe) SHA1(baba83b811eca659f00514a008a86ef0ac9680ee) )

	DISK_REGION( REGION_DISKS )
	DISK_IMAGE( "sfru2049", 0, MD5(2f56375670c0f72b69c1b5ec6a54ba70) SHA1(e08d026ab2745cac6b8c820f010b28dffd5388dd) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static void init_common(int ioasic, int serialnum)
{
	static struct smc91c9x_interface ethernet_intf =
	{
		ethernet_interrupt
	};

	/* initialize the subsystems */
	ide_controller_init(0, &ide_intf);
	midway_ioasic_init(ioasic, serialnum, 80, ioasic_irq);
	midway_ioasic_set_auto_ack(1);
	smc91c94_init(&ethernet_intf);

	/* set our VBLANK callback */
	voodoo_set_vblank_callback(vblank_assert);

	/* allocate RAM for the timekeeper */
	timekeeper_nvram_size = 0x8000;
	timekeeper_nvram = auto_malloc(timekeeper_nvram_size);
}


static DRIVER_INIT( gauntleg )
{
	dcs2_init(0);
	init_common(MIDWAY_IOASIC_CALSPEED, 340/* 322, others? */);
}


static DRIVER_INIT( gauntdl )
{
	dcs2_init(0);
	init_common(MIDWAY_IOASIC_GAUNTDL, 346/* 347, others? */);
}


static DRIVER_INIT( warfa )
{
	dcs2_init(0);
	init_common(MIDWAY_IOASIC_MACE, 337/* others? */);
}


static DRIVER_INIT( tenthdeg )
{
	dcs2_init(0);
	init_common(MIDWAY_IOASIC_GAUNTDL, 330/* others? */);
}


static DRIVER_INIT( roadburn )
{
	dcs3_init(0);
	init_common(MIDWAY_IOASIC_STANDARD, 325/* others? */);
}


static DRIVER_INIT( nbashowt )
{
	dcs3_init(0);
	init_common(MIDWAY_IOASIC_MACE, 322/* unknown */);
}


static DRIVER_INIT( nbanfl )
{
	dcs3_init(0);
	init_common(MIDWAY_IOASIC_STANDARD, 109/* others? */);
	/* NOT: MACE */
}


static DRIVER_INIT( cartfury )
{
	dcs3_init(0);
	init_common(MIDWAY_IOASIC_CARNEVIL, 495/* others? */);
}


static DRIVER_INIT( sfru2049 )
{
	dcs3_init(0);
	init_common(MIDWAY_IOASIC_STANDARD, 336/* others? */);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// Voodoo 2 games
GAME( 1998, gauntleg, 0,        vegas,    gauntleg, gauntleg, ROT0, "Atari Games", "Gauntlet Legends (version 1.6)", 0 )
GAME( 1998, gauntl12, gauntleg, vegas,    gauntleg, gauntleg, ROT0, "Atari Games", "Gauntlet Legends (version 1.2)", GAME_NO_SOUND )
GAME( 1999, gauntdl,  0,        vegas,    gauntdl , gauntdl,  ROT0, "Midway Games", "Gauntlet Dark Legacy", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, warfa,    0,        vegas250, warfa,    warfa,    ROT0, "Atari Games", "War: The Final Assault", GAME_NOT_WORKING )
GAME( 1998, tenthdeg, 0,        vegas,    tenthdeg, tenthdeg, ROT0, "Atari Games", "Tenth Degree", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, roadburn, 0,        vegas32m, roadburn, roadburn, ROT0, "Atari Games", "Road Burners", GAME_NO_SOUND | GAME_NOT_WORKING )

// Voodoo banshee games
GAME( 1998, nbashowt, 0,        vegasban, nbashowt, nbashowt, ROT0, "Midway Games", "NBA Showtime: NBA on NBC", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 1999, nbanfl,   0,        vegasban, nbashowt, nbanfl,   ROT0, "Midway Games", "NBA Showtime / NFL Blitz 2000", GAME_NO_SOUND | GAME_NOT_WORKING )

// Voodoo 3 games
GAME( 1998, sfru2049, 0,        vegasv3,  vegas,    sfru2049, ROT0, "Atari Games", "San Francisco Rush 2049", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000, cartfury, 0,        vegasv3,  roadburn, cartfury, ROT0, "Midway Games", "Cart Fury", GAME_NO_SOUND | GAME_NOT_WORKING )
