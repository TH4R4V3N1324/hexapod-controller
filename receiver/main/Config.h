// Receiver settings: addresses, pins, timings and the gamepad button mapping.
#ifndef RECEIVER_CONFIG_H
#define RECEIVER_CONFIG_H
#include <stdint.h>

namespace config {

// ---- Outputs -------------------------------------------------------------------------------------

// The handheld controller (oled-controller or touch-controller). Packets from any other ESP-NOW
// sender are ignored, and their address is printed so you can copy the right one in.
// All zeros: no handheld controller, gamepads only.
constexpr uint8_t CONTROLLER_MAC[6] = {0xFC, 0x01, 0x2C, 0xD9, 0x41, 0xE4};

// ESP32 stack: the robot's ESP32. When set, the receiver relays every ControlPacket to it over
// ESP-NOW, and the robot must have this receiver's MAC (printed at boot) as its CONTROLLER_MAC.
// All zeros: no relay (Servo2040 stack, I2C only).
constexpr uint8_t ROBOT_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Servo2040 stack: the receiver is an I2C slave; the Servo 2040 reads ControlPackets and writes
// HexPackets. Always on; harmless when nothing is connected.
constexpr uint8_t I2C_ADDRESS = 0x08;
constexpr int I2C_SDA_PIN = 21;
constexpr int I2C_SCL_PIN = 22;

// ---- Timing ----------------------------------------------------------------------------------------

constexpr uint32_t LOOP_PERIOD_MS = 10;          // control loop, and ESP-NOW relay rate
// Failsafe: an input source counts as lost after this long without a packet or report. When the
// active one is lost, the sticks are centred and the command cleared, so the robot stops.
constexpr uint32_t CONTROLLER_TIMEOUT_MS = 250;
// Gamepads only report when something changes, so a held stick can go quiet for a while
constexpr uint32_t GAMEPAD_TIMEOUT_MS = 500;
// A gamepad command is held this long, then cleared, so pressing the same button again counts
constexpr uint32_t COMMAND_HOLD_MS = 150;

// ---- Pairing -----------------------------------------------------------------------------------------

constexpr int PAIR_BUTTON_PIN = 0;              // BOOT on ESP32 DevKits (active low)
constexpr int STATUS_LED_PIN = 2;               // blue LED on most DevKits; -1 for none
constexpr uint32_t PAIRING_WINDOW_MS = 60000;   // how long a pairing window stays open
constexpr uint32_t FORGET_HOLD_MS = 5000;       // hold the button this long to forget all gamepads

// ---- Gamepad mapping ---------------------------------------------------------------------------------
// The layout is hexapod-gait-core's hexapod_teleop::GamepadTeleop (shared with ROS 2); these tune it.
// Bluepad32 names buttons by position, Xbox-style, whatever the gamepad.

constexpr int STICK_DEADZONE = 12;              // out of ±128, on top of the robot's own
constexpr int HEIGHT_STEP_MM = 10;              // per D-pad step (ROS 2: height_step)
constexpr uint32_t HEIGHT_REPEAT_MS = 300;      // D-pad held: a step this often (ROS 2: height_repeat)
constexpr int HEIGHT_MIN_MM = 60;               // gait core's GaitParams min/max_height; the robot refuses others
constexpr int HEIGHT_MAX_MM = 160;
constexpr uint32_t POWER_DOWN_HOLD_MS = 2000;   // hold RB this long to power down (ROS 2: power_down_hold)

}  // namespace config

#endif
