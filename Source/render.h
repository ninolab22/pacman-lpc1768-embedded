/*----------------------------------------------------------------------------
 * render.h - Proiezione del modello (grid[][], g) sul display LCD
 *----------------------------------------------------------------------------*/
#ifndef RENDER_H
#define RENDER_H

#include "maze.h"
#include "game.h"

void render_level(void);          /* disegna tutto il labirinto dal modello */
void render_cell(int r, int c);   /* ridisegna una singola cella            */
void render_pac(void);            /* disegna Pac-Man nella posizione attuale */
void render_ghost(void);          /* disegna il fantasma (rosso/blu, nulla se eaten) */
void render_hud(void);            /* etichette + valori iniziali            */
void render_score(void);          /* aggiorna il valore SCORE               */
void render_time(void);           /* aggiorna il valore countdown           */
void render_lives(void);          /* aggiorna le icone vite                 */
void render_pause(void);          /* mostra "PAUSE"                         */
void render_erase_pause(void);    /* cancella "PAUSE" ridisegnando dal modello */
void render_endscreen(GameMode mode); /* schermata Victory / Game Over     */

#endif /* RENDER_H */
