#ifndef COM_ESPNOW_H
#define COM_ESPNOW_H
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "HexapodPackets.h"

// The handheld controller's MAC address. Packets from anyone else are ignored, and
// reportUnknownSender() prints their address so you can copy the right one in.
uint8_t controllerMAC[] = {0xFC, 0x01, 0x2C, 0xD9, 0x41, 0xE4};

// Link-loss failsafe: stop the robot if no valid packet arrives within this time
constexpr uint32_t FAILSAFE_TIMEOUT_MS = 250;

ControlPacket controlPacket = {};
HexPacket hexPacket = {};

// Guards both packets: the ESP-NOW callback, the I2C callbacks and loop() all touch them
portMUX_TYPE packetMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t lastControlPacketMs = 0;
volatile bool linkValid = false; // false until the first packet arrives, and after a timeout

// Set by the callback, printed from loop(): Serial is too slow for the Wi-Fi task
volatile bool unknownSenderHeard = false;
uint8_t unknownSenderMAC[6] = {};
volatile int badPacketLength = -1;

static void receiveEventEspNOW(const uint8_t *mac, const uint8_t *data, int len) {
    if (memcmp(mac, controllerMAC, sizeof(controllerMAC)) != 0) {
        memcpy(unknownSenderMAC, mac, sizeof(unknownSenderMAC));
        unknownSenderHeard = true;
        return;
    }
    if (len != sizeof(ControlPacket)) {
        badPacketLength = len;
        return;
    }

    HexPacket reply;
    portENTER_CRITICAL(&packetMux);
    memcpy(&controlPacket, data, sizeof(ControlPacket));
    lastControlPacketMs = millis();
    linkValid = true;
    reply = hexPacket;
    portEXIT_CRITICAL(&packetMux);

    esp_now_send(controllerMAC, (const uint8_t *)&reply, sizeof(HexPacket));
}

// Call from loop(). If the controller has gone quiet, zero the sticks and clear the
// command so the robot stops instead of acting on the last packet. Height is kept
// so the robot doesn't drop.
void checkFailsafe() {
    bool tripped = false;
    portENTER_CRITICAL(&packetMux);
    if (linkValid && (millis() - lastControlPacketMs) > FAILSAFE_TIMEOUT_MS) {
        linkValid = false;
        controlPacket.joystick1X = 0;
        controlPacket.joystick1Y = 0;
        controlPacket.joystick2X = 0;
        controlPacket.joystick2Y = 0;
        controlPacket.command = CMD_NONE;
        memset(controlPacket.commandArgs, 0, sizeof(controlPacket.commandArgs));
        tripped = true;
    }
    portEXIT_CRITICAL(&packetMux);
    if (tripped) {
        Serial.printf("Failsafe: no packet from the controller for %u ms, sticks centred\n",
                      (unsigned)FAILSAFE_TIMEOUT_MS);
    }
}

// Call from loop(): prints problems the ESP-NOW callback noticed
void reportEspNowProblems() {
    if (unknownSenderHeard) {
        unknownSenderHeard = false;
        Serial.printf("Ignored a packet from %02X:%02X:%02X:%02X:%02X:%02X "
                      "(not controllerMAC in CommEspNow.h)\n",
                      unknownSenderMAC[0], unknownSenderMAC[1], unknownSenderMAC[2],
                      unknownSenderMAC[3], unknownSenderMAC[4], unknownSenderMAC[5]);
    }
    int len = badPacketLength;
    if (len >= 0) {
        badPacketLength = -1;
        Serial.printf("Ignored a %d-byte packet from the controller (expected %u): "
                      "is its HexapodPackets.h the same?\n", len, (unsigned)sizeof(ControlPacket));
    }
}

bool initEspNow() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.print("Receiver MAC (put this in the controller): ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }
    esp_now_register_recv_cb(receiveEventEspNOW);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, controllerMAC, sizeof(controllerMAC));
    peerInfo.channel = 0; // the current channel
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add ESP-NOW peer");
        return false;
    }
    return true;
}

#endif
