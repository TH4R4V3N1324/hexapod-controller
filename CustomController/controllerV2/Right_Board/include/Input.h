#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "DataPacket.h"

#define JOY_X_PIN PIN_A1
#define JOY_Y_PIN PIN_A2
#define JOY_BTN_PIN PIN_PC3
#define ENC_A_PIN PIN_PC2
#define ENC_B_PIN PIN_PC1
#define ENC_BTN_PIN PIN_PC0

// Declaration of encoder states
enum EncoderStates {AB, Ab, aB, ab};

class Input {
private:
    int centreX = 0;
    int centreY = 0;
    static volatile int encoderCount;
    static EncoderStates encoderState;

    void readStickData();
    uint8_t readButtonData();
    int calibrateCentre(int pin);
    void initJoystick();
    void initEncoder();
    static void readEncoderData();
public:
    void initInput();
    void readInput();
};

extern Input input;

#endif // INPUT_H