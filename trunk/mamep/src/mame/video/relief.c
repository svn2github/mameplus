/***************************************************************************

    Atari "Round" hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "includes/relief.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(relief_state::get_playfield_tile_info)
{
	UINT16 data1 = m_playfield[tile_index];
	UINT16 data2 = m_playfield_upper[tile_index] & 0xff;
	int code = data1 & 0x7fff;
	int color = 0x20 + (data2 & 0x0f);
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}


TILE_GET_INFO_MEMBER(relief_state::get_playfield2_tile_info)
{
	UINT16 data1 = m_playfield2[tile_index];
	UINT16 data2 = m_playfield_upper[tile_index] >> 8;
	int code = data1 & 0x7fff;
	int color = data2 & 0x0f;
	SET_TILE_INFO_MEMBER(0, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(relief_state,relief)
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x00ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x0007 }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};

	/* MOs are 5bpp but with a 4-bit color granularity */
	machine().gfx[1]->set_granularity(16);

	/* initialize the playfield */
	m_playfield_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(relief_state::get_playfield_tile_info),this), TILEMAP_SCAN_COLS,  8,8, 64,64);

	/* initialize the second playfield */
	m_playfield2_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(relief_state::get_playfield2_tile_info),this), TILEMAP_SCAN_COLS,  8,8, 64,64);
	m_playfield2_tilemap->set_transparent_pen(0);

	/* initialize the motion objects */
	atarimo_init(machine(), 0, &modesc);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

UINT32 relief_state::screen_update_relief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind8 &priority_bitmap = machine().priority_bitmap;
	atarimo_rect_list rectlist;
	bitmap_ind16 *mobitmap;
	int x, y, r;

	/* draw the playfield */
	priority_bitmap.fill(0, cliprect);
	m_playfield_tilemap->draw(bitmap, cliprect, 0, 0);
	m_playfield2_tilemap->draw(bitmap, cliprect, 0, 1);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = &mobitmap->pix16(y);
			UINT16 *pf = &bitmap.pix16(y);
			UINT8 *pri = &priority_bitmap.pix8(y);
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* verified from the GALs on the real PCB; equations follow
                     *
                     *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
                     *      PF/M = PFXS
                     *
                     *      --- CS0 is set to 1 if the MO is transparent
                     *      CS0=!MPX0*!MPX1*!MPX2*!MPX3
                     *
                     *      --- CS1 is 1 to select playfield pixels or 0 to select MO pixels
                     *      !CS1=MPX5*MPX6*MPX7*!CS0
                     *          +!MPX4*MPX5*MPX6*MPX7
                     *          +PFXS*!CS0
                     *          +!MPX4*PFXS
                     *
                     *      --- CRA10 is the 0x200 bit of the color RAM index; set for the top playfield only
                     *      CRA10:=CS1*PFXS
                     *
                     *      --- CRA9 is the 0x100 bit of the color RAM index; set for MOs only
                     *      !CA9:=CS1
                     *
                     *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
                     */
					int cs0 = 0;
					int cs1 = 1;

					/* compute the CS0 signal */
					cs0 = ((mo[x] & 0x0f) == 0);

					/* compute the CS1 signal */
					if ((!cs0 && (mo[x] & 0xe0) == 0xe0) ||
						((mo[x] & 0xf0) == 0xe0) ||
						(!pri[x] && !cs0) ||
						(!pri[x] && !(mo[x] & 0x10)))
						cs1 = 0;

					/* MO is displayed if cs1 == 0 */
					if (!cs1)
						pf[x] = mo[x];

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
