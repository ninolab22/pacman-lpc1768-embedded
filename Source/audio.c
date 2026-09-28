/*----------------------------------------------------------------------------
 * audio.c - Speaker via DAC (P0.26 AOUT) + Timer1.
 *   Toni a onda quadra: Timer1 alterna il DAC tra 0 e un livello a 2*f.
 *   Sequencer a "tracce": musica di sottofondo in loop + effetti sonori
 *   multi-nota con priorita'. Suona solo in ST_RUNNING.
 *----------------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "audio.h"
#include "game.h"
#include "timer/timer.h"

#define PCLK_TIMER  25000000u   /* PCLK del Timer1 (CCLK/4) */
#define DAC_LEVEL   (512u << 6) /* meta' scala -> volume medio (DACR bit 6..15) */

/* --- frequenze note (Hz) --- */
#define R    0      /* pausa */
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define C5 523
#define D5 587
#define E5 659
#define G5 784
#define A5 880
#define B5 988
#define C6 1047
#define D6 1175
#define E6 1319
#define G6 1568
#define C7 2093

typedef struct { unsigned short freq, dur; } Note;   /* dur in tick da 50 ms */

/* Musica di sottofondo (loop) */
static const Note MUSIC[] = {
    {E5,2},{C5,2},{E5,2},{G5,4},{R,1},
    {A4,2},{C5,2},{E5,2},{D5,4},{R,2},
    {G5,2},{E5,2},{C5,2},{D5,2},{E5,2},{C5,4},{R,2}
};
#define MUSIC_LEN ((int)(sizeof(MUSIC)/sizeof(MUSIC[0])))

/* Effetti sonori (multi-nota) */
static const Note SFX_PILL[]  = { {B5,1}, {R,1} };
static const Note SFX_POWER[] = { {G5,1}, {B5,1}, {D6,2} };
static const Note SFX_GHOST[] = { {E6,1}, {G6,1}, {C7,2} };
static const Note SFX_DEATH[] = { {E5,2}, {B5,1}, {G4,2}, {E4,2}, {C4,4}, {R,2} };

/* --- stato del sequencer --- */
static volatile int dac_high = 0;
static int playing = 0;

static int mus_i = 0, mus_cnt = 0;
static const Note *sfx = 0;
static int sfx_len = 0, sfx_i = 0, sfx_cnt = 0;

/* TIMER1: genera l'onda quadra alternando il DAC */
void audio_timer_isr(void)
{
    dac_high ^= 1;
    LPC_DAC->DACR = dac_high ? DAC_LEVEL : 0;
}

/* Imposta un tono (Hz). freq==0 => silenzio. */
static void tone(unsigned freq)
{
    if (freq == 0) {
        disable_timer(1);
        LPC_DAC->DACR = 0;
        dac_high = 0;
        return;
    }
    init_timer(1, 0, 0, 3, (PCLK_TIMER / (2u * freq)) - 1u);
    enable_timer(1);
}

void audio_init(void)
{
    LPC_PINCON->PINSEL1 &= ~(3u << 20);
    LPC_PINCON->PINSEL1 |=  (2u << 20);   /* P0.26 = AOUT (DAC) */
    LPC_DAC->DACR = 0;
    mus_i = 0; mus_cnt = 0;
    sfx = 0; sfx_len = 0; sfx_i = 0; sfx_cnt = 0;
    playing = 0;
}

static void start_sfx(const Note *s, int len)
{
    sfx = s; sfx_len = len; sfx_i = 0; sfx_cnt = 0;
}

void audio_sfx_pill(void)     { start_sfx(SFX_PILL,  (int)(sizeof(SFX_PILL) /sizeof(Note))); }
void audio_sfx_power(void)    { start_sfx(SFX_POWER, (int)(sizeof(SFX_POWER)/sizeof(Note))); }
void audio_sfx_eatghost(void) { start_sfx(SFX_GHOST, (int)(sizeof(SFX_GHOST)/sizeof(Note))); }
void audio_sfx_death(void)    { start_sfx(SFX_DEATH, (int)(sizeof(SFX_DEATH)/sizeof(Note))); }

void audio_tick(void)
{
    if (g.mode != ST_RUNNING) {           /* silenzio quando non si gioca */
        if (playing) { tone(0); playing = 0; }
        return;
    }

    if (sfx) {                            /* effetto in corso: ha priorita' */
        if (sfx_cnt <= 0) {
            if (sfx_i >= sfx_len) {       /* effetto finito: riprende la musica */
                sfx = 0;
                mus_cnt = 0;
            } else {
                tone(sfx[sfx_i].freq);
                sfx_cnt = sfx[sfx_i].dur;
                sfx_i++;
                playing = 1;
            }
        }
        if (sfx) { sfx_cnt--; return; }
    }

    if (mus_cnt <= 0) {                    /* confine di nota: nota successiva */
        tone(MUSIC[mus_i].freq);
        mus_cnt = MUSIC[mus_i].dur;
        mus_i = (mus_i + 1) % MUSIC_LEN;
        playing = 1;
    }
    mus_cnt--;
}
