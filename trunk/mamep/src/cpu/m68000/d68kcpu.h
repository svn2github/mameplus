#include <stdio.h>
/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.3
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright 1998-2001 Karl Stenerud.  All rights reserved.
 *
 * This code may be freely used for non-commercial purposes as long as this
 * copyright notice remains unaltered in the source code and any binary files
 * containing this code in compiled form.
 *
 * All other lisencing terms must be negotiated with the author
 * (Karl Stenerud).
 *
 * The latest version of this code can be obtained at:
 * http://kstenerud.cjb.net
 */

/*
 * DRC conversion by BUT
 */




#ifndef M68KCPU__HEADER
#define M68KCPU__HEADER

#include "m68k.h"
#include <limits.h>

#if M68K_EMULATE_ADDRESS_ERROR
#include <setjmp.h>
#endif /* M68K_EMULATE_ADDRESS_ERROR */

/* DRC */
#include "x86drc.h"


#define RECOMPILE_UNIMPLEMENTED		0x0000
#define RECOMPILE_MAY_CAUSE_EXCEPTION	0x0002
#define RECOMPILE_END_OF_STRING		0x0004
#define RECOMPILE_CHECK_INTERRUPTS	0x0008
#define RECOMPILE_ADD_DISPATCH		0x0010
#define RECOMPILE_DONT_ADD_PCDELTA	0x0020
#define RECOMPILE_VNCXZ_FLAGS_DIRTY	0x0100
#define RECOMPILE_VNCZ_FLAGS_DIRTY	0x0200
#define RECOMPILE_SUCCESSFUL		0x0001
#define RECOMPILE_SUCCESSFUL_CP(c,p)	(RECOMPILE_SUCCESSFUL | m68kdrc_recompile_flag | (((c) & 0xff) << 16) | (((p) & 0xff) << 24))

extern int m68kdrc_cycles;
extern int m68kdrc_recompile_flag;
extern int m68kdrc_check_code_modify;
extern unsigned int m68kdrc_instr_size;
extern link_info m68kdrc_link_make_cc;

#define m68kdrc_cpu		m68ki_cpu
#define m68kdrc_cpu_core	m68ki_cpu_core


/* ======================================================================== */
/* ==================== ARCHITECTURE-DEPENDANT DEFINES ==================== */
/* ======================================================================== */

/* Check for > 32bit sizes */
#if UINT_MAX > 0xffffffff
	#define M68K_INT_GT_32_BIT  1
#else
	#define M68K_INT_GT_32_BIT  0
#endif

/* Data types used in this emulation core */
#undef sint8
#undef sint16
#undef sint32
#undef sint64
#undef uint8
#undef uint16
#undef uint32
#undef uint64
#undef sint
#undef uint

#define sint8  signed   char			/* ASG: changed from char to signed char */
#define sint16 signed   short
#define sint32 signed   long
#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned long

/* signed and unsigned int must be at least 32 bits wide */
#define sint   signed   int
#define uint   unsigned int


#if M68K_USE_64_BIT
#define sint64 signed   long long
#define uint64 unsigned long long
#else
#define sint64 sint32
#define uint64 uint32
#endif /* M68K_USE_64_BIT */



/* Allow for architectures that don't have 8-bit sizes */
#if UCHAR_MAX == 0xff
	#define MAKE_INT_8(A) (sint8)(A)
#else
	#undef  sint8
	#define sint8  signed   int
	#undef  uint8
	#define uint8  unsigned int
	INLINE sint MAKE_INT_8(uint value)
	{
		return (value & 0x80) ? value | ~0xff : value & 0xff;
	}
#endif /* UCHAR_MAX == 0xff */


/* Allow for architectures that don't have 16-bit sizes */
#if USHRT_MAX == 0xffff
	#define MAKE_INT_16(A) (sint16)(A)
#else
	#undef  sint16
	#define sint16 signed   int
	#undef  uint16
	#define uint16 unsigned int
	INLINE sint MAKE_INT_16(uint value)
	{
		return (value & 0x8000) ? value | ~0xffff : value & 0xffff;
	}
#endif /* USHRT_MAX == 0xffff */


/* Allow for architectures that don't have 32-bit sizes */
#if ULONG_MAX == 0xffffffff
	#define MAKE_INT_32(A) (sint32)(A)
#else
	#undef  sint32
	#define sint32  signed   int
	#undef  uint32
	#define uint32  unsigned int
	INLINE sint MAKE_INT_32(uint value)
	{
		return (value & 0x80000000) ? value | ~0xffffffff : value & 0xffffffff;
	}
#endif /* ULONG_MAX == 0xffffffff */




/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */
/* ======================================================================== */

/* Exception Vectors handled by emulation */
#define EXCEPTION_BUS_ERROR                2 /* This one is not emulated! */
#define EXCEPTION_ADDRESS_ERROR            3 /* This one is partially emulated (doesn't stack a proper frame yet) */
#define EXCEPTION_ILLEGAL_INSTRUCTION      4
#define EXCEPTION_ZERO_DIVIDE              5
#define EXCEPTION_CHK                      6
#define EXCEPTION_TRAPV                    7
#define EXCEPTION_PRIVILEGE_VIOLATION      8
#define EXCEPTION_TRACE                    9
#define EXCEPTION_1010                    10
#define EXCEPTION_1111                    11
#define EXCEPTION_FORMAT_ERROR            14
#define EXCEPTION_UNINITIALIZED_INTERRUPT 15
#define EXCEPTION_SPURIOUS_INTERRUPT      24
#define EXCEPTION_INTERRUPT_AUTOVECTOR    24
#define EXCEPTION_TRAP_BASE               32

/* Function codes set by CPU during data/address bus activity */
#define FUNCTION_CODE_USER_DATA          1
#define FUNCTION_CODE_USER_PROGRAM       2
#define FUNCTION_CODE_SUPERVISOR_DATA    5
#define FUNCTION_CODE_SUPERVISOR_PROGRAM 6
#define FUNCTION_CODE_CPU_SPACE          7

/* CPU types for deciding what to emulate */
#define CPU_TYPE_000   1
#define CPU_TYPE_008   2
#define CPU_TYPE_010   4
#define CPU_TYPE_EC020 8
#define CPU_TYPE_020   16
#define CPU_TYPE_040   32

/* Different ways to stop the CPU */
#define STOP_LEVEL_STOP 1
#define STOP_LEVEL_HALT 2

/* Used for 68000 address error processing */
#define INSTRUCTION_YES 0
#define INSTRUCTION_NO  0x08
#define MODE_READ       0x10
#define MODE_WRITE      0

#define RUN_MODE_NORMAL          0
#define RUN_MODE_BERR_AERR_RESET 1

#ifndef NULL
#define NULL ((void*)0)
#endif

/* ======================================================================== */
/* ================================ MACROS ================================ */
/* ======================================================================== */


/* ---------------------------- General Macros ---------------------------- */

/* Bit Isolation Macros */
#define BIT_0(A)  ((A) & 0x00000001)
#define BIT_1(A)  ((A) & 0x00000002)
#define BIT_2(A)  ((A) & 0x00000004)
#define BIT_3(A)  ((A) & 0x00000008)
#define BIT_4(A)  ((A) & 0x00000010)
#define BIT_5(A)  ((A) & 0x00000020)
#define BIT_6(A)  ((A) & 0x00000040)
#define BIT_7(A)  ((A) & 0x00000080)
#define BIT_8(A)  ((A) & 0x00000100)
#define BIT_9(A)  ((A) & 0x00000200)
#define BIT_A(A)  ((A) & 0x00000400)
#define BIT_B(A)  ((A) & 0x00000800)
#define BIT_C(A)  ((A) & 0x00001000)
#define BIT_D(A)  ((A) & 0x00002000)
#define BIT_E(A)  ((A) & 0x00004000)
#define BIT_F(A)  ((A) & 0x00008000)
#define BIT_10(A) ((A) & 0x00010000)
#define BIT_11(A) ((A) & 0x00020000)
#define BIT_12(A) ((A) & 0x00040000)
#define BIT_13(A) ((A) & 0x00080000)
#define BIT_14(A) ((A) & 0x00100000)
#define BIT_15(A) ((A) & 0x00200000)
#define BIT_16(A) ((A) & 0x00400000)
#define BIT_17(A) ((A) & 0x00800000)
#define BIT_18(A) ((A) & 0x01000000)
#define BIT_19(A) ((A) & 0x02000000)
#define BIT_1A(A) ((A) & 0x04000000)
#define BIT_1B(A) ((A) & 0x08000000)
#define BIT_1C(A) ((A) & 0x10000000)
#define BIT_1D(A) ((A) & 0x20000000)
#define BIT_1E(A) ((A) & 0x40000000)
#define BIT_1F(A) ((A) & 0x80000000)

/* Get the most significant bit for specific sizes */
#define GET_MSB_8(A)  ((A) & 0x80)
#define GET_MSB_9(A)  ((A) & 0x100)
#define GET_MSB_16(A) ((A) & 0x8000)
#define GET_MSB_17(A) ((A) & 0x10000)
#define GET_MSB_32(A) ((A) & 0x80000000)
#if M68K_USE_64_BIT
#define GET_MSB_33(A) ((A) & 0x100000000)
#endif /* M68K_USE_64_BIT */

/* Isolate nibbles */
#define LOW_NIBBLE(A)  ((A) & 0x0f)
#define HIGH_NIBBLE(A) ((A) & 0xf0)

/* These are used to isolate 8, 16, and 32 bit sizes */
#define MASK_OUT_ABOVE_2(A)  ((A) & 3)
#define MASK_OUT_ABOVE_8(A)  ((A) & 0xff)
#define MASK_OUT_ABOVE_16(A) ((A) & 0xffff)
#define MASK_OUT_BELOW_2(A)  ((A) & ~3)
#define MASK_OUT_BELOW_8(A)  ((A) & ~0xff)
#define MASK_OUT_BELOW_16(A) ((A) & ~0xffff)

/* No need to mask if we are 32 bit */
#if M68K_INT_GT_32_BIT || M68K_USE_64_BIT
	#define MASK_OUT_ABOVE_32(A) ((A) & 0xffffffff)
	#define MASK_OUT_BELOW_32(A) ((A) & ~0xffffffff)
#else
	#define MASK_OUT_ABOVE_32(A) (A)
	#define MASK_OUT_BELOW_32(A) 0
#endif /* M68K_INT_GT_32_BIT || M68K_USE_64_BIT */

/* Simulate address lines of 68k family */
#define ADDRESS_68K(A) ((A)&CPU_ADDRESS_MASK)


/* ------------------------------ CPU Access ------------------------------ */

/* Access the CPU registers */
#define CPU_TYPE         m68kdrc_cpu.cpu_type

#define REG68K_DA        m68kdrc_cpu.dar /* easy access to data and address regs */
#define REG68K_D         m68kdrc_cpu.dar
#define REG68K_A         (m68kdrc_cpu.dar+8)
#define REG68K_PPC 	 m68kdrc_cpu.ppc
#define REG68K_PC        m68kdrc_cpu.pc
#define REG68K_SP_BASE   m68kdrc_cpu.sp
#define REG68K_USP       m68kdrc_cpu.sp[0]
#define REG68K_ISP       m68kdrc_cpu.sp[4]
#define REG68K_MSP       m68kdrc_cpu.sp[6]
#define REG68K_SP        m68kdrc_cpu.dar[15]
#define REG68K_VBR       m68kdrc_cpu.vbr
#define REG68K_SFC       m68kdrc_cpu.sfc
#define REG68K_DFC       m68kdrc_cpu.dfc
#define REG68K_CACR      m68kdrc_cpu.cacr
#define REG68K_CAAR      m68kdrc_cpu.caar
#define REG68K_IR        m68kdrc_cpu.ir

#define REG68K_FP        m68kdrc_cpu.fpr
#define REG68K_FPCR      m68kdrc_cpu.fpcr
#define REG68K_FPSR      m68kdrc_cpu.fpsr
#define REG68K_FPIAR     m68kdrc_cpu.fpiar

#define FLAG_T1          m68kdrc_cpu.t1_flag
#define FLAG_T0          m68kdrc_cpu.t0_flag
#define FLAG_S           m68kdrc_cpu.s_flag
#define FLAG_M           m68kdrc_cpu.m_flag
#define FLAG_X           m68kdrc_cpu.x_flag
#define FLAG_N           m68kdrc_cpu.n_flag
#define FLAG_Z           m68kdrc_cpu.not_z_flag
#define FLAG_V           m68kdrc_cpu.v_flag
#define FLAG_C           m68kdrc_cpu.c_flag
#define FLAG_INT_MASK    m68kdrc_cpu.int_mask

#define CPU_INT_LEVEL    m68kdrc_cpu.int_level /* ASG: changed from CPU_INTS_PENDING */
#define CPU_INT_CYCLES   m68kdrc_cpu.int_cycles /* ASG */
#define CPU_STOPPED      m68kdrc_cpu.stopped
#define CPU_PREF_ADDR    m68kdrc_cpu.pref_addr
#define CPU_PREF_DATA    m68kdrc_cpu.pref_data
#define CPU_ADDRESS_MASK m68kdrc_cpu.address_mask
#define CPU_SR_MASK      m68kdrc_cpu.sr_mask
#define CPU_INSTR_MODE   m68kdrc_cpu.instr_mode
#define CPU_RUN_MODE     m68kdrc_cpu.run_mode

#define CYC_INSTRUCTION  m68kdrc_cpu.cyc_instruction
#define CYC_EXCEPTION    m68kdrc_cpu.cyc_exception
#define CYC_BCC_NOTAKE_B m68kdrc_cpu.cyc_bcc_notake_b
#define CYC_BCC_NOTAKE_W m68kdrc_cpu.cyc_bcc_notake_w
#define CYC_DBCC_F_NOEXP m68kdrc_cpu.cyc_dbcc_f_noexp
#define CYC_DBCC_F_EXP   m68kdrc_cpu.cyc_dbcc_f_exp
#define CYC_SCC_R_TRUE   m68kdrc_cpu.cyc_scc_r_true
#define CYC_MOVEM_W      m68kdrc_cpu.cyc_movem_w
#define CYC_MOVEM_L      m68kdrc_cpu.cyc_movem_l
#define CYC_SHIFT        m68kdrc_cpu.cyc_shift
#define CYC_RESET        m68kdrc_cpu.cyc_reset


#define CALLBACK_INT_ACK      m68kdrc_cpu.int_ack_callback
#define CALLBACK_BKPT_ACK     m68kdrc_cpu.bkpt_ack_callback
#define CALLBACK_RESET_INSTR  m68kdrc_cpu.reset_instr_callback
#define CALLBACK_CMPILD_INSTR m68kdrc_cpu.cmpild_instr_callback
#define CALLBACK_RTE_INSTR    m68kdrc_cpu.rte_instr_callback
#define CALLBACK_PC_CHANGED   m68kdrc_cpu.pc_changed_callback
#define CALLBACK_SET_FC       m68kdrc_cpu.set_fc_callback
#define CALLBACK_INSTR_HOOK   m68kdrc_cpu.instr_hook_callback

#define INSTR_FLAG_DIRTY	m68kdrc_cpu.flag_dirty



/* ----------------------------- Configuration ---------------------------- */

/* These defines are dependant on the configuration defines in m68kconf.h */

/* Disable certain comparisons if we're not using all CPU types */
#if M68K_EMULATE_040
	#define CPU_TYPE_IS_040_PLUS(A)    ((A) & CPU_TYPE_040)
	#define CPU_TYPE_IS_040_LESS(A)    1
#else
	#define CPU_TYPE_IS_040_PLUS(A)    0
	#define CPU_TYPE_IS_040_LESS(A)    1
#endif

#if M68K_EMULATE_020
	#define CPU_TYPE_IS_020_PLUS(A)    ((A) & (CPU_TYPE_020 | CPU_TYPE_040))
	#define CPU_TYPE_IS_020_LESS(A)    1
#else
	#define CPU_TYPE_IS_020_PLUS(A)    0
	#define CPU_TYPE_IS_020_LESS(A)    1
#endif

#if M68K_EMULATE_EC020
	#define CPU_TYPE_IS_EC020_PLUS(A)  ((A) & (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_040))
	#define CPU_TYPE_IS_EC020_LESS(A)  ((A) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020))
#else
	#define CPU_TYPE_IS_EC020_PLUS(A)  CPU_TYPE_IS_020_PLUS(A)
	#define CPU_TYPE_IS_EC020_LESS(A)  CPU_TYPE_IS_020_LESS(A)
#endif

#if M68K_EMULATE_010
	#define CPU_TYPE_IS_010(A)         ((A) == CPU_TYPE_010)
	#define CPU_TYPE_IS_010_PLUS(A)    ((A) & (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_040))
	#define CPU_TYPE_IS_010_LESS(A)    ((A) & (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010))
#else
	#define CPU_TYPE_IS_010(A)         0
	#define CPU_TYPE_IS_010_PLUS(A)    CPU_TYPE_IS_EC020_PLUS(A)
	#define CPU_TYPE_IS_010_LESS(A)    CPU_TYPE_IS_EC020_LESS(A)
#endif

#if M68K_EMULATE_020 || M68K_EMULATE_EC020
	#define CPU_TYPE_IS_020_VARIANT(A) ((A) & (CPU_TYPE_EC020 | CPU_TYPE_020))
#else
	#define CPU_TYPE_IS_020_VARIANT(A) 0
#endif

#if M68K_EMULATE_040 || M68K_EMULATE_020 || M68K_EMULATE_EC020 || M68K_EMULATE_010
	#define CPU_TYPE_IS_000(A)         ((A) == CPU_TYPE_000 || (A) == CPU_TYPE_008)
#else
	#define CPU_TYPE_IS_000(A)         1
#endif


#if !M68K_SEPARATE_READS
#define m68k_read_immediate_16(A) m68ki_read_program_16(A)
#define m68k_read_immediate_32(A) m68ki_read_program_32(A)

#define m68k_read_pcrelative_8(A) m68ki_read_program_8(A)
#define m68k_read_pcrelative_16(A) m68ki_read_program_16(A)
#define m68k_read_pcrelative_32(A) m68ki_read_program_32(A)
#endif /* M68K_SEPARATE_READS */


/* Enable or disable callback functions */
#if M68K_EMULATE_INT_ACK
	extern int m68kdrc_call_int_ack(int int_line);
	#define m68kdrc_int_ack(reg)	do { _push_r32(reg); _call(m68kdrc_call_int_ack); _add_r32_imm(REG_ESP, 4); } while (0)
#else
	#define m68kdrc_int_ack(reg)
#endif /* M68K_EMULATE_INT_ACK */

#if M68K_EMULATE_BKPT_ACK
	extern void m68kdrc_call_bkpt_ack_callback(unsigned int data);
	#define m68kdrc_bkpt_ack(reg)	do { _push_r32(reg); _call(m68kdrc_call_bkpt_ack_callback); _add_r32_imm(REG_ESP, 4); } while (0)
#else
	#define m68kdrc_bkpt_ack(reg)
#endif /* M68K_EMULATE_BKPT_ACK */

#if M68K_EMULATE_RESET
	extern void m68kdrc_call_reset_instr_callback(void);
	#define m68kdrc_output_reset()	_call(m68kdrc_call_reset_instr_callback)
#else
	#define m68kdrc_output_reset()
#endif /* M68K_EMULATE_RESET */

#if M68K_CMPILD_HAS_CALLBACK
	extern void m68kdrc_call_cmpild_instr_callback(unsigned int val, int reg);
	#define m68kdrc_cmpild_callback(val,reg) \
		do { \
			_push_imm(reg); _push_imm(val); _call(m68kdrc_call_cmpild_instr_callback); _add_r32_imm(REG_ESP, 8); \
			m68kdrc_recompile_flag |= RECOMPILE_ADD_DISPATCH | RECOMPILE_END_OF_STRING; \
		} while (0)
#else
	#define m68kdrc_cmpild_callback(val,reg)
#endif /* M68K_CMPILD_HAS_CALLBACK */

#if M68K_RTE_HAS_CALLBACK
	extern void m68kdrc_call_rte_callback(void);
	#define m68kdrc_rte_callback()	_call(m68kdrc_call_rte_callback);
#else
	#define m68kdrc_rte_callback()
#endif /* M68K_RTE_HAS_CALLBACK */

#if M68K_INSTRUCTION_HOOK
	#if M68K_INSTRUCTION_HOOK == OPT_SPECIFY_HANDLER
		#define m68ki_instr_hook() M68K_INSTRUCTION_CALLBACK()
	#else
		#define m68ki_instr_hook() CALLBACK_INSTR_HOOK()
	#endif
#else
	#define m68ki_instr_hook()
#endif /* M68K_INSTRUCTION_HOOK */

#if M68K_MONITOR_PC
	extern void m68kdrc_call_pc_changed_callback(unsigned int new_pc);
	#define m68kdrc_pc_changed(reg)	do { _push_r32(reg); _call(m68kdrc_call_pc_changed_callback); _add_r32_imm(REG_ESP, 4); } while (0)
#else
	#define m68kdrc_pc_changed(reg)
#endif /* M68K_MONITOR_PC */


/* Enable or disable function code emulation */
#if M68K_EMULATE_FC
	#if M68K_EMULATE_FC == OPT_SPECIFY_HANDLER
		#define m68ki_set_fc(A) M68K_SET_FC_CALLBACK(A)
	#else
		#define m68ki_set_fc(A) CALLBACK_SET_FC(A)
	#endif
	#define m68ki_use_data_space() m68ki_address_space = FUNCTION_CODE_USER_DATA
	#define m68ki_use_program_space() m68ki_address_space = FUNCTION_CODE_USER_PROGRAM
	#define m68ki_get_address_space() m68ki_address_space
#else
	#define m68ki_set_fc(A)
	#define m68ki_use_data_space()
	#define m68ki_use_program_space()
	#define m68ki_get_address_space() FUNCTION_CODE_USER_DATA
#endif /* M68K_EMULATE_FC */


/* Enable or disable trace emulation */
#if M68K_EMULATE_TRACE
	/* Initiates trace checking before each instruction (t1) */
	#define m68ki_trace_t1() m68ki_tracing = FLAG_T1
	/* adds t0 to trace checking if we encounter change of flow */
	#define m68ki_trace_t0() m68ki_tracing |= FLAG_T0
	/* Clear all tracing */
	#define m68ki_clear_trace() m68ki_tracing = 0
	/* Cause a trace exception if we are tracing */
	#define m68ki_exception_if_trace() if(m68ki_tracing) m68ki_exception_trace()
#else
	#define m68ki_trace_t1()
	#define m68ki_trace_t0()
	#define m68ki_clear_trace()
	#define m68ki_exception_if_trace()
#endif /* M68K_EMULATE_TRACE */



/* Address error */
#if M68K_EMULATE_ADDRESS_ERROR
	#include <setjmp.h>
	extern jmp_buf m68ki_aerr_trap;

	#define m68ki_set_address_error_trap() \
		if(setjmp(m68ki_aerr_trap) != 0) \
		{ \
			m68k_exception_address_error(); \
			if(CPU_STOPPED) \
			{ \
				SET_CYCLES(0); \
				CPU_INT_CYCLES = 0; \
				return m68ki_initial_cycles; \
			} \
		}

	#define m68ki_check_address_error(ADDR, WRITE_MODE, FC) \
		if((ADDR)&1) \
		{ \
			m68ki_aerr_address = ADDR; \
			m68ki_aerr_write_mode = WRITE_MODE; \
			m68ki_aerr_fc = FC; \
			longjmp(m68ki_aerr_trap, 1); \
		}

	#define m68ki_check_address_error_010_less(ADDR, WRITE_MODE, FC) \
		if (CPU_TYPE_IS_010_LESS(CPU_TYPE)) \
		{ \
			m68ki_check_address_error(ADDR, WRITE_MODE, FC) \
		}
#else
	#define m68ki_set_address_error_trap()
	#define m68ki_check_address_error(ADDR, WRITE_MODE, FC)
	#define m68ki_check_address_error_010_less(ADDR, WRITE_MODE, FC)
#endif /* M68K_ADDRESS_ERROR */

/* Logging */
#if M68K_LOG_ENABLE
	#include <stdio.h>
	extern FILE* M68K_LOG_FILEHANDLE
	extern char* m68kdrc_cpu_names[];

	#define M68K_DO_LOG(A) if(M68K_LOG_FILEHANDLE) fprintf A
	#if M68K_LOG_1010_1111
		#define M68K_DO_LOG_EMU(A) if(M68K_LOG_FILEHANDLE) fprintf A
	#else
		#define M68K_DO_LOG_EMU(A)
	#endif
#else
	#define M68K_DO_LOG(A)
	#define M68K_DO_LOG_EMU(A)
#endif



/* -------------------------- EA / Operand Access ------------------------- */

/*
 * The general instruction format follows this pattern:
 * .... XXX. .... .YYY
 * where XXX is register X and YYY is register Y
 */
/* Data Register Isolation */
#define DX (REG68K_D[(REG68K_IR >> 9) & 7])
#define DY (REG68K_D[REG68K_IR & 7])
/* Address Register Isolation */
#define AX (REG68K_A[(REG68K_IR >> 9) & 7])
#define AY (REG68K_A[REG68K_IR & 7])


/* Effective Address Calculations */
#define OPER_I_8()     m68ki_read_imm_8()
#define OPER_I_16()    m68ki_read_imm_16()
#define OPER_I_32()    m68ki_read_imm_32()


/* address register indirect */
#define DRC_EA_AY_AI_8()   _mov_r32_m32abs(REG_EAX, &AY)
#define DRC_EA_AY_AI_16()  DRC_EA_AY_AI_8()
#define DRC_EA_AY_AI_32()  DRC_EA_AY_AI_8()

/* postincrement */
#define DRC_EA_AY_PI_8()   do { DRC_EA_AY_AI_8(); _add_m32abs_imm(&AY, 1); } while (0)		/* (size = byte) */
#define DRC_EA_AY_PI_16()  do { DRC_EA_AY_AI_16(); _add_m32abs_imm(&AY, 2); } while (0)		/* (size = word) */
#define DRC_EA_AY_PI_32()  do { DRC_EA_AY_AI_32(); _add_m32abs_imm(&AY, 4); } while (0)		/* (size = long) */

/* predecrement */
#define DRC_EA_AY_PD_8()   do { _sub_m32abs_imm(&AY, 1); DRC_EA_AY_AI_8(); } while (0)		/* (size = byte) */
#define DRC_EA_AY_PD_16()  do { _sub_m32abs_imm(&AY, 2); DRC_EA_AY_AI_16(); } while (0)		/* (size = word) */
#define DRC_EA_AY_PD_32()  do { _sub_m32abs_imm(&AY, 4); DRC_EA_AY_AI_32(); } while (0)		/* (size = long) */

/* displacement */
#define DRC_EA_AY_DI_8()   do { uint32 ea = MAKE_INT_16(OPER_I_16()); _mov_r32_m32abs(REG_EAX, &AY); _add_r32_imm(REG_EAX, ea); } while (0)
#define DRC_EA_AY_DI_16()  DRC_EA_AY_DI_8()
#define DRC_EA_AY_DI_32()  DRC_EA_AY_DI_8()

/* indirect + index */
#define DRC_EA_AY_IX_8()   do { m68kdrc_get_ea_ix(drc, &AY); } while (0)
#define DRC_EA_AY_IX_16()  DRC_EA_AY_IX_8()
#define DRC_EA_AY_IX_32()  DRC_EA_AY_IX_8()

/* address register indirect */
#define DRC_EA_AX_AI_8()   _mov_r32_m32abs(REG_EAX, &AX)
#define DRC_EA_AX_AI_16()  DRC_EA_AX_AI_8()
#define DRC_EA_AX_AI_32()  DRC_EA_AX_AI_8()

/* postincrement */
#define DRC_EA_AX_PI_8()   do { DRC_EA_AX_AI_8(); _add_m32abs_imm(&AX, 1); } while (0)		/* (size = byte) */
#define DRC_EA_AX_PI_16()  do { DRC_EA_AX_AI_16(); _add_m32abs_imm(&AX, 2); } while (0)		/* (size = word) */
#define DRC_EA_AX_PI_32()  do { DRC_EA_AX_AI_32(); _add_m32abs_imm(&AX, 4); } while (0)		/* (size = long) */

/* predecrement */
#define DRC_EA_AX_PD_8()   do { _sub_m32abs_imm(&AX, 1); DRC_EA_AX_AI_8(); } while (0)		/* (size = byte) */
#define DRC_EA_AX_PD_16()  do { _sub_m32abs_imm(&AX, 2); DRC_EA_AX_AI_16(); } while (0)		/* (size = word) */
#define DRC_EA_AX_PD_32()  do { _sub_m32abs_imm(&AX, 4); DRC_EA_AX_AI_32(); } while (0)		/* (size = long) */

/* displacement */
#define DRC_EA_AX_DI_8()   do { uint32 ea = MAKE_INT_16(OPER_I_16()); _mov_r32_m32abs(REG_EAX, &AX); _add_r32_imm(REG_EAX, ea); } while (0)
#define DRC_EA_AX_DI_16()  DRC_EA_AX_DI_8()
#define DRC_EA_AX_DI_32()  DRC_EA_AX_DI_8()

/* indirect + index */
#define DRC_EA_AX_IX_8()   do { m68kdrc_get_ea_ix(drc, &AX); } while (0)
#define DRC_EA_AX_IX_16()  DRC_EA_AX_IX_8()
#define DRC_EA_AX_IX_32()  DRC_EA_AX_IX_8()

#define DRC_EA_A7_AI_8()   do { _mov_r32_m32abs(REG_EAX, &REG68K_A[7]); } while (0)
#define DRC_EA_A7_PI_8()   do { DRC_EA_A7_AI_8(); _add_m32abs_imm(&REG68K_A[7], 2); } while (0)	
#define DRC_EA_A7_PD_8()   do { _sub_m32abs_imm(&REG68K_A[7], 2); DRC_EA_A7_AI_8(); } while (0)

/* absolute word */
#define DRC_EA_AW_8()      do { uint32 ea = MAKE_INT_16(OPER_I_16()); _mov_r32_imm(REG_EAX, ea); } while (0)
#define DRC_EA_AW_16()     DRC_EA_AW_8()
#define DRC_EA_AW_32()     DRC_EA_AW_8()

/* absolute long */
#define DRC_EA_AL_8()      do { uint32 ea = OPER_I_32(); _mov_r32_imm(REG_EAX, ea); } while (0)
#define DRC_EA_AL_16()     DRC_EA_AL_8()
#define DRC_EA_AL_32()     DRC_EA_AL_8()

/* pc indirect + displacement */
#define DRC_EA_PCDI_8()    m68kdrc_get_ea_pcdi(drc)
#define DRC_EA_PCDI_16()   DRC_EA_PCDI_8()
#define DRC_EA_PCDI_32()   DRC_EA_PCDI_8()

/* pc indirect + index */
#define DRC_EA_PCIX_8()    m68kdrc_get_ea_pcix(drc)
#define DRC_EA_PCIX_16()   DRC_EA_PCIX_8()
#define DRC_EA_PCIX_32()   DRC_EA_PCIX_8()


#define DRC_OPER_I_8()     { uint32 ea = OPER_I_8(); _mov_r32_imm(REG_EAX, ea); } while (0)
#define DRC_OPER_I_16()    { uint32 ea = OPER_I_16(); _mov_r32_imm(REG_EAX, ea); } while (0)
#define DRC_OPER_I_32()    { uint32 ea = OPER_I_32(); _mov_r32_imm(REG_EAX, ea); } while (0)


/* --------------------------- Status Register ---------------------------- */

/*
  Flag Calculation Macros
	src = ECX, dst = EBX, result: EAX

DRC_NFLAG_8(AL)
DRC_NFLAG_16(AH)
DRC_NFLAG_32(EAX)
DRC_NFLAG_64(EDX)

DRC_VFLAG_ADD_8(EAX, EBX, ECX)
DRC_VFLAG_ADD_16(EAX, EBX, ECX)
DRC_VFLAG_ADD_32(EAX, EBX, ECX)
DRC_VFLAG_SUB_8(EAX, EBX, ECX)
DRC_VFLAG_SUB_16(EAX, EBX, ECX)
DRC_VFLAG_SUB_32(EAX, EBX, ECX)

DRC_CFLAG_8(AH)
DRC_CFLAG_16(EAX)
DRC_CFLAG_COND_C(COND_C)
DRC_CFLAG_ADD_32(EAX, EBX, ECX)
DRC_CFLAG_SUB_32(EAX, EBX, ECX)

DRC_CFLAG_NEG_8(AL, CL)
DRC_CFLAG_NEG_16(AH, CH)
DRC_CFLAG_NEG_32(EAX, ECX)
*/


#define DRC_NFLAG_8()	_mov_m8abs_r8(&FLAG_N, REG_AL)
#define DRC_NFLAG_16()	_mov_m8abs_r8(&FLAG_N, REG_AH)
#define DRC_NFLAG_32() \
	do \
	{						/* break ECX */ \
		_mov_r32_r32(REG_ECX, REG_EAX); \
		_shr_r32_imm(REG_ECX, 16); \
		_mov_m8abs_r8(&FLAG_N, REG_CH); \
	} while (0)
#define DRC_NFLAG_64() \
	do \
	{						/* break ECX */ \
		_mov_r32_r32(REG_ECX, REG_EDX); \
		_shr_r32_imm(REG_ECX, 16); \
		_mov_m8abs_r8(&FLAG_N, REG_CH); \
	} while (0)

#define DRC_VFLAG_ADD_8() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EAX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_mov_m8abs_r8(&FLAG_V, REG_CL); \
	} while (0)
#define DRC_VFLAG_ADD_16() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EAX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_mov_m8abs_r8(&FLAG_V, REG_CH); \
	} while (0)
#define DRC_VFLAG_ADD_32() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EAX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_shr_r32_imm(REG_ECX, 16); \
		_mov_m8abs_r8(&FLAG_V, REG_CH); \
	} while (0)

#define DRC_VFLAG_SUB_8() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EBX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_mov_m8abs_r8(&FLAG_V, REG_CL); \
	} while (0)
#define DRC_VFLAG_SUB_16() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EBX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_mov_m8abs_r8(&FLAG_V, REG_CH); \
	} while (0)
#define DRC_VFLAG_SUB_32() \
	do \
	{						/* break EBX, ECX */ \
		_xor_r32_r32(REG_ECX, REG_EBX); \
		_xor_r32_r32(REG_EBX, REG_EAX); \
		_and_r32_r32(REG_ECX, REG_EBX); \
		_shr_r32_imm(REG_ECX, 16); \
		_mov_m8abs_r8(&FLAG_V, REG_CH); \
	} while (0)

#define DRC_VFLAG_NEG_8() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_ECX); \
		_and_r32_r32(REG_EBX, REG_EAX); \
		_mov_m8abs_r8(&FLAG_V, REG_BL); \
	} while (0)

#define DRC_VFLAG_NEG_16() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_ECX); \
		_and_r32_r32(REG_EBX, REG_EAX); \
		_mov_m8abs_r8(&FLAG_V, REG_BH); \
	} while (0)

#define DRC_VFLAG_NEG_32() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_ECX); \
		_and_r32_r32(REG_EBX, REG_EAX); \
		_shr_r32_imm(REG_EBX, 16); \
		_mov_m8abs_r8(&FLAG_V, REG_BH); \
	} while (0)

#define DRC_CFLAG_COND_C() \
	do \
	{ \
		_setcc_m8abs(COND_C, ((uint8 *)&FLAG_C) + 1); \
	} while (0)

#define DRC_CFLAG_8() \
	do \
	{ \
		_mov_m16abs_r16(&FLAG_C, REG_AX); \
	} while (0)
#define DRC_CFLAG_16() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_EAX); \
		_shr_r32_imm(REG_EBX, 8); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)
#define DRC_CFLAG_ADD_32() \
	do \
	{						/* break EBX, ECX, EDX */ \
		_mov_r32_r32(REG_EDX, REG_ECX); \
		_and_r32_r32(REG_EDX, REG_EBX);	/* EDX = (S & D) */ \
		_or_r32_r32(REG_ECX, REG_EBX);	/* ECX = (S | D) */ \
		_mov_r32_r32(REG_EBX, REG_EAX); \
		_not_r32(REG_EBX); \
		_and_r32_r32(REG_EBX, REG_ECX);	/* EBX = (~R & (S | D)) */ \
		_or_r32_r32(REG_EBX, REG_EDX);	/* EBX = ((S & D) | (~R & (S | D))) */ \
		_shr_r32_imm(REG_EBX, 23); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)
#define DRC_CFLAG_SUB_32() \
	do \
	{						/* break EBX, ECX, EDX */ \
		_mov_r32_r32(REG_EDX, REG_ECX); \
		_and_r32_r32(REG_EDX, REG_EAX);	/* EDX = (S & R) */ \
		_or_r32_r32(REG_ECX, REG_EAX);	/* ECX = (S | R) */ \
		_not_r32(REG_EBX); \
		_and_r32_r32(REG_EBX, REG_ECX);	/* EBX = (~D & (S | R)) */ \
		_or_r32_r32(REG_EBX, REG_EDX);	/* EBX = ((S & R) | (~D & (S | R))) */ \
		_shr_r32_imm(REG_EBX, 23); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)

#define DRC_CFLAG_NEG_8() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_EAX); \
		_or_r32_r32(REG_EBX, REG_ECX); \
		_shl_r32_imm(REG_EBX, 1); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)
#define DRC_CFLAG_NEG_16() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_EAX); \
		_or_r32_r32(REG_EBX, REG_ECX); \
		_shr_r32_imm(REG_EBX, 7); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)
#define DRC_CFLAG_NEG_32() \
	do \
	{						/* break EBX */ \
		_mov_r32_r32(REG_EBX, REG_EAX); \
		_or_r32_r32(REG_EBX, REG_ECX); \
		_shr_r32_imm(REG_EBX, 23); \
		_mov_m16abs_r16(&FLAG_C, REG_BX); \
	} while (0)

#define DRC_CXFLAG_COND_C() \
	do \
	{ \
		DRC_CFLAG_COND_C(); \
		_setcc_m8abs(COND_C, ((uint8 *)&FLAG_X) + 1); \
	} while (0)

#define DRC_CXFLAG_8() \
	do \
	{ \
 		DRC_CFLAG_8(); \
		_mov_m16abs_r16(&FLAG_X, REG_AX); \
	} while (0)
#define DRC_CXFLAG_16() \
	do \
	{ \
		DRC_CFLAG_16();				/* break EBX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)
#define DRC_CXFLAG_ADD_32() \
	do \
	{ \
		DRC_CFLAG_ADD_32();			/* break EBX, ECX, EDX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)
#define DRC_CXFLAG_SUB_32() \
	do \
	{ \
		DRC_CFLAG_SUB_32();			/* break EBX, ECX, EDX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)

#define DRC_CXFLAG_NEG_8() \
	do \
	{ \
 		DRC_CFLAG_NEG_8();			/* break EBX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)
#define DRC_CXFLAG_NEG_16() \
	do \
	{ \
		DRC_CFLAG_NEG_16();			/* break EBX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)
#define DRC_CXFLAG_NEG_32() \
	do \
	{ \
		DRC_CFLAG_NEG_32();			/* break EBX */ \
		_mov_m16abs_r16(&FLAG_X, REG_BX); \
	} while (0)


/* Flag values */
#define NFLAG_SET   0x80
#define NFLAG_CLEAR 0
#define CFLAG_SET   0x100
#define CFLAG_CLEAR 0
#define XFLAG_SET   0x100
#define XFLAG_CLEAR 0
#define VFLAG_SET   NFLAG_SET
#define VFLAG_CLEAR 0
#define ZFLAG_SET   0
#define ZFLAG_CLEAR 0xffffffff

#define SFLAG_SET   4
#define SFLAG_CLEAR 0
#define MFLAG_SET   2
#define MFLAG_CLEAR 0


/* copy XFLAG into Carry */
#define DRC_XFLAG_AS_COND_C() \
	do \
	{						/* fixme: break EDX */ \
		_mov_r8_m8abs(REG_DL, ((uint8 *)&FLAG_X) + 1); \
		_shr_r32_imm(REG_EDX, 1); \
	} while (0)


#define m68kdrc_cond(flag,value,cond) \
	_and_m32abs_imm(&flag, value); \
	_jcc_near_link(cond, &m68kdrc_link_make_cc)

#define m68kdrc_cond_nv(fail_cond) \
	_mov_r8_m8abs(REG_AL, &FLAG_N); \
	_mov_r8_m8abs(REG_BL, &FLAG_V); \
	_xor_r32_r32(REG_EAX, REG_EBX); \
	_and_r32_imm(REG_EAX, NFLAG_SET); 	/* M68K_COND_LT() if EAX != 0 */ \
	_jcc_near_link(fail_cond, &m68kdrc_link_make_cc)

#define m68kdrc_cond_hi(fail_cond) \
	_mov_r16_m16abs(REG_AX, &FLAG_C); \
	_and_r32_imm(REG_EAX, CFLAG_SET);	/* M68K_COND_CC() if EAX == 0 */ \
 \
	_test_m32abs_imm(&FLAG_Z, ZFLAG_CLEAR); \
	_setcc_r8(COND_Z, REG_AL);		/* M68K_COND_NE() if AL == 0 */ \
	_or_r32_r32(REG_EAX, REG_EAX);		/* M68K_COND_HI() if EAX == 0 */ \
	_jcc_near_link(fail_cond, &m68kdrc_link_make_cc)

#define m68kdrc_cond_gt(fail_cond) \
	_mov_r8_m8abs(REG_AL, &FLAG_N); \
	_mov_r8_m8abs(REG_BL, &FLAG_V); \
	_xor_r32_r32(REG_EAX, REG_EBX); \
	_and_r32_imm(REG_EAX, NFLAG_SET); 	/* M68K_COND_GE() if EAX == 0 */  \
 \
	_test_m32abs_imm(&FLAG_Z, ZFLAG_CLEAR); \
	_setcc_r8(COND_Z, REG_AH);		/* M68K_COND_NE() if AH == 0 */ \
	_or_r32_r32(REG_EAX, REG_EAX);		/* M68K_COND_GT() if EAX == 0 */ \
	_jcc_near_link(fail_cond, &m68kdrc_link_make_cc)


/* Conditions */
#define DRC_COND_CS() m68kdrc_cond(FLAG_C, CFLAG_SET, COND_Z)
#define DRC_COND_CC() m68kdrc_cond(FLAG_C, CFLAG_SET, COND_NZ)
#define DRC_COND_VS() m68kdrc_cond(FLAG_V, VFLAG_SET, COND_Z)
#define DRC_COND_VC() m68kdrc_cond(FLAG_V, VFLAG_SET, COND_NZ)
#define DRC_COND_NE() m68kdrc_cond(FLAG_Z, ZFLAG_CLEAR, COND_Z)
#define DRC_COND_EQ() m68kdrc_cond(FLAG_Z, ZFLAG_CLEAR, COND_NZ)
#define DRC_COND_MI() m68kdrc_cond(FLAG_N, NFLAG_SET, COND_Z)
#define DRC_COND_PL() m68kdrc_cond(FLAG_N, NFLAG_SET, COND_NZ)
#define DRC_COND_LT() m68kdrc_cond_nv(COND_Z)
#define DRC_COND_GE() m68kdrc_cond_nv(COND_NZ)
#define DRC_COND_HI() m68kdrc_cond_hi(COND_NZ)
#define DRC_COND_LS() m68kdrc_cond_hi(COND_Z)
#define DRC_COND_GT() m68kdrc_cond_gt(COND_NZ)
#define DRC_COND_LE() m68kdrc_cond_gt(COND_Z)


/* Reversed conditions */
#define DRC_COND_NOT_CS() DRC_COND_CC()
#define DRC_COND_NOT_CC() DRC_COND_CS()
#define DRC_COND_NOT_VS() DRC_COND_VC()
#define DRC_COND_NOT_VC() DRC_COND_VS()
#define DRC_COND_NOT_NE() DRC_COND_EQ()
#define DRC_COND_NOT_EQ() DRC_COND_NE()
#define DRC_COND_NOT_MI() DRC_COND_PL()
#define DRC_COND_NOT_PL() DRC_COND_MI()
#define DRC_COND_NOT_LT() DRC_COND_GE()
#define DRC_COND_NOT_GE() DRC_COND_LT()
#define DRC_COND_NOT_HI() DRC_COND_LS()
#define DRC_COND_NOT_LS() DRC_COND_HI()
#define DRC_COND_NOT_GT() DRC_COND_LE()
#define DRC_COND_NOT_LE() DRC_COND_GT()


/* Get the condition code register */
#define m68kdrc_get_ccr() \
	/* ___XNZVC */ \
	do \
	{ \
		link_info get_ccr_link1; \
 \
		_xor_r32_r32(REG_EAX, REG_EAX); \
 \
		/* (M68K_COND_XS() >> 4) : bit 4 */ \
		_mov_r16_m16abs(REG_DX, &FLAG_X); \
		_and_r32_imm(REG_EDX, XFLAG_SET); \
		_shr_r32_imm(REG_EDX, 4); \
		_or_r32_r32(REG_EAX, REG_EDX); \
 \
		/* (M68K_COND_MI() >> 4) : bit 3 */ \
		_mov_r8_m8abs(REG_DL, &FLAG_N); \
		_and_r32_imm(REG_EDX, NFLAG_SET); \
		_shr_r32_imm(REG_EDX, 4); \
		_or_r32_r32(REG_EAX, REG_EDX); \
 \
		/* (M68K_COND_EQ() << 2) : bit 2 */ \
		_mov_r32_m32abs(REG_EDX, &FLAG_Z); \
		_or_r32_r32(REG_EDX, REG_EDX); \
		_jcc_near_link(COND_NZ, &get_ccr_link1); \
		_or_r32_imm(REG_EAX, 1 << 2); \
		_resolve_link(&get_ccr_link1); \
 \
		/* (M68K_COND_VS() >> 6) : bit 1 */ \
		_mov_r8_m8abs(REG_DL, &FLAG_V); \
		_and_r32_imm(REG_EDX, VFLAG_SET); \
		_shr_r32_imm(REG_EDX, 6); \
		_or_r32_r32(REG_EAX, REG_EDX); \
 \
		/* (M68K_COND_CS() >> 8) : bit 0 */ \
		_mov_r16_m16abs(REG_DX, &FLAG_C); \
		_and_r32_imm(REG_EDX, CFLAG_SET); \
		_shr_r32_imm(REG_EDX, 8); \
		_or_r32_r32(REG_EAX, REG_EDX); \
	} while (0)

/* Get the status register */
#define m68kdrc_get_sr() \
	/* T1 T2 S M _ L2 L1 L0 */ \
	do \
	{ \
		m68kdrc_get_ccr(); \
 \
		/* T1 : bit 15 */ \
		_or_r32_m32abs(REG_EAX, &FLAG_T1); \
 \
		/* T1 : bit 14 */ \
		_or_r32_m32abs(REG_EAX, &FLAG_T0); \
 \
		/* (FLAG_S << 11) : bit 13 */ \
		_mov_r8_m8abs(REG_DL, &FLAG_S); \
		_and_r32_imm(REG_EDX, SFLAG_SET); \
		_shl_r32_imm(REG_EDX, 11); \
		_or_r32_r32(REG_EAX, REG_EDX); \
 \
		/* (FLAG_M << 11) : bit 12  */ \
		_mov_r8_m8abs(REG_DL, &FLAG_M); \
		_and_r32_imm(REG_EDX, MFLAG_SET); \
		_shl_r32_imm(REG_EDX, 11); \
		_or_r32_r32(REG_EAX, REG_EDX); \
 \
		/* FLAG_INT_MASK : bit 8-10 */ \
		_or_r32_m32abs(REG_EAX, &FLAG_INT_MASK); \
	} while (0)



/* ---------------------------- Cycle Counting ---------------------------- */

#define DRC_ADD_CYCLES(cycles)	do { uint32 val = cycles; if (val) _add_r32_imm(REG_EBP, val); } while (0)
#define DRC_USE_CYCLES(cycles)	do { uint32 val = cycles; if (val) _sub_r32_imm(REG_EBP, val); } while (0)
#define DRC_SET_CYCLES(cycles)	_mov_r32_imm(REG_EBP, cycles)
#define DRC_GET_CYCLES()	_mov_r32_r32(REG_EAX, REG_EBP)
#define DRC_USE_ALL_CYCLES()	_mov_r32_imm(REG_EBP, 0)


/* ----------------------------- Read / Write ----------------------------- */

/* Read from the current address space */
#define m68ki_read_8(A)  m68ki_read_8_fc (A, FLAG_S | m68ki_get_address_space())
#define m68ki_read_16(A) m68ki_read_16_fc(A, FLAG_S | m68ki_get_address_space())
#define m68ki_read_32(A) m68ki_read_32_fc(A, FLAG_S | m68ki_get_address_space())

/* Write to the current data space */
#define m68ki_write_8(A, V)  m68ki_write_8_fc (A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#define m68ki_write_16(A, V) m68ki_write_16_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#define m68ki_write_32(A, V) m68ki_write_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)

#if M68K_SIMULATE_PD_WRITES
#define m68ki_write_32_pd(A, V) m68ki_write_32_pd_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#else
#define m68ki_write_32_pd(A, V) m68ki_write_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA, V)
#endif

/* map read immediate 8 to read immediate 16 */
#define m68ki_read_imm_8() MASK_OUT_ABOVE_8(m68ki_read_imm_16())

/* Map PC-relative reads */
#define m68ki_read_pcrel_8(A) m68k_read_pcrelative_8(A)
#define m68ki_read_pcrel_16(A) m68k_read_pcrelative_16(A)
#define m68ki_read_pcrel_32(A) m68k_read_pcrelative_32(A)

/* Read from the program space */
#define m68ki_read_program_8(A) 	m68ki_read_8_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)
#define m68ki_read_program_16(A) 	m68ki_read_16_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)
#define m68ki_read_program_32(A) 	m68ki_read_32_fc(A, FLAG_S | FUNCTION_CODE_USER_PROGRAM)

/* Read from the data space */
#define m68ki_read_data_8(A) 	m68ki_read_8_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)
#define m68ki_read_data_16(A) 	m68ki_read_16_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)
#define m68ki_read_data_32(A) 	m68ki_read_32_fc(A, FLAG_S | FUNCTION_CODE_USER_DATA)



/* ======================================================================== */
/* =============================== PROTOTYPES ============================= */
/* ======================================================================== */

typedef union
{
	UINT64 i;
	double f;
} fp_reg;

typedef struct
{
	uint cpu_type;     /* CPU Type: 68000, 68008, 68010, 68EC020, or 68020 */
	uint dar[16];      /* Data and Address Registers */
	uint ppc;		   /* Previous program counter */
	uint pc;           /* Program Counter */
	uint sp[7];        /* User, Interrupt, and Master Stack Pointers */
	uint vbr;          /* Vector Base Register (m68010+) */
	uint sfc;          /* Source Function Code Register (m68010+) */
	uint dfc;          /* Destination Function Code Register (m68010+) */
	uint cacr;         /* Cache Control Register (m68020, unemulated) */
	uint caar;         /* Cache Address Register (m68020, unemulated) */
	uint ir;           /* Instruction Register */
    fp_reg fpr[8];     /* FPU Data Register (m68040) */
	uint fpiar;        /* FPU Instruction Address Register (m68040) */
	uint fpsr;         /* FPU Status Register (m68040) */
	uint fpcr;         /* FPU Control Register (m68040) */
	uint t1_flag;      /* Trace 1 */
	uint t0_flag;      /* Trace 0 */
	uint s_flag;       /* Supervisor */
	uint m_flag;       /* Master/Interrupt state */
	uint x_flag;       /* Extend */
	uint n_flag;       /* Negative */
	uint not_z_flag;   /* Zero, inverted for speedups */
	uint v_flag;       /* Overflow */
	uint c_flag;       /* Carry */
	uint int_mask;     /* I0-I2 */
	uint int_level;    /* State of interrupt pins IPL0-IPL2 -- ASG: changed from ints_pending */
	uint int_cycles;   /* ASG: extra cycles from generated interrupts */
	uint stopped;      /* Stopped state */
	uint pref_addr;    /* Last prefetch address */
	uint pref_data;    /* Data in the prefetch queue */
	uint address_mask; /* Available address pins */
	uint sr_mask;      /* Implemented status register bits */
	uint instr_mode;   /* Stores whether we are in instruction mode or group 0/1 exception mode */
	uint run_mode;     /* Stores whether we are processing a reset, bus error, address error, or something else */

	/* Clocks required for instructions / exceptions */
	uint cyc_bcc_notake_b;
	uint cyc_bcc_notake_w;
	uint cyc_dbcc_f_noexp;
	uint cyc_dbcc_f_exp;
	uint cyc_scc_r_true;
	uint cyc_movem_w;
	uint cyc_movem_l;
	uint cyc_shift;
	uint cyc_reset;
	uint8* cyc_instruction;
	uint8* cyc_exception;

	/* Callbacks to host */
	int  (*int_ack_callback)(int int_line);           /* Interrupt Acknowledge */
	void (*bkpt_ack_callback)(unsigned int data);     /* Breakpoint Acknowledge */
	void (*reset_instr_callback)(void);               /* Called when a RESET instruction is encountered */
	void (*cmpild_instr_callback)(unsigned int, int); /* Called when a CMPI.L #v, Dn instruction is encountered */
	void (*rte_instr_callback)(void);                 /* Called when a RTE instruction is encountered */
	void (*pc_changed_callback)(unsigned int new_pc); /* Called when the PC changes by a large amount */
	void (*set_fc_callback)(unsigned int new_fc);     /* Called when the CPU function code changes */
	void (*instr_hook_callback)(void);                /* Called every instruction cycle prior to execution */

	/* DRC staff */
	drc_core *drc;
	void *generate_exception_trap;
	void *generate_exception_trapN;
	void *generate_exception_trace;
	void *generate_exception_privilege_violation;
	void *generate_exception_1010;
	void *generate_exception_1111;
	void *generate_exception_illegal;
	void *generate_exception_format_error;
	void *generate_exception_address_error;
	void *generate_exception_interrupt;

	int *flag_dirty;
#ifdef MAME_DEBUG
	uint flags_dirty_mark;
#endif
} m68kdrc_cpu_core;


extern m68kdrc_cpu_core m68kdrc_cpu;
extern int            m68ki_initial_cycles;
extern sint           m68ki_remaining_cycles;
extern uint           m68ki_tracing;
extern uint8          m68ki_shift_8_table[];
extern uint16         m68ki_shift_16_table[];
extern uint           m68ki_shift_32_table[];
extern uint8          m68ki_exception_cycle_table[][256];
extern uint           m68ki_address_space;
extern uint8          m68ki_ea_idx_cycle_table[];

extern uint           m68ki_aerr_address;
extern uint           m68ki_aerr_write_mode;
extern uint           m68ki_aerr_fc;

/* Read data immediately after the program counter */
INLINE uint m68ki_read_imm_16(void);
INLINE uint m68ki_read_imm_32(void);

/* Read data with specific function code */
INLINE uint m68ki_read_8_fc  (uint address, uint fc);
INLINE uint m68ki_read_16_fc (uint address, uint fc);
INLINE uint m68ki_read_32_fc (uint address, uint fc);

/* Write data with specific function code */
INLINE void m68ki_write_8_fc (uint address, uint fc, uint value);
INLINE void m68ki_write_16_fc(uint address, uint fc, uint value);
INLINE void m68ki_write_32_fc(uint address, uint fc, uint value);
#if M68K_SIMULATE_PD_WRITES
INLINE void m68ki_write_32_pd_fc(uint address, uint fc, uint value);
#endif /* M68K_SIMULATE_PD_WRITES */

/* Indexed and PC-relative ea fetching */
INLINE void m68kdrc_get_ea_pcdi(drc_core *drc);
INLINE void m68kdrc_get_ea_pcix(drc_core *drc);
INLINE void m68kdrc_get_ea_ix(drc_core *drc, uint *pAn);

/* Program flow operations */
INLINE void m68kdrc_jump(drc_core *drc);
INLINE void m68kdrc_jump_vector(drc_core *drc);
INLINE void m68kdrc_branch_or_dispatch(drc_core *drc, uint32 newpc, int cycles, int eat_all_cycles);

/* Status register operations. */
INLINE void m68kdrc_set_s_flag(drc_core *drc, uint8 value);
INLINE void m68kdrc_set_sm_flag(drc_core *drc);
INLINE void m68kdrc_set_sm_flag_nosp(drc_core *drc);
INLINE void m68kdrc_set_ccr(drc_core *drc);
INLINE void m68kdrc_set_sr_noint(drc_core *drc);
INLINE void m68kdrc_set_sr_noint_nosp(drc_core *drc);
INLINE void m68kdrc_set_sr(drc_core *drc);

/* Exception processing */

/* quick disassembly (used for logging) */
char* m68ki_disassemble_quick(unsigned int pc, unsigned int cpu_type);


/* ======================================================================== */
/* =========================== UTILITY FUNCTIONS ========================== */
/* ======================================================================== */


/* ---------------------------- Read Immediate ---------------------------- */

/* Handles all immediate reads, does address error check, function code setting,
 * and prefetching if they are enabled in m68kconf.h
 */
INLINE uint m68ki_read_imm_16(void)
{
	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG68K_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */

#if M68K_EMULATE_PREFETCH
	if(MASK_OUT_BELOW_2(REG68K_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG68K_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
	}
	REG68K_PC += 2;
	return MASK_OUT_ABOVE_16(CPU_PREF_DATA >> ((2-((REG68K_PC-2)&2))<<3));
#else
	REG68K_PC += 2;
	return m68k_read_immediate_16(ADDRESS_68K(REG68K_PC-2));
#endif /* M68K_EMULATE_PREFETCH */
}
INLINE uint m68ki_read_imm_32(void)
{
#if M68K_EMULATE_PREFETCH
	uint temp_val;

	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG68K_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */

	if(MASK_OUT_BELOW_2(REG68K_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG68K_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
	}
	temp_val = CPU_PREF_DATA;
	REG68K_PC += 2;
	if(MASK_OUT_BELOW_2(REG68K_PC) != CPU_PREF_ADDR)
	{
		CPU_PREF_ADDR = MASK_OUT_BELOW_2(REG68K_PC);
		CPU_PREF_DATA = m68k_read_immediate_32(ADDRESS_68K(CPU_PREF_ADDR));
		temp_val = MASK_OUT_ABOVE_32((temp_val << 16) | (CPU_PREF_DATA >> 16));
	}
	REG68K_PC += 2;

	return temp_val;
#else
	m68ki_set_fc(FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error(REG68K_PC, MODE_READ, FLAG_S | FUNCTION_CODE_USER_PROGRAM); /* auto-disable (see m68kcpu.h) */

	REG68K_PC += 4;
	return m68k_read_immediate_32(ADDRESS_68K(REG68K_PC-4));
#endif /* M68K_EMULATE_PREFETCH */
}


/* ------------------------- Top level read/write ------------------------- */

/* Handles all memory accesses (except for immediate reads if they are
 * configured to use separate functions in m68kconf.h).
 * All memory accesses must go through these top level functions.
 * These functions will also check for address error and set the function
 * code if they are enabled in m68kconf.h.
 */
INLINE uint m68ki_read_8_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_8(ADDRESS_68K(address));
}
INLINE uint m68ki_read_16_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error_010_less(address, MODE_READ, fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_16(ADDRESS_68K(address));
}
INLINE uint m68ki_read_32_fc(uint address, uint fc)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error_010_less(address, MODE_READ, fc); /* auto-disable (see m68kcpu.h) */
	return m68k_read_memory_32(ADDRESS_68K(address));
}

INLINE void m68ki_write_8_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_8(ADDRESS_68K(address), value);
}
INLINE void m68ki_write_16_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error_010_less(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_16(ADDRESS_68K(address), value);
}
INLINE void m68ki_write_32_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error_010_less(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_32(ADDRESS_68K(address), value);
}

#if M68K_SIMULATE_PD_WRITES
INLINE void m68ki_write_32_pd_fc(uint address, uint fc, uint value)
{
	m68ki_set_fc(fc); /* auto-disable (see m68kcpu.h) */
	m68ki_check_address_error_010_less(address, MODE_WRITE, fc); /* auto-disable (see m68kcpu.h) */
	m68k_write_memory_32_pd(ADDRESS_68K(address), value);
}
#endif


/* DRC version */
INLINE void m68kdrc_append_save_call_restore(drc_core *drc, void *target, UINT32 stackadj)
{
	// save volatiles
	_mov_m32abs_r32(drc->icountptr, REG_EBP);
	_mov_m32abs_imm(drc->pcptr, REG68K_PC);

	// call	target
	_call(target);

	// restore volatiles
	_mov_r32_m32abs(REG_EBP, drc->icountptr);
	/* (don't change PC) */

	// adjust stack
	if (stackadj)
		_add_r32_imm(REG_ESP, stackadj);
}

extern uint8	m68kdrc_real_read_8(uint32 address);
extern uint16	m68kdrc_real_read_16(uint32 address);
extern uint32	m68kdrc_real_read_32(uint32 address);

extern uint8	m68kdrc_real_read_pcrel_8(uint32 address);
extern uint16	m68kdrc_real_read_pcrel_16(uint32 address);
extern uint32	m68kdrc_real_read_pcrel_32(uint32 address);

extern uint8	m68kdrc_real_read_data_8(uint32 address);
extern uint16	m68kdrc_real_read_data_16(uint32 address);
extern uint32	m68kdrc_real_read_data_32(uint32 address);

#define m68kdrc_read_8()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_8,  4)
#define m68kdrc_read_16()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_16, 4)
#define m68kdrc_read_32()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_32, 4)

#define m68kdrc_read_pcrel_8()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_pcrel_8,  4)
#define m68kdrc_read_pcrel_16()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_pcrel_16, 4)
#define m68kdrc_read_pcrel_32()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_pcrel_32, 4)

#define m68kdrc_read_data_8()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_data_8,  4)
#define m68kdrc_read_data_16()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_data_16, 4)
#define m68kdrc_read_data_32()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_data_32, 4)

#if M68K_EMULATE_FC	/* auto-disable (see m68kcpu.h) */
extern uint8	m68kdrc_real_read_8_fc(uint fc, uint32 address);
extern uint16	m68kdrc_real_read_16_fc(uint fc, uint32 address);
extern uint32	m68kdrc_real_read_32_fc(uint fc, uint32 address);

#define m68kdrc_read_8_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_8_fc,  8) } while (0)
#define m68kdrc_read_16_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_16_fc, 8) } while (0)
#define m68kdrc_read_32_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_read_32_fc, 8) } while (0)
#else
#define m68kdrc_read_8_fc(fc_ptr)	m68kdrc_read_8()
#define m68kdrc_read_16_fc(fc_ptr)	m68kdrc_read_16()
#define m68kdrc_read_32_fc(fc_ptr)	m68kdrc_read_32()
#endif


extern void m68kdrc_real_write_8(uint32 address, uint8 value);
extern void m68kdrc_real_write_16(uint32 address, uint16 value);
extern void m68kdrc_real_write_32(uint32 address, uint32 value);

#define m68kdrc_write_8()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_8,  8)
#define m68kdrc_write_16()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_16, 8)
#define m68kdrc_write_32()	m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_32, 8)

#if M68K_EMULATE_FC	/* auto-disable (see m68kcpu.h) */
extern uint8	m68kdrc_real_write_8_fc(uint fc, uint32 address, uint8 value);
extern uint16	m68kdrc_real_write_16_fc(uint fc, uint32 address, uint16 value);
extern uint32	m68kdrc_real_write_32_fc(uint fc, uint32 address, uint32 value);

#define m68kdrc_write_8_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_8_fc,  12) } while (0)
#define m68kdrc_write_16_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_16_fc, 12) } while (0)
#define m68kdrc_write_32_fc(fc_ptr)	do { _push_m32abs(fc_ptr); m68kdrc_append_save_call_restore(drc, (void *)(uint32)m68kdrc_real_write_32_fc, 12) } while (0)
#else
#define m68kdrc_write_8_fc(fc_ptr)	m68kdrc_write_8()
#define m68kdrc_write_16_fc(fc_ptr)	m68kdrc_write_16()
#define m68kdrc_write_32_fc(fc_ptr)	m68kdrc_write_32()
#endif


extern void m68kdrc_append_code_verify(drc_core *drc);
#define DRC_CODE_VERIFY(n)		do { m68kdrc_instr_size = (n); if (m68kdrc_check_code_modify) m68kdrc_append_code_verify(drc); } while (0)


/* --------------------- Effective Address Calculation -------------------- */

/* The program counter relative addressing modes cause operands to be
 * retrieved from program space, not data space.
 */
INLINE void m68kdrc_get_ea_pcdi(drc_core *drc)
{
	uint old_pc = REG68K_PC;
	uint32 ea = MAKE_INT_16(OPER_I_16());
	m68ki_use_program_space(); /* auto-disable */
	_mov_r32_imm(REG_EAX, old_pc + ea);
}

INLINE void m68kdrc_get_ea_pcix(drc_core *drc)
{
	m68ki_use_program_space(); /* auto-disable */
	_mov_m32abs_imm(&REG68K_PC, REG68K_PC);
	m68kdrc_get_ea_ix(drc, &REG68K_PC);
}

/* Indexed addressing modes are encoded as follows:
 *
 * Base instruction format:
 * F E D C B A 9 8 7 6 | 5 4 3 | 2 1 0
 * x x x x x x x x x x | 1 1 0 | BASE REGISTER      (An)
 *
 * Base instruction format for destination EA in move instructions:
 * F E D C | B A 9    | 8 7 6 | 5 4 3 2 1 0
 * x x x x | BASE REG | 1 1 0 | X X X X X X       (An)
 *
 * Brief extension format:
 *  F  |  E D C   |  B  |  A 9  | 8 | 7 6 5 4 3 2 1 0
 * D/A | REGISTER | W/L | SCALE | 0 |  DISPLACEMENT
 *
 * Full extension format:
 *  F     E D C      B     A 9    8   7    6    5 4       3   2 1 0
 * D/A | REGISTER | W/L | SCALE | 1 | BS | IS | BD SIZE | 0 | I/IS
 * BASE DISPLACEMENT (0, 16, 32 bit)                (bd)
 * OUTER DISPLACEMENT (0, 16, 32 bit)               (od)
 *
 * D/A:     0 = Dn, 1 = An                          (Xn)
 * W/L:     0 = W (sign extend), 1 = L              (.SIZE)
 * SCALE:   00=1, 01=2, 10=4, 11=8                  (*SCALE)
 * BS:      0=add base reg, 1=suppress base reg     (An suppressed)
 * IS:      0=add index, 1=suppress index           (Xn suppressed)
 * BD SIZE: 00=reserved, 01=NULL, 10=Word, 11=Long  (size of bd)
 *
 * IS I/IS Operation
 * 0  000  No Memory Indirect
 * 0  001  indir prex with null outer
 * 0  010  indir prex with word outer
 * 0  011  indir prex with long outer
 * 0  100  reserved
 * 0  101  indir postx with null outer
 * 0  110  indir postx with word outer
 * 0  111  indir postx with long outer
 * 1  000  no memory indirect
 * 1  001  mem indir with null outer
 * 1  010  mem indir with word outer
 * 1  011  mem indir with long outer
 * 1  100-111  reserved
 */
INLINE void m68kdrc_get_ea_ix(drc_core *drc, uint *pAn)
{
	uint extension = OPER_I_16();
	uint bd = 0;                        /* Base Displacement */
	uint od = 0;                        /* Outer Displacement */

	m68kdrc_instr_size += 2 - 10;	// adjust (10: maximum for code modify check)

	if(CPU_TYPE_IS_010_LESS(CPU_TYPE))
	{
		/* Calculate index */
		if(!BIT_B(extension))           /* W/L */
			_movsx_r32_m16abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */
		else
			_mov_r32_m32abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */

		/* Add base register and displacement and return */
		_add_r32_imm(REG_EAX, MAKE_INT_8(extension));
		_add_r32_m32abs(REG_EAX, pAn);

		return;
	}

	/* Brief extension format */
	if(!BIT_8(extension))
	{
		/* Calculate index */
		if(!BIT_B(extension))           /* W/L */
			_movsx_r32_m16abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */
		else
			_mov_r32_m32abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */

		/* Add scale if proper CPU type */
		if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		{
			if ((extension>>9) & 3)
				_shl_r32_imm(REG_EAX, (extension>>9) & 3);  /* SCALE */
		}

		/* Add base register and displacement and return */
		_add_r32_imm(REG_EAX, MAKE_INT_8(extension));
		_add_r32_m32abs(REG_EAX, pAn);

		return;
	}

	/* Full extension format */

	DRC_USE_CYCLES(m68ki_ea_idx_cycle_table[extension&0x3f]);

	/* Check if index is present */
	if(!BIT_6(extension))               /* IS */
	{
		/* Calculate index */
		if(!BIT_B(extension))           /* W/L */
			_movsx_r32_m16abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */
		else
			_mov_r32_m32abs(REG_EAX, &REG68K_DA[extension>>12]);     /* Xn */

		/* Add scale if proper CPU type */
		if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		{
			if ((extension>>9) & 3)
				_shl_r32_imm(REG_EAX, (extension>>9) & 3);  /* SCALE */
		}
	}
	else
		_xor_r32_r32(REG_EAX, REG_EAX);

	/* Check if base register is present */
	if(BIT_7(extension))                /* BS */
		pAn = NULL;                         /* An */

	/* Check if base displacement is present */
	if(BIT_5(extension))                /* BD SIZE */
	{
		if (BIT_4(extension))
		{
			bd = OPER_I_32();
			m68kdrc_instr_size += 4;
		}
		else
		{
			bd = MAKE_INT_16(OPER_I_16());
			m68kdrc_instr_size += 2;
		}
	}

	/* If no indirect action, we are done */
	if(!(extension&7))                  /* No Memory Indirect */
	{
		if (pAn)
			_add_r32_m32abs(REG_EAX, pAn);

		if (bd)
			_add_r32_imm(REG_EAX, bd);

		return;
	}

	/* Check if outer displacement is present */
	if(BIT_1(extension))                /* I/IS:  od */
	{
		if (BIT_0(extension))
		{
			od = OPER_I_32();
			m68kdrc_instr_size += 4;
		}
		else
		{
			od = MAKE_INT_16(OPER_I_16());
			m68kdrc_instr_size += 2;
		}
	}

	/* Postindex */
	if(BIT_2(extension))                /* I/IS:  0 = preindex, 1 = postindex */
	{
		_push_r32(REG_EAX);	/* save Xn */
		_mov_r32_imm(REG_EAX, bd);

		if (pAn)
			_add_r32_m32abs(REG_EAX, pAn);

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_pop_r32(REG_EBX);
		_add_r32_r32(REG_EAX, REG_EBX);

		if (od)
			_add_r32_imm(REG_EAX, od);
	}
	/* Preindex */
	else
	{
		if (bd)
			_add_r32_imm(REG_EAX, bd);

		if (pAn)
			_add_r32_m32abs(REG_EAX, pAn);

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		if (od)
			_add_r32_imm(REG_EAX, od);
	}
}


/* Fetch operands */
#define DRC_OPER_AY_AI_8()  do { DRC_EA_AY_AI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AY_AI_16() do { DRC_EA_AY_AI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AY_AI_32() do { DRC_EA_AY_AI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AY_PI_8()  do { DRC_EA_AY_PI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AY_PI_16() do { DRC_EA_AY_PI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AY_PI_32() do { DRC_EA_AY_PI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AY_PD_8()  do { DRC_EA_AY_PD_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AY_PD_16() do { DRC_EA_AY_PD_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AY_PD_32() do { DRC_EA_AY_PD_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AY_DI_8()  do { DRC_EA_AY_DI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AY_DI_16() do { DRC_EA_AY_DI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AY_DI_32() do { DRC_EA_AY_DI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AY_IX_8()  do { DRC_EA_AY_IX_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AY_IX_16() do { DRC_EA_AY_IX_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AY_IX_32() do { DRC_EA_AY_IX_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)

#define DRC_OPER_AX_AI_8()  do { DRC_EA_AX_AI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AX_AI_16() do { DRC_EA_AX_AI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AX_AI_32() do { DRC_EA_AX_AI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AX_PI_8()  do { DRC_EA_AX_PI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AX_PI_16() do { DRC_EA_AX_PI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AX_PI_32() do { DRC_EA_AX_PI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AX_PD_8()  do { DRC_EA_AX_PD_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AX_PD_16() do { DRC_EA_AX_PD_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AX_PD_32() do { DRC_EA_AX_PD_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AX_DI_8()  do { DRC_EA_AX_DI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AX_DI_16() do { DRC_EA_AX_DI_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AX_DI_32() do { DRC_EA_AX_DI_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AX_IX_8()  do { DRC_EA_AX_IX_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AX_IX_16() do { DRC_EA_AX_IX_16(); _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AX_IX_32() do { DRC_EA_AX_IX_32(); _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)

#define DRC_OPER_A7_PI_8()  do { DRC_EA_A7_PI_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_A7_PD_8()  do { DRC_EA_A7_PD_8();  _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)

#define DRC_OPER_AW_8()     do { DRC_EA_AW_8();     _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AW_16()    do { DRC_EA_AW_16();    _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AW_32()    do { DRC_EA_AW_32();    _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_AL_8()     do { DRC_EA_AL_8();     _push_r32(REG_EAX); m68kdrc_read_8();  } while (0)
#define DRC_OPER_AL_16()    do { DRC_EA_AL_16();    _push_r32(REG_EAX); m68kdrc_read_16(); } while (0)
#define DRC_OPER_AL_32()    do { DRC_EA_AL_32();    _push_r32(REG_EAX); m68kdrc_read_32(); } while (0)
#define DRC_OPER_PCDI_8()   do { DRC_EA_PCDI_8();   _push_r32(REG_EAX); m68kdrc_read_pcrel_8();  } while (0)
#define DRC_OPER_PCDI_16()  do { DRC_EA_PCDI_16();  _push_r32(REG_EAX); m68kdrc_read_pcrel_16(); } while (0)
#define DRC_OPER_PCDI_32()  do { DRC_EA_PCDI_32();  _push_r32(REG_EAX); m68kdrc_read_pcrel_32(); } while (0)
#define DRC_OPER_PCIX_8()   do { DRC_EA_PCIX_8();   _push_r32(REG_EAX); m68kdrc_read_pcrel_8();  } while (0)
#define DRC_OPER_PCIX_16()  do { DRC_EA_PCIX_16();  _push_r32(REG_EAX); m68kdrc_read_pcrel_16(); } while (0)
#define DRC_OPER_PCIX_32()  do { DRC_EA_PCIX_32();  _push_r32(REG_EAX); m68kdrc_read_pcrel_32(); } while (0)



/* ---------------------------- Stack Functions --------------------------- */

#define m68kdrc_push_16_imm(x)		do { _sub_m32abs_imm(&REG68K_SP, 2); _push_imm(x); _push_m32abs(&REG68K_SP); m68kdrc_write_16(); } while (0)
#define m68kdrc_push_32_imm(x)		do { _sub_m32abs_imm(&REG68K_SP, 4); _push_imm(x); _push_m32abs(&REG68K_SP); m68kdrc_write_32(); } while (0)
#define m68kdrc_push_16_m32abs(p)	do { _sub_m32abs_imm(&REG68K_SP, 2); _push_m32abs(p); _push_m32abs(&REG68K_SP); m68kdrc_write_16(); } while (0)
#define m68kdrc_push_32_m32abs(p)	do { _sub_m32abs_imm(&REG68K_SP, 4); _push_m32abs(p); _push_m32abs(&REG68K_SP); m68kdrc_write_32(); } while (0)
#define m68kdrc_push_16_r32(reg)	do { _sub_m32abs_imm(&REG68K_SP, 2); _push_r32(reg); _push_m32abs(&REG68K_SP); m68kdrc_write_16(); } while (0)
#define m68kdrc_push_32_r32(reg)	do { _sub_m32abs_imm(&REG68K_SP, 4); _push_r32(reg); _push_m32abs(&REG68K_SP); m68kdrc_write_32(); } while (0)

#define m68kdrc_pull_16()	do { _push_m32abs(&REG68K_SP); _add_m32abs_imm(&REG68K_SP, 2); m68kdrc_read_16(); } while (0)
#define m68kdrc_pull_32()	do { _push_m32abs(&REG68K_SP); _add_m32abs_imm(&REG68K_SP, 4); m68kdrc_read_32(); } while (0)
#define m68kdrc_fake_push_16()	do { _sub_m32abs_imm(&REG68K_SP, 2); } while (0)
#define m68kdrc_fake_push_32()	do { _sub_m32abs_imm(&REG68K_SP, 4); } while (0)
#define m68kdrc_fake_pull_16()	do { _add_m32abs_imm(&REG68K_SP, 2); } while (0)
#define m68kdrc_fake_pull_32()	do { _add_m32abs_imm(&REG68K_SP, 4); } while (0)


/* ----------------------------- Program Flow ----------------------------- */

/* Jump to a new program location or vector.
 * These functions will also call the pc_changed callback if it was enabled
 * in m68kconf.h.
 */
INLINE void m68kdrc_jump(drc_core *drc)
{
	link_info link1;

	_and_r32_imm(REG_EAX, CPU_ADDRESS_MASK);
	_mov_r32_r32(REG_EDI, REG_EAX);

	_cmp_r32_imm(REG_EDI, REG68K_PPC);
	_jcc_near_link(COND_NZ, &link1);

#if 0
	_push_imm(REG68K_PPC);
	_push_r32(REG_EAX);
	_push_imm("Eat all cycles (PC = %06x, PPC = %06x)\n");
	_call(printf);
	_add_r32_imm(REG_ESP, 12);
#endif

	DRC_USE_ALL_CYCLES();

_resolve_link(&link1);
	m68kdrc_pc_changed(REG_EDI);
	m68kdrc_recompile_flag |= RECOMPILE_ADD_DISPATCH | RECOMPILE_DONT_ADD_PCDELTA | RECOMPILE_END_OF_STRING;
}

/* REG_ESI: vector */
INLINE void m68kdrc_jump_vector(drc_core *drc)
{
	_mov_r32_m32abs(REG_EDX, &REG68K_VBR);
	_mov_r32_r32(REG_EAX, REG_ESI);
	_shl_r32_imm(REG_EAX, 2);
	_add_r32_r32(REG_EDX, REG_EAX);
	_push_r32(REG_EDX);

	m68kdrc_read_data_32();

	m68kdrc_jump(drc);
}

/* Branch to a new memory location.
 * The 32-bit branch will call pc_changed if it was enabled in m68kconf.h.
 * So far I've found no problems with not calling pc_changed for 8 or 16
 * bit branches.
 */
#define m68kdrc_branch_8(offset, eat_all_cycles)	m68kdrc_branch_or_dispatch(drc, REG68K_PC + MAKE_INT_8(offset), m68kdrc_cycles, eat_all_cycles)
#define m68kdrc_branch_16(offset, eat_all_cycles)	m68kdrc_branch_or_dispatch(drc, REG68K_PC + MAKE_INT_16(offset) - 2, m68kdrc_cycles, eat_all_cycles)
#define m68kdrc_branch_32(offset, eat_all_cycles)	m68kdrc_branch_or_dispatch(drc, REG68K_PC + MAKE_INT_32(offset) - 4, m68kdrc_cycles, eat_all_cycles)

INLINE void m68kdrc_branch_or_dispatch(drc_core *drc, uint32 newpc, int cycles, int eat_all_cycles)
{
	void *code = drc_get_code_at_pc(drc, newpc);

	_mov_r32_imm(REG_EDI, newpc);
	m68kdrc_pc_changed(REG_EDI);

	if (eat_all_cycles && newpc == REG68K_PPC)
	{
#if 0
		printf("%06x: eat all cycles\n", REG68K_PPC);
#endif
		//DRC_USE_ALL_CYCLES();
		DRC_SET_CYCLES(-cycles);
		_jmp(drc->out_of_cycles);
		return;
	}

	drc_append_standard_epilogue(drc, cycles, 0, 1);

	if (code)
		_jmp(code);
	else
		drc_append_tentative_fixed_dispatcher(drc, newpc);
}


/* ---------------------------- Status Register --------------------------- */

/* Set the S flag and change the active stack pointer.
 * Note that value MUST be 4 or 0.
 */
INLINE void m68kdrc_set_s_flag(drc_core *drc, uint8 value)
// in:		none
// break:	EBX, ECX, EDX
{
	uint8 new_s_flag = value & SFLAG_SET;

	_movzx_r32_m8abs(REG_EBX, &FLAG_S);
	_movzx_r32_m8abs(REG_ECX, &FLAG_M);

	_mov_r32_r32(REG_EDX, REG_EBX);
	_shr_r32_imm(REG_EDX, 1);
	_and_r32_r32(REG_EDX, REG_ECX);
	_or_r32_r32(REG_EBX, REG_EDX);
	_shl_r32_imm(REG_EBX, 2);

	_mov_r32_m32abs(REG_ECX, &REG68K_SP);
	_mov_m32bd_r32(REG_EBX, &REG68K_SP_BASE, REG_ECX);

	_mov_m8abs_imm(&FLAG_S, new_s_flag);

	if (new_s_flag)
	{
		_movzx_r32_m8abs(REG_ECX, &FLAG_M);
		_and_r32_imm(REG_ECX, new_s_flag >> 1);
		_or_r32_imm(REG_ECX, new_s_flag);
		_shl_r32_imm(REG_ECX, 2);
		_mov_r32_m32bd(REG_EBX, REG_ECX, &REG68K_SP_BASE);
	}
	else
	{
		_mov_r32_m32abs(REG_EBX, &REG68K_SP_BASE);
	}

	_mov_m32abs_r32(&REG68K_SP, REG_EBX);
}

/* Set the S and M flags and change the active stack pointer.
 * Note that value MUST be 0, 2, 4, or 6 (bit2 = S, bit1 = M).
 */
INLINE void m68kdrc_set_sm_flag(drc_core *drc)
// in:		EAX
// break:	EBX, ECX, EDX
{
	_movzx_r32_m8abs(REG_EBX, &FLAG_S);
	_movzx_r32_m8abs(REG_ECX, &FLAG_M);

	_mov_r32_r32(REG_EDX, REG_EBX);
	_shr_r32_imm(REG_EDX, 1);
	_and_r32_r32(REG_EDX, REG_ECX);
	_or_r32_r32(REG_EBX, REG_EDX);
	_shl_r32_imm(REG_EBX, 2);

	_mov_r32_m32abs(REG_ECX, &REG68K_SP);
	_mov_m32bd_r32(REG_EBX, &REG68K_SP_BASE, REG_ECX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, SFLAG_SET);
	_mov_m8abs_r8(&FLAG_S, REG_BL);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, MFLAG_SET);
	_mov_m8abs_r8(&FLAG_M, REG_CL);

	_mov_r32_r32(REG_EDX, REG_EBX);
	_shr_r32_imm(REG_EDX, 1);
	_and_r32_r32(REG_EDX, REG_ECX);
	_or_r32_r32(REG_EBX, REG_EDX);
	_shl_r32_imm(REG_EBX, 2);

	_mov_r32_m32bd(REG_ECX, REG_EBX, &REG68K_SP_BASE);
	_mov_m32abs_r32(&REG68K_SP, REG_ECX);
}

/* Set the S and M flags.  Don't touch the stack pointer. */
INLINE void m68kdrc_set_sm_flag_nosp(drc_core *drc)
// in:		EAX
// break:	EBX
{
	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, SFLAG_SET);
	_mov_m8abs_r8(&FLAG_S, REG_BL);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, MFLAG_SET);
	_mov_m8abs_r8(&FLAG_M, REG_BL);
}

/* Set the condition code register */
INLINE void m68kdrc_set_ccr(drc_core *drc)
// in:		EAX
// break:	EBX
{
	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 4);
	_shl_r32_imm(REG_EBX, 4);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 3);
	_shl_r32_imm(REG_EBX, 4);
	_mov_m8abs_r8(&FLAG_N, REG_BL);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 2);
	_xor_r32_imm(REG_EBX, 1 << 2);
	_mov_m32abs_r32(&FLAG_Z, REG_EBX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 1);
	_shl_r32_imm(REG_EBX, 6);
	_mov_m8abs_r8(&FLAG_V, REG_BL);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 0);
	_shl_r32_imm(REG_EBX, 8);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
}

/* Set the status register but don't check for interrupts */
INLINE void m68kdrc_set_sr_noint(drc_core *drc)
// in:		EAX
// break:	EAX, EBX
{
	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 15);
	_mov_m16abs_r16(&FLAG_T1, REG_BX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 14);
	_mov_m16abs_r16(&FLAG_T0, REG_BX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 0x0700);
	_mov_m16abs_r16(&FLAG_INT_MASK, REG_BX);

	m68kdrc_set_ccr(drc);
	_shr_r32_imm(REG_EAX, 11);
	m68kdrc_set_sm_flag(drc);
}

/* Set the status register but don't check for interrupts nor
 * change the stack pointer
 */
INLINE void m68kdrc_set_sr_noint_nosp(drc_core *drc)
// in:		EAX
// break:	EAX, EBX
{
	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 15);
	_mov_m16abs_r16(&FLAG_T1, REG_BX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 1 << 14);
	_mov_m16abs_r16(&FLAG_T0, REG_BX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, 0x0700);
	_mov_m16abs_r16(&FLAG_INT_MASK, REG_BX);

	m68kdrc_set_ccr(drc);
	_shr_r32_imm(REG_EAX, 11);
	m68kdrc_set_sm_flag_nosp(drc);
}

/* Set the status register and check for interrupts */
INLINE void m68kdrc_set_sr(drc_core *drc)
{
	m68kdrc_set_sr_noint(drc);
	m68kdrc_recompile_flag |= RECOMPILE_CHECK_INTERRUPTS;
}


/* ------------------- Update Condition Code Register --------------------- */

/* Check we have to update VNCZ falgs? */
INLINE int m68kdrc_update_vncz_check(void)
{
#if 1
	uint16 next_ir;

	m68kdrc_instr_size -= 2;
	next_ir = m68k_read_immediate_16(ADDRESS_68K(REG68K_PPC + m68kdrc_instr_size));

	if (INSTR_FLAG_DIRTY[next_ir])
	{
#ifdef MAME_DEBUG
		m68kdrc_recompile_flag |= RECOMPILE_VNCZ_FLAGS_DIRTY;
#endif
		return 0;
	}
#endif

	return 1;
}

/* Check we have to update VNCXZ falgs? */
INLINE int m68kdrc_update_vncxz_check(void)
{
#if 1
	uint16 next_ir;

	m68kdrc_instr_size -= 2;
	next_ir = m68k_read_immediate_16(ADDRESS_68K(REG68K_PPC + m68kdrc_instr_size));

	if (INSTR_FLAG_DIRTY[next_ir] == 2)
	{
#ifdef MAME_DEBUG
		m68kdrc_recompile_flag |= RECOMPILE_VNCXZ_FLAGS_DIRTY;
#endif
		return 0;
	}
#endif

	return 1;
}

INLINE void m68kdrc_vncz_flag_move_8(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		DRC_NFLAG_8();
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	}
}

INLINE void m68kdrc_vncz_flag_move_16(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		DRC_NFLAG_16();
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	}
}

INLINE void m68kdrc_vncz_flag_move_32(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		DRC_NFLAG_32();				/* break ECX */
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	}
}

INLINE void m68kdrc_vncxz_flag_add_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_ADD_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CXFLAG_8();

		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_add_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_ADD_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CXFLAG_16();			/* break EBX */

		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_add_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_CXFLAG_COND_C();
		DRC_VFLAG_ADD_32();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_addx_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_ADD_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CXFLAG_8();

		_mov_r8_m8abs(REG_BL, &FLAG_Z);
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m8abs_r8(&FLAG_Z, REG_BL);
	}
}

INLINE void m68kdrc_vncxz_flag_addx_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_ADD_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CXFLAG_16();			/* break EBX */

		_mov_r16_m16abs(REG_BX, &FLAG_Z);
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m16abs_r16(&FLAG_Z, REG_BX);
	}
}

INLINE void m68kdrc_vncxz_flag_addx_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_CXFLAG_COND_C();
		DRC_VFLAG_ADD_32();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_r32_m32abs(REG_EBX, &FLAG_Z);
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_EBX);
	}
}

INLINE void m68kdrc_vncxz_flag_sub_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_SUB_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CXFLAG_8();

		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_sub_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_SUB_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CXFLAG_16();			/* break EBX */

		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_sub_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_CXFLAG_COND_C();
		DRC_VFLAG_SUB_32();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncz_flag_sub_8(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_VFLAG_SUB_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CFLAG_8();

		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	}
}

INLINE void m68kdrc_vncz_flag_sub_16(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_VFLAG_SUB_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CFLAG_16();				/* break EBX */

		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncz_flag_sub_32(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_CFLAG_COND_C();
		DRC_VFLAG_SUB_16();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_subx_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_SUB_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CXFLAG_8();

		_mov_r8_m8abs(REG_BL, &FLAG_Z);		/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m8abs_r8(&FLAG_Z, REG_BL);
	}
}

INLINE void m68kdrc_vncxz_flag_subx_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_SUB_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CXFLAG_16();			/* break EBX */

		_mov_r16_m16abs(REG_BX, &FLAG_Z);	/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m16abs_r16(&FLAG_Z, REG_BX);
	}
}

INLINE void m68kdrc_vncxz_flag_subx_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_CXFLAG_COND_C();
		DRC_VFLAG_SUB_32();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_r32_m32abs(REG_EBX, &FLAG_Z);	/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncz_flag_cmp_8(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_VFLAG_SUB_8();			/* break EBX, ECX */
		DRC_NFLAG_8();
		DRC_CFLAG_8();

		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncz_flag_cmp_16(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_VFLAG_SUB_16();			/* break EBX, ECX */
		DRC_NFLAG_16();
		DRC_CFLAG_16();				/* break EBX */

		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncz_flag_cmp_32(drc_core *drc)
{
	if (m68kdrc_update_vncz_check())
	{
		DRC_CFLAG_COND_C();
		DRC_VFLAG_SUB_32();			/* break EBX, ECX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_neg_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_8();			/* break EBX */
		DRC_CXFLAG_NEG_8();			/* break EBX */
		DRC_NFLAG_8();

		_movzx_r32_r8(REG_EAX, REG_AL);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_neg_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_16();			/* break EBX */
		DRC_CXFLAG_NEG_16();			/* break EBX */
		DRC_NFLAG_16();

		_movzx_r32_r16(REG_EAX, REG_AX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_neg_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_32();			/* break EBX */
		DRC_CXFLAG_NEG_32();			/* break EBX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	}
}

INLINE void m68kdrc_vncxz_flag_negx_8(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_8();			/* break EBX */
		DRC_CXFLAG_NEG_8();			/* break EBX */
		DRC_NFLAG_8();

		_mov_r8_m8abs(REG_BL, &FLAG_Z);		/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m8abs_r8(&FLAG_Z, REG_BL);
	}
}

INLINE void m68kdrc_vncxz_flag_negx_16(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_16();			/* break EBX */
		DRC_CXFLAG_NEG_16();			/* break EBX */
		DRC_NFLAG_16();

		_mov_r16_m16abs(REG_BX, &FLAG_Z);	/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m16abs_r16(&FLAG_Z, REG_BX);
	}
}

INLINE void m68kdrc_vncxz_flag_negx_32(drc_core *drc)
{
	if (m68kdrc_update_vncxz_check())
	{
		DRC_VFLAG_NEG_32();			/* break EBX */
		DRC_CXFLAG_NEG_32();			/* break EBX */
		DRC_NFLAG_32();				/* break ECX */

		_mov_r32_m32abs(REG_EBX, &FLAG_Z);	/* break EBX */
		_or_r32_r32(REG_EBX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_EBX);
	}
}


/* ------------------------- Exception Processing ------------------------- */

#define m68kdrc_exception_pc_as_nextpc()	_mov_r32_imm(REG_EDI, REG68K_PC)
#define m68kdrc_exception_pc_as_previouspc()

#define m68kdrc_exception_cycles_add()		DRC_USE_CYCLES(m68kdrc_cycles)
#define m68kdrc_exception_cycles_remove()

#define m68kdrc_exception(reason,pc,cycles) \
	do \
	{ \
		m68kdrc_exception_pc_as_##pc(); \
		m68kdrc_exception_cycles_##cycles(); \
		_jmp(m68kdrc_cpu.generate_exception_##reason); \
	} while (0)

#define	m68kdrc_exception_trap(type)		do { _mov_r32_imm(REG_ESI, type); m68kdrc_exception(trap, nextpc, add); } while (0)
#define	m68kdrc_exception_trapN(n)		do { _mov_r32_imm(REG_ESI, EXCEPTION_TRAP_BASE + (n)); m68kdrc_exception(trapN, nextpc, add); } while (0)
#define	m68kdrc_exception_trace()		m68kdrc_exception(trace, nextpc, add)
#define	m68kdrc_exception_privilege_violation()	m68kdrc_exception(privilege_violation, previouspc, remove)
#define	m68kdrc_exception_1010()		m68kdrc_exception(1010, previouspc, remove)
#define	m68kdrc_exception_1111()		m68kdrc_exception(1111, previouspc, remove)
#define	m68kdrc_exception_illegal()		m68kdrc_exception(illegal, previouspc, remove)
#define	m68kdrc_exception_format_error()	m68kdrc_exception(format_error, nextpc, remove)
#define	m68kdrc_exception_address_error()	do { _mov_m16abs_imm(&REG68K_IR, REG68K_IR); m68kdrc_exception(address_error, nextpc, remove); } while (0)



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KCPU__HEADER */
