/*********************************************************************************************************
** File name:   IRQ_button.c
** Descriptions: INT0 (EINT0) -> richiesta di pausa, confermata con debounce nel RIT
*********************************************************************************************************/
#include "button.h"
#include "LPC17xx.h"

extern volatile int down_0;   /* definito in IRQ_RIT.c */

void EINT0_IRQHandler(void)   /* INT0 -> pausa/riprendi */
{
    down_0 = 1;
    NVIC_DisableIRQ(EINT0_IRQn);          /* disabilita finche' non rilasciato */
    LPC_PINCON->PINSEL4 &= ~(1 << 20);    /* pin a GPIO per leggerne lo stato  */
    LPC_SC->EXTINT = (1 << 0);            /* clear pending (write-1-to-clear)  */
}

void EINT1_IRQHandler(void)   /* KEY1 non usato */
{
    LPC_SC->EXTINT = (1 << 1);
}

void EINT2_IRQHandler(void)   /* KEY2 non usato */
{
    LPC_SC->EXTINT = (1 << 2);
}

/*********************************************************************************************************
**                            End Of File
*********************************************************************************************************/
