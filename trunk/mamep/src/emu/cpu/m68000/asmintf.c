/*
	Interface routine for 68kem <-> Mame
*/

#include "driver.h"
#include "m68000.h"

struct m68k_memory_interface a68k_memory_intf;

// If we are only using assembler cores, we need to define these
// otherwise they are declared by the C core.

#if 0 //ks hcmame s switch m68k core
#if HAS_M68000
#undef A68K0
#define A68K0
#endif

#if HAS_M68010
#undef A68K1
#define A68K1
#endif

#if HAS_M68020
#undef A68K2
#define A68K2
#endif
#endif //ks hcmame e switch m68k core


#if 0 //ks hcmame s switch m68k core
#if defined(A68K0) || defined(A68K1) || defined(A68K2)
int m68k_ICount;
struct m68k_memory_interface m68k_memory_intf;
offs_t m68k_encrypted_opcode_start[MAX_CPU];
offs_t m68k_encrypted_opcode_end[MAX_CPU];

void m68k_set_encrypted_opcode_range(int cpunum, offs_t start, offs_t end)
{
	m68k_encrypted_opcode_start[cpunum] = start;
	m68k_encrypted_opcode_end[cpunum] = end;
}
#endif
#endif //ks hcmame e switch m68k core

enum
{
	M68K_CPU_TYPE_INVALID,
	M68K_CPU_TYPE_68000,
	M68K_CPU_TYPE_68008,
	M68K_CPU_TYPE_68010,
	M68K_CPU_TYPE_68EC020,
	M68K_CPU_TYPE_68020,
	M68K_CPU_TYPE_68030,	/* Supported by disassembler ONLY */
	M68K_CPU_TYPE_68040		/* Supported by disassembler ONLY */
};

#define A68K_SET_PC_CALLBACK(A)     change_pc(A)

int illegal_op = 0 ;
int illegal_pc = 0 ;

UINT32 mem_amask;

#ifdef ENABLE_DEBUGGER
void m68k_illegal_opcode(void)
{
	logerror("Illegal Opcode %4x at %8x\n",illegal_op,illegal_pc);
}
#endif

unsigned int m68k_disassemble(char* str_buff, unsigned int pc, unsigned int cpu_type);

#ifdef _WIN32
#define CONVENTION __cdecl
#else
#define CONVENTION
#endif

/* Use the x86 assembly core */
typedef struct
{
	UINT32 d[8];             /* 0x0004 8 Data registers */
	UINT32 a[8];             /* 0x0024 8 Address registers */

	UINT32 isp;              /* 0x0048 */

	UINT32 sr_high;          /* 0x004C System registers */
	UINT32 ccr;              /* 0x0050 CCR in Intel Format */
	UINT32 x_carry;          /* 0x0054 Extended Carry */

	UINT32 pc;               /* 0x0058 Program Counter */

	UINT32 IRQ_level;        /* 0x005C IRQ level you want the MC68K process (0=None)  */
	UINT32 IRQ_cycles;       /* 0x0060 Interrupt cycles */

	/* Backward compatible with C emulator - Only set in Debug compile */

	UINT16 sr;
	UINT16 filler;

	int (*irq_callback)(int irqline);

	UINT32 previous_pc;      /* last PC used */

	void (*reset_callback)(void);
	void (*cmpild_instr_callback)(unsigned int, int);
	void (*rte_instr_callback)(void);
	int (*tas_instr_callback)(void);

	UINT32 sfc;              /* Source Function Code. (68010) */
	UINT32 dfc;              /* Destination Function Code. (68010) */
	UINT32 usp;              /* User Stack (All) */
	UINT32 vbr;              /* Vector Base Register. (68010) */

	UINT32 BankID;			 /* Memory bank in use */
	UINT32 CPUtype;		  	 /* CPU Type 0=68000,1=68010,2=68020 */
	UINT32 FullPC;
#ifdef ENABLE_DEBUGGER
	UINT32 asm68k_debug;     /* single step flag for debugger */
#endif
	struct m68k_memory_interface Memory_Interface;

} a68k_cpu_context;


#ifdef A68K0
extern a68k_cpu_context M68000_regs;

extern void CONVENTION M68000_RUN(void);
extern void CONVENTION M68000_RESET(void);

#endif

#ifdef A68K1
extern a68k_cpu_context M68010_regs;

extern void CONVENTION M68010_RUN(void);
extern void CONVENTION M68010_RESET(void);
#endif

#ifdef A68K2
extern a68k_cpu_context M68020_regs;

extern void CONVENTION M68020_RUN(void);
extern void CONVENTION M68020_RESET(void);
#endif

#ifdef ENABLE_DEBUGGER
extern unsigned int m68k_disassemble_raw(char* str_buff, unsigned int pc, const unsigned char* opdata, const unsigned char* argdata, unsigned int cpu_type);

#endif


/***************************************************************************/
/* Save State stuff                                                        */
/***************************************************************************/

#define state_save_register_UINT32(_mod,_inst,_name,_val,_count) \
	state_save_register_generic(_mod, _inst, _name, _val, UINT32, _count)
#define state_save_register_UINT16(_mod,_inst,_name,_val,_count) \
	state_save_register_generic(_mod, _inst, _name, _val, UINT16, _count)
#define state_save_register_UINT8(_mod,_inst,_name,_val,_count) \
	state_save_register_generic(_mod, _inst, _name, _val, UINT8, _count)

static int IntelFlag[32] = {
	0x0000,0x0001,0x0800,0x0801,0x0040,0x0041,0x0840,0x0841,
	0x0080,0x0081,0x0880,0x0881,0x00C0,0x00C1,0x08C0,0x08C1,
	0x0100,0x0101,0x0900,0x0901,0x0140,0x0141,0x0940,0x0941,
	0x0180,0x0181,0x0980,0x0981,0x01C0,0x01C1,0x09C0,0x09C1
};


// The assembler engine only keeps flags in intel format, so ...

static UINT32 dar[16];
static UINT32 zero = 0;
static UINT8 stopped = 0;
static UINT8 halted = 0;

#ifdef A68K0
static void m68000_prepare_substate(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		dar[i] = M68000_regs.d[i];
		dar[i + 8] = M68000_regs.a[i];
	}

	stopped = ((M68000_regs.IRQ_level & 0x80) != 0);
	halted = 0;
	zero = 0;

	M68000_regs.sr = ((M68000_regs.ccr >> 4) & 0x1C)
	               | (M68000_regs.ccr & 0x01)
	               | ((M68000_regs.ccr >> 10) & 0x02)
	               | (M68000_regs.sr_high << 8);
}

static void m68000_post_load(void)
{
	int intel = M68000_regs.sr & 0x1f;
	int i;

	for (i = 0; i < 8; i++)
	{
		M68000_regs.d[i] = dar[i];
		M68000_regs.a[i] = dar[i + 8];
	}

	M68000_regs.sr_high = M68000_regs.sr >> 8;
	M68000_regs.x_carry = (IntelFlag[intel] >> 8) & 0x01;
	M68000_regs.ccr     = IntelFlag[intel] & 0x0EFF;
}

void m68000_state_register(const char *type, int index)
{
	state_save_register_UINT32(type, index, "m68ki_cpu.dar",         &dar, 16);
	state_save_register_UINT32(type, index, "REG_PPC",               &M68000_regs.previous_pc, 1);
	state_save_register_UINT32(type, index, "REG_PC",                &M68000_regs.pc, 1);
	state_save_register_UINT32(type, index, "REG_USP",               &M68000_regs.usp, 1);
	state_save_register_UINT32(type, index, "REG_ISP",               &M68000_regs.isp, 1);
	state_save_register_UINT32(type, index, "REG_MSP",               &zero, 1);
	state_save_register_UINT32(type, index, "REG_VBR",               &M68000_regs.vbr, 1);
	state_save_register_UINT32(type, index, "REG_SFC",               &M68000_regs.sfc, 1);
	state_save_register_UINT32(type, index, "REG_DFC",               &M68000_regs.dfc, 1);
	state_save_register_UINT32(type, index, "REG_CACR",              &zero, 1);
	state_save_register_UINT32(type, index, "REG_CAAR",              &zero, 1);
	state_save_register_UINT16(type, index, "m68k_substate.sr",      &M68000_regs.sr, 1);
	state_save_register_UINT32(type, index, "CPU_INT_LEVEL",         &M68000_regs.IRQ_level, 1);
	state_save_register_UINT32(type, index, "CPU_INT_CYCLES",        &M68000_regs.IRQ_cycles, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.stopped", &stopped, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.halted",  &halted, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_ADDR",         &zero, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_DATA",         &zero, 1);
	state_save_register_func_presave(m68000_prepare_substate);
	state_save_register_func_postload(m68000_post_load);
}
#endif

#ifdef A68K1
static void m68010_prepare_substate(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		dar[i] = M68010_regs.d[i];
		dar[i + 8] = M68010_regs.a[i];
	}

	stopped = ((M68010_regs.IRQ_level & 0x80) != 0);
	halted = 0;
	zero = 0;

	M68010_regs.sr = ((M68010_regs.ccr >> 4) & 0x1C)
	               | (M68010_regs.ccr & 0x01)
	               | ((M68010_regs.ccr >> 10) & 0x02)
	               | (M68010_regs.sr_high << 8);
}

static void m68010_post_load(void)
{
	int intel = M68010_regs.sr & 0x1f;
	int i;

	for (i = 0; i < 8; i++)
	{
		M68010_regs.d[i] = dar[i];
		M68010_regs.a[i] = dar[i + 8];
	}

	M68010_regs.sr_high = M68010_regs.sr >> 8;
	M68010_regs.x_carry = (IntelFlag[intel] >> 8) & 0x01;
	M68010_regs.ccr     = IntelFlag[intel] & 0x0EFF;
}

void m68010_state_register(const char *type, int index)
{
	state_save_register_UINT32(type, index, "m68ki_cpu.dar",         &dar, 16);
	state_save_register_UINT32(type, index, "REG_PPC",               &M68010_regs.previous_pc, 1);
	state_save_register_UINT32(type, index, "REG_PC",                &M68010_regs.pc, 1);
	state_save_register_UINT32(type, index, "REG_USP",               &M68010_regs.usp, 1);
	state_save_register_UINT32(type, index, "REG_ISP",               &M68010_regs.isp, 1);
	state_save_register_UINT32(type, index, "REG_MSP",               &zero, 1);
	state_save_register_UINT32(type, index, "REG_VBR",               &M68010_regs.vbr, 1);
	state_save_register_UINT32(type, index, "REG_SFC",               &M68010_regs.sfc, 1);
	state_save_register_UINT32(type, index, "REG_DFC",               &M68010_regs.dfc, 1);
	state_save_register_UINT32(type, index, "REG_CACR",              &zero, 1);
	state_save_register_UINT32(type, index, "REG_CAAR",              &zero, 1);
	state_save_register_UINT16(type, index, "m68k_substate.sr",      &M68010_regs.sr, 1);
	state_save_register_UINT32(type, index, "CPU_INT_LEVEL",         &M68010_regs.IRQ_level, 1);
	state_save_register_UINT32(type, index, "CPU_INT_CYCLES",        &M68010_regs.IRQ_cycles, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.stopped", &stopped, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.halted",  &halted, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_ADDR",         &zero, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_DATA",         &zero, 1);
	state_save_register_func_presave(m68010_prepare_substate);
	state_save_register_func_postload(m68010_post_load);
}
#endif

#ifdef A68K2
static void m68020_prepare_substate(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		dar[i] = M68020_regs.d[i];
		dar[i + 8] = M68020_regs.a[i];
	}

	stopped = ((M68020_regs.IRQ_level & 0x80) != 0);
	halted = 0;
	zero = 0;

	M68020_regs.sr = ((M68020_regs.ccr >> 4) & 0x1C)
	               | (M68020_regs.ccr & 0x01)
	               | ((M68020_regs.ccr >> 10) & 0x02)
	               | (M68020_regs.sr_high << 8);
}

static void m68020_post_load(void)
{
	int intel = M68020_regs.sr & 0x1f;
	int i;

	for (i = 0; i < 8; i++)
	{
		M68020_regs.d[i] = dar[i];
		M68020_regs.a[i] = dar[i + 8];
	}

	M68020_regs.sr_high = M68020_regs.sr >> 8;
	M68020_regs.x_carry = (IntelFlag[intel] >> 8) & 0x01;
	M68020_regs.ccr     = IntelFlag[intel] & 0x0EFF;
}

void m68020_state_register(const char *type, int index)
{
	state_save_register_UINT32(type, index, "m68ki_cpu.dar",         &dar, 16);
	state_save_register_UINT32(type, index, "REG_PPC",               &M68020_regs.previous_pc, 1);
	state_save_register_UINT32(type, index, "REG_PC",                &M68020_regs.pc, 1);
	state_save_register_UINT32(type, index, "REG_USP",               &M68020_regs.usp, 1);
	state_save_register_UINT32(type, index, "REG_ISP",               &M68020_regs.isp, 1);
	state_save_register_UINT32(type, index, "REG_MSP",               &zero, 1);
	state_save_register_UINT32(type, index, "REG_VBR",               &M68020_regs.vbr, 1);
	state_save_register_UINT32(type, index, "REG_SFC",               &M68020_regs.sfc, 1);
	state_save_register_UINT32(type, index, "REG_DFC",               &M68020_regs.dfc, 1);
	state_save_register_UINT32(type, index, "REG_CACR",              &zero, 1);
	state_save_register_UINT32(type, index, "REG_CAAR",              &zero, 1);
	state_save_register_UINT16(type, index, "m68k_substate.sr",      &M68020_regs.sr, 1);
	state_save_register_UINT32(type, index, "CPU_INT_LEVEL",         &M68020_regs.IRQ_level, 1);
	state_save_register_UINT32(type, index, "CPU_INT_CYCLES",        &M68020_regs.IRQ_cycles, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.stopped", &stopped, 1);
	state_save_register_UINT8 (type, index, "m68k_substate.halted",  &halted, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_ADDR",         &zero, 1);
	state_save_register_UINT32(type, index, "CPU_PREF_DATA",         &zero, 1);
	state_save_register_func_presave(m68020_prepare_substate);
	state_save_register_func_postload(m68020_post_load);
}
#endif

static void change_pc_m68k(offs_t pc)
{
	change_pc(pc);
}


#if defined(A68K0) || defined(A68K1)

#if (HAS_M68008)
/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

static UINT16 m68008_read_immediate_16(offs_t address)
{
	offs_t addr = (address) ^ m68k_memory_intf.opcode_xor;
	return (cpu_readop(addr) << 8) | (cpu_readop(addr + 1));
}

static UINT16 readword_d8(offs_t address)
{
	UINT16 result = program_read_byte_8(address) << 8;
	return result | program_read_byte_8(address + 1);
}

static void writeword_d8(offs_t address, UINT16 data)
{
	program_write_byte_8(address, data >> 8);
	program_write_byte_8(address + 1, data);
}

static UINT32 readlong_d8(offs_t address)
{
	UINT32 result = program_read_byte_8(address) << 24;
	result |= program_read_byte_8(address + 1) << 16;
	result |= program_read_byte_8(address + 2) << 8;
	return result | program_read_byte_8(address + 3);
}

static void writelong_d8(offs_t address, UINT32 data)
{
	program_write_byte_8(address, data >> 24);
	program_write_byte_8(address + 1, data >> 16);
	program_write_byte_8(address + 2, data >> 8);
	program_write_byte_8(address + 3, data);
}

/* interface for 20/22-bit address bus, 8-bit data bus (68008) */
static const struct m68k_memory_interface interface_d8 =
{
	0,
	m68008_read_immediate_16,
	program_read_byte_8,
	readword_d8,
	readlong_d8,
	program_write_byte_8,
	writeword_d8,
	writelong_d8,
	change_pc_m68k,
	program_read_byte_8,			// Encrypted Versions
	readword_d8,
	readlong_d8,
	readword_d8,
	readlong_d8
};

#endif


/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

static UINT16 read_immediate_16(offs_t address)
{
	return cpu_readop16((address) ^ m68k_memory_intf.opcode_xor);
}

static UINT32 readlong_d16(offs_t address)
{
	UINT32 result = program_read_word_16be(address) << 16;
	return result | program_read_word_16be(address + 2);
}

static void writelong_d16(offs_t address, UINT32 data)
{
	program_write_word_16be(address, data >> 16);
	program_write_word_16be(address + 2, data);
}

/* interface for 24-bit address bus, 16-bit data bus (68000, 68010) */
static const struct m68k_memory_interface interface_d16 =
{
	0,
	read_immediate_16,
	program_read_byte_16be,
	program_read_word_16be,
	readlong_d16,
	program_write_byte_16be,
	program_write_word_16be,
	writelong_d16,
	change_pc_m68k,
	program_read_byte_16be,				// Encrypted Versions
	program_read_word_16be,
	readlong_d16,
	program_read_word_16be,
	readlong_d16
};

#endif // defined(A68K0) || defined(A68K1)

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

#ifdef A68K2

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT16 readword_d32(offs_t address)
{
	UINT16 result;

	if (!(address & 1))
		return program_read_word_32be(address);
	result = program_read_byte_32be(address) << 8;
	return result | program_read_byte_32be(address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writeword_d32(offs_t address, UINT16 data)
{
	if (!(address & 1))
	{
		program_write_word_32be(address, data);
		return;
	}
	program_write_byte_32be(address, data >> 8);
	program_write_byte_32be(address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT32 readlong_d32(offs_t address)
{
	UINT32 result;

	if (!(address & 3))
		return program_read_dword_32be(address);
	else if (!(address & 1))
	{
		result = program_read_word_32be(address) << 16;
		return result | program_read_word_32be(address + 2);
	}
	result = program_read_byte_32be(address) << 24;
	result |= program_read_word_32be(address + 1) << 8;
	return result | program_read_byte_32be(address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writelong_d32(offs_t address, UINT32 data)
{
	if (!(address & 3))
	{
		program_write_dword_32be(address, data);
		return;
	}
	else if (!(address & 1))
	{
		program_write_word_32be(address, data >> 16);
		program_write_word_32be(address + 2, data);
		return;
	}
	program_write_byte_32be(address, data >> 24);
	program_write_word_32be(address + 1, data >> 8);
	program_write_byte_32be(address + 3, data);
}

/* interface for 32-bit data bus (68EC020, 68020) */
static const struct m68k_memory_interface interface_d32 =
{
	WORD_XOR_BE(0),
	read_immediate_16,
	program_read_byte_32be,
	readword_d32,
	readlong_d32,
	program_write_byte_32be,
	writeword_d32,
	writelong_d32,
	change_pc_m68k,
	program_read_byte_32be,				// Encrypted Versions
	program_read_word_32be,
	readlong_d32,
	program_read_word_32be,
	readlong_d32
};


/* global access */
struct m68k_memory_interface m68k_memory_intf;

#endif // A68K2


/********************************************/
/* Interface routines to link Mame -> 68KEM */
/********************************************/

#define READOP(a)	(cpu_readop16((a) ^ a68k_memory_intf.opcode_xor))

#ifdef A68K0

static void m68000_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const struct m68k_encryption_interface *interface = config;

	// Default Memory Routines
	M68000_regs.Memory_Interface = interface_d16;

	// Import encryption routines if present
	if (interface)
	{
		M68000_regs.Memory_Interface.read8pc = interface->read8pc;
		M68000_regs.Memory_Interface.read16pc = interface->read16pc;
		M68000_regs.Memory_Interface.read32pc = interface->read32pc;
		M68000_regs.Memory_Interface.read16d = interface->read16d;
		M68000_regs.Memory_Interface.read32d = interface->read32d;
	}

	m68000_state_register("m68000", index);
	M68000_regs.reset_callback = 0;
	M68000_regs.cmpild_instr_callback = 0;
	M68000_regs.rte_instr_callback = 0;
	M68000_regs.tas_instr_callback = 0;
	M68000_regs.irq_callback = irqcallback;
}

static void m68000_reset(void)
{
	void (*resetc)(void);
	void (*cmpic)(unsigned int, int);
	void (*rtec)(void);
	int (*tasc)(void);
	int (*irqc)(int irqline);

	resetc = M68000_regs.reset_callback;
	cmpic = M68000_regs.cmpild_instr_callback;
	rtec = M68000_regs.rte_instr_callback;
	tasc = M68000_regs.tas_instr_callback;
	irqc = M68000_regs.irq_callback;

	a68k_memory_intf = M68000_regs.Memory_Interface;

	memset(&M68000_regs,0,sizeof(M68000_regs));

	M68000_regs.Memory_Interface = a68k_memory_intf;

	M68000_regs.reset_callback = resetc;
	M68000_regs.cmpild_instr_callback = cmpic;
	M68000_regs.rte_instr_callback = rtec;
	M68000_regs.tas_instr_callback = tasc;
	M68000_regs.irq_callback = irqc;

	M68000_regs.a[7] = M68000_regs.isp = (( READOP(0) << 16 ) | READOP(2));
	M68000_regs.pc   = (( READOP(4) << 16 ) | READOP(6)) & 0xffffff;
	M68000_regs.sr_high = 0x27;

	#ifdef ENABLE_DEBUGGER
		M68000_regs.sr = 0x2700;
	#endif

	M68000_RESET();

	a68k_memory_intf = M68000_regs.Memory_Interface;
	mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
}

static void m68000_exit(void)
{
	/* nothing to do ? */
}


#ifdef TRACE68K 							/* Trace */
	static int skiptrace=0;
#endif

static int m68000_execute(int cycles)
{
	if (M68000_regs.IRQ_level == 0x80) return cycles;		/* STOP with no IRQs */

	m68k_ICount = cycles;

#ifdef ENABLE_DEBUGGER
	do
	{
		M68000_regs.asm68k_debug = 0;

		if (Machine->debug_mode)
		{
			#ifdef TRACE68K

			int StartCycle = m68k_ICount;

			skiptrace++;

			if (skiptrace > 0)
			{
				int mycount, areg, dreg;

				areg = dreg = 0;
				for (mycount=7;mycount>=0;mycount--)
				{
					areg = areg + M68000_regs.a[mycount];
					dreg = dreg + M68000_regs.d[mycount];
				}

		   		logerror("=> %8x %8x ",areg,dreg);
				logerror("%6x %4x %d\n",M68000_regs.pc,M68000_regs.sr & 0x271F,m68k_ICount);
			}
			#endif

			m68k_ICount -= M68000_regs.IRQ_cycles;
			M68000_regs.IRQ_cycles = 0;
//			m68k_memory_intf = a68k_memory_intf;
			mame_debug_hook(M68000_regs.pc);

			if (Machine->debug_mode)
				M68000_regs.asm68k_debug = -1;

			M68000_RUN();

			#ifdef TRACE68K
			if ((M68000_regs.IRQ_level & 0x80) || (cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE)))
				m68k_ICount = 0;
			else
				m68k_ICount = StartCycle - 12;
			#endif
		}
		else
			M68000_RUN();

	} while (m68k_ICount > 0);

#else

	M68000_RUN();

#endif /* ENABLE_DEBUGGER */

	return (cycles - m68k_ICount);
}


static void m68000_get_context(void *dst)
{
	if( dst )
		*(a68k_cpu_context*)dst = M68000_regs;
}

static void m68000_set_context(void *src)
{
	if( src )
	{
		M68000_regs = *(a68k_cpu_context*)src;
		a68k_memory_intf = M68000_regs.Memory_Interface;
		mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
	}
}

static void m68k_assert_irq(int int_line)
{
	/* Save icount */
	int StartCount = m68k_ICount;

	M68000_regs.IRQ_level = int_line;

	/* Now check for Interrupt */

	m68k_ICount = -1;
	M68000_RUN();

	/* Restore Count */
	m68k_ICount = StartCount;
}

static void m68k_clear_irq(int int_line)
{
	M68000_regs.IRQ_level = 0;
}

static void m68000_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
		irqline = 7;
	switch(state)
	{
		case CLEAR_LINE:
			m68k_clear_irq(irqline);
			return;
		case ASSERT_LINE:
			m68k_assert_irq(irqline);
			return;
		default:
			m68k_assert_irq(irqline);
			return;
	}
}

static offs_t m68000_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	A68K_SET_PC_CALLBACK(pc);

#ifdef ENABLE_DEBUGGER
	m68k_memory_intf = a68k_memory_intf;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68000);
#else
	sprintf( buffer, "$%04X", (oprom[0] << 8) | oprom[1] );
	return 2;
#endif
}


#if (HAS_M68008)

static void m68008_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const struct m68k_encryption_interface *interface = config;

	// Default Memory Routines
	M68000_regs.Memory_Interface = interface_d8;

	// Import encryption routines if present
	if (interface)
	{
		M68000_regs.Memory_Interface.read8pc = interface->read8pc;
		M68000_regs.Memory_Interface.read16pc = interface->read16pc;
		M68000_regs.Memory_Interface.read32pc = interface->read32pc;
		M68000_regs.Memory_Interface.read16d = interface->read16d;
		M68000_regs.Memory_Interface.read32d = interface->read32d;
	}

	m68000_state_register("m68008", index);
	M68000_regs.reset_callback = 0;
	M68000_regs.cmpild_instr_callback = 0;
	M68000_regs.rte_instr_callback = 0;
	M68000_regs.tas_instr_callback = 0;
	M68000_regs.irq_callback = irqcallback;
}

static void m68008_reset(void)
{
	void (*resetc)(void);
	void (*cmpic)(unsigned int, int);
	void (*rtec)(void);
	int (*tasc)(void);
	int (*irqc)(int irqline);

	resetc = M68000_regs.reset_callback;
	cmpic = M68000_regs.cmpild_instr_callback;
	rtec = M68000_regs.rte_instr_callback;
	tasc = M68000_regs.tas_instr_callback;
	irqc = M68000_regs.irq_callback;

	a68k_memory_intf = M68000_regs.Memory_Interface;

	memset(&M68000_regs,0,sizeof(M68000_regs));

	M68000_regs.Memory_Interface = a68k_memory_intf;

	M68000_regs.reset_callback = resetc;
	M68000_regs.cmpild_instr_callback = cmpic;
	M68000_regs.rte_instr_callback = rtec;
	M68000_regs.tas_instr_callback = tasc;
	M68000_regs.irq_callback = irqc;

	M68000_regs.a[7] = M68000_regs.isp = (( READOP(0) << 16 ) | READOP(2));
	M68000_regs.pc   = (( READOP(4) << 16 ) | READOP(6)) & 0xffffff;
	M68000_regs.sr_high = 0x27;

	#ifdef ENABLE_DEBUGGER
		M68000_regs.sr = 0x2700;
	#endif

	M68000_RESET();

	a68k_memory_intf = M68000_regs.Memory_Interface;
	mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
}

static offs_t m68008_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	A68K_SET_PC_CALLBACK(pc);

#ifdef ENABLE_DEBUGGER
	m68k_memory_intf = a68k_memory_intf;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68008);
#else
	sprintf( buffer, "$%04X", (oprom[0] << 8) | oprom[1] );
	return 2;
#endif
}

#endif

#endif // A68K0


/****************************************************************************
 * M68010 section
 ****************************************************************************/

#ifdef A68K1

#if HAS_M68010

static void m68010_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const struct m68k_encryption_interface *interface = config;

	// Default Memory Routines
	M68010_regs.Memory_Interface = interface_d16;

	// Import encryption routines if present
	if (interface)
	{
		M68010_regs.Memory_Interface.read8pc = interface->read8pc;
		M68010_regs.Memory_Interface.read16pc = interface->read16pc;
		M68010_regs.Memory_Interface.read32pc = interface->read32pc;
		M68010_regs.Memory_Interface.read16d = interface->read16d;
		M68010_regs.Memory_Interface.read32d = interface->read32d;
	}

	m68010_state_register("m68010", index);
	M68010_regs.reset_callback = 0;
	M68010_regs.cmpild_instr_callback = 0;
	M68010_regs.rte_instr_callback = 0;
	M68010_regs.tas_instr_callback = 0;
	M68010_regs.irq_callback = irqcallback;
}

static void m68010_reset(void)
{
	void (*resetc)(void);
	void (*cmpic)(unsigned int, int);
	void (*rtec)(void);
	int (*tasc)(void);
	int (*irqc)(int irqline);

	resetc = M68010_regs.reset_callback;
	cmpic = M68010_regs.cmpild_instr_callback;
	rtec = M68010_regs.rte_instr_callback;
	tasc = M68010_regs.tas_instr_callback;
	irqc = M68010_regs.irq_callback;

	a68k_memory_intf = M68010_regs.Memory_Interface;

	memset(&M68010_regs,0,sizeof(M68010_regs));

	M68010_regs.Memory_Interface = a68k_memory_intf;

	M68010_regs.reset_callback = resetc;
	M68010_regs.cmpild_instr_callback = cmpic;
	M68010_regs.rte_instr_callback = rtec;
	M68010_regs.tas_instr_callback = tasc;
	M68010_regs.irq_callback = irqc;

	M68010_regs.a[7] = M68010_regs.isp = (( READOP(0) << 16 ) | READOP(2));
	M68010_regs.pc   = (( READOP(4) << 16 ) | READOP(6)) & 0xffffff;
	M68010_regs.sr_high = 0x27;

	#ifdef ENABLE_DEBUGGER
		M68010_regs.sr = 0x2700;
	#endif

	M68010_RESET();

	a68k_memory_intf = M68010_regs.Memory_Interface;
	mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
}

static void m68010_exit(void)
{
	/* nothing to do ? */
}

static int m68010_execute(int cycles)
{
	if (M68010_regs.IRQ_level == 0x80) return cycles;		/* STOP with no IRQs */

	m68k_ICount = cycles;

#ifdef ENABLE_DEBUGGER
	do
	{
		M68010_regs.asm68k_debug = 0;

		if (Machine->debug_mode)
		{
			#ifdef TRACE68K

			int StartCycle = m68k_ICount;

			skiptrace++;

			if (skiptrace > 0)
			{
				int mycount, areg, dreg;

				areg = dreg = 0;
				for (mycount=7;mycount>=0;mycount--)
				{
					areg = areg + M68010_regs.a[mycount];
					dreg = dreg + M68010_regs.d[mycount];
				}

		   		logerror("=> %8x %8x ",areg,dreg);
				logerror("%6x %4x %d\n",M68010_regs.pc,M68010_regs.sr & 0x271F,m68k_ICount);
			}
			#endif

			m68k_ICount -= M68010_regs.IRQ_cycles;
			M68010_regs.IRQ_cycles = 0;
//			m68k_memory_intf = a68k_memory_intf;
			mame_debug_hook(M68010_regs.pc);

			if (Machine->debug_mode)
				M68010_regs.asm68k_debug = -1;

			M68010_RUN();

			#ifdef TRACE68K
			if ((M68010_regs.IRQ_level & 0x80) || (cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE)))
				m68k_ICount = 0;
			else
				m68k_ICount = StartCycle - 12;
			#endif
		}
		else
			M68010_RUN();
	} while (m68k_ICount > 0);

#else

	M68010_RUN();

#endif /* ENABLE_DEBUGGER */

	return (cycles - m68k_ICount);
}

static void m68010_get_context(void *dst)
{
	if( dst )
		*(a68k_cpu_context*)dst = M68010_regs;
}

static void m68010_set_context(void *src)
{
	if( src )
	{
		M68010_regs = *(a68k_cpu_context*)src;
		a68k_memory_intf = M68010_regs.Memory_Interface;
		mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
	}
}

static void m68010_assert_irq(int int_line)
{
	/* Save icount */
	int StartCount = m68k_ICount;
	M68010_regs.IRQ_level = int_line;

	/* Now check for Interrupt */

	m68k_ICount = -1;
	M68010_RUN();

	/* Restore Count */
	m68k_ICount = StartCount;
}

static void m68010_clear_irq(int int_line)
{
	M68010_regs.IRQ_level = 0;
}

static void m68010_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
		irqline = 7;
	switch(state)
	{
		case CLEAR_LINE:
			m68010_clear_irq(irqline);
			return;
		case ASSERT_LINE:
			m68010_assert_irq(irqline);
			return;
		default:
			m68010_assert_irq(irqline);
			return;
	}
}

static offs_t m68010_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	A68K_SET_PC_CALLBACK(pc);

#ifdef ENABLE_DEBUGGER
	m68k_memory_intf = a68k_memory_intf;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68010);
#else
	sprintf( buffer, "$%04X", (oprom[0] << 8) | oprom[1] );
	return 2;
#endif
}

#endif

#endif // A68K1


/****************************************************************************
 * M68020 section
 ****************************************************************************/

#ifdef A68K2

#if HAS_M68020

static void m68020_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68020_state_register("m68020", index);
	M68020_regs.reset_callback = 0;
	M68020_regs.cmpild_instr_callback = 0;
	M68020_regs.rte_instr_callback = 0;
	M68020_regs.tas_instr_callback = 0;
	M68020_regs.irq_callback = irqcallback;
}

static void m68k32_reset_common(void)
{
	void (*resetc)(void);
	void (*cmpic)(unsigned int, int);
	void (*rtec)(void);
	int (*tasc)(void);
	int (*irqc)(int irqline);

	resetc = M68020_regs.reset_callback;
	cmpic = M68020_regs.cmpild_instr_callback;
	rtec = M68020_regs.rte_instr_callback;
	tasc = M68020_regs.tas_instr_callback;
	irqc = M68020_regs.irq_callback;

	memset(&M68020_regs,0,sizeof(M68020_regs));

	M68020_regs.reset_callback = resetc;
	M68020_regs.cmpild_instr_callback = cmpic;
	M68020_regs.rte_instr_callback = rtec;
	M68020_regs.tas_instr_callback = tasc;
	M68020_regs.irq_callback = irqc;

	M68020_regs.a[7] = M68020_regs.isp = (( READOP(0) << 16 ) | READOP(2));
	M68020_regs.pc   = (( READOP(4) << 16 ) | READOP(6)) & 0xffffff;
	M68020_regs.sr_high = 0x27;

	#ifdef ENABLE_DEBUGGER
		M68020_regs.sr = 0x2700;
	#endif

	M68020_RESET();
}

static void m68020_reset(void)
{
	a68k_memory_intf = interface_d32;
	mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;

	m68k32_reset_common();

	M68020_regs.CPUtype=2;
	M68020_regs.Memory_Interface = a68k_memory_intf;
}

static void m68020_exit(void)
{
	/* nothing to do ? */
}

static int m68020_execute(int cycles)
{
	if (M68020_regs.IRQ_level == 0x80) return cycles;		/* STOP with no IRQs */

	m68k_ICount = cycles;

#ifdef ENABLE_DEBUGGER
	do
	{
		M68020_regs.asm68k_debug = 0;

		if (Machine->debug_mode)
		{
			#ifdef TRACE68K

			int StartCycle = m68k_ICount;

			skiptrace++;

			if (skiptrace > 0)
			{
				int mycount, areg, dreg;

				areg = dreg = 0;
				for (mycount=7;mycount>=0;mycount--)
				{
					areg = areg + M68020_regs.a[mycount];
					dreg = dreg + M68020_regs.d[mycount];
				}

		   		logerror("=> %8x %8x ",areg,dreg);
				logerror("%6x %4x %d\n",M68020_regs.pc,M68020_regs.sr & 0x271F,m68k_ICount);
			}
			#endif

			m68k_ICount -= M68020_regs.IRQ_cycles;
			M68020_regs.IRQ_cycles = 0;
//			m68k_memory_intf = a68k_memory_intf;
			mame_debug_hook(M68020_regs.pc);


			if (Machine->debug_mode)
				M68020_regs.asm68k_debug = -1;

			M68020_RUN();

			#ifdef TRACE68K
			if ((M68020_regs.IRQ_level & 0x80) || (cpunum_is_suspended(cpunum, SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE)))
				m68k_ICount = 0;
			else
				m68k_ICount = StartCycle - 12;
			#endif
		}
		else
			M68020_RUN();
	} while (m68k_ICount > 0);

#else

	M68020_RUN();

#endif /* ENABLE_DEBUGGER */

	return (cycles - m68k_ICount);
}

static void m68020_get_context(void *dst)
{
	if( dst )
		*(a68k_cpu_context*)dst = M68020_regs;
}

static void m68020_set_context(void *src)
{
	if( src )
	{
		M68020_regs = *(a68k_cpu_context*)src;
		a68k_memory_intf = M68020_regs.Memory_Interface;
		mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;
	}
}

static void m68020_assert_irq(int int_line)
{
	/* Save icount */
	int StartCount = m68k_ICount;
	M68020_regs.IRQ_level = int_line;

	/* Now check for Interrupt */

	m68k_ICount = -1;
	M68020_RUN();

	/* Restore Count */
	m68k_ICount = StartCount;
}

static void m68020_clear_irq(int int_line)
{
	M68020_regs.IRQ_level = 0;
}

static void m68020_set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
		irqline = 7;
	switch(state)
	{
		case CLEAR_LINE:
			m68020_clear_irq(irqline);
			return;
		case ASSERT_LINE:
			m68020_assert_irq(irqline);
			return;
		default:
			m68020_assert_irq(irqline);
			return;
	}
}

static offs_t m68020_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	A68K_SET_PC_CALLBACK(pc);

#ifdef ENABLE_DEBUGGER
	m68k_memory_intf = a68k_memory_intf;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68020);
#else
	sprintf( buffer, "$%04X", (oprom[0] << 8) | oprom[1]);
	return 2;
#endif
}
#endif

#if (HAS_M68EC020)

static void m68ec020_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68020_state_register("m68ec020", index);
	M68020_regs.reset_callback = 0;
	M68020_regs.cmpild_instr_callback = 0;
	M68020_regs.rte_instr_callback = 0;
	M68020_regs.tas_instr_callback = 0;
	M68020_regs.irq_callback = irqcallback;
}

static void m68ec020_reset(void)
{
	a68k_memory_intf = interface_d32;
	mem_amask = active_address_space[ADDRESS_SPACE_PROGRAM].addrmask;

	m68k32_reset_common();

	M68020_regs.CPUtype=2;
	M68020_regs.Memory_Interface = a68k_memory_intf;
}

static offs_t m68ec020_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	A68K_SET_PC_CALLBACK(pc);

#ifdef ENABLE_DEBUGGER
	m68k_memory_intf = a68k_memory_intf;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68EC020);
#else
	sprintf( buffer, "$%04X", (oprom[0] << 8) | oprom[1]);
	return 2;
#endif
}

#endif

#endif // A68K2



/**************************************************************************
 * Generic set_info
 **************************************************************************/

#ifdef A68K0

static void m68000_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				m68000_set_irq_line(0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 1:				m68000_set_irq_line(1, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 2:				m68000_set_irq_line(2, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 3:				m68000_set_irq_line(3, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 4:				m68000_set_irq_line(4, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 5:				m68000_set_irq_line(5, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 6:				m68000_set_irq_line(6, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 7:				m68000_set_irq_line(7, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:  		M68000_regs.pc = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:		M68000_regs.isp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_USP:		M68000_regs.usp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SR:		M68000_regs.sr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_VBR:		M68000_regs.vbr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SFC:		M68000_regs.sfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_DFC:		M68000_regs.dfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D0:		M68000_regs.d[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D1:		M68000_regs.d[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D2:		M68000_regs.d[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D3:		M68000_regs.d[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D4:		M68000_regs.d[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D5:		M68000_regs.d[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D6:		M68000_regs.d[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D7:		M68000_regs.d[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A0:		M68000_regs.a[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A1:		M68000_regs.a[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A2:		M68000_regs.a[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A3:		M68000_regs.a[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A4:		M68000_regs.a[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A5:		M68000_regs.a[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A6:		M68000_regs.a[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A7:		M68000_regs.a[7] = info->i;					break;
		
		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		M68000_regs.reset_callback = info->f;		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		M68000_regs.cmpild_instr_callback = (void (*)(unsigned int,int))info->f;		break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:		M68000_regs.rte_instr_callback = info->f;		break;
		case CPUINFO_PTR_M68K_TAS_CALLBACK:		M68000_regs.tas_instr_callback = (int (*)(void))(info->f);		break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void m68000asm_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(M68000_regs);			break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;
		
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:					info->i = 0;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = M68000_regs.previous_pc;		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = M68000_regs.pc;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = M68000_regs.isp;				break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = M68000_regs.usp;				break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = M68000_regs.sr;				break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = M68000_regs.vbr;				break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = M68000_regs.sfc;				break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = M68000_regs.dfc;				break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = M68000_regs.d[0];				break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = M68000_regs.d[1];				break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = M68000_regs.d[2];				break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = M68000_regs.d[3];				break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = M68000_regs.d[4];				break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = M68000_regs.d[5];				break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = M68000_regs.d[6];				break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = M68000_regs.d[7];				break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = M68000_regs.a[0];				break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = M68000_regs.a[1];				break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = M68000_regs.a[2];				break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = M68000_regs.a[3];				break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = M68000_regs.a[4];				break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = M68000_regs.a[5];				break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = M68000_regs.a[6];				break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = M68000_regs.a[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68000_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68000_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68000_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68000_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68000_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68000_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68000_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68000_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68000"); break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.16"); break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__); break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998,99 Mike Coates, Darren Olafson. All rights reserved"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				M68000_regs.sr & 0x8000 ? 'T':'.',
				M68000_regs.sr & 0x4000 ? '?':'.',
				M68000_regs.sr & 0x2000 ? 'S':'.',
				M68000_regs.sr & 0x1000 ? '?':'.',
				M68000_regs.sr & 0x0800 ? '?':'.',
				M68000_regs.sr & 0x0400 ? 'I':'.',
				M68000_regs.sr & 0x0200 ? 'I':'.',
				M68000_regs.sr & 0x0100 ? 'I':'.',
				M68000_regs.sr & 0x0080 ? '?':'.',
				M68000_regs.sr & 0x0040 ? '?':'.',
				M68000_regs.sr & 0x0020 ? '?':'.',
				M68000_regs.sr & 0x0010 ? 'X':'.',
				M68000_regs.sr & 0x0008 ? 'N':'.',
				M68000_regs.sr & 0x0004 ? 'Z':'.',
				M68000_regs.sr & 0x0002 ? 'V':'.',
				M68000_regs.sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%06X", M68000_regs.pc); break;
		case CPUINFO_STR_REGISTER + M68K_ISP:			sprintf(info->s, "ISP:%08X", M68000_regs.isp); break;
		case CPUINFO_STR_REGISTER + M68K_USP:			sprintf(info->s, "USP:%08X", M68000_regs.usp); break;
		case CPUINFO_STR_REGISTER + M68K_SR:			sprintf(info->s, "SR :%08X", M68000_regs.sr); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", M68000_regs.vbr); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%08X", M68000_regs.sfc); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%08X", M68000_regs.dfc); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", M68000_regs.d[0]); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", M68000_regs.d[1]); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", M68000_regs.d[2]); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", M68000_regs.d[3]); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", M68000_regs.d[4]); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", M68000_regs.d[5]); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", M68000_regs.d[6]); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", M68000_regs.d[7]); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", M68000_regs.a[0]); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", M68000_regs.a[1]); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", M68000_regs.a[2]); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", M68000_regs.a[3]); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", M68000_regs.a[4]); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", M68000_regs.a[5]); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", M68000_regs.a[6]); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", M68000_regs.a[7]); break;
	}
}

#if (HAS_M68008)
/**************************************************************************
 * CPU-specific get_info
 **************************************************************************/

void m68008asm_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 22;					break;
		case CPUINFO_PTR_INIT:							info->init = m68008_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68008_reset;				break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68008_dasm;		break;
		case CPUINFO_STR_NAME:							strcpy(info->s, "68008"); break;
		default:
			m68000asm_get_info(state, info);
	}
}

#endif

#endif	// A68K0


#ifdef A68K1

#if HAS_M68010
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void m68010_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				m68010_set_irq_line(0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 1:				m68010_set_irq_line(1, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 2:				m68010_set_irq_line(2, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 3:				m68010_set_irq_line(3, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 4:				m68010_set_irq_line(4, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 5:				m68010_set_irq_line(5, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 6:				m68010_set_irq_line(6, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 7:				m68010_set_irq_line(7, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:  		M68010_regs.pc = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:		M68010_regs.isp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_USP:		M68010_regs.usp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SR:		M68010_regs.sr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_VBR:		M68010_regs.vbr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SFC:		M68010_regs.sfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_DFC:		M68010_regs.dfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D0:		M68010_regs.d[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D1:		M68010_regs.d[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D2:		M68010_regs.d[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D3:		M68010_regs.d[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D4:		M68010_regs.d[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D5:		M68010_regs.d[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D6:		M68010_regs.d[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D7:		M68010_regs.d[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A0:		M68010_regs.a[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A1:		M68010_regs.a[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A2:		M68010_regs.a[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A3:		M68010_regs.a[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A4:		M68010_regs.a[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A5:		M68010_regs.a[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A6:		M68010_regs.a[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A7:		M68010_regs.a[7] = info->i;					break;
		
		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		M68010_regs.reset_callback = info->f;		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		M68010_regs.cmpild_instr_callback = (void (*)(unsigned int,int))info->f;		break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:		M68010_regs.rte_instr_callback = info->f;		break;
	}
}

void m68010asm_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(M68010_regs);			break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;
		
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:					info->i = 0;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = M68010_regs.previous_pc;		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = M68010_regs.pc;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = M68010_regs.isp;				break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = M68010_regs.usp;				break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = M68010_regs.sr;				break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = M68010_regs.vbr;				break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = M68010_regs.sfc;				break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = M68010_regs.dfc;				break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = M68010_regs.d[0];				break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = M68010_regs.d[1];				break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = M68010_regs.d[2];				break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = M68010_regs.d[3];				break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = M68010_regs.d[4];				break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = M68010_regs.d[5];				break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = M68010_regs.d[6];				break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = M68010_regs.d[7];				break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = M68010_regs.a[0];				break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = M68010_regs.a[1];				break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = M68010_regs.a[2];				break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = M68010_regs.a[3];				break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = M68010_regs.a[4];				break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = M68010_regs.a[5];				break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = M68010_regs.a[6];				break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = M68010_regs.a[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68010_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68010_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68010_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68010_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68010_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68010_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68010_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68010_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68010"); break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.16"); break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__); break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998,99 Mike Coates, Darren Olafson. All rights reserved"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				M68010_regs.sr & 0x8000 ? 'T':'.',
				M68010_regs.sr & 0x4000 ? '?':'.',
				M68010_regs.sr & 0x2000 ? 'S':'.',
				M68010_regs.sr & 0x1000 ? '?':'.',
				M68010_regs.sr & 0x0800 ? '?':'.',
				M68010_regs.sr & 0x0400 ? 'I':'.',
				M68010_regs.sr & 0x0200 ? 'I':'.',
				M68010_regs.sr & 0x0100 ? 'I':'.',
				M68010_regs.sr & 0x0080 ? '?':'.',
				M68010_regs.sr & 0x0040 ? '?':'.',
				M68010_regs.sr & 0x0020 ? '?':'.',
				M68010_regs.sr & 0x0010 ? 'X':'.',
				M68010_regs.sr & 0x0008 ? 'N':'.',
				M68010_regs.sr & 0x0004 ? 'Z':'.',
				M68010_regs.sr & 0x0002 ? 'V':'.',
				M68010_regs.sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%06X", M68010_regs.pc); break;
		case CPUINFO_STR_REGISTER + M68K_ISP:			sprintf(info->s, "ISP:%08X", M68010_regs.isp); break;
		case CPUINFO_STR_REGISTER + M68K_USP:			sprintf(info->s, "USP:%08X", M68010_regs.usp); break;
		case CPUINFO_STR_REGISTER + M68K_SR:			sprintf(info->s, "SR :%08X", M68010_regs.sr); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", M68010_regs.vbr); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%08X", M68010_regs.sfc); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%08X", M68010_regs.dfc); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", M68010_regs.d[0]); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", M68010_regs.d[1]); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", M68010_regs.d[2]); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", M68010_regs.d[3]); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", M68010_regs.d[4]); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", M68010_regs.d[5]); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", M68010_regs.d[6]); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", M68010_regs.d[7]); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", M68010_regs.a[0]); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", M68010_regs.a[1]); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", M68010_regs.a[2]); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", M68010_regs.a[3]); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", M68010_regs.a[4]); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", M68010_regs.a[5]); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", M68010_regs.a[6]); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", M68010_regs.a[7]); break;
	}
}

#endif

#endif // A68K1


#ifdef A68K2

#if HAS_M68020
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void m68020_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				m68020_set_irq_line(0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 1:				m68020_set_irq_line(1, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 2:				m68020_set_irq_line(2, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 3:				m68020_set_irq_line(3, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 4:				m68020_set_irq_line(4, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 5:				m68020_set_irq_line(5, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 6:				m68020_set_irq_line(6, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + 7:				m68020_set_irq_line(7, info->i);			break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:  		M68020_regs.pc = info->i;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:		M68020_regs.isp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_USP:		M68020_regs.usp = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SR:		M68020_regs.sr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_VBR:		M68020_regs.vbr = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_SFC:		M68020_regs.sfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_DFC:		M68020_regs.dfc = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D0:		M68020_regs.d[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D1:		M68020_regs.d[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D2:		M68020_regs.d[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D3:		M68020_regs.d[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D4:		M68020_regs.d[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D5:		M68020_regs.d[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D6:		M68020_regs.d[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_D7:		M68020_regs.d[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A0:		M68020_regs.a[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A1:		M68020_regs.a[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A2:		M68020_regs.a[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A3:		M68020_regs.a[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A4:		M68020_regs.a[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A5:		M68020_regs.a[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A6:		M68020_regs.a[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + M68K_A7:		M68020_regs.a[7] = info->i;					break;
		
		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		M68020_regs.reset_callback = info->f;		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		M68020_regs.cmpild_instr_callback = (void (*)(unsigned int,int))info->f;		break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:		M68020_regs.rte_instr_callback = info->f;		break;
	}
}

void m68020asm_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(M68020_regs);			break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;
		
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:					info->i = 1;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:					info->i = 2;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:					info->i = 3;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:					info->i = 4;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:					info->i = 5;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:					info->i = 6;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:					info->i = 7;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = M68020_regs.previous_pc;		break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = M68020_regs.pc;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = M68020_regs.isp;				break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = M68020_regs.usp;				break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = M68020_regs.sr;				break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = M68020_regs.vbr;				break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = M68020_regs.sfc;				break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = M68020_regs.dfc;				break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = M68020_regs.d[0];				break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = M68020_regs.d[1];				break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = M68020_regs.d[2];				break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = M68020_regs.d[3];				break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = M68020_regs.d[4];				break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = M68020_regs.d[5];				break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = M68020_regs.d[6];				break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = M68020_regs.d[7];				break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = M68020_regs.a[0];				break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = M68020_regs.a[1];				break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = M68020_regs.a[2];				break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = M68020_regs.a[3];				break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = M68020_regs.a[4];				break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = M68020_regs.a[5];				break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = M68020_regs.a[6];				break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = M68020_regs.a[7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68020_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68020_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68020_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68020_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68020_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68020_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68020_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68020_dasm;		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68020"); break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.16"); break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__); break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998,99 Mike Coates, Darren Olafson. All rights reserved"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				M68020_regs.sr & 0x8000 ? 'T':'.',
				M68020_regs.sr & 0x4000 ? '?':'.',
				M68020_regs.sr & 0x2000 ? 'S':'.',
				M68020_regs.sr & 0x1000 ? '?':'.',
				M68020_regs.sr & 0x0800 ? '?':'.',
				M68020_regs.sr & 0x0400 ? 'I':'.',
				M68020_regs.sr & 0x0200 ? 'I':'.',
				M68020_regs.sr & 0x0100 ? 'I':'.',
				M68020_regs.sr & 0x0080 ? '?':'.',
				M68020_regs.sr & 0x0040 ? '?':'.',
				M68020_regs.sr & 0x0020 ? '?':'.',
				M68020_regs.sr & 0x0010 ? 'X':'.',
				M68020_regs.sr & 0x0008 ? 'N':'.',
				M68020_regs.sr & 0x0004 ? 'Z':'.',
				M68020_regs.sr & 0x0002 ? 'V':'.',
				M68020_regs.sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%06X", M68020_regs.pc); break;
		case CPUINFO_STR_REGISTER + M68K_ISP:			sprintf(info->s, "ISP:%08X", M68020_regs.isp); break;
		case CPUINFO_STR_REGISTER + M68K_USP:			sprintf(info->s, "USP:%08X", M68020_regs.usp); break;
		case CPUINFO_STR_REGISTER + M68K_SR:			sprintf(info->s, "SR :%08X", M68020_regs.sr); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", M68020_regs.vbr); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%08X", M68020_regs.sfc); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%08X", M68020_regs.dfc); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", M68020_regs.d[0]); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", M68020_regs.d[1]); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", M68020_regs.d[2]); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", M68020_regs.d[3]); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", M68020_regs.d[4]); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", M68020_regs.d[5]); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", M68020_regs.d[6]); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", M68020_regs.d[7]); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", M68020_regs.a[0]); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", M68020_regs.a[1]); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", M68020_regs.a[2]); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", M68020_regs.a[3]); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", M68020_regs.a[4]); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", M68020_regs.a[5]); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", M68020_regs.a[6]); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", M68020_regs.a[7]); break;
	}
}
#endif


#if (HAS_M68EC020)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m68ec020asm_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = m68ec020_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68ec020_reset;			break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68ec020_dasm;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68EC020"); break;

		default:
			m68020asm_get_info(state, info);
			break;
	}
}
#endif

#if (HAS_M68040)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

void m68040asm_get_info(UINT32 state, cpuinfo *info)
{
	m68020asm_get_info(state, info);
}
#endif

#endif	// A68K2
