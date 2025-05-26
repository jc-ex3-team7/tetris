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
}

char *my_itoa(int value, char *buffer, int base) {
    if (base < 2 || base > 32) {
        return buffer;
    }
    int n = abs(value);
    int i = 0;
    while (n) {
        int r = n % base;
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        } else {
            buffer[i++] = 48 + r;
        }
        n = n / base;
    }
    if (i == 0) {
        buffer[i++] = '0';
    }
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
}

int my_strlen(const char *str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

void gpio_input(char *string, char *buf) {
    int len = my_strlen(string);
    int n = 0;
    for (int i = 0; i < len; i++) {
        buf[i] = string[i];
        n++;
    }
    for (int i = n; i < len; i++) {
        buf[i] = '0';
    }
}

void display_score_7seg(int score) {
#if defined(NATIVE_MODE)
    printf("do nothing...\n");
#else
    char temp_buf[20] = {0};
    char seg_buf[8] = {0};
    my_itoa(score, seg_buf, 10);
    gpio_input(temp_buf, seg_buf);
    set_gpio_string(
        GPIO_STR8(seg_buf[7], seg_buf[6], seg_buf[5], seg_buf[4], seg_buf[3], seg_buf[2], seg_buf[1], seg_buf[0]));

#endif
}

void timer_handler() {
    if (current_state.phase == PHASE_GAME_OVER) {
        return;
    }
    update(TICK_COUNT, last_player_input);
    last_player_input = 0;
    TICK_COUNT++;
    display_score_7seg(current_state.score);
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
