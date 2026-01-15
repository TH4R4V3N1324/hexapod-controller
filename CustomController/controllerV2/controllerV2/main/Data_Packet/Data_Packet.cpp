#include "Data_Packet.h"

static const char *ESPN_TAG = "ESP_NOW";
static const char *I2C_TAG = "I2C";
uint8_t receiverMAC[6] = {0x30, 0xC9, 0x22, 0x28, 0x73, 0x4C};
static int64_t previousTimeUs = 0;

bool receiverConnected = false;

uint8_t byte;
LeftPacket leftPacket;
RightPacket rightPacket;
uint8_t left_packet_index = 0;
bool left_receiving = false;
uint8_t right_packet_index = 0;
bool right_receiving = false;

UARTController left_ctrl = {
    .packet = &leftPacket,
    .packet_size = sizeof(LeftPacket),
    .receiving = false,
    .index = 0,
    .request_cmd = 0x01
};

UARTController right_ctrl = {
    .packet = &rightPacket,
    .packet_size = sizeof(RightPacket),
    .receiving = false,
    .index = 0,
    .request_cmd = 0x02
};

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
        ESP_LOGE(ESPN_TAG, "Send failed: %s", esp_err_to_name(result));
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
}

/*
@brief Prints the MAC address of the device
*/
void print_mac(void) {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    ESP_LOGI("MAC", "STA MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/*
@brief ESP-NOW send callback
@param mac_addr The MAC address of the receiver
@param status The send status (success or fail)
*/
static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        receiverConnected = true;
    } else {
        //ESP_LOGW(TAG, "Send failed");
        receiverConnected = false;
    }
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

    ESP_ERROR_CHECK(
        esp_now_register_send_cb(onDataSent)
    );

    print_mac();
}

/*
@brief Scans the I2C bus for connected devices and logs their addresses
*/
void scanI2CDevices() {
    uint8_t devices[128];
    int numDevices = 0;

    for (uint8_t address = 1; address < 127; address++) {
        // Build a minimal write command to test ACK from the slave
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t result = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (result == ESP_OK) {
            devices[numDevices++] = address;
        }
    }

    ESP_LOGI(I2C_TAG, "I2C Devices Found: %d", numDevices);
    for (int i = 0; i < numDevices; i++) {
        ESP_LOGI(I2C_TAG, " - Address: 0x%02X", devices[i]);
    }
}

/*
@brief Initializes UART for communication with the left controller
*/
void initUart() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM, UART_BUF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX1, RX1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/*
@brief Sends a request command to the specified UART controller
@param ctrl Pointer to the UARTController struct
*/
void request_controller(UARTController *ctrl) {
    uart_write_bytes(UART_NUM, &ctrl->request_cmd, 1);
}

/*
@brief Handles receiving data from the specified UART controller
@param ctrl Pointer to the UARTController struct
@param log_tag Tag for logging
*/
void uart_receive_loop(UARTController *ctrl, const char *log_tag) {
    uint8_t byte;

    while(uart_read_bytes(UART_NUM, &byte, 1, 0) > 0) {

        // Waiting for START_BYTE
        if(!ctrl->receiving) {
            if(byte == START_BYTE) {
                ctrl->receiving = true;
                ctrl->index = 0;
                ((uint8_t*)ctrl->packet)[ctrl->index++] = byte;
            }
            continue;
        }

        // Receiving packet
        ((uint8_t*)ctrl->packet)[ctrl->index++] = byte;

        // Packet complete
        if(ctrl->index == ctrl->packet_size) {
            ctrl->receiving = false;

            // Calculate checksum
            const uint8_t *data = (const uint8_t*)ctrl->packet;
            uint8_t cs = 0;
            for(size_t i = 0; i < ctrl->packet_size - 1; i++) cs ^= data[i];

            uint8_t packet_cs = ((uint8_t*)ctrl->packet)[ctrl->packet_size - 1];

            if(cs != packet_cs) ESP_LOGW(log_tag, "Checksum error");
        }

        // Overflow safety
        if(ctrl->index > ctrl->packet_size) {
            ctrl->receiving = false;
            ctrl->index = 0;
        }
    }
}