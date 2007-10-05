/* System E stuff */

extern VIDEO_UPDATE(megatech_bios);
extern VIDEO_UPDATE(megatech_md_sms);
extern DRIVER_INIT( megatech_bios );
extern MACHINE_RESET(megatech_bios);
extern MACHINE_RESET(megatech_md_sms);
extern VIDEO_EOF(megatech_bios);
extern VIDEO_EOF(megatech_md_sms);

extern READ8_HANDLER( sms_vcounter_r );
extern READ8_HANDLER( sms_vdp_data_r );
extern WRITE8_HANDLER( sms_vdp_data_w );
extern READ8_HANDLER( sms_vdp_ctrl_r );
extern WRITE8_HANDLER( sms_vdp_ctrl_w );

extern void init_for_megadrive(void);
extern void segae_md_sms_stop_scanline_timer(void);


extern READ8_HANDLER( md_sms_vdp_vcounter_r );
extern READ8_HANDLER( md_sms_vdp_data_r );
extern WRITE8_HANDLER( md_sms_vdp_data_w );
extern READ8_HANDLER( md_sms_vdp_ctrl_r );
extern WRITE8_HANDLER( md_sms_vdp_ctrl_w );
extern WRITE8_HANDLER( sms_sn76496_w );


//** share between segae.c and sms.c

struct sms_vdp
{
	UINT8 chip_id;

	UINT8  cmd_pend;
	UINT8  cmd_part1;
	UINT8  cmd_part2;
	UINT16 addr_reg;
	UINT8  cmd_reg;
	UINT8  regs[0x10];
	UINT8  readbuf;
	UINT8* vram;
	UINT8* cram;
	UINT8  writemode;
	mame_bitmap* r_bitmap;
	UINT8* tile_renderline;
	UINT8* sprite_renderline;

	UINT8 sprite_collision;
	UINT8 sprite_overflow;

	UINT8  yscroll;
	UINT8  hint_counter;

	UINT8 frame_irq_pending;
	UINT8 line_irq_pending;

	UINT8 vdp_type;

	UINT8 gg_cram_latch; // gamegear specific.

	/* below are MAME specific, to make things easier */
	UINT8 screen_mode;
	UINT8 is_pal;
	int sms_scanline_counter;
	int sms_total_scanlines;
	int sms_framerate;
	mame_timer* sms_scanline_timer;
	UINT16* cram_mamecolours; // for use on RGB_DIRECT screen
	int	 (*set_irq)(int state);

};

enum
{
	SMS_VDP = 0,  // SMS1 VDP
	SMS2_VDP = 1, // SMS2 VDP, or Game Gear VDP running in SMS2 Mode
	GG_VDP = 2,   // Game Gear VDP running in Game Gear Mode
	GEN_VDP = 3   // Genesis VDP running in SMS2 Mode
};

struct sms_mode
{
	UINT8 sms2_name[40];
	int sms2_valid;
	int sms2_height;
	int sms2_tilemap_height;
	UINT8* sms_vcounter_table;
	UINT8* sms_hcounter_table;

};

extern UINT8* sms_mainram;
extern struct sms_vdp *vdp1;
extern struct sms_mode sms_mode_table[];
extern MACHINE_RESET(sms);
extern VIDEO_START(sms);
extern void *sms_start_vdp(running_machine *machine, int type);
extern int sms_vdp_cpu0_irq_callback(int status);
