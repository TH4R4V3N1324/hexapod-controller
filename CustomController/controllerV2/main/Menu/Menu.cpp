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

void FSM(){
    switch (currentState) {
        case STATE_HOME:
            homePage();
            break;
        case STATE_MENU:
            mainMenu();
            break;
        case STATE_CONFIG:
            configPage();
            break;
        case STATE_MODE:
            modePage();
            break;
        case STATE_GAIT:
            gaitPage();
            break;
        case STATE_ANIMATION:
            animationPage();
            break;
        case STATE_LEG:
            legPage();
            break;
        case STATE_JOINT:
            jointPage();
            break;
        default:
            break;
    }    
}

static void menu_btn_event_cb(lv_event_t *e) {
    previousState = currentState;
    currentState = STATE_MENU;
    mainMenu(); // Show your menu
}

static void back_btn_event_cb(lv_event_t *e) {
    StateStack restored = popState();
	currentState = restored.state;
    FSM();
}

static void menu_item_event_cb(lv_event_t *e) {
    States dest = (States)(intptr_t)lv_event_get_user_data(e);
    if(dest == STATE_NONE) return; // No state change for NONE destinations
    previousState = currentState;
    currentState = dest;
    pushState(previousState);
    FSM();
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

void mainMenu() {
   lv_obj_t * menu_screen = lv_obj_create(NULL);
   lv_obj_set_style_bg_color(menu_screen, lv_color_black(), 0);

   lv_obj_t * back_btn = lv_btn_create(menu_screen);
   lv_obj_set_size(back_btn, 90, 40);
   lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
   lv_obj_t * back_label = lv_label_create(back_btn);
   lv_label_set_text(back_label, "Back");
   lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);

   lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)previousState);

   // Menu title
   lv_obj_t * menu_label = lv_label_create(menu_screen);
   lv_label_set_text(menu_label, "Menu");
   lv_obj_set_style_text_color(menu_label, lv_color_white(), 0);
   lv_obj_set_style_text_font(menu_label, &lv_font_montserrat_24, 0);
   lv_obj_align(menu_label, LV_ALIGN_TOP_MID, 0, 10);

   // Menu items
   for (int i = 0; i < MENU_SIZE; i++) {
       // Icon
       lv_obj_t * icon = lv_label_create(menu_screen);
       lv_label_set_text(icon, MENU[i].icon);
       lv_obj_set_style_text_color(icon, lv_color_white(), 0);
       lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
       lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 30, 80 + i * 80);

       // Button
       lv_obj_t * btn = lv_btn_create(menu_screen);
       lv_obj_set_size(btn, 300, 60);
       lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 100, 70 + i * 80);
       lv_obj_t * btn_label = lv_label_create(btn);
       lv_label_set_text(btn_label, MENU[i].item);
       lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_24, 0);

       lv_obj_add_event_cb(btn, menu_item_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)MENU[i].destination);
   }

   lv_scr_load(menu_screen); 
}

void configPage() {
   lv_obj_t * config_screen = lv_obj_create(NULL);
   lv_obj_set_style_bg_color(config_screen, lv_color_black(), 0);

   lv_obj_t * back_btn = lv_btn_create(config_screen);
   lv_obj_set_size(back_btn, 90, 40);
   lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
   lv_obj_t * back_label = lv_label_create(back_btn);
   lv_label_set_text(back_label, "Back");
   lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);

   lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)previousState);

   // Config title
   lv_obj_t * config_label = lv_label_create(config_screen);
   lv_label_set_text(config_label, "Config");
   lv_obj_set_style_text_color(config_label, lv_color_white(), 0);
   lv_obj_set_style_text_font(config_label, &lv_font_montserrat_24, 0);
   lv_obj_align(config_label, LV_ALIGN_TOP_MID, 0, 10);

   // Config items
   for (int i = 0; i < CONFIG_SIZE; i++) {
       // Icon
       lv_obj_t * icon = lv_label_create(config_screen);
       lv_label_set_text(icon, CONFIG[i].icon);
       lv_obj_set_style_text_color(icon, lv_color_white(), 0);
       lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
       lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 30, 80 + i * 80);

       // Button
       lv_obj_t * btn = lv_btn_create(config_screen);
       lv_obj_set_size(btn, 300, 60);
       lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 100, 70 + i * 80);
       lv_obj_t * btn_label = lv_label_create(btn);
       lv_label_set_text(btn_label, CONFIG[i].item);
       lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_24, 0);

       // You can add event callbacks for config items here
   }

   lv_scr_load(config_screen); 
}

void modePage() {
    // Implement mode page similarly to configPage()
}

void gaitPage() {
    // Implement gait page similarly to configPage()
}

void animationPage() {
    // Implement animation page similarly to configPage()
}

void legPage() {
    // Implement leg page similarly to configPage()
}

void jointPage() {
    // Implement joint page similarly to configPage()
}

void pushState(int currentState) {
  if (stackIndex < STATE_STACK_MAX) {stateStack[stackIndex++] = { currentState};}
}

StateStack popState() {
  if (stackIndex > 0) {return stateStack[--stackIndex];}
  return { 0, 0 };  // Default fallback
}

void backPage() {
	StateStack restored = popState();
	currentState = restored.state;
}
