#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "updater.h"

#define Color unsigned char
#define BLACK 0
#define WHITE 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define YELLOW 5
#define CYAN 6
#define MAGENTA 7
#define ORANGE 8
#define PINK 9
#define BROWN 10
#define LIGHT_BLUE 11
#define LIGHT_GREEN 12
#define GRAY 13
#define DARK_GRAY 14

int color_to_code(Color color) {
    switch (color) {
        case BLACK:
            return 0;
        case RED:
            return 1;
        case GREEN:
            return 2;
        case YELLOW:
            return 3;
        case BLUE:
            return 4;
        case MAGENTA:
            return 5;
        case CYAN:
            return 6;
        case WHITE:
            return 7;
        case ORANGE:
            return 208;
        case PINK:
            return 218;
        case BROWN:
            return 94;
        case LIGHT_BLUE:
            return 81;
        case LIGHT_GREEN:
            return 120;
        case GRAY:
            return 245;
        case DARK_GRAY:
            return 238;
        default:
            return 0;
    }
}

// ANSI エスケープシーケンスを生成（背景色指定）
void get_color_named(char* out, Color bg) {
    int code = color_to_code(bg);
    if (code == 0) {
        snprintf(out, sizeof out, "\x1b[0m");
    } else {
        // 前景色ではなく背景色(48)を使用するとスペースで色が見える
        snprintf(out, sizeof out, "\x1b[48;5;%dm", code);
    }
}

#define BUFFER_SIZE 128
static char RENDER_BUFFER[BUFFER_SIZE];
static int RENDER_BUFFER_INDEX = 0;
static Color COLOR_FIELD[22][12];

// 出力をバッファリングし、必要時にフラッシュ
void buffered_print(const char* arg, bool flush) {
    int len = strlen(arg);
    if (RENDER_BUFFER_INDEX + len >= BUFFER_SIZE) {
        RENDER_BUFFER[RENDER_BUFFER_INDEX] = '\0';
        printf("%s", RENDER_BUFFER);
        RENDER_BUFFER_INDEX = 0;
    }
    memcpy(&RENDER_BUFFER[RENDER_BUFFER_INDEX], arg, len);
    RENDER_BUFFER_INDEX += len;
    if (flush) {
        RENDER_BUFFER[RENDER_BUFFER_INDEX] = '\0';
        printf("%s", RENDER_BUFFER);
        RENDER_BUFFER_INDEX = 0;
    }
}

// 描画関数
void render(State state) {
    // バッファ初期化
    memset(RENDER_BUFFER, 0, BUFFER_SIZE);
    RENDER_BUFFER_INDEX = 0;
    // 画面クリア
    buffered_print("\x1b[2J", false);
    buffered_print("\x1b[H", false);

    // フィールドの枠をセット
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 12; x++) {
            if (y == 0 || y == 21 || x == 0 || x == 11) {
                COLOR_FIELD[y][x] = WHITE;
            } else {
                COLOR_FIELD[y][x] = BLACK;
            }
        }
    }

    // テトリミノのマージ
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (state.field[y][x]) {
                COLOR_FIELD[y + 1][x + 1] = WHITE;
            }
        }
    }

    // テトリミノの描画
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (state.mino.block[i][j]) {
                int y = state.mino.position.y + i;
                int x = state.mino.position.x + j;
                switch (state.mino.type) {
                    case T:
                        COLOR_FIELD[y + 1][x + 1] = MAGENTA;
                        break;
                    case S:
                        COLOR_FIELD[y + 1][x + 1] = GREEN;
                        break;
                    case Z:
                        COLOR_FIELD[y + 1][x + 1] = RED;
                        break;
                    case L:
                        COLOR_FIELD[y + 1][x + 1] = ORANGE;
                        break;
                    case J:
                        COLOR_FIELD[y + 1][x + 1] = BLUE;
                        break;
                    case O:
                        COLOR_FIELD[y + 1][x + 1] = YELLOW;
                        break;
                    case I:
                        COLOR_FIELD[y + 1][x + 1] = CYAN;
                        break;
                }
            }
        }
    }

    // 描画ループ
    char color_buffer[16];
    Color prev_color = WHITE;
    // バッファに色をセット
    for (int y = 0; y < 22; y++) {
        prev_color = WHITE;
        get_color_named(color_buffer, WHITE);
        buffered_print(color_buffer, false);
        for (int x = 0; x < 12; x++) {
            if (COLOR_FIELD[y][x] != prev_color) {
                get_color_named(color_buffer, COLOR_FIELD[y][x]);
                buffered_print(color_buffer, false);
                prev_color = COLOR_FIELD[y][x];
            }
            buffered_print("XX", false);
        }
        // 行末にリセットと改行を挿入
        buffered_print("\x1b[0m\n", false);
    }

    // 最終フラッシュ
    buffered_print("", true);
}
