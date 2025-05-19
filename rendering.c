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

// typedef enum {
//     BLACK,
//     WHITE,
//     RED,
//     GREEN,
//     BLUE,
//     YELLOW,
//     CYAN,
//     MAGENTA,
//     ORANGE,
//     PINK,
//     BROWN,s
//     LIGHT_BLUE,
//     LIGHT_GREEN,
//     GRAY,
//     DARK_GRAY,
// } Color;

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

char* get_color_named(Color bg) {
    int bg_code = color_to_code(bg);
    return bg_code == 0 ? "\x1b[0m" : "\x1b[38;5;%dm";
}

// 文字列として出力をbufferに格納してから出力する

#define BUFFER_SIZE 128
static char RENDER_BUFFER[BUFFER_SIZE];
static Color COLOR_FIELD[22][12];
static int RENDER_BUFFER_INDEX = 0;

void buffered_print(const char* arg, bool flush) {
    // 文字列をバッファに格納する
    // bufferが溢れそうな時は、bufferを出力してクリアする
    int len = strlen(arg);
    if (RENDER_BUFFER_INDEX + len >= BUFFER_SIZE) {
        // バッファを出力する
        if (RENDER_BUFFER_INDEX < BUFFER_SIZE) {
            RENDER_BUFFER[RENDER_BUFFER_INDEX] = '\0';
        }
        printf("%s", RENDER_BUFFER);
        RENDER_BUFFER_INDEX = 0;
    }
    // バッファに格納する
    snprintf(RENDER_BUFFER + RENDER_BUFFER_INDEX, BUFFER_SIZE - RENDER_BUFFER_INDEX, "%s", arg);
    RENDER_BUFFER_INDEX += len;
    // flushがtrueの時は、バッファを出力してクリアする
    if (flush) {
        printf("%s", RENDER_BUFFER);
        RENDER_BUFFER_INDEX = 0;
    }
}

void render(State state) {
    // Render with buffered_print
    //  Clear the buffer
    memset(RENDER_BUFFER, 0, BUFFER_SIZE);
    RENDER_BUFFER_INDEX = 0;
    // clear the screen
    buffered_print("\x1b[2J", false);
    buffered_print("\x1b[H", false);
    // Set the frame color
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 12; x++) {
            if (y == 0 || y == 21 || x == 0 || x == 11) {
                COLOR_FIELD[y][x] = WHITE;
            } else {
                COLOR_FIELD[y][x] = BLACK;
            }
        }
    }
    // merge the mino with the field
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (state.field[y][x]) {
                COLOR_FIELD[y + 1][x + 1] = WHITE;
            }
        }
    }
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
    // Print the field with frame
    Color prev_color = WHITE;
    // buffered_print White
    buffered_print(get_color_named(WHITE), false);
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 12; x++) {
            // set color
            if (COLOR_FIELD[y][x] != prev_color) {
                buffered_print(get_color_named(COLOR_FIELD[y][x]), false);
                prev_color = COLOR_FIELD[y][x];
            }
            buffered_print("  ", false);
        }
    }
    buffered_print("", true);
}

// void render(State state) {
//     // Clear the buffer
//     memset(RENDER_BUFFER, 0, BUFFER_SIZE);
//     int index = 0;

//     // clear the screen
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[2J");
//     index += strlen(RENDER_BUFFER + index);
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[H");
//     index += strlen(RENDER_BUFFER + index);

//     // Set the frame color
//     for (int y = 0; y < 22; y++) {
//         for (int x = 0; x < 12; x++) {
//             if (y == 0 || y == 21 || x == 0 || x == 11) {
//                 COLOR_FIELD[y][x] = WHITE;
//             } else {
//                 COLOR_FIELD[y][x] = BLACK;
//             }
//         }
//     }

//     // merge the mino with the field
//     for (int y = 0; y < 20; y++) {
//         for (int x = 0; x < 10; x++) {
//             if (state.field[y][x]) {
//                 COLOR_FIELD[y + 1][x + 1] = WHITE;
//             }
//         }
//     }

//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < 4; j++) {
//             if (state.mino.block[i][j]) {
//                 int y = state.mino.position.y + i;
//                 int x = state.mino.position.x + j;
//                 switch (state.mino.type) {
//                     case T:
//                         COLOR_FIELD[y + 1][x + 1] = MAGENTA;
//                         break;
//                     case S:
//                         COLOR_FIELD[y + 1][x + 1] = GREEN;
//                         break;
//                     case Z:
//                         COLOR_FIELD[y + 1][x + 1] = RED;
//                         break;
//                     case L:
//                         COLOR_FIELD[y + 1][x + 1] = ORANGE;
//                         break;
//                     case J:
//                         COLOR_FIELD[y + 1][x + 1] = BLUE;
//                         break;
//                     case O:
//                         COLOR_FIELD[y + 1][x + 1] = YELLOW;
//                         break;
//                     case I:
//                         COLOR_FIELD[y + 1][x + 1] = CYAN;
//                         break;
//                 }
//             }
//         }
//     }

//     // Print the field with frame
//     for (int y = 0; y < 22; y++) {
//         for (int x = 0; x < 12; x++) {
//             // move cursor
//             snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[%d;%dH", y + 1, x * 2 + 1);
//             index += strlen(RENDER_BUFFER + index);

//             // set color
//             snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, get_color_named(COLOR_FIELD[y][x]),
//                      color_to_code(COLOR_FIELD[y][x]));
//             index += strlen(RENDER_BUFFER + index);

//             snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "  ");
//             index += strlen(RENDER_BUFFER + index);
//         }
//     }

//     // reset color
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[0m");
//     index += strlen(RENDER_BUFFER + index);
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[23;1H");
//     index += strlen(RENDER_BUFFER + index);
//     // Add end of line
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\n");
//     index += strlen(RENDER_BUFFER + index);
//     // Add flush command
//     snprintf(RENDER_BUFFER + index, BUFFER_SIZE - index, "\x1b[0m");
//     index += strlen(RENDER_BUFFER + index);
//     // Add end of string
//     RENDER_BUFFER[index] = '\0';
//     index++;
//     // Print the buffer
//     printf("%s", RENDER_BUFFER);
// }

// void set_color_named(Color bg) {
//     int bg_code = color_to_code(bg);
//     printf("\x1b[48;5;%dm", bg_code);
// }

// static Color prev_field[22][12];
// static bool initialized = false;

// void render(State state) {
//     Color field_with_frame[22][12];

//     for (int y = 0; y < 22; y++) {
//         for (int x = 0; x < 12; x++) {
//             field_with_frame[y][x] = BLACK;
//         }
//     }

//     // Set the frame color
//     for (int y = 0; y < 22; y++) {
//         field_with_frame[y][0] = WHITE;
//         field_with_frame[y][11] = WHITE;
//     }
//     for (int x = 0; x < 12; x++) {
//         field_with_frame[0][x] = WHITE;
//         field_with_frame[21][x] = WHITE;
//     }

//     // merge the mino with the field
//     for (int y = 0; y < 20; y++) {
//         for (int x = 0; x < 10; x++) {
//             if (state.field[y][x]) {
//                 field_with_frame[y + 1][x + 1] = WHITE;
//             }
//         }
//     }
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < 4; j++) {
//             if (state.mino.block[i][j]) {
//                 int y = state.mino.position.y + i;
//                 int x = state.mino.position.x + j;
//                 switch (state.mino.type) {
//                     case T:
//                         field_with_frame[y + 1][x + 1] = MAGENTA;
//                         break;
//                     case S:
//                         field_with_frame[y + 1][x + 1] = GREEN;
//                         break;
//                     case Z:
//                         field_with_frame[y + 1][x + 1] = RED;
//                         break;
//                     case L:
//                         field_with_frame[y + 1][x + 1] = ORANGE;
//                         break;
//                     case J:
//                         field_with_frame[y + 1][x + 1] = BLUE;
//                         break;
//                     case O:
//                         field_with_frame[y + 1][x + 1] = YELLOW;
//                         break;
//                     case I:
//                         field_with_frame[y + 1][x + 1] = CYAN;
//                         break;
//                 }
//             }
//         }
//     }

//     // Print the field with frame
//     for (int y = 0; y < 22; y++) {
//         for (int x = 0; x < 12; x++) {
//             // differential update
//             if (!initialized || field_with_frame[y][x] != prev_field[y][x]) {
//                 // move cursor
//                 printf("\x1b[%d;%dH", y + 1, x * 2 + 1);

//                 set_color_named(field_with_frame[y][x]);
//                 printf("  ");

//                 prev_field[y][x] = field_with_frame[y][x];
//             }
//         }
//     }
//     initialized = true;

//     // reset color
//     printf("\x1b[0m");
//     printf("\x1b[23;1H");
//     fflush(stdout);
// }
