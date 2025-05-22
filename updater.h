#pragma once

#include <stdbool.h>

typedef enum {
    T,
    S,
    Z,
    L,
    J,
    O,
    I,
} TetrisType;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    TetrisType type;
    Position position;
    bool block[4][4];
} Mino;

typedef enum {
    None,
    Right,
    Left,
    Down,
    RotateLeft,
    RotateRight,
    Drop,
} Operation;

typedef enum { Playing, LockDelay, Spawning, GameOver } GamePhase;

typedef struct {
    bool field[20][10];
    Mino mino;

    GamePhase phase;

    int free_fall_tick;
    int free_fall_interval;
    int lock_delay_tick;
    int lock_delay_interval;
    int score;
    int attack_lines;
} State;

typedef struct {
    State state;
    int linesToSend;
} Output;

Mino next_mino();

Mino move_mino(Mino current, Operation op);

bool is_mino_position_valid(bool field[20][10], Mino mino);

// returns the nnumber of lines cleared
int clear_lines(bool field[20][10]);

Output next_state(State current_state, Operation op, int attack_lines);

void hard_drop(State *state);
void lock_mino(State *state);
