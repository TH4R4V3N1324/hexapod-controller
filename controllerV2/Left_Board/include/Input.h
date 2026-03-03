#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "DataPacket.h"

#define JOY_X_PIN PIN_A1
#define JOY_Y_PIN PIN_A2
#define JOY_BTN_PIN PIN_PC3
#define UP_BTN_PIN PIN_PC2
#define DOWN_BTN_PIN PIN_PC1
#define LEFT_BTN_PIN PIN_PC0
#define RIGHT_BTN_PIN PIN_PB0

class Input {
private:
    int centreX = 0;
    int centreY = 0;
    void readStickData();
    uint8_t readButtonData();
    int calibrateCentre(int pin);
    void initJoystick();
public:
    void initInput();
    void readInput();
};

extern Input input;

#endif // INPUT_H