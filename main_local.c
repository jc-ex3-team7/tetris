#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

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

    Output output = next_state(current_state, op, 0);
    current_state = output.state;

    render(current_state);
}

void init() {
    current_state.free_fall_tick = 0;
    current_state.free_fall_interval = 15;
    current_state.phase = Spawning;
    current_state.score = 0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            current_state.field[i][j] = false;
        }
    }

    srand((unsigned int)time(NULL));
}

void timer_handler() {
    update(TICK_COUNT, last_player_input);
    last_player_input = 0;
    TICK_COUNT++;
}

void* worker_thread(void* arg) {
    while (1) {
        usleep(50 * 1000);  // sleep 100ms
        timer_handler();
    }
    return NULL;
}

int main() {
    printf("\033[H\033[2J");
    struct termios oldt, newt;
    char ch;

    // 端末の現在設定を保存
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // ノンカノニカルモード、エコーなしに設定
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    init();
    pthread_t thread;

    // スレッド生成
    if (pthread_create(&thread, NULL, worker_thread, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    while (1) {
        char c = getchar();
        if (c == 'q') {
            break;  // 'q'が押されたら終了
        }
        if (c) {
            last_player_input = c;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // スレッドの終了待ち（本例では無限ループなので到達しない）
    pthread_join(thread, NULL);

    printf("exit\n");

    return 0;
}
