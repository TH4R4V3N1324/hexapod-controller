#ifndef COM_ESPNOW_H
#define COM_ESPNOW_H
#include <WiFi.h>
#include <esp_now.h>
#include "ControlPacket.h"
#include "HexPacket.h"

// Mac address for hexapod esp32
uint8_t controllerMAC[] = {0xFC, 0x01, 0x2C, 0xD9, 0x41, 0xE4};

// Function to handle espNOW receive event (Arduino ESP32 signature)
static void receiveEventEspNOW(const uint8_t *mac, const uint8_t *data, int len) {
    if (len != sizeof(ControlPacket)) {
        Serial.println("Received ESP-NOW data size incorrect");
        Serial.print("Expected: ");
        Serial.print(sizeof(ControlPacket));
        Serial.print(" Received: ");
        Serial.println(len);
        return;
    }
    ControlPacket incomingControlPacket = {};
    memcpy(&incomingControlPacket, data, sizeof(ControlPacket));
    if (controlPacketChanged(incomingControlPacket, controlPacket)) {
        controlPacket = incomingControlPacket;
    }
    esp_now_send(controllerMAC, (uint8_t*)&hexPacket, sizeof(HexPacket));

    Serial.println("Received ESP-NOW packet from: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
}

// Function to initialize ESP-NOW communication
void initEspNow() {
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Initialize esp_now
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the receive callback function
    esp_now_register_recv_cb(receiveEventEspNOW);

    // Add the controller MAC address as a peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, controllerMAC, sizeof(controllerMAC));
    peerInfo.channel = 0; // Use the current channel
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add ESP-NOW peer");
        return;
    }
}

#endif