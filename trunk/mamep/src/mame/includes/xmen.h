
class xmen_state : public driver_device
{
public:
	xmen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_xmen6p_spriteramleft(*this, "spriteramleft"),
		m_xmen6p_spriteramright(*this, "spriteramright"),
		m_xmen6p_tilemapleft(*this, "tilemapleft"),
		m_xmen6p_tilemapright(*this, "tilemapright"){ }

	/* memory pointers */
//  UINT16 *   m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        m_layer_colorbase[3];
	int        m_sprite_colorbase;
	int        m_layerpri[3];

	/* for xmen6p */
	bitmap_ind16   *m_screen_right;
	bitmap_ind16   *m_screen_left;
	optional_shared_ptr<UINT16> m_xmen6p_spriteramleft;
	optional_shared_ptr<UINT16> m_xmen6p_spriteramright;
	optional_shared_ptr<UINT16> m_xmen6p_tilemapleft;
	optional_shared_ptr<UINT16> m_xmen6p_tilemapright;
	UINT16 *   m_k053247_ram;

	/* misc */
	UINT8       m_sound_curbank;
	UINT8       m_vblank_irq_mask;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_k054539;
	device_t *m_k052109;
	device_t *m_k053246;
	device_t *m_k053251;
	device_t *m_lscreen;
	device_t *m_rscreen;
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE16_MEMBER(sound_cmd_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_WRITE16_MEMBER(xmen_18fa00_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_CUSTOM_INPUT_MEMBER(xmen_frame_r);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_VIDEO_START(xmen6p);
	UINT32 screen_update_xmen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_xmen6p_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_xmen6p_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_xmen6p(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(xmen_scanline);
	void sound_reset_bank();
};

/*----------- defined in video/xmen.c -----------*/

void xmen_tile_callback(running_machine &machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
void xmen_sprite_callback(running_machine &machine, int *code,int *color,int *priority_mask);
