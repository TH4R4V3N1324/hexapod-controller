#ifndef DATAPACKET_H
#define DATAPACKET_H
#include <stdint.h>
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

enum Command : uint8_t {
    CMD_NONE = 0,
    CMD_SET_GAIT,
    CMD_SET_MODE,
    CMD_SET_CONFIG,
    CMD_HOME_STANCE,
    CMD_REQUEST_CONFIG
};

// Define the data structure with no padding
#pragma pack(push, 1)
// Define ControlPacket struct
struct ControlPacket {
    int16_t joystick1X;
    int16_t joystick1Y;
    int16_t joystick2X;
    int16_t joystick2Y;
    int16_t currentHeight;
    Command command;
    int16_t commandArgs[3];
};

// Define HexPacket struct
struct HexPacket {
    int16_t legConfigs[3];
    int16_t currentHeight;
    int16_t currentPhase;
    int16_t currentGait;
    int16_t currentMode;
};
#pragma pack(pop)

extern ControlPacket controlPacket;
extern HexPacket hexPacket;

class DataPacket {
private:   
    uint8_t receiverMAC[6] = {0x30, 0xC9, 0x22, 0x28, 0x73, 0x4C};
    int previousTime = 0;

public:
    DataPacket(){initializeESPNow();};
    static void onHexDataReceived(const uint8_t *mac, const uint8_t *data, int len);
    void sendData();
    void initializeESPNow();
};
extern DataPacket dataPacket;

#endif