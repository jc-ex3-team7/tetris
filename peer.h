#if !defined(PEER_H)
#define PEER_H

typedef enum {
    NONE,
    ATTACK_LINES,
    GAME_OVER,
} DATA_TYPE;

typedef struct {
    DATA_TYPE type;
    union {
        int lines;
    } data;
} DataPacket;

DataPacket receive_data();

void send_lines(int lines);

#endif
