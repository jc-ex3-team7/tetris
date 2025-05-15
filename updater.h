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
    RotateLeft,
    RotateRight,
    Drop,
} Operation;

typedef struct {
    bool field[20][10];
    Mino mino;
} State;

Mino move_mino(Mino current, Operation op);

bool is_mino_position_valid(bool field[20][10], Mino mino);

int get_complete_line(bool field[20][10]);

State next_state(State current_state, Operation op);
