/*
must fix:
	callm
	chk
*/
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

/* Special thanks to Bart Trzynadlowski for his insight into the
 * undocumented features of this chip:
 *
 * http://dynarec.com/~bart/files/68knotes.txt
 */

/*
 * DRC conversion by BUT
 */


/* Input file for m68kmake
 * -----------------------
 *
 * All sections begin with 80 X's in a row followed by an end-of-line
 * sequence.
 * After this, m68kmake will expect to find one of the following section
 * identifiers:
 *    M68KMAKE_PROTOTYPE_HEADER      - header for opcode handler prototypes
 *    M68KMAKE_PROTOTYPE_FOOTER      - footer for opcode handler prototypes
 *    M68KMAKE_TABLE_HEADER          - header for opcode handler jumptable
 *    M68KMAKE_TABLE_FOOTER          - footer for opcode handler jumptable
 *    M68KMAKE_TABLE_BODY            - the table itself
 *    M68KMAKE_OPCODE_HANDLER_HEADER - header for opcode handler implementation
 *    M68KMAKE_OPCODE_HANDLER_FOOTER - footer for opcode handler implementation
 *    M68KMAKE_OPCODE_HANDLER_BODY   - body section for opcode handler implementation
 *
 * NOTE: M68KMAKE_OPCODE_HANDLER_BODY must be last in the file and
 *       M68KMAKE_TABLE_BODY must be second last in the file.
 *
 * The M68KMAKE_OPHANDLER_BODY section contains the opcode handler
 * primitives themselves.  Each opcode handler begins with:
 *    M68KMAKE_OP(A, B, C, D)
 *
 * where A is the opcode handler name, B is the size of the operation,
 * C denotes any special processing mode, and D denotes a specific
 * addressing mode.
 * For C and D where nothing is specified, use "."
 *
 * Example:
 *     M68KMAKE_OP(abcd, 8, rr, .)   abcd, size 8, register to register, default EA
 *     M68KMAKE_OP(abcd, 8, mm, ax7) abcd, size 8, memory to memory, register X is A7
 *     M68KMAKE_OP(tst, 16, ., pcix) tst, size 16, PCIX addressing
 *
 * All opcode handler primitives end with a closing curly brace "}" at column 1
 *
 * NOTE: Do not place a M68KMAKE_OP() directive inside the opcode handler,
 *       and do not put a closing curly brace at column 1 unless it is
 *       marking the end of the handler!
 *
 * Inside the handler, m68kmake will recognize M68KMAKE_GET_OPER_xx_xx,
 * M68KMAKE_GET_EA_xx_xx, and M68KMAKE_CC directives, and create multiple
 * opcode handlers to handle variations in the opcode handler.
 * Note: M68KMAKE_CC will only be interpreted in condition code opcodes.
 * As well, M68KMAKE_GET_EA_xx_xx and M68KMAKE_GET_OPER_xx_xx will only
 * be interpreted on instructions where the corresponding table entry
 * specifies multiple effective addressing modes.
 * Example:
 * clr       32  .     .     0100001010......  A+-DXWL...  U U U   12   6   4
 *
 * This table entry says that the clr.l opcde has 7 variations (A+-DXWL).
 * It is run in user or supervisor mode for all CPUs, and uses 12 cycles for
 * 68000, 6 cycles for 68010, and 4 cycles for 68020.
 */

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_PROTOTYPE_HEADER

#ifndef M68KOPS__HEADER
#define M68KOPS__HEADER

struct _drc_core;


/* ======================================================================== */
/* ============================ OPCODE HANDLERS =========================== */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_PROTOTYPE_FOOTER


/* Build the opcode handler table */
void m68kdrc_build_opcode_table(void);

extern void (*m68kdrc_instruction_compile_table[0x10000])(struct _drc_core *drc); /* opcode handler jump table */
extern int m68kdrc_vncz_flag_dirty_table[0x10000];
extern unsigned char m68ki_cycles[][0x10000];


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KOPS__HEADER */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_HEADER

/* ======================================================================== */
/* ========================= OPCODE TABLE BUILDER ========================= */
/* ======================================================================== */

#include "d68kops.h"

#define NUM_CPU_TYPES 4

void  (*m68kdrc_instruction_compile_table[0x10000])(struct _drc_core *drc); /* opcode handler jump table */
int m68kdrc_vncz_flag_dirty_table[0x10000];

/* This is used to generate the opcode handler jump table */
typedef struct
{
	void (*opcode_handler)(struct _drc_core *drc);        /* handler function */
	unsigned int  mask;                  /* mask on opcode */
	unsigned int  match;                 /* what to match after masking */
	unsigned char cycles[NUM_CPU_TYPES]; /* cycles each cpu type takes */
	int vncz_flag_dirty;
} opcode_handler_struct;


/* Opcode handler table */
static opcode_handler_struct m68k_opcode_handler_table[] =
{
/*   function                      mask    match    000  010  020  040 */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_FOOTER

	{0, 0, 0, {0, 0, 0, 0}, 0}
};


/* Build the opcode handler jump table */
void m68kdrc_build_opcode_table(void)
{
	opcode_handler_struct *ostruct;
	int instr;
	int i;
	int j;
	int k;

	for(i = 0; i < 0x10000; i++)
	{
		/* default to illegal */
		m68kdrc_instruction_compile_table[i] = m68kdrc_op_illegal;
		m68kdrc_vncz_flag_dirty_table[i] = 0;
		for(k=0;k<NUM_CPU_TYPES;k++)
			m68ki_cycles[k][i] = 0;
	}

	ostruct = m68k_opcode_handler_table;
	while(ostruct->mask != 0xf180)
	{
		for(i = 0;i < 0x10000;i++)
		{
			if((i & ostruct->mask) == ostruct->match)
			{
				m68kdrc_instruction_compile_table[i] = ostruct->opcode_handler;
				m68kdrc_vncz_flag_dirty_table[i] = ostruct->vncz_flag_dirty;
				for(k=0;k<NUM_CPU_TYPES;k++)
					m68ki_cycles[k][i] = ostruct->cycles[k];
			}
		}
		ostruct++;
	}
	while (ostruct->mask != 0xff00)
	{
		for (i = ostruct->match + 0x0200; i < 0x10000; i++)
		{
			if ((i & ostruct->mask) == ostruct->match)
			{
				m68kdrc_instruction_compile_table[i] = ostruct->opcode_handler;
				m68kdrc_vncz_flag_dirty_table[i] = ostruct->vncz_flag_dirty;
				for(k=0;k<NUM_CPU_TYPES;k++)
					m68ki_cycles[k][i] = ostruct->cycles[k];
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xff00)
	{
		for(i = 0;i <= 0xff;i++)
		{
			m68kdrc_instruction_compile_table[ostruct->match | i] = ostruct->opcode_handler;
			m68kdrc_vncz_flag_dirty_table[ostruct->match | i] = ostruct->vncz_flag_dirty;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xf1f8)
	{
		for(i = 0;i < 8;i++)
		{
			for(j = 0;j < 8;j++)
			{
				instr = ostruct->match | (i << 9) | j;
				m68kdrc_instruction_compile_table[instr] = ostruct->opcode_handler;
				m68kdrc_vncz_flag_dirty_table[instr] = ostruct->vncz_flag_dirty;
				for(k=0;k<NUM_CPU_TYPES;k++)
					m68ki_cycles[k][instr] = ostruct->cycles[k];
				if((instr & 0xf000) == 0xe000 && (!(instr & 0x20)))
					m68ki_cycles[0][instr] = m68ki_cycles[1][instr] = ostruct->cycles[k] + ((((j-1)&7)+1)<<1);
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xfff0)
	{
		for(i = 0;i <= 0x0f;i++)
		{
			m68kdrc_instruction_compile_table[ostruct->match | i] = ostruct->opcode_handler;
			m68kdrc_vncz_flag_dirty_table[ostruct->match | i] = ostruct->vncz_flag_dirty;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xf1ff)
	{
		for(i = 0;i <= 0x07;i++)
		{
			m68kdrc_instruction_compile_table[ostruct->match | (i << 9)] = ostruct->opcode_handler;
			m68kdrc_vncz_flag_dirty_table[ostruct->match | (i << 9)] = ostruct->vncz_flag_dirty;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | (i << 9)] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xfff8)
	{
		for(i = 0;i <= 0x07;i++)
		{
			m68kdrc_instruction_compile_table[ostruct->match | i] = ostruct->opcode_handler;
			m68kdrc_vncz_flag_dirty_table[ostruct->match | i] = ostruct->vncz_flag_dirty;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xffff)
	{
		m68kdrc_instruction_compile_table[ostruct->match] = ostruct->opcode_handler;
		m68kdrc_vncz_flag_dirty_table[ostruct->match] = ostruct->vncz_flag_dirty;
		for(k=0;k<NUM_CPU_TYPES;k++)
			m68ki_cycles[k][ostruct->match] = ostruct->cycles[k];
		ostruct++;
	}
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_HEADER

#include "d68kcpu.h"


/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_FOOTER

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_BODY

The following table is arranged as follows:

name:        Opcode mnemonic

size:        Operation size

spec proc:   Special processing mode:
                 .:    normal
                 s:    static operand
                 r:    register operand
                 rr:   register to register
                 mm:   memory to memory
                 er:   effective address to register
                 re:   register to effective address
                 dd:   data register to data register
                 da:   data register to address register
                 aa:   address register to address register
                 cr:   control register to register
                 rc:   register to control register
                 toc:  to condition code register
                 tos:  to status register
                 tou:  to user stack pointer
                 frc:  from condition code register
                 frs:  from status register
                 fru:  from user stack pointer
                 * for move.x, the special processing mode is a specific
                   destination effective addressing mode.

spec ea:     Specific effective addressing mode:
                 .:    normal
                 i:    immediate
                 d:    data register
                 a:    address register
                 ai:   address register indirect
                 pi:   address register indirect with postincrement
                 pd:   address register indirect with predecrement
                 di:   address register indirect with displacement
                 ix:   address register indirect with index
                 aw:   absolute word address
                 al:   absolute long address
                 pcdi: program counter relative with displacement
                 pcix: program counter relative with index
                 a7:   register specified in instruction is A7
                 ax7:  register field X of instruction is A7
                 ay7:  register field Y of instruction is A7
                 axy7: register fields X and Y of instruction are A7

bit pattern: Pattern to recognize this opcode.  "." means don't care.

allowed ea:  List of allowed addressing modes:
                 .: not present
                 A: address register indirect
                 +: ARI (address register indirect) with postincrement
                 -: ARI with predecrement
                 D: ARI with displacement
                 X: ARI with index
                 W: absolute word address
                 L: absolute long address
                 d: program counter indirect with displacement
                 x: program counter indirect with index
                 I: immediate
mode:        CPU operating mode for each cpu type.  U = user or supervisor,
             S = supervisor only, "." = opcode not present.

cpu cycles:  Base number of cycles required to execute this opcode on the
             specified CPU type.
             Use "." if CPU does not have this opcode.



              spec  spec                    allowed ea  mode     cpu cycles
name    size  proc   ea   bit pattern       A+-DXWLdxI  0 1 2 4  000 010 020 040  comments
======  ====  ====  ====  ================  ==========  = = = =  === === === === =============
M68KMAKE_TABLE_START
1010       0  .     .     1010............  ..........  U U U U   4   4   4   4
1111       0  .     .     1111............  ..........  U U U U   4   4   4   4
abcd       8  rr    .     1100...100000...  ..........  U U U U   6   6   4   4
abcd       8  mm    ax7   1100111100001...  ..........  U U U U  18  18  16  16
abcd       8  mm    ay7   1100...100001111  ..........  U U U U  18  18  16  16
abcd       8  mm    axy7  1100111100001111  ..........  U U U U  18  18  16  16
abcd       8  mm    .     1100...100001...  ..........  U U U U  18  18  16  16
add        8  er    d     1101...000000...  ..........  U U U U   4   4   2   2
add        8  er    .     1101...000......  A+-DXWLdxI  U U U U   4   4   2   2
add       16  er    d     1101...001000...  ..........  U U U U   4   4   2   2
add       16  er    a     1101...001001...  ..........  U U U U   4   4   2   2
add       16  er    .     1101...001......  A+-DXWLdxI  U U U U   4   4   2   2
add       32  er    d     1101...010000...  ..........  U U U U   6   6   2   2
add       32  er    a     1101...010001...  ..........  U U U U   6   6   2   2
add       32  er    .     1101...010......  A+-DXWLdxI  U U U U   6   6   2   2
add        8  re    .     1101...100......  A+-DXWL...  U U U U   8   8   4   4
add       16  re    .     1101...101......  A+-DXWL...  U U U U   8   8   4   4
add       32  re    .     1101...110......  A+-DXWL...  U U U U  12  12   4   4
adda      16  .     d     1101...011000...  ..........  U U U U   8   8   2   2
adda      16  .     a     1101...011001...  ..........  U U U U   8   8   2   2
adda      16  .     .     1101...011......  A+-DXWLdxI  U U U U   8   8   2   2
adda      32  .     d     1101...111000...  ..........  U U U U   6   6   2   2
adda      32  .     a     1101...111001...  ..........  U U U U   6   6   2   2
adda      32  .     .     1101...111......  A+-DXWLdxI  U U U U   6   6   2   2
addi       8  .     d     0000011000000...  ..........  U U U U   8   8   2   2
addi       8  .     .     0000011000......  A+-DXWL...  U U U U  12  12   4   4
addi      16  .     d     0000011001000...  ..........  U U U U   8   8   2   2
addi      16  .     .     0000011001......  A+-DXWL...  U U U U  12  12   4   4
addi      32  .     d     0000011010000...  ..........  U U U U  16  14   2   2
addi      32  .     .     0000011010......  A+-DXWL...  U U U U  20  20   4   4
addq       8  .     d     0101...000000...  ..........  U U U U   4   4   2   2
addq       8  .     .     0101...000......  A+-DXWL...  U U U U   8   8   4   4
addq      16  .     d     0101...001000...  ..........  U U U U   4   4   2   2
addq      16  .     a     0101...001001...  ..........  U U U U   4   4   2   2
addq      16  .     .     0101...001......  A+-DXWL...  U U U U   8   8   4   4
addq      32  .     d     0101...010000...  ..........  U U U U   8   8   2   2
addq      32  .     a     0101...010001...  ..........  U U U U   8   8   2   2
addq      32  .     .     0101...010......  A+-DXWL...  U U U U  12  12   4   4
addx       8  rr    .     1101...100000...  ..........  U U U U   4   4   2   2
addx      16  rr    .     1101...101000...  ..........  U U U U   4   4   2   2
addx      32  rr    .     1101...110000...  ..........  U U U U   8   6   2   2
addx       8  mm    ax7   1101111100001...  ..........  U U U U  18  18  12  12
addx       8  mm    ay7   1101...100001111  ..........  U U U U  18  18  12  12
addx       8  mm    axy7  1101111100001111  ..........  U U U U  18  18  12  12
addx       8  mm    .     1101...100001...  ..........  U U U U  18  18  12  12
addx      16  mm    .     1101...101001...  ..........  U U U U  18  18  12  12
addx      32  mm    .     1101...110001...  ..........  U U U U  30  30  12  12
and        8  er    d     1100...000000...  ..........  U U U U   4   4   2   2
and        8  er    .     1100...000......  A+-DXWLdxI  U U U U   4   4   2   2
and       16  er    d     1100...001000...  ..........  U U U U   4   4   2   2
and       16  er    .     1100...001......  A+-DXWLdxI  U U U U   4   4   2   2
and       32  er    d     1100...010000...  ..........  U U U U   6   6   2   2
and       32  er    .     1100...010......  A+-DXWLdxI  U U U U   6   6   2   2
and        8  re    .     1100...100......  A+-DXWL...  U U U U   8   8   4   4
and       16  re    .     1100...101......  A+-DXWL...  U U U U   8   8   4   4
and       32  re    .     1100...110......  A+-DXWL...  U U U U  12  12   4   4
andi      16  toc   .     0000001000111100  ..........  U U U U  20  16  12  12
andi      16  tos   .     0000001001111100  ..........  S S S S  20  16  12  12
andi       8  .     d     0000001000000...  ..........  U U U U   8   8   2   2
andi       8  .     .     0000001000......  A+-DXWL...  U U U U  12  12   4   4
andi      16  .     d     0000001001000...  ..........  U U U U   8   8   2   2
andi      16  .     .     0000001001......  A+-DXWL...  U U U U  12  12   4   4
andi      32  .     d     0000001010000...  ..........  U U U U  14  14   2   2
andi      32  .     .     0000001010......  A+-DXWL...  U U U U  20  20   4   4
asr        8  s     .     1110...000000...  ..........  U U U U   6   6   6   6
asr       16  s     .     1110...001000...  ..........  U U U U   6   6   6   6
asr       32  s     .     1110...010000...  ..........  U U U U   8   8   6   6
asr        8  r     .     1110...000100...  ..........  U U U U   6   6   6   6
asr       16  r     .     1110...001100...  ..........  U U U U   6   6   6   6
asr       32  r     .     1110...010100...  ..........  U U U U   8   8   6   6
asr       16  .     .     1110000011......  A+-DXWL...  U U U U   8   8   5   5
asl        8  s     .     1110...100000...  ..........  U U U U   6   6   8   8
asl       16  s     .     1110...101000...  ..........  U U U U   6   6   8   8
asl       32  s     .     1110...110000...  ..........  U U U U   8   8   8   8
asl        8  r     .     1110...100100...  ..........  U U U U   6   6   8   8
asl       16  r     .     1110...101100...  ..........  U U U U   6   6   8   8
asl       32  r     .     1110...110100...  ..........  U U U U   8   8   8   8
asl       16  .     .     1110000111......  A+-DXWL...  U U U U   8   8   6   6
bcc        8  .     .     0110............  ..........  U U U U  10  10   6   6
bcc       16  .     .     0110....00000000  ..........  U U U U  10  10   6   6
bcc       32  .     .     0110....11111111  ..........  U U U U  10  10   6   6
bchg       8  r     .     0000...101......  A+-DXWL...  U U U U   8   8   4   4
bchg      32  r     d     0000...101000...  ..........  U U U U   8   8   4   4
bchg       8  s     .     0000100001......  A+-DXWL...  U U U U  12  12   4   4
bchg      32  s     d     0000100001000...  ..........  U U U U  12  12   4   4
bclr       8  r     .     0000...110......  A+-DXWL...  U U U U   8  10   4   4
bclr      32  r     d     0000...110000...  ..........  U U U U  10  10   4   4
bclr       8  s     .     0000100010......  A+-DXWL...  U U U U  12  12   4   4
bclr      32  s     d     0000100010000...  ..........  U U U U  14  14   4   4
bfchg     32  .     d     1110101011000...  ..........  . . U U   .   .  12  12  timing not quite correct
bfchg     32  .     .     1110101011......  A..DXWL...  . . U U   .   .  20  20
bfclr     32  .     d     1110110011000...  ..........  . . U U   .   .  12  12
bfclr     32  .     .     1110110011......  A..DXWL...  . . U U   .   .  20  20
bfexts    32  .     d     1110101111000...  ..........  . . U U   .   .   8   8
bfexts    32  .     .     1110101111......  A..DXWLdx.  . . U U   .   .  15  15
bfextu    32  .     d     1110100111000...  ..........  . . U U   .   .   8   8
bfextu    32  .     .     1110100111......  A..DXWLdx.  . . U U   .   .  15  15
bfffo     32  .     d     1110110111000...  ..........  . . U U   .   .  18  18
bfffo     32  .     .     1110110111......  A..DXWLdx.  . . U U   .   .  28  28
bfins     32  .     d     1110111111000...  ..........  . . U U   .   .  10  10
bfins     32  .     .     1110111111......  A..DXWL...  . . U U   .   .  17  17
bfset     32  .     d     1110111011000...  ..........  . . U U   .   .  12  12
bfset     32  .     .     1110111011......  A..DXWL...  . . U U   .   .  20  20
bftst     32  .     d     1110100011000...  ..........  . . U U   .   .   6   6
bftst     32  .     .     1110100011......  A..DXWLdx.  . . U U   .   .  13  13
bkpt       0  .     .     0100100001001...  ..........  . U U U   .  10  10  10
bra        8  .     .     01100000........  ..........  U U U U  10  10  10  10
bra       16  .     .     0110000000000000  ..........  U U U U  10  10  10  10
bra       32  .     .     0110000011111111  ..........  U U U U  10  10  10  10
bset      32  r     d     0000...111000...  ..........  U U U U   8   8   4   4
bset       8  r     .     0000...111......  A+-DXWL...  U U U U   8   8   4   4
bset       8  s     .     0000100011......  A+-DXWL...  U U U U  12  12   4   4
bset      32  s     d     0000100011000...  ..........  U U U U  12  12   4   4
bsr        8  .     .     01100001........  ..........  U U U U  18  18   7   7
bsr       16  .     .     0110000100000000  ..........  U U U U  18  18   7   7
bsr       32  .     .     0110000111111111  ..........  U U U U  18  18   7   7
btst       8  r     .     0000...100......  A+-DXWLdxI  U U U U   4   4   4   4
btst      32  r     d     0000...100000...  ..........  U U U U   6   6   4   4
btst       8  s     .     0000100000......  A+-DXWLdx.  U U U U   8   8   4   4
btst      32  s     d     0000100000000...  ..........  U U U U  10  10   4   4
callm     32  .     .     0000011011......  A..DXWLdx.  . . U U   .   .  60  60  not properly emulated
cas        8  .     .     0000101011......  A+-DXWL...  . . U U   .   .  12  12
cas       16  .     .     0000110011......  A+-DXWL...  . . U U   .   .  12  12
cas       32  .     .     0000111011......  A+-DXWL...  . . U U   .   .  12  12
cas2      16  .     .     0000110011111100  ..........  . . U U   .   .  12  12
cas2      32  .     .     0000111011111100  ..........  . . U U   .   .  12  12
chk       16  .     d     0100...110000...  ..........  U U U U  10   8   8   8
chk       16  .     .     0100...110......  A+-DXWLdxI  U U U U  10   8   8   8
chk       32  .     d     0100...100000...  ..........  . . U U   .   .   8   8
chk       32  .     .     0100...100......  A+-DXWLdxI  . . U U   .   .   8   8
chk2cmp2   8  .     pcdi  0000000011111010  ..........  . . U U   .   .  23  23
chk2cmp2   8  .     pcix  0000000011111011  ..........  . . U U   .   .  23  23
chk2cmp2   8  .     .     0000000011......  A..DXWL...  . . U U   .   .  18  18
chk2cmp2  16  .     pcdi  0000001011111010  ..........  . . U U   .   .  23  23
chk2cmp2  16  .     pcix  0000001011111011  ..........  . . U U   .   .  23  23
chk2cmp2  16  .     .     0000001011......  A..DXWL...  . . U U   .   .  18  18
chk2cmp2  32  .     pcdi  0000010011111010  ..........  . . U U   .   .  23  23
chk2cmp2  32  .     pcix  0000010011111011  ..........  . . U U   .   .  23  23
chk2cmp2  32  .     .     0000010011......  A..DXWL...  . . U U   .   .  18  18
clr        8  .     d     0100001000000...  ..........  U U U U   4   4   2   2
clr        8  .     .     0100001000......  A+-DXWL...  U U U U   8   4   4   4
clr       16  .     d     0100001001000...  ..........  U U U U   4   4   2   2
clr       16  .     .     0100001001......  A+-DXWL...  U U U U   8   4   4   4
clr       32  .     d     0100001010000...  ..........  U U U U   6   6   2   2
clr       32  .     .     0100001010......  A+-DXWL...  U U U U  12   6   4   4
cmp        8  .     d     1011...000000...  ..........  U U U U   4   4   2   2
cmp        8  .     .     1011...000......  A+-DXWLdxI  U U U U   4   4   2   2
cmp       16  .     d     1011...001000...  ..........  U U U U   4   4   2   2
cmp       16  .     a     1011...001001...  ..........  U U U U   4   4   2   2
cmp       16  .     .     1011...001......  A+-DXWLdxI  U U U U   4   4   2   2
cmp       32  .     d     1011...010000...  ..........  U U U U   6   6   2   2
cmp       32  .     a     1011...010001...  ..........  U U U U   6   6   2   2
cmp       32  .     .     1011...010......  A+-DXWLdxI  U U U U   6   6   2   2
cmpa      16  .     d     1011...011000...  ..........  U U U U   6   6   4   4
cmpa      16  .     a     1011...011001...  ..........  U U U U   6   6   4   4
cmpa      16  .     .     1011...011......  A+-DXWLdxI  U U U U   6   6   4   4
cmpa      32  .     d     1011...111000...  ..........  U U U U   6   6   4   4
cmpa      32  .     a     1011...111001...  ..........  U U U U   6   6   4   4
cmpa      32  .     .     1011...111......  A+-DXWLdxI  U U U U   6   6   4   4
cmpi       8  .     d     0000110000000...  ..........  U U U U   8   8   2   2
cmpi       8  .     .     0000110000......  A+-DXWL...  U U U U   8   8   2   2
cmpi       8  .     pcdi  0000110000111010  ..........  . . U U   .   .   7   7
cmpi       8  .     pcix  0000110000111011  ..........  . . U U   .   .   9   9
cmpi      16  .     d     0000110001000...  ..........  U U U U   8   8   2   2
cmpi      16  .     .     0000110001......  A+-DXWL...  U U U U   8   8   2   2
cmpi      16  .     pcdi  0000110001111010  ..........  . . U U   .   .   7   7
cmpi      16  .     pcix  0000110001111011  ..........  . . U U   .   .   9   9
cmpi      32  .     d     0000110010000...  ..........  U U U U  14  12   2   2
cmpi      32  .     .     0000110010......  A+-DXWL...  U U U U  12  12   2   2
cmpi      32  .     pcdi  0000110010111010  ..........  . . U U   .   .   7   7
cmpi      32  .     pcix  0000110010111011  ..........  . . U U   .   .   9   9
cmpm       8  .     ax7   1011111100001...  ..........  U U U U  12  12   9   9
cmpm       8  .     ay7   1011...100001111  ..........  U U U U  12  12   9   9
cmpm       8  .     axy7  1011111100001111  ..........  U U U U  12  12   9   9
cmpm       8  .     .     1011...100001...  ..........  U U U U  12  12   9   9
cmpm      16  .     .     1011...101001...  ..........  U U U U  12  12   9   9
cmpm      32  .     .     1011...110001...  ..........  U U U U  20  20   9   9
cpbcc     32  .     .     1111...01.......  ..........  . . U .   .   .   4   .  unemulated
cpdbcc    32  .     .     1111...001001...  ..........  . . U .   .   .   4   .  unemulated
cpgen     32  .     .     1111...000......  ..........  . . U .   .   .   4   .  unemulated
cpscc     32  .     .     1111...001......  ..........  . . U .   .   .   4   .  unemulated
cptrapcc  32  .     .     1111...001111...  ..........  . . U .   .   .   4   .  unemulated
dbt       16  .     .     0101000011001...  ..........  U U U U  12  12   6   6
dbf       16  .     .     0101000111001...  ..........  U U U U  12  12   6   6
dbcc      16  .     .     0101....11001...  ..........  U U U U  12  12   6   6
divs      16  .     d     1000...111000...  ..........  U U U U 158 122  56  56
divs      16  .     .     1000...111......  A+-DXWLdxI  U U U U 158 122  56  56
divu      16  .     d     1000...011000...  ..........  U U U U 140 108  44  44
divu      16  .     .     1000...011......  A+-DXWLdxI  U U U U 140 108  44  44
divl      32  .     d     0100110001000...  ..........  . . U U   .   .  84  84
divl      32  .     .     0100110001......  A+-DXWLdxI  . . U U   .   .  84  84
eor        8  .     d     1011...100000...  ..........  U U U U   4   4   2   2
eor        8  .     .     1011...100......  A+-DXWL...  U U U U   8   8   4   4
eor       16  .     d     1011...101000...  ..........  U U U U   4   4   2   2
eor       16  .     .     1011...101......  A+-DXWL...  U U U U   8   8   4   4
eor       32  .     d     1011...110000...  ..........  U U U U   8   6   2   2
eor       32  .     .     1011...110......  A+-DXWL...  U U U U  12  12   4   4
eori      16  toc   .     0000101000111100  ..........  U U U U  20  16  12  12
eori      16  tos   .     0000101001111100  ..........  S S S S  20  16  12  12
eori       8  .     d     0000101000000...  ..........  U U U U   8   8   2   2
eori       8  .     .     0000101000......  A+-DXWL...  U U U U  12  12   4   4
eori      16  .     d     0000101001000...  ..........  U U U U   8   8   2   2
eori      16  .     .     0000101001......  A+-DXWL...  U U U U  12  12   4   4
eori      32  .     d     0000101010000...  ..........  U U U U  16  14   2   2
eori      32  .     .     0000101010......  A+-DXWL...  U U U U  20  20   4   4
exg       32  dd    .     1100...101000...  ..........  U U U U   6   6   2   2
exg       32  aa    .     1100...101001...  ..........  U U U U   6   6   2   2
exg       32  da    .     1100...110001...  ..........  U U U U   6   6   2   2
ext       16  .     .     0100100010000...  ..........  U U U U   4   4   4   4
ext       32  .     .     0100100011000...  ..........  U U U U   4   4   4   4
extb      32  .     .     0100100111000...  ..........  . . U U   .   .   4   4
illegal    0  .     .     0100101011111100  ..........  U U U U   4   4   4   4
jmp       32  .     .     0100111011......  A..DXWLdx.  U U U U   4   4   0   0
jsr       32  .     .     0100111010......  A..DXWLdx.  U U U U  12  12   0   0
lea       32  .     .     0100...111......  A..DXWLdx.  U U U U   0   0   2   2
link      16  .     a7    0100111001010111  ..........  U U U U  16  16   5   5
link      16  .     .     0100111001010...  ..........  U U U U  16  16   5   5
link      32  .     a7    0100100000001111  ..........  . . U U   .   .   6   6
link      32  .     .     0100100000001...  ..........  . . U U   .   .   6   6
lsr        8  s     .     1110...000001...  ..........  U U U U   6   6   4   4
lsr       16  s     .     1110...001001...  ..........  U U U U   6   6   4   4
lsr       32  s     .     1110...010001...  ..........  U U U U   8   8   4   4
lsr        8  r     .     1110...000101...  ..........  U U U U   6   6   6   6
lsr       16  r     .     1110...001101...  ..........  U U U U   6   6   6   6
lsr       32  r     .     1110...010101...  ..........  U U U U   8   8   6   6
lsr       16  .     .     1110001011......  A+-DXWL...  U U U U   8   8   5   5
lsl        8  s     .     1110...100001...  ..........  U U U U   6   6   4   4
lsl       16  s     .     1110...101001...  ..........  U U U U   6   6   4   4
lsl       32  s     .     1110...110001...  ..........  U U U U   8   8   4   4
lsl        8  r     .     1110...100101...  ..........  U U U U   6   6   6   6
lsl       16  r     .     1110...101101...  ..........  U U U U   6   6   6   6
lsl       32  r     .     1110...110101...  ..........  U U U U   8   8   6   6
lsl       16  .     .     1110001111......  A+-DXWL...  U U U U   8   8   5   5
move       8  d     d     0001...000000...  ..........  U U U U   4   4   2   2
move       8  d     .     0001...000......  A+-DXWLdxI  U U U U   4   4   2   2
move       8  ai    d     0001...010000...  ..........  U U U U   8   8   4   4
move       8  ai    .     0001...010......  A+-DXWLdxI  U U U U   8   8   4   4
move       8  pi    d     0001...011000...  ..........  U U U U   8   8   4   4
move       8  pi    .     0001...011......  A+-DXWLdxI  U U U U   8   8   4   4
move       8  pi7   d     0001111011000...  ..........  U U U U   8   8   4   4
move       8  pi7   .     0001111011......  A+-DXWLdxI  U U U U   8   8   4   4
move       8  pd    d     0001...100000...  ..........  U U U U   8   8   5   5
move       8  pd    .     0001...100......  A+-DXWLdxI  U U U U   8   8   5   5
move       8  pd7   d     0001111100000...  ..........  U U U U   8   8   5   5
move       8  pd7   .     0001111100......  A+-DXWLdxI  U U U U   8   8   5   5
move       8  di    d     0001...101000...  ..........  U U U U  12  12   5   5
move       8  di    .     0001...101......  A+-DXWLdxI  U U U U  12  12   5   5
move       8  ix    d     0001...110000...  ..........  U U U U  14  14   7   7
move       8  ix    .     0001...110......  A+-DXWLdxI  U U U U  14  14   7   7
move       8  aw    d     0001000111000...  ..........  U U U U  12  12   4   4
move       8  aw    .     0001000111......  A+-DXWLdxI  U U U U  12  12   4   4
move       8  al    d     0001001111000...  ..........  U U U U  16  16   6   6
move       8  al    .     0001001111......  A+-DXWLdxI  U U U U  16  16   6   6
move      16  d     d     0011...000000...  ..........  U U U U   4   4   2   2
move      16  d     a     0011...000001...  ..........  U U U U   4   4   2   2
move      16  d     .     0011...000......  A+-DXWLdxI  U U U U   4   4   2   2
move      16  ai    d     0011...010000...  ..........  U U U U   8   8   4   4
move      16  ai    a     0011...010001...  ..........  U U U U   8   8   4   4
move      16  ai    .     0011...010......  A+-DXWLdxI  U U U U   8   8   4   4
move      16  pi    d     0011...011000...  ..........  U U U U   8   8   4   4
move      16  pi    a     0011...011001...  ..........  U U U U   8   8   4   4
move      16  pi    .     0011...011......  A+-DXWLdxI  U U U U   8   8   4   4
move      16  pd    d     0011...100000...  ..........  U U U U   8   8   5   5
move      16  pd    a     0011...100001...  ..........  U U U U   8   8   5   5
move      16  pd    .     0011...100......  A+-DXWLdxI  U U U U   8   8   5   5
move      16  di    d     0011...101000...  ..........  U U U U  12  12   5   5
move      16  di    a     0011...101001...  ..........  U U U U  12  12   5   5
move      16  di    .     0011...101......  A+-DXWLdxI  U U U U  12  12   5   5
move      16  ix    d     0011...110000...  ..........  U U U U  14  14   7   7
move      16  ix    a     0011...110001...  ..........  U U U U  14  14   7   7
move      16  ix    .     0011...110......  A+-DXWLdxI  U U U U  14  14   7   7
move      16  aw    d     0011000111000...  ..........  U U U U  12  12   4   4
move      16  aw    a     0011000111001...  ..........  U U U U  12  12   4   4
move      16  aw    .     0011000111......  A+-DXWLdxI  U U U U  12  12   4   4
move      16  al    d     0011001111000...  ..........  U U U U  16  16   6   6
move      16  al    a     0011001111001...  ..........  U U U U  16  16   6   6
move      16  al    .     0011001111......  A+-DXWLdxI  U U U U  16  16   6   6
move      32  d     d     0010...000000...  ..........  U U U U   4   4   2   2
move      32  d     a     0010...000001...  ..........  U U U U   4   4   2   2
move      32  d     .     0010...000......  A+-DXWLdxI  U U U U   4   4   2   2
move      32  ai    d     0010...010000...  ..........  U U U U  12  12   4   4
move      32  ai    a     0010...010001...  ..........  U U U U  12  12   4   4
move      32  ai    .     0010...010......  A+-DXWLdxI  U U U U  12  12   4   4
move      32  pi    d     0010...011000...  ..........  U U U U  12  12   4   4
move      32  pi    a     0010...011001...  ..........  U U U U  12  12   4   4
move      32  pi    .     0010...011......  A+-DXWLdxI  U U U U  12  12   4   4
move      32  pd    d     0010...100000...  ..........  U U U U  12  14   5   5
move      32  pd    a     0010...100001...  ..........  U U U U  12  14   5   5
move      32  pd    .     0010...100......  A+-DXWLdxI  U U U U  12  14   5   5
move      32  di    d     0010...101000...  ..........  U U U U  16  16   5   5
move      32  di    a     0010...101001...  ..........  U U U U  16  16   5   5
move      32  di    .     0010...101......  A+-DXWLdxI  U U U U  16  16   5   5
move      32  ix    d     0010...110000...  ..........  U U U U  18  18   7   7
move      32  ix    a     0010...110001...  ..........  U U U U  18  18   7   7
move      32  ix    .     0010...110......  A+-DXWLdxI  U U U U  18  18   7   7
move      32  aw    d     0010000111000...  ..........  U U U U  16  16   4   4
move      32  aw    a     0010000111001...  ..........  U U U U  16  16   4   4
move      32  aw    .     0010000111......  A+-DXWLdxI  U U U U  16  16   4   4
move      32  al    d     0010001111000...  ..........  U U U U  20  20   6   6
move      32  al    a     0010001111001...  ..........  U U U U  20  20   6   6
move      32  al    .     0010001111......  A+-DXWLdxI  U U U U  20  20   6   6
movea     16  .     d     0011...001000...  ..........  U U U U   4   4   2   2
movea     16  .     a     0011...001001...  ..........  U U U U   4   4   2   2
movea     16  .     .     0011...001......  A+-DXWLdxI  U U U U   4   4   2   2
movea     32  .     d     0010...001000...  ..........  U U U U   4   4   2   2
movea     32  .     a     0010...001001...  ..........  U U U U   4   4   2   2
movea     32  .     .     0010...001......  A+-DXWLdxI  U U U U   4   4   2   2
move      16  frc   d     0100001011000...  ..........  . U U U   .   4   4   4
move      16  frc   .     0100001011......  A+-DXWL...  . U U U   .   8   4   4
move      16  toc   d     0100010011000...  ..........  U U U U  12  12   4   4
move      16  toc   .     0100010011......  A+-DXWLdxI  U U U U  12  12   4   4
move      16  frs   d     0100000011000...  ..........  U S S S   6   4   8   8  U only for 000
move      16  frs   .     0100000011......  A+-DXWL...  U S S S   8   8   8   8  U only for 000
move      16  tos   d     0100011011000...  ..........  S S S S  12  12   8   8
move      16  tos   .     0100011011......  A+-DXWLdxI  S S S S  12  12   8   8
move      32  fru   .     0100111001101...  ..........  S S S S   4   6   2   2
move      32  tou   .     0100111001100...  ..........  S S S S   4   6   2   2
movec     32  cr    .     0100111001111010  ..........  . S S S   .  12   6   6
movec     32  rc    .     0100111001111011  ..........  . S S S   .  10  12  12
movem     16  re    pd    0100100010100...  ..........  U U U U   8   8   4   4
movem     16  re    .     0100100010......  A..DXWL...  U U U U   8   8   4   4
movem     32  re    pd    0100100011100...  ..........  U U U U   8   8   4   4
movem     32  re    .     0100100011......  A..DXWL...  U U U U   8   8   4   4
movem     16  er    pi    0100110010011...  ..........  U U U U  12  12   8   8
movem     16  er    pcdi  0100110010111010  ..........  U U U U  16  16   9   9
movem     16  er    pcix  0100110010111011  ..........  U U U U  18  18  11  11
movem     16  er    .     0100110010......  A..DXWL...  U U U U  12  12   8   8
movem     32  er    pi    0100110011011...  ..........  U U U U  12  12   8   8
movem     32  er    pcdi  0100110011111010  ..........  U U U U  16  16   9   9
movem     32  er    pcix  0100110011111011  ..........  U U U U  18  18  11  11
movem     32  er    .     0100110011......  A..DXWL...  U U U U  12  12   8   8
movep     16  er    .     0000...100001...  ..........  U U U U  16  16  12  12
movep     32  er    .     0000...101001...  ..........  U U U U  24  24  18  18
movep     16  re    .     0000...110001...  ..........  U U U U  16  16  11  11
movep     32  re    .     0000...111001...  ..........  U U U U  24  24  17  17
moveq     32  .     .     0111...0........  ..........  U U U U   4   4   2   2
moves      8  .     .     0000111000......  A+-DXWL...  . S S S   .  14   5   5
moves     16  .     .     0000111001......  A+-DXWL...  . S S S   .  14   5   5
moves     32  .     .     0000111010......  A+-DXWL...  . S S S   .  16   5   5
move16    32  .     .     1111011000100...  ..........  . . . U   .   .   .   4  TODO: correct timing
muls      16  .     d     1100...111000...  ..........  U U U U  54  32  27  27
muls      16  .     .     1100...111......  A+-DXWLdxI  U U U U  54  32  27  27
mulu      16  .     d     1100...011000...  ..........  U U U U  54  30  27  27
mulu      16  .     .     1100...011......  A+-DXWLdxI  U U U U  54  30  27  27
mull      32  .     d     0100110000000...  ..........  . . U U   .   .  43  43
mull      32  .     .     0100110000......  A+-DXWLdxI  . . U U   .   .  43  43
nbcd       8  .     d     0100100000000...  ..........  U U U U   6   6   6   6
nbcd       8  .     .     0100100000......  A+-DXWL...  U U U U   8   8   6   6
neg        8  .     d     0100010000000...  ..........  U U U U   4   4   2   2
neg        8  .     .     0100010000......  A+-DXWL...  U U U U   8   8   4   4
neg       16  .     d     0100010001000...  ..........  U U U U   4   4   2   2
neg       16  .     .     0100010001......  A+-DXWL...  U U U U   8   8   4   4
neg       32  .     d     0100010010000...  ..........  U U U U   6   6   2   2
neg       32  .     .     0100010010......  A+-DXWL...  U U U U  12  12   4   4
negx       8  .     d     0100000000000...  ..........  U U U U   4   4   2   2
negx       8  .     .     0100000000......  A+-DXWL...  U U U U   8   8   4   4
negx      16  .     d     0100000001000...  ..........  U U U U   4   4   2   2
negx      16  .     .     0100000001......  A+-DXWL...  U U U U   8   8   4   4
negx      32  .     d     0100000010000...  ..........  U U U U   6   6   2   2
negx      32  .     .     0100000010......  A+-DXWL...  U U U U  12  12   4   4
nop        0  .     .     0100111001110001  ..........  U U U U   4   4   2   2
not        8  .     d     0100011000000...  ..........  U U U U   4   4   2   2
not        8  .     .     0100011000......  A+-DXWL...  U U U U   8   8   4   4
not       16  .     d     0100011001000...  ..........  U U U U   4   4   2   2
not       16  .     .     0100011001......  A+-DXWL...  U U U U   8   8   4   4
not       32  .     d     0100011010000...  ..........  U U U U   6   6   2   2
not       32  .     .     0100011010......  A+-DXWL...  U U U U  12  12   4   4
or         8  er    d     1000...000000...  ..........  U U U U   4   4   2   2
or         8  er    .     1000...000......  A+-DXWLdxI  U U U U   4   4   2   2
or        16  er    d     1000...001000...  ..........  U U U U   4   4   2   2
or        16  er    .     1000...001......  A+-DXWLdxI  U U U U   4   4   2   2
or        32  er    d     1000...010000...  ..........  U U U U   6   6   2   2
or        32  er    .     1000...010......  A+-DXWLdxI  U U U U   6   6   2   2
or         8  re    .     1000...100......  A+-DXWL...  U U U U   8   8   4   4
or        16  re    .     1000...101......  A+-DXWL...  U U U U   8   8   4   4
or        32  re    .     1000...110......  A+-DXWL...  U U U U  12  12   4   4
ori       16  toc   .     0000000000111100  ..........  U U U U  20  16  12  12
ori       16  tos   .     0000000001111100  ..........  S S S S  20  16  12  12
ori        8  .     d     0000000000000...  ..........  U U U U   8   8   2   2
ori        8  .     .     0000000000......  A+-DXWL...  U U U U  12  12   4   4
ori       16  .     d     0000000001000...  ..........  U U U U   8   8   2   2
ori       16  .     .     0000000001......  A+-DXWL...  U U U U  12  12   4   4
ori       32  .     d     0000000010000...  ..........  U U U U  16  14   2   2
ori       32  .     .     0000000010......  A+-DXWL...  U U U U  20  20   4   4
pack      16  rr    .     1000...101000...  ..........  . . U U   .   .   6   6
pack      16  mm    ax7   1000111101001...  ..........  . . U U   .   .  13  13
pack      16  mm    ay7   1000...101001111  ..........  . . U U   .   .  13  13
pack      16  mm    axy7  1000111101001111  ..........  . . U U   .   .  13  13
pack      16  mm    .     1000...101001...  ..........  . . U U   .   .  13  13
pea       32  .     .     0100100001......  A..DXWLdx.  U U U U   6   6   5   5
pflush    32  .     .     1111010100011000  ..........  . . . S   .   .   .   4   TODO: correct timing
reset      0  .     .     0100111001110000  ..........  S S S S   0   0   0   0
ror        8  s     .     1110...000011...  ..........  U U U U   6   6   8   8
ror       16  s     .     1110...001011...  ..........  U U U U   6   6   8   8
ror       32  s     .     1110...010011...  ..........  U U U U   8   8   8   8
ror        8  r     .     1110...000111...  ..........  U U U U   6   6   8   8
ror       16  r     .     1110...001111...  ..........  U U U U   6   6   8   8
ror       32  r     .     1110...010111...  ..........  U U U U   8   8   8   8
ror       16  .     .     1110011011......  A+-DXWL...  U U U U   8   8   7   7
rol        8  s     .     1110...100011...  ..........  U U U U   6   6   8   8
rol       16  s     .     1110...101011...  ..........  U U U U   6   6   8   8
rol       32  s     .     1110...110011...  ..........  U U U U   8   8   8   8
rol        8  r     .     1110...100111...  ..........  U U U U   6   6   8   8
rol       16  r     .     1110...101111...  ..........  U U U U   6   6   8   8
rol       32  r     .     1110...110111...  ..........  U U U U   8   8   8   8
rol       16  .     .     1110011111......  A+-DXWL...  U U U U   8   8   7   7
roxr       8  s     .     1110...000010...  ..........  U U U U   6   6  12  12
roxr      16  s     .     1110...001010...  ..........  U U U U   6   6  12  12
roxr      32  s     .     1110...010010...  ..........  U U U U   8   8  12  12
roxr       8  r     .     1110...000110...  ..........  U U U U   6   6  12  12
roxr      16  r     .     1110...001110...  ..........  U U U U   6   6  12  12
roxr      32  r     .     1110...010110...  ..........  U U U U   8   8  12  12
roxr      16  .     .     1110010011......  A+-DXWL...  U U U U   8   8   5   5
roxl       8  s     .     1110...100010...  ..........  U U U U   6   6  12  12
roxl      16  s     .     1110...101010...  ..........  U U U U   6   6  12  12
roxl      32  s     .     1110...110010...  ..........  U U U U   8   8  12  12
roxl       8  r     .     1110...100110...  ..........  U U U U   6   6  12  12
roxl      16  r     .     1110...101110...  ..........  U U U U   6   6  12  12
roxl      32  r     .     1110...110110...  ..........  U U U U   8   8  12  12
roxl      16  .     .     1110010111......  A+-DXWL...  U U U U   8   8   5   5
rtd       32  .     .     0100111001110100  ..........  . U U U   .  16  10  10
rte       32  .     .     0100111001110011  ..........  S S S S  20  24  20  20  bus fault not emulated
rtm       32  .     .     000001101100....  ..........  . . U U   .   .  19  19  not properly emulated
rtr       32  .     .     0100111001110111  ..........  U U U U  20  20  14  14
rts       32  .     .     0100111001110101  ..........  U U U U  16  16  10  10
sbcd       8  rr    .     1000...100000...  ..........  U U U U   6   6   4   4
sbcd       8  mm    ax7   1000111100001...  ..........  U U U U  18  18  16  16
sbcd       8  mm    ay7   1000...100001111  ..........  U U U U  18  18  16  16
sbcd       8  mm    axy7  1000111100001111  ..........  U U U U  18  18  16  16
sbcd       8  mm    .     1000...100001...  ..........  U U U U  18  18  16  16
st         8  .     d     0101000011000...  ..........  U U U U   6   4   4   4
st         8  .     .     0101000011......  A+-DXWL...  U U U U   8   8   6   6
sf         8  .     d     0101000111000...  ..........  U U U U   4   4   4   4
sf         8  .     .     0101000111......  A+-DXWL...  U U U U   8   8   6   6
scc        8  .     d     0101....11000...  ..........  U U U U   4   4   4   4
scc        8  .     .     0101....11......  A+-DXWL...  U U U U   8   8   6   6
stop       0  .     .     0100111001110010  ..........  S S S S   4   4   8   8
sub        8  er    d     1001...000000...  ..........  U U U U   4   4   2   2
sub        8  er    .     1001...000......  A+-DXWLdxI  U U U U   4   4   2   2
sub       16  er    d     1001...001000...  ..........  U U U U   4   4   2   2
sub       16  er    a     1001...001001...  ..........  U U U U   4   4   2   2
sub       16  er    .     1001...001......  A+-DXWLdxI  U U U U   4   4   2   2
sub       32  er    d     1001...010000...  ..........  U U U U   6   6   2   2
sub       32  er    a     1001...010001...  ..........  U U U U   6   6   2   2
sub       32  er    .     1001...010......  A+-DXWLdxI  U U U U   6   6   2   2
sub        8  re    .     1001...100......  A+-DXWL...  U U U U   8   8   4   4
sub       16  re    .     1001...101......  A+-DXWL...  U U U U   8   8   4   4
sub       32  re    .     1001...110......  A+-DXWL...  U U U U  12  12   4   4
suba      16  .     d     1001...011000...  ..........  U U U U   8   8   2   2
suba      16  .     a     1001...011001...  ..........  U U U U   8   8   2   2
suba      16  .     .     1001...011......  A+-DXWLdxI  U U U U   8   8   2   2
suba      32  .     d     1001...111000...  ..........  U U U U   6   6   2   2
suba      32  .     a     1001...111001...  ..........  U U U U   6   6   2   2
suba      32  .     .     1001...111......  A+-DXWLdxI  U U U U   6   6   2   2
subi       8  .     d     0000010000000...  ..........  U U U U   8   8   2   2
subi       8  .     .     0000010000......  A+-DXWL...  U U U U  12  12   4   4
subi      16  .     d     0000010001000...  ..........  U U U U   8   8   2   2
subi      16  .     .     0000010001......  A+-DXWL...  U U U U  12  12   4   4
subi      32  .     d     0000010010000...  ..........  U U U U  16  14   2   2
subi      32  .     .     0000010010......  A+-DXWL...  U U U U  20  20   4   4
subq       8  .     d     0101...100000...  ..........  U U U U   4   4   2   2
subq       8  .     .     0101...100......  A+-DXWL...  U U U U   8   8   4   4
subq      16  .     d     0101...101000...  ..........  U U U U   4   4   2   2
subq      16  .     a     0101...101001...  ..........  U U U U   8   4   2   2
subq      16  .     .     0101...101......  A+-DXWL...  U U U U   8   8   4   4
subq      32  .     d     0101...110000...  ..........  U U U U   8   8   2   2
subq      32  .     a     0101...110001...  ..........  U U U U   8   8   2   2
subq      32  .     .     0101...110......  A+-DXWL...  U U U U  12  12   4   4
subx       8  rr    .     1001...100000...  ..........  U U U U   4   4   2   2
subx      16  rr    .     1001...101000...  ..........  U U U U   4   4   2   2
subx      32  rr    .     1001...110000...  ..........  U U U U   8   6   2   2
subx       8  mm    ax7   1001111100001...  ..........  U U U U  18  18  12  12
subx       8  mm    ay7   1001...100001111  ..........  U U U U  18  18  12  12
subx       8  mm    axy7  1001111100001111  ..........  U U U U  18  18  12  12
subx       8  mm    .     1001...100001...  ..........  U U U U  18  18  12  12
subx      16  mm    .     1001...101001...  ..........  U U U U  18  18  12  12
subx      32  mm    .     1001...110001...  ..........  U U U U  30  30  12  12
swap      32  .     .     0100100001000...  ..........  U U U U   4   4   4   4
tas        8  .     d     0100101011000...  ..........  U U U U   4   4   4   4
tas        8  .     .     0100101011......  A+-DXWL...  U U U U  14  14  12  12
trap       0  .     .     010011100100....  ..........  U U U U   4   4   4   4
trapt      0  .     .     0101000011111100  ..........  . . U U   .   .   4   4
trapt     16  .     .     0101000011111010  ..........  . . U U   .   .   6   6
trapt     32  .     .     0101000011111011  ..........  . . U U   .   .   8   8
trapf      0  .     .     0101000111111100  ..........  . . U U   .   .   4   4
trapf     16  .     .     0101000111111010  ..........  . . U U   .   .   6   6
trapf     32  .     .     0101000111111011  ..........  . . U U   .   .   8   8
trapcc     0  .     .     0101....11111100  ..........  . . U U   .   .   4   4
trapcc    16  .     .     0101....11111010  ..........  . . U U   .   .   6   6
trapcc    32  .     .     0101....11111011  ..........  . . U U   .   .   8   8
trapv      0  .     .     0100111001110110  ..........  U U U U   4   4   4   4
tst        8  .     d     0100101000000...  ..........  U U U U   4   4   2   2
tst        8  .     .     0100101000......  A+-DXWL...  U U U U   4   4   2   2
tst        8  .     pcdi  0100101000111010  ..........  . . U U   .   .   7   7
tst        8  .     pcix  0100101000111011  ..........  . . U U   .   .   9   9
tst        8  .     i     0100101000111100  ..........  . . U U   .   .   6   6
tst       16  .     d     0100101001000...  ..........  U U U U   4   4   2   2
tst       16  .     a     0100101001001...  ..........  . . U U   .   .   2   2
tst       16  .     .     0100101001......  A+-DXWL...  U U U U   4   4   2   2
tst       16  .     pcdi  0100101001111010  ..........  . . U U   .   .   7   7
tst       16  .     pcix  0100101001111011  ..........  . . U U   .   .   9   9
tst       16  .     i     0100101001111100  ..........  . . U U   .   .   6   6
tst       32  .     d     0100101010000...  ..........  U U U U   4   4   2   2
tst       32  .     a     0100101010001...  ..........  . . U U   .   .   2   2
tst       32  .     .     0100101010......  A+-DXWL...  U U U U   4   4   2   2
tst       32  .     pcdi  0100101010111010  ..........  . . U U   .   .   7   7
tst       32  .     pcix  0100101010111011  ..........  . . U U   .   .   9   9
tst       32  .     i     0100101010111100  ..........  . . U U   .   .   6   6
unlk      32  .     a7    0100111001011111  ..........  U U U U  12  12   6   6
unlk      32  .     .     0100111001011...  ..........  U U U U  12  12   6   6
unpk      16  rr    .     1000...110000...  ..........  . . U U   .   .   8   8
unpk      16  mm    ax7   1000111110001...  ..........  . . U U   .   .  13  13
unpk      16  mm    ay7   1000...110001111  ..........  . . U U   .   .  13  13
unpk      16  mm    axy7  1000111110001111  ..........  . . U U   .   .  13  13
unpk      16  mm    .     1000...110001...  ..........  . . U U   .   .  13  13



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_BODY

M68KMAKE_OP(1010, 0, ., .)
{
	m68kdrc_exception_1010();
}


M68KMAKE_OP(1111, 0, ., .)
{
	m68kdrc_exception_1111();
}


M68KMAKE_OP(abcd, 8, rr, .)
{
	link_info link1;
	link_info link2;

	_mov_r8_m8abs(REG_CL, &DX);
	_mov_r8_r8(REG_CH, REG_CL);
	_and_r32_imm(REG_ECX, 0xf00f);

	_mov_r8_m8abs(REG_AL, &DY);
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_r8(REG_EAX, REG_CL);
	_mov_r32_r32(REG_EBX, REG_ECX);
	_shr_r32_imm(REG_EBX, 8);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_LE, &link1);

	_add_r32_imm(REG_EAX, 6);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_G, REG_BH);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);
	_jcc_near_link(COND_LE, &link2);

	_sub_r32_imm(REG_EAX, 0xa0);

_resolve_link(&link2);
	_not_r32(REG_ECX);			/* Undefined V behavior */
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	DRC_NFLAG_8();				/* Undefined N behavior */

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(abcd, 8, mm, ax7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_AY_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_r8(REG_EAX, REG_CL);
	_mov_r32_r32(REG_EBX, REG_ECX);
	_shr_r32_imm(REG_EBX, 8);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_LE, &link1);

	_add_r32_imm(REG_EAX, 6);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_G, REG_BH);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);
	_jcc_near_link(COND_LE, &link2);

	_sub_r32_imm(REG_EAX, 0xa0);

_resolve_link(&link2);
	_not_r32(REG_ECX);			/* Undefined V behavior */
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	DRC_NFLAG_8();				/* Undefined N behavior */

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(abcd, 8, mm, ay7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_A7_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_r8(REG_EAX, REG_CL);
	_mov_r32_r32(REG_EBX, REG_ECX);
	_shr_r32_imm(REG_EBX, 8);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_LE, &link1);

	_add_r32_imm(REG_EAX, 6);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_G, REG_BH);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);
	_jcc_near_link(COND_LE, &link2);

	_sub_r32_imm(REG_EAX, 0xa0);

_resolve_link(&link2);
	_not_r32(REG_ECX);			/* Undefined V behavior */
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	DRC_NFLAG_8();			/* Undefined N behavior */

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(abcd, 8, mm, axy7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_A7_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_r8(REG_EAX, REG_CL);
	_mov_r32_r32(REG_EBX, REG_ECX);
	_shr_r32_imm(REG_EBX, 8);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_LE, &link1);

	_add_r32_imm(REG_EAX, 6);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_G, REG_BH);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);
	_jcc_near_link(COND_LE, &link2);

	_sub_r32_imm(REG_EAX, 0xa0);

_resolve_link(&link2);
	_not_r32(REG_ECX);			/* Undefined V behavior */
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	DRC_NFLAG_8();				/* Undefined N behavior */

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(abcd, 8, mm, .)
{
	link_info link1;
	link_info link2;

	DRC_OPER_AY_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_r8(REG_EAX, REG_CL);
	_mov_r32_r32(REG_EBX, REG_ECX);
	_shr_r32_imm(REG_EBX, 8);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_LE, &link1);

	_add_r32_imm(REG_EAX, 6);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_G, REG_BH);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);
	_jcc_near_link(COND_LE, &link2);

	_sub_r32_imm(REG_EAX, 0xa0);

_resolve_link(&link2);
	_not_r32(REG_ECX);			/* Undefined V behavior */
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	DRC_NFLAG_8();				/* Undefined N behavior */

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(add, 8, er, d)
{
	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DY);

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(add, 8, er, .)
{
	M68KMAKE_GET_OPER_AY_8;
	_mov_r16_r16(REG_CX, REG_AX);

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(add, 16, er, d)
{
	_movzx_r32_m16abs(REG_ECX, &DY);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(add, 16, er, a)
{
	_movzx_r32_m16abs(REG_ECX, &AY);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(add, 16, er, .)
{
	M68KMAKE_GET_OPER_AY_16;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(add, 32, er, d)
{
	_mov_r32_m32abs(REG_ECX, &DY);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(add, 32, er, a)
{
	_mov_r32_m32abs(REG_ECX, &AY);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(add, 32, er, .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(add, 8, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r16_r16(REG_BX, REG_AX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(add, 16, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r32_r32(REG_EBX, REG_EAX);

	_movzx_r32_m16abs(REG_ECX, &DX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(add, 32, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_m32abs(REG_ECX, &DX);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(adda, 16, ., d)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_movsx_r32_m16abs(REG_ECX, &DY);
	_add_r32_r32(REG_EAX, REG_ECX);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(adda, 16, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_movsx_r32_m16abs(REG_ECX, &AY);
	_add_r32_r32(REG_EAX, REG_ECX);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(adda, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EAX, REG_AX);
	_mov_r32_m32abs(REG_ECX, &AX);
	_add_r32_r32(REG_ECX, REG_EAX);
	_mov_m32abs_r32(&AX, REG_ECX);
}


M68KMAKE_OP(adda, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_add_r32_m32abs(REG_EAX, &DY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(adda, 32, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_add_r32_m32abs(REG_EAX, &AY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(adda, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_m32abs(REG_ECX, &AX);
	_add_r32_r32(REG_ECX, REG_EAX);
	_mov_m32abs_r32(&AX, REG_ECX);
}


M68KMAKE_OP(addi, 8, ., d)
{
	uint8 src = OPER_I_8();

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DY);
	_mov_r16_r16(REG_AX, REG_BX);

	_mov_r16_imm(REG_CX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(addi, 8, ., .)
{
	uint8 src = OPER_I_8();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r16_r16(REG_BX, REG_AX);

	_mov_r16_imm(REG_CX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addi, 16, ., d)
{
	uint16 src = OPER_I_16();

	_movzx_r32_m16abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(addi, 16, ., .)
{
	uint16 src = OPER_I_16();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(addi, 32, ., d)
{
	uint32 src = OPER_I_32();

	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(addi, 32, ., .)
{
	uint32 src = OPER_I_32();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, src);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(addq, 8, ., d)
{
	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DY);
	_mov_r16_r16(REG_AX, REG_BX);

	_mov_r16_imm(REG_CX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(addq, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r16_r16(REG_BX, REG_AX);

	_mov_r16_imm(REG_CX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addq, 16, ., d)
{
	_movzx_r32_m16abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(addq, 16, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_add_r32_imm(REG_EAX, (((REG68K_IR >> 9) - 1) & 7) + 1);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(addq, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(addq, 32, ., d)
{
	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(addq, 32, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_add_r32_imm(REG_EAX, (((REG68K_IR >> 9) - 1) & 7) + 1);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(addq, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_add_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_add_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(addx, 8, rr, .)
{
	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DY);

	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8abs(REG_AL, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(addx, 16, rr, .)
{
	_movzx_r32_m16abs(REG_ECX, &DY);

	_movzx_r32_m16abs(REG_EAX, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_16(drc);	/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(addx, 32, rr, .)
{
	_mov_r32_m32abs(REG_ECX, &DY);

	_mov_r32_m32abs(REG_EAX, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_32(drc);	/* break EBX, ECX, EDX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(addx, 8, mm, ax7)
{
	DRC_OPER_AY_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addx, 8, mm, ay7)
{
	DRC_OPER_A7_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addx, 8, mm, axy7)
{
	DRC_OPER_A7_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addx, 8, mm, .)
{
	DRC_OPER_AY_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(addx, 16, mm, .)
{
	DRC_OPER_AY_PD_16();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_16();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_16(drc);	/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(addx, 32, mm, .)
{
	DRC_OPER_AY_PD_32();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_32();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_adc_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_addx_32(drc);	/* break EBX, ECX, EDX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(and, 8, er, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	_mov_r8_m8abs(REG_BL, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(and, 8, er, .)
{
	M68KMAKE_GET_OPER_AY_8;

	_mov_r8_m8abs(REG_BL, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(and, 16, er, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	_mov_r16_m16abs(REG_BX, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(and, 16, er, .)
{
	M68KMAKE_GET_OPER_AY_16;

	_mov_r16_m16abs(REG_BX, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(and, 32, er, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_and_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(and, 32, er, .)
{
	M68KMAKE_GET_OPER_AY_32;

	_and_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(and, 8, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_BL, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(and, 16, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r16_m16abs(REG_BX, &DX);
	_and_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(and, 32, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_and_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(andi, 8, ., d)
{
	uint8 src = OPER_I_8();

	_mov_r8_m8abs(REG_AL, &DY);

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(andi, 8, ., .)
{
	uint8 src = OPER_I_8();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(andi, 16, ., d)
{
	uint16 src = OPER_I_16();

	_mov_r16_m16abs(REG_AX, &DY);

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(andi, 16, ., .)
{
	uint16 src = OPER_I_16();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(andi, 32, ., d)
{
	uint32 src = OPER_I_32();

	_mov_r32_m32abs(REG_EAX, &DY);

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(andi, 32, ., .)
{
	uint32 src = OPER_I_32();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_and_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(andi, 16, toc, .)
{
	uint16 src = OPER_I_16();

	m68kdrc_get_ccr();

	_and_r32_imm(REG_EAX, src);

	m68kdrc_set_ccr(drc);
}


M68KMAKE_OP(andi, 16, tos, .)
{
	link_info link1;

	uint16 src = OPER_I_16();

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	m68kdrc_get_sr();
	_and_r32_imm(REG_EAX, src);
	m68kdrc_set_sr(drc);
}


M68KMAKE_OP(asr, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movsx_r32_m8abs(REG_EAX, &DY);
	_sar_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(asr, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movsx_r32_m16abs(REG_EAX, &DY);
	_sar_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(asr, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);
	_sar_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_32();		/* break ECX */
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(asr, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movsx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_sar_r32_imm(REG_EAX, 16);
	_sar_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_sar_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(asr, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movsx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_sar_r32_imm(REG_EAX, 16);
	_sar_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_sar_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(asr, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_sar_r32_imm(REG_EAX, 16);
	_sar_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_sar_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(asr, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_movsx_r32_r16(REG_EAX, REG_AX);

	_sar_r32_imm(REG_EAX, 1);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(asl, 8, s, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_r8(REG_BL, REG_AL);
	_and_r32_imm(REG_EBX, m68ki_shift_8_table[shift + 1]);
	_jcc_near_link(COND_Z, &link1);

	_cmp_r32_imm(REG_EBX, m68ki_shift_8_table[shift + 1]);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link3);
	_mov_r8_imm(REG_CL, shift);
	_shl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(asl, 16, s, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r16_r16(REG_BX, REG_AX);
	_and_r32_imm(REG_EBX, m68ki_shift_16_table[shift + 1]);
	_jcc_near_link(COND_Z, &link1);

	_cmp_r32_imm(REG_EBX, m68ki_shift_16_table[shift + 1]);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link3);
	_mov_r8_imm(REG_CL, shift);
	_shl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(asl, 32, s, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, m68ki_shift_32_table[shift + 1]);
	_jcc_near_link(COND_Z, &link1);

	_cmp_r32_imm(REG_EBX, m68ki_shift_32_table[shift + 1]);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link3);
	_shl_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(asl, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;
	link_info link5;
	link_info link6;

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	_mov_r32_r32(REG_EDX, REG_ECX);
	_add_r32_imm(REG_EDX, 1);
	_mov_r8_m8bd(REG_DL, REG_EDX, m68ki_shift_8_table);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link3);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link4);

_resolve_link(&link2);
_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link4);
	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link5);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shl_r8_cl(REG_AL);
	_shl_r8_cl(REG_AL);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link5);
	_shl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
	_jmp_near_link(&link6);

_resolve_link(&link1);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link6);
	DRC_NFLAG_8();
}


M68KMAKE_OP(asl, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;
	link_info link5;
	link_info link6;

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	_mov_r32_r32(REG_EDX, REG_ECX);
	_add_r32_imm(REG_EDX, 1);
	_shl_r32_imm(REG_EDX, 1);
	_mov_r16_m16bd(REG_DX, REG_EDX, m68ki_shift_16_table);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link3);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link4);

_resolve_link(&link2);
_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link4);
	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link5);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shl_r16_cl(REG_AX);
	_shl_r16_cl(REG_AX);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link5);
	_shl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
	_jmp_near_link(&link6);

_resolve_link(&link1);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link6);
	DRC_NFLAG_16();
}


M68KMAKE_OP(asl, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;
	link_info link5;
	link_info link6;

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	_mov_r32_r32(REG_EDX, REG_ECX);
	_add_r32_imm(REG_EDX, 1);
	_shl_r32_imm(REG_EDX, 2);
	_mov_r32_m32bd(REG_EDX, REG_EDX, m68ki_shift_32_table);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_r32(REG_EBX, REG_EDX);
	_jcc_near_link(COND_Z, &link3);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link4);

_resolve_link(&link2);
_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link4);
	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link5);

	_shl_r32_imm(REG_EAX, 16);
	_shl_r32_imm(REG_EAX, 16);

_resolve_link(&link5);
	_shl_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
	_jmp_near_link(&link6);

_resolve_link(&link1);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link6);
	DRC_NFLAG_32();
}


M68KMAKE_OP(asl, 16, ., .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, m68ki_shift_16_table[2]);
	_jcc_near_link(COND_Z, &link1);

	_cmp_r32_imm(REG_EBX, m68ki_shift_16_table[2]);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

_resolve_link(&link3);
	_mov_r8_imm(REG_CL, 1);
	_shl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(bcc, 8, ., .)
{
	M68KMAKE_CC;

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 1);

_resolve_link(&link_make_cc);
	DRC_USE_CYCLES(CYC_BCC_NOTAKE_B);
}


M68KMAKE_OP(bcc, 16, ., .)
{
	M68KMAKE_CC;

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68kdrc_branch_16(OPER_I_16(), 1);

_resolve_link(&link_make_cc);
	DRC_USE_CYCLES(CYC_BCC_NOTAKE_W);
}


M68KMAKE_OP(bcc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68KMAKE_CC;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68kdrc_branch_32(OPER_I_32(), 1);
_resolve_link(&link_make_cc);
	}
	else
	{
		M68KMAKE_CC;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 1);

_resolve_link(&link_make_cc);
		DRC_USE_CYCLES(CYC_BCC_NOTAKE_B);
	}
}


M68KMAKE_OP(bchg, 32, r, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x1f);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_xor_r32_r32(REG_EAX, REG_EBX);
	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bchg, 8, r, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 7);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_xor_r32_r32(REG_EAX, REG_EBX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bchg, 32, s, d)
{
	uint32 mask = 1 << (OPER_I_8() & 0x1f);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_xor_r32_imm(REG_EAX, mask);
	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bchg, 8, s, .)
{
	uint32 mask = 1 << (OPER_I_8() & 7);

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_xor_r32_imm(REG_EAX, mask);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bclr, 32, r, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x1f);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_not_r32(REG_EBX);
	_and_r32_r32(REG_EAX, REG_EBX);
	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bclr, 8, r, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 7);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_not_r32(REG_EBX);
	_and_r32_r32(REG_EAX, REG_EBX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bclr, 32, s, d)
{
	uint32 mask = 1 << (OPER_I_8() & 0x1f);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_and_r32_imm(REG_EAX, ~mask);
	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bclr, 8, s, .)
{
	uint32 mask = 1 << (OPER_I_8() & 7);

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_and_r32_imm(REG_EAX, ~mask);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bfchg, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
		}

		_mov_r32_m32abs(REG_EAX, &DY);
		_mov_r32_r32(REG_EBX, REG_EAX);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
			_mov_r8_r8(REG_CH, REG_CL);
			_ror_r32_cl(REG_EDX);

			_mov_r8_r8(REG_CL, REG_CH);
			_shl_r32_cl(REG_EBX);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_ror_r32_imm(REG_EDX, offset);
			_shl_r32_imm(REG_EBX, offset);
		}

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_xor_r32_r32(REG_EAX, REG_EDX);
		_mov_m32abs_r32(&DY, REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfchg, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();
/*
	(data
	(ea+4
	(mask_base
	(ea+4
	width
	offset
	(data
	ea
	mask_long
*/
		_sub_r32_imm(REG_ESP, 16);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);			// width

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);	// mask_base
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_sar_r32_imm(REG_EBX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EBX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_mov_r8_r8(REG_CL, REG_BL);
			_shr_r32_cl(REG_EDX);

			_push_r32(REG_EBX);			// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			if (offset)
				_shr_r32_imm(REG_EDX, offset);
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_mov_m32bd_r32(REG_ESP, 8, REG_EBX);		// ea+4
		_mov_m32bd_r32(REG_ESP, 16, REG_EBX);		// ea+4
		_sub_r32_imm(REG_ESP, 4);			// data
		_push_r32(REG_EAX);				// ea
		_push_r32(REG_EDX);				// mask_long

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32bd(REG_ECX, REG_ESP, 12);		// offset
		_shl_r32_cl(REG_EBX);

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_pop_r32(REG_EDX);				// mask_long
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_xor_r32_r32(REG_EAX, REG_EDX);
		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);		// data
		m68kdrc_write_32();

		_pop_r32(REG_ECX);				// offset
		_pop_r32(REG_EAX);				// width

		_add_r32_r32(REG_EAX, REG_ECX);
		_cmp_r32_imm(REG_EAX, 32);
		_jcc_near_link(COND_LE, &link1);

		m68kdrc_read_8();

		_pop_r32(REG_EBX);				// mask_base
		_mov_r32_r32(REG_ECX, REG_EBX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_r8_m8abs(REG_DL, &FLAG_Z);
		_or_r32_r32(REG_EDX, REG_ECX);
		_mov_m8abs_r8(&FLAG_Z, REG_DL);

		_xor_r32_r32(REG_EAX, REG_EBX);

		_mov_m8bd_r8(REG_ESP, 4, REG_AL);
		m68kdrc_write_8();

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 16);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfclr, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
		}

		_mov_r32_m32abs(REG_EAX, &DY);
		_mov_r32_r32(REG_EBX, REG_EAX);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
			_mov_r8_r8(REG_CH, REG_CL);
			_ror_r32_cl(REG_EDX);

			_mov_r8_r8(REG_CL, REG_CH);
			_shl_r32_cl(REG_EBX);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_ror_r32_imm(REG_EDX, offset);
			_shl_r32_imm(REG_EBX, offset);
		}

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_not_r32(REG_EDX);
		_and_r32_r32(REG_EAX, REG_EDX);
		_mov_m32abs_r32(&DY, REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfclr, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();
/*
	(data
	(ea+4
	(mask_base
	(ea+4
	width
	offset
	(data
	ea
	mask_long
*/
		_sub_r32_imm(REG_ESP, 16);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);			// width

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);	// mask_base
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_sar_r32_imm(REG_EBX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EBX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_mov_r8_r8(REG_CL, REG_BL);
			_shr_r32_cl(REG_EDX);

			_push_r32(REG_EBX);			// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			if (offset)
				_shr_r32_imm(REG_EDX, offset);
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_mov_m32bd_r32(REG_ESP, 8, REG_EBX);		// ea+4
		_mov_m32bd_r32(REG_ESP, 16, REG_EBX);		// ea+4
		_sub_r32_imm(REG_ESP, 4);			// data
		_push_r32(REG_EAX);				// ea
		_push_r32(REG_EDX);				// mask_long

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32bd(REG_ECX, REG_ESP, 12);		// offset
		_shl_r32_cl(REG_EBX);

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_pop_r32(REG_EDX);				// mask_long
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_not_r32(REG_EDX);
		_and_r32_r32(REG_EAX, REG_EDX);
		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);		// data
		m68kdrc_write_32();

		_pop_r32(REG_ECX);				// offset
		_pop_r32(REG_EAX);				// width

		_add_r32_r32(REG_EAX, REG_ECX);
		_cmp_r32_imm(REG_EAX, 32);
		_jcc_near_link(COND_LE, &link1);

		m68kdrc_read_8();

		_pop_r32(REG_EBX);				// mask_base
		_mov_r32_r32(REG_ECX, REG_EBX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_r8_m8abs(REG_DL, &FLAG_Z);
		_or_r32_r32(REG_EDX, REG_ECX);
		_mov_m8abs_r8(&FLAG_Z, REG_DL);

		_not_r32(REG_EBX);
		_and_r32_r32(REG_EAX, REG_EBX);

		_mov_m8bd_r8(REG_ESP, 4, REG_AL);
		m68kdrc_write_8();

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 16);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfexts, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_mov_r32_m32abs(REG_EAX, &DY);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_mov_r8_imm(REG_CL, offset);
		}

		_rol_r32_cl(REG_EAX);

		DRC_NFLAG_32();		/* break ECX */

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_sar_r32_cl(REG_EAX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_sar_r32_imm(REG_EAX, 32 - width);
		}

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfexts, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);		// width
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_mov_r32_r32(REG_EDX, REG_EBX);
			_sar_r32_imm(REG_EDX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EDX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_push_r32(REG_EBX);		// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_push_r32(REG_EBX);			// ea+4

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_m32bd(REG_EBX, REG_ESP, 4);	// offset
		_mov_r8_r8(REG_CL, REG_BL);
		_shl_r32_cl(REG_EAX);

		_mov_r32_m32bd(REG_ECX, REG_ESP, 8);	// width
		_add_r32_r32(REG_EBX, REG_ECX);

		_cmp_r32_imm(REG_EBX, 32);
		_jcc_near_link(COND_LE, &link1);

		_mov_m32bd_r32(REG_ESP, 12, REG_EAX);

		m68kdrc_read_8();

		_pop_r32(REG_ECX);	// offset

		_shl_r32_cl(REG_EAX);
		_mov_r32_m32bd(REG_EBX, REG_ESP, 8);
		_or_r32_r32(REG_EAX, REG_EBX);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 8);

_resolve_link(&link2);
		DRC_NFLAG_32();		/* break ECX */

		_pop_r32(REG_EBX);	// width
		_mov_r32_imm(REG_ECX, 32);
		_sub_r32_r32(REG_ECX, REG_EBX);

		_sar_r32_cl(REG_EAX);

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);

		_add_r32_imm(REG_ESP, 4);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfextu, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_mov_r32_m32abs(REG_EAX, &DY);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_mov_r8_imm(REG_CL, offset);
		}

		_rol_r32_cl(REG_EAX);

		DRC_NFLAG_32();		/* break ECX */

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_shr_r32_cl(REG_EAX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_shr_r32_imm(REG_EAX, 32 - width);
		}

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfextu, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);		// width
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_mov_r32_r32(REG_EDX, REG_EBX);
			_sar_r32_imm(REG_EDX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EDX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_push_r32(REG_EBX);		// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_push_r32(REG_EBX);			// ea+4

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_m32bd(REG_EBX, REG_ESP, 4);	// offset
		_mov_r8_r8(REG_CL, REG_BL);
		_shl_r32_cl(REG_EAX);

		_mov_r32_m32bd(REG_ECX, REG_ESP, 8);	// width
		_add_r32_r32(REG_EBX, REG_ECX);

		_cmp_r32_imm(REG_EBX, 32);
		_jcc_near_link(COND_LE, &link1);

		_mov_m32bd_r32(REG_ESP, 12, REG_EAX);

		m68kdrc_read_8();

		_pop_r32(REG_ECX);	// offset

		_shl_r32_cl(REG_EAX);
		_mov_r32_m32bd(REG_EBX, REG_ESP, 8);
		_or_r32_r32(REG_EAX, REG_EBX);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 8);

_resolve_link(&link2);
		DRC_NFLAG_32();		/* break ECX */

		_pop_r32(REG_EBX);	// width
		_mov_r32_imm(REG_ECX, 32);
		_sub_r32_r32(REG_ECX, REG_EBX);

		_shr_r32_cl(REG_EAX);

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);

		_add_r32_imm(REG_ESP, 4);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfffo, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;
		void *loop;

		uint16 word2 = OPER_I_16();

		_mov_r32_m32abs(REG_EAX, &DY);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
			_mov_r32_r32(REG_EBX, REG_ECX);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_mov_r32_imm(REG_ECX, offset);
			_mov_r32_r32(REG_EBX, REG_ECX);
		}

		_rol_r32_cl(REG_EAX);

		DRC_NFLAG_32();		/* break ECX */

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_shr_r32_cl(REG_EAX);

			_mov_r32_r32(REG_ECX, REG_EDX);
			_sub_r32_imm(REG_ECX, 1);
			_mov_r32_imm(REG_EDX, 1);
			_shl_r32_cl(REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_shr_r32_imm(REG_EAX, 32 - width);
			_mov_r32_imm(REG_EDX, 1 << (width - 1));
		}

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_or_r32_r32(REG_EDX, REG_EDX);
loop = drc->cache_top;
		_jcc_near_link(COND_Z, &link1);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_jcc_near_link(COND_NZ, &link2);

		_add_r32_imm(REG_EBX, 1);
		_shr_r32_imm(REG_EDX, 1);
		_jmp(loop);

_resolve_link(&link1);
_resolve_link(&link2);
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EBX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfffo, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;
		void *loop;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);		// width
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);
		}

		M68KMAKE_GET_EA_AY_8;

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_push_r32(REG_EBX);		// offset

			_xor_r32_r32(REG_ECX, REG_ECX);
			_mov_r32_r32(REG_EDX, REG_EBX);
			_sar_r32_imm(REG_EDX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EDX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_push_r32(REG_EBX);		// local_offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_push_imm(offset);

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			_push_imm(offset % 8);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_push_r32(REG_EBX);			// ea+4

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_m32bd(REG_EBX, REG_ESP, 4);	// local_offset
		_mov_r8_r8(REG_CL, REG_BL);
		_shl_r32_cl(REG_EAX);

		_mov_r32_m32bd(REG_ECX, REG_ESP, 12);	// width
		_add_r32_r32(REG_EBX, REG_ECX);

		_cmp_r32_imm(REG_EBX, 32);
		_jcc_near_link(COND_LE, &link1);
		_mov_m32bd_r32(REG_ESP, 16, REG_EAX);	// data

		m68kdrc_read_8();

		_pop_r32(REG_ECX);	// local_offset

		_shl_r32_cl(REG_EAX);
		_shr_r32_imm(REG_EAX, 8);
		_mov_r32_m32bd(REG_EBX, REG_ESP, 8);	// data
		_or_r32_r32(REG_EAX, REG_EBX);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 8);

_resolve_link(&link2);
		DRC_NFLAG_32();		/* break ECX */

		_pop_r32(REG_EBX);	// offset
		_pop_r32(REG_EDX);	// width

		_mov_r32_imm(REG_ECX, 32);
		_sub_r32_r32(REG_ECX, REG_EDX);
		_shr_r32_cl(REG_EAX);

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_mov_r32_r32(REG_ECX, REG_EDX);
		_sub_r32_imm(REG_ECX, 1);
		_mov_r32_imm(REG_EDX, 1);
		_shl_r32_cl(REG_EDX);

		_or_r32_r32(REG_EDX, REG_EDX);
loop = drc->cache_top;
		_jcc_near_link(COND_Z, &link1);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_jcc_near_link(COND_NZ, &link2);

		_add_r32_imm(REG_EBX, 1);
		_shr_r32_imm(REG_EDX, 1);
		_jmp(loop);

_resolve_link(&link1);
_resolve_link(&link2);
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EBX);

		_add_r32_imm(REG_ESP, 4);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfins, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_r32(REG_EBX, REG_ECX);
			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
			_mov_r32_r32(REG_ECX, REG_EBX);
			_shl_r32_cl(REG_EAX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
			_shl_r32_imm(REG_EAX, 32 - width);
		}

		DRC_NFLAG_32();		/* break ECX */

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_mov_r8_imm(REG_CL, offset);
		}

		_mov_r8_r8(REG_CH, REG_CL);
		_ror_r32_cl(REG_EDX);

		_mov_r8_r8(REG_CL, REG_CH);
		_ror_r32_cl(REG_EAX);

		_mov_r32_m32abs(REG_EBX, &DY);
		_not_r32(REG_EDX);
		_and_r32_r32(REG_EBX, REG_EDX);
		_or_r32_r32(REG_EAX, REG_EBX);
		_mov_m32abs_r32(&DY, REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfins, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();
/*
	write data
	ea+4
	insert_base
	mask_base
	ea+4
	width
	offset
	write data
	ea
	mask_long
	ea
*/
		_sub_r32_imm(REG_ESP, 8);

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_r32(REG_EBX, REG_ECX);

			_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);
			_shl_r32_cl(REG_EAX);
			_push_r32(REG_EAX);			// insert_base

			_mov_r32_r32(REG_ECX, REG_EBX);
			_mov_r32_imm(REG_EAX, 0xffffffff);
			_shl_r32_cl(REG_EAX);
			_push_r32(REG_EAX);			// mask_base

			_sub_r32_imm(REG_ESP, 4);		// ea+4 for read
			_push_r32(REG_EDX);			// width
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2>>12)&7]);
			_shl_r32_imm(REG_EBX, (32 - width));
			_push_r32(REG_EBX);

			_push_imm(0xffffffff << (32 - width));

			_sub_r32_imm(REG_ESP, 4);
			_push_imm(width);
		}

		M68KMAKE_GET_EA_AY_8;

		_mov_r32_m32bd(REG_EDX, REG_ESP, 8);

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_sar_r32_imm(REG_EBX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EBX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_mov_r8_r8(REG_CL, REG_BL);
			_shr_r32_cl(REG_EDX);

			_push_r32(REG_EBX);			// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			if (offset)
				_shr_r32_imm(REG_EDX, offset);	// mask_long
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_mov_m32bd_r32(REG_ESP, 20, REG_EBX);		// ea+4 for write
		_mov_m32bd_r32(REG_ESP, 8, REG_EBX);		// ea+4 for write
		_sub_r32_imm(REG_ESP, 4);			// data
		_push_r32(REG_EAX);				// ea for write
		_push_r32(REG_EDX);				// mask_long

		_push_r32(REG_EAX);				// ea for read

		_mov_r32_m32bd(REG_EAX, REG_ESP, 32);		// insert_base
		DRC_NFLAG_32();		/* break ECX */
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);

		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		m68kdrc_read_32();

		_pop_r32(REG_EDX);				// mask_long
		_not_r32(REG_EDX);
		_and_r32_r32(REG_EAX, REG_EDX);

		_mov_r32_m32bd(REG_EDX, REG_ESP, 24);		// insert_base
		_mov_r32_m32bd(REG_ECX, REG_ESP, 8);		// offset
		_shr_r32_cl(REG_EDX);				// insert_long
		_or_r32_r32(REG_EAX, REG_EDX);

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_write_32();

		_pop_r32(REG_ECX);				// offset
		_pop_r32(REG_EAX);				// width

		_add_r32_r32(REG_EAX, REG_ECX);
		_cmp_r32_imm(REG_EAX, 32);
		_jcc_near_link(COND_LE, &link1);

		m68kdrc_read_8();

		_pop_r32(REG_EBX);				// mask_base

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EBX);

		_mov_r8_m8abs(REG_DL, &FLAG_Z);
		_or_r32_r32(REG_EDX, REG_ECX);
		_mov_m8abs_r8(&FLAG_Z, REG_DL);

		_pop_r32(REG_ECX);				// insert_base

		_not_r32(REG_EBX);
		_and_r32_r32(REG_EAX, REG_EBX);
		_or_r32_r32(REG_EAX, REG_ECX);

		_mov_m8bd_r8(REG_ESP, 4, REG_AL);
		m68kdrc_write_8();

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 20);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfset, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
		}

		_mov_r32_m32abs(REG_EAX, &DY);
		_mov_r32_r32(REG_EBX, REG_EAX);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
			_mov_r8_r8(REG_CH, REG_CL);
			_ror_r32_cl(REG_EDX);

			_mov_r8_r8(REG_CL, REG_CH);
			_shl_r32_cl(REG_EBX);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_ror_r32_imm(REG_EDX, offset);
			_shl_r32_imm(REG_EBX, offset);
		}

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_or_r32_r32(REG_EAX, REG_EDX);
		_mov_m32abs_r32(&DY, REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bfset, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();
/*
	(data
	(ea+4
	(mask_base
	(ea+4
	width
	offset
	(data
	ea
	mask_long
*/
		_sub_r32_imm(REG_ESP, 16);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);			// width

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);	// mask_base
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_sar_r32_imm(REG_EBX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EBX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_mov_r8_r8(REG_CL, REG_BL);
			_shr_r32_cl(REG_EDX);

			_push_r32(REG_EBX);			// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			if (offset)
				_shr_r32_imm(REG_EDX, offset);
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_mov_m32bd_r32(REG_ESP, 8, REG_EBX);		// ea+4
		_mov_m32bd_r32(REG_ESP, 16, REG_EBX);		// ea+4
		_sub_r32_imm(REG_ESP, 4);			// data
		_push_r32(REG_EAX);				// ea
		_push_r32(REG_EDX);				// mask_long

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32bd(REG_ECX, REG_ESP, 12);		// offset
		_shl_r32_cl(REG_EBX);

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_pop_r32(REG_EDX);				// mask_long
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_or_r32_r32(REG_EAX, REG_EDX);
		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);		// data
		m68kdrc_write_32();

		_pop_r32(REG_ECX);				// offset
		_pop_r32(REG_EAX);				// width

		_add_r32_r32(REG_EAX, REG_ECX);
		_cmp_r32_imm(REG_EAX, 32);
		_jcc_near_link(COND_LE, &link1);

		m68kdrc_read_8();

		_pop_r32(REG_EBX);				// mask_base
		_mov_r32_r32(REG_ECX, REG_EBX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_r8_m8abs(REG_DL, &FLAG_Z);
		_or_r32_r32(REG_EDX, REG_ECX);
		_mov_m8abs_r8(&FLAG_Z, REG_DL);

		_or_r32_r32(REG_EAX, REG_EBX);

		_mov_m8bd_r8(REG_ESP, 4, REG_AL);
		m68kdrc_write_8();

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 16);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bftst, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);
			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
		}

		_mov_r32_m32abs(REG_EAX, &DY);
		_mov_r32_r32(REG_EBX, REG_EAX);

		if (BIT_B(word2))
		{
			_mov_r8_m8abs(REG_CL, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_ECX, 31);
			_mov_r8_r8(REG_CH, REG_CL);
			_ror_r32_cl(REG_EDX);

			_mov_r8_r8(REG_CL, REG_CH);
			_shl_r32_cl(REG_EBX);
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			_ror_r32_imm(REG_EDX, offset);
			_shl_r32_imm(REG_EBX, offset);
		}

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bftst, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 8);

		M68KMAKE_GET_EA_AY_8;

		if (BIT_5(word2))
		{
			_mov_r8_m8abs(REG_DL, &REG68K_D[word2 & 7]);
			_sub_r32_imm(REG_EDX, 1);
			_and_r32_imm(REG_EDX, 31);
			_add_r32_imm(REG_EDX, 1);

			_push_r32(REG_EDX);

			_mov_r32_imm(REG_ECX, 32);
			_sub_r32_r32(REG_ECX, REG_EDX);

			_mov_r32_imm(REG_EDX, 0xffffffff);
			_shl_r32_cl(REG_EDX);
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);
		}
		else
		{
			uint8 width = ((word2 - 1) & 31) + 1;

			_push_imm(width);

			_mov_r32_imm(REG_EDX, 0xffffffff << (32 - width));
			_mov_m32bd_r32(REG_ESP, 8, REG_EDX);
		}

		if (BIT_B(word2))
		{
			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);

			_xor_r32_r32(REG_ECX, REG_ECX);
			_sar_r32_imm(REG_EBX, 3);
			_setcc_r8(COND_S, REG_CL);
			_add_r32_r32(REG_EAX, REG_EBX);
			_sub_r32_r32(REG_EAX, REG_ECX);

			_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 6) & 7]);
			_and_r32_imm(REG_EBX, 7);
			_shl_r32_imm(REG_ECX, 3);
			_add_r32_r32(REG_EBX, REG_ECX);

			_mov_r8_r8(REG_CL, REG_BL);
			_shr_r32_cl(REG_EDX);

			_push_r32(REG_EBX);			// offset
		}
		else
		{
			uint8 offset = (word2 >> 6) & 31;

			if (offset / 8)
				_add_r32_imm(REG_EAX, offset / 8);

			offset %= 8;
			if (offset)
				_shr_r32_imm(REG_EDX, offset);
			_push_imm(offset);
		}

		_lea_r32_m32bd(REG_EBX, REG_EAX, 4);
		_mov_m32bd_r32(REG_ESP, 8, REG_EBX);
		_push_r32(REG_EDX);

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32bd(REG_ECX, REG_ESP, 4);
		_shl_r32_cl(REG_EBX);

		//NFLAG_32();
		_shr_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		_mov_r32_r32(REG_ECX, REG_EAX);
		_pop_r32(REG_EDX);
		_and_r32_r32(REG_ECX, REG_EDX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		_pop_r32(REG_ECX);
		_pop_r32(REG_EAX);

		_add_r32_r32(REG_EAX, REG_ECX);
		_cmp_r32_imm(REG_EAX, 32);
		_jcc_near_link(COND_LE, &link1);

		m68kdrc_read_8();

		_pop_r32(REG_EBX);
		_mov_r32_r32(REG_ECX, REG_EBX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_r8_m8abs(REG_DL, &FLAG_Z);
		_or_r32_r32(REG_EDX, REG_ECX);
		_mov_m8abs_r8(&FLAG_Z, REG_DL);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		_add_r32_imm(REG_ESP, 8);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bkpt, 0, ., .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
			_mov_r32_imm(REG_EAX, REG68K_IR & 7);
		else
			_xor_r32_r32(REG_EAX, REG_EAX);

		m68kdrc_bkpt_ack(REG_EAX);		   /* auto-disable (see m68kcpu.h) */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(bra, 8, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 1);

	m68kdrc_recompile_flag |= RECOMPILE_END_OF_STRING;
}


M68KMAKE_OP(bra, 16, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_branch_16(OPER_I_16(), 1);

	m68kdrc_recompile_flag |= RECOMPILE_END_OF_STRING;
}


M68KMAKE_OP(bra, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		m68kdrc_branch_32(OPER_I_32(), 1);

		m68kdrc_recompile_flag |= RECOMPILE_END_OF_STRING;
	}
	else
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 1);

		m68kdrc_recompile_flag |= RECOMPILE_END_OF_STRING;
	}
}


M68KMAKE_OP(bset, 32, r, d)
{
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x1f);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_or_r32_r32(REG_EAX, REG_EBX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bset, 8, r, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 7);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_r32(REG_ECX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_or_r32_r32(REG_EAX, REG_EBX);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bset, 32, s, d)
{
	uint32 mask = 1 << (OPER_I_8() & 0x1f);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_or_r32_imm(REG_EAX, mask);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(bset, 8, s, .)
{
	uint8 mask = 1 << (OPER_I_8() & 7);

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_r32(REG_ECX, REG_EAX);
	_and_r32_imm(REG_ECX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_ECX);

	_or_r32_imm(REG_EAX, mask);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(bsr, 8, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_push_32_imm(REG68K_PC);

	m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 0);
}


M68KMAKE_OP(bsr, 16, ., .)
{
	uint16 offset = OPER_I_16();

	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_push_32_imm(REG68K_PC);

	m68kdrc_branch_16(offset, 0);
}


M68KMAKE_OP(bsr, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint32 offset = OPER_I_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		m68kdrc_push_32_imm(REG68K_PC);

		m68kdrc_branch_32(offset, 0);
	}
	else
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		m68kdrc_push_32_imm(REG68K_PC);

		m68kdrc_branch_8(MASK_OUT_ABOVE_8(REG68K_IR), 0);
	}
}


M68KMAKE_OP(btst, 32, r, d)
{
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x1f);
	_mov_r32_imm(REG_EAX, 1);
	_shl_r32_cl(REG_EAX);
	_and_r32_m32abs(REG_EAX, &DY);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(btst, 8, r, .)
{
	M68KMAKE_GET_OPER_AY_8;

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x7);
	_mov_r32_imm(REG_EBX, 1);
	_shl_r32_cl(REG_EBX);
	_and_r32_r32(REG_EAX, REG_EBX);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(btst, 32, s, d)
{
	uint32 mask = (1 << (OPER_I_8() & 0x1f));

	_mov_r32_m32abs(REG_EAX, &DY);
	_and_r32_imm(REG_EAX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(btst, 8, s, .)
{
	uint8 mask = (1 << (OPER_I_8() & 7));

	M68KMAKE_GET_OPER_AY_8;

	_and_r32_imm(REG_EAX, mask);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(callm, 32, ., .)
{
	/* note: watch out for pcrelative modes */
	if (CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
#if 0
		uint ea = M68KMAKE_GET_EA_AY_32;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG68K_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
		return;
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cas, 8, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_8;
		_push_r32(REG_EAX);

		_push_r32(REG_EAX);
		m68kdrc_read_8();
		_mov_r32_r32(REG_ECX, REG_EAX);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_movzx_r32_m8abs(REG_EBX, &REG68K_D[word2 & 7]);
		_mov_r8_r8(REG_DL, REG_BL);
		_mov_r16_r16(REG_AX, REG_BX);
		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_8(drc);		/* break EBX, ECX */

		_or_r32_r32(REG_EAX, REG_EAX);
		_jcc_near_link(COND_NZ, &link1);

		_mov_m8abs_r8(&REG68K_D[word2 & 7], REG_DL);
		_add_r32_imm(REG_ESP, 8);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		DRC_USE_CYCLES(3);

		_mov_r8_m8abs(REG_DL, &REG68K_D[(word2 >> 6) & 7]);

		_mov_m8bd_r8(REG_ESP, 4, REG_DL);
		m68kdrc_write_8();

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cas, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_16;
		_push_r32(REG_EAX);

		_push_r32(REG_EAX);
		m68kdrc_read_16();
		_mov_r32_r32(REG_ECX, REG_EAX);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_movzx_r32_m16abs(REG_EBX, &REG68K_D[word2 & 7]);
		_mov_r16_r16(REG_DX, REG_BX);
		_mov_r32_r32(REG_EAX, REG_EBX);
		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_16(drc);		/* break EBX, ECX */

		_or_r32_r32(REG_EAX, REG_EAX);
		_jcc_near_link(COND_NZ, &link1);

		_mov_m16abs_r16(&REG68K_D[word2 & 7], REG_DX);
		_add_r32_imm(REG_ESP, 8);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		DRC_USE_CYCLES(3);

		_mov_r16_m16abs(REG_DX, &REG68K_D[(word2 >> 6) & 7]);

		_mov_m16bd_r16(REG_ESP, 4, REG_DX);
		m68kdrc_write_16();

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cas, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_32;
		_push_r32(REG_EAX);

		_push_r32(REG_EAX);
		m68kdrc_read_32();
		_mov_r32_r32(REG_ECX, REG_EAX);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_mov_r32_m32abs(REG_EBX, &REG68K_D[word2 & 7]);
		_mov_r32_r32(REG_EDX, REG_EBX);
		_mov_r32_r32(REG_EAX, REG_EBX);
		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_32(drc);		/* break EBX, ECX */

		_or_r32_r32(REG_EAX, REG_EAX);
		_jcc_near_link(COND_NZ, &link1);

		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);
		_add_r32_imm(REG_ESP, 8);

		_jmp_near_link(&link2);

_resolve_link(&link1);
		DRC_USE_CYCLES(3);

		_mov_r32_m32abs(REG_EDX, &REG68K_D[(word2 >> 6) & 7]);

		_mov_m32bd_r32(REG_ESP, 4, REG_EDX);
		m68kdrc_write_32();

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cas2, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;
		link_info link3;

		uint32 word2 = OPER_I_32();

		_mov_r32_m32abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		_sub_r32_imm(REG_ESP, 4);
		_push_r32(REG_EBX);

		_mov_r32_m32abs(REG_EAX, &REG68K_DA[(word2 >> 28) & 15]);
		_sub_r32_imm(REG_ESP, 4);
		_push_r32(REG_EAX);
		_push_r32(REG_EBX);

		_push_r32(REG_EAX);
		m68kdrc_read_16();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_mov_m32bd_r32(REG_ESP, 16, REG_EAX);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32abs(REG_ECX, &REG68K_D[(word2 >> 16) & 7]);
		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_16(drc);		/* break EBX, ECX */

		m68kdrc_read_16();
		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_r32(REG_EDX, REG_EAX);
		_mov_r32_m32abs(REG_ECX, &REG68K_D[word2 & 7]);

		_test_m32abs_imm(&FLAG_Z, ZFLAG_CLEAR);
		_jcc_near_link(COND_NZ, &link1);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_16(drc);		/* break EBX, ECX */

		_or_r32_r32(REG_EAX, REG_EAX);
		_jcc_near_link(COND_NZ, &link2);

		DRC_USE_CYCLES(3);

		_mov_r16_m16abs(REG_AX, &REG68K_D[(word2 >> 22) & 7]);
		_mov_m16bd_r16(REG_ESP, 4, REG_AX);
		m68kdrc_write_16();

		_mov_r16_m16abs(REG_AX, &REG68K_D[(word2 >> 6) & 7]);
		_mov_m16bd_r16(REG_ESP, 4, REG_AX);
		m68kdrc_write_16();

		_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
		if (BIT_1F(word2))
		{
			_mov_r16_m16bd(REG_BX, REG_ESP, 12);
			_movsx_r32_r16(REG_EBX, REG_BX);
			_mov_m32abs_r32(&REG68K_D[(word2 >> 16) & 7], REG_EBX);

			_movsx_r32_r16(REG_EDX, REG_DX);
			_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);
		}
		else
		{
			_mov_r16_m16bd(REG_BX, REG_ESP, 12);
			_mov_m16abs_r16(&REG68K_D[(word2 >> 16) & 7], REG_BX);
			_mov_m16abs_r16(&REG68K_D[word2 & 7], REG_DX);
		}

		_add_r32_imm(REG_ESP, 16);

_resolve_link(&link3);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cas2, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;
		link_info link3;

		uint32 word2 = OPER_I_32();

		_mov_r32_m32abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		_sub_r32_imm(REG_ESP, 4);
		_push_r32(REG_EBX);

		_mov_r32_m32abs(REG_EAX, &REG68K_DA[(word2 >> 28) & 15]);
		_sub_r32_imm(REG_ESP, 4);
		_push_r32(REG_EAX);
		_push_r32(REG_EBX);

		_push_r32(REG_EAX);
		m68kdrc_read_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_mov_m32bd_r32(REG_ESP, 16, REG_EAX);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_m32abs(REG_ECX, &REG68K_D[(word2 >> 16) & 7]);
		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_32(drc);		/* break EBX, ECX */

		m68kdrc_read_32();
		_mov_r32_r32(REG_EBX, REG_EAX);
		_mov_r32_r32(REG_EDX, REG_EAX);
		_mov_r32_m32abs(REG_ECX, &REG68K_D[word2 & 7]);

		_test_m32abs_imm(&FLAG_Z, ZFLAG_CLEAR);
		_jcc_near_link(COND_NZ, &link1);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_sub_32(drc);		/* break EBX, ECX */

		_or_r32_r32(REG_EAX, REG_EAX);
		_jcc_near_link(COND_NZ, &link2);

		DRC_USE_CYCLES(3);

		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 22) & 7]);
		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
		m68kdrc_write_32();

		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 6) & 7]);
		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
		m68kdrc_write_32();

		_jmp_near_link(&link3);

_resolve_link(&link1);
_resolve_link(&link2);
		_mov_r32_m32bd(REG_EBX, REG_ESP, 12);
		_mov_m32abs_r32(&REG68K_D[(word2 >> 16) & 7], REG_EBX);
		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);

		_add_r32_imm(REG_ESP, 16);

_resolve_link(&link3);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk, 16, ., d)
{
	link_info link1;
	link_info link2;

	_movsx_r32_m16abs(REG_EAX, &DX);

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);	/* Undocumented */
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undocumented */
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);	/* Undocumented */

	_cmp_r32_imm(REG_EAX, 0);
	_jcc_near_link(COND_L, &link1);

	_movsx_r32_m16abs(REG_EBX, &DY);
	_sub_r32_r32(REG_EBX, REG_EAX);
	_jcc_near_link(COND_NC, &link2);

_resolve_link(&link1);
	_shl_r32_imm(REG_EAX, 7);
	DRC_NFLAG_16();

	m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link2);
}


M68KMAKE_OP(chk, 16, ., .)
{
	link_info link1;
	link_info link2;

	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EAX, REG_AX);
	_movsx_r32_m16abs(REG_EBX, &DX);

	_mov_m32abs_r32(&FLAG_Z, REG_EBX);	/* Undocumented */
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undocumented */
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);	/* Undocumented */

	_cmp_r32_imm(REG_EBX, 0);
	_jcc_near_link(COND_L, &link1);

	_sub_r32_r32(REG_EAX, REG_EBX);
	_jcc_near_link(COND_NC, &link2);

_resolve_link(&link1);
	// NFLAG_16();
	_mov_m8abs_r8(&FLAG_N, REG_BH);

	m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link2);
}


M68KMAKE_OP(chk, 32, ., d)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		_mov_r32_m32abs(REG_EAX, &DX);

		_mov_m32abs_r32(&FLAG_Z, REG_EAX);	/* Undocumented */
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undocumented */
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);	/* Undocumented */

		_cmp_r32_imm(REG_EAX, 0);
		_jcc_near_link(COND_L, &link1);

		_mov_r32_m32abs(REG_EBX, &DY);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_jcc_near_link(COND_NC, &link2);

_resolve_link(&link1);
		// NFLAG_32();
		_shl_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		link_info link1;
		link_info link2;

		M68KMAKE_GET_OPER_AY_16;
		_mov_r32_m32abs(REG_EBX, &DX);

		_mov_m32abs_r32(&FLAG_Z, REG_EBX);	/* Undocumented */
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undocumented */
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);	/* Undocumented */

		_cmp_r32_imm(REG_EBX, 0);
		_jcc_near_link(COND_L, &link1);

		_sub_r32_r32(REG_EAX, REG_EBX);
		_jcc_near_link(COND_NC, &link2);

_resolve_link(&link1);
		// NFLAG_32();
		_shl_r32_imm(REG_EBX, 16);
		_mov_m8abs_r8(&FLAG_N, REG_BH);

		m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 8, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCDI_8();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 1);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_8();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_8();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r8(REG_EAX, REG_AL);
			_movsx_r32_r8(REG_EDX, REG_DL);
			_movsx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
		else
			_movzx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 8, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCIX_8();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 1);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_8();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_8();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r8(REG_EAX, REG_AL);
			_movsx_r32_r8(REG_EDX, REG_DL);
			_movsx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
		else
			_movzx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 8, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_8;
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 1);
		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_8();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r8(REG_EAX, REG_AL);
			_movsx_r32_r8(REG_EDX, REG_DL);
			_movsx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
		else
			_movzx_r32_m8abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 16, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCDI_16();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 2);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_16();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_16();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r16(REG_EAX, REG_AX);
			_movsx_r32_r16(REG_EDX, REG_DX);
			_movsx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
			_movzx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_shr_r32_imm(REG_EBX, 8);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 16, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCIX_16();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 2);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_16();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_16();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r16(REG_EAX, REG_AX);
			_movsx_r32_r16(REG_EDX, REG_DX);
			_movsx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
		else
			_movzx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_shr_r32_imm(REG_EBX, 8);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_16;
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 2);
		_push_r32(REG_EAX);
		m68kdrc_read_16();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_16();

		_pop_r32(REG_EDX);

		if (!BIT_F(word2))
		{
			_movsx_r32_r16(REG_EAX, REG_AX);
			_movsx_r32_r16(REG_EDX, REG_DX);
			_movsx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);
		}
		else
			_movzx_r32_m16abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_or_r32_r32(REG_EBX, REG_EDX);
		_shr_r32_imm(REG_EBX, 8);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 32, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCDI_32();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 4);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_32();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_32();

		_pop_r32(REG_EDX);

		_mov_r32_m32abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_setcc_r8(COND_C, REG_DH);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_setcc_r8(COND_C, REG_BH);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 32, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		DRC_EA_PCIX_32();
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 4);
		_push_r32(REG_EAX);
		m68kdrc_read_pcrel_32();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_pcrel_32();

		_pop_r32(REG_EDX);

		_mov_r32_m32abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_setcc_r8(COND_C, REG_DH);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_setcc_r8(COND_C, REG_BH);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(chk2cmp2, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 word2 = OPER_I_16();

		_sub_r32_imm(REG_ESP, 4);

		M68KMAKE_GET_EA_AY_32;
		_push_r32(REG_EAX);

		_add_r32_imm(REG_EAX, 4);
		_push_r32(REG_EAX);
		m68kdrc_read_32();

		_mov_m32bd_r32(REG_ESP, 4, REG_EAX);

		m68kdrc_read_32();

		_pop_r32(REG_EDX);

		_mov_r32_m32abs(REG_EBX, &REG68K_DA[(word2 >> 12) & 15]);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_sub_r32_r32(REG_EDX, REG_EBX);
		_setcc_r8(COND_NZ, REG_CL);
		_setcc_r8(COND_C, REG_DH);
		_sub_r32_r32(REG_EBX, REG_EAX);
		_setcc_r8(COND_NZ, REG_AL);
		_setcc_r8(COND_C, REG_BH);
		_or_r32_r32(REG_EBX, REG_EDX);
		_mov_m16abs_r16(&FLAG_C, REG_BX);
		_and_r32_r32(REG_ECX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_ECX);

		if (BIT_B(word2))
		{
			link_info link1;

			_test_r32_imm(REG_EBX, CFLAG_SET);
			_jcc_near_link(COND_Z, &link1);

			m68kdrc_exception_trap(EXCEPTION_CHK);

_resolve_link(&link1);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(clr, 8, ., d)
{
	_mov_m8abs_imm(&DY, 0);

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);
}


M68KMAKE_OP(clr, 8, ., .)
{
	M68KMAKE_GET_EA_AY_8;

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);

	_push_imm(0);
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(clr, 16, ., d)
{
	_mov_m16abs_imm(&DY, 0);

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);
}


M68KMAKE_OP(clr, 16, ., .)
{
	M68KMAKE_GET_EA_AY_16;

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);

	_push_imm(0);
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(clr, 32, ., d)
{
	_mov_m32abs_imm(&DY, 0);

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);
}


M68KMAKE_OP(clr, 32, ., .)
{
	M68KMAKE_GET_EA_AY_32;

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_imm(&FLAG_Z, ZFLAG_SET);

	_push_imm(0);
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(cmp, 8, ., d)
{
	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 8, ., .)
{
	M68KMAKE_GET_OPER_AY_8;
	_mov_r16_r16(REG_CX, REG_AX);

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 16, ., d)
{
	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_movzx_r32_m16abs(REG_ECX, &DY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 16, ., a)
{
	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_movzx_r32_m16abs(REG_ECX, &AY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 32, ., d)
{
	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_m32abs(REG_ECX, &DY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 32, ., a)
{
	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_m32abs(REG_ECX, &AY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmp, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 16, ., d)
{
	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_movsx_r32_m16abs(REG_ECX, &DY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 16, ., a)
{
	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_movsx_r32_m16abs(REG_ECX, &AY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_ECX, REG_AX);

	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 32, ., d)
{
	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_m32abs(REG_ECX, &DY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 32, ., a)
{
	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_m32abs(REG_ECX, &AY);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpa, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_mov_r32_m32abs(REG_EBX, &AX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 8, ., d)
{
	uint8 src = OPER_I_8();

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_imm(REG_CL, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 8, ., .)
{
	uint8 src = OPER_I_8();

	_xor_r32_r32(REG_EBX, REG_EBX);
	M68KMAKE_GET_OPER_AY_8;
	_mov_r16_r16(REG_BX, REG_AX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_imm(REG_CL, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 8, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint8 src = OPER_I_8();

		DRC_OPER_PCDI_8();
		_mov_r16_r16(REG_BX, REG_AX);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_mov_r8_imm(REG_CL, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpi, 8, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint8 src = OPER_I_8();

		DRC_OPER_PCIX_8();
		_mov_r16_r16(REG_BX, REG_AX);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_mov_r8_imm(REG_CL, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpi, 16, ., d)
{
	uint16 src = OPER_I_16();

	_movzx_r32_m16abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 16, ., .)
{
	uint16 src = OPER_I_16();

	M68KMAKE_GET_OPER_AY_16;
	_mov_r32_r32(REG_EBX, REG_EAX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r16_imm(REG_CX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 16, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 src = OPER_I_16();

		DRC_OPER_PCDI_16();
		_mov_r32_r32(REG_EBX, REG_EAX);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_mov_r16_imm(REG_CX, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpi, 16, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 src = OPER_I_16();

		DRC_OPER_PCIX_16();
		_mov_r32_r32(REG_EBX, REG_EAX);

		_xor_r32_r32(REG_ECX, REG_ECX);
		_mov_r16_imm(REG_CX, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpi, 32, ., d)
{
	uint32 src = OPER_I_32();

	m68kdrc_cmpild_callback(src, REG68K_IR & 7);		   /* auto-disable (see m68kcpu.h) */

	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 32, ., .)
{
	uint32 src = OPER_I_32();

	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpi, 32, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint32 src = OPER_I_32();

		DRC_OPER_PCDI_32();
		_mov_r32_r32(REG_EBX, REG_EAX);

		_mov_r32_imm(REG_ECX, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpi, 32, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint32 src = OPER_I_32();

		DRC_OPER_PCIX_32();
		_mov_r32_r32(REG_EBX, REG_EAX);

		_mov_r32_imm(REG_ECX, src);

		_sub_r32_r32(REG_EAX, REG_ECX);

		m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(cmpm, 8, ., ax7)
{
	DRC_OPER_AY_PI_8();
	_push_r32(REG_EAX);

	DRC_OPER_A7_PI_8();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpm, 8, ., ay7)
{
	DRC_OPER_A7_PI_8();
	_push_r32(REG_EAX);

	DRC_OPER_AX_PI_8();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpm, 8, ., axy7)
{
	DRC_OPER_A7_PI_8();
	_push_r32(REG_EAX);

	DRC_OPER_A7_PI_8();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpm, 8, ., .)
{
	DRC_OPER_AY_PI_8();
	_push_r32(REG_EAX);

	DRC_OPER_AX_PI_8();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_8(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpm, 16, ., .)
{
	DRC_OPER_AY_PI_16();
	_push_r32(REG_EAX);

	DRC_OPER_AX_PI_16();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_16(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cmpm, 32, ., .)
{
	DRC_OPER_AY_PI_32();
	_push_r32(REG_EAX);

	DRC_OPER_AX_PI_32();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_pop_r32(REG_ECX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncz_flag_cmp_32(drc);		/* break EBX, ECX */
}


M68KMAKE_OP(cpbcc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
#if 0
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_1111();
}


M68KMAKE_OP(cpdbcc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
#if 0
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_1111();
}


M68KMAKE_OP(cpgen, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
#if 0
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_1111();
}


M68KMAKE_OP(cpscc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
#if 0
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_1111();
}


M68KMAKE_OP(cptrapcc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
#if 0
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_1111();
}


M68KMAKE_OP(dbt, 16, ., .)
{
	REG68K_PC += 2;
}


M68KMAKE_OP(dbf, 16, ., .)
{
	link_info link1;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_sub_r32_imm(REG_EAX, 1);
	_mov_m16abs_r16(&DY, REG_AX);

	_jcc_near_link(COND_S, &link1);

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	DRC_USE_CYCLES(CYC_DBCC_F_NOEXP);
	m68kdrc_branch_16(OPER_I_16(), 0);

_resolve_link(&link1);
	DRC_USE_CYCLES(CYC_DBCC_F_EXP);
}


M68KMAKE_OP(dbcc, 16, ., .)
{
	link_info link1;

	M68KMAKE_NOT_CC;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_sub_r32_imm(REG_EAX, 1);
	_mov_m16abs_r16(&DY, REG_AX);

	_jcc_near_link(COND_S, &link1);

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	DRC_USE_CYCLES(CYC_DBCC_F_NOEXP);
	m68kdrc_branch_16(OPER_I_16(), 0);

_resolve_link(&link1);
	DRC_USE_CYCLES(CYC_DBCC_F_EXP);

_resolve_link(&link_make_cc);
}


M68KMAKE_OP(divs, 16, ., d)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_movsx_r32_m16abs(REG_EBX, &DY);

	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	_mov_r32_m32abs(REG_EAX, &DX);

	_cdq();
	_idiv_r32(REG_EBX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link2);
	_cmp_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link3);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link4);

_resolve_link(&link2);
_resolve_link(&link3);
	m68kdrc_vncz_flag_move_16(drc);

	_shl_r32_imm(REG_EDX, 16);
	_or_r32_r32(REG_EAX, REG_EDX);

	_mov_m32abs_r32(&DX, REG_EAX);

_resolve_link(&link4);
}


M68KMAKE_OP(divs, 16, ., .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EBX, REG_AX);

	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	_mov_r32_m32abs(REG_EAX, &DX);

	_cdq();
	_idiv_r32(REG_EBX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link2);
	_cmp_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link3);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link4);

_resolve_link(&link2);
_resolve_link(&link3);
	m68kdrc_vncz_flag_move_16(drc);

	_shl_r32_imm(REG_EDX, 16);
	_or_r32_r32(REG_EAX, REG_EDX);

	_mov_m32abs_r32(&DX, REG_EAX);

_resolve_link(&link4);
}


M68KMAKE_OP(divu, 16, ., d)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m16abs(REG_EBX, &DY);

	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	_mov_r32_m32abs(REG_EAX, &DX);

	_xor_r32_r32(REG_EDX, REG_EDX);
	_div_r32(REG_EBX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link2);
	m68kdrc_vncz_flag_move_16(drc);

	_shl_r32_imm(REG_EDX, 16);
	_or_r32_r32(REG_EAX, REG_EDX);

	_mov_m32abs_r32(&DX, REG_EAX);

_resolve_link(&link3);
}


M68KMAKE_OP(divu, 16, ., .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	M68KMAKE_GET_OPER_AY_16;
	_movzx_r32_r16(REG_EBX, REG_AX);

	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	_mov_r32_m32abs(REG_EAX, &DX);

	_xor_r32_r32(REG_EDX, REG_EDX);
	_div_r32(REG_EBX);

	_mov_r32_r32(REG_EBX, REG_EAX);
	_and_r32_imm(REG_EBX, ~0xffff);
	_jcc_near_link(COND_Z, &link2);

	_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
	_jmp_near_link(&link3);

_resolve_link(&link2);
	m68kdrc_vncz_flag_move_16(drc);

	_shl_r32_imm(REG_EDX, 16);
	_or_r32_r32(REG_EAX, REG_EDX);

	_mov_m32abs_r32(&DX, REG_EAX);

_resolve_link(&link3);
}


M68KMAKE_OP(divl, 32, ., d)
{
	link_info link1;

	uint16 word2 = OPER_I_16();

	if (!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_illegal();

	_mov_r32_m32abs(REG_EBX, &DY);
	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	if (BIT_A(word2))	/* 64 bit */
	{
		link_info link2;
		link_info link3;

		_mov_r32_m32abs(REG_EDX, &REG68K_D[word2 & 7]);	/* dividend (high) */

		if (BIT_B(word2))	/* signed */
		{
			link_info link4;
			link_info link5;

			_mov_r32_r32(REG_ECX, REG_EDX);
			_test_r32_imm(REG_ECX, 0x8000);
			_jcc_near_link(COND_Z, &link4);

			_neg_r32(REG_ECX);

_resolve_link(&link4);
			_mov_r32_r32(REG_EAX, REG_EBX);
			_test_r32_imm(REG_EAX, 0x8000);

			_jcc_near_link(COND_Z, &link5);

			_neg_r32(REG_EAX);

_resolve_link(&link5);
			_sub_r32_r32(REG_ECX, REG_EAX);
			_jcc_near_link(COND_C, &link2);
		}
		else	   		/* unsigned */
		{
			_mov_r32_r32(REG_EAX, REG_EDX);
			_sub_r32_r32(REG_EAX, REG_EBX);
			_jcc_near_link(COND_C, &link2);
		}

		/* overflow */
		_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);		/* dividend (low) */

		if(BIT_B(word2))	/* signed */
			_idiv_r32(REG_EBX);
		else			/* unsigned */
			_div_r32(REG_EBX);

		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);		/* remainder */
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);	/* quotient */

		m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

_resolve_link(&link3);
	}
	else	/* 32 bit */
	{
		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);

		if(BIT_B(word2))	/* signed */
		{
			_cdq();
			_idiv_r32(REG_EBX);
		}
		else			/* unsigned */
		{
			_xor_r32_r32(REG_EDX, REG_EDX);
			_div_r32(REG_EBX);
		}

		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);		/* remainder */
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);	/* quotient */

		m68kdrc_vncz_flag_move_32(drc);		/* break ECX */
	}
}


M68KMAKE_OP(divl, 32, ., .)
{
	link_info link1;

	uint16 word2 = OPER_I_16();

	if (!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_illegal();

	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_EBX, REG_EAX);
	_or_r32_r32(REG_EBX, REG_EBX);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_trap(EXCEPTION_ZERO_DIVIDE);

_resolve_link(&link1);
	if (BIT_A(word2))	/* 64 bit */
	{
		link_info link2;
		link_info link3;

		_mov_r32_m32abs(REG_EDX, &REG68K_D[word2 & 7]);	/* dividend (high) */

		if (BIT_B(word2))	/* signed */
		{
			link_info link4;
			link_info link5;

			_mov_r32_r32(REG_ECX, REG_EDX);
			_test_r32_imm(REG_ECX, 0x8000);
			_jcc_near_link(COND_Z, &link4);

			_neg_r32(REG_ECX);

_resolve_link(&link4);
			_mov_r32_r32(REG_EAX, REG_EBX);
			_test_r32_imm(REG_EAX, 0x8000);

			_jcc_near_link(COND_Z, &link5);

			_neg_r32(REG_EAX);

_resolve_link(&link5);
			_sub_r32_r32(REG_ECX, REG_EAX);
			_jcc_near_link(COND_C, &link2);
		}
		else	   		/* unsigned */
		{
			_mov_r32_r32(REG_EAX, REG_EDX);
			_sub_r32_r32(REG_EAX, REG_EBX);
			_jcc_near_link(COND_C, &link2);
		}

		/* overflow */
		_mov_m8abs_imm(&FLAG_V, VFLAG_SET);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);		/* dividend (low) */

		if(BIT_B(word2))	/* signed */
			_idiv_r32(REG_EBX);
		else			/* unsigned */
			_div_r32(REG_EBX);

		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);		/* remainder */
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);	/* quotient */

		m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

_resolve_link(&link3);
	}
	else	/* 32 bit */
	{
		_mov_r32_m32abs(REG_EAX, &REG68K_D[(word2 >> 12) & 7]);

		if(BIT_B(word2))	/* signed */
		{
			_cdq();
			_idiv_r32(REG_EBX);
		}
		else			/* unsigned */
		{
			_xor_r32_r32(REG_EDX, REG_EDX);
			_div_r32(REG_EBX);
		}

		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);		/* remainder */
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);	/* quotient */

		m68kdrc_vncz_flag_move_32(drc);		/* break ECX */
	}
}


M68KMAKE_OP(eor, 8, ., d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	_mov_r8_m8abs(REG_BL, &DX);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(eor, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_BL, &DX);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(eor, 16, ., d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	_mov_r16_m16abs(REG_BX, &DX);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(eor, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r16_m16abs(REG_BX, &DX);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(eor, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_xor_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(eor, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_xor_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(eori, 8, ., d)
{
	uint8 src = OPER_I_8();

	_mov_r8_m8abs(REG_AL, &DY);

	_mov_r8_imm(REG_BL, src);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(eori, 8, ., .)
{
	uint8 src = OPER_I_8();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_imm(REG_BL, src);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(eori, 16, ., d)
{
	uint16 src = OPER_I_16();

	_mov_r16_m16abs(REG_AX, &DY);

	_mov_r16_imm(REG_BX, src);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(eori, 16, ., .)
{
	uint16 src = OPER_I_16();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r16_imm(REG_BX, src);
	_xor_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(eori, 32, ., d)
{
	uint32 src = OPER_I_32();

	_mov_r32_m32abs(REG_EAX, &DY);

	_xor_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(eori, 32, ., .)
{
	uint32 src = OPER_I_32();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_xor_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(eori, 16, toc, .)
{
	uint16 src = OPER_I_16();

	m68kdrc_get_ccr();

	_xor_r32_imm(REG_EAX, src);

	m68kdrc_set_ccr(drc);
}


M68KMAKE_OP(eori, 16, tos, .)
{
	link_info link1;

	uint16 src = OPER_I_16();

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	m68kdrc_get_sr();
	_xor_r32_imm(REG_EAX, src);
	m68kdrc_set_sr(drc);
}


M68KMAKE_OP(exg, 32, dd, .)
{
	_mov_r32_m32abs(REG_EAX, &DX);
	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_m32abs_r32(&DX, REG_EBX);
	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(exg, 32, aa, .)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_mov_r32_m32abs(REG_EBX, &AY);
	_mov_m32abs_r32(&AX, REG_EBX);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(exg, 32, da, .)
{
	_mov_r32_m32abs(REG_EAX, &DX);
	_mov_r32_m32abs(REG_EBX, &AY);
	_mov_m32abs_r32(&DX, REG_EBX);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(ext, 16, ., .)
{
	_movsx_r32_m8abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(ext, 32, ., .)
{
	_movsx_r32_m16abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(extb, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		_movsx_r32_m8abs(REG_EAX, &DY);

		m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

		_mov_m32abs_r32(&DY, REG_EAX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(illegal, 0, ., .)
{
	m68kdrc_exception_illegal();
}

M68KMAKE_OP(jmp, 32, ., .)
{
	M68KMAKE_GET_EA_AY_32;

	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_jump(drc);
}


M68KMAKE_OP(jsr, 32, ., .)
{
	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_push_32_imm(REG68K_PC);

	_pop_r32(REG_EAX);
	m68kdrc_jump(drc);
}


M68KMAKE_OP(lea, 32, ., .)
{
	M68KMAKE_GET_EA_AY_32;
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(link, 16, ., a7)
{
	uint32 dis = MAKE_INT_16(OPER_I_16());

	_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
	_sub_r32_imm(REG_EAX, 4);

	_push_r32(REG_EAX);
	_push_r32(REG_EAX);
	m68kdrc_write_32();

	_add_m32abs_imm(&REG68K_A[7], dis);
}


M68KMAKE_OP(link, 16, ., .)
{
	uint32 dis = MAKE_INT_16(OPER_I_16());

	m68kdrc_push_32_m32abs(&AY);

	_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
	_mov_m32abs_r32(&AY, REG_EAX);

	_add_m32abs_imm(&REG68K_A[7], dis);
}


M68KMAKE_OP(link, 32, ., a7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint32 dis = OPER_I_32();

		_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
		_sub_r32_imm(REG_EAX, 4);

		_push_r32(REG_EAX);
		_push_r32(REG_EAX);
		m68kdrc_write_32();

		_add_m32abs_imm(&REG68K_A[7], dis);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(link, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint32 dis = OPER_I_32();

		m68kdrc_push_32_m32abs(&AY);

		_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
		_mov_m32abs_r32(&AY, REG_EAX);
		_add_m32abs_imm(&REG68K_A[7], dis);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(lsr, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_shr_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(lsr, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_shr_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(lsr, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_shr_r32_imm(REG_EAX, shift);
	_mov_m32abs_r32(&DY, REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
}


M68KMAKE_OP(lsr, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shr_r8_cl(REG_AL);
	_shr_r8_cl(REG_AL);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_shr_r8_cl(REG_AL);
	_mov_m8abs_r8(&DY, REG_AL);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(lsr, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shr_r16_cl(REG_AX);
	_shr_r16_cl(REG_AX);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_shr_r16_cl(REG_AX);
	_mov_m16abs_r16(&DY, REG_AX);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(lsr, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_shr_r32_imm(REG_EAX, 16);
	_shr_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_shr_r32_cl(REG_EAX);
	_mov_m32abs_r32(&DY, REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(lsr, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_shr_r32_imm(REG_EAX, 1);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_imm(&FLAG_N, NFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(lsl, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_shl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(lsl, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_shl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(lsl, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_shl_r32_imm(REG_EAX, shift);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_32();		/* break ECX */
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(lsl, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m8abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shl_r8_cl(REG_AL);
	_shl_r8_cl(REG_AL);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_shl_r8_cl(REG_AL);
	_mov_m8abs_r8(&DY, REG_AL);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(lsl, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_shl_r16_cl(REG_AX);
	_shl_r16_cl(REG_AX);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_shl_r16_cl(REG_AX);
	_mov_m16abs_r16(&DY, REG_AX);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(lsl, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_shl_r32_imm(REG_EAX, 16);
	_shl_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_shl_r32_cl(REG_EAX);
	_mov_m32abs_r32(&DY, REG_EAX);

	DRC_CXFLAG_COND_C();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

_resolve_link(&link3);
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}



M68KMAKE_OP(lsl, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_shl_r32_imm(REG_EAX, 1);

	DRC_CXFLAG_16();			/* break EBX */
	DRC_NFLAG_16();
	_movzx_r32_r16(REG_EAX, REG_AX);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 8, d, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(move, 8, d, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(move, 8, ai, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, ai, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pi7, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_A7_PI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pi, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pi7, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_A7_PI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pi, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pd7, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pd, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pd7, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, pd, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, di, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, di, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, ix, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, ix, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, aw, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AW_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, aw, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AW_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, al, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AL_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 8, al, .)
{
	M68KMAKE_GET_OPER_AY_8;

	m68kdrc_vncz_flag_move_8(drc);

	_push_r32(REG_EAX);
	DRC_EA_AL_8();
	_push_r32(REG_EAX);
	m68kdrc_write_8();
}


M68KMAKE_OP(move, 16, d, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(move, 16, d, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(move, 16, d, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(move, 16, ai, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, ai, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, ai, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pi, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pi, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pi, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pd, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pd, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, pd, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, di, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, di, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, di, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, ix, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, ix, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, ix, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, aw, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AW_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, aw, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AW_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, aw, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AW_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, al, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AL_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, al, a)
{
	_mov_r16_m16abs(REG_AX, &AY);

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AL_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, al, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_vncz_flag_move_16(drc);

	_push_r32(REG_EAX);
	DRC_EA_AL_16();
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 32, d, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(move, 32, d, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(move, 32, d, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(move, 32, ai, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, ai, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, ai, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_AI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pi, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pi, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pi, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pd, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pd, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, pd, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_PD_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, di, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, di, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, di, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_DI_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, ix, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, ix, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, ix, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AX_IX_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, aw, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AW_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, aw, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AW_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, aw, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AW_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, al, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AL_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, al, a)
{
	_mov_r32_m32abs(REG_EAX, &AY);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AL_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(move, 32, al, .)
{
	M68KMAKE_GET_OPER_AY_32;

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_push_r32(REG_EAX);
	DRC_EA_AL_32();
	_push_r32(REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(movea, 16, ., d)
{
	_movsx_r32_m16abs(REG_EAX, &DY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(movea, 16, ., a)
{
	_movsx_r32_m16abs(REG_EAX, &AY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(movea, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EAX, REG_AX);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(movea, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(movea, 32, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(movea, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(move, 16, frc, d)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		m68kdrc_get_ccr();
		_mov_m16abs_r16(&DY, REG_AX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(move, 16, frc, .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		m68kdrc_get_ccr();

		_push_r32(REG_EAX);
		M68KMAKE_GET_EA_AY_16;
		_push_r32(REG_EAX);
		m68kdrc_write_16();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(move, 16, toc, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	m68kdrc_set_ccr(drc);
}


M68KMAKE_OP(move, 16, toc, .)
{
	M68KMAKE_GET_OPER_AY_16;

	m68kdrc_set_ccr(drc);
}


M68KMAKE_OP(move, 16, frs, d)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))	/* NS990408 */
	{
		link_info link1;

		_test_m8abs_imm(&FLAG_S, SFLAG_SET);
		_jcc_near_link(COND_NZ, &link1);

		m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	}

	m68kdrc_get_sr();

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(move, 16, frs, .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))	/* NS990408 */
	{
		link_info link1;

		_test_m8abs_imm(&FLAG_S, SFLAG_SET);
		_jcc_near_link(COND_NZ, &link1);

		m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	}

	m68kdrc_get_sr();

	_push_r32(REG_EAX);
	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);
	m68kdrc_write_16();
}


M68KMAKE_OP(move, 16, tos, d)
{
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	_mov_r16_m16abs(REG_AX, &DY);
	m68kdrc_set_sr(drc);
}


M68KMAKE_OP(move, 16, tos, .)
{
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	M68KMAKE_GET_OPER_AY_16;
	m68kdrc_set_sr(drc);
}


M68KMAKE_OP(move, 32, fru, .)
{
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	_mov_r32_m32abs(REG_EAX, &REG68K_USP);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(move, 32, tou, .)
{
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	_mov_r32_m32abs(REG_EAX, &AY);
	_mov_m32abs_r32(&REG68K_USP, REG_EAX);
}


M68KMAKE_OP(movec, 32, cr, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	uint16 word2;

	if(!CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		m68kdrc_exception_illegal();
		return;
	}

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	word2 = OPER_I_16();

	m68ki_trace_t0();		   /* auto-disable (see m68kcpu.h) */

	if(!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		switch (word2 & 0xfff)
		{
		case 0x802:			   /* CAAR */
		case 0x803:			   /* MSP */
		case 0x804:			   /* ISP */
		case 0x002:			   /* CACR */
			word2 = 0xffff;		/* illegal */
		}
	}

	switch (word2 & 0xfff)
	{
	case 0x000:			   /* SFC */
		_mov_r32_m32abs(REG_EAX, &REG68K_SFC);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x001:			   /* DFC */
		_mov_r32_m32abs(REG_EAX, &REG68K_DFC);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x002:			   /* CACR */
		_mov_r32_m32abs(REG_EAX, &REG68K_CACR);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x800:			   /* USP */
		_mov_r32_m32abs(REG_EAX, &REG68K_USP);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x801:			   /* VBR */
		_mov_r32_m32abs(REG_EAX, &REG68K_VBR);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x802:			   /* CAAR */
		_mov_r32_m32abs(REG_EAX, &REG68K_CAAR);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x803:			   /* MSP */
		_test_m8abs_imm(&FLAG_M, MFLAG_SET);
		_jcc_near_link(COND_Z, &link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_SP);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_MSP);

_resolve_link(&link3);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x804:			   /* ISP */
		_test_m8abs_imm(&FLAG_M, MFLAG_SET);
		_jcc_near_link(COND_Z, &link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_ISP);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_r32_m32abs(REG_EAX, &REG68K_SP);

_resolve_link(&link3);
		_mov_m32abs_r32(&REG68K_DA[(word2 >> 12) & 15], REG_EAX);
		break;
	case 0x003:				/* TC */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x004:				/* ITT0 */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x005:				/* ITT1 */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x006:				/* DTT0 */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x007:				/* DTT1 */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x805:				/* MMUSR */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x806:				/* URP */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x807:				/* SRP */
		if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	default:
		m68kdrc_exception_illegal();
	}
}


M68KMAKE_OP(movec, 32, rc, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	uint16 word2;

	if(!CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		m68kdrc_exception_illegal();
		return;
	}

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	word2 = OPER_I_16();

	m68ki_trace_t0();		   /* auto-disable (see m68kcpu.h) */

	if(!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		switch (word2 & 0xfff)
		{
		case 0x802:			   /* CAAR */
		case 0x803:			   /* MSP */
		case 0x804:			   /* ISP */
		case 0x002:			   /* CACR */
			word2 = 0xffff;		/* illegal */
		}
	}

	_mov_r32_m32abs(REG_EAX, &REG68K_DA[(word2 >> 12) & 15]);

	switch (word2 & 0xfff)
	{
	case 0x000:			   /* SFC */
		_mov_m32abs_r32(&REG68K_SFC, REG_EAX);
		break;
	case 0x001:			   /* DFC */
		_mov_m32abs_r32(&REG68K_DFC, REG_EAX);
		break;
	case 0x002:			   /* CACR */
		_mov_m32abs_r32(&REG68K_CACR, REG_EAX);
		break;
	case 0x800:			   /* USP */
		_mov_m32abs_r32(&REG68K_USP, REG_EAX);
		break;
	case 0x801:			   /* VBR */
		_mov_m32abs_r32(&REG68K_VBR, REG_EAX);
		break;
	case 0x802:			   /* CAAR */
		_mov_m32abs_r32(&REG68K_CAAR, REG_EAX);
		break;
	case 0x803:			   /* MSP */
		_test_m8abs_imm(&FLAG_M, MFLAG_SET);
		_jcc_near_link(COND_Z, &link2);
		_mov_m32abs_r32(&REG68K_SP, REG_EAX);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_m32abs_r32(&REG68K_MSP, REG_EAX);

_resolve_link(&link3);
		break;
	case 0x804:			   /* ISP */
		_test_m8abs_imm(&FLAG_M, MFLAG_SET);
		_jcc_near_link(COND_Z, &link2);
		_mov_m32abs_r32(&REG68K_ISP, REG_EAX);
		_jmp_near_link(&link3);

_resolve_link(&link2);
		_mov_m32abs_r32(&REG68K_SP, REG_EAX);

_resolve_link(&link3);
		break;
	case 0x003:			/* TC */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x004:			/* ITT0 */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x005:			/* ITT1 */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x006:			/* DTT0 */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x007:			/* DTT1 */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x805:			/* MMUSR */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x806:			/* URP */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	case 0x807:			/* SRP */
		if (CPU_TYPE_IS_040_PLUS(CPU_TYPE))
		{
			/* TODO */
			return;
		}
		m68kdrc_exception_illegal();
		return;
	default:
		m68kdrc_exception_illegal();
	}
}


M68KMAKE_OP(movem, 16, re, pd)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	_mov_r32_m32abs(REG_EAX, &AY);

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_sub_r32_imm(REG_EAX, 2);
			_push_r32(REG_EAX);

			_push_m32abs(&REG68K_DA[15-i]);
			_push_r32(REG_EAX);
			m68kdrc_write_16();

			_pop_r32(REG_EAX);
			count++;
		}

	_mov_m32abs_r32(&AY, REG_EAX);

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, re, .)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	M68KMAKE_GET_EA_AY_16;

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_m32abs(&REG68K_DA[i]);
			_push_r32(REG_EAX);
			m68kdrc_write_16();

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 2);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 32, re, pd)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	_mov_r32_m32abs(REG_EAX, &AY);

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_sub_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);

			_push_m32abs(&REG68K_DA[15-i]);
			_push_r32(REG_EAX);
			m68kdrc_write_32();

			_pop_r32(REG_EAX);
			count++;
		}

	_mov_m32abs_r32(&AY, REG_EAX);

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, re, .)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	M68KMAKE_GET_EA_AY_32;

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_m32abs(&REG68K_DA[i]);
			_push_r32(REG_EAX);
			m68kdrc_write_32();

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 16, er, pi)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	_mov_r32_m32abs(REG_EAX, &AY);

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_16();

			_movsx_r32_r16(REG_EAX, REG_AX);
			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 2);

			count++;
		}

	_mov_m32abs_r32(&AY, REG_EAX);

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, pcdi)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	DRC_EA_PCDI_16();

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_pcrel_16();

			_movsx_r32_r16(REG_EAX, REG_AX);
			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 2);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, pcix)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	DRC_EA_PCIX_16();

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_pcrel_16();

			_movsx_r32_r16(REG_EAX, REG_AX);
			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 2);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, .)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	M68KMAKE_GET_EA_AY_16;

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_16();

			_movsx_r32_r16(REG_EAX, REG_AX);
			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 2);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_W);
}

M68KMAKE_OP(movem, 32, er, pi)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	_mov_r32_m32abs(REG_EAX, &AY);

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_32();

			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);

			count++;
		}

	_mov_m32abs_r32(&AY, REG_EAX);

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, pcdi)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	DRC_EA_PCDI_32();

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_pcrel_32();

			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, pcix)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	DRC_EA_PCIX_32();

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_pcrel_32();

			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, .)
{
	uint16 register_list = OPER_I_16();
	int count = 0;
	int i;

	M68KMAKE_GET_EA_AY_32;

	for (i = 0; i < 16; i++)
		if(register_list & (1 << i))
		{
			_push_r32(REG_EAX);

			_push_r32(REG_EAX);
			m68kdrc_read_32();

			_mov_m32abs_r32(&REG68K_DA[i], REG_EAX);

			_pop_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);

			count++;
		}

	DRC_USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movep, 16, re, .)
{
	_mov_r32_m32abs(REG_EAX, &DX);

	_push_r32(REG_EAX);		/* (ea + 2) */
	_sub_r32_imm(REG_ESP, 4);

	_shr_r32_imm(REG_EAX, 8);
	_push_r32(REG_EAX);		/* (ea) */

	DRC_EA_AY_DI_16();
	_push_r32(REG_EAX);

	_add_r32_imm(REG_EAX, 2);
	_mov_m32bd_r32(REG_ESP, 8, REG_EAX);

	m68kdrc_write_8();
	m68kdrc_write_8();
}


M68KMAKE_OP(movep, 32, re, .)
{
	_mov_r32_m32abs(REG_EAX, &DX);

	_push_r32(REG_EAX);		/* (ea + 6) */
	_sub_r32_imm(REG_ESP, 4);

	_shr_r32_imm(REG_EAX, 8);
	_push_r32(REG_EAX);		/* (ea + 4) */
	_sub_r32_imm(REG_ESP, 4);

	_shr_r32_imm(REG_EAX, 8);
	_push_r32(REG_EAX);		/* (ea + 2) */
	_sub_r32_imm(REG_ESP, 4);

	_shr_r32_imm(REG_EAX, 8);
	_push_r32(REG_EAX);		/* (ea) */

	DRC_EA_AY_DI_32();

	_push_r32(REG_EAX);

	_add_r32_imm(REG_EAX, 2);
	_mov_m32bd_r32(REG_ESP, 8, REG_EAX);

	_add_r32_imm(REG_EAX, 2);
	_mov_m32bd_r32(REG_ESP, 16, REG_EAX);

	_add_r32_imm(REG_EAX, 2);
	_mov_m32bd_r32(REG_ESP, 24, REG_EAX);

	m68kdrc_write_8();
	m68kdrc_write_8();
	m68kdrc_write_8();
	m68kdrc_write_8();
}


M68KMAKE_OP(movep, 16, er, .)
{
	_push_imm(0);

	DRC_EA_AY_DI_16();

	_push_r32(REG_EAX);
	_add_r32_imm(REG_EAX, 2);
	_push_r32(REG_EAX);

	m68kdrc_read_8();		/* ea + 2 */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);

	m68kdrc_read_8();		/* ea */
	_shl_r32_imm(REG_EAX, 8);

	_pop_r32(REG_EBX);
	_or_r32_r32(REG_EAX, REG_EBX);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(movep, 32, er, .)
{
	_push_imm(0);

	DRC_EA_AY_DI_32();

	_lea_r32_m32bd(REG_EBX, REG_EAX, 6);
	_push_r32(REG_EBX);		/* ea + 6 */

	_sub_r32_imm(REG_EBX, 2);
	_push_r32(REG_EBX);		/* ea + 4 */

	_sub_r32_imm(REG_EBX, 2);
	_push_r32(REG_EBX);		/* ea + 2 */

	_push_r32(REG_EAX);		/* ea */

	m68kdrc_read_8();
	_mov_m8bd_r8(REG_ESP, 12 + 3, REG_AL);	/* (ea) */

	m68kdrc_read_8();
	_mov_m8bd_r8(REG_ESP, 8 + 2, REG_AL);	/* (ea + 2) */

	m68kdrc_read_8();
	_mov_m8bd_r8(REG_ESP, 4 + 1, REG_AL);	/* (ea + 4) */

	m68kdrc_read_8();

	_pop_r32(REG_ECX);
	_or_r32_r32(REG_EAX, REG_ECX);
	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(moves, 8, ., .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		link_info link1;
		uint16 word2 = OPER_I_16();

		_test_m8abs_imm(&FLAG_S, SFLAG_SET);
		_jcc_near_link(COND_NZ, &link1);

		m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
		M68KMAKE_GET_EA_AY_8;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		if (BIT_B(word2))		   /* Register to memory */
		{
			_push_m32abs(&REG68K_DA[(word2 >> 12) & 15]);
			_push_r32(REG_EAX);

			m68kdrc_write_8_fc(&REG68K_DFC);
		}
		else				   /* Memory to address register */
		{
			_push_r32(REG_EAX);
			m68kdrc_read_8_fc(&REG68K_SFC);

			if (BIT_F(word2))
			{
				_movsx_r32_r8(REG_EAX, REG_AL);
				_mov_m32abs_r32(&REG68K_A[(word2 >> 12) & 7], REG_EAX);
			}
			else
				_mov_m8abs_r8(&REG68K_A[(word2 >> 12) & 7], REG_AL);

			if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
				DRC_USE_CYCLES(2);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(moves, 16, ., .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		link_info link1;
		uint16 word2 = OPER_I_16();

		_test_m8abs_imm(&FLAG_S, SFLAG_SET);
		_jcc_near_link(COND_NZ, &link1);

		m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
		M68KMAKE_GET_EA_AY_16;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		if (BIT_B(word2))		   /* Register to memory */
		{
			_push_m32abs(&REG68K_DA[(word2 >> 12) & 15]);
			_push_r32(REG_EAX);

			m68kdrc_write_16_fc(&REG68K_DFC);
		}
		else				   /* Memory to address register */
		{
			_push_r32(REG_EAX);
			m68kdrc_read_16_fc(&REG68K_SFC);

			if (BIT_F(word2))
			{
				_movsx_r32_r16(REG_EAX, REG_AX);
				_mov_m32abs_r32(&REG68K_A[(word2 >> 12) & 7], REG_EAX);
			}
			else
				_mov_m16abs_r16(&REG68K_A[(word2 >> 12) & 7], REG_AX);

			if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
				DRC_USE_CYCLES(2);
		}
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(moves, 32, ., .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		link_info link1;
		uint16 word2 = OPER_I_16();

		_test_m8abs_imm(&FLAG_S, SFLAG_SET);
		_jcc_near_link(COND_NZ, &link1);

		m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
		M68KMAKE_GET_EA_AY_32;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		if (BIT_B(word2))		   /* Register to memory */
		{
			_push_m32abs(&REG68K_DA[(word2 >> 12) & 15]);
			_push_r32(REG_EAX);

			m68kdrc_write_32_fc(&REG68K_DFC);
		}
		else				   /* Memory to address register */
		{
			_push_r32(REG_EAX);
			m68kdrc_read_32_fc(&REG68K_SFC);

			_mov_m32abs_r32(&REG68K_A[(word2 >> 12) & 7], REG_EAX);
		}

		if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
			DRC_USE_CYCLES(2);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(moveq, 32, ., .)
{
	uint res = MAKE_INT_8(MASK_OUT_ABOVE_8(REG68K_IR));

	_mov_r32_imm(REG_EAX, res);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(move16, 32, ., .)
{
	uint16 w2 = OPER_I_16();

	int ax = REG68K_IR & 7;
	int ay = (w2 >> 12) & 7;

	int i;

	for (i = 0; i < 4; i++)
	{
		_push_m32abs(&REG68K_A[ax]);
		m68kdrc_read_32();

		_push_r32(REG_EAX);
		_push_m32abs(&REG68K_A[ay]);
		m68kdrc_write_32();

		_add_m32abs_imm(&REG68K_A[ax], 4);
		_add_m32abs_imm(&REG68K_A[ay], 4);
	}
}


M68KMAKE_OP(muls, 16, ., d)
{
	_movsx_r32_m16abs(REG_EAX, &DY);
	_movsx_r32_m16abs(REG_EBX, &DX);
	_imul_r32(REG_EBX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(muls, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EAX, REG_AX);
	_movsx_r32_m16abs(REG_EBX, &DX);
	_imul_r32(REG_EBX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(mulu, 16, ., d)
{
	_movzx_r32_m16abs(REG_EAX, &DY);
	_movzx_r32_m16abs(REG_EBX, &DX);
	_mul_r32(REG_EBX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(mulu, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movzx_r32_r16(REG_EAX, REG_AX);
	_movzx_r32_m16abs(REG_EBX, &DX);
	_mul_r32(REG_EBX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(mull, 32, ., d)
{
	uint16 word2 = OPER_I_16();

	if (!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_illegal();

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 12) & 7]);

	if (BIT_B(word2))			   /* signed */
		_imul_r32(REG_EBX);
	else
		_mul_r32(REG_EBX);

	if (BIT_A(word2))	/* 64 bit */
	{
		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);

		DRC_NFLAG_64();		/* break ECX */
		_or_r32_r32(REG_EDX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else			/* 32 bit */
	{
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		DRC_NFLAG_32();		/* break ECX */
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		if (BIT_B(word2))		/* signed */
		{
			_mov_r32_r32(REG_ECX, REG_EDX);
			_cdq();
			_sub_r32_r32(REG_ECX, REG_EDX);
		}
		else				/* unsigned */
			_or_r32_r32(REG_EDX, REG_EDX);

		_setcc_r8(COND_NZ, REG_DL);
		_shl_r32_imm(REG_EDX, 7);
		_mov_m8abs_r8(&FLAG_V, REG_DL);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);
	}
}


M68KMAKE_OP(mull, 32, ., .)
{
	uint16 word2 = OPER_I_16();

	if (!CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_illegal();

	M68KMAKE_GET_OPER_AY_32;

	_mov_r32_m32abs(REG_EBX, &REG68K_D[(word2 >> 12) & 7]);

	if (BIT_B(word2))			   /* signed */
		_imul_r32(REG_EBX);
	else
		_mul_r32(REG_EBX);

	if (BIT_A(word2))	/* 64 bit */
	{
		_mov_m32abs_r32(&REG68K_D[word2 & 7], REG_EDX);
		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);

		DRC_NFLAG_64();		/* break ECX */
		_or_r32_r32(REG_EDX, REG_EAX);
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
		_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	}
	else			/* 32 bit */
	{
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		DRC_NFLAG_32();		/* break ECX */
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

		if (BIT_B(word2))		/* signed */
		{
			_mov_r32_r32(REG_ECX, REG_EDX);
			_cdq();
			_sub_r32_r32(REG_ECX, REG_EDX);
		}
		else				/* unsigned */
			_or_r32_r32(REG_EDX, REG_EDX);

		_setcc_r8(COND_NZ, REG_DL);
		_shl_r32_imm(REG_EDX, 7);
		_mov_m8abs_r8(&FLAG_V, REG_DL);

		_mov_m32abs_r32(&REG68K_D[(word2 >> 12) & 7], REG_EAX);
	}
}


M68KMAKE_OP(nbcd, 8, ., d)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r8_m8abs(REG_BL, &DY);

	_mov_r8_imm(REG_AL, 0x9a);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 0x9a);
	_jcc_near_link(COND_NZ, &link1);

	_mov_r8_r8(REG_CL, REG_AL);
	_not_r32(REG_ECX);			/* Undefined V behavior */

	_mov_r8_r8(REG_BL, REG_AL);
	_and_r32_imm(REG_EBX, 0x0f);

	_cmp_r32_imm(REG_EBX, 0x0a);
	_jcc_near_link(COND_NZ, &link2);

	_and_r32_imm(REG_EAX, 0xf0);
	_add_r32_imm(REG_EAX, 0x10);

_resolve_link(&link2);
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	_mov_m8abs_r8(&DY, REG_AL);

	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m16abs_imm(&FLAG_C, CFLAG_SET);
	_mov_m16abs_imm(&FLAG_X, XFLAG_SET);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_8();				/* Undefined N behavior */
}


M68KMAKE_OP(nbcd, 8, ., .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_imm(REG_BL, 0x9a);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EBX, REG_EAX);
	_movzx_r32_r8(REG_EAX, REG_BL);

	_cmp_r32_imm(REG_EAX, 0x9a);
	_jcc_near_link(COND_NZ, &link1);

	_mov_r8_r8(REG_CL, REG_AL);
	_not_r32(REG_ECX);			/* Undefined V behavior */

	_mov_r8_r8(REG_BL, REG_AL);
	_and_r32_imm(REG_EBX, 0x0f);

	_cmp_r32_imm(REG_EBX, 0x0a);
	_jcc_near_link(COND_NZ, &link2);

	_and_r32_imm(REG_EAX, 0xf0);
	_add_r32_imm(REG_EAX, 0x10);

_resolve_link(&link2);
	_and_r32_r32(REG_ECX, REG_EAX);		/* Undefined V behavior part II */
	_mov_m8abs_r8(&FLAG_V, REG_CL);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();

	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m16abs_imm(&FLAG_C, CFLAG_SET);
	_mov_m16abs_imm(&FLAG_X, XFLAG_SET);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_add_r32_imm(REG_ESP, 8);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_X, XFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_8();				/* Undefined N behavior */
}


M68KMAKE_OP(neg, 8, ., d)
{
	_movsx_r32_m8abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_8(drc);		/* break EBX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(neg, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();
	_movsx_r32_r8(REG_EAX, REG_AL);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_8(drc);		/* break EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(neg, 16, ., d)
{
	_movsx_r32_m16abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_16(drc);		/* break EBX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(neg, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_movsx_r32_r16(REG_EAX, REG_AX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_16(drc);		/* break EBX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(neg, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(neg, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);

	m68kdrc_vncxz_flag_neg_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(negx, 8, ., d)
{
	_movsx_r32_m8abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_8(drc);		/* break EBX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(negx, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();
	_movsx_r32_r8(REG_EAX, REG_AL);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_8(drc);		/* break EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(negx, 16, ., d)
{
	_movsx_r32_m16abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_16(drc);	/* break EBX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(negx, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_movsx_r32_r16(REG_EAX, REG_AX);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_16(drc);	/* break EBX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(negx, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_32(drc);	/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(negx, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_r32(REG_ECX, REG_EAX);
	_neg_r32(REG_EAX);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncxz_flag_negx_32(drc);	/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(nop, 0, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
}


M68KMAKE_OP(not, 8, ., d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(not, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(not, 16, ., d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(not, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(not, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(not, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_not_r32(REG_EAX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(or, 8, er, d)
{
	_mov_r8_m8abs(REG_AL, &DY);

	_mov_r8_m8abs(REG_BL, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(or, 8, er, .)
{
	M68KMAKE_GET_OPER_AY_8;

	_mov_r8_m8abs(REG_BL, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(or, 16, er, d)
{
	_mov_r16_m16abs(REG_AX, &DY);

	_mov_r16_m16abs(REG_BX, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(or, 16, er, .)
{
	M68KMAKE_GET_OPER_AY_16;

	_mov_r16_m16abs(REG_BX, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(or, 32, er, d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_or_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(or, 32, er, .)
{
	M68KMAKE_GET_OPER_AY_32;

	_or_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(or, 8, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_m8abs(REG_BL, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(or, 16, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r16_m16abs(REG_BX, &DX);
	_or_r32_r32(REG_EAX, REG_EBX);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(or, 32, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_or_r32_m32abs(REG_EAX, &DX);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(ori, 8, ., d)
{
	uint8 src = OPER_I_8();

	_mov_r8_m8abs(REG_AL, &DY);

	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(ori, 8, ., .)
{
	uint8 src = OPER_I_8();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_8(drc);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(ori, 16, ., d)
{
	uint16 src = OPER_I_16();

	_mov_r16_m16abs(REG_AX, &DY);
	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(ori, 16, ., .)
{
	uint16 src = OPER_I_16();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_16(drc);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(ori, 32, ., d)
{
	uint32 src = OPER_I_32();

	_mov_r32_m32abs(REG_EAX, &DY);
	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(ori, 32, ., .)
{
	uint32 src = OPER_I_32();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_or_r32_imm(REG_EAX, src);

	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(ori, 16, toc, .)
{
	uint16 src = OPER_I_16();

	m68kdrc_get_ccr();

	_or_r32_imm(REG_EAX, src);

	m68kdrc_set_ccr(drc);
}


M68KMAKE_OP(ori, 16, tos, .)
{
	link_info link1;

	uint16 src = OPER_I_16();

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	m68kdrc_get_sr();
	_or_r32_imm(REG_EAX, src);
	m68kdrc_set_sr(drc);
}


M68KMAKE_OP(pack, 16, rr, .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: DX and DY are reversed in Motorola's docs */

		uint16 offset = OPER_I_16();

		_mov_r8_m8abs(REG_AL, &DY);
		_add_r32_imm(REG_EAX, offset);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EAX, 0x0f);
		_shr_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0xf0);
		_or_r32_r32(REG_EAX, REG_EBX);

		_mov_m8abs_r8(&DX, REG_AL);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(pack, 16, mm, ax7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_EA_AY_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_shl_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_AY_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_pop_r32(REG_EBX);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EAX, 0x0f);
		_shr_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0xf0);
		_or_r32_r32(REG_EAX, REG_EBX);

		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(pack, 16, mm, ay7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_shl_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_pop_r32(REG_EBX);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EAX, 0x0f);
		_shr_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0xf0);
		_or_r32_r32(REG_EAX, REG_EBX);

		_push_r32(REG_EAX);

		DRC_EA_AX_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(pack, 16, mm, axy7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 offset;

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_shl_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_pop_r32(REG_EBX);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EAX, 0x0f);
		_shr_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0xf0);
		_or_r32_r32(REG_EAX, REG_EBX);

		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(pack, 16, mm, .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_EA_AY_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_shl_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_AY_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_read_8();

		_pop_r32(REG_EBX);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EAX, 0x0f);
		_shr_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0xf0);
		_or_r32_r32(REG_EAX, REG_EBX);

		_push_r32(REG_EAX);

		DRC_EA_AX_PD_8();

		_push_r32(REG_EAX);
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(pea, 32, ., .)
{
	M68KMAKE_GET_EA_AY_32;

	m68kdrc_push_32_r32(REG_EAX);
}


M68KMAKE_OP(pflush, 32, ., .)
{
	if(CPU_TYPE_IS_040_PLUS(CPU_TYPE))
	{
		// Nothing to do, unless address translation cache is emulated
		return;
	}
	m68kdrc_exception_illegal();
}


M68KMAKE_OP(reset, 0, ., .)
{
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	m68kdrc_output_reset();		   /* auto-disable (see m68kcpu.h) */
	DRC_USE_CYCLES(CYC_RESET);
}


M68KMAKE_OP(ror, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_ror_r8_cl(REG_AL);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(ror, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_ror_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(ror, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_ror_r32_imm(REG_EAX, shift);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(ror, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m8abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_ror_r8_cl(REG_AL);
	_ror_r8_cl(REG_AL);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_ror_r8_cl(REG_AL);

	DRC_CFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(ror, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_ror_r16_cl(REG_AX);
	_ror_r16_cl(REG_AX);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_ror_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(ror, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_ror_r32_imm(REG_EAX, 16);
	_ror_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_ror_r32_cl(REG_EAX);

	DRC_CFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(ror, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r8_imm(REG_CL, 1);
	_ror_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(rol, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_rol_r8_cl(REG_AL);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(rol, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	_rol_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(rol, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_rol_r32_imm(REG_EAX, shift);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(rol, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m8abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_rol_r8_cl(REG_AL);
	_rol_r8_cl(REG_AL);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_rol_r8_cl(REG_AL);

	DRC_CFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(rol, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_mov_r8_r8(REG_CH, REG_CL);
	_mov_r8_imm(REG_CL, 16);
	_rol_r16_cl(REG_AX);
	_rol_r16_cl(REG_AX);
	_mov_r8_r8(REG_CL, REG_CH);

_resolve_link(&link2);
	_rol_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(rol, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_Z, &link2);

	_rol_r32_imm(REG_EAX, 16);
	_rol_r32_imm(REG_EAX, 16);

_resolve_link(&link2);
	_rol_r32_cl(REG_EAX);

	DRC_CFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link1);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

_resolve_link(&link3);
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(rol, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r8_imm(REG_CL, 1);
	_rol_r16_cl(REG_AX);

	DRC_CFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(roxr, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(roxr, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(roxr, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(roxr, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_movzx_r32_m8abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r8_cl(REG_AL);
	_mov_r8_imm(REG_CL, 16);
	_rcr_r8_cl(REG_AL);
	_rcr_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxr, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r16_cl(REG_AX);
	_mov_r8_imm(REG_CL, 16);
	_rcr_r16_cl(REG_AX);
	_rcr_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxr, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r32_cl(REG_EAX);
	_mov_r8_imm(REG_CL, 16);
	_rcr_r32_cl(REG_EAX);
	_rcr_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxr, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r8_imm(REG_CL, 1);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcr_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(roxl, 8, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_8();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(roxl, 16, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_movzx_r32_m16abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(roxl, 32, s, .)
{
	uint shift = (((REG68K_IR >> 9) - 1) & 7) + 1;

	if(shift != 0)
		DRC_USE_CYCLES(shift<<CYC_SHIFT);

	_mov_r32_m32abs(REG_EAX, &DY);

	_mov_r8_imm(REG_CL, shift);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_32();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(roxl, 8, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_movzx_r32_m8abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r8_cl(REG_AL);
	_mov_r8_imm(REG_CL, 16);
	_rcl_r8_cl(REG_AL);
	_rcl_r8_cl(REG_AL);

	DRC_CXFLAG_COND_C();

	_mov_m8abs_r8(&DY, REG_AL);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxl, 16, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_movzx_r32_m16abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r16_cl(REG_AX);
	_mov_r8_imm(REG_CL, 16);
	_rcl_r16_cl(REG_AX);
	_rcl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();

	_mov_m16abs_r16(&DY, REG_AX);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxl, 32, r, .)
{
	link_info link1;
	link_info link2;
	link_info link3;
	link_info link4;

	_mov_r32_m32abs(REG_EAX, &DY);
	_mov_r8_m8abs(REG_CL, &DX);
	_and_r32_imm(REG_ECX, 0x3f);
	_jcc_near_link(COND_Z, &link1);

	if (CYC_SHIFT)
	{
		_mov_r32_r32(REG_EBX, REG_ECX);
		_shl_r32_imm(REG_EBX, CYC_SHIFT);
		_sub_r32_r32(REG_EBP, REG_EBX);
	}
	else
		_sub_r32_r32(REG_EBP, REG_ECX);

	/* ASG: on the 68k, the shift count is mod 64; on the x86, the */
	/* shift count is mod 32; we need to check for shifts of 32-63 */
	/* and produce zero */
	_test_r32_imm(REG_ECX, 0x20);
	_jcc_near_link(COND_NZ, &link2);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link3);

_resolve_link(&link2);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r32_cl(REG_EAX);
	_mov_r8_imm(REG_CL, 16);
	_rcl_r32_cl(REG_EAX);
	_rcl_r32_cl(REG_EAX);

	DRC_CXFLAG_COND_C();

	_mov_m32abs_r32(&DY, REG_EAX);

	_jmp_near_link(&link4);

_resolve_link(&link1);
	_mov_r16_m16abs(REG_DX, &FLAG_X);
	_mov_m16abs_r16(&FLAG_C, REG_DX);

_resolve_link(&link3);
_resolve_link(&link4);
	DRC_NFLAG_32();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
}


M68KMAKE_OP(roxl, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r8_imm(REG_CL, 1);
	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_rcl_r16_cl(REG_AX);

	DRC_CXFLAG_COND_C();
	DRC_NFLAG_16();
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(rtd, 32, ., .)
{
	if (CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		sint16 offset = MAKE_INT_16(OPER_I_16());

		m68kdrc_pull_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		_add_m32abs_imm(&REG68K_A[7], offset);

		m68kdrc_jump(drc);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(rte, 32, ., .)
{
	link_info link1;

	m68kdrc_rte_callback();		   /* auto-disable (see m68kcpu.h) */
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);
	if (CPU_TYPE_IS_000(CPU_TYPE))
	{
		m68kdrc_pull_16();
		_push_r32(REG_EAX);

		m68kdrc_pull_32();
		m68kdrc_jump(drc);

		_pop_r32(REG_EAX);
		m68kdrc_set_sr(drc);

		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);
	}

	else if (CPU_TYPE_IS_010(CPU_TYPE))
	{
		link_info link2;
		link_info link3;

		_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
		_add_r32_imm(REG_EAX, 6);

		_push_r32(REG_EAX);
		m68kdrc_read_16();

		_shr_r32_imm(REG_EAX, 12);
		_jcc_near_link(COND_Z, &link2);

		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);
		/* Not handling bus fault (9) */
		m68kdrc_exception_format_error();

		_jmp_near_link(&link3);

_resolve_link(&link2);
		m68kdrc_pull_16();
		_push_r32(REG_EAX);

		m68kdrc_pull_32();
		m68kdrc_fake_pull_16();	/* format word */

		m68kdrc_jump(drc);

		_pop_r32(REG_EAX);
		m68kdrc_set_sr(drc);

		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);

_resolve_link(&link3);
	}
	else	/* Otherwise it's 020 */
	{
		void *entry = drc->cache_top;

		link_info link2;
		link_info link3;
		link_info link4;
		link_info link5;

		_mov_r32_m32abs(REG_EAX, &REG68K_A[7]);
		_add_r32_imm(REG_EAX, 6);

		_push_r32(REG_EAX);
		m68kdrc_read_16();

		_shr_r32_imm(REG_EAX, 12);
		_jcc_near_link(COND_Z, &link2);	/* 0: Normal */

		_sub_r32_imm(REG_EAX, 1);
		_jcc_near_link(COND_Z, &link3);	/* 1: Throwaway */

		_sub_r32_imm(REG_EAX, 1);
		_jcc_near_link(COND_Z, &link4);	/* 2: Trap */

		/* Not handling long or short bus fault */
		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);
		m68kdrc_exception_format_error();

_resolve_link(&link2);
		/* Normal */
		m68kdrc_pull_16();
		_push_r32(REG_EAX);

		m68kdrc_pull_32();
		m68kdrc_fake_pull_16();	/* format word */

		m68kdrc_jump(drc);

		_pop_r32(REG_EAX);
		m68kdrc_set_sr(drc);

		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);

		_jmp_near_link(&link5);

_resolve_link(&link3);
		/* Throwaway */
		m68kdrc_pull_16();
		m68kdrc_fake_pull_32();	/* program counter */
		m68kdrc_fake_pull_16();	/* format word */

		m68kdrc_set_sr_noint(drc);

		_jmp(entry);

_resolve_link(&link4);
		/* Trap */
		m68kdrc_pull_16();
		_push_r32(REG_EAX);

		m68kdrc_pull_32();
		m68kdrc_fake_pull_16();	/* format word */
		m68kdrc_fake_pull_32();	/* address */

		m68kdrc_jump(drc);

		_pop_r32(REG_EAX);
		m68kdrc_set_sr(drc);

		_mov_m32abs_imm(&CPU_INSTR_MODE, INSTRUCTION_YES);
		_mov_m32abs_imm(&CPU_RUN_MODE, RUN_MODE_NORMAL);

_resolve_link(&link5);
	}
}


M68KMAKE_OP(rtm, 32, ., .)
{
	if (CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
#if 0
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG68K_PC - 2), REG68K_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG68K_PC - 2))));
#else
		m68kdrc_recompile_flag = RECOMPILE_UNIMPLEMENTED;
#endif
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(rtr, 32, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_pull_16();

	m68kdrc_set_ccr(drc);

	m68kdrc_pull_32();

	m68kdrc_jump(drc);
}


M68KMAKE_OP(rts, 32, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */

	m68kdrc_pull_32();

	m68kdrc_jump(drc);
}


M68KMAKE_OP(sbcd, 8, rr, .)
{
	link_info link1;
	link_info link2;

	_mov_r8_m8abs(REG_CL, &DY);
	_mov_r8_r8(REG_CH, REG_CL);
	_and_r32_imm(REG_ECX, 0xf00f);

	_mov_r8_m8abs(REG_AL, &DX);
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

	_movzx_r32_r8(REG_EBX, REG_AH);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_BE, &link1);

	_sub_r32_imm(REG_EAX, 0x105);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_A, REG_BL);

	_shl_r32_imm(REG_EBX, 7);
	_mov_m8abs_r8(&FLAG_N, REG_BL);		/* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */

	_shl_r32_imm(REG_EBX, 1);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_test_r32_imm(REG_EBX, CFLAG_SET);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_imm(REG_EAX, 0x60);

_resolve_link(&link2);
	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(sbcd, 8, mm, ax7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_AY_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

	_movzx_r32_r8(REG_EBX, REG_AH);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_BE, &link1);

	_sub_r32_imm(REG_EAX, 0x105);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_A, REG_BL);

	_shl_r32_imm(REG_EBX, 7);
	_mov_m8abs_r8(&FLAG_N, REG_BL);		/* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */

	_shl_r32_imm(REG_EBX, 1);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_test_r32_imm(REG_EBX, CFLAG_SET);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_imm(REG_EAX, 0x60);

_resolve_link(&link2);
	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(sbcd, 8, mm, ay7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_A7_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

	_movzx_r32_r8(REG_EBX, REG_AH);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_BE, &link1);

	_sub_r32_imm(REG_EAX, 0x105);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_A, REG_BL);

	_shl_r32_imm(REG_EBX, 7);
	_mov_m8abs_r8(&FLAG_N, REG_BL);		/* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */

	_shl_r32_imm(REG_EBX, 1);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_test_r32_imm(REG_EBX, CFLAG_SET);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_imm(REG_EAX, 0x60);

_resolve_link(&link2);
	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(sbcd, 8, mm, axy7)
{
	link_info link1;
	link_info link2;

	DRC_OPER_A7_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

	_movzx_r32_r8(REG_EBX, REG_AH);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_BE, &link1);

	_sub_r32_imm(REG_EAX, 0x105);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_A, REG_BL);

	_shl_r32_imm(REG_EBX, 7);
	_mov_m8abs_r8(&FLAG_N, REG_BL);		/* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */

	_shl_r32_imm(REG_EBX, 1);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_test_r32_imm(REG_EBX, CFLAG_SET);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_imm(REG_EAX, 0x60);

_resolve_link(&link2);
	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(sbcd, 8, mm, .)
{
	link_info link1;
	link_info link2;

	DRC_OPER_AY_PD_8();
	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r8_r8(REG_AH, REG_AL);
	_and_r32_imm(REG_EAX, 0xf00f);

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);	/* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

	_movzx_r32_r8(REG_EBX, REG_AH);
	_movzx_r32_r8(REG_EAX, REG_AL);

	_cmp_r32_imm(REG_EAX, 9);
	_jcc_near_link(COND_BE, &link1);

	_sub_r32_imm(REG_EAX, 0x105);

_resolve_link(&link1);
	_add_r32_r32(REG_EAX, REG_EBX);

	_cmp_r32_imm(REG_EAX, 0x99);
	_setcc_r8(COND_A, REG_BL);

	_shl_r32_imm(REG_EBX, 7);
	_mov_m8abs_r8(&FLAG_N, REG_BL);		/* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */

	_shl_r32_imm(REG_EBX, 1);
	_mov_m16abs_r16(&FLAG_C, REG_BX);
	_mov_m16abs_r16(&FLAG_X, REG_BX);

	_test_r32_imm(REG_EBX, CFLAG_SET);
	_jcc_near_link(COND_Z, &link2);

	_sub_r32_imm(REG_EAX, 0x60);

_resolve_link(&link2);
	_mov_r8_m8abs(REG_BL, &FLAG_Z);
	_or_r32_r32(REG_EBX, REG_EAX);
	_mov_m8abs_r8(&FLAG_Z, REG_BL);

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(st, 8, ., d)
{
	_mov_m8abs_imm(&DY, 0xff);
}


M68KMAKE_OP(st, 8, ., .)
{
	_push_imm(0xff);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	m68kdrc_write_8();
}


M68KMAKE_OP(sf, 8, ., d)
{
	_mov_m8abs_imm(&DY, 0);
}


M68KMAKE_OP(sf, 8, ., .)
{
	_push_imm(0);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	m68kdrc_write_8();
}


M68KMAKE_OP(scc, 8, ., d)
{
	link_info link1;

	M68KMAKE_CC;

	_mov_m8abs_imm(&DY, 0xff);
	DRC_USE_CYCLES(CYC_SCC_R_TRUE);

	_jmp_near_link(&link1);

_resolve_link(&link_make_cc);
	_mov_m8abs_imm(&DY, 0);

_resolve_link(&link1);
}


M68KMAKE_OP(scc, 8, ., .)
{
	link_info link1;

	M68KMAKE_CC;

	_push_imm(0xff);

	_jmp_near_link(&link1);

_resolve_link(&link_make_cc);
	_push_imm(0x00);

_resolve_link(&link1);
	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	m68kdrc_write_8();
}


M68KMAKE_OP(stop, 0, ., .)
{
	uint16 new_sr = OPER_I_16();
	link_info link1;

	_test_m8abs_imm(&FLAG_S, SFLAG_SET);
	_jcc_near_link(COND_NZ, &link1);

	m68kdrc_exception_privilege_violation();

_resolve_link(&link1);

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

	_or_m32abs_imm(&CPU_STOPPED, STOP_LEVEL_STOP);

	_mov_r32_imm(REG_EAX, new_sr);
	m68kdrc_set_sr(drc);

	DRC_USE_ALL_CYCLES();
}


M68KMAKE_OP(sub, 8, er, d)
{
	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DY);

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(sub, 8, er, .)
{
	M68KMAKE_GET_OPER_AY_8;
	_mov_r16_r16(REG_CX, REG_AX);

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DX);
	_mov_r16_r16(REG_AX, REG_BX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(sub, 16, er, d)
{
	_movzx_r32_m16abs(REG_ECX, &DY);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(sub, 16, er, a)
{
	_movzx_r32_m16abs(REG_ECX, &AY);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(sub, 16, er, .)
{
	M68KMAKE_GET_OPER_AY_16;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_movzx_r32_m16abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(sub, 32, er, d)
{
	_mov_r32_m32abs(REG_ECX, &DY);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(sub, 32, er, a)
{
	_mov_r32_m32abs(REG_ECX, &AY);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(sub, 32, er, .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_r32(REG_ECX, REG_EAX);

	_mov_r32_m32abs(REG_EBX, &DX);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(sub, 8, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r16_r16(REG_BX, REG_AX);

	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(sub, 16, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_movzx_r32_m16abs(REG_ECX, &DX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(sub, 32, re, .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_m32abs(REG_ECX, &DX);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(suba, 16, ., d)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_movsx_r32_m16abs(REG_ECX, &DY);
	_sub_r32_r32(REG_EAX, REG_ECX);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(suba, 16, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_movsx_r32_m16abs(REG_ECX, &AY);
	_sub_r32_r32(REG_EAX, REG_ECX);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(suba, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;
	_movsx_r32_r16(REG_EAX, REG_AX);
	_mov_r32_m32abs(REG_ECX, &AX);
	_sub_r32_r32(REG_ECX, REG_EAX);
	_mov_m32abs_r32(&AX, REG_ECX);
}


M68KMAKE_OP(suba, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_sub_r32_m32abs(REG_EAX, &DY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(suba, 32, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AX);
	_sub_r32_m32abs(REG_EAX, &AY);
	_mov_m32abs_r32(&AX, REG_EAX);
}


M68KMAKE_OP(suba, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;
	_mov_r32_m32abs(REG_ECX, &AX);
	_sub_r32_r32(REG_ECX, REG_EAX);
	_mov_m32abs_r32(&AX, REG_ECX);
}


M68KMAKE_OP(subi, 8, ., d)
{
	uint8 src = OPER_I_8();

	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DY);
	_mov_r16_r16(REG_AX, REG_BX);

	_mov_r16_imm(REG_CX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(subi, 8, ., .)
{
	uint8 src = OPER_I_8();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();
	_mov_r16_r16(REG_BX, REG_AX);

	_mov_r16_imm(REG_CX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subi, 16, ., d)
{
	uint16 src = OPER_I_16();

	_movzx_r32_m16abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(subi, 16, ., .)
{
	uint16 src = OPER_I_16();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(subi, 32, ., d)
{
	uint32 src = OPER_I_32();

	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(subi, 32, ., .)
{
	uint32 src = OPER_I_32();

	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, src);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(subq, 8, ., d)
{
	_xor_r32_r32(REG_EBX, REG_EBX);
	_mov_r8_m8abs(REG_BL, &DY);
	_mov_r16_r16(REG_AX, REG_BX);

	_mov_r16_imm(REG_CX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(subq, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();
	_mov_r16_r16(REG_BX, REG_AX);

	_mov_r16_imm(REG_CX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_8(drc);		/* break EBX, ECX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subq, 16, ., d)
{
	_movzx_r32_m16abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16abs_r16(&DY, REG_AX);
}


M68KMAKE_OP(subq, 16, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_sub_r32_imm(REG_EAX, (((REG68K_IR >> 9) - 1) & 7) + 1);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(subq, 16, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_16;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_16(drc);		/* break EBX, ECX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(subq, 32, ., d)
{
	_mov_r32_m32abs(REG_EBX, &DY);
	_mov_r32_r32(REG_EAX, REG_EBX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(subq, 32, ., a)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_sub_r32_imm(REG_EAX, (((REG68K_IR >> 9) - 1) & 7) + 1);
	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(subq, 32, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_32;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();
	_mov_r32_r32(REG_EBX, REG_EAX);

	_mov_r32_imm(REG_ECX, (((REG68K_IR >> 9) - 1) & 7) + 1);

	_sub_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_sub_32(drc);		/* break EBX, ECX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(subx, 8, rr, .)
{
	_xor_r32_r32(REG_ECX, REG_ECX);
	_mov_r8_m8abs(REG_CL, &DY);

	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8abs(REG_AL, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_8(drc);		/* break ECX and EBX */

	_mov_m8abs_r8(&DX, REG_AL);
}


M68KMAKE_OP(subx, 16, rr, .)
{
	_movzx_r32_m16abs(REG_ECX, &DY);

	_movzx_r32_m16abs(REG_EAX, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_16(drc);	/* break ECX and EBX */

	_mov_m16abs_r16(&DX, REG_AX);
}


M68KMAKE_OP(subx, 32, rr, .)
{
	_mov_r32_m32abs(REG_ECX, &DY);

	_mov_r32_m32abs(REG_EAX, &DX);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_32(drc);	/* break ECX and EBX */

	_mov_m32abs_r32(&DX, REG_EAX);
}


M68KMAKE_OP(subx, 8, mm, ax7)
{
	DRC_OPER_AY_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_8(drc);		/* break ECX and EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subx, 8, mm, ay7)
{
	DRC_OPER_A7_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_8(drc);		/* break ECX and EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subx, 8, mm, axy7)
{
	DRC_OPER_A7_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_A7_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_8(drc);		/* break ECX and EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subx, 8, mm, .)
{
	DRC_OPER_AY_PD_8();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_8();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_8(drc);		/* break ECX and EBX */

	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(subx, 16, mm, .)
{
	DRC_OPER_AY_PD_16();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_16();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_16();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_16(drc);	/* break ECX and EBX */

	_mov_m16bd_r16(REG_ESP, 4, REG_AX);
	m68kdrc_write_16();
}


M68KMAKE_OP(subx, 32, mm, .)
{
	DRC_OPER_AY_PD_32();
	_push_r32(REG_EAX);

	DRC_EA_AX_PD_32();
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_32();

	_mov_r32_m32bd(REG_ECX, REG_ESP, 4);

	DRC_XFLAG_AS_COND_C();			/* break EDX */
	_sbb_r32_r32(REG_EAX, REG_ECX);

	m68kdrc_vncxz_flag_subx_32(drc);	/* break ECX and EBX */

	_mov_m32bd_r32(REG_ESP, 4, REG_EAX);
	m68kdrc_write_32();
}


M68KMAKE_OP(swap, 32, ., .)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	_ror_r32_imm(REG_EAX, 16);
	m68kdrc_vncz_flag_move_32(drc);		/* break ECX */

	_mov_m32abs_r32(&DY, REG_EAX);
}


M68KMAKE_OP(tas, 8, ., d)
{
	_movzx_r32_m8abs(REG_EAX, &DY);

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	DRC_NFLAG_8();
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_or_r32_imm(REG_EAX, 0x80);
	_mov_m8abs_r8(&DY, REG_AL);
}


M68KMAKE_OP(tas, 8, ., .)
{
	_sub_r32_imm(REG_ESP, 4);

	M68KMAKE_GET_EA_AY_8;
	_push_r32(REG_EAX);

	_push_r32(REG_EAX);
	m68kdrc_read_8();

	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	DRC_NFLAG_8();
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	_mov_m8abs_imm(&FLAG_V, VFLAG_CLEAR);

	_or_r32_imm(REG_EAX, 0x80);
	_mov_m8bd_r8(REG_ESP, 4, REG_AL);
	m68kdrc_write_8();
}


M68KMAKE_OP(trap, 0, ., .)
{
	/* Trap#n stacks exception frame type 0 */
	m68kdrc_exception_trapN(REG68K_IR & 0xf);
}


M68KMAKE_OP(trapt, 0, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapt, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapt, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapf, 0, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		;
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapf, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		REG68K_PC += 2;
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapf, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
		REG68K_PC += 4;
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapcc, 0, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68KMAKE_CC;

		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */

_resolve_link(&link_make_cc);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapcc, 16, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68KMAKE_CC;

		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */

_resolve_link(&link_make_cc);
		REG68K_PC += 2;
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapcc, 32, ., .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68KMAKE_CC;

		m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */

_resolve_link(&link_make_cc);
		REG68K_PC += 4;
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(trapv, 0, ., .)
{
	DRC_COND_VS();

	m68kdrc_exception_trap(EXCEPTION_TRAPV);	/* HJB 990403 */

_resolve_link(&link_make_cc);
}


M68KMAKE_OP(tst, 8, ., d)
{
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r8_m8abs(REG_AL, &DY);

	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 8, ., .)
{
	M68KMAKE_GET_OPER_AY_8;

	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 8, ., pcdi)
{
	DRC_OPER_PCDI_8();

	DRC_NFLAG_8();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 8, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_PCIX_8();

		DRC_NFLAG_8();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 8, ., i)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_I_8();

		DRC_NFLAG_8();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);

	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 16, ., d)
{
	_xor_r32_r32(REG_EAX, REG_EAX);
	_mov_r16_m16abs(REG_AX, &DY);

	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 16, ., a)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		_movsx_r32_m16abs(REG_EAX, &AY);

		DRC_NFLAG_16();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 16, ., .)
{
	M68KMAKE_GET_OPER_AY_16;

	DRC_NFLAG_16();
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 16, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_PCDI_16();

		DRC_NFLAG_16();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 16, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_PCIX_16();

		DRC_NFLAG_16();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 16, ., i)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_I_16();

		DRC_NFLAG_16();
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 32, ., d)
{
	_mov_r32_m32abs(REG_EAX, &DY);

	DRC_NFLAG_32();		/* break ECX */
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 32, ., a)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		_mov_r32_m32abs(REG_EAX, &AY);

		DRC_NFLAG_32();		/* break ECX */
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 32, ., .)
{
	M68KMAKE_GET_OPER_AY_32;

	DRC_NFLAG_32();		/* break ECX */
	_mov_m32abs_r32(&FLAG_Z, REG_EAX);
	_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
	_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
}


M68KMAKE_OP(tst, 32, ., pcdi)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_PCDI_32();

		DRC_NFLAG_32();		/* break ECX */
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 32, ., pcix)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_PCIX_32();

		DRC_NFLAG_32();		/* break ECX */
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(tst, 32, ., i)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		DRC_OPER_I_32();

		DRC_NFLAG_32();		/* break ECX */
		_mov_m32abs_r32(&FLAG_Z, REG_EAX);
		_mov_m32abs_imm(&FLAG_V, VFLAG_CLEAR);
		_mov_m16abs_imm(&FLAG_C, CFLAG_CLEAR);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(unlk, 32, ., a7)
{
	_push_m32abs(&REG68K_A[7]);
	m68kdrc_read_32();

	_mov_m32abs_r32(&REG68K_A[7], REG_EAX);
}


M68KMAKE_OP(unlk, 32, ., .)
{
	_mov_r32_m32abs(REG_EAX, &AY);
	_mov_m32abs_r32(&REG68K_A[7], REG_EAX);

	m68kdrc_pull_32();

	_mov_m32abs_r32(&AY, REG_EAX);
}


M68KMAKE_OP(unpk, 16, rr, .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: DX and DY are reversed in Motorola's docs */
		uint16 offset = OPER_I_16();

		_mov_r16_m16abs(REG_AX, &DY);
		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EBX, 0x000f);
		_shl_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0x0f00);
		_or_r32_r32(REG_EAX, REG_EBX);

		_add_r32_imm(REG_EAX, offset);

		_mov_m16abs_r16(&DX, REG_AX);
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(unpk, 16, mm, ax7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_OPER_AY_PD_8();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EBX, 0x000f);
		_shl_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0x0f00);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_push_r32(REG_EAX);

		_shr_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();
		m68kdrc_write_8();

		DRC_EA_A7_PD_8();
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(unpk, 16, mm, ay7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_OPER_A7_PD_8();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EBX, 0x000f);
		_shl_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0x0f00);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_push_r32(REG_EAX);

		_shr_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_AX_PD_8();
		m68kdrc_write_8();

		DRC_EA_AX_PD_8();
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(unpk, 16, mm, axy7)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint16 offset;

		DRC_OPER_A7_PD_8();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EBX, 0x000f);
		_shl_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0x0f00);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_push_r32(REG_EAX);

		_shr_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_A7_PD_8();
		m68kdrc_write_8();

		DRC_EA_A7_PD_8();
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}


M68KMAKE_OP(unpk, 16, mm, .)
{
	if (CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint16 offset;

		DRC_OPER_AY_PD_8();

		_mov_r32_r32(REG_EBX, REG_EAX);
		_and_r32_imm(REG_EBX, 0x000f);
		_shl_r32_imm(REG_EBX, 4);
		_and_r32_imm(REG_EBX, 0x0f00);
		_or_r32_r32(REG_EAX, REG_EBX);

		offset = OPER_I_16();
		_add_r32_imm(REG_EAX, offset);

		_push_r32(REG_EAX);

		_shr_r32_imm(REG_EAX, 8);
		_push_r32(REG_EAX);

		DRC_EA_AX_PD_8();
		m68kdrc_write_8();

		DRC_EA_AX_PD_8();
		m68kdrc_write_8();
	}
	else
		m68kdrc_exception_illegal();
}



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_UPDATE_VNCZ_BODY

M68KMAKE_UPDATE_VNCZ(and, 8, er, d)
M68KMAKE_UPDATE_VNCZ(and, 8, er, .)
M68KMAKE_UPDATE_VNCZ(and, 16, er, d)
M68KMAKE_UPDATE_VNCZ(and, 16, er, .)
M68KMAKE_UPDATE_VNCZ(and, 32, er, d)
M68KMAKE_UPDATE_VNCZ(and, 32, er, .)
M68KMAKE_UPDATE_VNCZ(and, 8, re, .)
M68KMAKE_UPDATE_VNCZ(and, 16, re, .)
M68KMAKE_UPDATE_VNCZ(and, 32, re, .)
M68KMAKE_UPDATE_VNCZ(andi, 8, ., d)
M68KMAKE_UPDATE_VNCZ(andi, 8, ., .)
M68KMAKE_UPDATE_VNCZ(andi, 16, ., d)
M68KMAKE_UPDATE_VNCZ(andi, 16, ., .)
M68KMAKE_UPDATE_VNCZ(andi, 32, ., d)
M68KMAKE_UPDATE_VNCZ(andi, 32, ., .)
M68KMAKE_UPDATE_VNCZ(cmp, 8, ., d)
M68KMAKE_UPDATE_VNCZ(cmp, 8, ., .)
M68KMAKE_UPDATE_VNCZ(cmp, 16, ., d)
M68KMAKE_UPDATE_VNCZ(cmp, 16, ., a)
M68KMAKE_UPDATE_VNCZ(cmp, 16, ., .)
M68KMAKE_UPDATE_VNCZ(cmp, 32, ., d)
M68KMAKE_UPDATE_VNCZ(cmp, 32, ., a)
M68KMAKE_UPDATE_VNCZ(cmp, 32, ., .)
M68KMAKE_UPDATE_VNCZ(cmpa, 16, ., d)
M68KMAKE_UPDATE_VNCZ(cmpa, 16, ., a)
M68KMAKE_UPDATE_VNCZ(cmpa, 16, ., .)
M68KMAKE_UPDATE_VNCZ(cmpa, 32, ., d)
M68KMAKE_UPDATE_VNCZ(cmpa, 32, ., a)
M68KMAKE_UPDATE_VNCZ(cmpa, 32, ., .)
M68KMAKE_UPDATE_VNCZ(cmpi, 8, ., d)
M68KMAKE_UPDATE_VNCZ(cmpi, 8, ., .)
M68KMAKE_UPDATE_VNCZ(cmpi, 8, ., pcdi)
M68KMAKE_UPDATE_VNCZ(cmpi, 8, ., pcix)
M68KMAKE_UPDATE_VNCZ(cmpi, 16, ., d)
M68KMAKE_UPDATE_VNCZ(cmpi, 16, ., .)
M68KMAKE_UPDATE_VNCZ(cmpi, 16, ., pcdi)
M68KMAKE_UPDATE_VNCZ(cmpi, 16, ., pcix)
M68KMAKE_UPDATE_VNCZ(cmpi, 32, ., d)
M68KMAKE_UPDATE_VNCZ(cmpi, 32, ., .)
M68KMAKE_UPDATE_VNCZ(cmpi, 32, ., pcdi)
M68KMAKE_UPDATE_VNCZ(cmpi, 32, ., pcix)
M68KMAKE_UPDATE_VNCZ(cmpm, 8, ., ax7)
M68KMAKE_UPDATE_VNCZ(cmpm, 8, ., ay7)
M68KMAKE_UPDATE_VNCZ(cmpm, 8, ., axy7)
M68KMAKE_UPDATE_VNCZ(cmpm, 8, ., .)
M68KMAKE_UPDATE_VNCZ(cmpm, 16, ., .)
M68KMAKE_UPDATE_VNCZ(cmpm, 32, ., .)
M68KMAKE_UPDATE_VNCZ(divs, 16, ., d)
M68KMAKE_UPDATE_VNCZ(divs, 16, ., .)
M68KMAKE_UPDATE_VNCZ(divu, 16, ., d)
M68KMAKE_UPDATE_VNCZ(divu, 16, ., .)
M68KMAKE_UPDATE_VNCZ(divl, 32, ., d)
M68KMAKE_UPDATE_VNCZ(divl, 32, ., .)
M68KMAKE_UPDATE_VNCZ(eor, 8, ., d)
M68KMAKE_UPDATE_VNCZ(eor, 8, ., .)
M68KMAKE_UPDATE_VNCZ(eor, 16, ., d)
M68KMAKE_UPDATE_VNCZ(eor, 16, ., .)
M68KMAKE_UPDATE_VNCZ(eor, 32, ., d)
M68KMAKE_UPDATE_VNCZ(eor, 32, ., .)
M68KMAKE_UPDATE_VNCZ(eori, 8, ., d)
M68KMAKE_UPDATE_VNCZ(eori, 8, ., .)
M68KMAKE_UPDATE_VNCZ(eori, 16, ., d)
M68KMAKE_UPDATE_VNCZ(eori, 16, ., .)
M68KMAKE_UPDATE_VNCZ(eori, 32, ., d)
M68KMAKE_UPDATE_VNCZ(eori, 32, ., .)
M68KMAKE_UPDATE_VNCZ(ext, 16, ., .)
M68KMAKE_UPDATE_VNCZ(ext, 32, ., .)
M68KMAKE_UPDATE_VNCZ(extb, 32, ., .)
M68KMAKE_UPDATE_VNCZ(move, 8, d, d)
M68KMAKE_UPDATE_VNCZ(move, 8, d, .)
M68KMAKE_UPDATE_VNCZ(move, 8, ai, d)
M68KMAKE_UPDATE_VNCZ(move, 8, ai, .)
M68KMAKE_UPDATE_VNCZ(move, 8, pi7, d)
M68KMAKE_UPDATE_VNCZ(move, 8, pi, d)
M68KMAKE_UPDATE_VNCZ(move, 8, pi7, .)
M68KMAKE_UPDATE_VNCZ(move, 8, pi, .)
M68KMAKE_UPDATE_VNCZ(move, 8, pd7, d)
M68KMAKE_UPDATE_VNCZ(move, 8, pd, d)
M68KMAKE_UPDATE_VNCZ(move, 8, pd7, .)
M68KMAKE_UPDATE_VNCZ(move, 8, pd, .)
M68KMAKE_UPDATE_VNCZ(move, 8, di, d)
M68KMAKE_UPDATE_VNCZ(move, 8, di, .)
M68KMAKE_UPDATE_VNCZ(move, 8, ix, d)
M68KMAKE_UPDATE_VNCZ(move, 8, ix, .)
M68KMAKE_UPDATE_VNCZ(move, 8, aw, d)
M68KMAKE_UPDATE_VNCZ(move, 8, aw, .)
M68KMAKE_UPDATE_VNCZ(move, 8, al, d)
M68KMAKE_UPDATE_VNCZ(move, 8, al, .)
M68KMAKE_UPDATE_VNCZ(move, 16, d, d)
M68KMAKE_UPDATE_VNCZ(move, 16, d, a)
M68KMAKE_UPDATE_VNCZ(move, 16, d, .)
M68KMAKE_UPDATE_VNCZ(move, 16, ai, d)
M68KMAKE_UPDATE_VNCZ(move, 16, ai, a)
M68KMAKE_UPDATE_VNCZ(move, 16, ai, .)
M68KMAKE_UPDATE_VNCZ(move, 16, pi, d)
M68KMAKE_UPDATE_VNCZ(move, 16, pi, a)
M68KMAKE_UPDATE_VNCZ(move, 16, pi, .)
M68KMAKE_UPDATE_VNCZ(move, 16, pd, d)
M68KMAKE_UPDATE_VNCZ(move, 16, pd, a)
M68KMAKE_UPDATE_VNCZ(move, 16, pd, .)
M68KMAKE_UPDATE_VNCZ(move, 16, di, d)
M68KMAKE_UPDATE_VNCZ(move, 16, di, a)
M68KMAKE_UPDATE_VNCZ(move, 16, di, .)
M68KMAKE_UPDATE_VNCZ(move, 16, ix, d)
M68KMAKE_UPDATE_VNCZ(move, 16, ix, a)
M68KMAKE_UPDATE_VNCZ(move, 16, ix, .)
M68KMAKE_UPDATE_VNCZ(move, 16, aw, d)
M68KMAKE_UPDATE_VNCZ(move, 16, aw, a)
M68KMAKE_UPDATE_VNCZ(move, 16, aw, .)
M68KMAKE_UPDATE_VNCZ(move, 16, al, d)
M68KMAKE_UPDATE_VNCZ(move, 16, al, a)
M68KMAKE_UPDATE_VNCZ(move, 16, al, .)
M68KMAKE_UPDATE_VNCZ(move, 32, d, d)
M68KMAKE_UPDATE_VNCZ(move, 32, d, a)
M68KMAKE_UPDATE_VNCZ(move, 32, d, .)
M68KMAKE_UPDATE_VNCZ(move, 32, ai, d)
M68KMAKE_UPDATE_VNCZ(move, 32, ai, a)
M68KMAKE_UPDATE_VNCZ(move, 32, ai, .)
M68KMAKE_UPDATE_VNCZ(move, 32, pi, d)
M68KMAKE_UPDATE_VNCZ(move, 32, pi, a)
M68KMAKE_UPDATE_VNCZ(move, 32, pi, .)
M68KMAKE_UPDATE_VNCZ(move, 32, pd, d)
M68KMAKE_UPDATE_VNCZ(move, 32, pd, a)
M68KMAKE_UPDATE_VNCZ(move, 32, pd, .)
M68KMAKE_UPDATE_VNCZ(move, 32, di, d)
M68KMAKE_UPDATE_VNCZ(move, 32, di, a)
M68KMAKE_UPDATE_VNCZ(move, 32, di, .)
M68KMAKE_UPDATE_VNCZ(move, 32, ix, d)
M68KMAKE_UPDATE_VNCZ(move, 32, ix, a)
M68KMAKE_UPDATE_VNCZ(move, 32, ix, .)
M68KMAKE_UPDATE_VNCZ(move, 32, aw, d)
M68KMAKE_UPDATE_VNCZ(move, 32, aw, a)
M68KMAKE_UPDATE_VNCZ(move, 32, aw, .)
M68KMAKE_UPDATE_VNCZ(move, 32, al, d)
M68KMAKE_UPDATE_VNCZ(move, 32, al, a)
M68KMAKE_UPDATE_VNCZ(move, 32, al, .)
M68KMAKE_UPDATE_VNCZ(moveq, 32, ., .)
M68KMAKE_UPDATE_VNCZ(muls, 16, ., d)
M68KMAKE_UPDATE_VNCZ(muls, 16, ., .)
M68KMAKE_UPDATE_VNCZ(mulu, 16, ., d)
M68KMAKE_UPDATE_VNCZ(mulu, 16, ., .)
M68KMAKE_UPDATE_VNCZ(not, 8, ., d)
M68KMAKE_UPDATE_VNCZ(not, 8, ., .)
M68KMAKE_UPDATE_VNCZ(not, 16, ., d)
M68KMAKE_UPDATE_VNCZ(not, 16, ., .)
M68KMAKE_UPDATE_VNCZ(not, 32, ., d)
M68KMAKE_UPDATE_VNCZ(not, 32, ., .)
M68KMAKE_UPDATE_VNCZ(or, 8, er, d)
M68KMAKE_UPDATE_VNCZ(or, 8, er, .)
M68KMAKE_UPDATE_VNCZ(or, 16, er, d)
M68KMAKE_UPDATE_VNCZ(or, 16, er, .)
M68KMAKE_UPDATE_VNCZ(or, 32, er, d)
M68KMAKE_UPDATE_VNCZ(or, 32, er, .)
M68KMAKE_UPDATE_VNCZ(or, 8, re, .)
M68KMAKE_UPDATE_VNCZ(or, 16, re, .)
M68KMAKE_UPDATE_VNCZ(or, 32, re, .)
M68KMAKE_UPDATE_VNCZ(ori, 8, ., d)
M68KMAKE_UPDATE_VNCZ(ori, 8, ., .)
M68KMAKE_UPDATE_VNCZ(ori, 16, ., d)
M68KMAKE_UPDATE_VNCZ(ori, 16, ., .)
M68KMAKE_UPDATE_VNCZ(ori, 32, ., d)
M68KMAKE_UPDATE_VNCZ(ori, 32, ., .)
M68KMAKE_UPDATE_VNCZ(swap, 32, ., .)


XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_END
