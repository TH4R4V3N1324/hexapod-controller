#include "Data_Packet.h"

static const char *TAG = "ESP_NOW";
uint8_t receiverMAC[6] = {0x30, 0xC9, 0x22, 0x28, 0x73, 0x4C};
static int64_t previousTimeUs = 0;

// Define the global packet variables
ControlPacket controlPacket = {
    .joystick1X = 0,
    .joystick1Y = 0,
    .joystick2X = 0,
    .joystick2Y = 0,
    .currentHeight = 0,
    .command = CMD_NONE,
    .commandArgs = {0, 0, 0}
};

HexPacket hexPacket = {};

/*
@brief Sends data to Hexapod esp32 at regular intervals
@note This function uses esp_now_send to transmit the controlPacket
*/
void sendData() {
  uint64_t currentTimeUs = esp_timer_get_time();
  if (currentTimeUs - previousTimeUs > 50000) {  // send every 10ms
	esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&controlPacket, sizeof(ControlPacket));

	if (result != ESP_OK) {
        ESP_LOGE(TAG, "Send failed: %s", esp_err_to_name(result));
    }

	previousTimeUs = currentTimeUs; // update the last send time
  }
}

/*
@brief ESP-NOW receive callback for Arduino ESP32
@param mac The MAC address of the sender
@param data The received data
@param len The length of the received data
*/
static void onHexDataReceived(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
	static bool firstPacket = true;
	if (len == sizeof(HexPacket)) {
		memcpy(&hexPacket, data, sizeof(HexPacket));
		if (firstPacket) {
			controlPacket.currentHeight = hexPacket.currentHeight;
			firstPacket = false;
		}
	}

    ESP_LOGI(TAG, "Data received from: %02X:%02X:%02X:%02X:%02X:%02X, Length: %d",
            recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
            recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5], len);
}

void print_mac(void) {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    ESP_LOGI("MAC", "STA MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/*
@brief Initializes ESP-NOW for communication
*/
void initESPNow(void) {
    ESP_ERROR_CHECK(esp_now_init());

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(receiverMAC)) {
        ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
    }

    ESP_ERROR_CHECK(
        esp_now_register_recv_cb(onHexDataReceived)
    );

    print_mac();
}

