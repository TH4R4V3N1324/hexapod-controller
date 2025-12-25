#include "Display.h"

lv_obj_t *config_page = NULL;
lv_obj_t *gait_page = NULL;
lv_obj_t *mode_page = NULL;
lv_obj_t *debug_page = NULL;

static lv_timer_t * auto_step_timer;
lv_obj_t * SD_Size;
lv_obj_t * FlashSize;
lv_obj_t * BAT_Volts;
lv_obj_t * Board_angle;
lv_obj_t * RTC_Time;
lv_obj_t * Wireless_Scan;

static lv_obj_t *battery_icon;
static lv_obj_t *battery_percent;
static lv_obj_t *conn_status;
static lv_obj_t *up_time;

// --- Battery voltage averaging ---
#define BAT_AVG_BUF_SIZE 100
static float bat_voltage_buf[BAT_AVG_BUF_SIZE] = {0};
static int bat_voltage_idx = 0;
static int bat_voltage_count = 0;

static float get_bat_voltage_avg() {
    float sum = 0;
    int n = bat_voltage_count < BAT_AVG_BUF_SIZE ? bat_voltage_count : BAT_AVG_BUF_SIZE;
    for (int i = 0; i < n; ++i) sum += bat_voltage_buf[i];
    return n > 0 ? sum / n : 0;
}

page MENU[] = {
    {"Config", LV_SYMBOL_SETTINGS, nullptr, &config_page, nullptr},
    {"Mode", LV_SYMBOL_EDIT, nullptr, &mode_page, nullptr},
    {"Gait", LV_SYMBOL_REFRESH, nullptr, &gait_page, nullptr},
    {"Animation", LV_SYMBOL_PLAY, nullptr, nullptr, nullptr},
    {"DEBUG", LV_SYMBOL_WARNING, nullptr, &debug_page, nullptr}
};
const int MENU_SIZE = sizeof(MENU) / sizeof(MENU[0]);

page LEG[] = {
    {"Leg1", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Leg2", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Leg3", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Leg4", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Leg5", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Leg6", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr}
};
const int LEG_SIZE = sizeof(LEG) / sizeof(LEG[0]);

page GAIT[] = {
    {"Tripod", LV_SYMBOL_REFRESH, nullptr, nullptr, nullptr},
    {"Ripple", LV_SYMBOL_REFRESH, nullptr, nullptr, nullptr},
    {"Wave", LV_SYMBOL_REFRESH, nullptr, nullptr, nullptr}
};
const int GAIT_SIZE = sizeof(GAIT) / sizeof(GAIT[0]);

page MODE[] = {
    {"Normal", LV_SYMBOL_EDIT, nullptr, nullptr, nullptr},
    {"Strafe", LV_SYMBOL_EDIT, nullptr, nullptr, nullptr},
    {"Tilt", LV_SYMBOL_EDIT, nullptr, nullptr, nullptr},
    {"Config", LV_SYMBOL_EDIT, nullptr, nullptr, nullptr}
};
const int MODE_SIZE = sizeof(MODE) / sizeof(MODE[0]);

page JOINT[] = {
    {"Coxa", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Femur", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr},
    {"Tibia", LV_SYMBOL_SETTINGS, nullptr, nullptr, nullptr}
};
const int JOINT_SIZE = sizeof(JOINT) / sizeof(JOINT[0]);

page ANIMATION[] = {
    {"Animation1", LV_SYMBOL_PLAY, nullptr, nullptr, nullptr},
    {"Animation2", LV_SYMBOL_PLAY, nullptr, nullptr, nullptr},
    {"Animation3", LV_SYMBOL_PLAY, nullptr, nullptr, nullptr}
};
const int ANIMATION_SIZE = sizeof(ANIMATION) / sizeof(ANIMATION[0]);

static void back_btn_event_cb(lv_event_t *e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);
    // If on root page, go home
    if(lv_menu_back_btn_is_root(menu, obj)) {
        homePage();
    }
}

static void ta_event_cb(lv_event_t * e) {}

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

void add_menu_item(lv_obj_t * menu, lv_obj_t * page, struct page item) {
    lv_obj_t * cont = lv_menu_cont_create(page);

    lv_obj_t * icon = lv_label_create(cont);
    lv_label_set_text(icon, item.icon);

    lv_obj_t * label = lv_label_create(cont);
    lv_label_set_text(label, item.item);

    lv_obj_t *dest = (item.destinationPage) ? *item.destinationPage : nullptr;
    if (dest == nullptr) {return;} // no action if no destination

    lv_menu_set_load_page_event(menu, cont, dest);
}

void homePage() {
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, &lv_font_montserrat_32);

    static lv_style_t home_style;
    lv_style_init(&home_style);
    lv_style_set_text_font(&home_style, &lv_font_montserrat_16);

    static lv_style_t icon_style;
    lv_style_init(&icon_style);
    lv_style_set_text_font(&icon_style, &lv_font_montserrat_32);

    lv_obj_t * home_screen = lv_obj_create(NULL);
    lv_obj_set_size(home_screen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_add_style(home_screen, &home_style, 0);

    static lv_coord_t col_home[] = { 
        LV_GRID_FR(1), 
        LV_GRID_TEMPLATE_LAST 
    };

    static lv_coord_t row_home[] = {
        60,   // status bar
        110,  // gait/state
        260,  // visualization
        110,  // info tiles
        60,   // action bar
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(home_screen, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(home_screen, col_home, row_home);

    /* Status Bar */
    lv_obj_t * status_bar = lv_obj_create(home_screen);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(status_bar, &icon_style, 0);

    lv_obj_set_grid_cell(
        status_bar,
        LV_GRID_ALIGN_STRETCH, 0, 1,    // column
        LV_GRID_ALIGN_STRETCH, 0, 1     // row
    );

    static lv_coord_t cols_status_bar[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1), 
        LV_GRID_FR(1), 
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t rows_status_bar[] = {
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(status_bar, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(status_bar, cols_status_bar, rows_status_bar);

    // Connectivity Status
    conn_status = lv_label_create(status_bar);
    lv_label_set_text(conn_status, LV_SYMBOL_WIFI);

    lv_obj_set_grid_cell(
        conn_status,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    // up time
    up_time = lv_label_create(status_bar);
    lv_label_set_text(up_time, "00:00");
    
    lv_obj_set_grid_cell(
        up_time,
        LV_GRID_ALIGN_CENTER, 1, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    // Battery Status
    lv_obj_t * battery_cont = lv_obj_create(status_bar);
    lv_obj_set_flex_flow(battery_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(battery_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    battery_icon = lv_label_create(battery_cont);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);

    battery_percent = lv_label_create(battery_cont);
    lv_label_set_text(battery_percent, "--%"); // placeholder

    lv_obj_set_grid_cell(
        battery_cont,
        LV_GRID_ALIGN_END, 2, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    /* State Panel */
    lv_obj_t * state_panel = lv_obj_create(home_screen);
    lv_obj_set_grid_cell(
        state_panel,
        LV_GRID_ALIGN_STRETCH, 0, 1,    // column
        LV_GRID_ALIGN_STRETCH, 1, 1     // row
    );

    static lv_coord_t cols_state_panel[] = {
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t rows_state_panel[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(state_panel, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(state_panel, cols_state_panel, rows_state_panel);

    // Gait Label
    lv_obj_t * gait_label = lv_label_create(state_panel);
    lv_label_set_text(gait_label, "Gait: Tripod");
    lv_obj_set_grid_cell(
        gait_label,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    // Mode Label
    lv_obj_t * mode_label = lv_label_create(state_panel);
    lv_label_set_text(mode_label, "Mode: Normal");
    lv_obj_set_grid_cell(
        mode_label,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 1, 1     // row
    );

    // motion state Label
    lv_obj_t * motion_label = lv_label_create(state_panel);
    lv_label_set_text(motion_label, "State: Idle");
    lv_obj_set_grid_cell(
        motion_label,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 2, 1     // row
    );

    /* Visualization Panel */
    lv_obj_t * visualization_panel = lv_obj_create(home_screen);
    lv_obj_clear_flag(visualization_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_cell(
        visualization_panel,
        LV_GRID_ALIGN_STRETCH, 0, 1,    // column
        LV_GRID_ALIGN_STRETCH, 2, 1     // row
    );

    hexapodIcon(visualization_panel);
    
    /* Info Tiles */
    lv_obj_t * info_tiles = lv_obj_create(home_screen);
    lv_obj_set_grid_cell(
        info_tiles,
        LV_GRID_ALIGN_STRETCH, 0, 1,    // column
        LV_GRID_ALIGN_STRETCH, 3, 1     // row
    );

    static lv_coord_t cols_info_tiles[] = {
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t rows_info_tiles[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(info_tiles, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(info_tiles, cols_info_tiles, rows_info_tiles);

    // tilt info
    lv_obj_t * tilt_info = lv_label_create(info_tiles);
    lv_label_set_text(tilt_info, "Tilt: x:0 y:0 z:0");
    lv_obj_set_grid_cell(
        tilt_info,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    // battery info
    lv_obj_t * battery_info = lv_label_create(info_tiles);
    lv_label_set_text_fmt(battery_info, "Battery: %.2fV", 11.1);
    lv_obj_set_grid_cell(
        battery_info,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 1, 1     // row
    );

    /* Action Bar */
    lv_obj_t * action_bar = lv_obj_create(home_screen);   
    lv_obj_clear_flag(action_bar, LV_OBJ_FLAG_SCROLLABLE); 

    lv_obj_set_grid_cell(
        action_bar,
        LV_GRID_ALIGN_STRETCH, 0, 1,    // column
        LV_GRID_ALIGN_STRETCH, 4, 1     // row
    );

    static lv_coord_t cols_action_bar[] = {
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t rows_action_bar[] = {
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(action_bar, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(action_bar, cols_action_bar, rows_action_bar);

    // Menu Button
    lv_obj_t *menu_btn = lv_btn_create(action_bar);
    lv_obj_add_style(menu_btn, &icon_style, 0);
    lv_obj_t *menu_btn_label = lv_label_create(menu_btn);
    lv_obj_align(menu_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(menu_btn_label, "Menu");
    lv_obj_set_grid_cell(
        menu_btn,
        LV_GRID_ALIGN_START, 0, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    lv_obj_add_event_cb(menu_btn, [](lv_event_t * e){ menuPage(); }, LV_EVENT_CLICKED, NULL);

    // Gait Button
    lv_obj_t *gait_btn = lv_btn_create(action_bar);
    lv_obj_add_style(gait_btn, &icon_style, 0);
    lv_obj_t *gait_btn_label = lv_label_create(gait_btn);
    lv_obj_align(gait_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(gait_btn_label, "Gait");
    lv_obj_set_grid_cell(
        gait_btn,
        LV_GRID_ALIGN_CENTER, 1, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    // Mode Button
    lv_obj_t *mode_btn = lv_btn_create(action_bar);
    lv_obj_add_style(mode_btn, &icon_style, 0);
    lv_obj_t *mode_btn_label = lv_label_create(mode_btn);
    lv_obj_align(mode_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(mode_btn_label, "Mode");
    lv_obj_set_grid_cell(
        mode_btn,
        LV_GRID_ALIGN_END, 2, 1,    // column
        LV_GRID_ALIGN_CENTER, 0, 1     // row
    );

    auto_step_timer = lv_timer_create(homeInfo, 100, NULL);

    lv_scr_load(home_screen);
}

void debugPage(lv_obj_t * parent) {
    lv_obj_t * cont;
    lv_obj_t * label;

    debug_page = lv_menu_page_create(parent, "DEBUG");

    // SD Card info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * SD_label = lv_label_create(cont);
    lv_label_set_text(SD_label, "SD Card");

    SD_Size = lv_textarea_create(cont);
    lv_textarea_set_one_line(SD_Size, true);
    lv_textarea_set_placeholder_text(SD_Size, "SD Size");
    lv_obj_add_event_cb(SD_Size, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(SD_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(SD_Size, &lv_font_montserrat_24, 0);

    // Flash info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * Flash_label = lv_label_create(cont);
    lv_label_set_text(Flash_label, "Flash Size");

    FlashSize = lv_textarea_create(cont);
    lv_textarea_set_one_line(FlashSize, true);
    lv_textarea_set_placeholder_text(FlashSize, "Flash Size");
    lv_obj_add_event_cb(FlashSize, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(Flash_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(FlashSize, &lv_font_montserrat_24, 0);

    // Battery voltage info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * BAT_label = lv_label_create(cont);
    lv_label_set_text(BAT_label, "Battery");

    BAT_Volts = lv_textarea_create(cont);
    lv_textarea_set_one_line(BAT_Volts, true);
    lv_textarea_set_placeholder_text(BAT_Volts, "BAT Volts");
    lv_obj_add_event_cb(BAT_Volts, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(BAT_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(BAT_Volts, &lv_font_montserrat_24, 0);

    // Board angle info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * angle_label = lv_label_create(cont);
    lv_label_set_text(angle_label, "IMU Angle");

    Board_angle = lv_textarea_create(cont);
    lv_textarea_set_one_line(Board_angle, true);
    lv_textarea_set_placeholder_text(Board_angle, "Board angle");
    lv_obj_add_event_cb(Board_angle, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(angle_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(Board_angle, &lv_font_montserrat_24, 0);

    // RTC Time info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * Time_label = lv_label_create(cont);
    lv_label_set_text(Time_label, "RTC Time");

    RTC_Time = lv_textarea_create(cont);
    lv_textarea_set_one_line(RTC_Time, true);
    lv_textarea_set_placeholder_text(RTC_Time, "Display time");
    lv_obj_add_event_cb(RTC_Time, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(Time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(RTC_Time, &lv_font_montserrat_24, 0);

    // Wireless scan info
    cont = lv_menu_cont_create(debug_page);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * Wireless_label = lv_label_create(cont);
    lv_label_set_text(Wireless_label, "Wireless");

    Wireless_Scan = lv_textarea_create(cont);
    lv_textarea_set_one_line(Wireless_Scan, true);
    lv_textarea_set_placeholder_text(Wireless_Scan, "Wireless number");
    lv_obj_add_event_cb(Wireless_Scan, ta_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_set_style_text_font(Wireless_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_font(Wireless_Scan, &lv_font_montserrat_24, 0);
}

void menuPage() {
    lv_obj_t * menu_scr = lv_obj_create(NULL); 

    lv_obj_t * menu = lv_menu_create(menu_scr);
    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(menu, back_btn_event_cb, LV_EVENT_CLICKED, menu);
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(menu);
    lv_obj_set_style_text_font(menu, lv_theme_get_font_normal(NULL), 0);

    lv_obj_t * cont;
    lv_obj_t * label;

    // config page
    config_page = lv_menu_page_create(menu, "Config");
    create_dropdown(LEG, LEG_SIZE, config_page, "Leg"); // leg selection
    create_dropdown(JOINT, JOINT_SIZE, config_page, "Joint"); // joint selection

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

    // debug page
    debugPage(menu);
    
    // gait page
    gait_page = lv_menu_page_create(menu, "Gait");
    create_dropdown(GAIT, GAIT_SIZE, gait_page, "Gait Type");

    // mode page
    mode_page = lv_menu_page_create(menu, "Mode");
    create_dropdown(MODE, MODE_SIZE, mode_page, "Mode Type");

    // main menu
    lv_obj_t * main_menu = lv_menu_page_create(menu, "Main Menu");
    for (int i = 0; i < MENU_SIZE; i++) {
        add_menu_item(menu, main_menu, MENU[i]);
    }

    lv_menu_set_page(menu, main_menu);

    auto_step_timer = lv_timer_create(debugInfo, 100, NULL);

    lv_scr_load(menu_scr);
}

void debugInfo(lv_timer_t * timer){
    char buf[100]; 
    
    snprintf(buf, sizeof(buf), "%ld MB\r\n", SDCard_Size);
    lv_textarea_set_placeholder_text(SD_Size, buf);
    snprintf(buf, sizeof(buf), "%ld MB\r\n", Flash_Size);
    lv_textarea_set_placeholder_text(FlashSize, buf);
    snprintf(buf, sizeof(buf), "%.2f V\r\n", BAT_analogVolts);
    lv_textarea_set_placeholder_text(BAT_Volts, buf);
    snprintf(buf, sizeof(buf), "X:%.2f  Y:%.2f  Z:%.2f\r\n", Accel.x, Accel.y, Accel.z);
    lv_textarea_set_placeholder_text(Board_angle, buf);
    snprintf(buf, sizeof(buf), "%d.%d.%d   %d:%d:%d\r\n",datetime.year,datetime.month,datetime.day,datetime.hour,datetime.minute,datetime.second);
    lv_textarea_set_placeholder_text(RTC_Time, buf);
    if(Scan_finish)
        snprintf(buf, sizeof(buf), "WIFI: %d    BLE: %d    ..Scan Finish.\r\n",WIFI_NUM,BLE_NUM);
    else
        snprintf(buf, sizeof(buf), "WIFI: %d    BLE: %d\r\n",WIFI_NUM,BLE_NUM);
    lv_textarea_set_placeholder_text(Wireless_Scan, buf);
}

void homeInfo(lv_timer_t * timer){
    // Add latest voltage to buffer
    bat_voltage_buf[bat_voltage_idx] = BAT_analogVolts;
    bat_voltage_idx = (bat_voltage_idx + 1) % BAT_AVG_BUF_SIZE;
    if (bat_voltage_count < BAT_AVG_BUF_SIZE) bat_voltage_count++;

    float avg_voltage = get_bat_voltage_avg();
    int percent = (int)((avg_voltage - 2.8f) / (4.2f - 2.8f) * 100);
    if(percent > 100) percent = 100;
    if(percent < 0) percent = 0;

    // Update icon
    if(percent >= 90)
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    else if(percent >= 75)
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
    else if(percent >= 50)
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    else if(percent >= 25)
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
    else
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);

    // Update percentage
    lv_label_set_text_fmt(battery_percent, "%d%%", percent);

    // Update connection status color
    if (conn_status != NULL) {
        lv_obj_set_style_text_color(conn_status, (receiverConnected) ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000), 0);
    }

    lv_label_set_text_fmt(up_time, "%02d:%02d", datetime.hour , datetime.minute);
}