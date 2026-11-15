#include "bitmaps.h"
#include "DataPacket.h"
#include "Input.h"
#include "Pages.h"
#include "Display.h"
#include "ControllerLogic.h"
#include <Arduino.h>
#if CONFIG_IDF_TARGET_ESP32S2
#include "USB.h"
USBCDC USBSerial;   // v3 (ESP32-S2): TinyUSB CDC. On v5 (ESP32-S3) Serial is the built-in USB serial/JTAG port
#endif

void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true);
#if CONFIG_IDF_TARGET_ESP32S2
	USBSerial.begin();
	USB.begin();
#endif

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

