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

void create_dropdown(page * items, int size, lv_obj_t * parent, const char * label_text) {
    lv_obj_t * cont = lv_menu_cont_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * label = lv_label_create(cont);
    lv_label_set_text(label, label_text);

    lv_obj_t * dd = lv_dropdown_create(cont);
    char options[128] = "";
    for (int i = 0; i < size; i++) {
        strcat(options, items[i].item);
        if (i < size - 1) strcat(options, "\n");
    }
    lv_dropdown_set_options(dd, options);
    lv_dropdown_set_selected(dd, 0);
    lv_obj_set_width(dd, 200);
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
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(menu);
    lv_obj_set_style_bg_color(menu, lv_color_black(), 0);
    lv_obj_set_style_text_color(menu, lv_color_white(), 0);
    lv_obj_set_style_text_font(menu, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(lv_menu_get_main_header_back_btn(menu), lv_color_white(), 0);


    lv_obj_t * cont;
    lv_obj_t * label;

    // config page
    lv_obj_t * config_page = lv_menu_page_create(menu, "Config");
    create_dropdown(CONFIG, CONFIG_SIZE, config_page, "Leg"); // leg selection
    create_dropdown(LEG, LEG_SIZE, config_page, "Joint"); // joint selection

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

    // spacer to push icon down
    lv_obj_t * spacer = lv_obj_create(config_page);
    lv_obj_set_size(spacer, 10, 40); // width doesn't matter, height sets spacing
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(spacer, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // config page icon
    lv_obj_t * config_icon = lv_img_create(config_page);
    lv_img_set_src(config_icon, &normal_mode_icon);
    lv_obj_align_to(config_icon, cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    // apply button
    lv_obj_t * apply_btn = lv_btn_create(config_page);
    lv_obj_set_size(apply_btn, 120, 60);
    lv_obj_align_to(apply_btn, config_icon, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 10);
    lv_obj_t * apply_label = lv_label_create(apply_btn);
    lv_label_set_text(apply_label, "Apply");
    lv_obj_set_style_text_font(apply_label, &lv_font_montserrat_24, 0);
    lv_obj_center(apply_label);

    // gait page
    lv_obj_t * gait_page = lv_menu_page_create(menu, "Gait");
    create_dropdown(GAIT, GAIT_SIZE, gait_page, "Gait Type");

    // mode page
    lv_obj_t * mode_page = lv_menu_page_create(menu, "Mode");
    create_dropdown(MODE, MODE_SIZE, mode_page, "Mode Type");

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
