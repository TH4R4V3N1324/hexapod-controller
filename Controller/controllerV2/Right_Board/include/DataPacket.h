#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <Arduino.h>

#define START_BYTE 0xAA

typedef struct __attribute__((packed)) {
    uint8_t start;
    uint8_t version; 
    int16_t joy_x;
    int16_t joy_y;
    int8_t joy_btn;
    int16_t enc_count;
    uint8_t enc_btn;
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