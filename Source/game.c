/*----------------------------------------------------------------------------
 * game.c - Regole del gioco. Lavora SOLO sul modello (grid[][], g, ghost) e
 *          chiama le funzioni di rendering/audio. Nessuna lettura di pixel.
 *----------------------------------------------------------------------------*/
#include "game.h"
#include "ghost.h"
#include "render.h"
#include "audio.h"
#include <stdlib.h>

Game g;

extern volatile uint32_t g_ticks;    /* contatore tick RIT (IRQ_RIT.c) */
extern volatile uint32_t seed_ctr;   /* entropia per il seed  (IRQ_RIT.c) */

static uint32_t next_power_tick = 0;
static int      started = 0;

void game_init(void)
{
    g.mode          = ST_PAUSED;     /* Spec 7: parte in pausa */
    g.score         = 0;
    g.lives         = START_LIVES;
    g.time_left     = START_TIME;
    g.pills_left    = TOTAL_PILLS;
    g.power_spawned = 0;
    g.power_on_field= 0;
    g.next_life     = EXTRA_LIFE_STEP;
    g.pac.row       = START_ROW;
    g.pac.col       = START_COL;
    g.pac.dir       = DIR_NONE;
    g.pac.next      = DIR_NONE;
    started         = 0;
    ghost_init();                    /* EP2: fantasma allo spawn centrale */
}

static int spawn_interval(void)
{
    return SPAWN_MIN + (rand() % (SPAWN_MAX - SPAWN_MIN + 1));
}

void game_on_start(void)
{
    if (started) return;
    started = 1;
    srand(seed_ctr);                       /* Spec 2 EP1: seed casuale al primo input */
    next_power_tick = g_ticks + spawn_interval();
}

/* Calcola la cella destinazione in direzione d, gestendo il tunnel (Spec 4).
   Ritorna 1 se la cella e' calpestabile (non muro), 0 altrimenti. */
static int next_cell(int r, int c, Direction d, int *nr, int *nc)
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

static int can_move(int r, int c, Direction d)
{
    int a, b;
    return next_cell(r, c, d, &a, &b);
}

static void eat(int r, int c)
{
    if (grid[r][c] == PILL) {                  /* Spec 5: +10 */
        g.score += 10; g.pills_left--; grid[r][c] = EMPTY;
        render_score();
        audio_sfx_pill();
    } else if (grid[r][c] == POWER) {          /* Spec 5: +50 + frightened (EP2) */
        g.score += 50; g.pills_left--; g.power_on_field--; grid[r][c] = EMPTY;
        render_score();
        ghost_frighten();
        audio_sfx_power();
    }
}

static void check_extra_life(void)             /* Spec 6 */
{
    while (g.score >= g.next_life) {
        g.lives++;
        g.next_life += EXTRA_LIFE_STEP;
        render_lives();
    }
}

/* EP2: Pac-Man perde una vita -> reset posizioni (labirinto invariato). */
static void game_reset_positions(void)
{
    render_cell(g.pac.row, g.pac.col);
    render_cell(ghost.row, ghost.col);
    g.pac.row = START_ROW; g.pac.col = START_COL;
    g.pac.dir = DIR_NONE;  g.pac.next = DIR_NONE;
    ghost.row = GHOST_ROW; ghost.col = GHOST_COL;
    ghost.mode = G_CHASE;  ghost.fright_timer = 0;
    render_pac();
    render_ghost();
}

void game_check_contact(void)                  /* Spec 1 EP2 */
{
    if (g.mode != ST_RUNNING) return;
    if (ghost.mode == G_EATEN) return;
    if (ghost.row != g.pac.row || ghost.col != g.pac.col) return;

    if (ghost.mode == G_FRIGHTENED) {          /* Pac-Man mangia il fantasma: +100 */
        g.score += 100; render_score();
        ghost.mode = G_EATEN; ghost.eaten_timer = EATEN_SECS;
        render_cell(ghost.row, ghost.col);     /* rimuovi il fantasma dallo schermo */
        render_pac();
        audio_sfx_eatghost();
    } else {                                   /* Chase: Pac-Man perde una vita */
        g.lives--; render_lives();
        audio_sfx_death();
        if (g.lives <= 0) g.mode = ST_GAMEOVER;
        else game_reset_positions();
    }
}

void game_step(void)                           /* Spec 3 */
{
    int nr, nc;
    if (g.mode != ST_RUNNING) return;

    if (g.pac.next != DIR_NONE && can_move(g.pac.row, g.pac.col, g.pac.next))
        g.pac.dir = g.pac.next;

    if (g.pac.dir == DIR_NONE) return;

    if (!next_cell(g.pac.row, g.pac.col, g.pac.dir, &nr, &nc))
        return;                                /* muro davanti: fermo, aspetta input */

    render_cell(g.pac.row, g.pac.col);
    if (ghost.row == g.pac.row && ghost.col == g.pac.col)
        render_ghost();                        /* se il fantasma era sotto, ridisegnalo */
    g.pac.row = nr; g.pac.col = nc;
    eat(nr, nc);
    render_pac();
    check_extra_life();

    if (g.pills_left == 0) { g.mode = ST_VICTORY; return; }   /* Spec 9 */

    game_check_contact();                      /* EP2: Pac-Man e' entrato nel fantasma? */
}

void game_request_dir(Direction d)
{
    g.pac.next = d;
}

void game_toggle_pause(void)                   /* Spec 7 */
{
    if (g.mode == ST_PAUSED)       g.mode = ST_RUNNING;
    else if (g.mode == ST_RUNNING) g.mode = ST_PAUSED;
}

void game_tick_second(void)                    /* Spec 8/10 + timer fantasma EP2 */
{
    if (g.mode != ST_RUNNING) return;
    ghost_tick_second();
    if (g.time_left > 0) {
        g.time_left--;
        render_time();
    }
    if (g.time_left == 0) g.mode = ST_GAMEOVER;
}

void game_try_spawn_power(void)                /* Spec 2 EP1 */
{
    int tries, r, c;
    if (g.mode != ST_RUNNING) return;
    if (g.power_spawned >= POWER_MAX) return;
    if (g_ticks < next_power_tick) return;

    for (tries = 0; tries < 200; tries++) {
        r = MAZE_TOP + (rand() % (MAZE_BOT - MAZE_TOP + 1));
        c = rand() % COLS;
        if (grid[r][c] == PILL) {
            grid[r][c] = POWER;
            render_cell(r, c);
            g.power_spawned++;
            g.power_on_field++;
            break;
        }
    }
    next_power_tick = g_ticks + spawn_interval();
}
