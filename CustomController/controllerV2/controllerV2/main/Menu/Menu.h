#ifndef MENU_H
#define MENU_H

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
#include <math.h>
#define STATE_STACK_MAX 10

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

struct hex_leg_line {
    lv_coord_t line_start_x;
    lv_coord_t line_start_y;
    lv_coord_t line_end_x;
    lv_coord_t line_end_y;
};

enum Joints {coxa, femur, tibia};
enum Gaits {GAIT_TRIPOD, GAIT_RIPPLE, GAIT_WAVE, NUM_GAITS};
enum Modes {MODE_NORMAL, MODE_STRAFE, MODE_TILT, MODE_CONFIG, NUM_MODES};

void homePage();
void menuPage();
void debugInfo(lv_timer_t * timer);
void homeInfo(lv_timer_t * timer);

#ifdef __cplusplus
}
#endif

#endif // MENU_H