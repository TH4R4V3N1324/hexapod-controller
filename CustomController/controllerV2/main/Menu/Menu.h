#ifndef MENU_H
#define MENU_H

#include "LVGL_Driver.h"
#include "Bitmap.h"
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

enum Joints {coxa, femur, tibia};
enum Gaits {GAIT_TRIPOD, GAIT_RIPPLE, GAIT_WAVE, NUM_GAITS};
enum Modes {MODE_NORMAL, MODE_STRAFE, MODE_TILT, MODE_CONFIG, NUM_MODES};

void homePage();
void menuPage();

#ifdef __cplusplus
}
#endif

#endif // MENU_H