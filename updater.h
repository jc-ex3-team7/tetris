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
    int score;
} State;

Mino next_mino();

Mino move_mino(Mino current, Operation op);

bool is_mino_position_valid(bool field[20][10], Mino mino);

// returns the nnumber of lines cleared
int clear_lines(bool **field);

State next_state(State current_state, Operation op);

void update(unsigned long long tick_count, char player_input);
