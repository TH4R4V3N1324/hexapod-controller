#ifndef PAGES_H
#define PAGES_H

#include "bitmaps.h"
#include "Input.h"

// Main FSM states, matches availble pages
enum States {
	STATE_NONE,
	STATE_HOME,
	STATE_MENU,
	STATE_CONFIG,
	STATE_GAIT,
	STATE_MODE,
	STATE_ANIMATION,
	STATE_LEG,
	STATE_JOINT
};

// Definition for page information
struct page {
	const char* item;
	const unsigned char* icon;
	States destination;
};

enum Joints {coxa, femur, tibia};
enum Gaits {GAIT_TRIPOD, GAIT_RIPPLE, GAIT_WAVE, NUM_GAITS};
enum Modes {MODE_NORMAL, MODE_STRAFE, MODE_TILT, MODE_CONFIG, NUM_MODES};

class Pages {
private:
    int currentPhase = 0;

public:
    Pages();
    States state;
    int leg_selected = 0;
    int joint_selected = 0;
    int jointOffset = 0;
    int16_t LEG_OFFSET[6][3] = {0};
    Gaits activeGait = GAIT_TRIPOD;
    Modes activeMode = MODE_NORMAL;
    static const int MENU_ITEMS = 4;
    static const int CONFIG_ITEMS = 6;
    static const int LEG_ITEMS = 3;
    static const int GAIT_ITEMS = 3;
    static const int MODE_ITEMS = 3;
    static const int ANIMATION_ITEMS = 5;
    page MENU[MENU_ITEMS];
    page CONFIG[CONFIG_ITEMS];
    page LEG[LEG_ITEMS];
    page GAIT[GAIT_ITEMS];
    page MODE[MODE_ITEMS];
    page ANIMATION[ANIMATION_ITEMS];
    void homePage();
    void menuPage();
    void configPage();
    void legPage();
    void gaitPage();
    void modePage();
    void animationPage();
    void jointPage();
    void displayPages();
};
extern Pages pages;

#endif