#include "cpuintrf.h"
#include <math.h>

#define FPCC_N			0x08000000
#define FPCC_Z			0x04000000
#define FPCC_I			0x02000000
#define FPCC_NAN		0x01000000

#define DOUBLE_INFINITY					U64(0x7ff0000000000000)
#define DOUBLE_EXPONENT					U64(0x7ff0000000000000)
#define DOUBLE_MANTISSA					U64(0x000fffffffffffff)

static drc_core *drc;

static void SET_CONDITION_CODES(fp_reg *p)
{
	REG68K_FPSR &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (p->i & U64(0x8000000000000000))
	{
		REG68K_FPSR |= FPCC_N;
	}

	// zero flag
	if ((p->i & U64(0x7fffffffffffffff)) == 0)
	{
		REG68K_FPSR |= FPCC_Z;
	}

	// infinity flag
	if ((p->i & U64(0x7fffffffffffffff)) == DOUBLE_INFINITY)
	{
		REG68K_FPSR |= FPCC_I;
	}

	// NaN flag
	if (((p->i & DOUBLE_EXPONENT) == DOUBLE_EXPONENT) && ((p->i & DOUBLE_MANTISSA) != 0))
	{
		REG68K_FPSR |= FPCC_NAN;
	}
}

static int TEST_COND_LT(void)
{
	int n = (REG68K_FPSR & FPCC_N) != 0;
	int z = (REG68K_FPSR & FPCC_Z) != 0;
	int nan = (REG68K_FPSR & FPCC_NAN) != 0;

	return (n && !(nan || z));		
}

static int TEST_COND_LE(void)
{
	int n = (REG68K_FPSR & FPCC_N) != 0;
	int z = (REG68K_FPSR & FPCC_Z) != 0;
	int nan = (REG68K_FPSR & FPCC_NAN) != 0;

	return (z || (n && !nan));		
}

static int TEST_COND_NGE(void)
{
	int n = (REG68K_FPSR & FPCC_N) != 0;
	int z = (REG68K_FPSR & FPCC_Z) != 0;
	int nan = (REG68K_FPSR & FPCC_NAN) != 0;

	return (nan || (n && !z));		
}

static void TEST_CONDITION(int condition)
{
	switch (condition)
	{
		case 0x00:	// False
			emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
			return;

		case 0x01:	// Equal
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_Z);
			return;

		case 0x0e:	// Not Equal
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
			emit_not_r32(DRCTOP, REG_EAX);
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_Z);
			return;

		case 0x0f:	// True
			emit_or_r32_imm(DRCTOP, REG_EAX, 1);
			return;

		case 0x12:	// Greater Than
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
			emit_not_r32(DRCTOP, REG_EAX);
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_NAN | FPCC_Z | FPCC_N);
			return;

		case 0x13:	// Greater or Equal
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
			emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EAX);
			emit_not_r32(DRCTOP, REG_EBX);
			emit_and_r32_imm(DRCTOP, REG_EBX, FPCC_NAN | FPCC_N);
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_Z);
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);
			return;

		case 0x14:	// Less Than
			emit_call(DRCTOP, (x86code *)TEST_COND_LT);
			return;

		case 0x15:	// Less Than or Equal
			emit_call(DRCTOP, (x86code *)TEST_COND_LE);
			return;

		case 0x1a:	// Not Less Than or Equal
			emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EAX);
			emit_not_r32(DRCTOP, REG_EBX);
			emit_and_r32_imm(DRCTOP, REG_EBX, FPCC_N | FPCC_Z);
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_NAN);
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);
			return;

		case 0x1b:	// Not Less Than
			emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EAX);
			emit_not_r32(DRCTOP, REG_EBX);
			emit_and_r32_imm(DRCTOP, REG_EBX, FPCC_N);
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_NAN | FPCC_Z);
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);
			return;

		case 0x1c:	// Not Greater or Equal Than
			emit_call(DRCTOP, (x86code *)TEST_COND_NGE);
			return;

		case 0x1d:	// Not Greater Than
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
			emit_and_r32_imm(DRCTOP, REG_EAX, FPCC_NAN | FPCC_Z | FPCC_N);
			return;

		default:		fatalerror("M68040: test_condition: unhandled condition %02X\n", condition);
	}

	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
}

static void DRC_READ_EA_8(void)
{
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (REG68K_IR & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&DY));
			return;
		}
		case 5:		// (d16, An)
		{
			DRC_OPER_AY_DI_8();
			return;
		}
		case 6:		// (An) + (Xn) + d8
		{
			DRC_OPER_AY_IX_8();
			return;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16();
					UINT32 d2 = OPER_I_16();
					UINT32 ea = (d1 << 16) | d2;
					emit_push_imm(DRCTOP, ea);
					m68kdrc_read_8();
					return;
				}
				case 4:		// #<data>
				{
					DRC_OPER_I_8();
					return;
				}
				default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_READ_EA_16(void)
{
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (REG68K_IR & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&DY));
			return;
		}
		case 2:		// (An)
		{
			DRC_OPER_AY_AI_16();
			return;
		}
		case 5:		// (d16, An)
		{
			DRC_OPER_AY_DI_16();
			return;
		}
		case 6:		// (An) + (Xn) + d8
		{
			DRC_OPER_AY_IX_16();
			return;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16();
					UINT32 d2 = OPER_I_16();
					UINT32 ea = (d1 << 16) | d2;
					emit_push_imm(DRCTOP, ea);
					m68kdrc_read_16();
					return;
				}
				case 4:		// #<data>
				{
					DRC_OPER_I_16();
					return;
				}

				default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_READ_EA_32(void)
{
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (REG68K_IR & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&DY));
			return;
		}
		case 2:		// (An)
		{
			DRC_OPER_AY_AI_32();
			return;
		}
		case 3:		// (An)+
		{
			DRC_OPER_AY_PI_32();
			return;
		}
		case 5:		// (d16, An)
		{
			DRC_OPER_AY_DI_32();
			return;
		}
		case 6:		// (An) + (Xn) + d8
		{
			DRC_OPER_AY_IX_32();
			return;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16();
					UINT32 d2 = OPER_I_16();
					UINT32 ea = (d1 << 16) | d2;
					emit_push_imm(DRCTOP, ea);
					m68kdrc_read_32();
					return;
				}
				case 2:		// (d16, PC)
				{
					DRC_OPER_PCDI_32();
					return;
				}
				case 4:		// #<data>
				{
					DRC_OPER_I_32();
					return;
				}
				default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_WRITE_EA_32(void)
{
	// IN: EAX = data

	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			emit_mov_m32_r32(DRCTOP, MABS(&DY), REG_EAX);
			break;
		}
		case 2:		// (An)
		{
			emit_push_r32(DRCTOP, REG_EAX);
			DRC_EA_AY_AI_32();
			emit_push_r32(DRCTOP, REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 3:		// (An)+
		{
			emit_push_r32(DRCTOP, REG_EAX);
			DRC_EA_AY_PI_32();
			emit_push_r32(DRCTOP, REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 4:		// -(An)
		{
			emit_push_r32(DRCTOP, REG_EAX);
			DRC_EA_AY_PD_32();
			emit_push_r32(DRCTOP, REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 5:		// (d16, An)
		{
			emit_push_r32(DRCTOP, REG_EAX);
			DRC_EA_AY_DI_32();
			emit_push_r32(DRCTOP, REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			emit_push_r32(DRCTOP, REG_EAX);
			DRC_EA_AY_IX_32();
			emit_push_r32(DRCTOP, REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					UINT32 d1 = OPER_I_16();
					UINT32 d2 = OPER_I_16();
					UINT32 ea = (d1 << 16) | d2;
					emit_push_r32(DRCTOP, REG_EAX);
					emit_push_imm(DRCTOP, ea);
					m68kdrc_write_32();
					break;
				}
				case 2:		// (d16, PC)
				{
					emit_push_r32(DRCTOP, REG_EAX);
					DRC_EA_PCDI_32();
					emit_push_r32(DRCTOP, REG_EAX);
					m68kdrc_write_32();
					break;
				}
				default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_READ_EA_64(UINT64 *p)
{
	UINT32 *ptr = (UINT32 *)p;
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			DRC_EA_AY_AI_32();
			emit_add_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);

			return;
		}
		case 3:		// (An)+
		{
			DRC_EA_AY_AI_8();
			emit_add_r32_imm(DRCTOP, REG_EAX, 8);
			emit_mov_m32_r32(DRCTOP, MABS(&AY), REG_EAX);

			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);


			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);

			return;
		}
		case 5:		// (d16, An)
		{
			DRC_EA_AY_DI_32();
			emit_add_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);

			return;
		}
		case 7:
		{
			switch (reg)
			{
				case 4:		// #<data>
				{
					UINT32 h1 = OPER_I_32();
					UINT32 h2 = OPER_I_32();

					emit_mov_r32_imm(DRCTOP, REG_EAX, h1);
					emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
					emit_mov_r32_imm(DRCTOP, REG_EBX, h2);
					emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);

					return;
				}
				case 2:		// (d16, PC)
				{
					DRC_EA_PCDI_32();
					emit_add_r32_imm(DRCTOP, REG_EAX, 4);
					emit_push_r32(DRCTOP, REG_EAX);
					emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
					emit_push_r32(DRCTOP, REG_EAX);

					m68kdrc_read_32();
					emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
					m68kdrc_read_32();
					emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);

					return;
				}
				default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_WRITE_EA_64(UINT64 *p)
{
	UINT32 *ptr = (UINT32 *)p;
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			DRC_EA_AY_AI_32();
			emit_push_m32(DRCTOP, MABS(&ptr[0]));
			emit_add_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[1]));
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_write_32();
			m68kdrc_write_32();

			break;
		}
		case 4:		// -(An)
		{
			DRC_EA_AY_AI_32();
			emit_sub_r32_imm(DRCTOP, REG_EAX, 8);
			emit_mov_m32_r32(DRCTOP, MABS(&AY), REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[0]));
			emit_add_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[1]));
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_write_32();
			m68kdrc_write_32();

			break;
		}
		case 5:		// (d16, An)
		{
			DRC_EA_AY_DI_32();
			emit_push_m32(DRCTOP, MABS(&ptr[0]));
			emit_add_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[1]));
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_write_32();
			m68kdrc_write_32();

			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}

static void DRC_READ_EA_FPE(fp_reg *p)
{
	UINT32 *ptr = (UINT32 *)&p->i;
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 3:		// (An)+
		{
			DRC_EA_AY_AI_32();
			emit_add_r32_imm(DRCTOP, REG_EAX, 12);
			emit_mov_m32_r32(DRCTOP, MABS(&AY), REG_EAX);

			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[1]), REG_EAX);
			m68kdrc_read_32();
			emit_mov_m32_r32(DRCTOP, MABS(&ptr[0]), REG_EAX);
			m68kdrc_read_32();

			break;
		}
		default:	fatalerror("MC68040: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
	}

	return;
}

static void DRC_WRITE_EA_FPE(fp_reg *p)
{
	UINT32 *ptr = (UINT32 *)&p->i;
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 4:		// -(An)
		{
			DRC_EA_AY_AI_32();
			emit_sub_r32_imm(DRCTOP, REG_EAX, 12);
			emit_mov_m32_r32(DRCTOP, MABS(&AY), REG_EAX);

			emit_push_imm(DRCTOP, 0);
			emit_add_r32_imm(DRCTOP, REG_EAX, 8);
			emit_push_r32(DRCTOP, REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[0]));
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			emit_push_m32(DRCTOP, MABS(&ptr[1]));
			emit_sub_r32_imm(DRCTOP, REG_EAX, 4);
			emit_push_r32(DRCTOP, REG_EAX);

			m68kdrc_write_32();
			m68kdrc_write_32();
			m68kdrc_write_32();

			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_FPE: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
}


static void m68kdrc_fmove(double *dst, double *src);
static void m68kdrc_fsqrt(double *dst, double *src);
static void m68kdrc_fabs(double *dst, double *src);
static void m68kdrc_fneg(double *dst, double *src);
static void m68kdrc_fdiv(double *dst, double *src);
static void m68kdrc_fadd(double *dst, double *src);
static void m68kdrc_fmul(double *dst, double *src);
static void m68kdrc_fsub(double *dst, double *src);

static void INT32_to_double(INT32 d, double *source);
static void UINT32_to_float(UINT32 d, double *source);
static void INT16_to_double(INT16 d, double *source);
static void UINT64_to_double(UINT32 *ptr, double *source);
static void INT8_to_double(INT8 d, double *source);
static INT32 double_to_INT32(fp_reg *source);
static UINT32 float_to_UINT32(fp_reg *source);

static void fpgen_rm_reg(UINT16 w2)
{
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	static double source;
	static fp_reg res;

	if (rm)
	{
		emit_push_imm(DRCTOP, (FPTR)&source);

		switch (src)
		{
			case 0:		// Long-Word Integer
			{
				DRC_READ_EA_32();
				emit_push_r32(DRCTOP, REG_EAX);
				emit_call(DRCTOP, (x86code *)INT32_to_double);
				break;
			}
			case 1:		// Single-precision Real
			{
				DRC_READ_EA_32();
				emit_push_r32(DRCTOP, REG_EAX);
				emit_call(DRCTOP, (x86code *)UINT32_to_float);
				break;
			}
			case 2:		// Extended-precision Real
			{
				fatalerror("fpgen_rm_reg: extended-precision real load unimplemented at %08X\n", REG_PC-4);
				break;
			}
			case 3:		// Packed-decimal Real
			{
				fatalerror("fpgen_rm_reg: packed-decimal real load unimplemented at %08X\n", REG_PC-4);
				break;
			}
			case 4:		// Word Integer
			{
				DRC_READ_EA_16();
				emit_push_r32(DRCTOP, REG_EAX);
				emit_call(DRCTOP, (x86code *)INT16_to_double);
				break;
			}
			case 5:		// Double-precision Real
			{
				static UINT64 temp;

				DRC_READ_EA_64(&temp);
				emit_push_imm(DRCTOP, (FPTR)&temp);
				emit_call(DRCTOP, (x86code *)UINT64_to_double);
				break;
			}
			case 6:		// Byte Integer
			{
				DRC_READ_EA_8();
				emit_push_r32(DRCTOP, REG_EAX);
				emit_call(DRCTOP, (x86code *)INT8_to_double);
				break;
			}
			default:	fatalerror("fmove_rm_reg: invalid source specifier at %08X\n", REG_PC-4);
		}

		emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	}
	else
	{
		//memcpy(&source, &REG68K_FP[src].f, sizeof (source));
		emit_push_imm(DRCTOP, sizeof (source));
		emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[src].f);
		emit_push_imm(DRCTOP, (FPTR)&source);
		emit_call(DRCTOP, (x86code *)memcpy);
		emit_add_r32_imm(DRCTOP, REG_ESP, 12);
	}

	//func(&REG68K_FP[dst].f, &source);

	emit_push_imm(DRCTOP, (FPTR)&source);
	emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst].f);

	switch (opmode)
	{
		case 0x00:		// FMOVE
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fmove);
			DRC_USE_CYCLES(4);
			break;
		}
		case 0x04:		// FSQRT
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fsqrt);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(109);
			break;
		}
		case 0x18:		// FABS
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fabs);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(3);
			break;
		}
		case 0x1a:		// FNEG
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fneg);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(3);
			break;
		}
		case 0x20:		// FDIV
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fdiv);
			DRC_USE_CYCLES(43);
			break;
		}
		case 0x22:		// FADD
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fadd);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(9);
			break;
		}
		case 0x23:		// FMUL
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fmul);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(11);
			break;
		}
		case 0x28:		// FSUB
		{
			emit_call(DRCTOP, (x86code *)m68kdrc_fsub);
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst]);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(9);
			break;
		}
		case 0x38:		// FCMP
		{
			emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[dst].f);
			emit_push_imm(DRCTOP, (FPTR)&res.f);
			emit_call(DRCTOP, (x86code *)m68kdrc_fmove);
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);

			emit_push_imm(DRCTOP, (FPTR)&source);
			emit_push_imm(DRCTOP, (FPTR)&res.f);
			emit_call(DRCTOP, (x86code *)m68kdrc_fsub);
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);

			emit_push_imm(DRCTOP, (FPTR)&res);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(7);
			break;
		}
		case 0x3a:		// FTST
		{
			emit_push_imm(DRCTOP, (FPTR)&source);
			emit_push_imm(DRCTOP, (FPTR)&res.f);
			emit_call(DRCTOP, (x86code *)m68kdrc_fmove);
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);

			emit_push_imm(DRCTOP, (FPTR)&res);
			emit_call(DRCTOP, (x86code *)SET_CONDITION_CODES);
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);
			DRC_USE_CYCLES(7);
			break;
		}

		default:	fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, REG_PC-4);
	}

	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
}

static void fmove_reg_mem(UINT16 w2)
{
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	//int kfactor = w2 & 0x7f;

	emit_push_imm(DRCTOP, (FPTR)&REG68K_FP[src]);

	switch (dst)
	{
		case 0:		// Long-Word Integer
		{
			emit_call(DRCTOP, (x86code *)double_to_INT32);
			DRC_WRITE_EA_32();
			break;
		}
		case 1:		// Single-precision Real
		{
			emit_call(DRCTOP, (x86code *)float_to_UINT32);
			DRC_WRITE_EA_32();
			break;
		}
		case 2:		// Extended-precision Real
		{
			fatalerror("fmove_reg_mem: extended-precision real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 3:		// Packed-decimal Real with Static K-factor
		{
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 4:		// Word Integer
		{
			fatalerror("fmove_reg_mem: word integer store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 5:		// Double-precision Real
		{
			DRC_WRITE_EA_64(&REG68K_FP[src].i);
			break;
		}
		case 6:		// Byte Integer
		{
			fatalerror("fmove_reg_mem: byte integer store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 7:		// Packed-decimal Real with Dynamic K-factor
		{
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
	}

	emit_add_r32_imm(DRCTOP, REG_ESP, 4);

	DRC_USE_CYCLES(12);
}

static void fmove_fpcr(UINT16 w2)
{
	int dir = (w2 >> 13) & 0x1;
	int reg = (w2 >> 10) & 0x7;

	if (dir)	// From system control reg to <ea>
	{
		switch (reg)
		{
			case 1:
				emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPIAR));
				break;
			case 2:
				emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPSR));
				break;
			case 4:
				emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG68K_FPCR));
				break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}

		DRC_WRITE_EA_32();
	}
	else		// From <ea> to system control reg
	{
		DRC_READ_EA_32();

		switch (reg)
		{
			case 1:
				emit_mov_m32_r32(DRCTOP, MABS(&REG68K_FPIAR), REG_EAX);
				break;
			case 2:
				emit_mov_m32_r32(DRCTOP, MABS(&REG68K_FPSR), REG_EAX);
				break;
			case 4:
				emit_mov_m32_r32(DRCTOP, MABS(&REG68K_FPCR), REG_EAX);
				break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}
	}

	DRC_USE_CYCLES(10);
}

static void fmovem(UINT16 w2)
{
	int i;
	int dir = (w2 >> 13) & 0x1;
	int mode = (w2 >> 11) & 0x3;
	int reglist = w2 & 0xff;

	if (dir)	// From FP regs to mem
	{
		switch (mode)
		{
			case 0:		// Static register list, predecrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						DRC_WRITE_EA_FPE(&REG68K_FP[i]);
						DRC_USE_CYCLES(2);
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
	else		// From mem to FP regs
	{
		switch (mode)
		{
			case 2:		// Static register list, postincrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						DRC_READ_EA_FPE(&REG68K_FP[7-i]);
						DRC_USE_CYCLES(2);
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
}

static void fbcc16(void)
{
	INT32 offset;
	int condition = REG68K_IR & 0x1f;
	emit_link link1;

	offset = (INT16)(OPER_I_16());

	// TODO: condition and jump!!!
	TEST_CONDITION(condition);
	emit_jcc_near_link(DRCTOP, COND_Z, &link1);

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68kdrc_branch_16(offset-2, 1);

resolve_link(DRCTOP, &link1);
	DRC_USE_CYCLES(7);
}

static void fbcc32(void)
{
	INT32 offset;
	int condition = REG68K_IR & 0x1f;
	emit_link link1;

	offset = OPER_I_32();

	// TODO: condition and jump!!!
	TEST_CONDITION(condition);
	emit_jcc_near_link(DRCTOP, COND_Z, &link1);

	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68kdrc_branch_32(offset-4, 1);

resolve_link(DRCTOP, &link1);
	DRC_USE_CYCLES(7);
}


void m68040drc_fpu_op0(drc_core *drcp)
{
	uint save_pc = REG68K_PC;
	drc = drcp;

	switch ((REG68K_IR >> 6) & 0x3)
	{
		case 0:
		{
			UINT16 w2 = OPER_I_16();
			switch ((w2 >> 13) & 0x7)
			{
				case 0x0:	// FPU ALU FP, FP
				case 0x2:	// FPU ALU ea, FP
				{
					fpgen_rm_reg(w2);
					break;
				}

				case 0x3:	// FMOVE FP, ea
				{
					fmove_reg_mem(w2);
					break;
				}

				case 0x4:	// FMOVE ea, FPCR
				case 0x5:	// FMOVE FPCR, ea
				{
					fmove_fpcr(w2);
					break;
				}

				case 0x6:	// FMOVEM ea, list
				case 0x7:	// FMOVEM list, ea
				{
					fmovem(w2);
					break;
				}

				default:	fatalerror("m68040_fpu_op0: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, REG_PC-4);
			}
			break;
		}

		case 2:		// FBcc disp16
		{
			fbcc16();
			break;
		}
		case 3:		// FBcc disp32
		{
			fbcc32();
			break;
		}

		default:	fatalerror("m68040_fpu_op0: unimplemented main op %d\n", (REG68K_IR >> 6)	& 0x3);
	}

	m68kdrc_instr_size = REG68K_PC - save_pc + 2;
}

void m68040drc_fpu_op1(drc_core *drcp)
{
	uint save_pc = REG68K_PC;
	drc = drcp;

	switch ((REG68K_IR >> 6) & 0x3)
	{
		case 0:		// FSAVE <ea>
		{
			emit_mov_r32_imm(DRCTOP, REG_EAX, 0);
			DRC_WRITE_EA_32();
			// TODO: correct state frame
			break;
		}

		case 1:		// FRESTORE <ea>
		{
			DRC_READ_EA_32();
			// TODO: correct state frame
			break;
		}

		default:	fatalerror("m68040_fpu_op1: unimplemented op %d at %08X\n", (REG68K_IR >> 6) & 0x3, REG_PC-2);
	}

	m68kdrc_instr_size = REG68K_PC - save_pc + 2;
}



static void m68kdrc_fmove(double *dst, double *src)
{
	*dst = *src;
}

static void m68kdrc_fsqrt(double *dst, double *src)
{
	*dst = sqrt(*src);
}

static void m68kdrc_fabs(double *dst, double *src)
{
	*dst = fabs(*src);
}

static void m68kdrc_fneg(double *dst, double *src)
{
	*dst = -*src;
}

static void m68kdrc_fdiv(double *dst, double *src)
{
	*dst /= *src;
}

static void m68kdrc_fadd(double *dst, double *src)
{
	*dst += *src;
}

static void m68kdrc_fmul(double *dst, double *src)
{
	*dst *= *src;
}

static void m68kdrc_fsub(double *dst, double *src)
{
	*dst -= *src;
}

static void INT32_to_double(INT32 d, double *source)
{
	*source = (double)(d);
}

static void UINT32_to_float(UINT32 d, double *source)
{
	*source = (double)(*(float*)&d);
}

static void INT16_to_double(INT16 d, double *source)
{
	*source = (double)(d);
}

static void UINT64_to_double(UINT32 *ptr, double *source)
{
	UINT64 d = ((UINT64)ptr[0] << 32) | (ptr[1]);

	*source = *(double*)&d;
}

static void INT8_to_double(INT8 d, double *source)
{
	*source = (double)(d);
}

static INT32 double_to_INT32(fp_reg *source)
{
	INT32 d = (INT32)(source->f);
	return d;
}

static UINT32 float_to_UINT32(fp_reg *source)
{
	float f = (float)(source->f);
	UINT32 d = *(UINT32 *)&f;
	return d;
}
