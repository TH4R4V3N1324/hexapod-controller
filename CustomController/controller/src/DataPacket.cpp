#include "DataPacket.h"

// Define global packets
ControlPacket controlPacket = {};
HexPacket hexPacket = {};
DataPacket dataPacket;

// Sends data to Hexapod esp32 at regular intervals
void DataPacket::sendData() {
  unsigned long currentTime = millis();
  if (currentTime - previousTime > 10) {  // send every 10ms
	esp_now_send(receiverMAC, (uint8_t *)&controlPacket, sizeof(ControlPacket));
	previousTime = currentTime; // update the last send time
  }
}

// ESP-NOW receive callback for Arduino ESP32
void DataPacket::onHexDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
	static bool firstPacket = true;
	if (len == sizeof(HexPacket)) {
		memcpy(&hexPacket, data, sizeof(HexPacket));
		if (firstPacket) {
			controlPacket.currentHeight = hexPacket.currentHeight;
			firstPacket = false;
		}
	}
}

void DataPacket::initializeESPNow() {
	WiFi.mode(WIFI_STA);
	esp_now_init();
	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, receiverMAC, 6);
	peerInfo.channel = 0;
	peerInfo.encrypt = false;
	if (!esp_now_is_peer_exist(receiverMAC)) {
		esp_now_add_peer(&peerInfo);
	}
	esp_now_register_recv_cb(onHexDataReceived);
}
