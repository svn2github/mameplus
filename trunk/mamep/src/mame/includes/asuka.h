/*************************************************************************

    Asuka & Asuka  (+ Taito/Visco games on similar hardware)

*************************************************************************/
#include "video/taitoic.h"
#include "sound/msm5205.h"

class asuka_state : public driver_device
{
public:
	asuka_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cadash_shared_ram(*this, "sharedram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_pc090oj(*this, "pc090oj"),
		m_tc0100scn(*this, "tc0100scn") { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_video_ctrl;
	UINT16      m_video_mask;

	/* c-chip */
	int         m_current_round;
	int         m_current_bank;

	UINT8       m_cval[26];
	UINT8       m_cc_port;
	UINT8       m_restart_status;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	optional_shared_ptr<UINT8> m_cadash_shared_ram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<pc090oj_device> m_pc090oj;
	required_device<tc0100scn_device> m_tc0100scn;
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_address_w);
	DECLARE_READ16_MEMBER(cadash_share_r);
	DECLARE_WRITE16_MEMBER(cadash_share_w);
	DECLARE_WRITE16_MEMBER(asuka_spritectrl_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_2151_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_start_w);
	DECLARE_WRITE8_MEMBER(asuka_msm5205_stop_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_bonzeadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_asuka(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_asuka(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(cadash_interrupt);
	TIMER_CALLBACK_MEMBER(cadash_interrupt5);

	/*----------- defined in machine/bonzeadv.c -----------*/
	void WriteLevelData();
	void WriteRestartPos(int level );

	DECLARE_READ16_MEMBER( bonzeadv_cchip_ctrl_r );
	DECLARE_READ16_MEMBER( bonzeadv_cchip_ram_r );
	DECLARE_WRITE16_MEMBER( bonzeadv_cchip_ctrl_w );
	DECLARE_WRITE16_MEMBER( bonzeadv_cchip_bank_w );
	DECLARE_WRITE16_MEMBER( bonzeadv_cchip_ram_w );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(asuka_msm5205_vck);
};
