#include "Input.h"

EncoderStates Input::encoderState = AB;
volatile int Input::encoderCount = 0;

Input input;

/*
@brief Reads input data from various sources
*/
void Input::readInput() {
    readButtonData();
    readStickData();
}

/*
@brief Reads debounced states of buttons
*/
uint8_t Input::readButtonData() {
    inputPacket.joy_btn = !digitalRead(JOY_BTN_PIN);
    inputPacket.enc_btn = !digitalRead(ENC_BTN_PIN);
}

/*
@brief Calibrates the center position of a joystick
@param pin The analog pin connected to the joystick axis
@return The calibrated center value
*/
int Input::calibrateCentre(int pin) {
    long total = 0;
    const int samples = 20;

    for (int i = 0; i < samples; i++) {
        total += analogRead(pin);
        delay(10); // Small delay between samples
    }

    return total / samples;
}

/*
@brief Reads and interprets raw joystick data
*/
void Input::readStickData() {
    int xRaw = analogRead(JOY_X_PIN);
    int yRaw = analogRead(JOY_Y_PIN);

    // Subtract center
    int xCentered = xRaw - centreX;
    int yCentered = yRaw - centreY;

    // Optional axis flip
    xCentered = -xCentered;
    yCentered = -yCentered;

    // Scale to -128 to 127
    int x = (xCentered * 128L) / 512;
    int y = (yCentered * 128L) / 512;

    // Deadzone
    if (abs(x) < 25) x = 0;
    if (abs(y) < 25) y = 0;

    inputPacket.joy_x = constrain(x, -128, 127);
    inputPacket.joy_y = constrain(y, -128, 127);
}

/*
@brief Initializes the joystick center positions
*/
void Input::initJoystick() {
    centreX = calibrateCentre(JOY_X_PIN);
    centreY = calibrateCentre(JOY_Y_PIN);
}

/*
@brief Initializes input pins and joystick
*/
void Input::initInput() {
    pinMode(JOY_X_PIN, INPUT); // Joystick X
    pinMode(JOY_Y_PIN, INPUT); // Joystick Y
    pinMode(JOY_BTN_PIN, INPUT_PULLUP); // Joystick Button
    pinMode(ENC_A_PIN, INPUT_PULLUP); // Encoder A
    pinMode(ENC_B_PIN, INPUT_PULLUP); // Encoder B
    pinMode(ENC_BTN_PIN, INPUT_PULLUP); // Encoder Button

    // Encoder interrupt setup
    attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), Input::readEncoderData, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_B_PIN), Input::readEncoderData, CHANGE);

    // Initialize joystick
    initJoystick();

    // Initialize encoder
    initEncoder();
}

/*
@brief Initializes encoder state
*/
void Input::initEncoder() {
    if (digitalRead(ENC_A_PIN) && digitalRead(ENC_B_PIN)) encoderState = AB;
    if (!digitalRead(ENC_A_PIN) && digitalRead(ENC_B_PIN)) encoderState = aB;
    if (digitalRead(ENC_A_PIN) && !digitalRead(ENC_B_PIN)) encoderState = Ab;
    if (!digitalRead(ENC_A_PIN) && !digitalRead(ENC_B_PIN)) encoderState = ab;
}

/*
@brief Reads encoder data and updates the count
*/
void Input::readEncoderData() {
    switch (encoderState) {
        case AB:
            if (!digitalRead(ENC_A_PIN)) {encoderState = aB; inputPacket.enc_count++;}
            if (!digitalRead(ENC_B_PIN)) {encoderState = Ab; inputPacket.enc_count--;}
            break;
        case aB:
            if (!digitalRead(ENC_B_PIN)) {encoderState = ab; inputPacket.enc_count++;}
            if (digitalRead(ENC_A_PIN)) {encoderState = AB; inputPacket.enc_count--;}
            break;
        case Ab:
            if (digitalRead(ENC_B_PIN)) {encoderState = AB; inputPacket.enc_count++;}
            if (!digitalRead(ENC_A_PIN)) {encoderState = ab; inputPacket.enc_count--;}
            break;
        case ab:
            if (digitalRead(ENC_A_PIN)) {encoderState = Ab; inputPacket.enc_count++;}
            if (digitalRead(ENC_B_PIN)) {encoderState = aB; inputPacket.enc_count--;}
            break;
        default:
            break;
    }
}