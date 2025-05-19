#include <stdbool.h>
#include <stdio.h>

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

int color_to_code(char* out, Color color) {
    out[0] = '\x1b';
    out[1] = '[';
    out[2] = '3';
    out[3] = '8';
    out[4] = ';';
    out[5] = '5';
    out[6] = ';';
    int index = 7;
    switch (color) {
        case BLACK:
        default:
            out[0] = '\x1b';
            out[1] = '[';
            out[2] = '0';
            out[3] = 'm';
            out[4] = '\0';
            return 4;
        case RED:
            out[index++] = '1';
            break;
        case GREEN:
            out[index++] = '2';
            break;
        case YELLOW:
            out[index++] = '3';
            break;
        case BLUE:
            out[index++] = '4';
            break;
        case MAGENTA:
            out[index++] = '5';
            break;
        case CYAN:
            out[index++] = '6';
            break;
        case WHITE:
            out[index++] = '7';
            break;
        case ORANGE:
            out[index++] = '2';
            out[index++] = '0';
            out[index++] = '8';
            break;
        case PINK:
            out[index++] = '2';
            out[index++] = '1';
            out[index++] = '8';
            break;
        case BROWN:
            out[index++] = '9';
            out[index++] = '4';
            break;
        case LIGHT_BLUE:
            out[index++] = '8';
            out[index++] = '1';
            break;
        case LIGHT_GREEN:
            out[index++] = '1';
            out[index++] = '2';
            out[index++] = '0';
            break;
        case GRAY:
            out[index++] = '2';
            out[index++] = '4';
            out[index++] = '5';
            break;
        case DARK_GRAY:
            out[index++] = '2';
            out[index++] = '3';
            out[index++] = '8';
            break;
    }
    out[index++] = 'm';
    out[index] = '\0';
    return index;
}

#define BUFFER_SIZE 128
static char RENDER_BUFFER[BUFFER_SIZE];
static int RENDER_BUFFER_INDEX = 0;
static Color COLOR_FIELD[22][12];

// 出力をバッファリングし、必要時にフラッシュ
void buffered_print(const char* arg, int len, bool flush) {
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
    buffered_print("\x1b[2J", 4, false);
    buffered_print("\x1b[H", 3, false);

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
    int buffered_len = 0;
    Color prev_color = WHITE;
    // バッファに色をセット
    for (int y = 0; y < 22; y++) {
        prev_color = WHITE;
        buffered_len = color_to_code(color_buffer, WHITE);
        buffered_print(color_buffer, buffered_len, false);
        for (int x = 0; x < 12; x++) {
            if (COLOR_FIELD[y][x] != prev_color) {
                buffered_len = color_to_code(color_buffer, COLOR_FIELD[y][x]);
                buffered_print(color_buffer, buffered_len, false);
                prev_color = COLOR_FIELD[y][x];
            }
            buffered_print("  ", 2, false);
        }
        // 行末にリセットと改行を挿入
        buffered_print("\x1b[0m\n", 5, false);
    }

    // 最終フラッシュ
    buffered_print("", 0, true);
}
