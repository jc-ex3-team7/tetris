#if defined(NATIVE_MODE)

int main() {}

#else

#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "peer.h"
#include "rendering.h"
#include "updater.h"

#define TICK_INTERVAL 50

unsigned long long TICK_COUNT = 0;
char last_player_input = 0;

State current_state = {0};
bool is_game_started = false;
bool is_ready_sent = false;
bool is_ready_received = false;
bool is_seed_received = false;
int my_seed = 0;
int enemy_seed = 0;

void update(unsigned long long tick_count, char player_input) {
    Operation op = None;
    switch (player_input) {
        case 'a':
            op = Left;
            break;
        case 'd':
            op = Right;
            break;
        case 's':
            op = Down;
            break;
        case 'l':
            op = RotateLeft;
            break;
        case 'j':
            op = RotateRight;
            break;
        case ' ':
            op = Drop;
            break;
        default:
            break;
    }

    DataPacket packet = receive_data();
    int attack_lines = 0;
    if (packet.type == ATTACK_LINES) {
        attack_lines = packet.data.lines;
    } else if (packet.type == GAME_OVER) {
        printf("\x1b[0m");
        printf("You Won!\n");
        current_state.phase = PHASE_GAME_OVER;
        return;
    } else if (packet.type == READY) {
        is_ready_received = true;
    } else if (packet.type == SEED) {
        is_seed_received = true;
        printf("\x1b[0m");
        printf("Seed received: %d\n", packet.data.seed);
    }

    if (!is_game_started) {
        if (player_input > 0 && !is_ready_sent) {
            is_ready_sent = true;
            send_ready();
            current_state.phase = PHASE_SPAWNING;
        }
        if (is_ready_received) {
            my_seed = tick_count & 0xFF;
            send_seed(my_seed);
        }
        if (is_seed_received) {
            enemy_seed = packet.data.seed;
            int seed = 0x0F0F0F0F ^ (my_seed + enemy_seed);
            srand(seed);
            is_game_started = true;
        }
        return;
    }

    Output out = next_state(current_state, op, attack_lines);
    current_state = out.state;

    if (out.linesToSend > 0) {
        send_lines(out.linesToSend);
    }

    render(current_state);

    if (current_state.phase == PHASE_GAME_OVER) {
        printf("\x1b[0m");
        printf("Game Over!\n");
        printf("You Lost!\n");
        send_game_over();
        return;
    }
}

void init() {
    current_state.free_fall_tick = 0;
    current_state.free_fall_interval = 20;
    current_state.lock_delay_interval = 20;
    current_state.phase = PHASE_SPAWNING;
    current_state.score = 0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            current_state.field[i][j] = false;
        }
    }

    printf("\x1b[2J");    // Clear the screen
    printf("\x1b[H");     // Move cursor to home position
    printf("\x1b[?25l");  // Hide cursor
    printf("Press any key to ready for the game...\n");
}

void timer_handler() {
    if (current_state.phase == PHASE_GAME_OVER) {
        return;
    }
    update(TICK_COUNT, last_player_input);
    last_player_input = 0;
    TICK_COUNT++;
}

int main() {
    init();
    set_mtimer_interval(TICK_INTERVAL /*msec*/);
    timer_interrupt_hook = timer_handler;
    enable_timer_interrupt();
    char c;
    while (1) {
        if (c = io_getch()) {
            last_player_input = c;
        }
    }
    return 0;
}

#endif
