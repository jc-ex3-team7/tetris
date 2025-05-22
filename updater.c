#include "updater.h"

#include <stdbool.h>
#include <stdlib.h>

TetrisType mino_bag[7];
unsigned char mino_bag_index = 7;

TetrisType next_mino_type() {
    if (mino_bag_index >= 7) {
        for (int i = 0; i < 7; i++) {
            mino_bag[i] = i;
        }
        for (int i = 6; i > 0; i--) {
            int j = rand() % (i + 1);
            TetrisType temp = mino_bag[i];
            mino_bag[i] = mino_bag[j];
            mino_bag[j] = temp;
        }
        mino_bag_index = 0;
    }
    return mino_bag[mino_bag_index++];
}

Mino next_mino() {
    Mino res = {0};
    res.type = next_mino_type();
    res.position.x = 3;
    res.position.y = 0;

    switch (res.type) {
        case T:
            res.block[1][1] = true;
            res.block[2][0] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            break;
        case S:
            res.block[1][1] = true;
            res.block[1][2] = true;
            res.block[2][0] = true;
            res.block[2][1] = true;
            break;
        case Z:
            res.block[1][0] = true;
            res.block[1][1] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            break;
        case L:
            res.block[1][2] = true;
            res.block[2][0] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            break;
        case J:
            res.block[1][0] = true;
            res.block[2][0] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            break;
        case O:
            res.block[1][1] = true;
            res.block[1][2] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            break;
        case I:
            res.block[2][0] = true;
            res.block[2][1] = true;
            res.block[2][2] = true;
            res.block[2][3] = true;
            break;
    }

    return res;
}

static bool can_spawn_mino(State *state, Mino mino) { return is_mino_position_valid(state->field, mino); }

static void handle_spawning_phase(State *state) {
    // handle the opponent's attack
    if (state->attack_lines > 0) {
        for (int i = state->attack_lines; i < 20; i++) {
            for (int j = 0; j < 10; j++) {
                state->field[i - state->attack_lines][j] = state->field[i][j];
            }
        }
        int empty = rand() % 10;
        for (int i = 20 - state->attack_lines; i < 20; i++) {
            for (int j = 0; j < 10; j++) {
                if (j == empty) {
                    state->field[i][j] = false;
                } else {
                    state->field[i][j] = true;
                }
            }
        }
        state->attack_lines = 0;
    }

    state->mino = next_mino();
    if (!can_spawn_mino(state, state->mino)) {
        state->phase = GameOver;
        return;
    }
    state->phase = Playing;
}

static void handle_lock_delay_phase(State *state) {
    if (is_mino_position_valid(state->field, move_mino(state->mino, Down))) {
        state->phase = Playing;
        state->lock_delay_tick = 0;
    } else {
        state->lock_delay_tick++;
        if (state->lock_delay_tick >= state->lock_delay_interval) {
            lock_mino(state);
        }
    }
}

Output next_state(State current_state, Operation op, int attack_lines) {  // TODO: handle attack_lines
    Output res = {0};
    res.state = current_state;
    current_state.attack_lines += attack_lines;

    if (current_state.phase == GameOver) {
        res.state = current_state;
        return res;
    }

    switch (current_state.phase) {
        case Spawning:
            handle_spawning_phase(&current_state);
            res.state = current_state;
            return res;
        case LockDelay:
            handle_lock_delay_phase(&current_state);
            break;
    }

    // Handle user operation
    Mino moved = move_mino(current_state.mino, op);
    if (is_mino_position_valid(current_state.field, moved)) {
        current_state.mino = moved;
    } else if (op == Down && current_state.phase == Playing) {
        current_state.lock_delay_tick = 0;
        current_state.phase = LockDelay;
    }
    if (op == Drop) {
        hard_drop(&current_state);
        lock_mino(&current_state);
    }

    // Handle free fall
    if (op == Down) {
        current_state.free_fall_tick = 0;  // no free fall
    }
    if (current_state.free_fall_tick >= current_state.free_fall_interval) {
        Mino moved_down = move_mino(current_state.mino, Down);
        if (is_mino_position_valid(current_state.field, moved_down)) {
            current_state.mino = moved_down;
            current_state.free_fall_tick = 0;
        } else {
            lock_mino(&current_state);
        }
    } else {
        current_state.free_fall_tick++;
    }

    int cleared_lines = clear_lines(current_state.field);
    if (cleared_lines > 0) {
        res.linesToSend = cleared_lines;
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

    res.state = current_state;
    return res;
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
    Mino res = {0};

    res.type = current.type;
    res.position = current.position;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res.block[i][j] = current.block[i][j];
        }
    }

    switch (op) {
        case Right:
            res.position.x++;
            break;
        case Left:
            res.position.x--;
            break;
        case RotateLeft:
            switch (res.type) {
                case T:
                case S:
                case Z:
                case L:
                case J:
                    for (int y = 0; y < 3; y++)
                        for (int x = 0; x < 3; x++) {
                            res.block[x + 1][2 - y] = current.block[y + 1][x];
                        }
                    break;
                case O:
                    break;
                case I:
                    for (int y = 0; y < 4; y++) {
                        for (int x = 0; x < 4; x++) {
                            res.block[x][3 - y] = current.block[y][x];
                        }
                    }
                    break;
            }
            break;
        case RotateRight:
            switch (res.type) {
                case T:
                case S:
                case Z:
                case L:
                case J:
                    for (int y = 0; y < 3; y++)
                        for (int x = 0; x < 3; x++) {
                            res.block[3 - x][y] = current.block[y + 1][x];
                        }
                    break;
                case O:
                    break;
                case I:
                    for (int y = 0; y < 4; y++) {
                        for (int x = 0; x < 4; x++) {
                            res.block[3 - x][y] = current.block[y][x];
                        }
                    }
                    break;
            }
            break;
        case Down:
            res.position.y++;
            break;
        default:
            break;
    }

    return res;
}

int clear_lines(bool field[20][10]) {
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

void hard_drop(State *state) {
    while (is_mino_position_valid(state->field, move_mino(state->mino, Down))) {
        state->mino.position.y++;
    }
}

void lock_mino(State *state) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (state->mino.block[i][j]) {
                int y = state->mino.position.y + i;
                int x = state->mino.position.x + j;
                state->field[y][x] = true;
            }
        }
    }
    state->phase = Spawning;
}
