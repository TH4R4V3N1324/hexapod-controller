#include "Link.h"

#include <freertos/FreeRTOS.h>
#include <stdio.h>

#include "Clock.h"
#include <string.h>

#include "Config.h"

namespace link {
namespace {

// Guards everything below: written from the Wi-Fi task, the I2C task, Bluepad32's task and the loop
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

struct Input {
    ControlPacket packet;
    uint32_t lastMs;   // last packet or report, from nowMs()
    bool heard;        // anything since boot (lastMs is meaningless until then)
};

Input controller = {};
Input gamepad = {};
// What the robot reads; stamped with the version from the start, for the I2C slave's first load
ControlPacket output = [] {
    ControlPacket p = {};
    p.version = HEXAPOD_PROTOCOL_VERSION;
    return p;
}();
HexPacket robotStatus = {};
Source active = SOURCE_NONE;

bool alive(const Input& in, uint32_t timeoutMs, uint32_t nowMs) {
    return in.heard && (nowMs - in.lastMs) <= timeoutMs;
}

}  // namespace

void controllerInput(const ControlPacket& packet) {
    const uint32_t now = nowMs();
    portENTER_CRITICAL(&mux);
    controller.packet = packet;
    controller.lastMs = now;
    controller.heard = true;
    portEXIT_CRITICAL(&mux);
}

void gamepadInput(const ControlPacket& packet, uint32_t lastReportMs) {
    portENTER_CRITICAL(&mux);
    gamepad.packet = packet;
    gamepad.lastMs = lastReportMs;
    gamepad.heard = lastReportMs != 0;
    portEXIT_CRITICAL(&mux);
}

void setStatus(const HexPacket& status) {
    portENTER_CRITICAL(&mux);
    robotStatus = status;
    portEXIT_CRITICAL(&mux);
}

ControlPacket control() {
    portENTER_CRITICAL(&mux);
    ControlPacket p = output;
    portEXIT_CRITICAL(&mux);
    return p;
}

HexPacket status() {
    portENTER_CRITICAL(&mux);
    HexPacket s = robotStatus;
    portEXIT_CRITICAL(&mux);
    return s;
}

Source update(uint32_t nowMs) {
    portENTER_CRITICAL(&mux);
    const bool controllerAlive = alive(controller, config::CONTROLLER_TIMEOUT_MS, nowMs);
    const bool gamepadAlive = alive(gamepad, config::GAMEPAD_TIMEOUT_MS, nowMs);
    const Source before = active;

    // Whoever is driving keeps control while alive; otherwise the other one, if alive
    if (!(active == SOURCE_CONTROLLER && controllerAlive) && !(active == SOURCE_GAMEPAD && gamepadAlive)) {
        active = controllerAlive ? SOURCE_CONTROLLER : gamepadAlive ? SOURCE_GAMEPAD : SOURCE_NONE;
    }

    if (active == SOURCE_CONTROLLER) {
        output = controller.packet;
    } else if (active == SOURCE_GAMEPAD) {
        output = gamepad.packet;
    } else {
        output.version = HEXAPOD_PROTOCOL_VERSION;
        output.joystick1X = output.joystick1Y = output.joystick2X = output.joystick2Y = 0;
        output.tiltYaw = output.tiltZ = 0;
        output.command = CMD_NONE;
        memset(output.commandArgs, 0, sizeof(output.commandArgs));
        // currentHeight kept, so the robot doesn't drop
    }
    const Source now = active;
    portEXIT_CRITICAL(&mux);

    if (now != before) {
        if (now == SOURCE_NONE) {
            printf("Failsafe: lost the %s, sticks centred\n", sourceName(before));
        } else {
            printf("Driving from the %s\n", sourceName(now));
        }
    }
    return now;
}

const char* sourceName(Source source) {
    switch (source) {
        case SOURCE_CONTROLLER: return "handheld controller";
        case SOURCE_GAMEPAD: return "gamepad";
        default: return "nothing";
    }
}

}  // namespace link
