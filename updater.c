#include "updater.h"

#include <stdbool.h>
#include <stdlib.h>

TetrisType mino_bag[7];
unsigned char mino_bag_index = 7;

TetrisType next_mino_type() {
    TetrisType temp;
    if (mino_bag_index >= 7) {
        for (int i = 0; i < 7; i++) {
            mino_bag[i] = i;
        }
        for (int i = 0; i < 7; i++) {
            int j = rand() % 7;
            temp = mino_bag[i];
            mino_bag[i] = mino_bag[j];
            mino_bag[j] = temp;
        }
        mino_bag_index = 0;
    }
    return mino_bag[mino_bag_index++];
}

Mino next_mino() {
    TetrisType type = next_mino_type();
    bool block[4][4] = {0};

    switch (type) {
        case T:
            block[1][1] = true;
            block[2][0] = true;
            block[2][1] = true;
            block[2][2] = true;
            break;
        case S:
            block[1][1] = true;
            block[1][2] = true;
            block[2][0] = true;
            block[2][1] = true;
            break;
        case Z:
            block[1][0] = true;
            block[1][1] = true;
            block[2][1] = true;
            block[2][2] = true;
            break;
        case L:
            block[1][2] = true;
            block[2][0] = true;
            block[2][1] = true;
            block[2][2] = true;
            break;
        case J:
            block[1][0] = true;
            block[2][0] = true;
            block[2][1] = true;
            block[2][2] = true;
            break;
        case O:
            block[1][1] = true;
            block[1][2] = true;
            block[2][1] = true;
            block[2][2] = true;
            break;
        case I:
            block[2][0] = true;
            block[2][1] = true;
            block[2][2] = true;
            block[2][3] = true;
            break;
    }
    return (Mino){type, {4, 0}, block};
}

State next_state(State current_state, Operation op) {
    if (current_state.phase == Spawning) {
        current_state.mino = next_mino();
        current_state.phase = Playing;

        return current_state;
    }

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

    int cleared_lines = clear_lines(current_state.field);
    if (cleared_lines > 0) {
        // TODO: score points
        switch (cleared_lines) {
            case 1:
                current_state.score += 100;
                break;
            case 2:
                current_state.score += 300;
                break;
            case 3:
                current_state.score += 500;
                break;
            case 4:
                current_state.score += 800;
                break;
            default:
                current_state.score += 0;
                break;
        }
    }

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

int clear_line(bool **field) {
    int lines_cleared = 0;
    for (int i = 0; i < 20; i++) {
        bool full_line = true;
        for (int j = 0; j < 10; j++) {
            if (!field[i][j]) {
                full_line = false;
                break;
            }
        }
        if (full_line) {
            lines_cleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < 10; j++) {
                    field[k][j] = field[k - 1][j];
                }
            }
            for (int j = 0; j < 10; j++) {
                field[0][j] = false;
            }
        }
    }
    return lines_cleared;
}

void update(unsigned long long tick_count, char player_input) {
    // TODO: Implement the update logic
}
