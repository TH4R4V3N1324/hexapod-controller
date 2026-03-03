#ifndef INPUT_H
#define INPUT_H
#include "DataPacket.h"
#include <stdint.h>
#include <Arduino.h>
#define stick1X 2
#define stick1Y 1
#define stick2X 5 
#define stick2Y 6 
#define button1 38
#define button2 39
#define button3 40
#define button4 41
#define switch1 15
#define switch2 14
#define switch3 16
#define switch4 17
#define encoderA 9
#define encoderB 8
#define encoderButton 7

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