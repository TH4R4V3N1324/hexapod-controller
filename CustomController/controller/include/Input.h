#ifndef INPUT_H
#define INPUT_H
#include "DataPacket.h"
#include <stdint.h>
#include <Arduino.h>
#define stick1X 16
#define stick1Y 17 
#define stick2X 18  
#define stick2Y 19  
#define button1 4
#define button2 5
#define button3 6
#define button4 7
#define switch1 8
#define switch2 9
#define switch3 10
#define switch4 11
#define encoderA 20
#define encoderB 21
#define encoderButton 33

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
};

extern Input input;

#endif