// ESP-NOW: packets from the handheld controller in; on the ESP32 stack, a relay to the robot
#ifndef RECEIVER_ESPNOW_LINK_H
#define RECEIVER_ESPNOW_LINK_H

namespace espnow {

bool begin();
// Call every loop: sends the current ControlPacket to config::ROBOT_MAC, if set
void relay();
// Call from the loop: prints what the receive callback noticed (Serial is too slow for it)
void report();
bool relaying();

}  // namespace espnow

#endif
