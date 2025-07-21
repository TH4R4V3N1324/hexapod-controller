#ifndef HEX_PACKET_H
#define HEX_PACKET_H
#include <stdint.h>

// Define HexPacket struct
#pragma pack(push, 1)
struct HexPacket {
    int16_t legConfigs[3];
    int16_t currentHeight;
    int16_t currentPhase;
    int16_t currentGait;
    int16_t currentMode;
};
#pragma pack(pop)

extern HexPacket hexPacket;

// Compare if two HexPackets are different
bool hexPacketChanged(const HexPacket& a, const HexPacket& b) {
    return memcmp(&a, &b, sizeof(HexPacket)) != 0;
}

#endif