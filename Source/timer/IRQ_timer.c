/*********************************************************************************************************
** File name:   IRQ_timer.c
** Descrizione: Timer0 -> countdown 1 s + invio CAN (Spec 8/10/3);  Timer1 -> toni speaker (Spec 2)
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"
#include "../game.h"
#include "../audio.h"
#include "../can.h"

void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1) {        /* MR0: 1 secondo */
        game_tick_second();        /* countdown + timer fantasma */
        can_due = 1;               /* EP2: invio stato su CAN (gestito nel main) */
        LPC_TIM0->IR = 1;          /* clear interrupt flag */
    }
}

void TIMER1_IRQHandler(void)
{
    if (LPC_TIM1->IR & 1) {        /* MR0: genera l'onda quadra dello speaker */
        audio_timer_isr();
        LPC_TIM1->IR = 1;          /* clear interrupt flag */
    }
}

/*********************************************************************************************************
**                            End Of File
*********************************************************************************************************/
