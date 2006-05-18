/***************************************************************************
	pong.c
	Sound handler

	J. Buchmueller, November '99
****************************************************************************/

#include "driver.h"
#include "vidhrdw/pong.h"
#include "sound/samples.h"

/* HJB 99/11/22 corrected HIT_CLOCK and VBLANK_CLOCK */
#define HIT_CLOCK		(PONG_MAX_V-PONG_VBLANK) * PONG_FPS / 16 / 2
#define VBLANK_CLOCK	(PONG_MAX_V-PONG_VBLANK) * PONG_FPS / 16 / 4
#define SCORE_CLOCK 	PONG_MAX_V * PONG_FPS / 32

#define CHANNEL_HIT	0
#define CHANNEL_VBLANK	1
#define CHANNEL_SCORE	2

static	INT16 waveform[4] = { -120, -120, 120, 120 };

int pong_hit_sound = 0;
int pong_vblank_sound = 0;
int pong_score_sound = 0;

/************************************/
/* Sound handler start				*/
/************************************/
static void pong_sh_update(int param)
{
	sample_set_volume(CHANNEL_HIT, pong_hit_sound ? 100.0 : 0.0);
	sample_set_volume(CHANNEL_VBLANK, pong_vblank_sound ? 100.0 : 0.0);
	sample_set_volume(CHANNEL_SCORE, pong_score_sound ? 100.0 : 0.0);
}

static void pong_sh_start(void)
{
	sample_start_raw(CHANNEL_HIT ,waveform, sizeof(waveform), sizeof(waveform)*HIT_CLOCK, 1);
	sample_start_raw(CHANNEL_VBLANK,waveform, sizeof(waveform), sizeof(waveform)*VBLANK_CLOCK, 1);
	sample_start_raw(CHANNEL_SCORE, waveform, sizeof(waveform), sizeof(waveform)*SCORE_CLOCK, 1);

	sample_set_volume(CHANNEL_HIT, 0.0);
	sample_set_volume(CHANNEL_VBLANK, 0.0);
	sample_set_volume(CHANNEL_SCORE, 0.0);

	timer_pulse(TIME_IN_HZ(Machine->refresh_rate[0]), 0, pong_sh_update);
}

struct Samplesinterface pong_samples_interface =
{
	3,
	NULL,
	pong_sh_start
};
