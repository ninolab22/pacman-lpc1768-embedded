/*----------------------------------------------------------------------------
 * render.c - Disegna lo stato del gioco. Usa solo le primitive di GLCD.
 *            Non legge mai i pixel: tutto deriva dal modello (grid[][], g).
 *----------------------------------------------------------------------------*/
#include "render.h"
#include "ghost.h"
#include "GLCD/GLCD.h"
#include <stdio.h>

/* Colori definiti in GLCD.h: White, Black, Blue, Red, Yellow */

/* --- posizioni HUD --- */
#define HUD_TIME_X     4
#define HUD_TIME_Y     10
#define HUD_TIME_VX    44
#define HUD_SCORE_X    140
#define HUD_SCORE_Y    10
#define HUD_SCORE_VX   188
#define LIVES_LABEL_X  0
#define LIVES_LABEL_Y  305
#define LIVES_ICON_X0  56
#define LIVES_ICON_Y   311
#define LIVES_STEP     18

/* --- messaggio PAUSE (dentro la central box, zona senza pillole) --- */
#define PAUSE_X   100
#define PAUSE_Y   185
#define PAUSE_R0  18
#define PAUSE_R1  20
#define PAUSE_C0  10
#define PAUSE_C1  13

static void fill_rect(int x, int y, int w, int h, uint16_t color)
{
    int j;
    for (j = 0; j < h; j++)
        LCD_DrawLine(x, y + j, x + w - 1, y + j, color);
}

void render_cell(int r, int c)
{
    int x = c * CELL, y = r * CELL;
    switch (grid[r][c]) {
        case WALL:
            fill_rect(x, y, CELL, CELL, Blue);
            break;
        case PILL:
            fill_rect(x, y, CELL, CELL, Black);
            LCD_DrawFilledCircle(x + CELL/2, y + CELL/2, 1, White);
            break;
        case POWER:
            fill_rect(x, y, CELL, CELL, Black);
            LCD_DrawFilledCircle(x + CELL/2, y + CELL/2, 3, Red);
            break;
        default: /* EMPTY */
            fill_rect(x, y, CELL, CELL, Black);
            break;
    }
}

void render_level(void)
{
    int r, c;
    for (r = MAZE_TOP; r <= MAZE_BOT; r++)
        for (c = 0; c < COLS; c++)
            render_cell(r, c);
}

void render_pac(void)
{
    LCD_DrawFilledCircle(g.pac.col * CELL + CELL/2,
                         g.pac.row * CELL + CELL/2, 4, Yellow);
}

void render_ghost(void)
{
    uint16_t col;
    if (ghost.mode == G_EATEN) return;          /* non disegnato finche' respawna */
    col = (ghost.mode == G_FRIGHTENED) ? Blue : Red;
    LCD_DrawFilledCircle(ghost.col * CELL + CELL/2,
                         ghost.row * CELL + CELL/2, 4, col);
}

void render_score(void)
{
    char buf[8];
    sprintf(buf, "%4d", g.score);
    GUI_Text(HUD_SCORE_VX, HUD_SCORE_Y, (uint8_t *)buf, White, Black);
}

void render_time(void)
{
    char buf[6];
    sprintf(buf, "%2d", g.time_left);
    GUI_Text(HUD_TIME_VX, HUD_TIME_Y, (uint8_t *)buf, White, Black);
}

void render_lives(void)
{
    int i, n = g.lives;
    if (n > 5) n = 5;
    fill_rect(LIVES_ICON_X0, LIVES_ICON_Y - 5, 5 * LIVES_STEP, 12, Black);
    for (i = 0; i < n; i++)
        LCD_DrawFilledCircle(LIVES_ICON_X0 + i * LIVES_STEP, LIVES_ICON_Y, 4, Yellow);
}

void render_hud(void)
{
    GUI_Text(HUD_TIME_X,    HUD_TIME_Y,    (uint8_t *)"TIME",  White, Black);
    GUI_Text(HUD_SCORE_X,   HUD_SCORE_Y,   (uint8_t *)"SCORE", White, Black);
    GUI_Text(LIVES_LABEL_X, LIVES_LABEL_Y, (uint8_t *)"LIVES", White, Black);
    render_time();
    render_score();
    render_lives();
}

void render_pause(void)
{
    GUI_Text(PAUSE_X, PAUSE_Y, (uint8_t *)"PAUSE", Red, Black);
}

void render_erase_pause(void)
{
    int r, c;
    for (r = PAUSE_R0; r <= PAUSE_R1; r++)
        for (c = PAUSE_C0; c <= PAUSE_C1; c++)
            render_cell(r, c);
}

void render_endscreen(GameMode mode)
{
    LCD_Clear(Black);
    if (mode == ST_VICTORY)
        GUI_Text(88, 150, (uint8_t *)"VICTORY!", Yellow, Black);   /* Spec 9  */
    else
        GUI_Text(84, 150, (uint8_t *)"GAME OVER", Red, Black);     /* Spec 10 */
}
