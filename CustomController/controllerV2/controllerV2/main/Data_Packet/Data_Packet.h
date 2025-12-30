#ifndef DATA_PACKET_H
#define DATA_PACKET_H

#include <stdint.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_MASTER_NUM I2C_NUM_0 // Use I2C_NUM_0 to match i2c_port_t type

typedef enum : uint8_t {
    CMD_NONE = 0,
    CMD_SET_GAIT,
    CMD_SET_MODE,
    CMD_SET_CONFIG,
    CMD_HOME_STANCE,
    CMD_REQUEST_CONFIG
} Command;

// Define the data structure with no padding
#pragma pack(push, 1)
// Define ControlPacket struct
typedef struct {
    int16_t joystick1X;
    int16_t joystick1Y;
    int16_t joystick2X;
    int16_t joystick2Y;
    int16_t currentHeight;
    Command command;
    int16_t commandArgs[3];
} ControlPacket;

// Define HexPacket struct
typedef struct {
    int16_t legConfigs[3];
    int16_t currentHeight;
    int16_t currentPhase;
    int16_t currentGait;
    int16_t currentMode;
} HexPacket;
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

extern ControlPacket controlPacket;
extern HexPacket hexPacket;
extern bool receiverConnected;

void sendData();
void initESPNow();
void scanI2CDevices();

#ifdef __cplusplus
}
#endif

#endif // ESP_NOW_H