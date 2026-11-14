#include "Input.h"

Input input;

/*
@brief Reads input data from various sources
*/
void Input::readInput() {
    inputPacket.buttons = readButtonData();
    readStickData();
}

/*
@brief Reads debounced states of buttons
*/
uint8_t Input::readButtonData() {
    uint8_t b = 0;

    if (!digitalRead(JOY_BTN_PIN))  b |= BTN_JOY;
    if (!digitalRead(UP_BTN_PIN))       b |= BTN_UP;
    if (!digitalRead(DOWN_BTN_PIN))     b |= BTN_DOWN;
    if (!digitalRead(LEFT_BTN_PIN))     b |= BTN_LEFT;
    if (!digitalRead(RIGHT_BTN_PIN))    b |= BTN_RIGHT;

    return b;
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
    pinMode(UP_BTN_PIN, INPUT_PULLUP); // Up Button
    pinMode(DOWN_BTN_PIN, INPUT_PULLUP); // Down Button
    pinMode(LEFT_BTN_PIN, INPUT_PULLUP); // Left Button
    pinMode(RIGHT_BTN_PIN, INPUT_PULLUP); // Right Button

    // Initialize joystick
    initJoystick();
}