#ifndef BOARD_H
#define BOARD_H
// Pin map for each OLED controller PCB revision. platformio.ini sets BOARD_REV per environment:
//   env:v5  ESP32-S3-MINI-1U-N8  (current board, hexapod-hardware controllers/oled-controller)
//   env:v3  ESP32-S2-MINI-1-N4R2 (previous board)
// GPIO numbers, not module pin numbers.

#ifndef BOARD_REV
#define BOARD_REV 5
#endif

#if BOARD_REV == 5
// Sticks: ADC1 channels, so they keep working with Wi-Fi (ESP-NOW) on
#define PIN_STICK1_X     1
#define PIN_STICK1_Y     2
#define PIN_STICK2_X     5
#define PIN_STICK2_Y     3
#define PIN_STICK1_BTN   46   // strapping pin: internal pull-down at reset, the switch only pulls it low, so boot is unaffected
#define PIN_STICK2_BTN   4
// Face buttons
#define PIN_BUTTON1      37
#define PIN_BUTTON2      39
#define PIN_BUTTON3      42
#define PIN_BUTTON4      38
// Shoulder switches (Alps SKRT side-push, one on each face at each top corner)
#define PIN_SWITCH1      14
#define PIN_SWITCH2      15
#define PIN_SWITCH3      35
#define PIN_SWITCH4      36
// Rotary encoder
#define PIN_ENC_A        8
#define PIN_ENC_B        6
#define PIN_ENC_BTN      7
// SSD1309 OLED, 4-wire SPI
#define PIN_OLED_CLK     10
#define PIN_OLED_MOSI    9
#define PIN_OLED_CS      11
#define PIN_OLED_DC      12
#define PIN_OLED_RST     13
// I2C bus: BQ25628E charger (0x6A), MAX17048 fuel gauge (0x36), BMI270 IMU (0x68), and the I2C Qwiic port (J9)
#define PIN_I2C_SDA      34
#define PIN_I2C_SCL      47
#define PIN_IMU_INT1     33
#define PIN_CHG_INT      48   // charger INT and fuel-gauge ALRT, open drain, shared pull-up (R23)
// UART Qwiic port (J10): TX out, RX in
#define PIN_QWIIC_TX     16
#define PIN_QWIIC_RX     17
// Power button sense (also on D5/R29); GPIO44 is U0RXD, so don't start Serial0 on its default pins
#define PIN_PWR_BTN      44

#elif BOARD_REV == 3
#define PIN_STICK1_X     2
#define PIN_STICK1_Y     1
#define PIN_STICK2_X     5
#define PIN_STICK2_Y     6
#define PIN_BUTTON1      38
#define PIN_BUTTON2      39
#define PIN_BUTTON3      40
#define PIN_BUTTON4      41
#define PIN_SWITCH1      15
#define PIN_SWITCH2      14
#define PIN_SWITCH3      16
#define PIN_SWITCH4      17
#define PIN_ENC_A        9
#define PIN_ENC_B        8
#define PIN_ENC_BTN      7
// esp32s2 SPI default is 36 (SCK) and 35 (MOSI)
#define PIN_OLED_CLK     36
#define PIN_OLED_MOSI    35
#define PIN_OLED_CS      34
#define PIN_OLED_DC      33
#define PIN_OLED_RST     21

#else
#error "Unknown BOARD_REV (expected 3 or 5)"
#endif

#endif
