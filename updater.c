#include "updater.h"

#include <stdbool.h>

State next_state(State current_state, Operation op) {
    // Handle user operation
    Mino moved = move_mino(current_state.mino, op);
    if (is_mino_position_valid(current_state.field, moved)) {
        current_state.mino = moved;
    } else if (op == Drop) {
        // TODO: logic to handle hard drop
        // TODO: lock the mino in place
    }

    // Handle free fall
    if (current_state.free_fall_tick >= current_state.free_fall_interval) {
        Mino moved_down = move_mino(current_state.mino, Down);
        if (is_mino_position_valid(current_state.field, moved_down)) {
            current_state.mino = moved_down;
            current_state.free_fall_tick = 0;
        } else {
            // TODO: logic to lock the mino in place
        }
    } else {
        current_state.free_fall_tick++;
    }

    // TODO: Check for complete lines

    return current_state;
}

bool is_mino_position_valid(bool field[20][10], Mino mino) {
    // Check if the mino is within the bounds of the field
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (mino.block[i][j]) {
                int y = mino.position.y + i;
                int x = mino.position.x + j;
                if (x < 0 || x >= 10 || y < 0 || y >= 20 || field[y][x]) {
                    return false;
                }
            }
        }
    }
    return true;
}

Mino move_mino(Mino current, Operation op) {
    TetrisType type = current.type;
    Position position = current.position;
    bool next_block[4][4] = {0};

    switch (op) {
        case Right:
            position.x++;
            break;
        case Left:
            position.x--;
            break;
        case RotateLeft:
            switch (type) {
                case T:
                case S:
                case Z:
                case L:
                case J:
                    for (int y = 0; y < 3; y++)
                        for (int x = 0; x < 3; x++) {
                            next_block[3 - x][y] = current.block[y + 1][x];
                        }
                    break;
                case O:
                    break;
                case I:
                    for (int y = 0; y < 4; y++) {
                        for (int x = 0; x < 4; x++) {
                            next_block[3 - x][y] = current.block[y][x];
                        }
                    }
                    break;
            }
            break;
        case RotateRight:
            switch (type) {
                case T:
                case S:
                case Z:
                case L:
                case J:
                    for (int y = 0; y < 3; y++)
                        for (int x = 0; x < 3; x++) {
                            next_block[x + 1][2 - y] = current.block[y + 1][x];
                        }
                    break;
                case O:
                    break;
                case I:
                    for (int y = 0; y < 4; y++) {
                        for (int x = 0; x < 4; x++) {
                            next_block[x][3 - y] = current.block[y][x];
                        }
                    }
                    break;
            }
            break;
        case Down:
            position.y++;
            break;
    }

    return (Mino){type, position, next_block};
}
