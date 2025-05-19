#include <stdio.h>

#include "io.h"
#include "rendering.h"
#include "updater.h"

#define TICK_INTERVAL 50

unsigned long long TICK_COUNT = 0;
char last_player_input = 0;

State current_state;

void update(unsigned long long tick_count, char player_input) {
    Operation op = None;
    switch (player_input) {
        case 'a':
            op = Left;
            break;
        case 'd':
            op = Right;
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

    current_state = next_state(current_state, op);

    render(current_state);
}

void init() {
    current_state.free_fall_tick = 0;
    current_state.free_fall_interval = 1000;
    current_state.phase = Spawning;
    current_state.score = 0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            current_state.field[i][j] = false;
        }
    }
}

void timer_handler() {
    update(TICK_COUNT, last_player_input);
    last_player_input = 0;
    TICK_COUNT++;
}

int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    init();
    set_mtimer_interval(TICK_INTERVAL /*msec*/);
    timer_interrupt_hook = timer_handler;
    enable_timer_interrupt();
    while (1) {
        if (io_getch()) {
            last_player_input = io_getch();
        }
    }
#endif
    return 0;
}
