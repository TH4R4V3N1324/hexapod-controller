#ifndef COM_I2C_H
#define COM_I2C_H
#include <Arduino.h>
#include <Wire.h>
#include "CommEspNow.h"

// The Servo 2040 is the I2C master; it reads ControlPackets and writes HexPackets
#define I2C_SLAVE_ADDR 0x08

volatile int badI2CLength = -1;

// Master read: send the latest ControlPacket
static void requestEventI2C() {
    // Snapshot under the lock so the master never gets a half-updated packet
    ControlPacket snapshot;
    portENTER_CRITICAL(&packetMux);
    snapshot = controlPacket;
    portEXIT_CRITICAL(&packetMux);
    Wire.write((const uint8_t *)&snapshot, sizeof(ControlPacket));
}

// Master write: the robot's status, passed back to the controller with the next reply
static void receiveEventI2C(int numBytes) {
    if (numBytes != sizeof(HexPacket)) {
        while (Wire.available()) Wire.read();
        badI2CLength = numBytes;
        return;
    }
    HexPacket incoming;
    Wire.readBytes((char *)&incoming, sizeof(HexPacket));
    portENTER_CRITICAL(&packetMux);
    hexPacket = incoming;
    portEXIT_CRITICAL(&packetMux);
}

// Call from loop(): prints problems the I2C callbacks noticed
void reportI2CProblems() {
    int len = badI2CLength;
    if (len >= 0) {
        badI2CLength = -1;
        Serial.printf("Ignored a %d-byte I2C write (expected %u): is the Servo 2040's "
                      "DataPacket.h the same as HexapodPackets.h?\n", len, (unsigned)sizeof(HexPacket));
    }
}

void initI2C() {
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onRequest(requestEventI2C);
    Wire.onReceive(receiveEventI2C);
}

#endif
