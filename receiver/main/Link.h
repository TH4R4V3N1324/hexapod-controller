// The receiver's shared state: the latest input from each source, which source is driving, and
// the ControlPacket and HexPacket that the outputs (I2C, ESP-NOW) read and write.
#ifndef RECEIVER_LINK_H
#define RECEIVER_LINK_H
#include <stdint.h>
#include <gait_core/protocol/packets.h>

enum Source : uint8_t {
    SOURCE_NONE,        // nothing driving: sticks centred, no command (the failsafe)
    SOURCE_CONTROLLER,  // the handheld controller over ESP-NOW
    SOURCE_GAMEPAD,     // a Bluetooth gamepad through Bluepad32
};

namespace link {

// From the ESP-NOW callback: a packet from the handheld controller
void controllerInput(const ControlPacket& packet);
// From the loop: the gamepad's packet, and when it last reported (0 = not connected)
void gamepadInput(const ControlPacket& packet, uint32_t lastReportMs);

// From the I2C or ESP-NOW callbacks: the robot's status
void setStatus(const HexPacket& status);

// Thread-safe snapshots for the outputs
ControlPacket control();
HexPacket status();

/**
 * @brief Pick the source and apply the failsafe. Call every loop.
 * @note The source driving keeps control while it's alive; the other takes over only when it's
 *       lost. With neither alive, the sticks are centred and the command cleared; height is kept
 *       so the robot doesn't drop.
 * @return The source now driving
 */
Source update(uint32_t nowMs);

const char* sourceName(Source source);

}  // namespace link

#endif
