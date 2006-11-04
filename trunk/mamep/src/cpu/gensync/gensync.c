/*****************************************************************************
 * Generic Video Synchronization CPU replacement for non-CPU games
 * It does nothing but count horizontal and vertical synchronization,
 * so a driver can use cpu_getscanline() and the provided registers to
 * access the bits of generic horizontal and vertical counters.
 * A driver defines it's video layout by specifying ten parameters:
 * h_max			horizontal modulo value (including blanking/sync)
 * v_max			vertical modulo value (including blanking/sync)
 * hblank_start 	start of horizontal blanking
 * hsync_start		start of horizontal sync
 * hsync_end		end of horizontal sync
 * hblank_end		end of horizontal blanking
 * vblank_start 	start of vertical blanking
 * vsync_start		start of vertical sync
 * vsync_end		end of vertical sync
 * vblank_end		end of vertical blanking
 *****************************************************************************/

#include <stdio.h>
#include "driver.h"
#include "debugger.h"
#include "state.h"
#include "gensync.h"

typedef struct
{
	int pc;
	int h_max, v_max, size;
	int hblank_start, hsync_start, hsync_end, hblank_end;
	int vblank_start, vsync_start, vsync_end, vblank_end;
} GENSYNC;

static GENSYNC gensync;

static int gensync_icount;

/*
 * Call this function with an ponter to an array of ten ints:
 * The horizontal and vertical maximum values, the horizontal
 * blanking start, sync start, sync end and blanking end followed
 * by the vertical blanking start, sync start, sync end counter values.
 * In your machine driver add a pointer to an array like:
 *		int video[] = {454,262, 0,32,64,80, 0,4,8,16 };
 * for the reset_param to the CPU_GENSYNC entry.
 */
static void gensync_reset(void)
{
	gensync.pc = 0;
}

static void gensync_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const int *video = config;

	gensync.h_max = video[0];
	gensync.v_max = video[1];
	gensync.size = gensync.h_max * gensync.v_max;
	gensync.hblank_start = video[2];
	gensync.hsync_start = video[3];
	gensync.hsync_end = video[4];
	gensync.hblank_end = video[5];
	gensync.vblank_start = video[6];
	gensync.vsync_start = video[7];
	gensync.vsync_end = video[8];
	gensync.vblank_end = video[9];
}

static void gensync_exit(void)
{
}

static int gensync_execute(int cycles)
{
	gensync_icount = cycles;

#ifdef  MAME_DEBUG
	do
	{
		CALL_MAME_DEBUG;
		if (++gensync.pc == gensync.size)
			gensync.pc = 0;
	} while (--gensync_icount > 0);
#else
	gensync.pc += gensync_icount;
	gensync_icount = 0;
#endif

	return cycles - gensync_icount;
}

static void gensync_get_context(void *reg)
{
	if (reg)
		*(GENSYNC *)reg = gensync;
}

static void gensync_set_context(void *reg)
{
	if (reg)
		gensync = *(GENSYNC *)reg;
}

static offs_t gensync_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
#ifdef MAME_DEBUG
	return gensyncd(buffer, pc);
#else
	sprintf(buffer, "%3d", pc);
	return 1;
#endif
}

/* SU 078u2 */

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void gensync_set_info(UINT32 state, union cpuinfo *info)
{
	int h = gensync.pc % gensync.h_max;
	int v = (gensync.pc / gensync.h_max) % gensync.v_max;

	switch (state)
	{
	/* --- the following bits of info are set as 64-bit signed integers --- */
	case CPUINFO_INT_REGISTER + GS_PC:			gensync.pc 		= info->i % gensync.size;	break;
	case CPUINFO_INT_REGISTER + GS_H:			h		   	= info->i % gensync.h_max;	break;
	case CPUINFO_INT_REGISTER + GS_V:			v		   	= info->i % gensync.v_max;	break;
	case CPUINFO_INT_REGISTER + GS_HBLANK_START:		gensync.hblank_start	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_HSYNC_START:		gensync.hsync_start	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_HSYNC_END:		gensync.hsync_end	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_HBLANK_END:		gensync.hblank_end	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_VBLANK_START:		gensync.vblank_start	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_VSYNC_START:		gensync.vsync_start	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_VSYNC_END:		gensync.vsync_end	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_VBLANK_END:		gensync.vblank_end	= info->i;			break;
	case CPUINFO_INT_REGISTER + GS_1H:			h 		= (h & ~0x001) | ((info->i & 1) << 0);	break;
	case CPUINFO_INT_REGISTER + GS_2H:			h 		= (h & ~0x002) | ((info->i & 1) << 1);	break;
	case CPUINFO_INT_REGISTER + GS_4H:			h 		= (h & ~0x004) | ((info->i & 1) << 2);	break;
	case CPUINFO_INT_REGISTER + GS_8H:			h 		= (h & ~0x008) | ((info->i & 1) << 3);	break;
	case CPUINFO_INT_REGISTER + GS_16H:			h 		= (h & ~0x010) | ((info->i & 1) << 4);	break;
	case CPUINFO_INT_REGISTER + GS_32H:			h 		= (h & ~0x020) | ((info->i & 1) << 5);	break;
	case CPUINFO_INT_REGISTER + GS_64H:			h 		= (h & ~0x040) | ((info->i & 1) << 6);	break;
	case CPUINFO_INT_REGISTER + GS_128H:			h 		= (h & ~0x080) | ((info->i & 1) << 7);	break;
	case CPUINFO_INT_REGISTER + GS_256H:			h 		= (h & ~0x100) | ((info->i & 1) << 8);	break;
	case CPUINFO_INT_REGISTER + GS_512H:			h 		= (h & ~0x200) | ((info->i & 1) << 9);	break;
	case CPUINFO_INT_REGISTER + GS_1V:			v 		= (v & ~0x001) | ((info->i & 1) << 0);	break;
	case CPUINFO_INT_REGISTER + GS_2V:			v 		= (v & ~0x002) | ((info->i & 1) << 1);	break;
	case CPUINFO_INT_REGISTER + GS_4V:			v 		= (v & ~0x004) | ((info->i & 1) << 2);	break;
	case CPUINFO_INT_REGISTER + GS_8V:			v 		= (v & ~0x008) | ((info->i & 1) << 3);	break;
	case CPUINFO_INT_REGISTER + GS_16V:			v 		= (v & ~0x010) | ((info->i & 1) << 4);	break;
	case CPUINFO_INT_REGISTER + GS_32V:			v 		= (v & ~0x020) | ((info->i & 1) << 5);	break;
	case CPUINFO_INT_REGISTER + GS_64V:			v 		= (v & ~0x040) | ((info->i & 1) << 6);	break;
	case CPUINFO_INT_REGISTER + GS_128V:			v 		= (v & ~0x080) | ((info->i & 1) << 7);	break;
	case CPUINFO_INT_REGISTER + GS_256V:			v 		= (v & ~0x100) | ((info->i & 1) << 8);	break;
	case CPUINFO_INT_REGISTER + GS_512V:			v 		= (v & ~0x200) | ((info->i & 1) << 9);	break;

	case CPUINFO_INT_PC:					gensync.pc 		= info->i % (gensync.h_max * gensync.v_max);	break;
	case CPUINFO_INT_SP:					;							break;

	/* --- the following bits of info are set as pointers to data or functions --- */
	}

	gensync.pc = v * gensync.h_max + h;
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

void gensync_get_info(UINT32 state, union cpuinfo *info)
{
	int h, v;

	if  (gensync.h_max > 0)
	{
		h = gensync.pc % gensync.h_max;
		v = (gensync.pc / gensync.h_max) % gensync.v_max;
	}
	else /* Machine has not been reset yet so we fill in hardcoded values */
	{
		h = gensync.pc % 454;
		v = (gensync.pc / 454) % 261;
	}

	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(gensync);			break;
	case CPUINFO_INT_INPUT_LINES:				info->i = 0;					break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;					break;
	case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_LE;				break;
	case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;					break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;					break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 1;					break;
	case CPUINFO_INT_MIN_CYCLES:				info->i = 1;					break;
	case CPUINFO_INT_MAX_CYCLES:				info->i = 1;					break;

	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
	case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 	info->i = 0;					break;
	case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 	info->i = 0;					break;

	case CPUINFO_INT_PREVIOUSPC:				info->i = 0;	/* not implemented */		break;
	case CPUINFO_INT_PC:					info->i = gensync.pc; 				break;
	case CPUINFO_INT_SP:					info->i = 0;					break;

	case CPUINFO_INT_REGISTER + GS_H:			info->i = h;					break;
	case CPUINFO_INT_REGISTER + GS_V:			info->i = v;					break;
	case CPUINFO_INT_REGISTER + GS_MAX_H:			info->i = gensync.h_max;			break;
	case CPUINFO_INT_REGISTER + GS_MAX_V:			info->i = gensync.v_max;			break;
	case CPUINFO_INT_REGISTER + GS_X:			info->i = gensync.hblank_start < gensync.hblank_end ?
										     ((h >= gensync.hblank_start && h < gensync.hblank_end)?
										       -1 : h - gensync.hblank_end):
											  ((h >= gensync.hblank_start || h < gensync.hblank_end)?
											    -1 : h - gensync.hblank_start);		break;
	case CPUINFO_INT_REGISTER + GS_Y:			info->i =  gensync.vblank_start < gensync.vblank_end ?
										     ((v >= gensync.vblank_start && v < gensync.vblank_end)?
										      -1 : v - gensync.vblank_end):
											((v >= gensync.vblank_start || v < gensync.vblank_end)?
											  -1 : v - gensync.hblank_start);		break;
	case CPUINFO_INT_REGISTER + GS_HBLANK_START:		info->i =  gensync.hblank_start;		break;
	case CPUINFO_INT_REGISTER + GS_HSYNC_START:		info->i =  gensync.hsync_start;			break;
	case CPUINFO_INT_REGISTER + GS_HSYNC_END:		info->i =  gensync.hsync_end;			break;
	case CPUINFO_INT_REGISTER + GS_HBLANK_END:		info->i =  gensync.hblank_end;			break;
	case CPUINFO_INT_REGISTER + GS_VBLANK_START:		info->i =  gensync.vblank_start;		break;
	case CPUINFO_INT_REGISTER + GS_VSYNC_START:		info->i =  gensync.vsync_start;			break;
	case CPUINFO_INT_REGISTER + GS_VSYNC_END:		info->i =  gensync.vsync_end;			break;
	case CPUINFO_INT_REGISTER + GS_VBLANK_END:		info->i =  gensync.vblank_end;			break;
	case CPUINFO_INT_REGISTER + GS_HBLANK:			info->i =  gensync.hblank_start < gensync.hblank_end ?
										  h >= gensync.hblank_start && h < gensync.hblank_end:
										  h >= gensync.hblank_start || h < gensync.hblank_end;	break;
	case CPUINFO_INT_REGISTER + GS_HSYNC:			info->i =  gensync.hsync_start < gensync.hsync_end ?
										  h >= gensync.hsync_start && h < gensync.hsync_end:
										  h >= gensync.hsync_start || h < gensync.hsync_end;	break;
	case CPUINFO_INT_REGISTER + GS_VBLANK:			info->i =  gensync.vblank_start < gensync.vblank_end ?
										  v >= gensync.vblank_start && v < gensync.vblank_end:
										  v >= gensync.vblank_start || v < gensync.vblank_end;	break;
	case CPUINFO_INT_REGISTER + GS_VSYNC:			info->i =  gensync.vsync_start < gensync.vsync_end ?
										  v >= gensync.vsync_start && v < gensync.vsync_end:
										  v >= gensync.vsync_start || v < gensync.vsync_end;	break;
	case CPUINFO_INT_REGISTER + GS_1H:			info->i =  (h >> 0) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_2H:			info->i =  (h >> 1) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_4H:			info->i =  (h >> 2) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_8H:			info->i =  (h >> 3) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_16H:			info->i =  (h >> 4) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_32H:			info->i =  (h >> 5) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_64H:			info->i =  (h >> 6) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_128H:			info->i =  (h >> 7) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_256H:			info->i =  (h >> 8) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_512H:			info->i =  (h >> 9) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_1V:			info->i =  (v >> 0) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_2V:			info->i =  (v >> 1) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_4V:			info->i =  (v >> 2) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_8V:			info->i =  (v >> 3) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_16V:			info->i =  (v >> 4) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_32V:			info->i =  (v >> 5) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_64V:			info->i =  (v >> 6) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_128V:			info->i =  (v >> 7) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_256V:			info->i =  (v >> 8) & 1;			break;
	case CPUINFO_INT_REGISTER + GS_512V:			info->i =  (v >> 9) & 1;			break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_PTR_SET_INFO:				info->setinfo = gensync_set_info;		break;
	case CPUINFO_PTR_GET_CONTEXT:				info->getcontext = gensync_get_context;		break;
	case CPUINFO_PTR_SET_CONTEXT:				info->setcontext = gensync_set_context;		break;
	case CPUINFO_PTR_INIT:					info->init = gensync_init;			break;
	case CPUINFO_PTR_RESET:					info->reset = gensync_reset;			break;
	case CPUINFO_PTR_EXIT:					info->exit = gensync_exit;			break;
	case CPUINFO_PTR_EXECUTE:				info->execute = gensync_execute;		break;
	case CPUINFO_PTR_BURN:					info->burn = NULL;				break;
	case CPUINFO_PTR_DISASSEMBLE:				info->disassemble = gensync_dasm;		break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &gensync_icount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:					strcpy(info->s = cpuintrf_temp_str(), "GENSYNC"); break;
	case CPUINFO_STR_CORE_FAMILY:				strcpy(info->s = cpuintrf_temp_str(), "GENSYNC generic video synchronization"); break;
	case CPUINFO_STR_CORE_VERSION:				strcpy(info->s = cpuintrf_temp_str(), "0.2"); break;
	case CPUINFO_STR_CORE_FILE:				strcpy(info->s = cpuintrf_temp_str(), __FILE__); break;
	case CPUINFO_STR_CORE_CREDITS:				strcpy(info->s = cpuintrf_temp_str(), "Copyright (c) 1999, The MAMEDEV team."); break;


	case CPUINFO_STR_FLAGS:					sprintf(info->s = cpuintrf_temp_str(), 	 "%4d:%4d", 	v, h); 			break;

	case CPUINFO_STR_REGISTER + GS_PC: 			sprintf(info->s = cpuintrf_temp_str(),  "PC:%03X:%03X",h, v);  		break;
	case CPUINFO_STR_REGISTER + GS_HBLANK_START: 		sprintf(info->s = cpuintrf_temp_str(),  "HBS:%03X", 	gensync.hblank_start); 	break;
	case CPUINFO_STR_REGISTER + GS_HSYNC_START: 		sprintf(info->s = cpuintrf_temp_str(),  "HSS:%03X", 	gensync.hsync_start);  	break;
	case CPUINFO_STR_REGISTER + GS_HSYNC_END: 		sprintf(info->s = cpuintrf_temp_str(),  "HSE:%03X", 	gensync.hsync_end);  	break;
	case CPUINFO_STR_REGISTER + GS_HBLANK_END: 		sprintf(info->s = cpuintrf_temp_str(),  "HBE:%03X", 	gensync.hblank_end);  	break;
	case CPUINFO_STR_REGISTER + GS_VBLANK_START: 		sprintf(info->s = cpuintrf_temp_str(),  "VBS:%03X", 	gensync.hblank_start); 	break;
	case CPUINFO_STR_REGISTER + GS_VSYNC_START: 		sprintf(info->s = cpuintrf_temp_str(),  "VSS:%03X", 	gensync.hsync_start); 	break;
	case CPUINFO_STR_REGISTER + GS_VSYNC_END: 		sprintf(info->s = cpuintrf_temp_str(),  "VSE:%03X", 	gensync.hsync_end);  	break;
	case CPUINFO_STR_REGISTER + GS_VBLANK_END: 		sprintf(info->s = cpuintrf_temp_str(),  "VBE:%03X", 	gensync.hblank_end);  	break;
	case CPUINFO_STR_REGISTER + GS_1H:			sprintf(info->s = cpuintrf_temp_str(),  "1H:%X", 	(h >> 0) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_2H: 			sprintf(info->s = cpuintrf_temp_str(),  "2H:%X", 	(h >> 1) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_4H: 			sprintf(info->s = cpuintrf_temp_str(),  "4H:%X", 	(h >> 2) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_8H: 			sprintf(info->s = cpuintrf_temp_str(),  "8H:%X", 	(h >> 3) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_16H: 			sprintf(info->s = cpuintrf_temp_str(),  "16H:%X", 	(h >> 4) & 1); 		break;
	case CPUINFO_STR_REGISTER + GS_32H: 			sprintf(info->s = cpuintrf_temp_str(),  "32H:%X", 	(h >> 5) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_64H: 			sprintf(info->s = cpuintrf_temp_str(),  "64H:%X", 	(h >> 6) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_128H: 			sprintf(info->s = cpuintrf_temp_str(),  "128H:%X",	(h >> 7) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_256H: 			sprintf(info->s = cpuintrf_temp_str(),  "256H:%X", 	(h >> 8) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_512H: 			sprintf(info->s = cpuintrf_temp_str(),  "512H:%X", 	(h >> 9) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_1V: 			sprintf(info->s = cpuintrf_temp_str(),  "1V:%X", 	(v >> 0) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_2V: 			sprintf(info->s = cpuintrf_temp_str(),  "2V:%X", 	(v >> 1) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_4V: 			sprintf(info->s = cpuintrf_temp_str(),  "4V:%X", 	(v >> 2) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_8V: 			sprintf(info->s = cpuintrf_temp_str(),  "8V:%X", 	(v >> 3) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_16V:			sprintf(info->s = cpuintrf_temp_str(),  "16V:%X", 	(v >> 4) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_32V: 			sprintf(info->s = cpuintrf_temp_str(),  "32V:%X", 	(v >> 5) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_64V: 			sprintf(info->s = cpuintrf_temp_str(),  "64V:%X", 	(v >> 6) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_128V: 			sprintf(info->s = cpuintrf_temp_str(),  "128V:%X", 	(v >> 7) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_256V: 			sprintf(info->s = cpuintrf_temp_str(),  "256V:%X", 	(v >> 8) & 1);  	break;
	case CPUINFO_STR_REGISTER + GS_512V: 			sprintf(info->s = cpuintrf_temp_str(),  "512V:%X", 	(v >> 9) & 1);  	break;
	}
}
/* SU 078u2 */
