#ifndef DATAPACKET_H
#define DATAPACKET_H
#include <stdint.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <hexapod_protocol/packets.h>   // hexapod-gait-core: ControlPacket, HexPacket, Command, Gait, Mode

extern ControlPacket controlPacket;
extern HexPacket hexPacket;

class DataPacket {
private:   
    uint8_t receiverMAC[6] = {0x30, 0xC9, 0x22, 0x28, 0x73, 0x4C};
    int previousTime = 0;

public:
    static void onHexDataReceived(const uint8_t *mac, const uint8_t *data, int len);
    void sendData();
    void initializeESPNow();
};
extern DataPacket dataPacket;

#endif