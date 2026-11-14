#include "Gamepad.h"

#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <btstack_run_loop.h>
#include <uni.h>
}

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

// Bluepad32 axis (-512..511, Y down) to a packet axis (-128..127)
int16_t axis(int32_t value) {
    int32_t v = value * 128 / 512;
    if (v > -config::STICK_DEADZONE && v < config::STICK_DEADZONE) v = 0;
    return (int16_t)(v < -128 ? -128 : v > 127 ? 127 : v);
}

uni_gamepad_t previous = {};
Command heldCommand = CMD_NONE;
int16_t heldArgs[3] = {};
uint32_t heldUntilMs = 0;
int16_t heightTarget = 0;         // 0 = not set; the robot keeps its own height
uint32_t nextHeightStepMs = 0;    // D-pad held: when the next step happens
uint32_t sitPressedMs = 0;        // RB held since (0 = not held)
bool powerDownSent = false;
// Whether the robot is seated. HexPacket doesn't say, so it follows the commands sent from here.
// Both robots start seated (the Servo 2040 then stands by itself after 5 s, so its first press
// asks to stand again, which does nothing; the second sits).
bool seated = true;

void issue(Command command, int16_t arg0, uint32_t nowMs) {
    heldCommand = command;
    heldArgs[0] = arg0;
    heldArgs[1] = heldArgs[2] = 0;
    heldUntilMs = nowMs + config::COMMAND_HOLD_MS;
}

bool pressed(uint32_t now, uint32_t before, uint32_t mask) {
    return (now & mask) && !(before & mask);
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
    if (reportMs == 0) {
        previous = {};
        heldCommand = CMD_NONE;
        sitPressedMs = 0;
        heightTarget = 0;
        return 0;
    }

    // The layout follows the ROS 2 stack's controller_teleop (hexapod-ros2 ab90916), so one gamepad
    // drives every robot the same way. Not available over the packet yet: A/B/X/Y (feature toggles
    // in ROS 2), LT/RT (yaw in tilt mode) and the D-pad in tilt mode (raise/lower the body).

    // Left stick: walk (tilt mode: shift the body). Right stick: X turns (tilt mode: roll),
    // Y pitches in tilt mode. The packet's Y is forward, Bluepad32's is down.
    out.joystick1X = axis(gp.axis_x);
    out.joystick1Y = axis(-gp.axis_y);
    out.joystick2X = axis(gp.axis_rx);
    out.joystick2Y = axis(-gp.axis_ry);

    // Start [Options]: next mode (normal, strafe, tilt; config mode is for the handheld controller)
    if (pressed(gp.misc_buttons, previous.misc_buttons, MISC_BUTTON_START)) {
        const int next = status.currentMode >= MODE_TILT ? MODE_NORMAL : status.currentMode + 1;
        issue(CMD_SET_MODE, (int16_t)next, nowMs);
    }
    // Back [Create]: next gait
    if (pressed(gp.misc_buttons, previous.misc_buttons, MISC_BUTTON_SELECT)) {
        issue(CMD_SET_GAIT, (int16_t)((status.currentGait + 1) % NUM_GAITS), nowMs);
    }

    // RB [R1]: on release, sit down if standing, otherwise stand up (which also powers the servos
    // back on). Held for POWER_DOWN_HOLD_MS: power down instead (sit, then servo power off).
    if (gp.buttons & BUTTON_SHOULDER_R) {
        if (sitPressedMs == 0) {
            sitPressedMs = nowMs;
            powerDownSent = false;
        }
        if (!powerDownSent && nowMs - sitPressedMs >= config::POWER_DOWN_HOLD_MS) {
            issue(CMD_POWER_DOWN, 0, nowMs);
            powerDownSent = true;
            seated = true;
        }
    } else if (sitPressedMs != 0) {
        if (!powerDownSent) {
            issue(CMD_SIT, seated ? 0 : 1, nowMs);
            seated = !seated;
        }
        sitPressedMs = 0;
    }

    // D-pad up/down, walking: body height a step at a time, repeating while held; nothing while
    // seated. A step continues from the last target while the body is still on its way there, so
    // quick presses add up; otherwise from where the body is. (In tilt mode ROS 2 uses the D-pad
    // to shift the body up and down, which the packet can't carry, so it does nothing here.)
    const int dir = (gp.dpad & DPAD_UP) ? 1 : (gp.dpad & DPAD_DOWN) ? -1 : 0;
    const bool newPress = pressed(gp.dpad, previous.dpad, DPAD_UP | DPAD_DOWN);
    if (dir != 0 && !seated && status.currentMode != MODE_TILT && status.currentHeight > 0 &&
        (newPress || (int32_t)(nowMs - nextHeightStepMs) >= 0)) {
        const int current = status.currentHeight;
        const int from = (heightTarget != 0 && abs(current - heightTarget) < config::HEIGHT_STEP_MM)
                             ? heightTarget : current;
        int target = from + dir * config::HEIGHT_STEP_MM;
        if (target < config::HEIGHT_MIN_MM) target = config::HEIGHT_MIN_MM;
        if (target > config::HEIGHT_MAX_MM) target = config::HEIGHT_MAX_MM;
        heightTarget = (int16_t)target;
        nextHeightStepMs = nowMs + config::HEIGHT_REPEAT_MS;
    }
    out.currentHeight = heightTarget;

    // Hold each command briefly, then clear it, so the robot sees a repeat press as new
    if (heldCommand != CMD_NONE && (int32_t)(nowMs - heldUntilMs) < 0) {
        out.command = heldCommand;
        memcpy(out.commandArgs, heldArgs, sizeof(heldArgs));
    } else {
        heldCommand = CMD_NONE;
    }

    previous = gp;

    // Some gamepads (Xbox over BLE) only report changes, so a resting one goes quiet. Centred
    // sticks and no buttons held are safe to repeat, so count it as alive while connected; a
    // deflected stick that goes quiet still trips the failsafe.
    const bool resting = out.joystick1X == 0 && out.joystick1Y == 0 && out.joystick2X == 0 &&
                         out.joystick2Y == 0 && gp.buttons == 0 && gp.misc_buttons == 0 && gp.dpad == 0;
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
