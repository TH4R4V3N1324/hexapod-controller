/**
 * Packets between the handheld controller and the robot.
 *
 * This is the controller side's copy of the robot firmware's DataPacket.h:
 *   hexapod-esp32/include/DataPacket.h      (ESP-NOW, controller <-> robot)
 *   hexapod-servo2040/includes/DataPacket.h (I2C, receiver <-> Servo 2040)
 * It must stay byte-compatible with them. Append new values; never reorder or
 * change a field's type. The size checks at the bottom catch accidental changes.
 *
 * Works from both C and C++ (the V2 display board includes it from C).
 */
#ifndef HEXAPOD_PACKETS_H
#define HEXAPOD_PACKETS_H

#include <stdint.h>

#ifdef __cplusplus
#define HEXAPOD_ENUM(name) enum name : uint8_t
#else
// C has no fixed-width enums before C23: the type is a uint8_t, the values an enum
#define HEXAPOD_ENUM(name) typedef uint8_t name; enum
#endif

// Gait and Mode values equal hexapod-gait-core's; MODE_CONFIG is handled by the robot firmware
HEXAPOD_ENUM(Gait) {
    GAIT_TRIPOD,
    GAIT_RIPPLE,
    GAIT_WAVE,
    NUM_GAITS
};

HEXAPOD_ENUM(Mode) {
    MODE_NORMAL,
    MODE_STRAFE,
    MODE_TILT,
    MODE_CONFIG,         // every joint to its centre plus its calibration offset
    NUM_MODES
};

HEXAPOD_ENUM(Command) {
    CMD_NONE = 0,
    CMD_SET_GAIT,        // [0] gait
    CMD_SET_MODE,        // [0] mode
    CMD_SET_CONFIG,      // [0] leg 0-5, [1] joint 0-2, [2] offset in degrees (robot clamps to +-60)
    CMD_HOME_STANCE,     // stand up
    CMD_REQUEST_CONFIG,  // [0] leg; its offsets come back in HexPacket.legConfigs
    CMD_SIT,             // [0] 1 sit down, 0 stand up
    CMD_POWER_DOWN       // sit down, then turn servo power off; a stand command powers up again
};

#undef HEXAPOD_ENUM

#pragma pack(push, 1)
// Controller -> robot
typedef struct ControlPacket {
    int16_t joystick1X;      // -128..127, X right
    int16_t joystick1Y;      // -128..127, Y forward
    int16_t joystick2X;
    int16_t joystick2Y;
    int16_t currentHeight;   // body height in mm, 0 = unchanged
    Command command;         // applied once, when it or its arguments change
    int16_t commandArgs[3];
} ControlPacket;

// Robot -> controller, in reply to each ControlPacket
typedef struct HexPacket {
    int16_t legConfigs[3];   // offsets for the leg asked for with CMD_REQUEST_CONFIG
    int16_t currentHeight;   // mm
    int8_t currentPhase;     // always 0
    Gait currentGait;
    Mode currentMode;
} HexPacket;
#pragma pack(pop)

#ifdef __cplusplus
static_assert(sizeof(ControlPacket) == 17, "ControlPacket layout changed: update the robot firmware too");
static_assert(sizeof(HexPacket) == 11, "HexPacket layout changed: update the robot firmware too");
#else
_Static_assert(sizeof(ControlPacket) == 17, "ControlPacket layout changed: update the robot firmware too");
_Static_assert(sizeof(HexPacket) == 11, "HexPacket layout changed: update the robot firmware too");
#endif

#endif
