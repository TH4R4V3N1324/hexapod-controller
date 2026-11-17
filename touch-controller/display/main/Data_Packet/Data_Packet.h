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

// ControlPacket, HexPacket, Command, Gait, Mode: shared with the robot firmware
#include <gait_core/protocol/packets.h>

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