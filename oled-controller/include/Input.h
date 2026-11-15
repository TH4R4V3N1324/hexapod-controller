#ifndef INPUT_H
#define INPUT_H
#include "DataPacket.h"
#include <stdint.h>
#include <Arduino.h>
#include "Board.h"   // pins for the PCB revision being built
#define stick1X PIN_STICK1_X
#define stick1Y PIN_STICK1_Y
#define stick2X PIN_STICK2_X
#define stick2Y PIN_STICK2_Y
#define button1 PIN_BUTTON1
#define button2 PIN_BUTTON2
#define button3 PIN_BUTTON3
#define button4 PIN_BUTTON4
#define switch1 PIN_SWITCH1
#define switch2 PIN_SWITCH2
#define switch3 PIN_SWITCH3
#define switch4 PIN_SWITCH4
#define encoderA PIN_ENC_A
#define encoderB PIN_ENC_B
#define encoderButton PIN_ENC_BTN

// Declaration of encoder states
enum EncoderStates {AB, Ab, aB, ab};

class Input {
private:
    static EncoderStates encoderState;
    int center1X = 0;
    int center1Y = 0;
    int center2X = 0;
    int center2Y = 0;

public:
    static volatile int encoderCount;
    int lastEncoderCount = 0;
    int encoderDelta = 0;
    int encoderCountPerIndent = 4;
    bool button1Z0 = false;
    bool button1Z1 = false;
    bool button2Z0 = false;
    bool button2Z1 = false;
    bool button3Z0 = false;
    bool button3Z1 = false;
    bool button4Z0 = false;
    bool button4Z1 = false;
    bool encoderButtonZ0 = false;
    bool encoderButtonZ1 = false;
    void readStickData();
    void readButtonData();
    void readInputData();
    static void doEncoderFSM();
    void initializeEncoder();
    void initializeInput();
    int calibrateCenter(int pin);
    void initializeJoystick();
    void printControllerInputs(Print& serial);
};

extern Input input;

#endif