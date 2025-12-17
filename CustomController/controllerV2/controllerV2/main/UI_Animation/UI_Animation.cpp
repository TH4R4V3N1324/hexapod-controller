#include "UI_Animation.h"

static const char * idle_blink_frames[] = {
    "S:/ui/anims/idle/blink0.bin",
    "S:/ui/anims/idle/blink1.bin",
    "S:/ui/anims/idle/blink2.bin",
    "S:/ui/anims/idle/blink3.bin",
    "S:/ui/anims/idle/blink4.bin",
    "S:/ui/anims/idle/blink5.bin",
    "S:/ui/anims/idle/blink6.bin",
    "S:/ui/anims/idle/blink7.bin",
    "S:/ui/anims/idle/blink8.bin",
    "S:/ui/anims/idle/blink9.bin",
    "S:/ui/anims/idle/blink10.bin",
    "S:/ui/anims/idle/blink11.bin",
    "S:/ui/anims/idle/blink12.bin"
};

AnimationConfig idle_blink = {
    .frames = idle_blink_frames,
    .frame_count = sizeof(idle_blink_frames) / sizeof(idle_blink_frames[0]),
    .current_frame = 0
};

static void animation_timer_cb(lv_timer_t * timer) {
    AnimationConfig * anim_config = (AnimationConfig *)timer->user_data;
    lv_img_set_src(anim_config->anim_img, anim_config->frames[anim_config->current_frame]);
    anim_config->current_frame = (anim_config->current_frame + 1) % anim_config->frame_count;
}

void playAnimation(lv_obj_t * anim_img, AnimationConfig * anim_config, int frame_delay_ms) {
    anim_config->anim_img = anim_img;
    lv_timer_t * timer = lv_timer_create(animation_timer_cb, frame_delay_ms, anim_config);
}