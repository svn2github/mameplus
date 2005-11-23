#ifndef M68KDRC__HEADER
#define M68KDRC__HEADER

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

#include "m68k.h"

/* ======================================================================== */
/* ================================ DRC API =============================== */
/* ======================================================================== */

/* Do whatever initialisations the core requires.  Should be called
 * at least once at init time.
 */
void m68kdrc_init(void);

/* Pulse the RESET pin on the CPU.
 * You *MUST* reset the CPU at least once to initialize the emulation
 * Note: If you didn't call m68k_set_cpu_type() before resetting
 *       the CPU for the first time, the CPU will be set to
 *       M68K_CPU_TYPE_68000.
 */
void m68kdrc_pulse_reset(void);

/* execute num_cycles worth of instructions.  returns number of cycles used */
int m68kdrc_execute(int num_cycles);

/* free resource at exit */
void m68kdrc_exit(void);


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KDRC__HEADER */
