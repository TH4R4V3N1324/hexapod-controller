#ifndef PAGES_H
#define PAGES_H

#include "bitmaps.h"
#include "Input.h"
#include "HexapodPackets.h"

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
// Gait and Mode come from common/HexapodPackets.h, so the values match the robot's

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
    Gait activeGait = GAIT_TRIPOD;
    Mode activeMode = MODE_NORMAL;
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