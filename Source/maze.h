/*----------------------------------------------------------------------------
 * maze.h - Modello logico del labirinto (unica fonte di verita' per la logica)
 *----------------------------------------------------------------------------*/
#ifndef MAZE_H
#define MAZE_H

#include <stdint.h>

#define COLS        24      /* colonne griglia            */
#define ROWS        32      /* righe griglia              */
#define CELL        10      /* pixel per cella            */
#define MAZE_TOP     4      /* prima riga del labirinto   */
#define MAZE_BOT    29      /* ultima riga del labirinto  */

#define TOTAL_PILLS 240     /* Spec 1                     */

/* Tunnel (Spec 4): estremi della riga di teleport */
#define TUNNEL_ROW  19
#define TUNNEL_LCOL  0
#define TUNNEL_RCOL 23

/* Posizione iniziale di Pac-Man */
#define START_ROW   28
#define START_COL   11

typedef enum { WALL, PILL, POWER, EMPTY } CellType;

/* Stato dinamico della griglia: PILL/POWER diventano EMPTY quando mangiate. */
extern CellType grid[ROWS][COLS];

/* Costruisce grid[][] dal layout statico. Ritorna il numero di pillole (atteso 240). */
int maze_init(void);

#endif /* MAZE_H */
