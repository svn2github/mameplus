#ifndef _CPSCHNGHR_H_
#define _CPSCHNGHR_H_


/*----------- defined in machine/kabuki.c -----------*/


void wof_decode(running_machine *machine);


/*----------- defined in video/cpschngr.c -----------*/


extern UINT16 *cps1_gfxram;     /* Video RAM */
extern UINT16 *cps1_cps_a_regs;
extern UINT16 *cps1_cps_b_regs;
extern size_t cps1_gfxram_size;

WRITE16_HANDLER( cps1_mess_cps_a_w );
WRITE16_HANDLER( cps1_mess_cps_b_w );
READ16_HANDLER( cps1_mess_cps_b_r );
WRITE16_HANDLER( cps1_mess_gfxram_w );

DRIVER_INIT( cps1_mess );

VIDEO_START( cps1_mess );
VIDEO_UPDATE( cps1_mess );
VIDEO_EOF( cps1_mess );

#endif
