/*----------------------------------------------------------------------------
 * audio.h - Speaker (DAC P0.26 + Timer1): musica di sottofondo + effetti (Spec 2 EP2)
 *----------------------------------------------------------------------------*/
#ifndef AUDIO_H
#define AUDIO_H

void audio_init(void);          /* configura DAC su P0.26 */
void audio_tick(void);          /* avanzamento sequencer, ogni 50 ms (da RIT) */
void audio_timer_isr(void);     /* toggle onda quadra, chiamata da TIMER1_IRQHandler */

void audio_sfx_pill(void);      /* effetto: pillola mangiata */
void audio_sfx_power(void);     /* effetto: power pill */
void audio_sfx_eatghost(void);  /* effetto: fantasma mangiato */
void audio_sfx_death(void);     /* effetto: vita persa */

#endif /* AUDIO_H */
