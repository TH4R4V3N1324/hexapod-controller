#include <Arduino.h>
#include "CommEspNow.h"
#include "CommI2C.h"

// Instances of packets
ControlPacket controlPacket = {};
HexPacket hexPacket = {};

void setup() {
  Serial.begin(115200);
  initEspNow(); // Initialize ESP-NOW communication
  initI2C();    // Initialize I2C communication
}

void loop() {
  // Do nothing
}