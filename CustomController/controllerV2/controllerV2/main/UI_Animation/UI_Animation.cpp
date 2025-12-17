#include "UI_Animation.h"

#define LEG_STRIDE 10
#define LEG_LIFT   10

hex_leg_line leg1 = {0, 30, 30, 0};
hex_leg_line leg2 = {0, 0, 40, 0}; 
hex_leg_line leg3 = {0, 0, 30, 30};
hex_leg_line leg4 = {30, 0, 0, 30};
hex_leg_line leg5 = {40, 0, 0, 0};
hex_leg_line leg6 = {30, 30, 0, 0};
hex_leg_line legs[] = {leg1, leg2, leg3, leg4, leg5, leg6};
int leg_angles[] = {45, 0, -45, -135, 180, 135};
lv_coord_t line_offset = 17;
static lv_point_t leg_points[6][2];

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

void leg_anim_cb(void * var, int32_t v)
{
    hex_leg_anim_t * leg = (hex_leg_anim_t *)var;

    leg->pts[0].x = leg->hip_x;
    leg->pts[0].y = leg->hip_y;

    leg->pts[1].x = leg->hip_x + (int)(leg->dir_x * (30 + v));
    leg->pts[1].y = leg->hip_y + (int)(leg->dir_y * (30 + v))
                    - (abs(v) < LEG_LIFT ? LEG_LIFT - abs(v) : 0);

    lv_line_set_points(leg->line, leg->pts, 2);
}

void hexapodIcon(lv_obj_t * parent) {
    // Hexapod container
    lv_obj_t *hexapod = lv_obj_create(parent);
    lv_obj_set_size(hexapod, 250, 250);
    lv_obj_center(hexapod);
    lv_obj_clear_flag(hexapod, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(hexapod, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hexapod, 0, 0);

    // Hexapod body
    lv_obj_t *body = lv_obj_create(hexapod);
    lv_obj_set_size(body, 120, 120);
    lv_obj_set_style_radius(body, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(body, lv_color_white(), 0);
    lv_obj_center(body);

    // Hexapod left eye
    lv_obj_t *eye_left = lv_obj_create(body);
    lv_obj_clear_flag(eye_left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(eye_left, 40, 40);
    lv_obj_set_style_radius(eye_left, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_left, lv_color_black(), 0);
    lv_obj_align(eye_left, LV_ALIGN_CENTER, -20, -10);

    // left eye pupil and reflections
    lv_obj_t *pupil_left = lv_obj_create(eye_left);
    lv_obj_t *reflect1_left = lv_obj_create(eye_left);
    lv_obj_t *reflect2_left = lv_obj_create(eye_left);

    lv_obj_set_size(pupil_left, 15, 15);
    lv_obj_set_style_radius(pupil_left, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_left, lv_color_white(), 0);
    lv_obj_align(pupil_left, LV_ALIGN_CENTER, -7, -7);
    
    lv_obj_set_size(reflect1_left, 10, 10);
    lv_obj_set_style_radius(reflect1_left, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(reflect1_left, lv_color_white(), 0);
    lv_obj_align(reflect1_left, LV_ALIGN_CENTER, 3, 4);

    lv_obj_set_size(reflect2_left, 8, 8);
    lv_obj_set_style_radius(reflect2_left, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(reflect2_left, lv_color_white(), 0);
    lv_obj_align(reflect2_left, LV_ALIGN_CENTER, 6, -3);

    // Hexapod right eye (duplicate, not mirrored)
    lv_obj_t *eye_right = lv_obj_create(body);
    lv_obj_clear_flag(eye_right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(eye_right, 40, 40);
    lv_obj_set_style_radius(eye_right, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_right, lv_color_black(), 0);
    lv_obj_align(eye_right, LV_ALIGN_CENTER, 20, -10); // same y, positive x offset

    // right eye pupil and reflections (same offsets as left)
    lv_obj_t *pupil_right = lv_obj_create(eye_right);
    lv_obj_t *reflect1_right = lv_obj_create(eye_right);
    lv_obj_t *reflect2_right = lv_obj_create(eye_right);

    lv_obj_set_size(pupil_right, 15, 15);
    lv_obj_set_style_radius(pupil_right, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_right, lv_color_white(), 0);
    lv_obj_align(pupil_right, LV_ALIGN_CENTER, -7, -7);
    
    lv_obj_set_size(reflect1_right, 10, 10);
    lv_obj_set_style_radius(reflect1_right, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(reflect1_right, lv_color_white(), 0);
    lv_obj_align(reflect1_right, LV_ALIGN_CENTER, 3, 4);

    lv_obj_set_size(reflect2_right, 8, 8);
    lv_obj_set_style_radius(reflect2_right, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(reflect2_right, lv_color_white(), 0);
    lv_obj_align(reflect2_right, LV_ALIGN_CENTER, 6, -3);

    // Legs
    int radius = 60;
    int box_size = 250;

    static hex_leg_anim_t leg_anim[6];

    for (int i = 0; i < 6; i++) {
        lv_coord_t leg_pos_x = box_size / 2 + radius * cos(leg_angles[i] * M_PI / 180);
        lv_coord_t leg_pos_y = box_size / 2 - radius * sin(leg_angles[i] * M_PI / 180);

        lv_obj_t * leg_upper = lv_line_create(hexapod);

        lv_coord_t start_offset_x = legs[i].line_start_x + line_offset;
        lv_coord_t start_offset_y = legs[i].line_start_y + line_offset;

        leg_points[i][0].x = legs[i].line_start_x;
        leg_points[i][0].y = legs[i].line_start_y;
        leg_points[i][1].x = legs[i].line_end_x;
        leg_points[i][1].y = legs[i].line_end_y;

        lv_line_set_points(leg_upper, leg_points[i], 2);

        lv_obj_set_style_line_width(leg_upper, 25, LV_PART_MAIN);
        lv_obj_set_style_line_color(leg_upper, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_line_rounded(leg_upper, true, LV_PART_MAIN);

        /* Position */
        lv_obj_set_pos(leg_upper, leg_pos_x - start_offset_x, leg_pos_y - start_offset_y);

        float angle_rad = leg_angles[i] * M_PI / 180.0f;

        leg_anim[i].line = leg_upper;
        leg_anim[i].pts  = leg_points[i];

        leg_anim[i].hip_x = leg_points[i][0].x;
        leg_anim[i].hip_y = leg_points[i][0].y;

        leg_anim[i].dir_x = cosf(angle_rad);
        leg_anim[i].dir_y = -sinf(angle_rad);

        uint32_t cycle = 1400;
        leg_anim[i].phase = (i % 2) ? (cycle / 2) : 0;

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, &leg_anim[i]);
        lv_anim_set_exec_cb(&a, leg_anim_cb);

        lv_anim_set_values(&a, -LEG_STRIDE, LEG_STRIDE);
        lv_anim_set_time(&a, 700);
        lv_anim_set_playback_time(&a, 700);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_delay(&a, leg_anim[i].phase);

        lv_anim_start(&a);
    }
}