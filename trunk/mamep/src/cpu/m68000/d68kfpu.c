#include "cpuintrf.h"
#include <math.h>

static drc_core *drc;
static void DRC_READ_EA_8(void)
{
	int ea = REG68K_IR & 0x3f;
	int mode = (ea >> 3) & 0x7;
	int reg = (REG68K_IR & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			_mov_r32_m32abs(REG_EAX, &DY);
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
					_push_imm(ea);
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
			_mov_r32_m32abs(REG_EAX, &DY);
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
			_mov_r32_m32abs(REG_EAX, &DY);
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
					_push_imm(ea);
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
			_mov_m32abs_r32(&DY, REG_EAX);
			break;
		}
		case 2:		// (An)
		{
			_push_r32(REG_EAX);
			DRC_EA_AY_AI_32();
			_push_r32(REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 3:		// (An)+
		{
			_push_r32(REG_EAX);
			DRC_EA_AY_PI_32();
			_push_r32(REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 4:		// -(An)
		{
			_push_r32(REG_EAX);
			DRC_EA_AY_PD_32();
			_push_r32(REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 5:		// (d16, An)
		{
			_push_r32(REG_EAX);
			DRC_EA_AY_DI_32();
			_push_r32(REG_EAX);
			m68kdrc_write_32();
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			_push_r32(REG_EAX);
			DRC_EA_AY_IX_32();
			_push_r32(REG_EAX);
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
					_push_r32(REG_EAX);
					_push_imm(ea);
					m68kdrc_write_32();
					break;
				}
				case 2:		// (d16, PC)
				{
					_push_r32(REG_EAX);
					DRC_EA_PCDI_32();
					_push_r32(REG_EAX);
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
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);

			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[1], REG_EAX);
			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[0], REG_EAX);

			return;
		}
		case 3:		// (An)+
		{
			DRC_EA_AY_AI_8();
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);

			_add_r32_imm(REG_EAX, 4);
			_mov_m32abs_r32(&AY, REG_EAX);

			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[1], REG_EAX);
			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[0], REG_EAX);

			return;
		}
		case 5:		// (d16, An)
		{
			DRC_EA_AY_DI_32();
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);

			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[1], REG_EAX);
			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[0], REG_EAX);

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

					_mov_r32_imm(REG_EAX, h1);
					_mov_m32abs_r32(&ptr[1], REG_EAX);
					_mov_r32_imm(REG_EBX, h2);
					_mov_m32abs_r32(&ptr[0], REG_EAX);

					return;
				}
				case 2:		// (d16, PC)
				{
					DRC_EA_PCDI_32();
					_push_r32(REG_EAX);
					_add_r32_imm(REG_EAX, 4);
					_push_r32(REG_EAX);

					m68kdrc_read_32();
					_mov_m32abs_r32(&ptr[1], REG_EAX);
					m68kdrc_read_32();
					_mov_m32abs_r32(&ptr[0], REG_EAX);

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
		case 4:		// -(An)
		{
			DRC_EA_AY_AI_32();
			_sub_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);
			_push_m32abs(&ptr[1]);
			_sub_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);
			_push_m32abs(&ptr[0]);
			_mov_m32abs_r32(&AY, REG_EAX);

			m68kdrc_write_32();
			m68kdrc_write_32();

			break;
		}
		case 5:		// (d16, An)
		{
			DRC_EA_AY_DI_32();
			_push_r32(REG_EAX);
			_push_m32abs(&ptr[0]);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);
			_push_m32abs(&ptr[1]);

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
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_push_r32(REG_EAX);
			_add_r32_imm(REG_EAX, 4);
			_mov_m32abs_r32(&AY, REG_EAX);

			m68kdrc_read_32();
			//_mov_m32abs_r32(&ptr[2], REG_EAX);
			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[1], REG_EAX);
			m68kdrc_read_32();
			_mov_m32abs_r32(&ptr[0], REG_EAX);

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
			_sub_r32_imm(REG_EAX, 4);
			_push_imm(0);
			_sub_r32_imm(REG_EAX, 4);
			_push_m32abs(&ptr[1]);
			_sub_r32_imm(REG_EAX, 4);
			_push_m32abs(&ptr[0]);

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


static void INT32_to_double(double *source, INT32 d)
{
	*source = (double)(d);
}

static void UINT32_to_float(double *source, UINT32 d)
{
	*source = (double)(*(float*)&d);
}

static void INT16_to_double(double *source, INT16 d)
{
	*source = (double)(d);
}

static void UINT64_to_double(double *source, UINT32 *ptr)
{
	UINT64 d = ((UINT64)ptr[0] << 32) | (ptr[1]);

	*source = *(double*)&d;
}

static void INT8_to_double(double *source, INT8 d)
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

static void fpgen_rm_reg(UINT16 w2)
{
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	static double source;
	static UINT64 temp;

	if (rm)
	{
		_push_imm(&source);

		switch (src)
		{
			case 0:		// Long-Word Integer
			{
				DRC_READ_EA_32();
				_push_r32(REG_EAX);
				_call(INT32_to_double);
				break;
			}
			case 1:		// Single-precision Real
			{
				DRC_READ_EA_32();
				_push_r32(REG_EAX);
				_call(UINT32_to_float);
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
				_push_r32(REG_EAX);
				_call(INT16_to_double);
				break;
			}
			case 5:		// Double-precision Real
			{
				DRC_READ_EA_64(&temp);
				_push_imm(&temp);
				_call(UINT64_to_double);
				_add_r32_imm(REG_ESP, 4);
				break;
			}
			case 6:		// Byte Integer
			{
				DRC_READ_EA_8();
				_push_r32(REG_EAX);
				_call(INT8_to_double);
				break;
			}
			default:	fatalerror("fmove_rm_reg: invalid source specifier at %08X\n", REG_PC-4);
		}

		_add_r32_imm(REG_ESP, 8);
	}
	else
	{
		//memcpy(&source, &REG68K_FP[src].f, sizeof (source));
		_push_imm(sizeof (source));
		_push_imm(&REG68K_FP[src].f);
		_push_imm(&source);
		_call(memcpy);
		_add_r32_imm(REG_ESP, 12);
	}

	//func(&REG68K_FP[dst].f, &source);

	_push_imm(&source);
	_push_imm(&REG68K_FP[dst].f);

	switch (opmode)
	{
		case 0x00:		// FMOVE
		{
			_call(m68kdrc_fmove);
			DRC_USE_CYCLES(4);
			break;
		}
		case 0x04:		// FSQRT
		{
			_call(m68kdrc_fsqrt);
			// TODO: condition codes
			DRC_USE_CYCLES(109);
			break;
		}
		case 0x18:		// FABS
		{
			_call(m68kdrc_fabs);
			// TODO: condition codes
			DRC_USE_CYCLES(3);
			break;
		}
		case 0x1a:		// FNEG
		{
			_call(m68kdrc_fneg);
			// TODO: condition codes
			DRC_USE_CYCLES(3);
			break;
		}
		case 0x20:		// FDIV
		{
			_call(m68kdrc_fdiv);
			DRC_USE_CYCLES(43);
			break;
		}
		case 0x22:		// FADD
		{
			_call(m68kdrc_fadd);
			// TODO: condition codes
			DRC_USE_CYCLES(9);
			break;
		}
		case 0x23:		// FMUL
		{
			_call(m68kdrc_fmul);
			// TODO: condition codes
			DRC_USE_CYCLES(11);
			break;
		}
		case 0x28:		// FSUB
		{
			_call(m68kdrc_fsub);
			// TODO: condition codes
			DRC_USE_CYCLES(9);
			break;
		}
		case 0x38:		// FCMP
		{
			// TODO: condition codes !!!
			DRC_USE_CYCLES(7);
			break;
		}
		case 0x3a:		// FTST
		{
			// TODO: condition codes !!!
			DRC_USE_CYCLES(7);
			break;
		}

		default:	fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, REG_PC-4);
	}

	_add_r32_imm(REG_ESP, 8);
}

static void fmove_reg_mem(UINT16 w2)
{
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	//int kfactor = w2 & 0x7f;

	_push_imm(&REG68K_FP[src]);

	switch (dst)
	{
		case 0:		// Long-Word Integer
		{
			_call(double_to_INT32);
			DRC_WRITE_EA_32();
			break;
		}
		case 1:		// Single-precision Real
		{
			_call(double_to_INT32);
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

	_add_r32_imm(REG_ESP, 4);

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
				_mov_r32_m32abs(REG_EAX, &REG68K_FPIAR);
				break;
			case 2:
				_mov_r32_m32abs(REG_EAX, &REG68K_FPSR);
				break;
			case 4:
				_mov_r32_m32abs(REG_EAX, &REG68K_FPCR);
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
				_mov_m32abs_r32(&REG68K_FPIAR, REG_EAX);
				break;
			case 2:
				_mov_m32abs_r32(&REG68K_FPSR, REG_EAX);
				break;
			case 4:
				_mov_m32abs_r32(&REG68K_FPCR, REG_EAX);
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

static void fbcc(void)
{
	INT32 disp;
//  int condition = REG68K_IR & 0x3f;
	int size = (REG68K_IR >> 6) & 0x1;

	if (size)	// 32-bit displacement
	{
		disp = OPER_I_32();
	}
	else
	{
		disp = (INT16)(OPER_I_16());
	}

	// TODO: condition and jump!!!

	DRC_USE_CYCLES(7);
}


void m68040drc_fpu_op0(drc_core *drcp)
{
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
		case 3:		// FBcc disp32
		{
			fbcc();
			break;
		}

		default:	fatalerror("m68040_fpu_op0: unimplemented main op %d\n", (REG68K_IR >> 6)	& 0x3);
	}
}

void m68040drc_fpu_op1(drc_core *drcp)
{
	drc = drcp;

	switch ((REG68K_IR >> 6) & 0x3)
	{
		case 0:		// FSAVE <ea>
		{
			_mov_r32_imm(REG_EAX, 0);
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
