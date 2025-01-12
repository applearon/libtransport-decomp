#ifndef TRANSPORT_PACKETS_H
#define TRANSPORT_PACKETS_H


/*
 * OUTBOUND PACKETS
 */

// defines each of the packets
#define BEGIN_PACKET 'C', 'R', 'T',
#define SPACER_PACKET '\0', '\0',
#define GEN_PACKET(...) {BEGIN_PACKET SPACER_PACKET __VA_ARGS__}

// This is used to start reading input
#define REFRESH_PACKET 'S', 'T', 'P',
#define BRIGHTNESS_PACKET 'L', 'I', 'G',
#define SCREEN_TOGGLE_OFF_PACKET 'H', 'A', 'N',
#define SCREEN_TOGGLE_ON_PACKET 'D', 'I', 'S',
#define BACKGROUND_IMG_PACKET 'L', 'O', 'G',
#define CLEAR_PACKET 'C', 'L', 'E',
#define SET_CELL_IMG 'B', 'A', 'T',

/*
 * INBOUND PACKETS
 */
enum key { // at the very least, this is specific to the 293 model
    KEY_1=1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_10,
    KEY_11,KEY_12,KEY_13,KEY_14,KEY_15,
    ALL_KEYS = 0xff
};
struct key_input {
    enum key key;
    bool down;
};
#endif
