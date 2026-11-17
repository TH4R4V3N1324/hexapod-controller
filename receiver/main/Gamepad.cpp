#include "Gamepad.h"

#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <btstack_run_loop.h>
#include <uni.h>
}

#include <optional>

#include <gait_core/teleop/gamepad_teleop.hpp>

#include "Clock.h"
#include "Config.h"

namespace gamepad {
namespace {

// ---- State shared between Bluepad32's task (BTstack) and the loop ------------------------------

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
uni_hid_device_t* active = nullptr;   // the gamepad driving; only touched on the BTstack task
uni_gamepad_t latest = {};            // its latest report
uint32_t lastReportMs = 0;            // 0 = no gamepad
volatile bool isConnected = false;
volatile bool isPairing = false;
volatile uint32_t pairingUntilMs = 0;

// ---- Work posted to the BTstack task (Bluepad32 isn't thread-safe) ---------------------------

void runOnBtstack(btstack_context_callback_registration_t& reg, volatile bool& pending, void (*fn)(void*)) {
    // A registration can't be queued twice; drop the request if one is already waiting
    portENTER_CRITICAL(&mux);
    const bool busy = pending;
    pending = true;
    portEXIT_CRITICAL(&mux);
    if (busy) return;
    reg.callback = fn;
    reg.context = nullptr;
    btstack_run_loop_execute_on_main_thread(&reg);
}

// On the BTstack task: open or close the pairing window
void applyPairing(bool open) {
    uni_bt_allowlist_set_enabled(!open);
    if (open) {
        // Discover new gamepads; incoming connections from remembered ones are always allowed
        uni_bt_start_scanning_and_autoconnect_unsafe();
    } else if (active != nullptr) {
        uni_bt_stop_scanning_unsafe();
    }
}

btstack_context_callback_registration_t openReg, closeReg, forgetReg;
volatile bool openPending = false, closePending = false, forgetPending = false;

void openOnBtstack(void*) {
    openPending = false;
    applyPairing(true);
    printf("Pairing: open for %u s. Put a gamepad in pairing mode.\n",
                  (unsigned)(config::PAIRING_WINDOW_MS / 1000));
}

void closeOnBtstack(void*) {
    closePending = false;
    applyPairing(false);
}

void forgetOnBtstack(void*) {
    forgetPending = false;
    if (active != nullptr) {
        uni_bt_disconnect_device_safe(uni_hid_device_get_idx_for_instance(active));
    }
    uni_bt_allowlist_remove_all();
    uni_bt_del_keys_unsafe();
    printf("Pairing: forgot every gamepad\n");
    applyPairing(true);
}

void setPairing(bool open, uint32_t nowMs) {
    isPairing = open;
    pairingUntilMs = nowMs + config::PAIRING_WINDOW_MS;
}

// ---- Bluepad32 platform callbacks (BTstack task) ---------------------------------------------

void onInit(int, const char**) {}

void onInitComplete() {
    uni_bt_allow_incoming_connections(true);

    const bd_addr_t* addresses = nullptr;
    int total = 0;
    uni_bt_allowlist_get_all(&addresses, &total);
    printf("Gamepads: %d remembered\n", total);
    for (int i = 0; i < total; i++) {
        printf("  %s\n", bd_addr_to_str(addresses[i]));
    }

    // Nothing paired yet: open the window, so a new receiver works out of the box
    setPairing(total == 0, nowMs());
    applyPairing(total == 0);
    if (total == 0) {
        printf("Pairing: no gamepads remembered, so the pairing window is open\n");
    } else {
        // Scan so BLE gamepads (which advertise rather than call in) can reconnect
        uni_bt_start_scanning_and_autoconnect_unsafe();
    }
}

uni_error_t onDiscovered(bd_addr_t, const char*, uint16_t cod, uint8_t) {
    // Gamepads only: no keyboards or mice
    if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) == UNI_BT_COD_MINOR_KEYBOARD) {
        return UNI_ERROR_IGNORE_DEVICE;
    }
    return UNI_ERROR_SUCCESS;
}

void onConnected(uni_hid_device_t*) {}

void onDisconnected(uni_hid_device_t* d) {
    if (d != active) return;
    active = nullptr;
    portENTER_CRITICAL(&mux);
    lastReportMs = 0;
    memset(&latest, 0, sizeof(latest));
    portEXIT_CRITICAL(&mux);
    isConnected = false;
    printf("Gamepad disconnected\n");
    uni_bt_start_scanning_and_autoconnect_unsafe();  // let it, or a BLE one, come back
}

uni_error_t onReady(uni_hid_device_t* d) {
    if (active != nullptr) {
        printf("Gamepad: one is already connected, refusing a second\n");
        return UNI_ERROR_NO_SLOTS;
    }
    active = d;
    portENTER_CRITICAL(&mux);
    memset(&latest, 0, sizeof(latest));
    lastReportMs = nowMs();
    portEXIT_CRITICAL(&mux);
    isConnected = true;

    bd_addr_t addr;
    uni_bt_conn_get_address(&d->conn, addr);
    printf("Gamepad connected: %s (%s)\n", d->name[0] ? d->name : "unnamed", bd_addr_to_str(addr));

    if (isPairing) {
        uni_bt_allowlist_add_addr(addr);
        printf("Pairing: remembered it, window closed\n");
        isPairing = false;
    }
    applyPairing(false);  // allowlist on, and stop scanning while driving (frees the radio for ESP-NOW)

    // Say hello: player 1, a blue light bar, and a short rumble
    if (d->report_parser.set_player_leds != nullptr) d->report_parser.set_player_leds(d, 0x01);
    if (d->report_parser.set_lightbar_color != nullptr) d->report_parser.set_lightbar_color(d, 0x00, 0x40, 0xff);
    if (d->report_parser.play_dual_rumble != nullptr) d->report_parser.play_dual_rumble(d, 0, 150, 128, 40);
    return UNI_ERROR_SUCCESS;
}

void onControllerData(uni_hid_device_t* d, uni_controller_t* ctl) {
    if (d != active || ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD) return;
    const uint32_t now = nowMs();
    portENTER_CRITICAL(&mux);
    latest = ctl->gamepad;
    lastReportMs = now != 0 ? now : 1;
    portEXIT_CRITICAL(&mux);
}

const uni_property_t* getProperty(uni_property_idx_t) {
    return nullptr;
}

void onOobEvent(uni_platform_oob_event_t, void*) {}

// ---- Mapping (loop task) -------------------------------------------------------------------------
// The layout is gait-core's gait_core::teleop::GamepadTeleop, the same one ROS 2's controller_teleop
// uses. This file only turns Bluepad32's reports into its input, and its output into packets.

gait_core::teleop::GamepadTeleop teleop = [] {
    gait_core::teleop::TeleopParams p;
    p.powerDownHold = config::POWER_DOWN_HOLD_MS / 1000.0;
    p.heightRepeat = config::HEIGHT_REPEAT_MS / 1000.0;
    return gait_core::teleop::GamepadTeleop(p);
}();

Command heldCommand = CMD_NONE;
int16_t heldArgs[3] = {};
uint32_t heldUntilMs = 0;
std::optional<float> heightTarget;    // mm, the last height asked for; unset: the robot keeps its own

// Bluepad32 axis (-512..511) to -1..1, with the dead zone removed
float stick(int32_t value) {
    float v = static_cast<float>(value) / 512.0f;
    if (v * 128.0f > -config::STICK_DEADZONE && v * 128.0f < config::STICK_DEADZONE) v = 0;
    return v < -1.0f ? -1.0f : v > 1.0f ? 1.0f : v;
}

// -1..1 to a packet axis
int16_t toAxis(float v) {
    const float scaled = v * HEXAPOD_AXIS_FULL_SCALE;
    return static_cast<int16_t>(scaled < -128.0f ? -128 : scaled > 127.0f ? 127 : scaled);
}

gait_core::teleop::GamepadState toGamepadState(const uni_gamepad_t& gp) {
    gait_core::teleop::GamepadState pad;
    pad.leftX = stick(gp.axis_x);
    pad.leftY = -stick(gp.axis_y);    // Bluepad32's Y is down
    pad.rightX = stick(gp.axis_rx);
    pad.rightY = -stick(gp.axis_ry);
    pad.leftTrigger = static_cast<float>(gp.brake) / 1023.0f;
    pad.rightTrigger = static_cast<float>(gp.throttle) / 1023.0f;
    pad.dpadUp = gp.dpad & DPAD_UP;
    pad.dpadDown = gp.dpad & DPAD_DOWN;
    pad.a = gp.buttons & BUTTON_A;
    pad.b = gp.buttons & BUTTON_B;
    pad.x = gp.buttons & BUTTON_X;
    pad.y = gp.buttons & BUTTON_Y;
    pad.leftBumper = gp.buttons & BUTTON_SHOULDER_L;
    pad.rightBumper = gp.buttons & BUTTON_SHOULDER_R;
    pad.start = gp.misc_buttons & MISC_BUTTON_START;
    pad.back = gp.misc_buttons & MISC_BUTTON_SELECT;
    return pad;
}

void issue(Command command, int16_t arg0, int16_t arg1, uint32_t nowMs) {
    heldCommand = command;
    heldArgs[0] = arg0;
    heldArgs[1] = arg1;
    heldArgs[2] = 0;
    heldUntilMs = nowMs + config::COMMAND_HOLD_MS;
}

}  // namespace

struct uni_platform* platform() {
    static struct uni_platform plat = {};
    plat.name = "hexapod";
    plat.init = onInit;
    plat.on_init_complete = onInitComplete;
    plat.on_device_discovered = onDiscovered;
    plat.on_device_connected = onConnected;
    plat.on_device_disconnected = onDisconnected;
    plat.on_device_ready = onReady;
    plat.on_controller_data = onControllerData;
    plat.get_property = getProperty;
    plat.on_oob_event = onOobEvent;
    return &plat;
}

uint32_t update(uint32_t nowMs, const HexPacket& status, ControlPacket& out) {
    portENTER_CRITICAL(&mux);
    const uni_gamepad_t gp = latest;
    const uint32_t reportMs = lastReportMs;
    portEXIT_CRITICAL(&mux);

    memset(&out, 0, sizeof(out));
    out.version = HEXAPOD_PROTOCOL_VERSION;
    if (reportMs == 0) {
        teleop.reset();
        heldCommand = CMD_NONE;
        heightTarget.reset();
        return 0;
    }

    // Until the robot has answered, assume it's seated, as both robots start
    const bool heard = status.version == HEXAPOD_PROTOCOL_VERSION;
    gait_core::teleop::RobotState robot;
    robot.seated = !heard || (status.flags & HEX_FLAG_SEATED);
    robot.tiltMode = heard && status.currentMode == MODE_TILT;

    const gait_core::teleop::GamepadState pad = toGamepadState(gp);
    const gait_core::teleop::TeleopOutput o = teleop.update(pad, robot, nowMs / 1000.0);

    // The packet carries raw stick directions (X right, Y forward); the robot reads them as walking
    // or tilting by its mode, the same way GamepadTeleop's two sets of outputs do
    out.joystick1X = toAxis(-o.vy);
    out.joystick1Y = toAxis(o.vx);
    out.joystick2X = toAxis(-o.wz);    // = o.roll
    out.joystick2Y = toAxis(o.pitch);
    out.tiltYaw = toAxis(o.yaw);
    out.tiltZ = toAxis(o.shiftZ);

    // Events become commands. One per update: they come from separate presses
    if (o.nextMode) {
        // normal, strafe, tilt; config mode is for the handheld controller
        const int next = status.currentMode >= MODE_TILT ? MODE_NORMAL : status.currentMode + 1;
        issue(CMD_SET_MODE, (int16_t)next, 0, nowMs);
    }
    if (o.nextGait) {
        // Each robot numbers its own gaits and reports how many it has; until it has answered,
        // assume explorer's
        const int numGaits = heard && status.numGaits > 0 ? status.numGaits : NUM_GAITS;
        issue(CMD_SET_GAIT, (int16_t)((status.currentGait + 1) % numGaits), 0, nowMs);
    }
    for (int f = 0; f < NUM_FEATURES; ++f) {
        if (o.toggleFeature[f]) {
            const bool on = heard && (status.flags & HEX_FLAG_FEATURE(f));
            issue(CMD_SET_FEATURE, (int16_t)f, on ? 0 : 1, nowMs);
        }
    }
    if (o.sit) {
        issue(CMD_SIT, *o.sit ? 1 : 0, 0, nowMs);
        heightTarget.reset();
    }
    if (o.powerDown) issue(CMD_POWER_DOWN, 0, 0, nowMs);
    if (o.heightStep != 0 && status.currentHeight > 0) {
        heightTarget = gait_core::teleop::stepHeight(status.currentHeight, heightTarget, o.heightStep,
                                                  config::HEIGHT_STEP_MM, config::HEIGHT_MIN_MM,
                                                  config::HEIGHT_MAX_MM);
    }
    out.currentHeight = heightTarget ? static_cast<int16_t>(*heightTarget + 0.5f) : 0;

    // Hold each command briefly, then clear it, so the robot sees a repeat press as new
    if (heldCommand != CMD_NONE && (int32_t)(nowMs - heldUntilMs) < 0) {
        out.command = heldCommand;
        memcpy(out.commandArgs, heldArgs, sizeof(heldArgs));
    } else {
        heldCommand = CMD_NONE;
    }

    // Some gamepads (Xbox over BLE) only report changes, so a resting one goes quiet. Nothing
    // pushed or held is safe to repeat, so count it as alive while connected; a deflected stick or
    // a held button that goes quiet still trips the failsafe.
    const bool resting = out.joystick1X == 0 && out.joystick1Y == 0 && out.joystick2X == 0 &&
                         out.joystick2Y == 0 && out.tiltYaw == 0 && gp.buttons == 0 &&
                         gp.misc_buttons == 0 && gp.dpad == 0;
    return resting ? nowMs : reportMs;
}

void startPairing() {
    setPairing(true, nowMs());
    runOnBtstack(openReg, openPending, openOnBtstack);
}

void forgetAll() {
    setPairing(true, nowMs());
    runOnBtstack(forgetReg, forgetPending, forgetOnBtstack);
}

void checkPairingTimeout(uint32_t nowMs) {
    if (isPairing && (int32_t)(nowMs - pairingUntilMs) >= 0) {
        isPairing = false;
        printf("Pairing: window closed\n");
        runOnBtstack(closeReg, closePending, closeOnBtstack);
    }
}

bool pairing() {
    return isPairing;
}

bool connected() {
    return isConnected;
}

}  // namespace gamepad
