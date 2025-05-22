#if defined(NATIVE_MODE)

int main() {}

#else

#include <stdio.h>

#include "io.h"
#include "peer.h"
#include "rendering.h"
#include "updater.h"

#define TICK_INTERVAL 50

unsigned long long TICK_COUNT = 0;
char last_player_input = 0;

State current_state = {0};

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
    }

    Output out = next_state(current_state, op, attack_lines);
    current_state = out.state;

    if (out.linesToSend > 0) {
        send_lines(out.linesToSend);
    }

    render(current_state);

    if (current_state.phase == GameOver) {
        printf("\x1b[0m");
        printf("Game Over!\n");
        return;
    }
}

void init() {
    current_state.free_fall_tick = 0;
    current_state.free_fall_interval = 20;
    current_state.lock_delay_interval = 20;
    current_state.phase = Spawning;
    current_state.score = 0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            current_state.field[i][j] = false;
        }
    }
}

void timer_handler() {
    if (current_state.phase == GameOver) {
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
