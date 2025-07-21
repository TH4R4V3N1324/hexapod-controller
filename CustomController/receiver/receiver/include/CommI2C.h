#ifndef COM_I2C_H
#define COM_I2C_H
#include <Wire.h>
#include <stdint.h>
#include "ControlPacket.h"
#include "HexPacket.h"
#include <Arduino.h>
#define I2C_SLAVE_ADDR 0x08

// Function to handle I2C request event
static void requestEventI2C() {
    // Send the data structure to the master (Raspberry Pi Pico)
    Wire.write((uint8_t*)&controlPacket, sizeof(ControlPacket));
}

// Function to handle I2C receive event (ESP32 receives updated HexPacket from Pico)
static void receiveEventI2C(int numBytes) {
    if (numBytes != sizeof(HexPacket)) {
        Serial.println("Received I2C data size incorrect");
        return;
    }
    HexPacket incomingHexPacket = {};
    Wire.readBytes((char*)&incomingHexPacket, sizeof(HexPacket));
    if (hexPacketChanged(incomingHexPacket, hexPacket)) {
        hexPacket = incomingHexPacket; // Update the hexPacket if it has changed
    }
}

// Function to initialize I2C communication
void initI2C() {
    Wire.begin(I2C_SLAVE_ADDR); // Initialize I2C as slave with specified address
    Wire.onRequest(requestEventI2C); // Set callback function to handle I2C request event
    Wire.onReceive(receiveEventI2C); // Set callback function to handle I2C receive event
}

#endif