/*----------------------------------------------------------------------------
 * ghost.h - Fantasma AI (Blinky). Modi: Chase / Frightened / Eaten (Spec 1 EP2)
 *----------------------------------------------------------------------------*/
#ifndef GHOST_H
#define GHOST_H

#include "maze.h"
#include "game.h"

typedef enum { G_CHASE, G_FRIGHTENED, G_EATEN } GhostMode;

typedef struct {
    int row, col;
    GhostMode mode;
    int fright_timer;    /* secondi rimasti in frightened (max FRIGHT_SECS) */
    int eaten_timer;     /* secondi rimasti prima del respawn (max EATEN_SECS) */
} Ghost;

extern Ghost ghost;

#define GHOST_ROW   19      /* spawn centrale (central box, square vuoto) */
#define GHOST_COL   11
#define FRIGHT_SECS 10
#define EATEN_SECS   3

void ghost_init(void);            /* posiziona il fantasma allo spawn, modo Chase */
void ghost_step(void);            /* un passo di movimento (cadenza gestita dal main) */
void ghost_frighten(void);        /* entra in frightened (Pac-Man ha mangiato una power pill) */
void ghost_tick_second(void);     /* timer 1 s: frightened / respawn */
int  ghost_period_ticks(void);    /* cadenza attuale (velocita'): piu' piccola = piu' veloce */

#endif /* GHOST_H */
