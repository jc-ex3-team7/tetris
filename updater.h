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
    Right,
    Left,
    RotateLeft,
    RotateRight,
    Drop,
} Operation;

Mino move_mino(Mino current, Operation op);

bool is_valid_state(bool field[20][10], Mino mino);

int get_complete_line(bool field[20][10]);

void update_field(bool **field);
