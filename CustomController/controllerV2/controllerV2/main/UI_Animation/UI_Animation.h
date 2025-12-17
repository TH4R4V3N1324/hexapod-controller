#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include "LVGL_Driver.h"

typedef struct {
    const char** frames; // Array of frame file paths
    int frame_count;     // Number of frames
    int current_frame;  // Current frame index
    lv_obj_t* anim_img; // Image object to display animation
} AnimationConfig;

extern AnimationConfig idle_blink;

void playAnimation(lv_obj_t * anim_img, AnimationConfig * anim_config, int frame_delay_ms);

#endif // UI_ANIMATION_H