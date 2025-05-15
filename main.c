#include "io.h"
#include "updater.h"

#define TICK_INTERVAL 50

unsigned long long TICK_COUNT = 0;
char last_player_input = 0;

void timer_handler() {
    update(TICK_COUNT, last_player_input);
    last_player_input = 0;
    TICK_COUNT++;
}

int main() {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    set_mtimer_interval(TICK_INTERVAL /*msec*/);
    timer_interrupt_hook = timer_handler;
    enable_timer_interrupt();
    int timer_on = 1;
    printf("timer on\n");
    while (1) {
        if (io_getch()) {
            last_player_input = io_getch();
        }
    }
#endif
    return 0;
}
