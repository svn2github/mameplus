#define NUM_CPU_TYPES 3

int CYC_DBCC_F_NOEXP[NUM_CPU_TYPES] = { -2,  0,  0};
int CYC_DBCC_F_EXP[NUM_CPU_TYPES] =   {  2,  6,  4};
int CYC_BCC_NOTAKE_B[NUM_CPU_TYPES] = { -2, -4, -2};
int CYC_BCC_NOTAKE_W[NUM_CPU_TYPES] = {  2,  0,  0};
int CYC_MOVEM_W[NUM_CPU_TYPES] =      {  4,  4,  4};
int CYC_MOVEM_L[NUM_CPU_TYPES] =      {  8,  8,  4};
int CYC_SHIFT[NUM_CPU_TYPES] =        {  1,  1,  0};
int CYC_SCC_R_TRUE[NUM_CPU_TYPES] =   {  2,  0,  0};

unsigned char m68ki_exception_cycle_table[NUM_CPU_TYPES][256] =
{
	{ /* 000 */
		  4, /*  0: Reset - Initial Stack Pointer                      */
		  4, /*  1: Reset - Initial Program Counter                    */
		 50, /*  2: Bus Error                             (unemulated) */
		 50, /*  3: Address Error                         (unemulated) */
		 34, /*  4: Illegal Instruction                                */
		 38, /*  5: Divide by Zero -- ASG: changed from 42             */
		 40, /*  6: CHK -- ASG: chanaged from 44                       */
		 34, /*  7: TRAPV                                              */
		 34, /*  8: Privilege Violation                                */
		 34, /*  9: Trace                                              */
		  4, /* 10: 1010                                               */
		  4, /* 11: 1111                                               */
		  4, /* 12: RESERVED                                           */
		  4, /* 13: Coprocessor Protocol Violation        (unemulated) */
		  4, /* 14: Format Error                                       */
		 44, /* 15: Uninitialized Interrupt                            */
		  4, /* 16: RESERVED                                           */
		  4, /* 17: RESERVED                                           */
		  4, /* 18: RESERVED                                           */
		  4, /* 19: RESERVED                                           */
		  4, /* 20: RESERVED                                           */
		  4, /* 21: RESERVED                                           */
		  4, /* 22: RESERVED                                           */
		  4, /* 23: RESERVED                                           */
		 44, /* 24: Spurious Interrupt                                 */
		 44, /* 25: Level 1 Interrupt Autovector                       */
		 44, /* 26: Level 2 Interrupt Autovector                       */
		 44, /* 27: Level 3 Interrupt Autovector                       */
		 44, /* 28: Level 4 Interrupt Autovector                       */
		 44, /* 29: Level 5 Interrupt Autovector                       */
		 44, /* 30: Level 6 Interrupt Autovector                       */
		 44, /* 31: Level 7 Interrupt Autovector                       */
		 34, /* 32: TRAP #0 -- ASG: chanaged from 38                   */
		 34, /* 33: TRAP #1                                            */
		 34, /* 34: TRAP #2                                            */
		 34, /* 35: TRAP #3                                            */
		 34, /* 36: TRAP #4                                            */
		 34, /* 37: TRAP #5                                            */
		 34, /* 38: TRAP #6                                            */
		 34, /* 39: TRAP #7                                            */
		 34, /* 40: TRAP #8                                            */
		 34, /* 41: TRAP #9                                            */
		 34, /* 42: TRAP #10                                           */
		 34, /* 43: TRAP #11                                           */
		 34, /* 44: TRAP #12                                           */
		 34, /* 45: TRAP #13                                           */
		 34, /* 46: TRAP #14                                           */
		 34, /* 47: TRAP #15                                           */
		  4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
		  4, /* 49: FP Inexact Result                     (unemulated) */
		  4, /* 50: FP Divide by Zero                     (unemulated) */
		  4, /* 51: FP Underflow                          (unemulated) */
		  4, /* 52: FP Operand Error                      (unemulated) */
		  4, /* 53: FP Overflow                           (unemulated) */
		  4, /* 54: FP Signaling NAN                      (unemulated) */
		  4, /* 55: FP Unimplemented Data Type            (unemulated) */
		  4, /* 56: MMU Configuration Error               (unemulated) */
		  4, /* 57: MMU Illegal Operation Error           (unemulated) */
		  4, /* 58: MMU Access Level Violation Error      (unemulated) */
		  4, /* 59: RESERVED                                           */
		  4, /* 60: RESERVED                                           */
		  4, /* 61: RESERVED                                           */
		  4, /* 62: RESERVED                                           */
		  4, /* 63: RESERVED                                           */
		     /* 64-255: User Defined                                   */
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 010 */
		  4, /*  0: Reset - Initial Stack Pointer                      */
		  4, /*  1: Reset - Initial Program Counter                    */
		126, /*  2: Bus Error                             (unemulated) */
		126, /*  3: Address Error                         (unemulated) */
		 38, /*  4: Illegal Instruction                                */
		 44, /*  5: Divide by Zero                                     */
		 44, /*  6: CHK                                                */
		 34, /*  7: TRAPV                                              */
		 38, /*  8: Privilege Violation                                */
		 38, /*  9: Trace                                              */
		  4, /* 10: 1010                                               */
		  4, /* 11: 1111                                               */
		  4, /* 12: RESERVED                                           */
		  4, /* 13: Coprocessor Protocol Violation        (unemulated) */
		  4, /* 14: Format Error                                       */
		 44, /* 15: Uninitialized Interrupt                            */
		  4, /* 16: RESERVED                                           */
		  4, /* 17: RESERVED                                           */
		  4, /* 18: RESERVED                                           */
		  4, /* 19: RESERVED                                           */
		  4, /* 20: RESERVED                                           */
		  4, /* 21: RESERVED                                           */
		  4, /* 22: RESERVED                                           */
		  4, /* 23: RESERVED                                           */
		 46, /* 24: Spurious Interrupt                                 */
		 46, /* 25: Level 1 Interrupt Autovector                       */
		 46, /* 26: Level 2 Interrupt Autovector                       */
		 46, /* 27: Level 3 Interrupt Autovector                       */
		 46, /* 28: Level 4 Interrupt Autovector                       */
		 46, /* 29: Level 5 Interrupt Autovector                       */
		 46, /* 30: Level 6 Interrupt Autovector                       */
		 46, /* 31: Level 7 Interrupt Autovector                       */
		 38, /* 32: TRAP #0                                            */
		 38, /* 33: TRAP #1                                            */
		 38, /* 34: TRAP #2                                            */
		 38, /* 35: TRAP #3                                            */
		 38, /* 36: TRAP #4                                            */
		 38, /* 37: TRAP #5                                            */
		 38, /* 38: TRAP #6                                            */
		 38, /* 39: TRAP #7                                            */
		 38, /* 40: TRAP #8                                            */
		 38, /* 41: TRAP #9                                            */
		 38, /* 42: TRAP #10                                           */
		 38, /* 43: TRAP #11                                           */
		 38, /* 44: TRAP #12                                           */
		 38, /* 45: TRAP #13                                           */
		 38, /* 46: TRAP #14                                           */
		 38, /* 47: TRAP #15                                           */
		  4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
		  4, /* 49: FP Inexact Result                     (unemulated) */
		  4, /* 50: FP Divide by Zero                     (unemulated) */
		  4, /* 51: FP Underflow                          (unemulated) */
		  4, /* 52: FP Operand Error                      (unemulated) */
		  4, /* 53: FP Overflow                           (unemulated) */
		  4, /* 54: FP Signaling NAN                      (unemulated) */
		  4, /* 55: FP Unimplemented Data Type            (unemulated) */
		  4, /* 56: MMU Configuration Error               (unemulated) */
		  4, /* 57: MMU Illegal Operation Error           (unemulated) */
		  4, /* 58: MMU Access Level Violation Error      (unemulated) */
		  4, /* 59: RESERVED                                           */
		  4, /* 60: RESERVED                                           */
		  4, /* 61: RESERVED                                           */
		  4, /* 62: RESERVED                                           */
		  4, /* 63: RESERVED                                           */
		     /* 64-255: User Defined                                   */
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 020 */
		  4, /*  0: Reset - Initial Stack Pointer                      */
		  4, /*  1: Reset - Initial Program Counter                    */
		 50, /*  2: Bus Error                             (unemulated) */
		 50, /*  3: Address Error                         (unemulated) */
		 20, /*  4: Illegal Instruction                                */
		 38, /*  5: Divide by Zero                                     */
		 40, /*  6: CHK                                                */
		 20, /*  7: TRAPV                                              */
		 34, /*  8: Privilege Violation                                */
		 25, /*  9: Trace                                              */
		 20, /* 10: 1010                                               */
		 20, /* 11: 1111                                               */
		  4, /* 12: RESERVED                                           */
		  4, /* 13: Coprocessor Protocol Violation        (unemulated) */
		  4, /* 14: Format Error                                       */
		 30, /* 15: Uninitialized Interrupt                            */
		  4, /* 16: RESERVED                                           */
		  4, /* 17: RESERVED                                           */
		  4, /* 18: RESERVED                                           */
		  4, /* 19: RESERVED                                           */
		  4, /* 20: RESERVED                                           */
		  4, /* 21: RESERVED                                           */
		  4, /* 22: RESERVED                                           */
		  4, /* 23: RESERVED                                           */
		 30, /* 24: Spurious Interrupt                                 */
		 30, /* 25: Level 1 Interrupt Autovector                       */
		 30, /* 26: Level 2 Interrupt Autovector                       */
		 30, /* 27: Level 3 Interrupt Autovector                       */
		 30, /* 28: Level 4 Interrupt Autovector                       */
		 30, /* 29: Level 5 Interrupt Autovector                       */
		 30, /* 30: Level 6 Interrupt Autovector                       */
		 30, /* 31: Level 7 Interrupt Autovector                       */
		 20, /* 32: TRAP #0                                            */
		 20, /* 33: TRAP #1                                            */
		 20, /* 34: TRAP #2                                            */
		 20, /* 35: TRAP #3                                            */
		 20, /* 36: TRAP #4                                            */
		 20, /* 37: TRAP #5                                            */
		 20, /* 38: TRAP #6                                            */
		 20, /* 39: TRAP #7                                            */
		 20, /* 40: TRAP #8                                            */
		 20, /* 41: TRAP #9                                            */
		 20, /* 42: TRAP #10                                           */
		 20, /* 43: TRAP #11                                           */
		 20, /* 44: TRAP #12                                           */
		 20, /* 45: TRAP #13                                           */
		 20, /* 46: TRAP #14                                           */
		 20, /* 47: TRAP #15                                           */
		  4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
		  4, /* 49: FP Inexact Result                     (unemulated) */
		  4, /* 50: FP Divide by Zero                     (unemulated) */
		  4, /* 51: FP Underflow                          (unemulated) */
		  4, /* 52: FP Operand Error                      (unemulated) */
		  4, /* 53: FP Overflow                           (unemulated) */
		  4, /* 54: FP Signaling NAN                      (unemulated) */
		  4, /* 55: FP Unimplemented Data Type            (unemulated) */
		  4, /* 56: MMU Configuration Error               (unemulated) */
		  4, /* 57: MMU Illegal Operation Error           (unemulated) */
		  4, /* 58: MMU Access Level Violation Error      (unemulated) */
		  4, /* 59: RESERVED                                           */
		  4, /* 60: RESERVED                                           */
		  4, /* 61: RESERVED                                           */
		  4, /* 62: RESERVED                                           */
		  4, /* 63: RESERVED                                           */
		     /* 64-255: User Defined                                   */
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	}
};

/* This is used to generate the opcode handler jump table */
typedef struct
{
	unsigned int  mask;                  /* mask on opcode */
	unsigned int  match;                 /* what to match after masking */
	unsigned int  cycles[NUM_CPU_TYPES]; /* cycles each cpu type takes */
} opcode_handler_struct;


/* Opcode handler table */
static opcode_handler_struct m68k_opcode_handler_table[] =
{
/*	  mask    match    000  010  020 */
	{ 0xf000, 0xa000, {  4,   4,   4}},	// 1010
	{ 0xf000, 0xf000, {  4,   4,   4}},	// 1111
	{ 0xf100, 0x7000, {  4,   4,   2}},	// moveq_32
	{ 0xf180, 0xf080, {  0,   0,   4}},	// cpbcc_32
	{ 0xf1c0, 0xf000, {  0,   0,   4}},	// cpgen_32
	{ 0xf1c0, 0xf040, {  0,   0,   4}},	// cpscc_32
	{ 0xff00, 0x6000, { 10,  10,  10}},	// bra_8
	{ 0xff00, 0x6100, { 18,  18,   7}},	// bsr_8
	{ 0xff00, 0x6200, {  8,   8,   6}},	// bhi_8
	{ 0xff00, 0x6300, {  8,   8,   6}},	// bls_8
	{ 0xff00, 0x6400, { 10,  10,   6}},	// bcc_8
	{ 0xff00, 0x6500, {  8,   8,   6}},	// bcs_8
	{ 0xff00, 0x6600, {  8,   8,   6}},	// bne_8
	{ 0xff00, 0x6700, {  8,   8,   6}},	// beq_8
	{ 0xff00, 0x6800, {  8,   8,   6}},	// bvc_8
	{ 0xff00, 0x6900, {  8,   8,   6}},	// bvs_8
	{ 0xff00, 0x6a00, {  8,   8,   6}},	// bpl_8
	{ 0xff00, 0x6b00, {  8,   8,   6}},	// bmi_8
	{ 0xff00, 0x6c00, {  8,   8,   6}},	// bge_8
	{ 0xff00, 0x6d00, {  8,   8,   6}},	// blt_8
	{ 0xff00, 0x6e00, {  8,   8,   6}},	// bgt_8
	{ 0xff00, 0x6f00, {  8,   8,   6}},	// ble_8
	{ 0xf1f8, 0x0100, {  6,   6,   4}},	// btst_32_r_d
	{ 0xf1f8, 0x0108, { 16,  16,  12}},	// movep_16_er
	{ 0xf1f8, 0x0110, {  8,   8,   8}},	// btst_8_r_ai
	{ 0xf1f8, 0x0118, {  8,   8,   8}},	// btst_8_r_pi
	{ 0xf1f8, 0x0120, { 10,  10,   9}},	// btst_8_r_pd
	{ 0xf1f8, 0x0128, { 12,  12,   9}},	// btst_8_r_di
	{ 0xf1f8, 0x0130, { 14,  14,  11}},	// btst_8_r_ix
	{ 0xf1f8, 0x0140, {  8,   8,   4}},	// bchg_32_r_d
	{ 0xf1f8, 0x0148, { 24,  24,  18}},	// movep_32_er
	{ 0xf1f8, 0x0150, { 12,  12,   8}},	// bchg_8_r_ai
	{ 0xf1f8, 0x0158, { 12,  12,   8}},	// bchg_8_r_pi
	{ 0xf1f8, 0x0160, { 14,  14,   9}},	// bchg_8_r_pd
	{ 0xf1f8, 0x0168, { 16,  16,   9}},	// bchg_8_r_di
	{ 0xf1f8, 0x0170, { 18,  18,  11}},	// bchg_8_r_ix
	{ 0xf1f8, 0x0180, { 10,  10,   4}},	// bclr_32_r_d
	{ 0xf1f8, 0x0188, { 16,  16,  11}},	// movep_16_re
	{ 0xf1f8, 0x0190, { 12,  14,   8}},	// bclr_8_r_ai
	{ 0xf1f8, 0x0198, { 12,  14,   8}},	// bclr_8_r_pi
	{ 0xf1f8, 0x01a0, { 14,  16,   9}},	// bclr_8_r_pd
	{ 0xf1f8, 0x01a8, { 16,  18,   9}},	// bclr_8_r_di
	{ 0xf1f8, 0x01b0, { 18,  20,  11}},	// bclr_8_r_ix
	{ 0xf1f8, 0x01c0, {  8,   8,   4}},	// bset_32_r_d
	{ 0xf1f8, 0x01c8, { 24,  24,  17}},	// movep_32_re
	{ 0xf1f8, 0x01d0, { 12,  12,   8}},	// bset_8_r_ai
	{ 0xf1f8, 0x01d8, { 12,  12,   8}},	// bset_8_r_pi
	{ 0xf1f8, 0x01e0, { 14,  14,   9}},	// bset_8_r_pd
	{ 0xf1f8, 0x01e8, { 16,  16,   9}},	// bset_8_r_di
	{ 0xf1f8, 0x01f0, { 18,  18,  11}},	// bset_8_r_ix
	{ 0xf1f8, 0x1000, {  4,   4,   2}},	// move_8_d_d
	{ 0xf1f8, 0x1010, {  8,   8,   6}},	// move_8_d_ai
	{ 0xf1f8, 0x1018, {  8,   8,   6}},	// move_8_d_pi
	{ 0xf1f8, 0x1020, { 10,  10,   7}},	// move_8_d_pd
	{ 0xf1f8, 0x1028, { 12,  12,   7}},	// move_8_d_di
	{ 0xf1f8, 0x1030, { 14,  14,   9}},	// move_8_d_ix
	{ 0xf1f8, 0x1080, {  8,   8,   4}},	// move_8_ai_d
	{ 0xf1f8, 0x1090, { 12,  12,   8}},	// move_8_ai_ai
	{ 0xf1f8, 0x1098, { 12,  12,   8}},	// move_8_ai_pi
	{ 0xf1f8, 0x10a0, { 14,  14,   9}},	// move_8_ai_pd
	{ 0xf1f8, 0x10a8, { 16,  16,   9}},	// move_8_ai_di
	{ 0xf1f8, 0x10b0, { 18,  18,  11}},	// move_8_ai_ix
	{ 0xf1f8, 0x10c0, {  8,   8,   4}},	// move_8_pi_d
	{ 0xf1f8, 0x10d0, { 12,  12,   8}},	// move_8_pi_ai
	{ 0xf1f8, 0x10d8, { 12,  12,   8}},	// move_8_pi_pi
	{ 0xf1f8, 0x10e0, { 14,  14,   9}},	// move_8_pi_pd
	{ 0xf1f8, 0x10e8, { 16,  16,   9}},	// move_8_pi_di
	{ 0xf1f8, 0x10f0, { 18,  18,  11}},	// move_8_pi_ix
	{ 0xf1f8, 0x1100, {  8,   8,   5}},	// move_8_pd_d
	{ 0xf1f8, 0x1110, { 12,  12,   9}},	// move_8_pd_ai
	{ 0xf1f8, 0x1118, { 12,  12,   9}},	// move_8_pd_pi
	{ 0xf1f8, 0x1120, { 14,  14,  10}},	// move_8_pd_pd
	{ 0xf1f8, 0x1128, { 16,  16,  10}},	// move_8_pd_di
	{ 0xf1f8, 0x1130, { 18,  18,  12}},	// move_8_pd_ix
	{ 0xf1f8, 0x1140, { 12,  12,   5}},	// move_8_di_d
	{ 0xf1f8, 0x1150, { 16,  16,   9}},	// move_8_di_ai
	{ 0xf1f8, 0x1158, { 16,  16,   9}},	// move_8_di_pi
	{ 0xf1f8, 0x1160, { 18,  18,  10}},	// move_8_di_pd
	{ 0xf1f8, 0x1168, { 20,  20,  10}},	// move_8_di_di
	{ 0xf1f8, 0x1170, { 22,  22,  12}},	// move_8_di_ix
	{ 0xf1f8, 0x1180, { 14,  14,   7}},	// move_8_ix_d
	{ 0xf1f8, 0x1190, { 18,  18,  11}},	// move_8_ix_ai
	{ 0xf1f8, 0x1198, { 18,  18,  11}},	// move_8_ix_pi
	{ 0xf1f8, 0x11a0, { 20,  20,  12}},	// move_8_ix_pd
	{ 0xf1f8, 0x11a8, { 22,  22,  12}},	// move_8_ix_di
	{ 0xf1f8, 0x11b0, { 24,  24,  14}},	// move_8_ix_ix
	{ 0xf1f8, 0x2000, {  4,   4,   2}},	// move_32_d_d
	{ 0xf1f8, 0x2008, {  4,   4,   2}},	// move_32_d_a
	{ 0xf1f8, 0x2010, { 12,  12,   6}},	// move_32_d_ai
	{ 0xf1f8, 0x2018, { 12,  12,   6}},	// move_32_d_pi
	{ 0xf1f8, 0x2020, { 14,  14,   7}},	// move_32_d_pd
	{ 0xf1f8, 0x2028, { 16,  16,   7}},	// move_32_d_di
	{ 0xf1f8, 0x2030, { 18,  18,   9}},	// move_32_d_ix
	{ 0xf1f8, 0x2040, {  4,   4,   2}},	// movea_32_d
	{ 0xf1f8, 0x2048, {  4,   4,   2}},	// movea_32_a
	{ 0xf1f8, 0x2050, { 12,  12,   6}},	// movea_32_ai
	{ 0xf1f8, 0x2058, { 12,  12,   6}},	// movea_32_pi
	{ 0xf1f8, 0x2060, { 14,  14,   7}},	// movea_32_pd
	{ 0xf1f8, 0x2068, { 16,  16,   7}},	// movea_32_di
	{ 0xf1f8, 0x2070, { 18,  18,   9}},	// movea_32_ix
	{ 0xf1f8, 0x2080, { 12,  12,   4}},	// move_32_ai_d
	{ 0xf1f8, 0x2088, { 12,  12,   4}},	// move_32_ai_a
	{ 0xf1f8, 0x2090, { 20,  20,   8}},	// move_32_ai_ai
	{ 0xf1f8, 0x2098, { 20,  20,   8}},	// move_32_ai_pi
	{ 0xf1f8, 0x20a0, { 22,  22,   9}},	// move_32_ai_pd
	{ 0xf1f8, 0x20a8, { 24,  24,   9}},	// move_32_ai_di
	{ 0xf1f8, 0x20b0, { 26,  26,  11}},	// move_32_ai_ix
	{ 0xf1f8, 0x20c0, { 12,  12,   4}},	// move_32_pi_d
	{ 0xf1f8, 0x20c8, { 12,  12,   4}},	// move_32_pi_a
	{ 0xf1f8, 0x20d0, { 20,  20,   8}},	// move_32_pi_ai
	{ 0xf1f8, 0x20d8, { 20,  20,   8}},	// move_32_pi_pi
	{ 0xf1f8, 0x20e0, { 22,  22,   9}},	// move_32_pi_pd
	{ 0xf1f8, 0x20e8, { 24,  24,   9}},	// move_32_pi_di
	{ 0xf1f8, 0x20f0, { 26,  26,  11}},	// move_32_pi_ix
	{ 0xf1f8, 0x2100, { 12,  14,   5}},	// move_32_pd_d
	{ 0xf1f8, 0x2108, { 12,  14,   5}},	// move_32_pd_a
	{ 0xf1f8, 0x2110, { 20,  22,   9}},	// move_32_pd_ai
	{ 0xf1f8, 0x2118, { 20,  22,   9}},	// move_32_pd_pi
	{ 0xf1f8, 0x2120, { 22,  24,  10}},	// move_32_pd_pd
	{ 0xf1f8, 0x2128, { 24,  26,  10}},	// move_32_pd_di
	{ 0xf1f8, 0x2130, { 26,  28,  12}},	// move_32_pd_ix
	{ 0xf1f8, 0x2140, { 16,  16,   5}},	// move_32_di_d
	{ 0xf1f8, 0x2148, { 16,  16,   5}},	// move_32_di_a
	{ 0xf1f8, 0x2150, { 24,  24,   9}},	// move_32_di_ai
	{ 0xf1f8, 0x2158, { 24,  24,   9}},	// move_32_di_pi
	{ 0xf1f8, 0x2160, { 26,  26,  10}},	// move_32_di_pd
	{ 0xf1f8, 0x2168, { 28,  28,  10}},	// move_32_di_di
	{ 0xf1f8, 0x2170, { 30,  30,  12}},	// move_32_di_ix
	{ 0xf1f8, 0x2180, { 18,  18,   7}},	// move_32_ix_d
	{ 0xf1f8, 0x2188, { 18,  18,   7}},	// move_32_ix_a
	{ 0xf1f8, 0x2190, { 26,  26,  11}},	// move_32_ix_ai
	{ 0xf1f8, 0x2198, { 26,  26,  11}},	// move_32_ix_pi
	{ 0xf1f8, 0x21a0, { 28,  28,  12}},	// move_32_ix_pd
	{ 0xf1f8, 0x21a8, { 30,  30,  12}},	// move_32_ix_di
	{ 0xf1f8, 0x21b0, { 32,  32,  14}},	// move_32_ix_ix
	{ 0xf1f8, 0x3000, {  4,   4,   2}},	// move_16_d_d
	{ 0xf1f8, 0x3008, {  4,   4,   2}},	// move_16_d_a
	{ 0xf1f8, 0x3010, {  8,   8,   6}},	// move_16_d_ai
	{ 0xf1f8, 0x3018, {  8,   8,   6}},	// move_16_d_pi
	{ 0xf1f8, 0x3020, { 10,  10,   7}},	// move_16_d_pd
	{ 0xf1f8, 0x3028, { 12,  12,   7}},	// move_16_d_di
	{ 0xf1f8, 0x3030, { 14,  14,   9}},	// move_16_d_ix
	{ 0xf1f8, 0x3040, {  4,   4,   2}},	// movea_16_d
	{ 0xf1f8, 0x3048, {  4,   4,   2}},	// movea_16_a
	{ 0xf1f8, 0x3050, {  8,   8,   6}},	// movea_16_ai
	{ 0xf1f8, 0x3058, {  8,   8,   6}},	// movea_16_pi
	{ 0xf1f8, 0x3060, { 10,  10,   7}},	// movea_16_pd
	{ 0xf1f8, 0x3068, { 12,  12,   7}},	// movea_16_di
	{ 0xf1f8, 0x3070, { 14,  14,   9}},	// movea_16_ix
	{ 0xf1f8, 0x3080, {  8,   8,   4}},	// move_16_ai_d
	{ 0xf1f8, 0x3088, {  8,   8,   4}},	// move_16_ai_a
	{ 0xf1f8, 0x3090, { 12,  12,   8}},	// move_16_ai_ai
	{ 0xf1f8, 0x3098, { 12,  12,   8}},	// move_16_ai_pi
	{ 0xf1f8, 0x30a0, { 14,  14,   9}},	// move_16_ai_pd
	{ 0xf1f8, 0x30a8, { 16,  16,   9}},	// move_16_ai_di
	{ 0xf1f8, 0x30b0, { 18,  18,  11}},	// move_16_ai_ix
	{ 0xf1f8, 0x30c0, {  8,   8,   4}},	// move_16_pi_d
	{ 0xf1f8, 0x30c8, {  8,   8,   4}},	// move_16_pi_a
	{ 0xf1f8, 0x30d0, { 12,  12,   8}},	// move_16_pi_ai
	{ 0xf1f8, 0x30d8, { 12,  12,   8}},	// move_16_pi_pi
	{ 0xf1f8, 0x30e0, { 14,  14,   9}},	// move_16_pi_pd
	{ 0xf1f8, 0x30e8, { 16,  16,   9}},	// move_16_pi_di
	{ 0xf1f8, 0x30f0, { 18,  18,  11}},	// move_16_pi_ix
	{ 0xf1f8, 0x3100, {  8,   8,   5}},	// move_16_pd_d
	{ 0xf1f8, 0x3108, {  8,   8,   5}},	// move_16_pd_a
	{ 0xf1f8, 0x3110, { 12,  12,   9}},	// move_16_pd_ai
	{ 0xf1f8, 0x3118, { 12,  12,   9}},	// move_16_pd_pi
	{ 0xf1f8, 0x3120, { 14,  14,  10}},	// move_16_pd_pd
	{ 0xf1f8, 0x3128, { 16,  16,  10}},	// move_16_pd_di
	{ 0xf1f8, 0x3130, { 18,  18,  12}},	// move_16_pd_ix
	{ 0xf1f8, 0x3140, { 12,  12,   5}},	// move_16_di_d
	{ 0xf1f8, 0x3148, { 12,  12,   5}},	// move_16_di_a
	{ 0xf1f8, 0x3150, { 16,  16,   9}},	// move_16_di_ai
	{ 0xf1f8, 0x3158, { 16,  16,   9}},	// move_16_di_pi
	{ 0xf1f8, 0x3160, { 18,  18,  10}},	// move_16_di_pd
	{ 0xf1f8, 0x3168, { 20,  20,  10}},	// move_16_di_di
	{ 0xf1f8, 0x3170, { 22,  22,  12}},	// move_16_di_ix
	{ 0xf1f8, 0x3180, { 14,  14,   7}},	// move_16_ix_d
	{ 0xf1f8, 0x3188, { 14,  14,   7}},	// move_16_ix_a
	{ 0xf1f8, 0x3190, { 18,  18,  11}},	// move_16_ix_ai
	{ 0xf1f8, 0x3198, { 18,  18,  11}},	// move_16_ix_pi
	{ 0xf1f8, 0x31a0, { 20,  20,  12}},	// move_16_ix_pd
	{ 0xf1f8, 0x31a8, { 22,  22,  12}},	// move_16_ix_di
	{ 0xf1f8, 0x31b0, { 24,  24,  14}},	// move_16_ix_ix
	{ 0xf1f8, 0x4100, {  0,   0,   8}},	// chk_32_d
	{ 0xf1f8, 0x4110, {  0,   0,  12}},	// chk_32_ai
	{ 0xf1f8, 0x4118, {  0,   0,  12}},	// chk_32_pi
	{ 0xf1f8, 0x4120, {  0,   0,  13}},	// chk_32_pd
	{ 0xf1f8, 0x4128, {  0,   0,  13}},	// chk_32_di
	{ 0xf1f8, 0x4130, {  0,   0,  15}},	// chk_32_ix
	{ 0xf1f8, 0x4180, { 10,   8,   8}},	// chk_16_d
	{ 0xf1f8, 0x4190, { 14,  12,  12}},	// chk_16_ai
	{ 0xf1f8, 0x4198, { 14,  12,  12}},	// chk_16_pi
	{ 0xf1f8, 0x41a0, { 16,  14,  13}},	// chk_16_pd
	{ 0xf1f8, 0x41a8, { 18,  16,  13}},	// chk_16_di
	{ 0xf1f8, 0x41b0, { 20,  18,  15}},	// chk_16_ix
	{ 0xf1f8, 0x41d0, {  4,   4,   6}},	// lea_32_ai
	{ 0xf1f8, 0x41e8, {  8,   8,   7}},	// lea_32_di
	{ 0xf1f8, 0x41f0, { 12,  12,   9}},	// lea_32_ix
	{ 0xf1f8, 0x5000, {  4,   4,   2}},	// addq_8_d
	{ 0xf1f8, 0x5010, { 12,  12,   8}},	// addq_8_ai
	{ 0xf1f8, 0x5018, { 12,  12,   8}},	// addq_8_pi
	{ 0xf1f8, 0x5020, { 14,  14,   9}},	// addq_8_pd
	{ 0xf1f8, 0x5028, { 16,  16,   9}},	// addq_8_di
	{ 0xf1f8, 0x5030, { 18,  18,  11}},	// addq_8_ix
	{ 0xf1f8, 0x5040, {  4,   4,   2}},	// addq_16_d
	{ 0xf1f8, 0x5048, {  4,   4,   2}},	// addq_16_a
	{ 0xf1f8, 0x5050, { 12,  12,   8}},	// addq_16_ai
	{ 0xf1f8, 0x5058, { 12,  12,   8}},	// addq_16_pi
	{ 0xf1f8, 0x5060, { 14,  14,   9}},	// addq_16_pd
	{ 0xf1f8, 0x5068, { 16,  16,   9}},	// addq_16_di
	{ 0xf1f8, 0x5070, { 18,  18,  11}},	// addq_16_ix
	{ 0xf1f8, 0x5080, {  8,   8,   2}},	// addq_32_d
	{ 0xf1f8, 0x5088, {  8,   8,   2}},	// addq_32_a
	{ 0xf1f8, 0x5090, { 20,  20,   8}},	// addq_32_ai
	{ 0xf1f8, 0x5098, { 20,  20,   8}},	// addq_32_pi
	{ 0xf1f8, 0x50a0, { 22,  22,   9}},	// addq_32_pd
	{ 0xf1f8, 0x50a8, { 24,  24,   9}},	// addq_32_di
	{ 0xf1f8, 0x50b0, { 26,  26,  11}},	// addq_32_ix
	{ 0xf1f8, 0x5100, {  4,   4,   2}},	// subq_8_d
	{ 0xf1f8, 0x5110, { 12,  12,   8}},	// subq_8_ai
	{ 0xf1f8, 0x5118, { 12,  12,   8}},	// subq_8_pi
	{ 0xf1f8, 0x5120, { 14,  14,   9}},	// subq_8_pd
	{ 0xf1f8, 0x5128, { 16,  16,   9}},	// subq_8_di
	{ 0xf1f8, 0x5130, { 18,  18,  11}},	// subq_8_ix
	{ 0xf1f8, 0x5140, {  4,   4,   2}},	// subq_16_d
	{ 0xf1f8, 0x5148, {  8,   4,   2}},	// subq_16_a
	{ 0xf1f8, 0x5150, { 12,  12,   8}},	// subq_16_ai
	{ 0xf1f8, 0x5158, { 12,  12,   8}},	// subq_16_pi
	{ 0xf1f8, 0x5160, { 14,  14,   9}},	// subq_16_pd
	{ 0xf1f8, 0x5168, { 16,  16,   9}},	// subq_16_di
	{ 0xf1f8, 0x5170, { 18,  18,  11}},	// subq_16_ix
	{ 0xf1f8, 0x5180, {  8,   8,   2}},	// subq_32_d
	{ 0xf1f8, 0x5188, {  8,   8,   2}},	// subq_32_a
	{ 0xf1f8, 0x5190, { 20,  20,   8}},	// subq_32_ai
	{ 0xf1f8, 0x5198, { 20,  20,   8}},	// subq_32_pi
	{ 0xf1f8, 0x51a0, { 22,  22,   9}},	// subq_32_pd
	{ 0xf1f8, 0x51a8, { 24,  24,   9}},	// subq_32_di
	{ 0xf1f8, 0x51b0, { 26,  26,  11}},	// subq_32_ix
	{ 0xf1f8, 0x8000, {  4,   4,   2}},	// or_8_er_d
	{ 0xf1f8, 0x8010, {  8,   8,   6}},	// or_8_er_ai
	{ 0xf1f8, 0x8018, {  8,   8,   6}},	// or_8_er_pi
	{ 0xf1f8, 0x8020, { 10,  10,   7}},	// or_8_er_pd
	{ 0xf1f8, 0x8028, { 12,  12,   7}},	// or_8_er_di
	{ 0xf1f8, 0x8030, { 14,  14,   9}},	// or_8_er_ix
	{ 0xf1f8, 0x8040, {  4,   4,   2}},	// or_16_er_d
	{ 0xf1f8, 0x8050, {  8,   8,   6}},	// or_16_er_ai
	{ 0xf1f8, 0x8058, {  8,   8,   6}},	// or_16_er_pi
	{ 0xf1f8, 0x8060, { 10,  10,   7}},	// or_16_er_pd
	{ 0xf1f8, 0x8068, { 12,  12,   7}},	// or_16_er_di
	{ 0xf1f8, 0x8070, { 14,  14,   9}},	// or_16_er_ix
	{ 0xf1f8, 0x8080, {  8,   6,   2}},	// or_32_er_d
	{ 0xf1f8, 0x8090, { 14,  14,   6}},	// or_32_er_ai
	{ 0xf1f8, 0x8098, { 14,  14,   6}},	// or_32_er_pi
	{ 0xf1f8, 0x80a0, { 16,  16,   7}},	// or_32_er_pd
	{ 0xf1f8, 0x80a8, { 18,  18,   7}},	// or_32_er_di
	{ 0xf1f8, 0x80b0, { 20,  20,   9}},	// or_32_er_ix
	{ 0xf1f8, 0x80c0, {140, 108,  44}},	// divu_16_d
	{ 0xf1f8, 0x80d0, {144, 112,  48}},	// divu_16_ai
	{ 0xf1f8, 0x80d8, {144, 112,  48}},	// divu_16_pi
	{ 0xf1f8, 0x80e0, {146, 114,  49}},	// divu_16_pd
	{ 0xf1f8, 0x80e8, {148, 116,  49}},	// divu_16_di
	{ 0xf1f8, 0x80f0, {150, 118,  51}},	// divu_16_ix
	{ 0xf1f8, 0x8100, {  6,   6,   4}},	// sbcd_8_rr
	{ 0xf1f8, 0x8108, { 18,  18,  16}},	// sbcd_8_mm
	{ 0xf1f8, 0x8110, { 12,  12,   8}},	// or_8_re_ai
	{ 0xf1f8, 0x8118, { 12,  12,   8}},	// or_8_re_pi
	{ 0xf1f8, 0x8120, { 14,  14,   9}},	// or_8_re_pd
	{ 0xf1f8, 0x8128, { 16,  16,   9}},	// or_8_re_di
	{ 0xf1f8, 0x8130, { 18,  18,  11}},	// or_8_re_ix
	{ 0xf1f8, 0x8140, {  0,   0,   6}},	// pack_16_rr
	{ 0xf1f8, 0x8148, {  0,   0,  13}},	// pack_16_mm
	{ 0xf1f8, 0x8150, { 12,  12,   8}},	// or_16_re_ai
	{ 0xf1f8, 0x8158, { 12,  12,   8}},	// or_16_re_pi
	{ 0xf1f8, 0x8160, { 14,  14,   9}},	// or_16_re_pd
	{ 0xf1f8, 0x8168, { 16,  16,   9}},	// or_16_re_di
	{ 0xf1f8, 0x8170, { 18,  18,  11}},	// or_16_re_ix
	{ 0xf1f8, 0x8180, {  0,   0,   8}},	// unpk_16_rr
	{ 0xf1f8, 0x8188, {  0,   0,  13}},	// unpk_16_mm
	{ 0xf1f8, 0x8190, { 20,  20,   8}},	// or_32_re_ai
	{ 0xf1f8, 0x8198, { 20,  20,   8}},	// or_32_re_pi
	{ 0xf1f8, 0x81a0, { 22,  22,   9}},	// or_32_re_pd
	{ 0xf1f8, 0x81a8, { 24,  24,   9}},	// or_32_re_di
	{ 0xf1f8, 0x81b0, { 26,  26,  11}},	// or_32_re_ix
	{ 0xf1f8, 0x81c0, {158, 122,  56}},	// divs_16_d
	{ 0xf1f8, 0x81d0, {162, 126,  60}},	// divs_16_ai
	{ 0xf1f8, 0x81d8, {162, 126,  60}},	// divs_16_pi
	{ 0xf1f8, 0x81e0, {164, 128,  61}},	// divs_16_pd
	{ 0xf1f8, 0x81e8, {166, 130,  61}},	// divs_16_di
	{ 0xf1f8, 0x81f0, {168, 132,  63}},	// divs_16_ix
	{ 0xf1f8, 0x9000, {  4,   4,   2}},	// sub_8_er_d
	{ 0xf1f8, 0x9010, {  8,   8,   6}},	// sub_8_er_ai
	{ 0xf1f8, 0x9018, {  8,   8,   6}},	// sub_8_er_pi
	{ 0xf1f8, 0x9020, { 10,  10,   7}},	// sub_8_er_pd
	{ 0xf1f8, 0x9028, { 12,  12,   7}},	// sub_8_er_di
	{ 0xf1f8, 0x9030, { 14,  14,   9}},	// sub_8_er_ix
	{ 0xf1f8, 0x9040, {  4,   4,   2}},	// sub_16_er_d
	{ 0xf1f8, 0x9048, {  4,   4,   2}},	// sub_16_er_a
	{ 0xf1f8, 0x9050, {  8,   8,   6}},	// sub_16_er_ai
	{ 0xf1f8, 0x9058, {  8,   8,   6}},	// sub_16_er_pi
	{ 0xf1f8, 0x9060, { 10,  10,   7}},	// sub_16_er_pd
	{ 0xf1f8, 0x9068, { 12,  12,   7}},	// sub_16_er_di
	{ 0xf1f8, 0x9070, { 14,  14,   9}},	// sub_16_er_ix
	{ 0xf1f8, 0x9080, {  8,   6,   2}},	// sub_32_er_d
	{ 0xf1f8, 0x9088, {  8,   6,   2}},	// sub_32_er_a
	{ 0xf1f8, 0x9090, { 14,  14,   6}},	// sub_32_er_ai
	{ 0xf1f8, 0x9098, { 14,  14,   6}},	// sub_32_er_pi
	{ 0xf1f8, 0x90a0, { 16,  16,   7}},	// sub_32_er_pd
	{ 0xf1f8, 0x90a8, { 18,  18,   7}},	// sub_32_er_di
	{ 0xf1f8, 0x90b0, { 20,  20,   9}},	// sub_32_er_ix
	{ 0xf1f8, 0x90c0, {  8,   8,   2}},	// suba_16_d
	{ 0xf1f8, 0x90c8, {  8,   8,   2}},	// suba_16_a
	{ 0xf1f8, 0x90d0, { 12,  12,   6}},	// suba_16_ai
	{ 0xf1f8, 0x90d8, { 12,  12,   6}},	// suba_16_pi
	{ 0xf1f8, 0x90e0, { 14,  14,   7}},	// suba_16_pd
	{ 0xf1f8, 0x90e8, { 16,  16,   7}},	// suba_16_di
	{ 0xf1f8, 0x90f0, { 18,  18,   9}},	// suba_16_ix
	{ 0xf1f8, 0x9100, {  4,   4,   2}},	// subx_8_rr
	{ 0xf1f8, 0x9108, { 18,  18,  12}},	// subx_8_mm
	{ 0xf1f8, 0x9110, { 12,  12,   8}},	// sub_8_re_ai
	{ 0xf1f8, 0x9118, { 12,  12,   8}},	// sub_8_re_pi
	{ 0xf1f8, 0x9120, { 14,  14,   9}},	// sub_8_re_pd
	{ 0xf1f8, 0x9128, { 16,  16,   9}},	// sub_8_re_di
	{ 0xf1f8, 0x9130, { 18,  18,  11}},	// sub_8_re_ix
	{ 0xf1f8, 0x9140, {  4,   4,   2}},	// subx_16_rr
	{ 0xf1f8, 0x9148, { 18,  18,  12}},	// subx_16_mm
	{ 0xf1f8, 0x9150, { 12,  12,   8}},	// sub_16_re_ai
	{ 0xf1f8, 0x9158, { 12,  12,   8}},	// sub_16_re_pi
	{ 0xf1f8, 0x9160, { 14,  14,   9}},	// sub_16_re_pd
	{ 0xf1f8, 0x9168, { 16,  16,   9}},	// sub_16_re_di
	{ 0xf1f8, 0x9170, { 18,  18,  11}},	// sub_16_re_ix
	{ 0xf1f8, 0x9180, {  8,   6,   2}},	// subx_32_rr
	{ 0xf1f8, 0x9188, { 30,  30,  12}},	// subx_32_mm
	{ 0xf1f8, 0x9190, { 20,  20,   8}},	// sub_32_re_ai
	{ 0xf1f8, 0x9198, { 20,  20,   8}},	// sub_32_re_pi
	{ 0xf1f8, 0x91a0, { 22,  22,   9}},	// sub_32_re_pd
	{ 0xf1f8, 0x91a8, { 24,  24,   9}},	// sub_32_re_di
	{ 0xf1f8, 0x91b0, { 26,  26,  11}},	// sub_32_re_ix
	{ 0xf1f8, 0x91c0, {  8,   6,   2}},	// suba_32_d
	{ 0xf1f8, 0x91c8, {  8,   6,   2}},	// suba_32_a
	{ 0xf1f8, 0x91d0, { 14,  14,   6}},	// suba_32_ai
	{ 0xf1f8, 0x91d8, { 14,  14,   6}},	// suba_32_pi
	{ 0xf1f8, 0x91e0, { 16,  16,   7}},	// suba_32_pd
	{ 0xf1f8, 0x91e8, { 18,  18,   7}},	// suba_32_di
	{ 0xf1f8, 0x91f0, { 20,  20,   9}},	// suba_32_ix
	{ 0xf1f8, 0xb000, {  4,   4,   2}},	// cmp_8_d
	{ 0xf1f8, 0xb010, {  8,   8,   6}},	// cmp_8_ai
	{ 0xf1f8, 0xb018, {  8,   8,   6}},	// cmp_8_pi
	{ 0xf1f8, 0xb020, { 10,  10,   7}},	// cmp_8_pd
	{ 0xf1f8, 0xb028, { 12,  12,   7}},	// cmp_8_di
	{ 0xf1f8, 0xb030, { 14,  14,   9}},	// cmp_8_ix
	{ 0xf1f8, 0xb040, {  4,   4,   2}},	// cmp_16_d
	{ 0xf1f8, 0xb048, {  4,   4,   2}},	// cmp_16_a
	{ 0xf1f8, 0xb050, {  8,   8,   6}},	// cmp_16_ai
	{ 0xf1f8, 0xb058, {  8,   8,   6}},	// cmp_16_pi
	{ 0xf1f8, 0xb060, { 10,  10,   7}},	// cmp_16_pd
	{ 0xf1f8, 0xb068, { 12,  12,   7}},	// cmp_16_di
	{ 0xf1f8, 0xb070, { 14,  14,   9}},	// cmp_16_ix
	{ 0xf1f8, 0xb080, {  6,   6,   2}},	// cmp_32_d
	{ 0xf1f8, 0xb088, {  6,   6,   2}},	// cmp_32_a
	{ 0xf1f8, 0xb090, { 14,  14,   6}},	// cmp_32_ai
	{ 0xf1f8, 0xb098, { 14,  14,   6}},	// cmp_32_pi
	{ 0xf1f8, 0xb0a0, { 16,  16,   7}},	// cmp_32_pd
	{ 0xf1f8, 0xb0a8, { 18,  18,   7}},	// cmp_32_di
	{ 0xf1f8, 0xb0b0, { 20,  20,   9}},	// cmp_32_ix
	{ 0xf1f8, 0xb0c0, {  6,   6,   4}},	// cmpa_16_d
	{ 0xf1f8, 0xb0c8, {  6,   6,   4}},	// cmpa_16_a
	{ 0xf1f8, 0xb0d0, { 10,  10,   8}},	// cmpa_16_ai
	{ 0xf1f8, 0xb0d8, { 10,  10,   8}},	// cmpa_16_pi
	{ 0xf1f8, 0xb0e0, { 12,  12,   9}},	// cmpa_16_pd
	{ 0xf1f8, 0xb0e8, { 14,  14,   9}},	// cmpa_16_di
	{ 0xf1f8, 0xb0f0, { 16,  16,  11}},	// cmpa_16_ix
	{ 0xf1f8, 0xb100, {  4,   4,   2}},	// eor_8_d
	{ 0xf1f8, 0xb108, { 12,  12,   9}},	// cmpm_8
	{ 0xf1f8, 0xb110, { 12,  12,   8}},	// eor_8_ai
	{ 0xf1f8, 0xb118, { 12,  12,   8}},	// eor_8_pi
	{ 0xf1f8, 0xb120, { 14,  14,   9}},	// eor_8_pd
	{ 0xf1f8, 0xb128, { 16,  16,   9}},	// eor_8_di
	{ 0xf1f8, 0xb130, { 18,  18,  11}},	// eor_8_ix
	{ 0xf1f8, 0xb140, {  4,   4,   2}},	// eor_16_d
	{ 0xf1f8, 0xb148, { 12,  12,   9}},	// cmpm_16
	{ 0xf1f8, 0xb150, { 12,  12,   8}},	// eor_16_ai
	{ 0xf1f8, 0xb158, { 12,  12,   8}},	// eor_16_pi
	{ 0xf1f8, 0xb160, { 14,  14,   9}},	// eor_16_pd
	{ 0xf1f8, 0xb168, { 16,  16,   9}},	// eor_16_di
	{ 0xf1f8, 0xb170, { 18,  18,  11}},	// eor_16_ix
	{ 0xf1f8, 0xb180, {  8,   6,   2}},	// eor_32_d
	{ 0xf1f8, 0xb188, { 20,  20,   9}},	// cmpm_32
	{ 0xf1f8, 0xb190, { 20,  20,   8}},	// eor_32_ai
	{ 0xf1f8, 0xb198, { 20,  20,   8}},	// eor_32_pi
	{ 0xf1f8, 0xb1a0, { 22,  22,   9}},	// eor_32_pd
	{ 0xf1f8, 0xb1a8, { 24,  24,   9}},	// eor_32_di
	{ 0xf1f8, 0xb1b0, { 26,  26,  11}},	// eor_32_ix
	{ 0xf1f8, 0xb1c0, {  6,   6,   4}},	// cmpa_32_d
	{ 0xf1f8, 0xb1c8, {  6,   6,   4}},	// cmpa_32_a
	{ 0xf1f8, 0xb1d0, { 14,  14,   8}},	// cmpa_32_ai
	{ 0xf1f8, 0xb1d8, { 14,  14,   8}},	// cmpa_32_pi
	{ 0xf1f8, 0xb1e0, { 16,  16,   9}},	// cmpa_32_pd
	{ 0xf1f8, 0xb1e8, { 18,  18,   9}},	// cmpa_32_di
	{ 0xf1f8, 0xb1f0, { 20,  20,  11}},	// cmpa_32_ix
	{ 0xf1f8, 0xc000, {  4,   4,   2}},	// and_8_er_d
	{ 0xf1f8, 0xc010, {  8,   8,   6}},	// and_8_er_ai
	{ 0xf1f8, 0xc018, {  8,   8,   6}},	// and_8_er_pi
	{ 0xf1f8, 0xc020, { 10,  10,   7}},	// and_8_er_pd
	{ 0xf1f8, 0xc028, { 12,  12,   7}},	// and_8_er_di
	{ 0xf1f8, 0xc030, { 14,  14,   9}},	// and_8_er_ix
	{ 0xf1f8, 0xc040, {  4,   4,   2}},	// and_16_er_d
	{ 0xf1f8, 0xc050, {  8,   8,   6}},	// and_16_er_ai
	{ 0xf1f8, 0xc058, {  8,   8,   6}},	// and_16_er_pi
	{ 0xf1f8, 0xc060, { 10,  10,   7}},	// and_16_er_pd
	{ 0xf1f8, 0xc068, { 12,  12,   7}},	// and_16_er_di
	{ 0xf1f8, 0xc070, { 14,  14,   9}},	// and_16_er_ix
	{ 0xf1f8, 0xc080, {  8,   6,   2}},	// and_32_er_d
	{ 0xf1f8, 0xc090, { 14,  14,   6}},	// and_32_er_ai
	{ 0xf1f8, 0xc098, { 14,  14,   6}},	// and_32_er_pi
	{ 0xf1f8, 0xc0a0, { 16,  16,   7}},	// and_32_er_pd
	{ 0xf1f8, 0xc0a8, { 18,  18,   7}},	// and_32_er_di
	{ 0xf1f8, 0xc0b0, { 20,  20,   9}},	// and_32_er_ix
	{ 0xf1f8, 0xc0c0, { 54,  30,  27}},	// mulu_16_d
	{ 0xf1f8, 0xc0d0, { 58,  34,  31}},	// mulu_16_ai
	{ 0xf1f8, 0xc0d8, { 58,  34,  31}},	// mulu_16_pi
	{ 0xf1f8, 0xc0e0, { 60,  36,  32}},	// mulu_16_pd
	{ 0xf1f8, 0xc0e8, { 62,  38,  32}},	// mulu_16_di
	{ 0xf1f8, 0xc0f0, { 64,  40,  34}},	// mulu_16_ix
	{ 0xf1f8, 0xc100, {  6,   6,   4}},	// abcd_8_rr
	{ 0xf1f8, 0xc108, { 18,  18,  16}},	// abcd_8_mm
	{ 0xf1f8, 0xc110, { 12,  12,   8}},	// and_8_re_ai
	{ 0xf1f8, 0xc118, { 12,  12,   8}},	// and_8_re_pi
	{ 0xf1f8, 0xc120, { 14,  14,   9}},	// and_8_re_pd
	{ 0xf1f8, 0xc128, { 16,  16,   9}},	// and_8_re_di
	{ 0xf1f8, 0xc130, { 18,  18,  11}},	// and_8_re_ix
	{ 0xf1f8, 0xc140, {  6,   6,   2}},	// exg_32_dd
	{ 0xf1f8, 0xc148, {  6,   6,   2}},	// exg_32_aa
	{ 0xf1f8, 0xc150, { 12,  12,   8}},	// and_16_re_ai
	{ 0xf1f8, 0xc158, { 12,  12,   8}},	// and_16_re_pi
	{ 0xf1f8, 0xc160, { 14,  14,   9}},	// and_16_re_pd
	{ 0xf1f8, 0xc168, { 16,  16,   9}},	// and_16_re_di
	{ 0xf1f8, 0xc170, { 18,  18,  11}},	// and_16_re_ix
	{ 0xf1f8, 0xc188, {  6,   6,   2}},	// exg_32_da
	{ 0xf1f8, 0xc190, { 20,  20,   8}},	// and_32_re_ai
	{ 0xf1f8, 0xc198, { 20,  20,   8}},	// and_32_re_pi
	{ 0xf1f8, 0xc1a0, { 22,  22,   9}},	// and_32_re_pd
	{ 0xf1f8, 0xc1a8, { 24,  24,   9}},	// and_32_re_di
	{ 0xf1f8, 0xc1b0, { 26,  26,  11}},	// and_32_re_ix
	{ 0xf1f8, 0xc1c0, { 54,  32,  27}},	// muls_16_d
	{ 0xf1f8, 0xc1d0, { 58,  36,  31}},	// muls_16_ai
	{ 0xf1f8, 0xc1d8, { 58,  36,  31}},	// muls_16_pi
	{ 0xf1f8, 0xc1e0, { 60,  38,  32}},	// muls_16_pd
	{ 0xf1f8, 0xc1e8, { 62,  40,  32}},	// muls_16_di
	{ 0xf1f8, 0xc1f0, { 64,  42,  34}},	// muls_16_ix
	{ 0xf1f8, 0xd000, {  4,   4,   2}},	// add_8_er_d
	{ 0xf1f8, 0xd010, {  8,   8,   6}},	// add_8_er_ai
	{ 0xf1f8, 0xd018, {  8,   8,   6}},	// add_8_er_pi
	{ 0xf1f8, 0xd020, { 10,  10,   7}},	// add_8_er_pd
	{ 0xf1f8, 0xd028, { 12,  12,   7}},	// add_8_er_di
	{ 0xf1f8, 0xd030, { 14,  14,   9}},	// add_8_er_ix
	{ 0xf1f8, 0xd040, {  4,   4,   2}},	// add_16_er_d
	{ 0xf1f8, 0xd048, {  4,   4,   2}},	// add_16_er_a
	{ 0xf1f8, 0xd050, {  8,   8,   6}},	// add_16_er_ai
	{ 0xf1f8, 0xd058, {  8,   8,   6}},	// add_16_er_pi
	{ 0xf1f8, 0xd060, { 10,  10,   7}},	// add_16_er_pd
	{ 0xf1f8, 0xd068, { 12,  12,   7}},	// add_16_er_di
	{ 0xf1f8, 0xd070, { 14,  14,   9}},	// add_16_er_ix
	{ 0xf1f8, 0xd080, {  8,   6,   2}},	// add_32_er_d
	{ 0xf1f8, 0xd088, {  8,   6,   2}},	// add_32_er_a
	{ 0xf1f8, 0xd090, { 14,  14,   6}},	// add_32_er_ai
	{ 0xf1f8, 0xd098, { 14,  14,   6}},	// add_32_er_pi
	{ 0xf1f8, 0xd0a0, { 16,  16,   7}},	// add_32_er_pd
	{ 0xf1f8, 0xd0a8, { 18,  18,   7}},	// add_32_er_di
	{ 0xf1f8, 0xd0b0, { 20,  20,   9}},	// add_32_er_ix
	{ 0xf1f8, 0xd0c0, {  8,   8,   2}},	// adda_16_d
	{ 0xf1f8, 0xd0c8, {  8,   8,   2}},	// adda_16_a
	{ 0xf1f8, 0xd0d0, { 12,  12,   6}},	// adda_16_ai
	{ 0xf1f8, 0xd0d8, { 12,  12,   6}},	// adda_16_pi
	{ 0xf1f8, 0xd0e0, { 14,  14,   7}},	// adda_16_pd
	{ 0xf1f8, 0xd0e8, { 16,  16,   7}},	// adda_16_di
	{ 0xf1f8, 0xd0f0, { 18,  18,   9}},	// adda_16_ix
	{ 0xf1f8, 0xd100, {  4,   4,   2}},	// addx_8_rr
	{ 0xf1f8, 0xd108, { 18,  18,  12}},	// addx_8_mm
	{ 0xf1f8, 0xd110, { 12,  12,   8}},	// add_8_re_ai
	{ 0xf1f8, 0xd118, { 12,  12,   8}},	// add_8_re_pi
	{ 0xf1f8, 0xd120, { 14,  14,   9}},	// add_8_re_pd
	{ 0xf1f8, 0xd128, { 16,  16,   9}},	// add_8_re_di
	{ 0xf1f8, 0xd130, { 18,  18,  11}},	// add_8_re_ix
	{ 0xf1f8, 0xd140, {  4,   4,   2}},	// addx_16_rr
	{ 0xf1f8, 0xd148, { 18,  18,  12}},	// addx_16_mm
	{ 0xf1f8, 0xd150, { 12,  12,   8}},	// add_16_re_ai
	{ 0xf1f8, 0xd158, { 12,  12,   8}},	// add_16_re_pi
	{ 0xf1f8, 0xd160, { 14,  14,   9}},	// add_16_re_pd
	{ 0xf1f8, 0xd168, { 16,  16,   9}},	// add_16_re_di
	{ 0xf1f8, 0xd170, { 18,  18,  11}},	// add_16_re_ix
	{ 0xf1f8, 0xd180, {  8,   6,   2}},	// addx_32_rr
	{ 0xf1f8, 0xd188, { 30,  30,  12}},	// addx_32_mm
	{ 0xf1f8, 0xd190, { 20,  20,   8}},	// add_32_re_ai
	{ 0xf1f8, 0xd198, { 20,  20,   8}},	// add_32_re_pi
	{ 0xf1f8, 0xd1a0, { 22,  22,   9}},	// add_32_re_pd
	{ 0xf1f8, 0xd1a8, { 24,  24,   9}},	// add_32_re_di
	{ 0xf1f8, 0xd1b0, { 26,  26,  11}},	// add_32_re_ix
	{ 0xf1f8, 0xd1c0, {  8,   6,   2}},	// adda_32_d
	{ 0xf1f8, 0xd1c8, {  8,   6,   2}},	// adda_32_a
	{ 0xf1f8, 0xd1d0, { 14,  14,   6}},	// adda_32_ai
	{ 0xf1f8, 0xd1d8, { 14,  14,   6}},	// adda_32_pi
	{ 0xf1f8, 0xd1e0, { 16,  16,   7}},	// adda_32_pd
	{ 0xf1f8, 0xd1e8, { 18,  18,   7}},	// adda_32_di
	{ 0xf1f8, 0xd1f0, { 20,  20,   9}},	// adda_32_ix
	{ 0xf1f8, 0xe000, {  6,   6,   6}},	// asr_8_s
	{ 0xf1f8, 0xe008, {  6,   6,   4}},	// lsr_8_s
	{ 0xf1f8, 0xe010, {  6,   6,  12}},	// roxr_8_s
	{ 0xf1f8, 0xe018, {  6,   6,   8}},	// ror_8_s
	{ 0xf1f8, 0xe020, {  6,   6,   6}},	// asr_8_r
	{ 0xf1f8, 0xe028, {  6,   6,   6}},	// lsr_8_r
	{ 0xf1f8, 0xe030, {  6,   6,  12}},	// roxr_8_r
	{ 0xf1f8, 0xe038, {  6,   6,   8}},	// ror_8_r
	{ 0xf1f8, 0xe040, {  6,   6,   6}},	// asr_16_s
	{ 0xf1f8, 0xe048, {  6,   6,   4}},	// lsr_16_s
	{ 0xf1f8, 0xe050, {  6,   6,  12}},	// roxr_16_s
	{ 0xf1f8, 0xe058, {  6,   6,   8}},	// ror_16_s
	{ 0xf1f8, 0xe060, {  6,   6,   6}},	// asr_16_r
	{ 0xf1f8, 0xe068, {  6,   6,   6}},	// lsr_16_r
	{ 0xf1f8, 0xe070, {  6,   6,  12}},	// roxr_16_r
	{ 0xf1f8, 0xe078, {  6,   6,   8}},	// ror_16_r
	{ 0xf1f8, 0xe080, {  8,   8,   6}},	// asr_32_s
	{ 0xf1f8, 0xe088, {  8,   8,   4}},	// lsr_32_s
	{ 0xf1f8, 0xe090, {  8,   8,  12}},	// roxr_32_s
	{ 0xf1f8, 0xe098, {  8,   8,   8}},	// ror_32_s
	{ 0xf1f8, 0xe0a0, {  8,   8,   6}},	// asr_32_r
	{ 0xf1f8, 0xe0a8, {  8,   8,   6}},	// lsr_32_r
	{ 0xf1f8, 0xe0b0, {  8,   8,  12}},	// roxr_32_r
	{ 0xf1f8, 0xe0b8, {  8,   8,   8}},	// ror_32_r
	{ 0xf1f8, 0xe100, {  6,   6,   8}},	// asl_8_s
	{ 0xf1f8, 0xe108, {  6,   6,   4}},	// lsl_8_s
	{ 0xf1f8, 0xe110, {  6,   6,  12}},	// roxl_8_s
	{ 0xf1f8, 0xe118, {  6,   6,   8}},	// rol_8_s
	{ 0xf1f8, 0xe120, {  6,   6,   8}},	// asl_8_r
	{ 0xf1f8, 0xe128, {  6,   6,   6}},	// lsl_8_r
	{ 0xf1f8, 0xe130, {  6,   6,  12}},	// roxl_8_r
	{ 0xf1f8, 0xe138, {  6,   6,   8}},	// rol_8_r
	{ 0xf1f8, 0xe140, {  6,   6,   8}},	// asl_16_s
	{ 0xf1f8, 0xe148, {  6,   6,   4}},	// lsl_16_s
	{ 0xf1f8, 0xe150, {  6,   6,  12}},	// roxl_16_s
	{ 0xf1f8, 0xe158, {  6,   6,   8}},	// rol_16_s
	{ 0xf1f8, 0xe160, {  6,   6,   8}},	// asl_16_r
	{ 0xf1f8, 0xe168, {  6,   6,   6}},	// lsl_16_r
	{ 0xf1f8, 0xe170, {  6,   6,  12}},	// roxl_16_r
	{ 0xf1f8, 0xe178, {  6,   6,   8}},	// rol_16_r
	{ 0xf1f8, 0xe180, {  8,   8,   8}},	// asl_32_s
	{ 0xf1f8, 0xe188, {  8,   8,   4}},	// lsl_32_s
	{ 0xf1f8, 0xe190, {  8,   8,  12}},	// roxl_32_s
	{ 0xf1f8, 0xe198, {  8,   8,   8}},	// rol_32_s
	{ 0xf1f8, 0xe1a0, {  8,   8,   8}},	// asl_32_r
	{ 0xf1f8, 0xe1a8, {  8,   8,   6}},	// lsl_32_r
	{ 0xf1f8, 0xe1b0, {  8,   8,  12}},	// roxl_32_r
	{ 0xf1f8, 0xe1b8, {  8,   8,   8}},	// rol_32_r
	{ 0xf1f8, 0xf048, {  0,   0,   4}},	// cpdbcc_32
	{ 0xf1f8, 0xf078, {  0,   0,   4}},	// cptrapcc_32
	{ 0xfff0, 0x06c0, {  0,   0,  19}},	// rtm_32
	{ 0xfff0, 0x4e40, {  4,   4,   4}},	// trap
	{ 0xf1ff, 0x011f, {  8,   8,   8}},	// btst_8_r_pi7
	{ 0xf1ff, 0x0127, { 10,  10,   9}},	// btst_8_r_pd7
	{ 0xf1ff, 0x0138, { 12,  12,   8}},	// btst_8_r_aw
	{ 0xf1ff, 0x0139, { 16,  16,   8}},	// btst_8_r_al
	{ 0xf1ff, 0x013a, { 12,  12,   9}},	// btst_8_r_pcdi
	{ 0xf1ff, 0x013b, { 14,  14,  11}},	// btst_8_r_pcix
	{ 0xf1ff, 0x013c, {  8,   8,   6}},	// btst_8_r_i
	{ 0xf1ff, 0x015f, { 12,  12,   8}},	// bchg_8_r_pi7
	{ 0xf1ff, 0x0167, { 14,  14,   9}},	// bchg_8_r_pd7
	{ 0xf1ff, 0x0178, { 16,  16,   8}},	// bchg_8_r_aw
	{ 0xf1ff, 0x0179, { 20,  20,   8}},	// bchg_8_r_al
	{ 0xf1ff, 0x019f, { 12,  14,   8}},	// bclr_8_r_pi7
	{ 0xf1ff, 0x01a7, { 14,  16,   9}},	// bclr_8_r_pd7
	{ 0xf1ff, 0x01b8, { 16,  18,   8}},	// bclr_8_r_aw
	{ 0xf1ff, 0x01b9, { 20,  22,   8}},	// bclr_8_r_al
	{ 0xf1ff, 0x01df, { 12,  12,   8}},	// bset_8_r_pi7
	{ 0xf1ff, 0x01e7, { 14,  14,   9}},	// bset_8_r_pd7
	{ 0xf1ff, 0x01f8, { 16,  16,   8}},	// bset_8_r_aw
	{ 0xf1ff, 0x01f9, { 20,  20,   8}},	// bset_8_r_al
	{ 0xf1ff, 0x101f, {  8,   8,   6}},	// move_8_d_pi7
	{ 0xf1ff, 0x1027, { 10,  10,   7}},	// move_8_d_pd7
	{ 0xf1ff, 0x1038, { 12,  12,   6}},	// move_8_d_aw
	{ 0xf1ff, 0x1039, { 16,  16,   6}},	// move_8_d_al
	{ 0xf1ff, 0x103a, { 12,  12,   7}},	// move_8_d_pcdi
	{ 0xf1ff, 0x103b, { 14,  14,   9}},	// move_8_d_pcix
	{ 0xf1ff, 0x103c, {  8,   8,   4}},	// move_8_d_i
	{ 0xf1ff, 0x109f, { 12,  12,   8}},	// move_8_ai_pi7
	{ 0xf1ff, 0x10a7, { 14,  14,   9}},	// move_8_ai_pd7
	{ 0xf1ff, 0x10b8, { 16,  16,   8}},	// move_8_ai_aw
	{ 0xf1ff, 0x10b9, { 20,  20,   8}},	// move_8_ai_al
	{ 0xf1ff, 0x10ba, { 16,  16,   9}},	// move_8_ai_pcdi
	{ 0xf1ff, 0x10bb, { 18,  18,  11}},	// move_8_ai_pcix
	{ 0xf1ff, 0x10bc, { 12,  12,   6}},	// move_8_ai_i
	{ 0xf1ff, 0x10df, { 12,  12,   8}},	// move_8_pi_pi7
	{ 0xf1ff, 0x10e7, { 14,  14,   9}},	// move_8_pi_pd7
	{ 0xf1ff, 0x10f8, { 16,  16,   8}},	// move_8_pi_aw
	{ 0xf1ff, 0x10f9, { 20,  20,   8}},	// move_8_pi_al
	{ 0xf1ff, 0x10fa, { 16,  16,   9}},	// move_8_pi_pcdi
	{ 0xf1ff, 0x10fb, { 18,  18,  11}},	// move_8_pi_pcix
	{ 0xf1ff, 0x10fc, { 12,  12,   6}},	// move_8_pi_i
	{ 0xf1ff, 0x111f, { 12,  12,   9}},	// move_8_pd_pi7
	{ 0xf1ff, 0x1127, { 14,  14,  10}},	// move_8_pd_pd7
	{ 0xf1ff, 0x1138, { 16,  16,   9}},	// move_8_pd_aw
	{ 0xf1ff, 0x1139, { 20,  20,   9}},	// move_8_pd_al
	{ 0xf1ff, 0x113a, { 16,  16,  10}},	// move_8_pd_pcdi
	{ 0xf1ff, 0x113b, { 18,  18,  12}},	// move_8_pd_pcix
	{ 0xf1ff, 0x113c, { 12,  12,   7}},	// move_8_pd_i
	{ 0xf1ff, 0x115f, { 16,  16,   9}},	// move_8_di_pi7
	{ 0xf1ff, 0x1167, { 18,  18,  10}},	// move_8_di_pd7
	{ 0xf1ff, 0x1178, { 20,  20,   9}},	// move_8_di_aw
	{ 0xf1ff, 0x1179, { 24,  24,   9}},	// move_8_di_al
	{ 0xf1ff, 0x117a, { 20,  20,  10}},	// move_8_di_pcdi
	{ 0xf1ff, 0x117b, { 22,  22,  12}},	// move_8_di_pcix
	{ 0xf1ff, 0x117c, { 16,  16,   7}},	// move_8_di_i
	{ 0xf1ff, 0x119f, { 18,  18,  11}},	// move_8_ix_pi7
	{ 0xf1ff, 0x11a7, { 20,  20,  12}},	// move_8_ix_pd7
	{ 0xf1ff, 0x11b8, { 22,  22,  11}},	// move_8_ix_aw
	{ 0xf1ff, 0x11b9, { 26,  26,  11}},	// move_8_ix_al
	{ 0xf1ff, 0x11ba, { 22,  22,  12}},	// move_8_ix_pcdi
	{ 0xf1ff, 0x11bb, { 24,  24,  14}},	// move_8_ix_pcix
	{ 0xf1ff, 0x11bc, { 18,  18,   9}},	// move_8_ix_i
	{ 0xf1ff, 0x2038, { 16,  16,   6}},	// move_32_d_aw
	{ 0xf1ff, 0x2039, { 20,  20,   6}},	// move_32_d_al
	{ 0xf1ff, 0x203a, { 16,  16,   7}},	// move_32_d_pcdi
	{ 0xf1ff, 0x203b, { 18,  18,   9}},	// move_32_d_pcix
	{ 0xf1ff, 0x203c, { 12,  12,   6}},	// move_32_d_i
	{ 0xf1ff, 0x2078, { 16,  16,   6}},	// movea_32_aw
	{ 0xf1ff, 0x2079, { 20,  20,   6}},	// movea_32_al
	{ 0xf1ff, 0x207a, { 16,  16,   7}},	// movea_32_pcdi
	{ 0xf1ff, 0x207b, { 18,  18,   9}},	// movea_32_pcix
	{ 0xf1ff, 0x207c, { 12,  12,   6}},	// movea_32_i
	{ 0xf1ff, 0x20b8, { 24,  24,   8}},	// move_32_ai_aw
	{ 0xf1ff, 0x20b9, { 28,  28,   8}},	// move_32_ai_al
	{ 0xf1ff, 0x20ba, { 24,  24,   9}},	// move_32_ai_pcdi
	{ 0xf1ff, 0x20bb, { 26,  26,  11}},	// move_32_ai_pcix
	{ 0xf1ff, 0x20bc, { 20,  20,   8}},	// move_32_ai_i
	{ 0xf1ff, 0x20f8, { 24,  24,   8}},	// move_32_pi_aw
	{ 0xf1ff, 0x20f9, { 28,  28,   8}},	// move_32_pi_al
	{ 0xf1ff, 0x20fa, { 24,  24,   9}},	// move_32_pi_pcdi
	{ 0xf1ff, 0x20fb, { 26,  26,  11}},	// move_32_pi_pcix
	{ 0xf1ff, 0x20fc, { 20,  20,   8}},	// move_32_pi_i
	{ 0xf1ff, 0x2138, { 24,  26,   9}},	// move_32_pd_aw
	{ 0xf1ff, 0x2139, { 28,  30,   9}},	// move_32_pd_al
	{ 0xf1ff, 0x213a, { 24,  26,  10}},	// move_32_pd_pcdi
	{ 0xf1ff, 0x213b, { 26,  28,  12}},	// move_32_pd_pcix
	{ 0xf1ff, 0x213c, { 20,  22,   9}},	// move_32_pd_i
	{ 0xf1ff, 0x2178, { 28,  28,   9}},	// move_32_di_aw
	{ 0xf1ff, 0x2179, { 32,  32,   9}},	// move_32_di_al
	{ 0xf1ff, 0x217a, { 28,  28,  10}},	// move_32_di_pcdi
	{ 0xf1ff, 0x217b, { 30,  30,  12}},	// move_32_di_pcix
	{ 0xf1ff, 0x217c, { 24,  24,   9}},	// move_32_di_i
	{ 0xf1ff, 0x21b8, { 30,  30,  11}},	// move_32_ix_aw
	{ 0xf1ff, 0x21b9, { 34,  34,  11}},	// move_32_ix_al
	{ 0xf1ff, 0x21ba, { 30,  30,  12}},	// move_32_ix_pcdi
	{ 0xf1ff, 0x21bb, { 32,  32,  14}},	// move_32_ix_pcix
	{ 0xf1ff, 0x21bc, { 26,  26,  11}},	// move_32_ix_i
	{ 0xf1ff, 0x3038, { 12,  12,   6}},	// move_16_d_aw
	{ 0xf1ff, 0x3039, { 16,  16,   6}},	// move_16_d_al
	{ 0xf1ff, 0x303a, { 12,  12,   7}},	// move_16_d_pcdi
	{ 0xf1ff, 0x303b, { 14,  14,   9}},	// move_16_d_pcix
	{ 0xf1ff, 0x303c, {  8,   8,   4}},	// move_16_d_i
	{ 0xf1ff, 0x3078, { 12,  12,   6}},	// movea_16_aw
	{ 0xf1ff, 0x3079, { 16,  16,   6}},	// movea_16_al
	{ 0xf1ff, 0x307a, { 12,  12,   7}},	// movea_16_pcdi
	{ 0xf1ff, 0x307b, { 14,  14,   9}},	// movea_16_pcix
	{ 0xf1ff, 0x307c, {  8,   8,   4}},	// movea_16_i
	{ 0xf1ff, 0x30b8, { 16,  16,   8}},	// move_16_ai_aw
	{ 0xf1ff, 0x30b9, { 20,  20,   8}},	// move_16_ai_al
	{ 0xf1ff, 0x30ba, { 16,  16,   9}},	// move_16_ai_pcdi
	{ 0xf1ff, 0x30bb, { 18,  18,  11}},	// move_16_ai_pcix
	{ 0xf1ff, 0x30bc, { 12,  12,   6}},	// move_16_ai_i
	{ 0xf1ff, 0x30f8, { 16,  16,   8}},	// move_16_pi_aw
	{ 0xf1ff, 0x30f9, { 20,  20,   8}},	// move_16_pi_al
	{ 0xf1ff, 0x30fa, { 16,  16,   9}},	// move_16_pi_pcdi
	{ 0xf1ff, 0x30fb, { 18,  18,  11}},	// move_16_pi_pcix
	{ 0xf1ff, 0x30fc, { 12,  12,   6}},	// move_16_pi_i
	{ 0xf1ff, 0x3138, { 16,  16,   9}},	// move_16_pd_aw
	{ 0xf1ff, 0x3139, { 20,  20,   9}},	// move_16_pd_al
	{ 0xf1ff, 0x313a, { 16,  16,  10}},	// move_16_pd_pcdi
	{ 0xf1ff, 0x313b, { 18,  18,  12}},	// move_16_pd_pcix
	{ 0xf1ff, 0x313c, { 12,  12,   7}},	// move_16_pd_i
	{ 0xf1ff, 0x3178, { 20,  20,   9}},	// move_16_di_aw
	{ 0xf1ff, 0x3179, { 24,  24,   9}},	// move_16_di_al
	{ 0xf1ff, 0x317a, { 20,  20,  10}},	// move_16_di_pcdi
	{ 0xf1ff, 0x317b, { 22,  22,  12}},	// move_16_di_pcix
	{ 0xf1ff, 0x317c, { 16,  16,   7}},	// move_16_di_i
	{ 0xf1ff, 0x31b8, { 22,  22,  11}},	// move_16_ix_aw
	{ 0xf1ff, 0x31b9, { 26,  26,  11}},	// move_16_ix_al
	{ 0xf1ff, 0x31ba, { 22,  22,  12}},	// move_16_ix_pcdi
	{ 0xf1ff, 0x31bb, { 24,  24,  14}},	// move_16_ix_pcix
	{ 0xf1ff, 0x31bc, { 18,  18,   9}},	// move_16_ix_i
	{ 0xf1ff, 0x4138, {  0,   0,  12}},	// chk_32_aw
	{ 0xf1ff, 0x4139, {  0,   0,  12}},	// chk_32_al
	{ 0xf1ff, 0x413a, {  0,   0,  13}},	// chk_32_pcdi
	{ 0xf1ff, 0x413b, {  0,   0,  15}},	// chk_32_pcix
	{ 0xf1ff, 0x413c, {  0,   0,  12}},	// chk_32_i
	{ 0xf1ff, 0x41b8, { 18,  16,  12}},	// chk_16_aw
	{ 0xf1ff, 0x41b9, { 22,  20,  12}},	// chk_16_al
	{ 0xf1ff, 0x41ba, { 18,  16,  13}},	// chk_16_pcdi
	{ 0xf1ff, 0x41bb, { 20,  18,  15}},	// chk_16_pcix
	{ 0xf1ff, 0x41bc, { 14,  12,  10}},	// chk_16_i
	{ 0xf1ff, 0x41f8, {  8,   8,   6}},	// lea_32_aw
	{ 0xf1ff, 0x41f9, { 12,  12,   6}},	// lea_32_al
	{ 0xf1ff, 0x41fa, {  8,   8,   7}},	// lea_32_pcdi
	{ 0xf1ff, 0x41fb, { 12,  12,   9}},	// lea_32_pcix
	{ 0xf1ff, 0x501f, { 12,  12,   8}},	// addq_8_pi7
	{ 0xf1ff, 0x5027, { 14,  14,   9}},	// addq_8_pd7
	{ 0xf1ff, 0x5038, { 16,  16,   8}},	// addq_8_aw
	{ 0xf1ff, 0x5039, { 20,  20,   8}},	// addq_8_al
	{ 0xf1ff, 0x5078, { 16,  16,   8}},	// addq_16_aw
	{ 0xf1ff, 0x5079, { 20,  20,   8}},	// addq_16_al
	{ 0xf1ff, 0x50b8, { 24,  24,   8}},	// addq_32_aw
	{ 0xf1ff, 0x50b9, { 28,  28,   8}},	// addq_32_al
	{ 0xf1ff, 0x511f, { 12,  12,   8}},	// subq_8_pi7
	{ 0xf1ff, 0x5127, { 14,  14,   9}},	// subq_8_pd7
	{ 0xf1ff, 0x5138, { 16,  16,   8}},	// subq_8_aw
	{ 0xf1ff, 0x5139, { 20,  20,   8}},	// subq_8_al
	{ 0xf1ff, 0x5178, { 16,  16,   8}},	// subq_16_aw
	{ 0xf1ff, 0x5179, { 20,  20,   8}},	// subq_16_al
	{ 0xf1ff, 0x51b8, { 24,  24,   8}},	// subq_32_aw
	{ 0xf1ff, 0x51b9, { 28,  28,   8}},	// subq_32_al
	{ 0xf1ff, 0x801f, {  8,   8,   6}},	// or_8_er_pi7
	{ 0xf1ff, 0x8027, { 10,  10,   7}},	// or_8_er_pd7
	{ 0xf1ff, 0x8038, { 12,  12,   6}},	// or_8_er_aw
	{ 0xf1ff, 0x8039, { 16,  16,   6}},	// or_8_er_al
	{ 0xf1ff, 0x803a, { 12,  12,   7}},	// or_8_er_pcdi
	{ 0xf1ff, 0x803b, { 14,  14,   9}},	// or_8_er_pcix
	{ 0xf1ff, 0x803c, {  8,   8,   4}},	// or_8_er_i
	{ 0xf1ff, 0x8078, { 12,  12,   6}},	// or_16_er_aw
	{ 0xf1ff, 0x8079, { 16,  16,   6}},	// or_16_er_al
	{ 0xf1ff, 0x807a, { 12,  12,   7}},	// or_16_er_pcdi
	{ 0xf1ff, 0x807b, { 14,  14,   9}},	// or_16_er_pcix
	{ 0xf1ff, 0x807c, {  8,   8,   4}},	// or_16_er_i
	{ 0xf1ff, 0x80b8, { 18,  18,   6}},	// or_32_er_aw
	{ 0xf1ff, 0x80b9, { 22,  22,   6}},	// or_32_er_al
	{ 0xf1ff, 0x80ba, { 18,  18,   7}},	// or_32_er_pcdi
	{ 0xf1ff, 0x80bb, { 20,  20,   9}},	// or_32_er_pcix
	{ 0xf1ff, 0x80bc, { 16,  14,   6}},	// or_32_er_i
	{ 0xf1ff, 0x80f8, {148, 116,  48}},	// divu_16_aw
	{ 0xf1ff, 0x80f9, {152, 120,  48}},	// divu_16_al
	{ 0xf1ff, 0x80fa, {148, 116,  49}},	// divu_16_pcdi
	{ 0xf1ff, 0x80fb, {150, 118,  51}},	// divu_16_pcix
	{ 0xf1ff, 0x80fc, {144, 112,  46}},	// divu_16_i
	{ 0xf1ff, 0x810f, { 18,  18,  16}},	// sbcd_8_mm_ay7
	{ 0xf1ff, 0x811f, { 12,  12,   8}},	// or_8_re_pi7
	{ 0xf1ff, 0x8127, { 14,  14,   9}},	// or_8_re_pd7
	{ 0xf1ff, 0x8138, { 16,  16,   8}},	// or_8_re_aw
	{ 0xf1ff, 0x8139, { 20,  20,   8}},	// or_8_re_al
	{ 0xf1ff, 0x814f, {  0,   0,  13}},	// pack_16_mm_ay7
	{ 0xf1ff, 0x8178, { 16,  16,   8}},	// or_16_re_aw
	{ 0xf1ff, 0x8179, { 20,  20,   8}},	// or_16_re_al
	{ 0xf1ff, 0x818f, {  0,   0,  13}},	// unpk_16_mm_ay7
	{ 0xf1ff, 0x81b8, { 24,  24,   8}},	// or_32_re_aw
	{ 0xf1ff, 0x81b9, { 28,  28,   8}},	// or_32_re_al
	{ 0xf1ff, 0x81f8, {166, 130,  60}},	// divs_16_aw
	{ 0xf1ff, 0x81f9, {170, 134,  60}},	// divs_16_al
	{ 0xf1ff, 0x81fa, {166, 130,  61}},	// divs_16_pcdi
	{ 0xf1ff, 0x81fb, {168, 132,  63}},	// divs_16_pcix
	{ 0xf1ff, 0x81fc, {162, 126,  58}},	// divs_16_i
	{ 0xf1ff, 0x901f, {  8,   8,   6}},	// sub_8_er_pi7
	{ 0xf1ff, 0x9027, { 10,  10,   7}},	// sub_8_er_pd7
	{ 0xf1ff, 0x9038, { 12,  12,   6}},	// sub_8_er_aw
	{ 0xf1ff, 0x9039, { 16,  16,   6}},	// sub_8_er_al
	{ 0xf1ff, 0x903a, { 12,  12,   7}},	// sub_8_er_pcdi
	{ 0xf1ff, 0x903b, { 14,  14,   9}},	// sub_8_er_pcix
	{ 0xf1ff, 0x903c, {  8,   8,   4}},	// sub_8_er_i
	{ 0xf1ff, 0x9078, { 12,  12,   6}},	// sub_16_er_aw
	{ 0xf1ff, 0x9079, { 16,  16,   6}},	// sub_16_er_al
	{ 0xf1ff, 0x907a, { 12,  12,   7}},	// sub_16_er_pcdi
	{ 0xf1ff, 0x907b, { 14,  14,   9}},	// sub_16_er_pcix
	{ 0xf1ff, 0x907c, {  8,   8,   4}},	// sub_16_er_i
	{ 0xf1ff, 0x90b8, { 18,  18,   6}},	// sub_32_er_aw
	{ 0xf1ff, 0x90b9, { 22,  22,   6}},	// sub_32_er_al
	{ 0xf1ff, 0x90ba, { 18,  18,   7}},	// sub_32_er_pcdi
	{ 0xf1ff, 0x90bb, { 20,  20,   9}},	// sub_32_er_pcix
	{ 0xf1ff, 0x90bc, { 16,  14,   6}},	// sub_32_er_i
	{ 0xf1ff, 0x90f8, { 16,  16,   6}},	// suba_16_aw
	{ 0xf1ff, 0x90f9, { 20,  20,   6}},	// suba_16_al
	{ 0xf1ff, 0x90fa, { 16,  16,   7}},	// suba_16_pcdi
	{ 0xf1ff, 0x90fb, { 18,  18,   9}},	// suba_16_pcix
	{ 0xf1ff, 0x90fc, { 12,  12,   4}},	// suba_16_i
	{ 0xf1ff, 0x910f, { 18,  18,  12}},	// subx_8_mm_ay7
	{ 0xf1ff, 0x911f, { 12,  12,   8}},	// sub_8_re_pi7
	{ 0xf1ff, 0x9127, { 14,  14,   9}},	// sub_8_re_pd7
	{ 0xf1ff, 0x9138, { 16,  16,   8}},	// sub_8_re_aw
	{ 0xf1ff, 0x9139, { 20,  20,   8}},	// sub_8_re_al
	{ 0xf1ff, 0x9178, { 16,  16,   8}},	// sub_16_re_aw
	{ 0xf1ff, 0x9179, { 20,  20,   8}},	// sub_16_re_al
	{ 0xf1ff, 0x91b8, { 24,  24,   8}},	// sub_32_re_aw
	{ 0xf1ff, 0x91b9, { 28,  28,   8}},	// sub_32_re_al
	{ 0xf1ff, 0x91f8, { 18,  18,   6}},	// suba_32_aw
	{ 0xf1ff, 0x91f9, { 22,  22,   6}},	// suba_32_al
	{ 0xf1ff, 0x91fa, { 18,  18,   7}},	// suba_32_pcdi
	{ 0xf1ff, 0x91fb, { 20,  20,   9}},	// suba_32_pcix
	{ 0xf1ff, 0x91fc, { 16,  14,   6}},	// suba_32_i
	{ 0xf1ff, 0xb01f, {  8,   8,   6}},	// cmp_8_pi7
	{ 0xf1ff, 0xb027, { 10,  10,   7}},	// cmp_8_pd7
	{ 0xf1ff, 0xb038, { 12,  12,   6}},	// cmp_8_aw
	{ 0xf1ff, 0xb039, { 16,  16,   6}},	// cmp_8_al
	{ 0xf1ff, 0xb03a, { 12,  12,   7}},	// cmp_8_pcdi
	{ 0xf1ff, 0xb03b, { 14,  14,   9}},	// cmp_8_pcix
	{ 0xf1ff, 0xb03c, {  8,   8,   4}},	// cmp_8_i
	{ 0xf1ff, 0xb078, { 12,  12,   6}},	// cmp_16_aw
	{ 0xf1ff, 0xb079, { 16,  16,   6}},	// cmp_16_al
	{ 0xf1ff, 0xb07a, { 12,  12,   7}},	// cmp_16_pcdi
	{ 0xf1ff, 0xb07b, { 14,  14,   9}},	// cmp_16_pcix
	{ 0xf1ff, 0xb07c, {  8,   8,   4}},	// cmp_16_i
	{ 0xf1ff, 0xb0b8, { 18,  18,   6}},	// cmp_32_aw
	{ 0xf1ff, 0xb0b9, { 22,  22,   6}},	// cmp_32_al
	{ 0xf1ff, 0xb0ba, { 18,  18,   7}},	// cmp_32_pcdi
	{ 0xf1ff, 0xb0bb, { 20,  20,   9}},	// cmp_32_pcix
	{ 0xf1ff, 0xb0bc, { 14,  14,   6}},	// cmp_32_i
	{ 0xf1ff, 0xb0f8, { 14,  14,   8}},	// cmpa_16_aw
	{ 0xf1ff, 0xb0f9, { 18,  18,   8}},	// cmpa_16_al
	{ 0xf1ff, 0xb0fa, { 14,  14,   9}},	// cmpa_16_pcdi
	{ 0xf1ff, 0xb0fb, { 16,  16,  11}},	// cmpa_16_pcix
	{ 0xf1ff, 0xb0fc, { 10,  10,   6}},	// cmpa_16_i
	{ 0xf1ff, 0xb10f, { 12,  12,   9}},	// cmpm_8_ay7
	{ 0xf1ff, 0xb11f, { 12,  12,   8}},	// eor_8_pi7
	{ 0xf1ff, 0xb127, { 14,  14,   9}},	// eor_8_pd7
	{ 0xf1ff, 0xb138, { 16,  16,   8}},	// eor_8_aw
	{ 0xf1ff, 0xb139, { 20,  20,   8}},	// eor_8_al
	{ 0xf1ff, 0xb178, { 16,  16,   8}},	// eor_16_aw
	{ 0xf1ff, 0xb179, { 20,  20,   8}},	// eor_16_al
	{ 0xf1ff, 0xb1b8, { 24,  24,   8}},	// eor_32_aw
	{ 0xf1ff, 0xb1b9, { 28,  28,   8}},	// eor_32_al
	{ 0xf1ff, 0xb1f8, { 18,  18,   8}},	// cmpa_32_aw
	{ 0xf1ff, 0xb1f9, { 22,  22,   8}},	// cmpa_32_al
	{ 0xf1ff, 0xb1fa, { 18,  18,   9}},	// cmpa_32_pcdi
	{ 0xf1ff, 0xb1fb, { 20,  20,  11}},	// cmpa_32_pcix
	{ 0xf1ff, 0xb1fc, { 14,  14,   8}},	// cmpa_32_i
	{ 0xf1ff, 0xc01f, {  8,   8,   6}},	// and_8_er_pi7
	{ 0xf1ff, 0xc027, { 10,  10,   7}},	// and_8_er_pd7
	{ 0xf1ff, 0xc038, { 12,  12,   6}},	// and_8_er_aw
	{ 0xf1ff, 0xc039, { 16,  16,   6}},	// and_8_er_al
	{ 0xf1ff, 0xc03a, { 12,  12,   7}},	// and_8_er_pcdi
	{ 0xf1ff, 0xc03b, { 14,  14,   9}},	// and_8_er_pcix
	{ 0xf1ff, 0xc03c, {  8,   8,   4}},	// and_8_er_i
	{ 0xf1ff, 0xc078, { 12,  12,   6}},	// and_16_er_aw
	{ 0xf1ff, 0xc079, { 16,  16,   6}},	// and_16_er_al
	{ 0xf1ff, 0xc07a, { 12,  12,   7}},	// and_16_er_pcdi
	{ 0xf1ff, 0xc07b, { 14,  14,   9}},	// and_16_er_pcix
	{ 0xf1ff, 0xc07c, {  8,   8,   4}},	// and_16_er_i
	{ 0xf1ff, 0xc0b8, { 18,  18,   6}},	// and_32_er_aw
	{ 0xf1ff, 0xc0b9, { 22,  22,   6}},	// and_32_er_al
	{ 0xf1ff, 0xc0ba, { 18,  18,   7}},	// and_32_er_pcdi
	{ 0xf1ff, 0xc0bb, { 20,  20,   9}},	// and_32_er_pcix
	{ 0xf1ff, 0xc0bc, { 16,  14,   6}},	// and_32_er_i
	{ 0xf1ff, 0xc0f8, { 62,  38,  31}},	// mulu_16_aw
	{ 0xf1ff, 0xc0f9, { 66,  42,  31}},	// mulu_16_al
	{ 0xf1ff, 0xc0fa, { 62,  38,  32}},	// mulu_16_pcdi
	{ 0xf1ff, 0xc0fb, { 64,  40,  34}},	// mulu_16_pcix
	{ 0xf1ff, 0xc0fc, { 58,  34,  29}},	// mulu_16_i
	{ 0xf1ff, 0xc10f, { 18,  18,  16}},	// abcd_8_mm_ay7
	{ 0xf1ff, 0xc11f, { 12,  12,   8}},	// and_8_re_pi7
	{ 0xf1ff, 0xc127, { 14,  14,   9}},	// and_8_re_pd7
	{ 0xf1ff, 0xc138, { 16,  16,   8}},	// and_8_re_aw
	{ 0xf1ff, 0xc139, { 20,  20,   8}},	// and_8_re_al
	{ 0xf1ff, 0xc178, { 16,  16,   8}},	// and_16_re_aw
	{ 0xf1ff, 0xc179, { 20,  20,   8}},	// and_16_re_al
	{ 0xf1ff, 0xc1b8, { 24,  24,   8}},	// and_32_re_aw
	{ 0xf1ff, 0xc1b9, { 28,  28,   8}},	// and_32_re_al
	{ 0xf1ff, 0xc1f8, { 62,  40,  31}},	// muls_16_aw
	{ 0xf1ff, 0xc1f9, { 66,  44,  31}},	// muls_16_al
	{ 0xf1ff, 0xc1fa, { 62,  40,  32}},	// muls_16_pcdi
	{ 0xf1ff, 0xc1fb, { 64,  42,  34}},	// muls_16_pcix
	{ 0xf1ff, 0xc1fc, { 58,  36,  29}},	// muls_16_i
	{ 0xf1ff, 0xd01f, {  8,   8,   6}},	// add_8_er_pi7
	{ 0xf1ff, 0xd027, { 10,  10,   7}},	// add_8_er_pd7
	{ 0xf1ff, 0xd038, { 12,  12,   6}},	// add_8_er_aw
	{ 0xf1ff, 0xd039, { 16,  16,   6}},	// add_8_er_al
	{ 0xf1ff, 0xd03a, { 12,  12,   7}},	// add_8_er_pcdi
	{ 0xf1ff, 0xd03b, { 14,  14,   9}},	// add_8_er_pcix
	{ 0xf1ff, 0xd03c, {  8,   8,   4}},	// add_8_er_i
	{ 0xf1ff, 0xd078, { 12,  12,   6}},	// add_16_er_aw
	{ 0xf1ff, 0xd079, { 16,  16,   6}},	// add_16_er_al
	{ 0xf1ff, 0xd07a, { 12,  12,   7}},	// add_16_er_pcdi
	{ 0xf1ff, 0xd07b, { 14,  14,   9}},	// add_16_er_pcix
	{ 0xf1ff, 0xd07c, {  8,   8,   4}},	// add_16_er_i
	{ 0xf1ff, 0xd0b8, { 18,  18,   6}},	// add_32_er_aw
	{ 0xf1ff, 0xd0b9, { 22,  22,   6}},	// add_32_er_al
	{ 0xf1ff, 0xd0ba, { 18,  18,   7}},	// add_32_er_pcdi
	{ 0xf1ff, 0xd0bb, { 20,  20,   9}},	// add_32_er_pcix
	{ 0xf1ff, 0xd0bc, { 16,  14,   6}},	// add_32_er_i
	{ 0xf1ff, 0xd0f8, { 16,  16,   6}},	// adda_16_aw
	{ 0xf1ff, 0xd0f9, { 20,  20,   6}},	// adda_16_al
	{ 0xf1ff, 0xd0fa, { 16,  16,   7}},	// adda_16_pcdi
	{ 0xf1ff, 0xd0fb, { 18,  18,   9}},	// adda_16_pcix
	{ 0xf1ff, 0xd0fc, { 12,  12,   4}},	// adda_16_i
	{ 0xf1ff, 0xd10f, { 18,  18,  12}},	// addx_8_mm_ay7
	{ 0xf1ff, 0xd11f, { 12,  12,   8}},	// add_8_re_pi7
	{ 0xf1ff, 0xd127, { 14,  14,   9}},	// add_8_re_pd7
	{ 0xf1ff, 0xd138, { 16,  16,   8}},	// add_8_re_aw
	{ 0xf1ff, 0xd139, { 20,  20,   8}},	// add_8_re_al
	{ 0xf1ff, 0xd178, { 16,  16,   8}},	// add_16_re_aw
	{ 0xf1ff, 0xd179, { 20,  20,   8}},	// add_16_re_al
	{ 0xf1ff, 0xd1b8, { 24,  24,   8}},	// add_32_re_aw
	{ 0xf1ff, 0xd1b9, { 28,  28,   8}},	// add_32_re_al
	{ 0xf1ff, 0xd1f8, { 18,  18,   6}},	// adda_32_aw
	{ 0xf1ff, 0xd1f9, { 22,  22,   6}},	// adda_32_al
	{ 0xf1ff, 0xd1fa, { 18,  18,   7}},	// adda_32_pcdi
	{ 0xf1ff, 0xd1fb, { 20,  20,   9}},	// adda_32_pcix
	{ 0xf1ff, 0xd1fc, { 16,  14,   6}},	// adda_32_i
	{ 0xfff8, 0x0000, {  8,   8,   2}},	// ori_8_d
	{ 0xfff8, 0x0010, { 16,  16,   8}},	// ori_8_ai
	{ 0xfff8, 0x0018, { 16,  16,   8}},	// ori_8_pi
	{ 0xfff8, 0x0020, { 18,  18,   9}},	// ori_8_pd
	{ 0xfff8, 0x0028, { 20,  20,   9}},	// ori_8_di
	{ 0xfff8, 0x0030, { 22,  22,  11}},	// ori_8_ix
	{ 0xfff8, 0x0040, {  8,   8,   2}},	// ori_16_d
	{ 0xfff8, 0x0050, { 16,  16,   8}},	// ori_16_ai
	{ 0xfff8, 0x0058, { 16,  16,   8}},	// ori_16_pi
	{ 0xfff8, 0x0060, { 18,  18,   9}},	// ori_16_pd
	{ 0xfff8, 0x0068, { 20,  20,   9}},	// ori_16_di
	{ 0xfff8, 0x0070, { 22,  22,  11}},	// ori_16_ix
	{ 0xfff8, 0x0080, { 16,  14,   2}},	// ori_32_d
	{ 0xfff8, 0x0090, { 28,  28,   8}},	// ori_32_ai
	{ 0xfff8, 0x0098, { 28,  28,   8}},	// ori_32_pi
	{ 0xfff8, 0x00a0, { 30,  30,   9}},	// ori_32_pd
	{ 0xfff8, 0x00a8, { 32,  32,   9}},	// ori_32_di
	{ 0xfff8, 0x00b0, { 34,  34,  11}},	// ori_32_ix
	{ 0xfff8, 0x00d0, {  0,   0,  22}},	// chk2cmp2_8_ai
	{ 0xfff8, 0x00e8, {  0,   0,  23}},	// chk2cmp2_8_di
	{ 0xfff8, 0x00f0, {  0,   0,  25}},	// chk2cmp2_8_ix
	{ 0xfff8, 0x0200, {  8,   8,   2}},	// andi_8_d
	{ 0xfff8, 0x0210, { 16,  16,   8}},	// andi_8_ai
	{ 0xfff8, 0x0218, { 16,  16,   8}},	// andi_8_pi
	{ 0xfff8, 0x0220, { 18,  18,   9}},	// andi_8_pd
	{ 0xfff8, 0x0228, { 20,  20,   9}},	// andi_8_di
	{ 0xfff8, 0x0230, { 22,  22,  11}},	// andi_8_ix
	{ 0xfff8, 0x0240, {  8,   8,   2}},	// andi_16_d
	{ 0xfff8, 0x0250, { 16,  16,   8}},	// andi_16_ai
	{ 0xfff8, 0x0258, { 16,  16,   8}},	// andi_16_pi
	{ 0xfff8, 0x0260, { 18,  18,   9}},	// andi_16_pd
	{ 0xfff8, 0x0268, { 20,  20,   9}},	// andi_16_di
	{ 0xfff8, 0x0270, { 22,  22,  11}},	// andi_16_ix
	{ 0xfff8, 0x0280, { 14,  14,   2}},	// andi_32_d
	{ 0xfff8, 0x0290, { 28,  28,   8}},	// andi_32_ai
	{ 0xfff8, 0x0298, { 28,  28,   8}},	// andi_32_pi
	{ 0xfff8, 0x02a0, { 30,  30,   9}},	// andi_32_pd
	{ 0xfff8, 0x02a8, { 32,  32,   9}},	// andi_32_di
	{ 0xfff8, 0x02b0, { 34,  34,  11}},	// andi_32_ix
	{ 0xfff8, 0x02d0, {  0,   0,  22}},	// chk2cmp2_16_ai
	{ 0xfff8, 0x02e8, {  0,   0,  23}},	// chk2cmp2_16_di
	{ 0xfff8, 0x02f0, {  0,   0,  25}},	// chk2cmp2_16_ix
	{ 0xfff8, 0x0400, {  8,   8,   2}},	// subi_8_d
	{ 0xfff8, 0x0410, { 16,  16,   8}},	// subi_8_ai
	{ 0xfff8, 0x0418, { 16,  16,   8}},	// subi_8_pi
	{ 0xfff8, 0x0420, { 18,  18,   9}},	// subi_8_pd
	{ 0xfff8, 0x0428, { 20,  20,   9}},	// subi_8_di
	{ 0xfff8, 0x0430, { 22,  22,  11}},	// subi_8_ix
	{ 0xfff8, 0x0440, {  8,   8,   2}},	// subi_16_d
	{ 0xfff8, 0x0450, { 16,  16,   8}},	// subi_16_ai
	{ 0xfff8, 0x0458, { 16,  16,   8}},	// subi_16_pi
	{ 0xfff8, 0x0460, { 18,  18,   9}},	// subi_16_pd
	{ 0xfff8, 0x0468, { 20,  20,   9}},	// subi_16_di
	{ 0xfff8, 0x0470, { 22,  22,  11}},	// subi_16_ix
	{ 0xfff8, 0x0480, { 16,  14,   2}},	// subi_32_d
	{ 0xfff8, 0x0490, { 28,  28,   8}},	// subi_32_ai
	{ 0xfff8, 0x0498, { 28,  28,   8}},	// subi_32_pi
	{ 0xfff8, 0x04a0, { 30,  30,   9}},	// subi_32_pd
	{ 0xfff8, 0x04a8, { 32,  32,   9}},	// subi_32_di
	{ 0xfff8, 0x04b0, { 34,  34,  11}},	// subi_32_ix
	{ 0xfff8, 0x04d0, {  0,   0,  22}},	// chk2cmp2_32_ai
	{ 0xfff8, 0x04e8, {  0,   0,  23}},	// chk2cmp2_32_di
	{ 0xfff8, 0x04f0, {  0,   0,  25}},	// chk2cmp2_32_ix
	{ 0xfff8, 0x0600, {  8,   8,   2}},	// addi_8_d
	{ 0xfff8, 0x0610, { 16,  16,   8}},	// addi_8_ai
	{ 0xfff8, 0x0618, { 16,  16,   8}},	// addi_8_pi
	{ 0xfff8, 0x0620, { 18,  18,   9}},	// addi_8_pd
	{ 0xfff8, 0x0628, { 20,  20,   9}},	// addi_8_di
	{ 0xfff8, 0x0630, { 22,  22,  11}},	// addi_8_ix
	{ 0xfff8, 0x0640, {  8,   8,   2}},	// addi_16_d
	{ 0xfff8, 0x0650, { 16,  16,   8}},	// addi_16_ai
	{ 0xfff8, 0x0658, { 16,  16,   8}},	// addi_16_pi
	{ 0xfff8, 0x0660, { 18,  18,   9}},	// addi_16_pd
	{ 0xfff8, 0x0668, { 20,  20,   9}},	// addi_16_di
	{ 0xfff8, 0x0670, { 22,  22,  11}},	// addi_16_ix
	{ 0xfff8, 0x0680, { 16,  14,   2}},	// addi_32_d
	{ 0xfff8, 0x0690, { 28,  28,   8}},	// addi_32_ai
	{ 0xfff8, 0x0698, { 28,  28,   8}},	// addi_32_pi
	{ 0xfff8, 0x06a0, { 30,  30,   9}},	// addi_32_pd
	{ 0xfff8, 0x06a8, { 32,  32,   9}},	// addi_32_di
	{ 0xfff8, 0x06b0, { 34,  34,  11}},	// addi_32_ix
	{ 0xfff8, 0x06d0, {  0,   0,  64}},	// callm_32_ai
	{ 0xfff8, 0x06e8, {  0,   0,  65}},	// callm_32_di
	{ 0xfff8, 0x06f0, {  0,   0,  67}},	// callm_32_ix
	{ 0xfff8, 0x0800, { 10,  10,   4}},	// btst_32_s_d
	{ 0xfff8, 0x0810, { 12,  12,   8}},	// btst_8_s_ai
	{ 0xfff8, 0x0818, { 12,  12,   8}},	// btst_8_s_pi
	{ 0xfff8, 0x0820, { 14,  14,   9}},	// btst_8_s_pd
	{ 0xfff8, 0x0828, { 16,  16,   9}},	// btst_8_s_di
	{ 0xfff8, 0x0830, { 18,  18,  11}},	// btst_8_s_ix
	{ 0xfff8, 0x0840, { 12,  12,   4}},	// bchg_32_s_d
	{ 0xfff8, 0x0850, { 16,  16,   8}},	// bchg_8_s_ai
	{ 0xfff8, 0x0858, { 16,  16,   8}},	// bchg_8_s_pi
	{ 0xfff8, 0x0860, { 18,  18,   9}},	// bchg_8_s_pd
	{ 0xfff8, 0x0868, { 20,  20,   9}},	// bchg_8_s_di
	{ 0xfff8, 0x0870, { 22,  22,  11}},	// bchg_8_s_ix
	{ 0xfff8, 0x0880, { 14,  14,   4}},	// bclr_32_s_d
	{ 0xfff8, 0x0890, { 16,  16,   8}},	// bclr_8_s_ai
	{ 0xfff8, 0x0898, { 16,  16,   8}},	// bclr_8_s_pi
	{ 0xfff8, 0x08a0, { 18,  18,   9}},	// bclr_8_s_pd
	{ 0xfff8, 0x08a8, { 20,  20,   9}},	// bclr_8_s_di
	{ 0xfff8, 0x08b0, { 22,  22,  11}},	// bclr_8_s_ix
	{ 0xfff8, 0x08c0, { 12,  12,   4}},	// bset_32_s_d
	{ 0xfff8, 0x08d0, { 16,  16,   8}},	// bset_8_s_ai
	{ 0xfff8, 0x08d8, { 16,  16,   8}},	// bset_8_s_pi
	{ 0xfff8, 0x08e0, { 18,  18,   9}},	// bset_8_s_pd
	{ 0xfff8, 0x08e8, { 20,  20,   9}},	// bset_8_s_di
	{ 0xfff8, 0x08f0, { 22,  22,  11}},	// bset_8_s_ix
	{ 0xfff8, 0x0a00, {  8,   8,   2}},	// eori_8_d
	{ 0xfff8, 0x0a10, { 16,  16,   8}},	// eori_8_ai
	{ 0xfff8, 0x0a18, { 16,  16,   8}},	// eori_8_pi
	{ 0xfff8, 0x0a20, { 18,  18,   9}},	// eori_8_pd
	{ 0xfff8, 0x0a28, { 20,  20,   9}},	// eori_8_di
	{ 0xfff8, 0x0a30, { 22,  22,  11}},	// eori_8_ix
	{ 0xfff8, 0x0a40, {  8,   8,   2}},	// eori_16_d
	{ 0xfff8, 0x0a50, { 16,  16,   8}},	// eori_16_ai
	{ 0xfff8, 0x0a58, { 16,  16,   8}},	// eori_16_pi
	{ 0xfff8, 0x0a60, { 18,  18,   9}},	// eori_16_pd
	{ 0xfff8, 0x0a68, { 20,  20,   9}},	// eori_16_di
	{ 0xfff8, 0x0a70, { 22,  22,  11}},	// eori_16_ix
	{ 0xfff8, 0x0a80, { 16,  14,   2}},	// eori_32_d
	{ 0xfff8, 0x0a90, { 28,  28,   8}},	// eori_32_ai
	{ 0xfff8, 0x0a98, { 28,  28,   8}},	// eori_32_pi
	{ 0xfff8, 0x0aa0, { 30,  30,   9}},	// eori_32_pd
	{ 0xfff8, 0x0aa8, { 32,  32,   9}},	// eori_32_di
	{ 0xfff8, 0x0ab0, { 34,  34,  11}},	// eori_32_ix
	{ 0xfff8, 0x0ad0, {  0,   0,  16}},	// cas_8_ai
	{ 0xfff8, 0x0ad8, {  0,   0,  16}},	// cas_8_pi
	{ 0xfff8, 0x0ae0, {  0,   0,  17}},	// cas_8_pd
	{ 0xfff8, 0x0ae8, {  0,   0,  17}},	// cas_8_di
	{ 0xfff8, 0x0af0, {  0,   0,  19}},	// cas_8_ix
	{ 0xfff8, 0x0c00, {  8,   8,   2}},	// cmpi_8_d
	{ 0xfff8, 0x0c10, { 12,  12,   6}},	// cmpi_8_ai
	{ 0xfff8, 0x0c18, { 12,  12,   6}},	// cmpi_8_pi
	{ 0xfff8, 0x0c20, { 14,  14,   7}},	// cmpi_8_pd
	{ 0xfff8, 0x0c28, { 16,  16,   7}},	// cmpi_8_di
	{ 0xfff8, 0x0c30, { 18,  18,   9}},	// cmpi_8_ix
	{ 0xfff8, 0x0c40, {  8,   8,   2}},	// cmpi_16_d
	{ 0xfff8, 0x0c50, { 12,  12,   6}},	// cmpi_16_ai
	{ 0xfff8, 0x0c58, { 12,  12,   6}},	// cmpi_16_pi
	{ 0xfff8, 0x0c60, { 14,  14,   7}},	// cmpi_16_pd
	{ 0xfff8, 0x0c68, { 16,  16,   7}},	// cmpi_16_di
	{ 0xfff8, 0x0c70, { 18,  18,   9}},	// cmpi_16_ix
	{ 0xfff8, 0x0c80, { 14,  12,   2}},	// cmpi_32_d
	{ 0xfff8, 0x0c90, { 20,  20,   6}},	// cmpi_32_ai
	{ 0xfff8, 0x0c98, { 20,  20,   6}},	// cmpi_32_pi
	{ 0xfff8, 0x0ca0, { 22,  22,   7}},	// cmpi_32_pd
	{ 0xfff8, 0x0ca8, { 24,  24,   7}},	// cmpi_32_di
	{ 0xfff8, 0x0cb0, { 26,  26,   9}},	// cmpi_32_ix
	{ 0xfff8, 0x0cd0, {  0,   0,  16}},	// cas_16_ai
	{ 0xfff8, 0x0cd8, {  0,   0,  16}},	// cas_16_pi
	{ 0xfff8, 0x0ce0, {  0,   0,  17}},	// cas_16_pd
	{ 0xfff8, 0x0ce8, {  0,   0,  17}},	// cas_16_di
	{ 0xfff8, 0x0cf0, {  0,   0,  19}},	// cas_16_ix
	{ 0xfff8, 0x0e10, {  0,  18,   9}},	// moves_8_ai
	{ 0xfff8, 0x0e18, {  0,  18,   9}},	// moves_8_pi
	{ 0xfff8, 0x0e20, {  0,  20,  10}},	// moves_8_pd
	{ 0xfff8, 0x0e28, {  0,  26,  10}},	// moves_8_di
	{ 0xfff8, 0x0e30, {  0,  30,  12}},	// moves_8_ix
	{ 0xfff8, 0x0e50, {  0,  18,   9}},	// moves_16_ai
	{ 0xfff8, 0x0e58, {  0,  18,   9}},	// moves_16_pi
	{ 0xfff8, 0x0e60, {  0,  20,  10}},	// moves_16_pd
	{ 0xfff8, 0x0e68, {  0,  26,  10}},	// moves_16_di
	{ 0xfff8, 0x0e70, {  0,  30,  12}},	// moves_16_ix
	{ 0xfff8, 0x0e90, {  0,  22,   9}},	// moves_32_ai
	{ 0xfff8, 0x0e98, {  0,  22,   9}},	// moves_32_pi
	{ 0xfff8, 0x0ea0, {  0,  28,  10}},	// moves_32_pd
	{ 0xfff8, 0x0ea8, {  0,  32,  10}},	// moves_32_di
	{ 0xfff8, 0x0eb0, {  0,  36,  12}},	// moves_32_ix
	{ 0xfff8, 0x0ed0, {  0,   0,  16}},	// cas_32_ai
	{ 0xfff8, 0x0ed8, {  0,   0,  16}},	// cas_32_pi
	{ 0xfff8, 0x0ee0, {  0,   0,  17}},	// cas_32_pd
	{ 0xfff8, 0x0ee8, {  0,   0,  17}},	// cas_32_di
	{ 0xfff8, 0x0ef0, {  0,   0,  19}},	// cas_32_ix
	{ 0xfff8, 0x11c0, { 12,  12,   4}},	// move_8_aw_d
	{ 0xfff8, 0x11d0, { 16,  16,   8}},	// move_8_aw_ai
	{ 0xfff8, 0x11d8, { 16,  16,   8}},	// move_8_aw_pi
	{ 0xfff8, 0x11e0, { 18,  18,   9}},	// move_8_aw_pd
	{ 0xfff8, 0x11e8, { 20,  20,   9}},	// move_8_aw_di
	{ 0xfff8, 0x11f0, { 22,  22,  11}},	// move_8_aw_ix
	{ 0xfff8, 0x13c0, { 16,  16,   6}},	// move_8_al_d
	{ 0xfff8, 0x13d0, { 20,  20,  10}},	// move_8_al_ai
	{ 0xfff8, 0x13d8, { 20,  20,  10}},	// move_8_al_pi
	{ 0xfff8, 0x13e0, { 22,  22,  11}},	// move_8_al_pd
	{ 0xfff8, 0x13e8, { 24,  24,  11}},	// move_8_al_di
	{ 0xfff8, 0x13f0, { 26,  26,  13}},	// move_8_al_ix
	{ 0xfff8, 0x1ec0, {  8,   8,   4}},	// move_8_pi7_d
	{ 0xfff8, 0x1ed0, { 12,  12,   8}},	// move_8_pi7_ai
	{ 0xfff8, 0x1ed8, { 12,  12,   8}},	// move_8_pi7_pi
	{ 0xfff8, 0x1ee0, { 14,  14,   9}},	// move_8_pi7_pd
	{ 0xfff8, 0x1ee8, { 16,  16,   9}},	// move_8_pi7_di
	{ 0xfff8, 0x1ef0, { 18,  18,  11}},	// move_8_pi7_ix
	{ 0xfff8, 0x1f00, {  8,   8,   5}},	// move_8_pd7_d
	{ 0xfff8, 0x1f10, { 12,  12,   9}},	// move_8_pd7_ai
	{ 0xfff8, 0x1f18, { 12,  12,   9}},	// move_8_pd7_pi
	{ 0xfff8, 0x1f20, { 14,  14,  10}},	// move_8_pd7_pd
	{ 0xfff8, 0x1f28, { 16,  16,  10}},	// move_8_pd7_di
	{ 0xfff8, 0x1f30, { 18,  18,  12}},	// move_8_pd7_ix
	{ 0xfff8, 0x21c0, { 16,  16,   4}},	// move_32_aw_d
	{ 0xfff8, 0x21c8, { 16,  16,   4}},	// move_32_aw_a
	{ 0xfff8, 0x21d0, { 24,  24,   8}},	// move_32_aw_ai
	{ 0xfff8, 0x21d8, { 24,  24,   8}},	// move_32_aw_pi
	{ 0xfff8, 0x21e0, { 26,  26,   9}},	// move_32_aw_pd
	{ 0xfff8, 0x21e8, { 28,  28,   9}},	// move_32_aw_di
	{ 0xfff8, 0x21f0, { 30,  30,  11}},	// move_32_aw_ix
	{ 0xfff8, 0x23c0, { 20,  20,   6}},	// move_32_al_d
	{ 0xfff8, 0x23c8, { 20,  20,   6}},	// move_32_al_a
	{ 0xfff8, 0x23d0, { 28,  28,  10}},	// move_32_al_ai
	{ 0xfff8, 0x23d8, { 28,  28,  10}},	// move_32_al_pi
	{ 0xfff8, 0x23e0, { 30,  30,  11}},	// move_32_al_pd
	{ 0xfff8, 0x23e8, { 32,  32,  11}},	// move_32_al_di
	{ 0xfff8, 0x23f0, { 34,  34,  13}},	// move_32_al_ix
	{ 0xfff8, 0x31c0, { 12,  12,   4}},	// move_16_aw_d
	{ 0xfff8, 0x31c8, { 12,  12,   4}},	// move_16_aw_a
	{ 0xfff8, 0x31d0, { 16,  16,   8}},	// move_16_aw_ai
	{ 0xfff8, 0x31d8, { 16,  16,   8}},	// move_16_aw_pi
	{ 0xfff8, 0x31e0, { 18,  18,   9}},	// move_16_aw_pd
	{ 0xfff8, 0x31e8, { 20,  20,   9}},	// move_16_aw_di
	{ 0xfff8, 0x31f0, { 22,  22,  11}},	// move_16_aw_ix
	{ 0xfff8, 0x33c0, { 16,  16,   6}},	// move_16_al_d
	{ 0xfff8, 0x33c8, { 16,  16,   6}},	// move_16_al_a
	{ 0xfff8, 0x33d0, { 20,  20,  10}},	// move_16_al_ai
	{ 0xfff8, 0x33d8, { 20,  20,  10}},	// move_16_al_pi
	{ 0xfff8, 0x33e0, { 22,  22,  11}},	// move_16_al_pd
	{ 0xfff8, 0x33e8, { 24,  24,  11}},	// move_16_al_di
	{ 0xfff8, 0x33f0, { 26,  26,  13}},	// move_16_al_ix
	{ 0xfff8, 0x4000, {  4,   4,   2}},	// negx_8_d
	{ 0xfff8, 0x4010, { 12,  12,   8}},	// negx_8_ai
	{ 0xfff8, 0x4018, { 12,  12,   8}},	// negx_8_pi
	{ 0xfff8, 0x4020, { 14,  14,   9}},	// negx_8_pd
	{ 0xfff8, 0x4028, { 16,  16,   9}},	// negx_8_di
	{ 0xfff8, 0x4030, { 18,  18,  11}},	// negx_8_ix
	{ 0xfff8, 0x4040, {  4,   4,   2}},	// negx_16_d
	{ 0xfff8, 0x4050, { 12,  12,   8}},	// negx_16_ai
	{ 0xfff8, 0x4058, { 12,  12,   8}},	// negx_16_pi
	{ 0xfff8, 0x4060, { 14,  14,   9}},	// negx_16_pd
	{ 0xfff8, 0x4068, { 16,  16,   9}},	// negx_16_di
	{ 0xfff8, 0x4070, { 18,  18,  11}},	// negx_16_ix
	{ 0xfff8, 0x4080, {  6,   6,   2}},	// negx_32_d
	{ 0xfff8, 0x4090, { 20,  20,   8}},	// negx_32_ai
	{ 0xfff8, 0x4098, { 20,  20,   8}},	// negx_32_pi
	{ 0xfff8, 0x40a0, { 22,  22,   9}},	// negx_32_pd
	{ 0xfff8, 0x40a8, { 24,  24,   9}},	// negx_32_di
	{ 0xfff8, 0x40b0, { 26,  26,  11}},	// negx_32_ix
	{ 0xfff8, 0x40c0, {  6,   4,   8}},	// move_16_frs_d
	{ 0xfff8, 0x40d0, { 12,  12,  12}},	// move_16_frs_ai
	{ 0xfff8, 0x40d8, { 12,  12,  12}},	// move_16_frs_pi
	{ 0xfff8, 0x40e0, { 14,  14,  13}},	// move_16_frs_pd
	{ 0xfff8, 0x40e8, { 16,  16,  13}},	// move_16_frs_di
	{ 0xfff8, 0x40f0, { 18,  18,  15}},	// move_16_frs_ix
	{ 0xfff8, 0x4200, {  4,   4,   2}},	// clr_8_d
	{ 0xfff8, 0x4210, { 12,   8,   8}},	// clr_8_ai
	{ 0xfff8, 0x4218, { 12,   8,   8}},	// clr_8_pi
	{ 0xfff8, 0x4220, { 14,  10,   9}},	// clr_8_pd
	{ 0xfff8, 0x4228, { 16,  12,   9}},	// clr_8_di
	{ 0xfff8, 0x4230, { 18,  14,  11}},	// clr_8_ix
	{ 0xfff8, 0x4240, {  4,   4,   2}},	// clr_16_d
	{ 0xfff8, 0x4250, { 12,   8,   8}},	// clr_16_ai
	{ 0xfff8, 0x4258, { 12,   8,   8}},	// clr_16_pi
	{ 0xfff8, 0x4260, { 14,  10,   9}},	// clr_16_pd
	{ 0xfff8, 0x4268, { 16,  12,   9}},	// clr_16_di
	{ 0xfff8, 0x4270, { 18,  14,  11}},	// clr_16_ix
	{ 0xfff8, 0x4280, {  6,   6,   2}},	// clr_32_d
	{ 0xfff8, 0x4290, { 20,  12,   8}},	// clr_32_ai
	{ 0xfff8, 0x4298, { 20,  12,   8}},	// clr_32_pi
	{ 0xfff8, 0x42a0, { 22,  14,   9}},	// clr_32_pd
	{ 0xfff8, 0x42a8, { 24,  16,   9}},	// clr_32_di
	{ 0xfff8, 0x42b0, { 26,  20,  11}},	// clr_32_ix
	{ 0xfff8, 0x42c0, {  0,   4,   4}},	// move_16_frc_d
	{ 0xfff8, 0x42d0, {  0,  12,   8}},	// move_16_frc_ai
	{ 0xfff8, 0x42d8, {  0,  12,   8}},	// move_16_frc_pi
	{ 0xfff8, 0x42e0, {  0,  14,   9}},	// move_16_frc_pd
	{ 0xfff8, 0x42e8, {  0,  16,   9}},	// move_16_frc_di
	{ 0xfff8, 0x42f0, {  0,  18,  11}},	// move_16_frc_ix
	{ 0xfff8, 0x4400, {  4,   4,   2}},	// neg_8_d
	{ 0xfff8, 0x4410, { 12,  12,   8}},	// neg_8_ai
	{ 0xfff8, 0x4418, { 12,  12,   8}},	// neg_8_pi
	{ 0xfff8, 0x4420, { 14,  14,   9}},	// neg_8_pd
	{ 0xfff8, 0x4428, { 16,  16,   9}},	// neg_8_di
	{ 0xfff8, 0x4430, { 18,  18,  11}},	// neg_8_ix
	{ 0xfff8, 0x4440, {  4,   4,   2}},	// neg_16_d
	{ 0xfff8, 0x4450, { 12,  12,   8}},	// neg_16_ai
	{ 0xfff8, 0x4458, { 12,  12,   8}},	// neg_16_pi
	{ 0xfff8, 0x4460, { 14,  14,   9}},	// neg_16_pd
	{ 0xfff8, 0x4468, { 16,  16,   9}},	// neg_16_di
	{ 0xfff8, 0x4470, { 18,  18,  11}},	// neg_16_ix
	{ 0xfff8, 0x4480, {  6,   6,   2}},	// neg_32_d
	{ 0xfff8, 0x4490, { 20,  20,   8}},	// neg_32_ai
	{ 0xfff8, 0x4498, { 20,  20,   8}},	// neg_32_pi
	{ 0xfff8, 0x44a0, { 22,  22,   9}},	// neg_32_pd
	{ 0xfff8, 0x44a8, { 24,  24,   9}},	// neg_32_di
	{ 0xfff8, 0x44b0, { 26,  26,  11}},	// neg_32_ix
	{ 0xfff8, 0x44c0, { 12,  12,   4}},	// move_16_toc_d
	{ 0xfff8, 0x44d0, { 16,  16,   8}},	// move_16_toc_ai
	{ 0xfff8, 0x44d8, { 16,  16,   8}},	// move_16_toc_pi
	{ 0xfff8, 0x44e0, { 18,  18,   9}},	// move_16_toc_pd
	{ 0xfff8, 0x44e8, { 20,  20,   9}},	// move_16_toc_di
	{ 0xfff8, 0x44f0, { 22,  22,  11}},	// move_16_toc_ix
	{ 0xfff8, 0x4600, {  4,   4,   2}},	// not_8_d
	{ 0xfff8, 0x4610, { 12,  12,   8}},	// not_8_ai
	{ 0xfff8, 0x4618, { 12,  12,   8}},	// not_8_pi
	{ 0xfff8, 0x4620, { 14,  14,   9}},	// not_8_pd
	{ 0xfff8, 0x4628, { 16,  16,   9}},	// not_8_di
	{ 0xfff8, 0x4630, { 18,  18,  11}},	// not_8_ix
	{ 0xfff8, 0x4640, {  4,   4,   2}},	// not_16_d
	{ 0xfff8, 0x4650, { 12,  12,   8}},	// not_16_ai
	{ 0xfff8, 0x4658, { 12,  12,   8}},	// not_16_pi
	{ 0xfff8, 0x4660, { 14,  14,   9}},	// not_16_pd
	{ 0xfff8, 0x4668, { 16,  16,   9}},	// not_16_di
	{ 0xfff8, 0x4670, { 18,  18,  11}},	// not_16_ix
	{ 0xfff8, 0x4680, {  6,   6,   2}},	// not_32_d
	{ 0xfff8, 0x4690, { 20,  20,   8}},	// not_32_ai
	{ 0xfff8, 0x4698, { 20,  20,   8}},	// not_32_pi
	{ 0xfff8, 0x46a0, { 22,  22,   9}},	// not_32_pd
	{ 0xfff8, 0x46a8, { 24,  24,   9}},	// not_32_di
	{ 0xfff8, 0x46b0, { 26,  26,  11}},	// not_32_ix
	{ 0xfff8, 0x46c0, { 12,  12,   8}},	// move_16_tos_d
	{ 0xfff8, 0x46d0, { 16,  16,  12}},	// move_16_tos_ai
	{ 0xfff8, 0x46d8, { 16,  16,  12}},	// move_16_tos_pi
	{ 0xfff8, 0x46e0, { 18,  18,  13}},	// move_16_tos_pd
	{ 0xfff8, 0x46e8, { 20,  20,  13}},	// move_16_tos_di
	{ 0xfff8, 0x46f0, { 22,  22,  15}},	// move_16_tos_ix
	{ 0xfff8, 0x4800, {  6,   6,   6}},	// nbcd_8_d
	{ 0xfff8, 0x4808, {  0,   0,   6}},	// link_32
	{ 0xfff8, 0x4810, { 12,  12,  10}},	// nbcd_8_ai
	{ 0xfff8, 0x4818, { 12,  12,  10}},	// nbcd_8_pi
	{ 0xfff8, 0x4820, { 14,  14,  11}},	// nbcd_8_pd
	{ 0xfff8, 0x4828, { 16,  16,  11}},	// nbcd_8_di
	{ 0xfff8, 0x4830, { 18,  18,  13}},	// nbcd_8_ix
	{ 0xfff8, 0x4840, {  4,   4,   4}},	// swap_32
	{ 0xfff8, 0x4848, {  0,  10,  10}},	// bkpt
	{ 0xfff8, 0x4850, { 12,  12,   9}},	// pea_32_ai
	{ 0xfff8, 0x4868, { 16,  16,  10}},	// pea_32_di
	{ 0xfff8, 0x4870, { 20,  20,  12}},	// pea_32_ix
	{ 0xfff8, 0x4880, {  4,   4,   4}},	// ext_16
	{ 0xfff8, 0x4890, {  8,   8,   8}},	// movem_16_re_ai
	{ 0xfff8, 0x48a0, {  8,   8,   4}},	// movem_16_re_pd
	{ 0xfff8, 0x48a8, { 12,  12,   9}},	// movem_16_re_di
	{ 0xfff8, 0x48b0, { 14,  14,  11}},	// movem_16_re_ix
	{ 0xfff8, 0x48c0, {  4,   4,   4}},	// ext_32
	{ 0xfff8, 0x48d0, {  8,   8,   8}},	// movem_32_re_ai
	{ 0xfff8, 0x48e0, {  8,   8,   4}},	// movem_32_re_pd
	{ 0xfff8, 0x48e8, { 12,  12,   9}},	// movem_32_re_di
	{ 0xfff8, 0x48f0, { 14,  14,  11}},	// movem_32_re_ix
	{ 0xfff8, 0x49c0, {  0,   0,   4}},	// extb_32
	{ 0xfff8, 0x4a00, {  4,   4,   2}},	// tst_8_d
	{ 0xfff8, 0x4a10, {  8,   8,   6}},	// tst_8_ai
	{ 0xfff8, 0x4a18, {  8,   8,   6}},	// tst_8_pi
	{ 0xfff8, 0x4a20, { 10,  10,   7}},	// tst_8_pd
	{ 0xfff8, 0x4a28, { 12,  12,   7}},	// tst_8_di
	{ 0xfff8, 0x4a30, { 14,  14,   9}},	// tst_8_ix
	{ 0xfff8, 0x4a40, {  4,   4,   2}},	// tst_16_d
	{ 0xfff8, 0x4a48, {  0,   0,   2}},	// tst_16_a
	{ 0xfff8, 0x4a50, {  8,   8,   6}},	// tst_16_ai
	{ 0xfff8, 0x4a58, {  8,   8,   6}},	// tst_16_pi
	{ 0xfff8, 0x4a60, { 10,  10,   7}},	// tst_16_pd
	{ 0xfff8, 0x4a68, { 12,  12,   7}},	// tst_16_di
	{ 0xfff8, 0x4a70, { 14,  14,   9}},	// tst_16_ix
	{ 0xfff8, 0x4a80, {  4,   4,   2}},	// tst_32_d
	{ 0xfff8, 0x4a88, {  0,   0,   2}},	// tst_32_a
	{ 0xfff8, 0x4a90, { 12,  12,   6}},	// tst_32_ai
	{ 0xfff8, 0x4a98, { 12,  12,   6}},	// tst_32_pi
	{ 0xfff8, 0x4aa0, { 14,  14,   7}},	// tst_32_pd
	{ 0xfff8, 0x4aa8, { 16,  16,   7}},	// tst_32_di
	{ 0xfff8, 0x4ab0, { 18,  18,   9}},	// tst_32_ix
	{ 0xfff8, 0x4ac0, {  4,   4,   4}},	// tas_8_d
	{ 0xfff8, 0x4ad0, { 18,  18,  16}},	// tas_8_ai
	{ 0xfff8, 0x4ad8, { 18,  18,  16}},	// tas_8_pi
	{ 0xfff8, 0x4ae0, { 20,  20,  17}},	// tas_8_pd
	{ 0xfff8, 0x4ae8, { 22,  22,  17}},	// tas_8_di
	{ 0xfff8, 0x4af0, { 24,  24,  19}},	// tas_8_ix
	{ 0xfff8, 0x4c00, {  0,   0,  43}},	// mull_32_d
	{ 0xfff8, 0x4c10, {  0,   0,  47}},	// mull_32_ai
	{ 0xfff8, 0x4c18, {  0,   0,  47}},	// mull_32_pi
	{ 0xfff8, 0x4c20, {  0,   0,  48}},	// mull_32_pd
	{ 0xfff8, 0x4c28, {  0,   0,  48}},	// mull_32_di
	{ 0xfff8, 0x4c30, {  0,   0,  50}},	// mull_32_ix
	{ 0xfff8, 0x4c40, {  0,   0,  84}},	// divl_32_d
	{ 0xfff8, 0x4c50, {  0,   0,  88}},	// divl_32_ai
	{ 0xfff8, 0x4c58, {  0,   0,  88}},	// divl_32_pi
	{ 0xfff8, 0x4c60, {  0,   0,  89}},	// divl_32_pd
	{ 0xfff8, 0x4c68, {  0,   0,  89}},	// divl_32_di
	{ 0xfff8, 0x4c70, {  0,   0,  91}},	// divl_32_ix
	{ 0xfff8, 0x4c90, { 12,  12,  12}},	// movem_16_er_ai
	{ 0xfff8, 0x4c98, { 12,  12,   8}},	// movem_16_er_pi
	{ 0xfff8, 0x4ca8, { 16,  16,  13}},	// movem_16_er_di
	{ 0xfff8, 0x4cb0, { 18,  18,  15}},	// movem_16_er_ix
	{ 0xfff8, 0x4cd0, { 12,  12,  12}},	// movem_32_er_ai
	{ 0xfff8, 0x4cd8, { 12,  12,   8}},	// movem_32_er_pi
	{ 0xfff8, 0x4ce8, { 16,  16,  13}},	// movem_32_er_di
	{ 0xfff8, 0x4cf0, { 18,  18,  15}},	// movem_32_er_ix
	{ 0xfff8, 0x4e50, { 16,  16,   5}},	// link_16
	{ 0xfff8, 0x4e58, { 12,  12,   6}},	// unlk_32
	{ 0xfff8, 0x4e60, {  4,   6,   2}},	// move_32_tou
	{ 0xfff8, 0x4e68, {  4,   6,   2}},	// move_32_fru
	{ 0xfff8, 0x4e90, { 16,  16,   4}},	// jsr_32_ai
	{ 0xfff8, 0x4ea8, { 18,  18,   5}},	// jsr_32_di
	{ 0xfff8, 0x4eb0, { 22,  22,   7}},	// jsr_32_ix
	{ 0xfff8, 0x4ed0, {  8,   8,   4}},	// jmp_32_ai
	{ 0xfff8, 0x4ee8, { 10,  10,   5}},	// jmp_32_di
	{ 0xfff8, 0x4ef0, { 14,  14,   7}},	// jmp_32_ix
	{ 0xfff8, 0x50c0, {  6,   4,   4}},	// st_8_d
	{ 0xfff8, 0x50c8, { 12,  12,   6}},	// dbt_16
	{ 0xfff8, 0x50d0, { 12,  12,  10}},	// st_8_ai
	{ 0xfff8, 0x50d8, { 12,  12,  10}},	// st_8_pi
	{ 0xfff8, 0x50e0, { 14,  14,  11}},	// st_8_pd
	{ 0xfff8, 0x50e8, { 16,  16,  11}},	// st_8_di
	{ 0xfff8, 0x50f0, { 18,  18,  13}},	// st_8_ix
	{ 0xfff8, 0x51c0, {  4,   4,   4}},	// sf_8_d
	{ 0xfff8, 0x51c8, { 12,  12,   6}},	// dbf_16
	{ 0xfff8, 0x51d0, { 12,  12,  10}},	// sf_8_ai
	{ 0xfff8, 0x51d8, { 12,  12,  10}},	// sf_8_pi
	{ 0xfff8, 0x51e0, { 14,  14,  11}},	// sf_8_pd
	{ 0xfff8, 0x51e8, { 16,  16,  11}},	// sf_8_di
	{ 0xfff8, 0x51f0, { 18,  18,  13}},	// sf_8_ix
	{ 0xfff8, 0x52c0, {  4,   4,   4}},	// shi_8_d
	{ 0xfff8, 0x52c8, { 12,  12,   6}},	// dbhi_16
	{ 0xfff8, 0x52d0, { 12,  12,  10}},	// shi_8_ai
	{ 0xfff8, 0x52d8, { 12,  12,  10}},	// shi_8_pi
	{ 0xfff8, 0x52e0, { 14,  14,  11}},	// shi_8_pd
	{ 0xfff8, 0x52e8, { 16,  16,  11}},	// shi_8_di
	{ 0xfff8, 0x52f0, { 18,  18,  13}},	// shi_8_ix
	{ 0xfff8, 0x53c0, {  4,   4,   4}},	// sls_8_d
	{ 0xfff8, 0x53c8, { 12,  12,   6}},	// dbls_16
	{ 0xfff8, 0x53d0, { 12,  12,  10}},	// sls_8_ai
	{ 0xfff8, 0x53d8, { 12,  12,  10}},	// sls_8_pi
	{ 0xfff8, 0x53e0, { 14,  14,  11}},	// sls_8_pd
	{ 0xfff8, 0x53e8, { 16,  16,  11}},	// sls_8_di
	{ 0xfff8, 0x53f0, { 18,  18,  13}},	// sls_8_ix
	{ 0xfff8, 0x54c0, {  4,   4,   4}},	// scc_8_d
	{ 0xfff8, 0x54c8, { 12,  12,   6}},	// dbcc_16
	{ 0xfff8, 0x54d0, { 12,  12,  10}},	// scc_8_ai
	{ 0xfff8, 0x54d8, { 12,  12,  10}},	// scc_8_pi
	{ 0xfff8, 0x54e0, { 14,  14,  11}},	// scc_8_pd
	{ 0xfff8, 0x54e8, { 16,  16,  11}},	// scc_8_di
	{ 0xfff8, 0x54f0, { 18,  18,  13}},	// scc_8_ix
	{ 0xfff8, 0x55c0, {  4,   4,   4}},	// scs_8_d
	{ 0xfff8, 0x55c8, { 12,  12,   6}},	// dbcs_16
	{ 0xfff8, 0x55d0, { 12,  12,  10}},	// scs_8_ai
	{ 0xfff8, 0x55d8, { 12,  12,  10}},	// scs_8_pi
	{ 0xfff8, 0x55e0, { 14,  14,  11}},	// scs_8_pd
	{ 0xfff8, 0x55e8, { 16,  16,  11}},	// scs_8_di
	{ 0xfff8, 0x55f0, { 18,  18,  13}},	// scs_8_ix
	{ 0xfff8, 0x56c0, {  4,   4,   4}},	// sne_8_d
	{ 0xfff8, 0x56c8, { 12,  12,   6}},	// dbne_16
	{ 0xfff8, 0x56d0, { 12,  12,  10}},	// sne_8_ai
	{ 0xfff8, 0x56d8, { 12,  12,  10}},	// sne_8_pi
	{ 0xfff8, 0x56e0, { 14,  14,  11}},	// sne_8_pd
	{ 0xfff8, 0x56e8, { 16,  16,  11}},	// sne_8_di
	{ 0xfff8, 0x56f0, { 18,  18,  13}},	// sne_8_ix
	{ 0xfff8, 0x57c0, {  4,   4,   4}},	// seq_8_d
	{ 0xfff8, 0x57c8, { 12,  12,   6}},	// dbeq_16
	{ 0xfff8, 0x57d0, { 12,  12,  10}},	// seq_8_ai
	{ 0xfff8, 0x57d8, { 12,  12,  10}},	// seq_8_pi
	{ 0xfff8, 0x57e0, { 14,  14,  11}},	// seq_8_pd
	{ 0xfff8, 0x57e8, { 16,  16,  11}},	// seq_8_di
	{ 0xfff8, 0x57f0, { 18,  18,  13}},	// seq_8_ix
	{ 0xfff8, 0x58c0, {  4,   4,   4}},	// svc_8_d
	{ 0xfff8, 0x58c8, { 12,  12,   6}},	// dbvc_16
	{ 0xfff8, 0x58d0, { 12,  12,  10}},	// svc_8_ai
	{ 0xfff8, 0x58d8, { 12,  12,  10}},	// svc_8_pi
	{ 0xfff8, 0x58e0, { 14,  14,  11}},	// svc_8_pd
	{ 0xfff8, 0x58e8, { 16,  16,  11}},	// svc_8_di
	{ 0xfff8, 0x58f0, { 18,  18,  13}},	// svc_8_ix
	{ 0xfff8, 0x59c0, {  4,   4,   4}},	// svs_8_d
	{ 0xfff8, 0x59c8, { 12,  12,   6}},	// dbvs_16
	{ 0xfff8, 0x59d0, { 12,  12,  10}},	// svs_8_ai
	{ 0xfff8, 0x59d8, { 12,  12,  10}},	// svs_8_pi
	{ 0xfff8, 0x59e0, { 14,  14,  11}},	// svs_8_pd
	{ 0xfff8, 0x59e8, { 16,  16,  11}},	// svs_8_di
	{ 0xfff8, 0x59f0, { 18,  18,  13}},	// svs_8_ix
	{ 0xfff8, 0x5ac0, {  4,   4,   4}},	// spl_8_d
	{ 0xfff8, 0x5ac8, { 12,  12,   6}},	// dbpl_16
	{ 0xfff8, 0x5ad0, { 12,  12,  10}},	// spl_8_ai
	{ 0xfff8, 0x5ad8, { 12,  12,  10}},	// spl_8_pi
	{ 0xfff8, 0x5ae0, { 14,  14,  11}},	// spl_8_pd
	{ 0xfff8, 0x5ae8, { 16,  16,  11}},	// spl_8_di
	{ 0xfff8, 0x5af0, { 18,  18,  13}},	// spl_8_ix
	{ 0xfff8, 0x5bc0, {  4,   4,   4}},	// smi_8_d
	{ 0xfff8, 0x5bc8, { 12,  12,   6}},	// dbmi_16
	{ 0xfff8, 0x5bd0, { 12,  12,  10}},	// smi_8_ai
	{ 0xfff8, 0x5bd8, { 12,  12,  10}},	// smi_8_pi
	{ 0xfff8, 0x5be0, { 14,  14,  11}},	// smi_8_pd
	{ 0xfff8, 0x5be8, { 16,  16,  11}},	// smi_8_di
	{ 0xfff8, 0x5bf0, { 18,  18,  13}},	// smi_8_ix
	{ 0xfff8, 0x5cc0, {  4,   4,   4}},	// sge_8_d
	{ 0xfff8, 0x5cc8, { 12,  12,   6}},	// dbge_16
	{ 0xfff8, 0x5cd0, { 12,  12,  10}},	// sge_8_ai
	{ 0xfff8, 0x5cd8, { 12,  12,  10}},	// sge_8_pi
	{ 0xfff8, 0x5ce0, { 14,  14,  11}},	// sge_8_pd
	{ 0xfff8, 0x5ce8, { 16,  16,  11}},	// sge_8_di
	{ 0xfff8, 0x5cf0, { 18,  18,  13}},	// sge_8_ix
	{ 0xfff8, 0x5dc0, {  4,   4,   4}},	// slt_8_d
	{ 0xfff8, 0x5dc8, { 12,  12,   6}},	// dblt_16
	{ 0xfff8, 0x5dd0, { 12,  12,  10}},	// slt_8_ai
	{ 0xfff8, 0x5dd8, { 12,  12,  10}},	// slt_8_pi
	{ 0xfff8, 0x5de0, { 14,  14,  11}},	// slt_8_pd
	{ 0xfff8, 0x5de8, { 16,  16,  11}},	// slt_8_di
	{ 0xfff8, 0x5df0, { 18,  18,  13}},	// slt_8_ix
	{ 0xfff8, 0x5ec0, {  4,   4,   4}},	// sgt_8_d
	{ 0xfff8, 0x5ec8, { 12,  12,   6}},	// dbgt_16
	{ 0xfff8, 0x5ed0, { 12,  12,  10}},	// sgt_8_ai
	{ 0xfff8, 0x5ed8, { 12,  12,  10}},	// sgt_8_pi
	{ 0xfff8, 0x5ee0, { 14,  14,  11}},	// sgt_8_pd
	{ 0xfff8, 0x5ee8, { 16,  16,  11}},	// sgt_8_di
	{ 0xfff8, 0x5ef0, { 18,  18,  13}},	// sgt_8_ix
	{ 0xfff8, 0x5fc0, {  4,   4,   4}},	// sle_8_d
	{ 0xfff8, 0x5fc8, { 12,  12,   6}},	// dble_16
	{ 0xfff8, 0x5fd0, { 12,  12,  10}},	// sle_8_ai
	{ 0xfff8, 0x5fd8, { 12,  12,  10}},	// sle_8_pi
	{ 0xfff8, 0x5fe0, { 14,  14,  11}},	// sle_8_pd
	{ 0xfff8, 0x5fe8, { 16,  16,  11}},	// sle_8_di
	{ 0xfff8, 0x5ff0, { 18,  18,  13}},	// sle_8_ix
	{ 0xfff8, 0x8f08, { 18,  18,  16}},	// sbcd_8_mm_ax7
	{ 0xfff8, 0x8f48, {  0,   0,  13}},	// pack_16_mm_ax7
	{ 0xfff8, 0x8f88, {  0,   0,  13}},	// unpk_16_mm_ax7
	{ 0xfff8, 0x9f08, { 18,  18,  12}},	// subx_8_mm_ax7
	{ 0xfff8, 0xbf08, { 12,  12,   9}},	// cmpm_8_ax7
	{ 0xfff8, 0xcf08, { 18,  18,  16}},	// abcd_8_mm_ax7
	{ 0xfff8, 0xdf08, { 18,  18,  12}},	// addx_8_mm_ax7
	{ 0xfff8, 0xe0d0, { 12,  12,   9}},	// asr_16_ai
	{ 0xfff8, 0xe0d8, { 12,  12,   9}},	// asr_16_pi
	{ 0xfff8, 0xe0e0, { 14,  14,  10}},	// asr_16_pd
	{ 0xfff8, 0xe0e8, { 16,  16,  10}},	// asr_16_di
	{ 0xfff8, 0xe0f0, { 18,  18,  12}},	// asr_16_ix
	{ 0xfff8, 0xe1d0, { 12,  12,  10}},	// asl_16_ai
	{ 0xfff8, 0xe1d8, { 12,  12,  10}},	// asl_16_pi
	{ 0xfff8, 0xe1e0, { 14,  14,  11}},	// asl_16_pd
	{ 0xfff8, 0xe1e8, { 16,  16,  11}},	// asl_16_di
	{ 0xfff8, 0xe1f0, { 18,  18,  13}},	// asl_16_ix
	{ 0xfff8, 0xe2d0, { 12,  12,   9}},	// lsr_16_ai
	{ 0xfff8, 0xe2d8, { 12,  12,   9}},	// lsr_16_pi
	{ 0xfff8, 0xe2e0, { 14,  14,  10}},	// lsr_16_pd
	{ 0xfff8, 0xe2e8, { 16,  16,  10}},	// lsr_16_di
	{ 0xfff8, 0xe2f0, { 18,  18,  12}},	// lsr_16_ix
	{ 0xfff8, 0xe3d0, { 12,  12,   9}},	// lsl_16_ai
	{ 0xfff8, 0xe3d8, { 12,  12,   9}},	// lsl_16_pi
	{ 0xfff8, 0xe3e0, { 14,  14,  10}},	// lsl_16_pd
	{ 0xfff8, 0xe3e8, { 16,  16,  10}},	// lsl_16_di
	{ 0xfff8, 0xe3f0, { 18,  18,  12}},	// lsl_16_ix
	{ 0xfff8, 0xe4d0, { 12,  12,   9}},	// roxr_16_ai
	{ 0xfff8, 0xe4d8, { 12,  12,   9}},	// roxr_16_pi
	{ 0xfff8, 0xe4e0, { 14,  14,  10}},	// roxr_16_pd
	{ 0xfff8, 0xe4e8, { 16,  16,  10}},	// roxr_16_di
	{ 0xfff8, 0xe4f0, { 18,  18,  12}},	// roxr_16_ix
	{ 0xfff8, 0xe5d0, { 12,  12,   9}},	// roxl_16_ai
	{ 0xfff8, 0xe5d8, { 12,  12,   9}},	// roxl_16_pi
	{ 0xfff8, 0xe5e0, { 14,  14,  10}},	// roxl_16_pd
	{ 0xfff8, 0xe5e8, { 16,  16,  10}},	// roxl_16_di
	{ 0xfff8, 0xe5f0, { 18,  18,  12}},	// roxl_16_ix
	{ 0xfff8, 0xe6d0, { 12,  12,  11}},	// ror_16_ai
	{ 0xfff8, 0xe6d8, { 12,  12,  11}},	// ror_16_pi
	{ 0xfff8, 0xe6e0, { 14,  14,  12}},	// ror_16_pd
	{ 0xfff8, 0xe6e8, { 16,  16,  12}},	// ror_16_di
	{ 0xfff8, 0xe6f0, { 18,  18,  14}},	// ror_16_ix
	{ 0xfff8, 0xe7d0, { 12,  12,  11}},	// rol_16_ai
	{ 0xfff8, 0xe7d8, { 12,  12,  11}},	// rol_16_pi
	{ 0xfff8, 0xe7e0, { 14,  14,  12}},	// rol_16_pd
	{ 0xfff8, 0xe7e8, { 16,  16,  12}},	// rol_16_di
	{ 0xfff8, 0xe7f0, { 18,  18,  14}},	// rol_16_ix
	{ 0xfff8, 0xe8c0, {  0,   0,   6}},	// bftst_32_d
	{ 0xfff8, 0xe8d0, {  0,   0,  17}},	// bftst_32_ai
	{ 0xfff8, 0xe8e8, {  0,   0,  18}},	// bftst_32_di
	{ 0xfff8, 0xe8f0, {  0,   0,  20}},	// bftst_32_ix
	{ 0xfff8, 0xe9c0, {  0,   0,   8}},	// bfextu_32_d
	{ 0xfff8, 0xe9d0, {  0,   0,  19}},	// bfextu_32_ai
	{ 0xfff8, 0xe9e8, {  0,   0,  20}},	// bfextu_32_di
	{ 0xfff8, 0xe9f0, {  0,   0,  22}},	// bfextu_32_ix
	{ 0xfff8, 0xeac0, {  0,   0,  12}},	// bfchg_32_d
	{ 0xfff8, 0xead0, {  0,   0,  24}},	// bfchg_32_ai
	{ 0xfff8, 0xeae8, {  0,   0,  25}},	// bfchg_32_di
	{ 0xfff8, 0xeaf0, {  0,   0,  27}},	// bfchg_32_ix
	{ 0xfff8, 0xebc0, {  0,   0,   8}},	// bfexts_32_d
	{ 0xfff8, 0xebd0, {  0,   0,  19}},	// bfexts_32_ai
	{ 0xfff8, 0xebe8, {  0,   0,  20}},	// bfexts_32_di
	{ 0xfff8, 0xebf0, {  0,   0,  22}},	// bfexts_32_ix
	{ 0xfff8, 0xecc0, {  0,   0,  12}},	// bfclr_32_d
	{ 0xfff8, 0xecd0, {  0,   0,  24}},	// bfclr_32_ai
	{ 0xfff8, 0xece8, {  0,   0,  25}},	// bfclr_32_di
	{ 0xfff8, 0xecf0, {  0,   0,  27}},	// bfclr_32_ix
	{ 0xfff8, 0xedc0, {  0,   0,  18}},	// bfffo_32_d
	{ 0xfff8, 0xedd0, {  0,   0,  32}},	// bfffo_32_ai
	{ 0xfff8, 0xede8, {  0,   0,  33}},	// bfffo_32_di
	{ 0xfff8, 0xedf0, {  0,   0,  35}},	// bfffo_32_ix
	{ 0xfff8, 0xeec0, {  0,   0,  12}},	// bfset_32_d
	{ 0xfff8, 0xeed0, {  0,   0,  24}},	// bfset_32_ai
	{ 0xfff8, 0xeee8, {  0,   0,  25}},	// bfset_32_di
	{ 0xfff8, 0xeef0, {  0,   0,  27}},	// bfset_32_ix
	{ 0xfff8, 0xefc0, {  0,   0,  10}},	// bfins_32_d
	{ 0xfff8, 0xefd0, {  0,   0,  21}},	// bfins_32_ai
	{ 0xfff8, 0xefe8, {  0,   0,  22}},	// bfins_32_di
	{ 0xfff8, 0xeff0, {  0,   0,  24}},	// bfins_32_ix
	{ 0xffff, 0x001f, { 16,  16,   8}},	// ori_8_pi7
	{ 0xffff, 0x0027, { 18,  18,   9}},	// ori_8_pd7
	{ 0xffff, 0x0038, { 20,  20,   8}},	// ori_8_aw
	{ 0xffff, 0x0039, { 24,  24,   8}},	// ori_8_al
	{ 0xffff, 0x003c, { 20,  16,  12}},	// ori_16_toc
	{ 0xffff, 0x0078, { 20,  20,   8}},	// ori_16_aw
	{ 0xffff, 0x0079, { 24,  24,   8}},	// ori_16_al
	{ 0xffff, 0x007c, { 20,  16,  12}},	// ori_16_tos
	{ 0xffff, 0x00b8, { 32,  32,   8}},	// ori_32_aw
	{ 0xffff, 0x00b9, { 36,  36,   8}},	// ori_32_al
	{ 0xffff, 0x00f8, {  0,   0,  22}},	// chk2cmp2_8_aw
	{ 0xffff, 0x00f9, {  0,   0,  22}},	// chk2cmp2_8_al
	{ 0xffff, 0x00fa, {  0,   0,  23}},	// chk2cmp2_8_pcdi
	{ 0xffff, 0x00fb, {  0,   0,  23}},	// chk2cmp2_8_pcix
	{ 0xffff, 0x021f, { 16,  16,   8}},	// andi_8_pi7
	{ 0xffff, 0x0227, { 18,  18,   9}},	// andi_8_pd7
	{ 0xffff, 0x0238, { 20,  20,   8}},	// andi_8_aw
	{ 0xffff, 0x0239, { 24,  24,   8}},	// andi_8_al
	{ 0xffff, 0x023c, { 20,  16,  12}},	// andi_16_toc
	{ 0xffff, 0x0278, { 20,  20,   8}},	// andi_16_aw
	{ 0xffff, 0x0279, { 24,  24,   8}},	// andi_16_al
	{ 0xffff, 0x027c, { 20,  16,  12}},	// andi_16_tos
	{ 0xffff, 0x02b8, { 32,  32,   8}},	// andi_32_aw
	{ 0xffff, 0x02b9, { 36,  36,   8}},	// andi_32_al
	{ 0xffff, 0x02f8, {  0,   0,  22}},	// chk2cmp2_16_aw
	{ 0xffff, 0x02f9, {  0,   0,  22}},	// chk2cmp2_16_al
	{ 0xffff, 0x02fa, {  0,   0,  23}},	// chk2cmp2_16_pcdi
	{ 0xffff, 0x02fb, {  0,   0,  23}},	// chk2cmp2_16_pcix
	{ 0xffff, 0x041f, { 16,  16,   8}},	// subi_8_pi7
	{ 0xffff, 0x0427, { 18,  18,   9}},	// subi_8_pd7
	{ 0xffff, 0x0438, { 20,  20,   8}},	// subi_8_aw
	{ 0xffff, 0x0439, { 24,  24,   8}},	// subi_8_al
	{ 0xffff, 0x0478, { 20,  20,   8}},	// subi_16_aw
	{ 0xffff, 0x0479, { 24,  24,   8}},	// subi_16_al
	{ 0xffff, 0x04b8, { 32,  32,   8}},	// subi_32_aw
	{ 0xffff, 0x04b9, { 36,  36,   8}},	// subi_32_al
	{ 0xffff, 0x04f8, {  0,   0,  22}},	// chk2cmp2_32_aw
	{ 0xffff, 0x04f9, {  0,   0,  22}},	// chk2cmp2_32_al
	{ 0xffff, 0x04fa, {  0,   0,  23}},	// chk2cmp2_32_pcdi
	{ 0xffff, 0x04fb, {  0,   0,  23}},	// chk2cmp2_32_pcix
	{ 0xffff, 0x061f, { 16,  16,   8}},	// addi_8_pi7
	{ 0xffff, 0x0627, { 18,  18,   9}},	// addi_8_pd7
	{ 0xffff, 0x0638, { 20,  20,   8}},	// addi_8_aw
	{ 0xffff, 0x0639, { 24,  24,   8}},	// addi_8_al
	{ 0xffff, 0x0678, { 20,  20,   8}},	// addi_16_aw
	{ 0xffff, 0x0679, { 24,  24,   8}},	// addi_16_al
	{ 0xffff, 0x06b8, { 32,  32,   8}},	// addi_32_aw
	{ 0xffff, 0x06b9, { 36,  36,   8}},	// addi_32_al
	{ 0xffff, 0x06f8, {  0,   0,  64}},	// callm_32_aw
	{ 0xffff, 0x06f9, {  0,   0,  64}},	// callm_32_al
	{ 0xffff, 0x06fa, {  0,   0,  65}},	// callm_32_pcdi
	{ 0xffff, 0x06fb, {  0,   0,  67}},	// callm_32_pcix
	{ 0xffff, 0x081f, { 12,  12,   8}},	// btst_8_s_pi7
	{ 0xffff, 0x0827, { 14,  14,   9}},	// btst_8_s_pd7
	{ 0xffff, 0x0838, { 16,  16,   8}},	// btst_8_s_aw
	{ 0xffff, 0x0839, { 20,  20,   8}},	// btst_8_s_al
	{ 0xffff, 0x083a, { 16,  16,   9}},	// btst_8_s_pcdi
	{ 0xffff, 0x083b, { 18,  18,  11}},	// btst_8_s_pcix
	{ 0xffff, 0x085f, { 16,  16,   8}},	// bchg_8_s_pi7
	{ 0xffff, 0x0867, { 18,  18,   9}},	// bchg_8_s_pd7
	{ 0xffff, 0x0878, { 20,  20,   8}},	// bchg_8_s_aw
	{ 0xffff, 0x0879, { 24,  24,   8}},	// bchg_8_s_al
	{ 0xffff, 0x089f, { 16,  16,   8}},	// bclr_8_s_pi7
	{ 0xffff, 0x08a7, { 18,  18,   9}},	// bclr_8_s_pd7
	{ 0xffff, 0x08b8, { 20,  20,   8}},	// bclr_8_s_aw
	{ 0xffff, 0x08b9, { 24,  24,   8}},	// bclr_8_s_al
	{ 0xffff, 0x08df, { 16,  16,   8}},	// bset_8_s_pi7
	{ 0xffff, 0x08e7, { 18,  18,   9}},	// bset_8_s_pd7
	{ 0xffff, 0x08f8, { 20,  20,   8}},	// bset_8_s_aw
	{ 0xffff, 0x08f9, { 24,  24,   8}},	// bset_8_s_al
	{ 0xffff, 0x0a1f, { 16,  16,   8}},	// eori_8_pi7
	{ 0xffff, 0x0a27, { 18,  18,   9}},	// eori_8_pd7
	{ 0xffff, 0x0a38, { 20,  20,   8}},	// eori_8_aw
	{ 0xffff, 0x0a39, { 24,  24,   8}},	// eori_8_al
	{ 0xffff, 0x0a3c, { 20,  16,  12}},	// eori_16_toc
	{ 0xffff, 0x0a78, { 20,  20,   8}},	// eori_16_aw
	{ 0xffff, 0x0a79, { 24,  24,   8}},	// eori_16_al
	{ 0xffff, 0x0a7c, { 20,  16,  12}},	// eori_16_tos
	{ 0xffff, 0x0ab8, { 32,  32,   8}},	// eori_32_aw
	{ 0xffff, 0x0ab9, { 36,  36,   8}},	// eori_32_al
	{ 0xffff, 0x0adf, {  0,   0,  16}},	// cas_8_pi7
	{ 0xffff, 0x0ae7, {  0,   0,  17}},	// cas_8_pd7
	{ 0xffff, 0x0af8, {  0,   0,  16}},	// cas_8_aw
	{ 0xffff, 0x0af9, {  0,   0,  16}},	// cas_8_al
	{ 0xffff, 0x0c1f, { 12,  12,   6}},	// cmpi_8_pi7
	{ 0xffff, 0x0c27, { 14,  14,   7}},	// cmpi_8_pd7
	{ 0xffff, 0x0c38, { 16,  16,   6}},	// cmpi_8_aw
	{ 0xffff, 0x0c39, { 20,  20,   6}},	// cmpi_8_al
	{ 0xffff, 0x0c3a, {  0,   0,   7}},	// cmpi_8_pcdi
	{ 0xffff, 0x0c3b, {  0,   0,   9}},	// cmpi_8_pcix
	{ 0xffff, 0x0c78, { 16,  16,   6}},	// cmpi_16_aw
	{ 0xffff, 0x0c79, { 20,  20,   6}},	// cmpi_16_al
	{ 0xffff, 0x0c7a, {  0,   0,   7}},	// cmpi_16_pcdi
	{ 0xffff, 0x0c7b, {  0,   0,   9}},	// cmpi_16_pcix
	{ 0xffff, 0x0cb8, { 24,  24,   6}},	// cmpi_32_aw
	{ 0xffff, 0x0cb9, { 28,  28,   6}},	// cmpi_32_al
	{ 0xffff, 0x0cba, {  0,   0,   7}},	// cmpi_32_pcdi
	{ 0xffff, 0x0cbb, {  0,   0,   9}},	// cmpi_32_pcix
	{ 0xffff, 0x0cf8, {  0,   0,  16}},	// cas_16_aw
	{ 0xffff, 0x0cf9, {  0,   0,  16}},	// cas_16_al
	{ 0xffff, 0x0cfc, {  0,   0,  12}},	// cas2_16
	{ 0xffff, 0x0e1f, {  0,  18,   9}},	// moves_8_pi7
	{ 0xffff, 0x0e27, {  0,  20,  10}},	// moves_8_pd7
	{ 0xffff, 0x0e38, {  0,  26,   9}},	// moves_8_aw
	{ 0xffff, 0x0e39, {  0,  30,   9}},	// moves_8_al
	{ 0xffff, 0x0e78, {  0,  26,   9}},	// moves_16_aw
	{ 0xffff, 0x0e79, {  0,  30,   9}},	// moves_16_al
	{ 0xffff, 0x0eb8, {  0,  32,   9}},	// moves_32_aw
	{ 0xffff, 0x0eb9, {  0,  36,   9}},	// moves_32_al
	{ 0xffff, 0x0ef8, {  0,   0,  16}},	// cas_32_aw
	{ 0xffff, 0x0ef9, {  0,   0,  16}},	// cas_32_al
	{ 0xffff, 0x0efc, {  0,   0,  12}},	// cas2_32
	{ 0xffff, 0x11df, { 16,  16,   8}},	// move_8_aw_pi7
	{ 0xffff, 0x11e7, { 18,  18,   9}},	// move_8_aw_pd7
	{ 0xffff, 0x11f8, { 20,  20,   8}},	// move_8_aw_aw
	{ 0xffff, 0x11f9, { 24,  24,   8}},	// move_8_aw_al
	{ 0xffff, 0x11fa, { 20,  20,   9}},	// move_8_aw_pcdi
	{ 0xffff, 0x11fb, { 22,  22,  11}},	// move_8_aw_pcix
	{ 0xffff, 0x11fc, { 16,  16,   6}},	// move_8_aw_i
	{ 0xffff, 0x13df, { 20,  20,  10}},	// move_8_al_pi7
	{ 0xffff, 0x13e7, { 22,  22,  11}},	// move_8_al_pd7
	{ 0xffff, 0x13f8, { 24,  24,  10}},	// move_8_al_aw
	{ 0xffff, 0x13f9, { 28,  28,  10}},	// move_8_al_al
	{ 0xffff, 0x13fa, { 24,  24,  11}},	// move_8_al_pcdi
	{ 0xffff, 0x13fb, { 26,  26,  13}},	// move_8_al_pcix
	{ 0xffff, 0x13fc, { 20,  20,   8}},	// move_8_al_i
	{ 0xffff, 0x1edf, { 12,  12,   8}},	// move_8_pi7_pi7
	{ 0xffff, 0x1ee7, { 14,  14,   9}},	// move_8_pi7_pd7
	{ 0xffff, 0x1ef8, { 16,  16,   8}},	// move_8_pi7_aw
	{ 0xffff, 0x1ef9, { 20,  20,   8}},	// move_8_pi7_al
	{ 0xffff, 0x1efa, { 16,  16,   9}},	// move_8_pi7_pcdi
	{ 0xffff, 0x1efb, { 18,  18,  11}},	// move_8_pi7_pcix
	{ 0xffff, 0x1efc, { 12,  12,   6}},	// move_8_pi7_i
	{ 0xffff, 0x1f1f, { 12,  12,   9}},	// move_8_pd7_pi7
	{ 0xffff, 0x1f27, { 14,  14,  10}},	// move_8_pd7_pd7
	{ 0xffff, 0x1f38, { 16,  16,   9}},	// move_8_pd7_aw
	{ 0xffff, 0x1f39, { 20,  20,   9}},	// move_8_pd7_al
	{ 0xffff, 0x1f3a, { 16,  16,  10}},	// move_8_pd7_pcdi
	{ 0xffff, 0x1f3b, { 18,  18,  12}},	// move_8_pd7_pcix
	{ 0xffff, 0x1f3c, { 12,  12,   7}},	// move_8_pd7_i
	{ 0xffff, 0x21f8, { 28,  28,   8}},	// move_32_aw_aw
	{ 0xffff, 0x21f9, { 32,  32,   8}},	// move_32_aw_al
	{ 0xffff, 0x21fa, { 28,  28,   9}},	// move_32_aw_pcdi
	{ 0xffff, 0x21fb, { 30,  30,  11}},	// move_32_aw_pcix
	{ 0xffff, 0x21fc, { 24,  24,   8}},	// move_32_aw_i
	{ 0xffff, 0x23f8, { 32,  32,  10}},	// move_32_al_aw
	{ 0xffff, 0x23f9, { 36,  36,  10}},	// move_32_al_al
	{ 0xffff, 0x23fa, { 32,  32,  11}},	// move_32_al_pcdi
	{ 0xffff, 0x23fb, { 34,  34,  13}},	// move_32_al_pcix
	{ 0xffff, 0x23fc, { 28,  28,  10}},	// move_32_al_i
	{ 0xffff, 0x31f8, { 20,  20,   8}},	// move_16_aw_aw
	{ 0xffff, 0x31f9, { 24,  24,   8}},	// move_16_aw_al
	{ 0xffff, 0x31fa, { 20,  20,   9}},	// move_16_aw_pcdi
	{ 0xffff, 0x31fb, { 22,  22,  11}},	// move_16_aw_pcix
	{ 0xffff, 0x31fc, { 16,  16,   6}},	// move_16_aw_i
	{ 0xffff, 0x33f8, { 24,  24,  10}},	// move_16_al_aw
	{ 0xffff, 0x33f9, { 28,  28,  10}},	// move_16_al_al
	{ 0xffff, 0x33fa, { 24,  24,  11}},	// move_16_al_pcdi
	{ 0xffff, 0x33fb, { 26,  26,  13}},	// move_16_al_pcix
	{ 0xffff, 0x33fc, { 20,  20,   8}},	// move_16_al_i
	{ 0xffff, 0x401f, { 12,  12,   8}},	// negx_8_pi7
	{ 0xffff, 0x4027, { 14,  14,   9}},	// negx_8_pd7
	{ 0xffff, 0x4038, { 16,  16,   8}},	// negx_8_aw
	{ 0xffff, 0x4039, { 20,  20,   8}},	// negx_8_al
	{ 0xffff, 0x4078, { 16,  16,   8}},	// negx_16_aw
	{ 0xffff, 0x4079, { 20,  20,   8}},	// negx_16_al
	{ 0xffff, 0x40b8, { 24,  24,   8}},	// negx_32_aw
	{ 0xffff, 0x40b9, { 28,  28,   8}},	// negx_32_al
	{ 0xffff, 0x40f8, { 16,  16,  12}},	// move_16_frs_aw
	{ 0xffff, 0x40f9, { 20,  20,  12}},	// move_16_frs_al
	{ 0xffff, 0x421f, { 12,   8,   8}},	// clr_8_pi7
	{ 0xffff, 0x4227, { 14,  10,   9}},	// clr_8_pd7
	{ 0xffff, 0x4238, { 16,  12,   8}},	// clr_8_aw
	{ 0xffff, 0x4239, { 20,  14,   8}},	// clr_8_al
	{ 0xffff, 0x4278, { 16,  12,   8}},	// clr_16_aw
	{ 0xffff, 0x4279, { 20,  14,   8}},	// clr_16_al
	{ 0xffff, 0x42b8, { 24,  16,   8}},	// clr_32_aw
	{ 0xffff, 0x42b9, { 28,  20,   8}},	// clr_32_al
	{ 0xffff, 0x42f8, {  0,  16,   8}},	// move_16_frc_aw
	{ 0xffff, 0x42f9, {  0,  20,   8}},	// move_16_frc_al
	{ 0xffff, 0x441f, { 12,  12,   8}},	// neg_8_pi7
	{ 0xffff, 0x4427, { 14,  14,   9}},	// neg_8_pd7
	{ 0xffff, 0x4438, { 16,  16,   8}},	// neg_8_aw
	{ 0xffff, 0x4439, { 20,  20,   8}},	// neg_8_al
	{ 0xffff, 0x4478, { 16,  16,   8}},	// neg_16_aw
	{ 0xffff, 0x4479, { 20,  20,   8}},	// neg_16_al
	{ 0xffff, 0x44b8, { 24,  24,   8}},	// neg_32_aw
	{ 0xffff, 0x44b9, { 28,  28,   8}},	// neg_32_al
	{ 0xffff, 0x44f8, { 20,  20,   8}},	// move_16_toc_aw
	{ 0xffff, 0x44f9, { 24,  24,   8}},	// move_16_toc_al
	{ 0xffff, 0x44fa, { 20,  20,   9}},	// move_16_toc_pcdi
	{ 0xffff, 0x44fb, { 22,  22,  11}},	// move_16_toc_pcix
	{ 0xffff, 0x44fc, { 16,  16,   6}},	// move_16_toc_i
	{ 0xffff, 0x461f, { 12,  12,   8}},	// not_8_pi7
	{ 0xffff, 0x4627, { 14,  14,   9}},	// not_8_pd7
	{ 0xffff, 0x4638, { 16,  16,   8}},	// not_8_aw
	{ 0xffff, 0x4639, { 20,  20,   8}},	// not_8_al
	{ 0xffff, 0x4678, { 16,  16,   8}},	// not_16_aw
	{ 0xffff, 0x4679, { 20,  20,   8}},	// not_16_al
	{ 0xffff, 0x46b8, { 24,  24,   8}},	// not_32_aw
	{ 0xffff, 0x46b9, { 28,  28,   8}},	// not_32_al
	{ 0xffff, 0x46f8, { 20,  20,  12}},	// move_16_tos_aw
	{ 0xffff, 0x46f9, { 24,  24,  12}},	// move_16_tos_al
	{ 0xffff, 0x46fa, { 20,  20,  13}},	// move_16_tos_pcdi
	{ 0xffff, 0x46fb, { 22,  22,  15}},	// move_16_tos_pcix
	{ 0xffff, 0x46fc, { 16,  16,  10}},	// move_16_tos_i
	{ 0xffff, 0x480f, {  0,   0,   6}},	// link_32_a7
	{ 0xffff, 0x481f, { 12,  12,  10}},	// nbcd_8_pi7
	{ 0xffff, 0x4827, { 14,  14,  11}},	// nbcd_8_pd7
	{ 0xffff, 0x4838, { 16,  16,  10}},	// nbcd_8_aw
	{ 0xffff, 0x4839, { 20,  20,  10}},	// nbcd_8_al
	{ 0xffff, 0x4878, { 16,  16,   9}},	// pea_32_aw
	{ 0xffff, 0x4879, { 20,  20,   9}},	// pea_32_al
	{ 0xffff, 0x487a, { 16,  16,  10}},	// pea_32_pcdi
	{ 0xffff, 0x487b, { 20,  20,  12}},	// pea_32_pcix
	{ 0xffff, 0x48b8, { 12,  12,   8}},	// movem_16_re_aw
	{ 0xffff, 0x48b9, { 16,  16,   8}},	// movem_16_re_al
	{ 0xffff, 0x48f8, { 12,  12,   8}},	// movem_32_re_aw
	{ 0xffff, 0x48f9, { 16,  16,   8}},	// movem_32_re_al
	{ 0xffff, 0x4a1f, {  8,   8,   6}},	// tst_8_pi7
	{ 0xffff, 0x4a27, { 10,  10,   7}},	// tst_8_pd7
	{ 0xffff, 0x4a38, { 12,  12,   6}},	// tst_8_aw
	{ 0xffff, 0x4a39, { 16,  16,   6}},	// tst_8_al
	{ 0xffff, 0x4a3a, {  0,   0,   7}},	// tst_8_pcdi
	{ 0xffff, 0x4a3b, {  0,   0,   9}},	// tst_8_pcix
	{ 0xffff, 0x4a3c, {  0,   0,   6}},	// tst_8_i
	{ 0xffff, 0x4a78, { 12,  12,   6}},	// tst_16_aw
	{ 0xffff, 0x4a79, { 16,  16,   6}},	// tst_16_al
	{ 0xffff, 0x4a7a, {  0,   0,   7}},	// tst_16_pcdi
	{ 0xffff, 0x4a7b, {  0,   0,   9}},	// tst_16_pcix
	{ 0xffff, 0x4a7c, {  0,   0,   6}},	// tst_16_i
	{ 0xffff, 0x4ab8, { 16,  16,   6}},	// tst_32_aw
	{ 0xffff, 0x4ab9, { 20,  20,   6}},	// tst_32_al
	{ 0xffff, 0x4aba, {  0,   0,   7}},	// tst_32_pcdi
	{ 0xffff, 0x4abb, {  0,   0,   9}},	// tst_32_pcix
	{ 0xffff, 0x4abc, {  0,   0,   6}},	// tst_32_i
	{ 0xffff, 0x4adf, { 18,  18,  16}},	// tas_8_pi7
	{ 0xffff, 0x4ae7, { 20,  20,  17}},	// tas_8_pd7
	{ 0xffff, 0x4af8, { 22,  22,  16}},	// tas_8_aw
	{ 0xffff, 0x4af9, { 26,  26,  16}},	// tas_8_al
	{ 0xffff, 0x4afc, {  4,   4,   4}},	// illegal
	{ 0xffff, 0x4c38, {  0,   0,  47}},	// mull_32_aw
	{ 0xffff, 0x4c39, {  0,   0,  47}},	// mull_32_al
	{ 0xffff, 0x4c3a, {  0,   0,  48}},	// mull_32_pcdi
	{ 0xffff, 0x4c3b, {  0,   0,  50}},	// mull_32_pcix
	{ 0xffff, 0x4c3c, {  0,   0,  47}},	// mull_32_i
	{ 0xffff, 0x4c78, {  0,   0,  88}},	// divl_32_aw
	{ 0xffff, 0x4c79, {  0,   0,  88}},	// divl_32_al
	{ 0xffff, 0x4c7a, {  0,   0,  89}},	// divl_32_pcdi
	{ 0xffff, 0x4c7b, {  0,   0,  91}},	// divl_32_pcix
	{ 0xffff, 0x4c7c, {  0,   0,  88}},	// divl_32_i
	{ 0xffff, 0x4cb8, { 16,  16,  12}},	// movem_16_er_aw
	{ 0xffff, 0x4cb9, { 20,  20,  12}},	// movem_16_er_al
	{ 0xffff, 0x4cba, { 16,  16,   9}},	// movem_16_er_pcdi
	{ 0xffff, 0x4cbb, { 18,  18,  11}},	// movem_16_er_pcix
	{ 0xffff, 0x4cf8, { 16,  16,  12}},	// movem_32_er_aw
	{ 0xffff, 0x4cf9, { 20,  20,  12}},	// movem_32_er_al
	{ 0xffff, 0x4cfa, { 16,  16,   9}},	// movem_32_er_pcdi
	{ 0xffff, 0x4cfb, { 18,  18,  11}},	// movem_32_er_pcix
	{ 0xffff, 0x4e57, { 16,  16,   5}},	// link_16_a7
	{ 0xffff, 0x4e5f, { 12,  12,   6}},	// unlk_32_a7
	{ 0xffff, 0x4e70, {132, 130, 518}},	// reset
	{ 0xffff, 0x4e71, {  4,   4,   2}},	// nop
	{ 0xffff, 0x4e72, {  4,   4,   8}},	// stop
	{ 0xffff, 0x4e73, { 20,  24,  20}},	// rte_32
	{ 0xffff, 0x4e74, {  0,  16,  10}},	// rtd_32
	{ 0xffff, 0x4e75, { 16,  16,  10}},	// rts_32
	{ 0xffff, 0x4e76, {  4,   4,   4}},	// trapv
	{ 0xffff, 0x4e77, { 20,  20,  14}},	// rtr_32
	{ 0xffff, 0x4e7a, {  0,  12,   6}},	// movec_32_cr
	{ 0xffff, 0x4e7b, {  0,  10,  12}},	// movec_32_rc
	{ 0xffff, 0x4eb8, { 18,  18,   4}},	// jsr_32_aw
	{ 0xffff, 0x4eb9, { 20,  20,   4}},	// jsr_32_al
	{ 0xffff, 0x4eba, { 18,  18,   5}},	// jsr_32_pcdi
	{ 0xffff, 0x4ebb, { 22,  22,   7}},	// jsr_32_pcix
	{ 0xffff, 0x4ef8, { 10,  10,   4}},	// jmp_32_aw
	{ 0xffff, 0x4ef9, { 12,  12,   4}},	// jmp_32_al
	{ 0xffff, 0x4efa, { 10,  10,   5}},	// jmp_32_pcdi
	{ 0xffff, 0x4efb, { 14,  14,   7}},	// jmp_32_pcix
	{ 0xffff, 0x50df, { 12,  12,  10}},	// st_8_pi7
	{ 0xffff, 0x50e7, { 14,  14,  11}},	// st_8_pd7
	{ 0xffff, 0x50f8, { 16,  16,  10}},	// st_8_aw
	{ 0xffff, 0x50f9, { 20,  20,  10}},	// st_8_al
	{ 0xffff, 0x50fa, {  0,   0,   6}},	// trapt_16
	{ 0xffff, 0x50fb, {  0,   0,   8}},	// trapt_32
	{ 0xffff, 0x50fc, {  0,   0,   4}},	// trapt
	{ 0xffff, 0x51df, { 12,  12,  10}},	// sf_8_pi7
	{ 0xffff, 0x51e7, { 14,  14,  11}},	// sf_8_pd7
	{ 0xffff, 0x51f8, { 16,  16,  10}},	// sf_8_aw
	{ 0xffff, 0x51f9, { 20,  20,  10}},	// sf_8_al
	{ 0xffff, 0x51fa, {  0,   0,   6}},	// trapf_16
	{ 0xffff, 0x51fb, {  0,   0,   8}},	// trapf_32
	{ 0xffff, 0x51fc, {  0,   0,   4}},	// trapf
	{ 0xffff, 0x52df, { 12,  12,  10}},	// shi_8_pi7
	{ 0xffff, 0x52e7, { 14,  14,  11}},	// shi_8_pd7
	{ 0xffff, 0x52f8, { 16,  16,  10}},	// shi_8_aw
	{ 0xffff, 0x52f9, { 20,  20,  10}},	// shi_8_al
	{ 0xffff, 0x52fa, {  0,   0,   6}},	// traphi_16
	{ 0xffff, 0x52fb, {  0,   0,   8}},	// traphi_32
	{ 0xffff, 0x52fc, {  0,   0,   4}},	// traphi
	{ 0xffff, 0x53df, { 12,  12,  10}},	// sls_8_pi7
	{ 0xffff, 0x53e7, { 14,  14,  11}},	// sls_8_pd7
	{ 0xffff, 0x53f8, { 16,  16,  10}},	// sls_8_aw
	{ 0xffff, 0x53f9, { 20,  20,  10}},	// sls_8_al
	{ 0xffff, 0x53fa, {  0,   0,   6}},	// trapls_16
	{ 0xffff, 0x53fb, {  0,   0,   8}},	// trapls_32
	{ 0xffff, 0x53fc, {  0,   0,   4}},	// trapls
	{ 0xffff, 0x54df, { 12,  12,  10}},	// scc_8_pi7
	{ 0xffff, 0x54e7, { 14,  14,  11}},	// scc_8_pd7
	{ 0xffff, 0x54f8, { 16,  16,  10}},	// scc_8_aw
	{ 0xffff, 0x54f9, { 20,  20,  10}},	// scc_8_al
	{ 0xffff, 0x54fa, {  0,   0,   6}},	// trapcc_16
	{ 0xffff, 0x54fb, {  0,   0,   8}},	// trapcc_32
	{ 0xffff, 0x54fc, {  0,   0,   4}},	// trapcc
	{ 0xffff, 0x55df, { 12,  12,  10}},	// scs_8_pi7
	{ 0xffff, 0x55e7, { 14,  14,  11}},	// scs_8_pd7
	{ 0xffff, 0x55f8, { 16,  16,  10}},	// scs_8_aw
	{ 0xffff, 0x55f9, { 20,  20,  10}},	// scs_8_al
	{ 0xffff, 0x55fa, {  0,   0,   6}},	// trapcs_16
	{ 0xffff, 0x55fb, {  0,   0,   8}},	// trapcs_32
	{ 0xffff, 0x55fc, {  0,   0,   4}},	// trapcs
	{ 0xffff, 0x56df, { 12,  12,  10}},	// sne_8_pi7
	{ 0xffff, 0x56e7, { 14,  14,  11}},	// sne_8_pd7
	{ 0xffff, 0x56f8, { 16,  16,  10}},	// sne_8_aw
	{ 0xffff, 0x56f9, { 20,  20,  10}},	// sne_8_al
	{ 0xffff, 0x56fa, {  0,   0,   6}},	// trapne_16
	{ 0xffff, 0x56fb, {  0,   0,   8}},	// trapne_32
	{ 0xffff, 0x56fc, {  0,   0,   4}},	// trapne
	{ 0xffff, 0x57df, { 12,  12,  10}},	// seq_8_pi7
	{ 0xffff, 0x57e7, { 14,  14,  11}},	// seq_8_pd7
	{ 0xffff, 0x57f8, { 16,  16,  10}},	// seq_8_aw
	{ 0xffff, 0x57f9, { 20,  20,  10}},	// seq_8_al
	{ 0xffff, 0x57fa, {  0,   0,   6}},	// trapeq_16
	{ 0xffff, 0x57fb, {  0,   0,   8}},	// trapeq_32
	{ 0xffff, 0x57fc, {  0,   0,   4}},	// trapeq
	{ 0xffff, 0x58df, { 12,  12,  10}},	// svc_8_pi7
	{ 0xffff, 0x58e7, { 14,  14,  11}},	// svc_8_pd7
	{ 0xffff, 0x58f8, { 16,  16,  10}},	// svc_8_aw
	{ 0xffff, 0x58f9, { 20,  20,  10}},	// svc_8_al
	{ 0xffff, 0x58fa, {  0,   0,   6}},	// trapvc_16
	{ 0xffff, 0x58fb, {  0,   0,   8}},	// trapvc_32
	{ 0xffff, 0x58fc, {  0,   0,   4}},	// trapvc
	{ 0xffff, 0x59df, { 12,  12,  10}},	// svs_8_pi7
	{ 0xffff, 0x59e7, { 14,  14,  11}},	// svs_8_pd7
	{ 0xffff, 0x59f8, { 16,  16,  10}},	// svs_8_aw
	{ 0xffff, 0x59f9, { 20,  20,  10}},	// svs_8_al
	{ 0xffff, 0x59fa, {  0,   0,   6}},	// trapvs_16
	{ 0xffff, 0x59fb, {  0,   0,   8}},	// trapvs_32
	{ 0xffff, 0x59fc, {  0,   0,   4}},	// trapvs
	{ 0xffff, 0x5adf, { 12,  12,  10}},	// spl_8_pi7
	{ 0xffff, 0x5ae7, { 14,  14,  11}},	// spl_8_pd7
	{ 0xffff, 0x5af8, { 16,  16,  10}},	// spl_8_aw
	{ 0xffff, 0x5af9, { 20,  20,  10}},	// spl_8_al
	{ 0xffff, 0x5afa, {  0,   0,   6}},	// trappl_16
	{ 0xffff, 0x5afb, {  0,   0,   8}},	// trappl_32
	{ 0xffff, 0x5afc, {  0,   0,   4}},	// trappl
	{ 0xffff, 0x5bdf, { 12,  12,  10}},	// smi_8_pi7
	{ 0xffff, 0x5be7, { 14,  14,  11}},	// smi_8_pd7
	{ 0xffff, 0x5bf8, { 16,  16,  10}},	// smi_8_aw
	{ 0xffff, 0x5bf9, { 20,  20,  10}},	// smi_8_al
	{ 0xffff, 0x5bfa, {  0,   0,   6}},	// trapmi_16
	{ 0xffff, 0x5bfb, {  0,   0,   8}},	// trapmi_32
	{ 0xffff, 0x5bfc, {  0,   0,   4}},	// trapmi
	{ 0xffff, 0x5cdf, { 12,  12,  10}},	// sge_8_pi7
	{ 0xffff, 0x5ce7, { 14,  14,  11}},	// sge_8_pd7
	{ 0xffff, 0x5cf8, { 16,  16,  10}},	// sge_8_aw
	{ 0xffff, 0x5cf9, { 20,  20,  10}},	// sge_8_al
	{ 0xffff, 0x5cfa, {  0,   0,   6}},	// trapge_16
	{ 0xffff, 0x5cfb, {  0,   0,   8}},	// trapge_32
	{ 0xffff, 0x5cfc, {  0,   0,   4}},	// trapge
	{ 0xffff, 0x5ddf, { 12,  12,  10}},	// slt_8_pi7
	{ 0xffff, 0x5de7, { 14,  14,  11}},	// slt_8_pd7
	{ 0xffff, 0x5df8, { 16,  16,  10}},	// slt_8_aw
	{ 0xffff, 0x5df9, { 20,  20,  10}},	// slt_8_al
	{ 0xffff, 0x5dfa, {  0,   0,   6}},	// traplt_16
	{ 0xffff, 0x5dfb, {  0,   0,   8}},	// traplt_32
	{ 0xffff, 0x5dfc, {  0,   0,   4}},	// traplt
	{ 0xffff, 0x5edf, { 12,  12,  10}},	// sgt_8_pi7
	{ 0xffff, 0x5ee7, { 14,  14,  11}},	// sgt_8_pd7
	{ 0xffff, 0x5ef8, { 16,  16,  10}},	// sgt_8_aw
	{ 0xffff, 0x5ef9, { 20,  20,  10}},	// sgt_8_al
	{ 0xffff, 0x5efa, {  0,   0,   6}},	// trapgt_16
	{ 0xffff, 0x5efb, {  0,   0,   8}},	// trapgt_32
	{ 0xffff, 0x5efc, {  0,   0,   4}},	// trapgt
	{ 0xffff, 0x5fdf, { 12,  12,  10}},	// sle_8_pi7
	{ 0xffff, 0x5fe7, { 14,  14,  11}},	// sle_8_pd7
	{ 0xffff, 0x5ff8, { 16,  16,  10}},	// sle_8_aw
	{ 0xffff, 0x5ff9, { 20,  20,  10}},	// sle_8_al
	{ 0xffff, 0x5ffa, {  0,   0,   6}},	// traple_16
	{ 0xffff, 0x5ffb, {  0,   0,   8}},	// traple_32
	{ 0xffff, 0x5ffc, {  0,   0,   4}},	// traple
	{ 0xffff, 0x6000, { 10,  10,  10}},	// bra_16
	{ 0xffff, 0x60ff, {  0,   0,  10}},	// bra_32
	{ 0xffff, 0x6100, { 18,  18,   7}},	// bsr_16
	{ 0xffff, 0x61ff, {  0,   0,   7}},	// bsr_32
	{ 0xffff, 0x6200, { 10,  10,   6}},	// bhi_16
	{ 0xffff, 0x62ff, {  0,   0,   6}},	// bhi_32
	{ 0xffff, 0x6300, { 10,  10,   6}},	// bls_16
	{ 0xffff, 0x63ff, {  0,   0,   6}},	// bls_32
	{ 0xffff, 0x6400, { 10,  10,   6}},	// bcc_16
	{ 0xffff, 0x64ff, {  0,   0,   6}},	// bcc_32
	{ 0xffff, 0x6500, { 10,  10,   6}},	// bcs_16
	{ 0xffff, 0x65ff, {  0,   0,   6}},	// bcs_32
	{ 0xffff, 0x6600, { 10,  10,   6}},	// bne_16
	{ 0xffff, 0x66ff, {  0,   0,   6}},	// bne_32
	{ 0xffff, 0x6700, { 10,  10,   6}},	// beq_16
	{ 0xffff, 0x67ff, {  0,   0,   6}},	// beq_32
	{ 0xffff, 0x6800, { 10,  10,   6}},	// bvc_16
	{ 0xffff, 0x68ff, {  0,   0,   6}},	// bvc_32
	{ 0xffff, 0x6900, { 10,  10,   6}},	// bvs_16
	{ 0xffff, 0x69ff, {  0,   0,   6}},	// bvs_32
	{ 0xffff, 0x6a00, { 10,  10,   6}},	// bpl_16
	{ 0xffff, 0x6aff, {  0,   0,   6}},	// bpl_32
	{ 0xffff, 0x6b00, { 10,  10,   6}},	// bmi_16
	{ 0xffff, 0x6bff, {  0,   0,   6}},	// bmi_32
	{ 0xffff, 0x6c00, { 10,  10,   6}},	// bge_16
	{ 0xffff, 0x6cff, {  0,   0,   6}},	// bge_32
	{ 0xffff, 0x6d00, { 10,  10,   6}},	// blt_16
	{ 0xffff, 0x6dff, {  0,   0,   6}},	// blt_32
	{ 0xffff, 0x6e00, { 10,  10,   6}},	// bgt_16
	{ 0xffff, 0x6eff, {  0,   0,   6}},	// bgt_32
	{ 0xffff, 0x6f00, { 10,  10,   6}},	// ble_16
	{ 0xffff, 0x6fff, {  0,   0,   6}},	// ble_32
	{ 0xffff, 0x8f0f, { 18,  18,  16}},	// sbcd_8_mm_axy7
	{ 0xffff, 0x8f4f, {  0,   0,  13}},	// pack_16_mm_axy7
	{ 0xffff, 0x8f8f, {  0,   0,  13}},	// unpk_16_mm_axy7
	{ 0xffff, 0x9f0f, { 18,  18,  12}},	// subx_8_mm_axy7
	{ 0xffff, 0xbf0f, { 12,  12,   9}},	// cmpm_8_axy7
	{ 0xffff, 0xcf0f, { 18,  18,  16}},	// abcd_8_mm_axy7
	{ 0xffff, 0xdf0f, { 18,  18,  12}},	// addx_8_mm_axy7
	{ 0xffff, 0xe0f8, { 16,  16,   9}},	// asr_16_aw
	{ 0xffff, 0xe0f9, { 20,  20,   9}},	// asr_16_al
	{ 0xffff, 0xe1f8, { 16,  16,  10}},	// asl_16_aw
	{ 0xffff, 0xe1f9, { 20,  20,  10}},	// asl_16_al
	{ 0xffff, 0xe2f8, { 16,  16,   9}},	// lsr_16_aw
	{ 0xffff, 0xe2f9, { 20,  20,   9}},	// lsr_16_al
	{ 0xffff, 0xe3f8, { 16,  16,   9}},	// lsl_16_aw
	{ 0xffff, 0xe3f9, { 20,  20,   9}},	// lsl_16_al
	{ 0xffff, 0xe4f8, { 16,  16,   9}},	// roxr_16_aw
	{ 0xffff, 0xe4f9, { 20,  20,   9}},	// roxr_16_al
	{ 0xffff, 0xe5f8, { 16,  16,   9}},	// roxl_16_aw
	{ 0xffff, 0xe5f9, { 20,  20,   9}},	// roxl_16_al
	{ 0xffff, 0xe6f8, { 16,  16,  11}},	// ror_16_aw
	{ 0xffff, 0xe6f9, { 20,  20,  11}},	// ror_16_al
	{ 0xffff, 0xe7f8, { 16,  16,  11}},	// rol_16_aw
	{ 0xffff, 0xe7f9, { 20,  20,  11}},	// rol_16_al
	{ 0xffff, 0xe8f8, {  0,   0,  17}},	// bftst_32_aw
	{ 0xffff, 0xe8f9, {  0,   0,  17}},	// bftst_32_al
	{ 0xffff, 0xe8fa, {  0,   0,  18}},	// bftst_32_pcdi
	{ 0xffff, 0xe8fb, {  0,   0,  20}},	// bftst_32_pcix
	{ 0xffff, 0xe9f8, {  0,   0,  19}},	// bfextu_32_aw
	{ 0xffff, 0xe9f9, {  0,   0,  19}},	// bfextu_32_al
	{ 0xffff, 0xe9fa, {  0,   0,  20}},	// bfextu_32_pcdi
	{ 0xffff, 0xe9fb, {  0,   0,  22}},	// bfextu_32_pcix
	{ 0xffff, 0xeaf8, {  0,   0,  24}},	// bfchg_32_aw
	{ 0xffff, 0xeaf9, {  0,   0,  24}},	// bfchg_32_al
	{ 0xffff, 0xebf8, {  0,   0,  19}},	// bfexts_32_aw
	{ 0xffff, 0xebf9, {  0,   0,  19}},	// bfexts_32_al
	{ 0xffff, 0xebfa, {  0,   0,  20}},	// bfexts_32_pcdi
	{ 0xffff, 0xebfb, {  0,   0,  22}},	// bfexts_32_pcix
	{ 0xffff, 0xecf8, {  0,   0,  24}},	// bfclr_32_aw
	{ 0xffff, 0xecf9, {  0,   0,  24}},	// bfclr_32_al
	{ 0xffff, 0xedf8, {  0,   0,  32}},	// bfffo_32_aw
	{ 0xffff, 0xedf9, {  0,   0,  32}},	// bfffo_32_al
	{ 0xffff, 0xedfa, {  0,   0,  33}},	// bfffo_32_pcdi
	{ 0xffff, 0xedfb, {  0,   0,  35}},	// bfffo_32_pcix
	{ 0xffff, 0xeef8, {  0,   0,  24}},	// bfset_32_aw
	{ 0xffff, 0xeef9, {  0,   0,  24}},	// bfset_32_al
	{ 0xffff, 0xeff8, {  0,   0,  21}},	// bfins_32_aw
	{ 0xffff, 0xeff9, {  0,   0,  21}},	// bfins_32_al
	{ 0, 0, {0, 0, 0}}
};

#undef NUM_CPU_TYPES
