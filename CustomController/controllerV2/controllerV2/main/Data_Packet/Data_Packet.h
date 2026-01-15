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
#include "driver/uart.h"

#define I2C_MASTER_NUM I2C_NUM_0

#define UART_NUM UART_NUM_1
#define RX1 44
#define TX1 43
#define UART_BUF_SIZE 256

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

typedef struct __attribute__((packed)) {
    uint8_t start;    
    uint8_t version;  
    int16_t joy_x;
    int16_t joy_y;
    uint8_t buttons; 
    uint8_t checksum;
} LeftPacket;

typedef struct __attribute__((packed)) {
    uint8_t start;
    uint8_t version; 
    int16_t joy_x;
    int16_t joy_y;
    int8_t joy_btn;
    int16_t enc_count;
    uint8_t enc_btn;
    uint8_t checksum;
} RightPacket;

#define START_BYTE 0xAA

#define BTN_JOY   (1 << 0)
#define BTN_UP    (1 << 1)
#define BTN_DOWN  (1 << 2)
#define BTN_LEFT  (1 << 3)
#define BTN_RIGHT (1 << 4)

typedef struct {
    void *packet;              // Pointer to packet struct (LeftPacket / RightPacket)
    size_t packet_size;        // Size of the packet
    bool receiving;            // Are we currently receiving a packet?
    size_t index;              // Current byte index
    uint8_t request_cmd;       // Command to request this controller
} UARTController;

#ifdef __cplusplus
extern "C" {
#endif

extern ControlPacket controlPacket;
extern HexPacket hexPacket;
extern LeftPacket leftPacket;
extern RightPacket rightPacket;
extern bool receiverConnected;
extern UARTController left_ctrl;
extern UARTController right_ctrl;

void sendData();
void initESPNow();
void scanI2CDevices();
void initUart();
void request_controller(UARTController *ctrl);
void uart_receive_loop(UARTController *ctrl, const char *log_tag);

#ifdef __cplusplus
}
#endif

#endif // ESP_NOW_H