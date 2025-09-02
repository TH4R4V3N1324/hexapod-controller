#ifndef CONTROL_PACKET_H
#define CONTROL_PACKET_H
#include <stdint.h>

// Enum for availble commands
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
struct ControlPacket {
    int16_t joystick1X;
    int16_t joystick1Y;
    int16_t joystick2X;
    int16_t joystick2Y;
    int16_t currentHeight;
    Command command;
    int16_t commandArgs[3];
};
#pragma pack(pop)

extern ControlPacket controlPacket;

// Compare if two ControlPackets are different
bool controlPacketChanged(const ControlPacket& a, const ControlPacket& b) {
    return memcmp(&a, &b, sizeof(ControlPacket)) != 0;
}

#endif