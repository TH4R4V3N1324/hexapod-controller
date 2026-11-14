// Bluetooth gamepads through Bluepad32: pairing, and turning a gamepad into ControlPackets
#ifndef RECEIVER_GAMEPAD_H
#define RECEIVER_GAMEPAD_H
#include <stdint.h>
#include "HexapodPackets.h"

struct uni_platform;

namespace gamepad {

// Bluepad32 platform; pass to uni_platform_set_custom() before uni_init()
struct uni_platform* platform();

/**
 * @brief Build this tick's ControlPacket from the gamepad. Call every loop.
 * @param status The robot's latest status, for the next gait and mode and the current height
 * @return When the gamepad last reported (millis()), or 0 if none is connected
 */
uint32_t update(uint32_t nowMs, const HexPacket& status, ControlPacket& out);

// Pairing: while the window is open any gamepad may connect, and the first to do so is remembered.
// Outside it, only remembered gamepads can connect. Safe to call from any task.
void startPairing();
void forgetAll();       // forget every remembered gamepad, then open a pairing window
bool pairing();
bool connected();
void checkPairingTimeout(uint32_t nowMs);

}  // namespace gamepad

#endif
