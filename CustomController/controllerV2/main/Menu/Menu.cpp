#include "Menu.h"

#include "Menu.h"

// Global state stack and indices definitions
struct StateStack stateStack[STATE_STACK_MAX];
int stackTop = -1;
int stackIndex = 0;
States currentState = STATE_HOME;
States previousState = STATE_HOME;

page MENU[] = {
    {"Config", LV_SYMBOL_SETTINGS, STATE_CONFIG},
    {"Mode", LV_SYMBOL_EDIT, STATE_MODE},
    {"Gait", LV_SYMBOL_REFRESH, STATE_GAIT},
    {"Animation", LV_SYMBOL_PLAY, STATE_ANIMATION}
};
const int MENU_SIZE = sizeof(MENU) / sizeof(MENU[0]);

page CONFIG[] = {
    {"Leg1", LV_SYMBOL_SETTINGS, STATE_LEG},
    {"Leg2", LV_SYMBOL_SETTINGS, STATE_LEG},
    {"Leg3", LV_SYMBOL_SETTINGS, STATE_LEG},
    {"Leg4", LV_SYMBOL_SETTINGS, STATE_LEG},
    {"Leg5", LV_SYMBOL_SETTINGS, STATE_LEG},
    {"Leg6", LV_SYMBOL_SETTINGS, STATE_LEG}
};
const int CONFIG_SIZE = sizeof(CONFIG) / sizeof(CONFIG[0]);

page GAIT[] = {
    {"Tripod", LV_SYMBOL_REFRESH, STATE_NONE},
    {"Ripple", LV_SYMBOL_REFRESH, STATE_NONE},
    {"Wave", LV_SYMBOL_REFRESH, STATE_NONE}
};
const int GAIT_SIZE = sizeof(GAIT) / sizeof(GAIT[0]);

page MODE[] = {
    {"Normal", LV_SYMBOL_EDIT, STATE_NONE},
    {"Strafe", LV_SYMBOL_EDIT, STATE_NONE},
    {"Tilt", LV_SYMBOL_EDIT, STATE_NONE},
    {"Config", LV_SYMBOL_EDIT, STATE_NONE}
};
const int MODE_SIZE = sizeof(MODE) / sizeof(MODE[0]);

page LEG[] = {
    {"Coxa", LV_SYMBOL_SETTINGS, STATE_JOINT},
    {"Femur", LV_SYMBOL_SETTINGS, STATE_JOINT},
    {"Tibia", LV_SYMBOL_SETTINGS, STATE_JOINT}
};
const int LEG_SIZE = sizeof(LEG) / sizeof(LEG[0]);

page ANIMATION[] = {
    {"Animation1", LV_SYMBOL_PLAY, STATE_NONE},
    {"Animation2", LV_SYMBOL_PLAY, STATE_NONE},
    {"Animation3", LV_SYMBOL_PLAY, STATE_NONE}
};
const int ANIMATION_SIZE = sizeof(ANIMATION) / sizeof(ANIMATION[0]);

static void menu_btn_event_cb(lv_event_t *e) {
    menuPage();
}

static void back_btn_event_cb(lv_event_t *e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);
    // If on root page, go home
    if(lv_menu_back_btn_is_root(menu, obj)) {
        homePage();
    }
}

void homePage() {
    lv_obj_t * home_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(home_screen, lv_color_black(), 0);

    lv_obj_t *menu_btn = lv_btn_create(home_screen);
    lv_obj_set_size(menu_btn, 120, 60);
    lv_obj_align(menu_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_t *menu_label = lv_label_create(menu_btn);
    lv_obj_align(menu_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(menu_label, "Menu");
    lv_obj_set_style_text_font(menu_label, &lv_font_montserrat_24, 0);

    lv_obj_add_event_cb(menu_btn, menu_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_scr_load(home_screen);
}

void menuPage() {
    lv_obj_t * menu = lv_menu_create(lv_scr_act());
    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(menu, back_btn_event_cb, LV_EVENT_CLICKED, menu);
    lv_obj_set_size(menu, 480, 640);
    lv_obj_center(menu);
    lv_obj_set_style_bg_color(menu, lv_color_black(), 0);
    lv_obj_set_style_text_color(menu, lv_color_white(), 0);
    lv_obj_set_style_text_font(menu, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(lv_menu_get_main_header_back_btn(menu), lv_color_white(), 0);


    lv_obj_t * cont;
    lv_obj_t * label;

    // config page
    lv_obj_t * config_page = lv_menu_page_create(menu, "Config");

    // leg selection
    cont = lv_menu_cont_create(config_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(cont); 
    lv_label_set_text(label, "Leg");

    lv_obj_t * leg_dd = lv_dropdown_create(cont);
    char leg_options[128] = "";
    for (int i = 0; i < CONFIG_SIZE; i++) {
        strcat(leg_options, CONFIG[i].item);
        if (i < CONFIG_SIZE - 1) strcat(leg_options, "\n");
    }
    lv_dropdown_set_options(leg_dd, leg_options);
    lv_dropdown_set_selected(leg_dd, 0);
    lv_obj_set_width(leg_dd, 200);

    // joint selection
    cont = lv_menu_cont_create(config_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(cont);
    lv_label_set_text(label, "Joint");

    lv_obj_t * joint_dd = lv_dropdown_create(cont);
    char joint_options[128] = "";
    for (int i = 0; i < LEG_SIZE; i++) {
        strcat(joint_options, LEG[i].item);
        if (i < LEG_SIZE - 1) strcat(joint_options, "\n");
    }
    lv_dropdown_set_options(joint_dd, joint_options);
    lv_dropdown_set_selected(joint_dd, 0);
    lv_obj_set_width(joint_dd, 200);

    // offset selection
    cont = lv_menu_cont_create(config_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(cont);
    lv_label_set_text(label, "Offset");

    lv_obj_t * offset_spinbox = lv_spinbox_create(cont);
    lv_spinbox_set_range(offset_spinbox, -90, 90);
    lv_spinbox_set_value(offset_spinbox, 0);
    lv_spinbox_set_digit_format(offset_spinbox, 2, 0);
    lv_obj_set_width(offset_spinbox, 200);

    // config page icon
    lv_obj_t * config_icon = lv_img_create(config_page);
    lv_img_set_src(config_icon, &normal_mode_icon);
    lv_obj_align(config_icon, LV_ALIGN_BOTTOM_MID, 0, 20);

    // gait page
    lv_obj_t * gait_page = lv_menu_page_create(menu, "Gait");
    cont = lv_menu_cont_create(gait_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(cont);
    lv_label_set_text(label, "Gait Type");

    lv_obj_t * gait_dd = lv_dropdown_create(cont);
    lv_dropdown_set_options(gait_dd, "Tripod\nRipple\nWave");
    lv_dropdown_set_selected(gait_dd, 0);
    lv_obj_set_width(gait_dd, 200);

    // mode page
    lv_obj_t * mode_page = lv_menu_page_create(menu, "Mode");
    cont = lv_menu_cont_create(mode_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label = lv_label_create(cont);
    lv_label_set_text(label, "Mode Type");

    lv_obj_t * mode_dd = lv_dropdown_create(cont);
    lv_dropdown_set_options(mode_dd, "Normal\nStrafe\nTilt\nConfig");
    lv_dropdown_set_selected(mode_dd, 0);
    lv_obj_set_width(mode_dd, 200);

    // main menu
    lv_obj_t * main_menu = lv_menu_page_create(menu, "Main Menu");
    cont = lv_menu_cont_create(main_menu);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Config");
    lv_menu_set_load_page_event(menu, cont, config_page);

    cont = lv_menu_cont_create(main_menu);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Gait");
    lv_menu_set_load_page_event(menu, cont, gait_page);

    cont = lv_menu_cont_create(main_menu);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Mode");
    lv_menu_set_load_page_event(menu, cont, mode_page);

    lv_menu_set_page(menu, main_menu);
}
