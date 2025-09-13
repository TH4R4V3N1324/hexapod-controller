#include "bitmaps.h"
#include "DataPacket.h"
#include "Input.h"
#include "Pages.h"
#include "Display.h"
#include "ControllerLogic.h"
#include <Arduino.h>
#include "USB.h"

USBCDC USBSerial;

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	USBSerial.begin();
	USB.begin();

	dataPacket.initializeESPNow();
	input.initializeInput();
	display.initializeDisplay();

	pages.state = STATE_HOME;
}

void loop() {
	dataPacket.sendData();
  	input.readInputData();
	controllerLogic.mainFSM();
  	pages.displayPages();
}

