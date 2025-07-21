#include "Input.h"

EncoderStates Input::encoderState = AB;
volatile int Input::encoderCount = 0;

Input input;

// State machine to interpret encoder states
void Input::doEncoderFSM() {
    switch (encoderState) {
        case AB:
            if (!digitalRead(encoderA)) {encoderState = aB; encoderCount++;}
            if (!digitalRead(encoderB)) {encoderState = Ab; encoderCount--;}
            break;
        case aB:
            if (!digitalRead(encoderB)) {encoderState = ab; encoderCount++;}
            if (digitalRead(encoderA)) {encoderState = AB; encoderCount--;}
            break;
        case Ab:
            if (digitalRead(encoderB)) {encoderState = AB; encoderCount++;}
            if (!digitalRead(encoderA)) {encoderState = ab; encoderCount--;}
            break;
        case ab:
            if (digitalRead(encoderA)) {encoderState = Ab; encoderCount++;}
            if (digitalRead(encoderB)) {encoderState = aB; encoderCount--;}
            break;
        default:
            printf("Invalid state");
            break;
    }
}

// Sets encoder initial state
void Input::initializeEncoder() {
    if (digitalRead(encoderA) && digitalRead(encoderB)) encoderState = AB;
    if (!digitalRead(encoderA) && digitalRead(encoderB)) encoderState = aB;
    if (digitalRead(encoderA) && !digitalRead(encoderB)) encoderState = Ab;
    if (!digitalRead(encoderA) && !digitalRead(encoderB)) encoderState = ab;
}

// Reads the data from controller inputs
void Input::readInputData() {
    encoderDelta = encoderCount - lastEncoderCount;
    readButtonData();
    readStickData();
}

// Reads debounced states of buttons
void Input::readButtonData() {
    button1Z1 = button1Z0; button1Z0 = digitalRead(button1);
    button2Z1 = button2Z0; button2Z0 = digitalRead(button2);
    button3Z1 = button3Z0; button3Z0 = digitalRead(button3);
    button4Z1 = button4Z0; button4Z0 = digitalRead(button4);
    encoderButtonZ1 = encoderButtonZ0; encoderButtonZ0 = digitalRead(encoderButton);
}

// Calibrates the centre position of a joystick
int Input::calibrateCenter(int pin) {
    long total = 0;
    const int samples = 20;

    for (int i = 0; i < samples; i++) {
        total += analogRead(pin);
        delay(10); // Small delay between samples
    }

    return total / samples;
}

// Reads and interprets raw joystick data
void Input::readStickData() {
    int x1Raw = analogRead(stick1X);
    int y1Raw = analogRead(stick1Y);
    int x2Raw = analogRead(stick2X);
    int y2Raw = analogRead(stick2Y);

    // Subtract center
    int x1Centered = x1Raw - center1X;
    int y1Centered = y1Raw - center1Y;
    int x2Centered = x2Raw - center2X;
    int y2Centered = y2Raw - center2Y;

    // Optional axis flip
    x1Centered = -x1Centered;
    x2Centered = -x2Centered;

    // Scale to -128 to 127
    int x1 = (x1Centered * 128L) / 2048;
    int y2 = (y1Centered * 128L) / 2048;
    int x2 = (x2Centered * 128L) / 2048;
    int y1 = (y2Centered * 128L) / 2048;

    // Deadzone
    if (abs(x1) < 25) x1 = 0;
    if (abs(y1) < 25) y1 = 0;
    if (abs(x2) < 25) x2 = 0;
    if (abs(y2) < 25) y2 = 0;

    controlPacket.joystick1X = constrain(x1, -128, 127);
    controlPacket.joystick1Y = constrain(y1, -128, 127);
    controlPacket.joystick2X = constrain(x2, -128, 127);
    controlPacket.joystick2Y = constrain(y2, -128, 127);
}

// Sets center positions for joysticks
void Input::initializeJoystick() {
    center1X = calibrateCenter(stick1X);
    center1Y = calibrateCenter(stick1Y);
    center2X = calibrateCenter(stick2X);
    center2Y = calibrateCenter(stick2Y);
}

// Initializes the input system
void Input::initializeInput() {
    pinMode(encoderButton, INPUT_PULLUP);
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    pinMode(button1, INPUT_PULLUP);
    pinMode(button2, INPUT_PULLUP);
    pinMode(button3, INPUT_PULLUP);
    pinMode(button4, INPUT_PULLUP);
    pinMode(switch1, INPUT_PULLUP);
    pinMode(switch2, INPUT_PULLUP);
    pinMode(switch3, INPUT_PULLUP);
    pinMode(switch4, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(encoderA), Input::doEncoderFSM, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encoderB), Input::doEncoderFSM, CHANGE);

    // Initialize encoder state
    initializeEncoder();

    // Read joystick center positions
    initializeJoystick();
}