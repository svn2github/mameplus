/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************/

#include "emu.h"
#include "includes/dec0.h"


/******************************************************************************/

WRITE16_MEMBER(dec0_state::dec0_update_sprites_w)
{
	memcpy(m_buffered_spriteram,m_spriteram,0x800);
}

/******************************************************************************/

static void update_24bitcol(running_machine &machine, int offset)
{
	dec0_state *state = machine.driver_data<dec0_state>();
	int r,g,b;

	r = (state->m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (state->m_generic_paletteram_16[offset] >> 8) & 0xff;
	b = (state->m_generic_paletteram2_16[offset] >> 0) & 0xff;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_MEMBER(dec0_state::dec0_paletteram_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(machine(), offset);
}

WRITE16_MEMBER(dec0_state::dec0_paletteram_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(machine(), offset);
}

/******************************************************************************/


UINT32 dec0_state::screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	flip_screen_set(m_tilegen1->get_flip_state());

	m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, 0x08, 0x0f);
	m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, 0x00, 0x0f);
	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

UINT32 dec0_state::screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_set(m_tilegen1->get_flip_state());

	/* WARNING: inverted wrt Midnight Resistance */
	if ((m_pri & 0x01) == 0)
	{
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 2)
			m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (m_pri & 4)
			m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}
	else
	{
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 2)
			m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (m_pri & 4)
			m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

UINT32 dec0_state::screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int trans;

	flip_screen_set(m_tilegen1->get_flip_state());

	if (m_pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (m_pri & 0x01)
	{
		/* WARNING: inverted wrt Midnight Resistance */
		/* Robocop uses it only for the title screen, so this might be just */
		/* completely wrong. The top 8 bits of the register might mean */
		/* something (they are 0x80 in midres, 0x00 here) */
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (m_pri & 0x02)
		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans^0x08, 0x0f);
	else
		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


UINT32 dec0_automat_state::screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int trans;

	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	address_space &space = machine().driver_data()->generic_space();
	deco_bac06_pf_control_0_w(m_tilegen1,space,0,0x0003, 0x00ff); // 8x8
	deco_bac06_pf_control_0_w(m_tilegen1,space,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen1,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen1,space,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(m_tilegen2,space,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(m_tilegen2,space,1,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen2,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen2,space,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(m_tilegen3,space,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(m_tilegen3,space,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen3,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen3,space,3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	deco_bac06_pf_control_1_w(m_tilegen1,space,0,0x0000, 0xffff); // no scroll?
	deco_bac06_pf_control_1_w(m_tilegen1,space,1,0x0000, 0xffff); // no scroll?

	deco_bac06_pf_control_1_w(m_tilegen2,space,0,m_automat_scroll_regs[3] - 0x010a, 0xffff);
	deco_bac06_pf_control_1_w(m_tilegen2,space,1,m_automat_scroll_regs[2], 0xffff);

	deco_bac06_pf_control_1_w(m_tilegen3,space,0,m_automat_scroll_regs[1] - 0x0108, 0xffff);
	deco_bac06_pf_control_1_w(m_tilegen3,space,1,m_automat_scroll_regs[0], 0xffff);


	flip_screen_set(m_tilegen1->get_flip_state());

	if (m_pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (m_pri & 0x01)
	{
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (m_pri & 0x02)
		m_spritegen->draw_sprites_bootleg(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans^0x08, 0x0f);
	else
		m_spritegen->draw_sprites_bootleg(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

UINT32 dec0_automat_state::screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	address_space &space = machine().driver_data()->generic_space();
	deco_bac06_pf_control_0_w(m_tilegen1,space,0,0x0003, 0x00ff); // 8x8
	deco_bac06_pf_control_0_w(m_tilegen1,space,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen1,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen1,space,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(m_tilegen2,space,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(m_tilegen2,space,1,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen2,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen2,space,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(m_tilegen3,space,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(m_tilegen3,space,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen3,space,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(m_tilegen3,space,3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	deco_bac06_pf_control_1_w(m_tilegen1,space,0,0x0000, 0xffff); // no scroll?
	deco_bac06_pf_control_1_w(m_tilegen1,space,1,0x0000, 0xffff); // no scroll?

	deco_bac06_pf_control_1_w(m_tilegen2,space,0,m_automat_scroll_regs[3] - 0x010a, 0xffff);
	deco_bac06_pf_control_1_w(m_tilegen2,space,1,m_automat_scroll_regs[2], 0xffff);

	deco_bac06_pf_control_1_w(m_tilegen3,space,0,m_automat_scroll_regs[1] - 0x0108, 0xffff);
	deco_bac06_pf_control_1_w(m_tilegen3,space,1,m_automat_scroll_regs[0], 0xffff);

	flip_screen_set(m_tilegen1->get_flip_state());

	m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	m_spritegen->draw_sprites_bootleg(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (m_pri&0x80)
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


/******************************************************************************/

UINT32 dec0_state::screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	flip_screen_set(m_tilegen1->get_flip_state());

	/* This game doesn't have the extra playfield chip on the game board, but
    the palette does show through. */
	bitmap.fill(machine().pens[768], cliprect);
	m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);
	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

UINT32 dec0_state::screen_update_hippodrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_set(m_tilegen1->get_flip_state());

	if (m_pri & 0x01)
	{
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);
	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

UINT32 dec0_state::screen_update_slyspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_set(m_tilegen1->get_flip_state());

	m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (m_pri&0x80)
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

UINT32 dec0_state::screen_update_midres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int trans;

	flip_screen_set(m_tilegen1->get_flip_state());

	if (m_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

	if (m_pri & 0x01)
	{
		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		m_tilegen3->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (m_pri & 0x02)
			m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans, 0x0f);

		m_tilegen2->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (m_pri & 0x02)
		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x08, trans ^ 0x08, 0x0f);
	else
		m_spritegen->draw_sprites(machine(), bitmap, cliprect, m_buffered_spriteram, 0x00, 0x00, 0x0f);

	m_tilegen1->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


WRITE16_MEMBER(dec0_state::dec0_priority_w)
{
	COMBINE_DATA(&m_pri);
}

VIDEO_START_MEMBER(dec0_state,dec0_nodma)
{
	m_buffered_spriteram = m_spriteram;
}

VIDEO_START_MEMBER(dec0_state,dec0)
{
	VIDEO_START_CALL_MEMBER(dec0_nodma);
	m_buffered_spriteram = auto_alloc_array(machine(), UINT16, 0x800/2);
}

VIDEO_START_MEMBER(dec0_automat_state,automat)
{
}

/******************************************************************************/
