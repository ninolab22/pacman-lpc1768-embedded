/*----------------------------------------------------------------------------
 * game.h - Stato e regole del gioco (nessun accesso ai pixel/hardware)
 *----------------------------------------------------------------------------*/
#ifndef GAME_H
#define GAME_H

#include "maze.h"

typedef enum { DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT } Direction;
typedef enum { ST_PAUSED, ST_RUNNING, ST_VICTORY, ST_GAMEOVER } GameMode;

typedef struct { int row, col; Direction dir, next; } Pac;

typedef struct {
    GameMode mode;
    int score;
    int lives;
    int time_left;       /* secondi rimanenti (Spec 8)              */
    int pills_left;      /* da 240 a 0 -> vittoria (Spec 9)          */
    int power_spawned;   /* power pill totali generate (<= 6)        */
    int power_on_field;  /* power pill attualmente presenti          */
    int next_life;       /* prossima soglia per vita extra (Spec 6)  */
    Pac pac;
} Game;

extern Game g;

/* Parametri di gioco (documentare nel PDF) */
#define START_LIVES      1
#define START_TIME       60
#define EXTRA_LIFE_STEP  1000
#define POWER_MAX        6
#define MOVE_TICKS       3      /* passo Pac-Man ogni 3 tick RIT (~150 ms) */
#define SPAWN_MIN        40     /* tick (~2 s)  intervallo min power pill   */
#define SPAWN_MAX        120    /* tick (~6 s)  intervallo max power pill   */

void game_init(void);            /* stato iniziale: PAUSED */
void game_step(void);            /* un passo di movimento (Spec 3/4/5/9) */
void game_request_dir(Direction d);  /* input joystick (Spec 3) */
void game_toggle_pause(void);    /* INT0 (Spec 7) */
void game_tick_second(void);     /* Timer0: 1 secondo (Spec 8/10) */
void game_try_spawn_power(void); /* scheduler power pill (Spec 2) */
void game_on_start(void);        /* seed RNG + schedule al primo RUNNING */
void game_check_contact(void);   /* EP2: collisione Pac-Man / fantasma */

#endif /* GAME_H */
