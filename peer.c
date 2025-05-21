#include "peer.h"

#include "io.h"

static const unsigned char TYPE_SEND_LINES = 0x01;

DataPacket receive_data() {
    DataPacket packet = {0};

    UARTBuf* ub_rx = &UART_BUF_RX[1];
    if (!ub_rx->not_empty) {
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

void sendLiens(int lines) {
    set_uart_ID(1);

    io_putch(TYPE_SEND_LINES);
    // lines must be less than 255
    io_putch((char)lines);

    set_uart_ID(0);
}
