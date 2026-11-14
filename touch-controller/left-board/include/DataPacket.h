#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <Arduino.h>

#define START_BYTE 0xAA
#define BTN_JOY   (1 << 0)
#define BTN_UP    (1 << 1)
#define BTN_DOWN  (1 << 2)
#define BTN_LEFT  (1 << 3)
#define BTN_RIGHT (1 << 4)

typedef struct __attribute__((packed)) {
    uint8_t start;     // START_BYTE (0xAA)
    uint8_t version;   // Protocol version

    int16_t joy_x;
    int16_t joy_y;

    uint8_t buttons;   // bitfield (see below)

    uint8_t checksum;
} InputPacket;

extern InputPacket inputPacket;

class DataPacket {
private:
    uint8_t calcChecksum(const uint8_t* data, size_t len);
public:
    void sendInputPacket();
};

extern DataPacket dataPacket;


#endif // DATAPACKET_H