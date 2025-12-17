#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include "LVGL_Driver.h"
#include <math.h>

typedef struct {
    const char** frames; // Array of frame file paths
    int frame_count;     // Number of frames
    int current_frame;  // Current frame index
    lv_obj_t* anim_img; // Image object to display animation
} AnimationConfig;

struct hex_leg_line {
    lv_coord_t line_start_x;
    lv_coord_t line_start_y;
    lv_coord_t line_end_x;
    lv_coord_t line_end_y;
};

typedef struct {
    lv_obj_t * line;
    lv_point_t * pts;

    int hip_x;
    int hip_y;

    float dir_x;
    float dir_y;

    int phase;
} hex_leg_anim_t;

extern AnimationConfig idle_blink;

void playAnimation(lv_obj_t * anim_img, AnimationConfig * anim_config, int frame_delay_ms);
void hexapodIcon(lv_obj_t * parent);

#endif // UI_ANIMATION_H