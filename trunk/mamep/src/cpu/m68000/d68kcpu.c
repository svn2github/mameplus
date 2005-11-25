/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */

#if 0
static const char* copyright_notice =
"MUSASHI\n"
"Version 3.3 (2001-01-29)\n"
"A portable Motorola M680x0 processor emulation engine.\n"
"Copyright 1998-2001 Karl Stenerud.  All rights reserved.\n"
"\n"
"This code may be freely used for non-commercial purpooses as long as this\n"
"copyright notice remains unaltered in the source code and any binary files\n"
"containing this code in compiled form.\n"
"\n"
"All other lisencing terms must be negotiated with the author\n"
"(Karl Stenerud).\n"
"\n"
"The latest version of this code can be obtained at:\n"
"http://kstenerud.cjb.net\n"
"\n"
"DRC conversion by BUT\n"
;
#endif


/* ======================================================================== */
/* ================================= NOTES ================================ */
/* ======================================================================== */



/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include "d68kops.h"
#include "d68kcpu.h"


/* ======================================================================== */
/* ================================ DEFINES =============================== */
/* ======================================================================== */

#define CACHE_SIZE			(8 * 1024 * 1024)
#define MAX_INSTRUCTIONS	512

#define ADD_CYCLES(A)    m68ki_remaining_cycles += (A)
#define USE_CYCLES(A)    m68ki_remaining_cycles -= (A)
#define SET_CYCLES(A)    m68ki_remaining_cycles = A
#define GET_CYCLES()     m68ki_remaining_cycles
#define USE_ALL_CYCLES() m68ki_remaining_cycles = 0


#if M68K_EMULATE_INT_ACK
	#if M68K_EMULATE_INT_ACK == OPT_SPECIFY_HANDLER
		#define m68ki_int_ack(A) M68K_INT_ACK_CALLBACK(A)
	#else
		#define m68ki_int_ack(A) CALLBACK_INT_ACK(A)
	#endif
#endif /* M68K_EMULATE_INT_ACK */

#if M68K_EMULATE_BKPT_ACK
	#if M68K_EMULATE_BKPT_ACK == OPT_SPECIFY_HANDLER
		#define m68ki_bkpt_ack(A) M68K_BKPT_ACK_CALLBACK(A)
	#else
		#define m68ki_bkpt_ack(A) CALLBACK_BKPT_ACK(A)
	#endif
#endif /* M68K_EMULATE_BKPT_ACK */

#if M68K_EMULATE_RESET
	#if M68K_EMULATE_RESET == OPT_SPECIFY_HANDLER
		#define m68ki_output_reset() M68K_RESET_CALLBACK()
	#else
		#define m68ki_output_reset() CALLBACK_RESET_INSTR()
	#endif
#endif /* M68K_EMULATE_RESET */

#if M68K_CMPILD_HAS_CALLBACK
	#if M68K_CMPILD_HAS_CALLBACK == OPT_SPECIFY_HANDLER
		#define m68ki_cmpild_callback(v,r) M68K_CMPILD_CALLBACK(v,r)
	#else
		#define m68ki_cmpild_callback(v,r) CALLBACK_CMPILD_INSTR(v,r)
	#endif
#else
	#define m68ki_cmpild_callback(v,r)
#endif /* M68K_CMPILD_HAS_CALLBACK */

#if M68K_RTE_HAS_CALLBACK
	#if M68K_RTE_HAS_CALLBACK == OPT_SPECIFY_HANDLER
		#define m68ki_rte_callback() M68K_RTE_CALLBACK()
	#else
		#define m68ki_rte_callback() CALLBACK_RTE_INSTR()
	#endif
#else
	#define m68ki_rte_callback()
#endif /* M68K_RTE_HAS_CALLBACK */

#if M68K_MONITOR_PC
	#if M68K_MONITOR_PC == OPT_SPECIFY_HANDLER
		#define m68ki_pc_changed(A) M68K_SET_PC_CALLBACK(ADDRESS_68K(A))
	#else
		#define m68ki_pc_changed(A) CALLBACK_PC_CHANGED(ADDRESS_68K(A))
	#endif
#endif /* M68K_MONITOR_PC */


/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

int m68kdrc_cycles;
int m68kdrc_recompile_flag;

int m68kdrc_update_vncz_flag;


/* ======================================================================== */
/* =============================== CALLBACKS ============================== */
/* ======================================================================== */

#if M68K_EMULATE_INT_ACK
int m68kdrc_call_int_ack(int int_line)
{
	return m68ki_int_ack(int_line);
}
#endif /* M68K_EMULATE_INT_ACK */

#if M68K_EMULATE_BKPT_ACK
void m68kdrc_call_bkpt_ack_callback(unsigned int data)
{
	m68ki_bkpt_ack(data);
}
#endif /* M68K_EMULATE_BKPT_ACK */

#if M68K_EMULATE_RESET
void m68kdrc_call_reset_instr_callback(void)
{
	m68ki_output_reset();
}
#endif /* M68K_EMULATE_RESET */

#if M68K_CMPILD_HAS_CALLBACK
void m68kdrc_call_cmpild_instr_callback(unsigned int val, int reg)
{
	m68ki_cmpild_callback(val, reg);
}
#endif /* M68K_CMPILD_HAS_CALLBACK */

#if M68K_RTE_HAS_CALLBACK
void m68kdrc_call_rte_callback(void)
{
	m68ki_rte_callback();
}
#endif /* M68K_RTE_HAS_CALLBACK */

#if M68K_MONITOR_PC
void m68kdrc_call_pc_changed_callback(unsigned int new_pc)
{
	m68ki_pc_changed(new_pc);
}
#endif /* M68K_MONITOR_PC */



/* ======================================================================== */
/* ========================= INTERNAL ROUTINES ============================ */
/* ======================================================================== */

/* ------------------------- Exception Processing ------------------------- */

/* Initiate exception processing */
static void m68kdrc_init_exception(drc_core *drc)
{
	/* Save the old status register */
	m68kdrc_get_sr();

	/* Turn off trace flag, clear pending traces */
	_mov_m16abs_imm(&FLAG_T0, 0);
	_mov_m16abs_imm(&FLAG_T1, 0);
	m68ki_clear_trace();

	/* Enter supervisor mode */
	m68kdrc_set_s_flag(drc, SFLAG_SET);
}

/* 3 word stack frame (68000 only) */
/* SR: REG_EAX */
static void m68kdrc_stack_frame_3word(drc_core *drc)
{
	_push_r32(REG_EAX);
	m68kdrc_push_32_r32(REG_EDI);
	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);
}

/* Format 0 stack frame.
 * This is the standard stack frame for 68010+.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_0000(drc_core *drc)
{
	/* Stack a 3-word frame if we are 68000 */
	if(CPU_TYPE == CPU_TYPE_000 || CPU_TYPE == CPU_TYPE_008)
	{
		m68kdrc_stack_frame_3word(drc);
	}
	else
	{

		_push_r32(REG_EAX);
		_mov_r32_r32(REG_EBX, REG_ESI);
		_shl_r32_imm(REG_EBX, 2);
		m68kdrc_push_16_r32(REG_EBX);

		m68kdrc_push_32_r32(REG_EDI);
		_pop_r32(REG_EAX);
		m68kdrc_push_16_r32(REG_EAX);
	}
}

/* Format 1 stack frame (68020).
 * For 68020, this is the 4 word throwaway frame.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_0001(drc_core *drc)
{
	_push_r32(REG_EAX);
	_mov_r32_r32(REG_EBX, REG_ESI);
	_shl_r32_imm(REG_EBX, 2);
	_or_r32_imm(REG_EBX, 0x1000);

	m68kdrc_push_16_r32(REG_EBX);

	m68kdrc_push_32_r32(REG_EDI);
	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);
}

/* Format 2 stack frame.
 * This is used only by 68020 for trap exceptions.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_0010(drc_core *drc)
{
	_push_r32(REG_EAX);
	m68kdrc_push_32_m32abs(&REG68K_PPC);
	_mov_r32_r32(REG_EBX, REG_ESI);
	_shl_r32_imm(REG_EBX, 2);
	_or_r32_imm(REG_EBX, 0x2000);
	m68kdrc_push_16_r32(REG_EBX);

	m68kdrc_push_32_r32(REG_EDI);
	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);
}

/* Bus error stack frame (68000 only).
 */
/* SR: REG_EAX */
static void m68kdrc_stack_frame_buserr(drc_core *drc)
{
	_push_r32(REG_EAX);

	m68kdrc_push_32_r32(REG_EDI);
	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);
	m68kdrc_push_16_m32abs(&REG68K_IR);
	m68kdrc_push_32_m32abs(&m68ki_aerr_address);	/* access address */
	/* 0 0 0 0 0 0 0 0 0 0 0 R/W I/N FC
	 * R/W  0 = write, 1 = read
	 * I/N  0 = instruction, 1 = not
	 * FC   3-bit function code
	 */
	_mov_r16_m16abs(REG_BX, &m68ki_aerr_write_mode);
	_or_r32_m32abs(REG_EBX, &CPU_INSTR_MODE);
	_or_r32_m32abs(REG_EBX, &m68ki_aerr_fc);
	m68kdrc_push_16_r32(REG_EBX);
}

#if 0
/* Format 8 stack frame (68010).
 * 68010 only.  This is the 29 word bus/address error frame.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_1000(drc_core *drc)
{
	_push_r32(REG_EAX);

	/* VERSION
	 * NUMBER
	 * INTERNAL INFORMATION, 16 WORDS
	 */
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();
	m68kdrc_fake_push_32();

	m68kdrc_push_16_imm(0);		/* INSTRUCTION INPUT BUFFER */
	m68kdrc_fake_push_16();		/* UNUSED, RESERVED (not written) */
	m68kdrc_push_16_imm(0);		/* DATA INPUT BUFFER */
	m68kdrc_fake_push_16();		/* UNUSED, RESERVED (not written) */
	m68kdrc_push_16_imm(0);		/* DATA OUTPUT BUFFER */
	m68kdrc_fake_push_16();		/* UNUSED, RESERVED (not written) */
	m68kdrc_push_32_imm(0);		/* FAULT ADDRESS */
	m68kdrc_push_16_imm(0);		/* SPECIAL STATUS WORD */

	_mov_r32_r32(REG_EBX, REG_ESI);
	_shl_r32_imm(REG_EBX, 2);
	_or_r32_imm(REG_EBX, 0x8000);
	m68kdrc_push_16_r32(REG_EBX);	/* 1000, VECTOR OFFSET */

	/* PROGRAM COUNTER */
	m68kdrc_push_32_r32(REG_EDI);

	/* STATUS REGISTER */
	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);
}

/* Format A stack frame (short bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens at an instruction boundary.
 * PC stacked is address of next instruction.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_1010(drc_core *drc)
{
	_push_r32(REG_EAX);

	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_32_imm(0);		/* DATA OUTPUT BUFFER (2 words) */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_32_imm(0);		/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68kdrc_push_16_imm(0);		/* INSTRUCTION PIPE STAGE B */
	m68kdrc_push_16_imm(0);		/* INSTRUCTION PIPE STAGE C */
	m68kdrc_push_16_imm(0);		/* SPECIAL STATUS REGISTER */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */

	_mov_r32_r32(REG_EBX, REG_ESI);
	_shl_r32_imm(REG_EBX, 2);
	_or_r32_imm(REG_EBX, 0xa000);
	m68kdrc_push_16_r32(REG_EBX);	/* 1010, VECTOR OFFSET */

	m68kdrc_push_32_r32(REG_EDI);	/* PROGRAM COUNTER */

	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);	/* STATUS REGISTER */
}

/* Format B stack frame (long bus fault).
 * This is used only by 68020 for bus fault and address error
 * if the error happens during instruction execution.
 * PC stacked is address of instruction in progress.
 */
/* SR: REG_EAX, vector: REG_ESI */
static void m68kdrc_stack_frame_1011(drc_core *drc)
{
	_push_r32(REG_EAX);

	m68kdrc_push_32_imm(0);		/* INTERNAL REGISTERS (18 words) */
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);
	m68kdrc_push_16_imm(0);		/* VERSION# (4 bits), INTERNAL INFORMATION */
	m68kdrc_push_32_imm(0);		/* INTERNAL REGISTERS (3 words) */
	m68kdrc_push_16_imm(0);
	m68kdrc_push_32_imm(0);		/* DATA INTPUT BUFFER (2 words) */
	m68kdrc_push_32_imm(0);		/* INTERNAL REGISTERS (2 words) */
	m68kdrc_push_32_imm(0);		/* STAGE B ADDRESS (2 words) */
	m68kdrc_push_32_imm(0);		/* INTERNAL REGISTER (4 words) */
	m68kdrc_push_32_imm(0);
	m68kdrc_push_32_imm(0);		/* DATA OUTPUT BUFFER (2 words) */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */
	m68kdrc_push_32_imm(0);		/* DATA CYCLE FAULT ADDRESS (2 words) */
	m68kdrc_push_16_imm(0);		/* INSTRUCTION PIPE STAGE B */
	m68kdrc_push_16_imm(0);		/* INSTRUCTION PIPE STAGE C */
	m68kdrc_push_16_imm(0);		/* SPECIAL STATUS REGISTER */
	m68kdrc_push_16_imm(0);		/* INTERNAL REGISTER */

	_mov_r32_r32(REG_EBX, REG_ESI);
	_shl_r32_imm(REG_EBX, 2);
	_or_r32_imm(REG_EBX, 0xb000);
	m68kdrc_push_16_r32(REG_EBX);	/* 1011, VECTOR OFFSET */

	m68kdrc_push_32_r32(REG_EDI);	/* PROGRAM COUNTER */

	_pop_r32(REG_EAX);
	m68kdrc_push_16_r32(REG_EAX);	/* STATUS REGISTER */
}
#endif

/* Used for Group 2 exceptions.
 * These stack a type 2 frame on the 020.
 */
/* REG_ESI: vector */
static void append_exception_trap(drc_core *drc)
{
	m68kdrc_init_exception(drc);

	if (CPU_TYPE_IS_010_LESS(CPU_TYPE))
		m68kdrc_stack_frame_0000(drc);
	else
		m68kdrc_stack_frame_0010(drc);

	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Trap#n stacks a 0 frame but behaves like group2 otherwise */
/* REG_ESI: vector */
static void append_exception_trapN(drc_core *drc)
{
	m68kdrc_init_exception(drc);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for trace mode */
static void append_exception_trace(drc_core *drc)
{
	m68kdrc_init_exception(drc);

	_mov_r32_imm(REG_ESI, EXCEPTION_TRACE);

	if(CPU_TYPE_IS_010_LESS(CPU_TYPE))
	{
		#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
		if(CPU_TYPE_IS_000(CPU_TYPE))
		{
			_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_NO);
		}
		#endif /* M68K_EMULATE_ADDRESS_ERROR */
		m68kdrc_stack_frame_0000(drc);
	}
	else
		m68kdrc_stack_frame_0010(drc);

	m68kdrc_jump_vector(drc);

	/* Trace nullifies a STOP instruction */
	_and_m32abs_imm(&CPU_STOPPED, ~STOP_LEVEL_STOP);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for privilege violation */
static void append_exception_privilege_violation(drc_core *drc)
{
	m68kdrc_init_exception(drc);

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_NO);
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	_mov_r32_imm(REG_ESI, EXCEPTION_PRIVILEGE_VIOLATION);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for A-Line instructions */
static void append_exception_1010(drc_core *drc)
{
#if M68K_LOG_1010_1111 == OPT_ON
	//M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: called 1010 instruction %04x (%s)\n",
	//	 m68kdrc_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PPC), REG68K_IR,
	//	 m68ki_disassemble_quick(ADDRESS_68K(REG68K_PPC))));
#endif

	m68kdrc_init_exception(drc);
	_mov_r32_imm(REG_ESI, EXCEPTION_1010);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for F-Line instructions */
static void append_exception_1111(drc_core *drc)
{
#if M68K_LOG_1010_1111 == OPT_ON
	//M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: called 1111 instruction %04x (%s)\n",
	//	 m68kdrc_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PPC), REG68K_IR,
	//	 m68ki_disassemble_quick(ADDRESS_68K(REG68K_PPC))));
#endif

	m68kdrc_init_exception(drc);
	_mov_r32_imm(REG_ESI, EXCEPTION_1111);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for illegal instructions */
static void append_exception_illegal(drc_core *drc)
{
	//M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: illegal instruction %04x (%s)\n",
	//	 m68kdrc_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PPC), REG68K_IR,
	//	 m68ki_disassemble_quick(ADDRESS_68K(REG68K_PPC))));

	m68kdrc_init_exception(drc);

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_NO);
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	_mov_r32_imm(REG_ESI, EXCEPTION_ILLEGAL_INSTRUCTION);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for format errror in RTE */
static void append_exception_format_error(drc_core *drc)
{
	m68kdrc_init_exception(drc);
	_mov_r32_imm(REG_ESI, EXCEPTION_FORMAT_ERROR);
	m68kdrc_stack_frame_0000(drc);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

	drc_append_dispatcher(drc);
}

/* Exception for address error */
static void append_exception_address_error(drc_core *drc)
{
	link_info link1;
	link_info link2;

	m68kdrc_init_exception(drc);

	/* If we were processing a bus error, address error, or reset,
	 * this is a catastrophic failure.
	 * Halt the CPU
	 */
	_cmp_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_BERR_AERR_RESET);
	_jcc_near_link(COND_NZ, &link1);

_push_imm(0x00ffff01);
_call(m68k_memory_intf.read8);	// m68k_read_memory_8
_add_r32_imm(REG_ESP, 4);
	_mov_m32abs_imm(&CPU_STOPPED, STOP_LEVEL_HALT);
	_jmp_near_link(&link2);

_resolve_link(&link1);
	_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_BERR_AERR_RESET);

	/* Note: This is implemented for 68000 only! */
	m68kdrc_stack_frame_buserr(drc);

	_mov_r32_imm(REG_ESI, EXCEPTION_ADDRESS_ERROR);
	m68kdrc_jump_vector(drc);

	/* Use up some clock cycles */
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_sub_r32_r32(REG_EBP, REG_EAX);

_resolve_link(&link2);
	drc_append_dispatcher(drc);
}


static void append_exception_interrupt(drc_core *drc)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;
	link_info link5;

	_sub_r32_imm(REG_ESP, 6);

	#if M68K_EMULATE_ADDRESS_ERROR == OPT_ON
	if(CPU_TYPE_IS_000(CPU_TYPE))
	{
		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_NO);
	}
	#endif /* M68K_EMULATE_ADDRESS_ERROR */

	/* Acknowledge the interrupt */
	_push_r32(REG_EAX);
	m68kdrc_int_ack(REG_EAX);
	_pop_r32(REG_EBX);

	/* Get the interrupt vector */
	_cmp_r32_imm(REG_EAX, M68K_INT_ACK_AUTOVECTOR);
	_jcc_near_link(COND_Z, &link1);

	_cmp_r32_imm(REG_EAX, M68K_INT_ACK_SPURIOUS);
	_jcc_near_link(COND_Z, &link2);

	_cmp_r32_imm(REG_EAX, 256);
	_jcc_near_link(COND_LE, &link3);

	//M68K_DO_LOG_EMU((M68K_LOG_FILEHANDLE "%s at %08x: Interrupt acknowledge returned invalid vector $%x\n",
	//		 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC), vector));
	drc_append_dispatcher(drc);

_resolve_link(&link1);
	/* Use the autovectors.  This is the most commonly used implementation */
	_mov_r32_imm(REG_EAX, EXCEPTION_INTERRUPT_AUTOVECTOR);
	_add_r32_r32(REG_EAX, REG_EBX);

	_jmp_near_link(&link4);

_resolve_link(&link2);
	/* Called if no devices respond to the interrupt acknowledge */
	_mov_r32_imm(REG_EAX, EXCEPTION_SPURIOUS_INTERRUPT);

_resolve_link(&link3);
_resolve_link(&link4);
	_mov_r32_r32(REG_ESI, REG_EAX);	// vector
	_push_r32(REG_EBX);		// int_level
	m68kdrc_init_exception(drc);
	_pop_r32(REG_EBX);
	_mov_m16bd_r16(REG_ESP, 4, REG_AX);

	/* Set the interrupt mask to the level of the one being serviced */
	_shl_r32_imm(REG_EBX, 8);
	_mov_m32abs_r32(&FLAG_INT_MASK, REG_EBX);

	/* Get the new PC */
	_mov_r32_r32(REG_EAX, REG_ESI);
	_shl_r32_imm(REG_EAX, 2);
	_add_r32_m32abs(REG_EAX, &REG68K_VBR);

	_push_r32(REG_EAX);
	m68kdrc_read_data_32();

	_or_r32_r32(REG_EAX, REG_EAX);
	_jcc_near_link(COND_NZ, &link5);

	/* If vector is uninitialized, call the uninitialized interrupt vector */
	_mov_r32_m32abs(REG_EAX, &REG68K_VBR);
	_add_r32_imm(REG_EAX, EXCEPTION_UNINITIALIZED_INTERRUPT << 2);

	_push_r32(REG_EAX);
	m68kdrc_read_data_32();

_resolve_link(&link5);
	_mov_m32bd_r32(REG_ESP, 0, REG_EAX);

	/* Generate a stack frame */
	_mov_r16_m16bd(REG_AX, REG_ESP, 4);
	m68kdrc_stack_frame_0000(drc);

	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link6;

		_and_m8abs_imm(&FLAG_M, MFLAG_SET);
		_jcc_near_link(COND_Z, &link6);
		_mov_r32_m32abs(REG_EAX, &REG68K_SP);

		/* Create throwaway frame */
		_mov_r16_m16bd(REG_AX, REG_ESP, 4);
		_mov_r8_m8abs(REG_AL, &FLAG_S);
		m68kdrc_set_sm_flag(drc);	/* clear M */

		_or_r32_imm(REG_EAX, 0x2000);	/* Same as SR in master stack frame except S is forced high */

		m68kdrc_stack_frame_0001(drc);

_resolve_link(&link6);
	}

	_mov_r32_m32bd(REG_EDI, REG_ESP, 0);
	_add_r32_imm(REG_ESP, 6);

	/* Defer cycle counting until later */
	_xor_r32_r32(REG_EAX, REG_EAX); \
	_mov_r8_m8bd(REG_AL, REG_ESI, CYC_EXCEPTION);
	_add_r32_m32abs(REG_EAX, &CPU_INT_CYCLES);
	_mov_m32abs_r32(&CPU_INT_CYCLES, REG_EAX);

#if !M68K_EMULATE_INT_ACK
	/* Automatically clear IRQ if we are not using an acknowledge scheme */
	_mov_m32abs_imm(&CPU_INT_LEVEL, 0);
#endif /* M68K_EMULATE_INT_ACK */

	drc_append_dispatcher(drc);
}


/* ------------------------------ DRC staff ------------------------------- */

static void append_check_interrupts(drc_core *drc)
{
	link_info link1;

	_mov_r32_m32abs(REG_EAX, &CPU_INT_LEVEL);
	_cmp_r32_m32abs(REG_EAX, &FLAG_INT_MASK);
	_jcc_near_link(COND_LE, &link1);

	_shr_r32_imm(REG_EAX, 8);
	_jmp_m32abs(&m68ki_cpu.generate_exception_interrupt);

_resolve_link(&link1);
}


#define append_generate_exception(reason) \
	do \
	{ \
		m68kdrc_cpu.generate_exception_##reason = drc->cache_top; \
		append_exception_##reason(drc); \
	} while (0)

static void m68kdrc_reset(drc_core *drc)
{
	//printf("DRC reset: %p\n", drc->cache_top);

	append_generate_exception(trap);
	append_generate_exception(trapN);
	append_generate_exception(trace);
	append_generate_exception(privilege_violation);
	append_generate_exception(1010);
	append_generate_exception(1111);
	append_generate_exception(illegal);
	append_generate_exception(format_error);
	append_generate_exception(address_error);
	append_generate_exception(interrupt);
}

static void m68kdrc_entrygen(drc_core *drc)
{
	//printf("DRC entrygen: %p\n", drc->cache_top);

#if 0
_call(cpu_getcurrentframe);
_push_r32(REG_EAX);
_push_imm("entry : %d\n");
_call(printf);
_add_r32_imm(REG_ESP, 8);
#endif
}

#ifdef MAME_DEBUG
static void check_stack(uint32 sp)
{
	static uint32 old_sp;

	if (!old_sp)
	{
		old_sp = sp;
	}
	else if (old_sp != sp)
	{
		printf("%08x: wrong sp\n", REG68K_PC);
		exit(1);
	}
}
#endif


static uint32 recompile_instruction(drc_core *drc, uint32 pc)
{
	REG68K_PPC = REG68K_PC = pc;
	REG68K_IR = m68ki_read_imm_16();

	//printf("recompile %08x: %04x\n", pc, REG68K_IR);
	m68kdrc_recompile_flag = 0;
	m68kdrc_cycles = CYC_INSTRUCTION[REG68K_IR];

#ifdef MAME_DEBUG
	/* check stack pointer */
	_push_r32(REG_ESP);
	_call(check_stack);
	_add_r32_imm(REG_ESP, 4);
#endif

	/* update previous PC */
	_mov_m32abs_r32(&REG68K_PPC, REG_EDI);

	/* do compile */
	m68kdrc_instruction_compile_table[REG68K_IR](drc);

	if (m68kdrc_recompile_flag & RECOMPILE_UNIMPLEMENTED)
		return m68kdrc_recompile_flag;

	if (m68kdrc_recompile_flag & RECOMPILE_DONT_ADD_PCDELTA)
		return RECOMPILE_SUCCESSFUL_CP(m68kdrc_cycles, 0);

	return RECOMPILE_SUCCESSFUL_CP(m68kdrc_cycles, REG68K_PC - pc);
}

static uint32 compile_one(drc_core *drc, uint32 pc)
{
	int pcdelta, cycles;
	UINT32 *opptr;
	uint32 result;

	/* register this instruction */
	drc_register_code_at_cache_top(drc, pc);

	/* get a pointer to the current instruction */
	change_pc(pc);
	opptr = cpu_opptr(pc);

	/* emit debugging and self-modifying code checks */
	drc_append_call_debugger(drc);
	drc_append_verify_code(drc, opptr, 4);

	/* compile the instruction */
	result = recompile_instruction(drc, pc);

	/* handle the results */		
	if (!(result & RECOMPILE_SUCCESSFUL))
	{
		printf("Unimplemented op %08X (%02X,%02X)\n", *opptr, *opptr >> 26, *opptr & 0x3f);
		exit(1);
	}
	pcdelta = (sint8)(result >> 24);
	cycles = (uint8)(result >> 16);

	/* epilogue */
	drc_append_standard_epilogue(drc, cycles, pcdelta, 1);

	if (result & RECOMPILE_CHECK_INTERRUPTS)
		append_check_interrupts(drc);

	if (result & RECOMPILE_ADD_DISPATCH)
		drc_append_dispatcher(drc);

	return (result & 0xffff) | ((uint8)cycles << 16) | ((uint8)pcdelta << 24);
}

static void m68kdrc_recompile(drc_core *drc)
{
	int remaining = MAX_INSTRUCTIONS;
	uint32 savepc = REG68K_PC;
	uint32 pc = savepc;

	//printf("recompile_callback @ PC=%08X\n", pc);

	m68kdrc_update_vncz_flag = 0;

	/* begin the sequence */
	drc_begin_sequence(drc, pc);

	/* loop until we hit an unconditional branch */
	while (--remaining != 0)
	{
		uint32 result;

		if (remaining == 1)
			m68kdrc_update_vncz_flag = 1;

		/* compile one instruction */
		result = compile_one(drc, pc);
		pc += (sint8)(result >> 24);
		if (result & RECOMPILE_END_OF_STRING)
			break;
	}

	/* add dispatcher just in case */
	if (remaining == 0)
		drc_append_dispatcher(drc);

	/* end the sequence */
	drc_end_sequence(drc);

	REG68K_PC = savepc;
}



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

/* Execute some instructions until we use up num_cycles clock cycles */
/* ASG: removed per-instruction interrupt checks */
int m68kdrc_execute(int num_cycles)
{
	/* Make sure we're not stopped */
	if(!CPU_STOPPED)
	{
		/* Set our pool of clock cycles available */
		SET_CYCLES(num_cycles);
		m68ki_initial_cycles = num_cycles;

		/* ASG: update cycles */
		USE_CYCLES(CPU_INT_CYCLES);
		CPU_INT_CYCLES = 0;

		/* Return point if we had an address error */
		m68ki_set_address_error_trap(); /* auto-disable (see m68kcpu.h) */

		/* Main loop.  Keep going until we run out of clock cycles */
		do
		{
			/* Set tracing accodring to T1. (T0 is done inside instruction) */
			m68ki_trace_t1(); /* auto-disable (see m68kcpu.h) */

			/* Set the address space for reads */
			m68ki_use_data_space(); /* auto-disable (see m68kcpu.h) */

			/* Call external hook to peek at CPU */
			m68ki_instr_hook(); /* auto-disable (see m68kcpu.h) */

			/* Compile and execute */
			drc_execute(m68kdrc_cpu.drc);

			/* Trace m68k_exception, if necessary */
			m68ki_exception_if_trace(); /* auto-disable (see m68kcpu.h) */
		} while(GET_CYCLES() > 0);

		/* set previous PC to current PC for the next entry into the loop */
		REG68K_PPC = REG68K_PC;

		/* ASG: update cycles */
		USE_CYCLES(CPU_INT_CYCLES);
		CPU_INT_CYCLES = 0;

		/* return how many clocks we used */
		return m68ki_initial_cycles - GET_CYCLES();
	}

	/* We get here if the CPU is stopped or halted */
	SET_CYCLES(0);
	CPU_INT_CYCLES = 0;

	return num_cycles;
}


void m68kdrc_init(void)
{
	static uint emulation_initialized = 0;

	/* The first call to this function initializes the opcode handler jump table */
	if(!emulation_initialized)
	{
		m68kdrc_build_opcode_table();
		INSTR_VNCZ_FLAG_DIRTY = m68kdrc_vncz_flag_dirty_table;
		emulation_initialized = 1;
	}

	m68k_set_int_ack_callback(NULL);
	m68k_set_bkpt_ack_callback(NULL);
	m68k_set_reset_instr_callback(NULL);
	m68k_set_cmpild_instr_callback(NULL);
	m68k_set_rte_instr_callback(NULL);
	m68k_set_pc_changed_callback(NULL);
	m68k_set_fc_callback(NULL);
	m68k_set_instr_hook_callback(NULL);
}

/* Pulse the RESET line on the CPU */
void m68kdrc_pulse_reset(void)
{
	if (!m68kdrc_cpu.drc)
	{
		drc_config drconfig;
		int abits;

		for (abits = 1; abits < 32; abits++)
			if (!(CPU_ADDRESS_MASK & (1 << abits)))
				break;

		/* fill in the config */
		memset(&drconfig, 0, sizeof(drconfig));
		drconfig.cache_size       = CACHE_SIZE;
		drconfig.max_instructions = MAX_INSTRUCTIONS;
		drconfig.address_bits     = abits;
		drconfig.lsbs_to_ignore   = 1;
		drconfig.uses_fp          = 1;	// m68kdrc doesn't use fp, but it crashes if uses_fp == 0
		drconfig.uses_sse         = 0;
		drconfig.pc_in_memory     = 0;
		drconfig.icount_in_memory = 0;
		drconfig.pcptr            = &REG68K_PC;
		drconfig.icountptr        = &GET_CYCLES();
		drconfig.esiptr           = NULL;
		drconfig.cb_recompile     = m68kdrc_recompile;
		drconfig.cb_reset         = m68kdrc_reset;
		drconfig.cb_entrygen      = m68kdrc_entrygen;

		m68kdrc_cpu.drc = drc_init(cpu_getactivecpu(), &drconfig);
	}

	m68k_pulse_reset();
}


void m68kdrc_exit(void)
{
	if (m68kdrc_cpu.drc)
		drc_exit(m68kdrc_cpu.drc);

	m68kdrc_cpu.drc = NULL;
}


/* memory access from drc code */

uint8  m68kdrc_real_read_8(uint32 address)
{
	return m68ki_read_8(address);
}

uint16 m68kdrc_real_read_16(uint32 address)
{
	return m68ki_read_16(address);
}

uint32 m68kdrc_real_read_32(uint32 address)
{
	return m68ki_read_32(address);
}

uint8  m68kdrc_real_read_pcrel_8(uint32 address)
{
	return m68ki_read_pcrel_8(address);
}

uint16 m68kdrc_real_read_pcrel_16(uint32 address)
{
	return m68ki_read_pcrel_16(address);
}

uint32 m68kdrc_real_read_pcrel_32(uint32 address)
{
	return m68ki_read_pcrel_32(address);
}

uint8  m68kdrc_real_read_data_8(uint32 address)
{
	return m68ki_read_data_8(address);
}

uint16 m68kdrc_real_read_data_16(uint32 address)
{
	return m68ki_read_data_16(address);
}

uint32 m68kdrc_real_read_data_32(uint32 address)
{
	return m68ki_read_data_32(address);
}

#if M68K_EMULATE_FC
uint8	m68kdrc_real_read_8_fc(uint fc, uint32 address)
{
	return m68ki_read_data_8_fc(address, fc);
}

uint16	m68kdrc_real_read_16_fc(uint fc, uint32 address)
{
	return m68ki_read_data_16_fc(address, fc);
}

uint32	m68kdrc_real_read_32_fc(uint fc, uint32 address)
{
	return m68ki_read_data_32_fc(address, fc);
}
#endif

void m68kdrc_real_write_8(uint32 address, uint8 value)
{
	m68ki_write_8(address, value);
}

void m68kdrc_real_write_16(uint32 address, uint16 value)
{
	m68ki_write_16(address, value);
}

void m68kdrc_real_write_32(uint32 address, uint32 value)
{
	m68ki_write_32(address, value);
}

#if M68K_EMULATE_FC
uint8	m68kdrc_real_write_8_fc(uint fc, uint32 address, uint8 value)
{
	m68ki_write_8_fc(address, fc, value);
}

uint16	m68kdrc_real_write_16_fc(uint fc, uint32 address, uint16 value)
{
	m68ki_write_16_fc(address, fc, value);
}

uint32	m68kdrc_real_write_32_fc(uint fc, uint32 address, uint32 value)
{
	m68ki_write_32_fc(address, fc, value);
}
#endif


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
