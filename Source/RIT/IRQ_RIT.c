/*********************************************************************************************************
** File name:   IRQ_RIT.c
** Descrizione: RIT (50 ms) -> tick di gioco, joystick, debounce INT0, sequencer audio
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "../game.h"
#include "../audio.h"

volatile uint32_t g_ticks  = 0;   /* tempo di gioco in tick da 50 ms */
volatile uint32_t seed_ctr = 0;   /* entropia per il seed di rand()  */
volatile int      down_0   = 0;   /* stato debounce INT0 (set in IRQ_button.c) */

/* Joystick su PORT1: UP=29 DOWN=26 LEFT=27 RIGHT=28 (attivi bassi) */
static void input_sample(void)
{
    if      ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0) game_request_dir(DIR_UP);
    else if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0) game_request_dir(DIR_DOWN);
    else if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0) game_request_dir(DIR_LEFT);
    else if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0) game_request_dir(DIR_RIGHT);
}

void RIT_IRQHandler(void)
{
    g_ticks++;
    seed_ctr++;

    input_sample();
    audio_tick();                 /* EP2: avanza musica/effetti */

    /* INT0: toggle pausa con debounce (azione singola alla pressione) */
    if (down_0 != 0) {
        down_0++;
        if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0) {
            if (down_0 == 2) game_toggle_pause();
        } else {
            down_0 = 0;
            NVIC_EnableIRQ(EINT0_IRQn);
            LPC_PINCON->PINSEL4 |= (1 << 20);
        }
    }

    reset_RIT();
    LPC_RIT->RICTRL |= 0x1;       /* clear interrupt flag */
}

/*********************************************************************************************************
**                            End Of File
*********************************************************************************************************/
