/*----------------------------------------------------------------------------
 * ghost.c - AI del fantasma Blinky.
 *   Pathfinding: BFS distance-field dalla cella di Pac-Man (equivalente ad A*
 *   con euristica nulla su griglia non pesata -> cammino minimo garantito).
 *   Chase  : passo verso la distanza MINIMA (insegue).
 *   Frightened: passo verso la distanza MASSIMA (fugge).
 *----------------------------------------------------------------------------*/
#include "ghost.h"
#include "render.h"

Ghost ghost;

static int dist[ROWS][COLS];
static int qr[ROWS * COLS];
static int qc[ROWS * COLS];

/* Vicino calpestabile in direzione d (gestisce il tunnel). 1 se valido. */
static int walk_neighbor(int r, int c, int d, int *nr, int *nc)
{
    int tr = r, tc = c;
    switch (d) {
        case DIR_UP:    tr = r - 1; break;
        case DIR_DOWN:  tr = r + 1; break;
        case DIR_LEFT:  tc = c - 1; break;
        case DIR_RIGHT: tc = c + 1; break;
        default: return 0;
    }
    if (d == DIR_LEFT  && r == TUNNEL_ROW && c == TUNNEL_LCOL) { tr = TUNNEL_ROW; tc = TUNNEL_RCOL; }
    else if (d == DIR_RIGHT && r == TUNNEL_ROW && c == TUNNEL_RCOL) { tr = TUNNEL_ROW; tc = TUNNEL_LCOL; }

    if (tr < 0 || tr >= ROWS || tc < 0 || tc >= COLS) return 0;
    if (grid[tr][tc] == WALL) return 0;
    *nr = tr; *nc = tc;
    return 1;
}

/* BFS: riempie dist[][] con la distanza minima da (sr,sc). */
static void bfs_from(int sr, int sc)
{
    int head = 0, tail = 0, r, c, d, nr, nc;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            dist[r][c] = -1;

    dist[sr][sc] = 0;
    qr[tail] = sr; qc[tail] = sc; tail++;

    while (head < tail) {
        r = qr[head]; c = qc[head]; head++;
        for (d = DIR_UP; d <= DIR_RIGHT; d++) {
            if (walk_neighbor(r, c, d, &nr, &nc) && dist[nr][nc] < 0) {
                dist[nr][nc] = dist[r][c] + 1;
                qr[tail] = nr; qc[tail] = nc; tail++;
            }
        }
    }
}

void ghost_init(void)
{
    ghost.row = GHOST_ROW;
    ghost.col = GHOST_COL;
    ghost.mode = G_CHASE;
    ghost.fright_timer = 0;
    ghost.eaten_timer  = 0;
}

void ghost_frighten(void)
{
    if (ghost.mode != G_EATEN) {
        ghost.mode = G_FRIGHTENED;
        ghost.fright_timer = FRIGHT_SECS;
    }
}

void ghost_tick_second(void)
{
    if (ghost.mode == G_FRIGHTENED) {
        if (ghost.fright_timer > 0) ghost.fright_timer--;
        if (ghost.fright_timer == 0) ghost.mode = G_CHASE;
    } else if (ghost.mode == G_EATEN) {
        if (ghost.eaten_timer > 0) ghost.eaten_timer--;
        if (ghost.eaten_timer == 0) {       /* respawn al centro, torna a inseguire */
            ghost.row = GHOST_ROW;
            ghost.col = GHOST_COL;
            ghost.mode = G_CHASE;
            render_ghost();
        }
    }
}

int ghost_period_ticks(void)
{
    /* piu' veloce col progredire della partita: da 4 a 2 tick */
    int elapsed = START_TIME - g.time_left;     /* 0..60 */
    int p = 4 - elapsed / 25;                    /* 4, 3, 2 */
    if (ghost.mode == G_FRIGHTENED) p += 2;      /* piu' lento quando spaventato */
    if (p < 2) p = 2;
    return p;
}

void ghost_step(void)
{
    int d, nr, nc, best_r, best_c, best, found = 0;

    if (g.mode != ST_RUNNING) return;
    if (ghost.mode == G_EATEN) return;           /* fermo finche' non respawna */

    bfs_from(g.pac.row, g.pac.col);

    best_r = ghost.row; best_c = ghost.col;
    best = (ghost.mode == G_FRIGHTENED) ? -1 : 1000000;

    for (d = DIR_UP; d <= DIR_RIGHT; d++) {
        if (!walk_neighbor(ghost.row, ghost.col, d, &nr, &nc)) continue;
        if (dist[nr][nc] < 0) continue;
        if (ghost.mode == G_FRIGHTENED) {
            if (dist[nr][nc] > best) { best = dist[nr][nc]; best_r = nr; best_c = nc; found = 1; }
        } else {
            if (dist[nr][nc] < best) { best = dist[nr][nc]; best_r = nr; best_c = nc; found = 1; }
        }
    }
    if (!found) return;

    render_cell(ghost.row, ghost.col);                       /* ripristina la cella lasciata */
    if (ghost.row == g.pac.row && ghost.col == g.pac.col)
        render_pac();                                        /* se Pac-Man era sotto, ridisegnalo */

    ghost.row = best_r;
    ghost.col = best_c;
    render_ghost();
    /* il controllo del contatto e' fatto da game_check_contact() dopo lo step */
}
