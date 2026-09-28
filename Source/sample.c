/*----------------------------------------------------------------------------
 * sample.c - Pac-Man per LandTiger / LPC1768
 *
 * EP1: modello a griglia (maze/game) + rendering (render), RIT + Timer0 + INT0.
 * EP2: fantasma AI (ghost), speaker (audio, DAC+Timer1), CAN loopback (can).
 *----------------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include "RIT/RIT.h"
#include "timer/timer.h"
#include "button_EXINT/button.h"
#include "joystick/joystick.h"
#include "maze.h"
#include "game.h"
#include "render.h"
#include "ghost.h"
#include "audio.h"
#include "can.h"

#ifdef SIMULATOR
extern uint8_t ScaleFlag;   /* visibile per l'emulatore LandTiger */
#endif

extern volatile uint32_t g_ticks;   /* definito in IRQ_RIT.c */

int main(void)
{
    uint32_t last_move  = 0;
    uint32_t last_ghost = 0;
    int pause_shown = 0;
    int end_shown   = 0;

    SystemInit();
    LCD_Initialization();
    LCD_Clear(Black);
    BUTTON_init();          /* EINT0/1/2 */
    joystick_init();
    audio_init();           /* EP2: DAC speaker */
    can_init();             /* EP2: CAN1/CAN2 loopback */

    maze_init();            /* grid[][] + 240 pillole */
    game_init();            /* stato: PAUSED, score 0, lives 1, time 60; ghost allo spawn */

    render_level();
    render_hud();
    render_pac();
    render_ghost();         /* EP2 */

    init_RIT(0x004C4B40);   /* 50 ms */
    enable_RIT();

    init_timer(0, 0, 0, 3, 25000000);   /* countdown 1 s */
    enable_timer(0);

    while (1) {
        __WFI();

        if (g.mode == ST_RUNNING) {
            game_on_start();

            if ((uint32_t)(g_ticks - last_move) >= MOVE_TICKS) {
                last_move = g_ticks;
                game_step();
            }
            if ((uint32_t)(g_ticks - last_ghost) >= (uint32_t)ghost_period_ticks()) {
                last_ghost = g_ticks;
                ghost_step();           /* EP2: passo del fantasma */
                game_check_contact();   /* il fantasma ha raggiunto Pac-Man? */
            }
            game_try_spawn_power();
        }

        if (can_due) {                  /* EP2: invio stato su CAN ogni secondo */
            can_due = 0;
            can_send_status();
        }

        /* overlay PAUSE (Spec 7) */
        if (g.mode == ST_PAUSED && !pause_shown) {
            render_pause();
            pause_shown = 1;
        } else if (g.mode == ST_RUNNING && pause_shown) {
            render_erase_pause();
            render_pac();
            render_ghost();
            pause_shown = 0;
        }

        /* schermate finali (Spec 9/10) */
        if ((g.mode == ST_VICTORY || g.mode == ST_GAMEOVER) && !end_shown) {
            render_endscreen(g.mode);
            end_shown = 1;
        }
    }
}
