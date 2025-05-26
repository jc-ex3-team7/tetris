#if !defined(PEER_H)
#define PEER_H

typedef enum {
    NONE,
    ATTACK_LINES,
    GAME_OVER,
    READY,
    SEED,
} DATA_TYPE;

typedef struct {
    DATA_TYPE type;
    union {
        int lines;
        int seed;
    } data;
} DataPacket;

DataPacket receive_data();

void send_lines(int lines);

void send_game_over();

void send_ready();

void send_seed(int seed);

#endif
