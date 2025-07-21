#include "bitmaps.h"
#include "DataPacket.h"
#include "Input.h"
#include "Pages.h"
#include "Display.h"
#include "ControllerLogic.h"
#include <Arduino.h>

void setup() {
	Serial.begin(115200);
}

void loop() {
	dataPacket.sendData();
  input.readInputData();
	controllerLogic.mainFSM();
  pages.displayPages();
}

