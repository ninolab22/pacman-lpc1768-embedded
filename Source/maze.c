/*----------------------------------------------------------------------------
 * maze.c - Layout statico del labirinto e conversione nel modello grid[][]
 *
 * Layout (26 righe x 24 colonne, righe MAZE_TOP..MAZE_BOT):
 *   '#' = muro      '.' = pillola standard
 *   ' ' = vuoto (interno della central box)   '-' = posizione iniziale (vuoto)
 *
 * Verificato: esattamente 240 pillole.  Tunnel: (19,0) <-> (19,23).
 *----------------------------------------------------------------------------*/
#include "maze.h"

CellType grid[ROWS][COLS];

static const char* const MAZE[26] = {
    "########################",
    "#.........###..........#",
    "#.###.###.###.####.#.#.#",
    "#.###.###.###.####.#.#.#",
    "#.###.###.###.####.....#",
    "#.###.###.###.#####.##.#",
    "#.##..###.###.#####.##.#",
    "#......................#",
    "#.###.###.####.#.#.###.#",
    "#.###.###.####.#.#.###.#",
    "#.....###.####.#.#.....#",
    "#####.##.......#.#.#####",
    "#####....## ##...#.#####",
    "#####.##.#   #.#.#.#####",
    "#####.##.#   #.#.#.#####",
    "......##.#   #.#.#......",
    "#####....#   #.#.#.#####",
    "#####.##.#   #.#.#.#####",
    "#####.##.#   #.....#####",
    "#####.##.#####.#.#.#####",
    "#..............#.#.....#",
    "#.##.###.##.##.#.#.###.#",
    "#.##.....##....#.#.###.#",
    "#.##.###.#####.#.#.###.#",
    "#..........-...........#",
    "########################"
};

int maze_init(void)
{
    int r, c, pills = 0;

    /* default: tutto vuoto (incluse le righe HUD fuori dal labirinto) */
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            grid[r][c] = EMPTY;

    /* conversione del layout statico */
    for (r = 0; r < 26; r++) {
        int gr = MAZE_TOP + r;
        for (c = 0; c < COLS; c++) {
            char ch = MAZE[r][c];
            switch (ch) {
                case '#': grid[gr][c] = WALL;             break;
                case '.': grid[gr][c] = PILL; pills++;    break;
                default:  grid[gr][c] = EMPTY;            break; /* ' ' e '-' */
            }
        }
    }
    return pills;
}
