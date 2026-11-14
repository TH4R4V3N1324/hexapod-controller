// Robot-side receiver for the Servo2040 stack: ESP-NOW from the handheld controller in,
// I2C to the Servo 2040 out. See the README for the packet flow.
#include <Arduino.h>
#include "CommEspNow.h"
#include "CommI2C.h"

void setup() {
  Serial.begin(115200);
  initEspNow();
  initI2C();
}

void loop() {
  checkFailsafe(); // Stop the robot if the controller link drops
  reportEspNowProblems();
  reportI2CProblems();
  delay(10);
}
