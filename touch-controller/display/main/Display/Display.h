#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include "LVGL_Driver.h"
#include "Bitmap.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "SD_MMC.h"
#include "Wireless.h"
#include "Buzzer.h"
#include "BAT_Driver.h"
#include "UI_Animation.h"
#include "Data_Packet.h"

#ifdef __cplusplus
extern "C" {
#endif

// Definition for page information
struct page {
    const char* item;
    const char* icon;
    const void* bitmap;
    lv_obj_t ** destinationPage;
    void (*callback)(void); // optional action
};

enum Joints {coxa, femur, tibia};
// Gait and Mode come from common/HexapodPackets.h (through Data_Packet.h)

void homePage();
void menuPage();
void debugInfo(lv_timer_t * timer);
void homeInfo(lv_timer_t * timer);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_H