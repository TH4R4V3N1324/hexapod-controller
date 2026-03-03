#include <stdint.h>
#include <Arduino.h>
#include "Input.h"
#include "DataPacket.h"

void setup() {
    Serial.begin(115200);
    input.initInput();
}

void loop() {
    if (Serial.available()) {
        uint8_t cmd = Serial.read();

        if (cmd == 0x02) {               // REQUEST_INPUT
            input.readInput();
            dataPacket.sendInputPacket();
        }
    }
}



