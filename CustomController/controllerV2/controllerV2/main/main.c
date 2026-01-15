#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "GT911.h"
#include "SD_MMC.h"
#include "LVGL_Example.h"
#include "Wireless.h"
#include "BAT_Driver.h"
#include "Display.h"
#include "Data_Packet.h"

void Driver_Loop(void *parameter)
{
    while(1)
    {
        QMI8658_Loop();
        RTC_Loop();
        BAT_Get_Volts();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}
void Driver_Init(void)
{
    Flash_Searching();
    BAT_Init();
    I2C_Init();
    PCF85063_Init();
    QMI8658_Init();
    EXIO_Init();                    // Example Initialize EXIO
    initUart();
    xTaskCreatePinnedToCore(
        Driver_Loop, 
        "Other Driver task",
        4096, 
        NULL, 
        3, 
        NULL, 
        0);
}

void app_main(void)
{   
    Wireless_Init();
    initESPNow();  // Initialize ESP-NOW after WiFi
    Driver_Init();
    LCD_Init();
    Touch_Init();
    SD_Init();
    LVGL_Init();
    homePage();
    scanI2CDevices();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        sendData();
        //request_left_controller();
        //left_controller_loop();
        request_right_controller();
        right_controller_loop();
        lv_timer_handler();
    }
}
