/*----------------------------------------------------------------------------
 * can.h - CAN in loopback esterno (CAN1 TX, CAN2 RX) (Spec 3 EP2)
 *   Messaggio 32 bit: [time:8][lives:8][score:16]
 *----------------------------------------------------------------------------*/
#ifndef CAN_H
#define CAN_H

#include <stdint.h>

extern volatile uint32_t can_tx_msg;   /* ultimo messaggio inviato (CAN1) */
extern volatile uint32_t can_rx_msg;   /* ultimo messaggio ricevuto (CAN2) */
extern volatile int      can_due;      /* impostato ogni secondo da Timer0 */

void can_init(void);                   /* CAN1/CAN2 in modo normale (loopback via cavi) */
void can_send(uint32_t msg);           /* trasmette dal CAN1 */
int  can_receive(uint32_t *msg);       /* legge da CAN2 (1 se disponibile) */
void can_send_status(void);            /* codifica time/lives/score, invia e rilegge */

#endif /* CAN_H */
