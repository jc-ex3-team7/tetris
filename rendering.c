#include <stdbool.h>
#include <stdio.h>

#include "updater.h"

typedef enum {
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    ORANGE,
    PINK,
    BROWN,
    LIGHT_BLUE,
    LIGHT_GREEN,
    GRAY,
    DARK_GRAY,
    DEFAULT
} Color;

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
        case DEFAULT:
            return 9;  // default terminal color (not 256 color)
        default:
            return 9;
    }
}

void set_color_named(Color bg) {
    int bg_code = color256_to_code(bg);
    printf("\x1b[48;5;%dm", bg_code);
}

void clear_screen() { printf("\033[H\033[2J"); }

void render(State state) {
    clear_screen();

    Color field_with_frame[22][12];

    // Initialize the field with the default color
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 12; x++) {
            field_with_frame[y][x] = DEFAULT;
        }
    }

    // Set the frame color
    for (int y = 0; y < 22; y++) {
        field_with_frame[y][0] = BLACK;
        field_with_frame[y][11] = BLACK;
    }
    for (int x = 0; x < 12; x++) {
        field_with_frame[0][x] = BLACK;
        field_with_frame[21][x] = BLACK;
    }

    // merge the mino with the field
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            if (state.field[y][x]) {
                field_with_frame[y + 1][x + 1] = WHITE;
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
                        field_with_frame[y + 1][x + 1] = MAGENTA;
                        break;
                    case S:
                        field_with_frame[y + 1][x + 1] = GREEN;
                        break;
                    case Z:
                        field_with_frame[y + 1][x + 1] = RED;
                        break;
                    case L:
                        field_with_frame[y + 1][x + 1] = ORANGE;
                        break;
                    case J:
                        field_with_frame[y + 1][x + 1] = BLUE;
                        break;
                    case O:
                        field_with_frame[y + 1][x + 1] = YELLOW;
                        break;
                    case I:
                        field_with_frame[y + 1][x + 1] = CYAN;
                        break;
                }
            }
        }
    }

    // Print the field with frame
    for (int y = 0; y < 22; y++) {
        for (int x = 0; x < 12; x++) {
            set_color_named(field_with_frame[y][x]);
            printf("  ");
        }
        printf("\n");
    }
}
