/*----------------------------------------------------------------------------
 * can.c - Driver CAN per LandTiger (LPC1768) in loopback ESTERNO.
 *   CAN1 = trasmettitore, CAN2 = ricevitore. CANH/CANL di CAN1 e CAN2 vanno
 *   collegati fisicamente sul connettore della board (vedi figura del PDF).
 *   Messaggio (32 bit) salvato in uint32_t:
 *       [ Remaining time : 8 ][ Remaining lives : 8 ][ Score : 16 ]
 *----------------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "can.h"
#include "game.h"

volatile uint32_t can_tx_msg = 0;
volatile uint32_t can_rx_msg = 0;
volatile int      can_due    = 0;

void can_init(void)
{
    LPC_SC->PCONP |= (1 << 13) | (1 << 14);   /* PCAN1, PCAN2 */

    /* Pin: P0.0 RD1 (01), P0.1 TD1 (01), P0.4 RD2 (10), P0.5 TD2 (10) */
    LPC_PINCON->PINSEL0 &= ~((3u << 0) | (3u << 2) | (3u << 8) | (3u << 10));
    LPC_PINCON->PINSEL0 |=  ((1u << 0) | (1u << 2) | (2u << 8) | (2u << 10));

    LPC_CAN1->MOD = 1;          /* reset mode */
    LPC_CAN2->MOD = 1;
    LPC_CAN1->IER = 0;          /* nessun interrupt: useremo polling */
    LPC_CAN2->IER = 0;
    LPC_CAN1->GSR = 0;
    LPC_CAN2->GSR = 0;

    /* Bit timing: stesso valore sui due controller.
       NB: dipende da PCLK_CAN; regolare se il baud non combacia sulla board. */
    LPC_CAN1->BTR = 0x001C001D;
    LPC_CAN2->BTR = 0x001C001D;

    LPC_CANAF->AFMR = 2;        /* Acceptance Filter: bypass (riceve tutto) */

    LPC_CAN1->MOD = 0;          /* normal mode */
    LPC_CAN2->MOD = 0;
}

void can_send(uint32_t msg)
{
    can_tx_msg = msg;
    LPC_CAN1->TFI1 = (4u << 16);     /* DLC = 4 byte */
    LPC_CAN1->TID1 = 0x100;          /* identificatore standard */
    LPC_CAN1->TDA1 = msg;            /* 4 byte di dati */
    LPC_CAN1->TDB1 = 0;
    LPC_CAN1->CMR  = (1u << 0) | (1u << 5);   /* TR + STB1 (trasmetti dal buffer 1) */
}

int can_receive(uint32_t *msg)
{
    if (LPC_CAN2->GSR & (1u << 0)) {  /* RBS: messaggio disponibile */
        *msg = LPC_CAN2->RDA;
        can_rx_msg = *msg;
        LPC_CAN2->CMR = (1u << 2);    /* RRB: release receive buffer */
        return 1;
    }
    return 0;
}

void can_send_status(void)
{
    uint32_t msg = ((uint32_t)(g.time_left & 0xFF) << 24)
                 | ((uint32_t)(g.lives     & 0xFF) << 16)
                 | ((uint32_t)(g.score   & 0xFFFF));
    uint32_t rx;
    volatile int w;

    can_send(msg);
    for (w = 0; w < 20000; w++) {     /* breve polling della ricezione (loopback) */
        if (can_receive(&rx)) break;
    }
}
