#include "DataPacket.h"

InputPacket inputPacket;
DataPacket dataPacket;

/*
@brief Sends the input data packet over Serial
*/
void DataPacket::sendInputPacket() {
    InputPacket pkt;

    pkt.start   = START_BYTE;   // 0xAA
    pkt.version = 1;
    pkt.joy_x   = inputPacket.joy_x;
    pkt.joy_y   = inputPacket.joy_y;
    pkt.buttons = inputPacket.buttons;

    pkt.checksum = calcChecksum((uint8_t *)&pkt, sizeof(InputPacket));

    Serial.write((uint8_t *)&pkt, sizeof(pkt));
}

/*
@brief Calculates checksum by XORing all bytes in the data
@param data Pointer to the data
@param len Length of the data in bytes
@return Calculated checksum byte
*/
uint8_t DataPacket::calcChecksum(const uint8_t* data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint8_t cs = 0;

    for (size_t i = 0; i < len - 1; i++) {
        cs ^= p[i];
    }
    return cs;
}