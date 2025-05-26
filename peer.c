#if defined(NATIVE_MODE)

#include "peer.h"

DataPacket receive_data() { return; }

void send_lines(int lines) {}

void send_game_over() {}

#else

#include "io.h"
#include "peer.h"

static const unsigned char TYPE_SEND_LINES = 0x01;

DataPacket receive_data() {
    DataPacket packet = {0};

    UART *uart = _uart[1];
    if (UART_RX_EMPTY(uart)) {
        return packet;  /// no data
    }

    set_uart_ID(1);
    char type = io_getch();
    if (type == TYPE_SEND_LINES) {
        packet.type = ATTACK_LINES;
        packet.data.lines = io_getch();
    }
    set_uart_ID(0);

    return packet;
}

void send_lines(int lines) {
    set_uart_ID(1);

    io_putch(TYPE_SEND_LINES);
    // lines must be less than 255
    io_putch((char)lines);

    set_uart_ID(0);
}

#endif
