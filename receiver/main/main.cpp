// Receiver: the handheld controller (ESP-NOW) and Bluetooth gamepads (Bluepad32) in; the Servo 2040
// (I2C) and/or the robot's ESP32 (ESP-NOW relay) out. See the README for the wiring and pairing.
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

extern "C" {
#include <btstack_port_esp32.h>
#include <btstack_run_loop.h>
#include <uni.h>
}

#include "Clock.h"
#include "Config.h"
#include "EspNowLink.h"
#include "Gamepad.h"
#include "I2CLink.h"
#include "Link.h"

namespace {

// BOOT button: a press opens a pairing window; holding it forgets every gamepad
void handlePairButton(uint32_t now) {
    static uint32_t downSinceMs = 0;
    static bool forgot = false;
    const bool down = gpio_get_level((gpio_num_t)config::PAIR_BUTTON_PIN) == 0;

    if (down) {
        if (downSinceMs == 0) downSinceMs = now;
        if (!forgot && now - downSinceMs >= config::FORGET_HOLD_MS) {
            gamepad::forgetAll();
            forgot = true;
        }
    } else {
        if (downSinceMs != 0 && !forgot && now - downSinceMs >= 50) {
            gamepad::startPairing();
        }
        downSinceMs = 0;
        forgot = false;
    }
}

// Status LED: fast blink while pairing, on while something drives, off otherwise
void updateLed(uint32_t now, Source source) {
    if (config::STATUS_LED_PIN < 0) return;
    const bool on = gamepad::pairing() ? ((now / 125) % 2 == 0) : source != SOURCE_NONE;
    gpio_set_level((gpio_num_t)config::STATUS_LED_PIN, on ? 1 : 0);
}

void printControl(uint32_t now, Source source) {
    static uint32_t lastPrintMs = 0;
    if (!config::PRINT_CONTROL_DATA ||
        (lastPrintMs != 0 && now - lastPrintMs < config::CONTROL_PRINT_PERIOD_MS)) {
        return;
    }
    lastPrintMs = now;

    const ControlPacket packet = link::control();
    printf("Control [%s]: sticks=(%d,%d) right=(%d,%d) tilt=(%d,%d) height=%d "
           "command=%u args=(%d,%d,%d)\n",
           link::sourceName(source), packet.joystick1X, packet.joystick1Y, packet.joystick2X,
           packet.joystick2Y, packet.tiltYaw, packet.tiltZ, packet.currentHeight,
           static_cast<unsigned>(packet.command), packet.commandArgs[0], packet.commandArgs[1],
           packet.commandArgs[2]);
}

void receiverTask(void*) {
    gpio_reset_pin((gpio_num_t)config::PAIR_BUTTON_PIN);
    gpio_set_direction((gpio_num_t)config::PAIR_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)config::PAIR_BUTTON_PIN, GPIO_PULLUP_ONLY);
    if (config::STATUS_LED_PIN >= 0) {
        gpio_reset_pin((gpio_num_t)config::STATUS_LED_PIN);
        gpio_set_direction((gpio_num_t)config::STATUS_LED_PIN, GPIO_MODE_OUTPUT);
    }

    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        const uint32_t now = nowMs();

        ControlPacket fromGamepad;
        const uint32_t lastReport = gamepad::update(now, link::status(), fromGamepad);
        link::gamepadInput(fromGamepad, lastReport);

        const Source source = link::update(now);
        printControl(now, source);
        espnow::relay();

        handlePairButton(now);
        gamepad::checkPairingTimeout(now);
        updateLed(now, source);
        espnow::report();
        i2c::report();

        vTaskDelayUntil(&wake, pdMS_TO_TICKS(config::LOOP_PERIOD_MS));
    }
}

}  // namespace

extern "C" void app_main(void) {
    // Wi-Fi (and NVS, which Bluepad32 also uses) before Bluetooth
    espnow::begin();
    i2c::begin();
    xTaskCreatePinnedToCore(receiverTask, "receiver", 4096, nullptr, 5, nullptr, 1);

    // Bluepad32 runs on BTstack's run loop, on this task. It never returns.
    btstack_init();
    uni_platform_set_custom(gamepad::platform());
    uni_init(0, nullptr);
    btstack_run_loop_execute();
}
