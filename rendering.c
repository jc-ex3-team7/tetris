#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "updater.h"

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    unsigned char uc = (unsigned char)c;
    while (n--) {
        *p++ = uc;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

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

int color_to_code(char *out, Color color) {
    out[0] = '\x1b';
    out[1] = '[';
    out[2] = '4';
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

#define BUFFER_SIZE 512
static char RENDER_BUFFER[BUFFER_SIZE];
static int RENDER_BUFFER_INDEX = 0;
static Color COLOR_FIELD[22][13];
static Color PREV_COLOR_FIELD[22][13];
static bool first_render = true;

// 簡易的整数→文字列変換
static int int_to_str(int num, char *buf) {
    int len = 0;
    int temp = num;
    char rev[10];
    if (num == 0) {
        buf[len++] = '0';
        buf[len] = '\0';
        return 1;
    }
    while (temp > 0) {
        rev[len++] = '0' + (temp % 10);
        temp /= 10;
    }
    for (int i = 0; i < len; ++i) {
        buf[i] = rev[len - 1 - i];
    }
    buf[len] = '\0';
    return len;
}

// カーソル移動コード生成
static int move_cursor_code(char *out, int row, int col) {
    int idx = 0;
    out[idx++] = '\x1b';
    out[idx++] = '[';
    idx += int_to_str(row, out + idx);
    out[idx++] = ';';
    idx += int_to_str(col, out + idx);
    out[idx++] = 'H';
    out[idx] = '\0';
    return idx;
}

// 出力バッファリング
void buffered_print(const char *arg, int len, bool flush) {
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

// カーソルの非表示／表示
static void hide_cursor(void) { buffered_print("\x1b[?25l", 6, false); }
static void show_cursor(void) { buffered_print("\x1b[?25h", 6, false); }

// 描画関数
void render(State state) {
    // カーソル非表示
    hide_cursor();

    // フィールドの準備
    for (int y = 0; y < 22; ++y) {
        for (int x = 0; x < 13; ++x) {
            if (y == 0 || y == 21 || x == 0 || x == 11) {
                COLOR_FIELD[y][x] = WHITE;
            } else {
                COLOR_FIELD[y][x] = BLACK;
            }
        }
    }
    for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 10; ++x) {
            if (state.field[y][x]) {
                COLOR_FIELD[y + 1][x + 1] = GRAY;
            }
        }
    }
    for (int i = 0; i < 22; i++) {
        COLOR_FIELD[21 - i][12] = i < state.attack_lines ? RED : BLACK;
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (state.mino.block[i][j]) {
                int yy = state.mino.position.y + i + 1;
                int xx = state.mino.position.x + j + 1;
                switch (state.mino.type) {
                    case T:
                        COLOR_FIELD[yy][xx] = MAGENTA;
                        break;
                    case S:
                        COLOR_FIELD[yy][xx] = GREEN;
                        break;
                    case Z:
                        COLOR_FIELD[yy][xx] = RED;
                        break;
                    case L:
                        COLOR_FIELD[yy][xx] = ORANGE;
                        break;
                    case J:
                        COLOR_FIELD[yy][xx] = BLUE;
                        break;
                    case O:
                        COLOR_FIELD[yy][xx] = YELLOW;
                        break;
                    case I:
                        COLOR_FIELD[yy][xx] = CYAN;
                        break;
                }
            }
        }
    }

    int y, x, len;
    char color_buf[16], cs_buf[16];

    if (first_render) {
        // 全描画
        memset(RENDER_BUFFER, 0, BUFFER_SIZE);
        RENDER_BUFFER_INDEX = 0;
        buffered_print("\x1b[2J", 4, false);
        buffered_print("\x1b[H", 3, false);
        for (y = 0; y < 22; ++y) {
            Color prev = WHITE;
            len = color_to_code(color_buf, WHITE);
            buffered_print(color_buf, len, false);
            for (x = 0; x < 13; ++x) {
                if (COLOR_FIELD[y][x] != prev) {
                    len = color_to_code(color_buf, COLOR_FIELD[y][x]);
                    buffered_print(color_buf, len, false);
                    prev = COLOR_FIELD[y][x];
                }
                buffered_print("  ", 2, false);
            }
            buffered_print("\x1b[0m\n", 5, false);
        }
    } else {
        // 差分更新
        for (y = 0; y < 22; ++y) {
            for (int x = 0; x < 13; ++x) {
                if (COLOR_FIELD[y][x] != PREV_COLOR_FIELD[y][x]) {
                    len = move_cursor_code(cs_buf, y + 1, x * 2 + 1);
                    buffered_print(cs_buf, len, false);
                    len = color_to_code(color_buf, COLOR_FIELD[y][x]);
                    buffered_print(color_buf, len, false);
                    buffered_print("  ", 2, false);
                }
            }
        }
    }

    // リセット＋カーソルを画面下に
    buffered_print("\x1b[0m", 4, false);
    len = move_cursor_code(cs_buf, 23, 1);
    buffered_print(cs_buf, len, false);

    // カーソル再表示＆フラッシュ
    show_cursor();
    buffered_print("", 0, true);

    // 前回バッファを更新
    for (y = 0; y < 22; ++y) {
        for (x = 0; x < 13; ++x) {
            PREV_COLOR_FIELD[y][x] = COLOR_FIELD[y][x];
        }
    }
    first_render = false;
}
