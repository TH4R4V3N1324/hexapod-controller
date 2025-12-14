#ifndef MENU_H
#define MENU_H

#include "LVGL_Driver.h"
#define STATE_STACK_MAX 10

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_HOME,
    STATE_MENU,
    STATE_CONFIG,
    STATE_MODE,
    STATE_GAIT,
    STATE_ANIMATION,
    STATE_LEG,
    STATE_JOINT,
    STATE_NONE
} States;


// Definition of the stack used for navigation
struct StateStack {
    States state;
    int item_selected;
#ifdef __cplusplus
    StateStack(int s = 0, int i = 0) : state(static_cast<States>(s)), item_selected(i) {}
#endif
};

extern struct StateStack stateStack[STATE_STACK_MAX];
extern int stackTop;
extern int stackIndex;

extern States currentState;
extern States previousState;

// Definition for page information
struct page {
    const char* item;
    const char* icon;
    States destination;
};

enum Joints {coxa, femur, tibia};
enum Gaits {GAIT_TRIPOD, GAIT_RIPPLE, GAIT_WAVE, NUM_GAITS};
enum Modes {MODE_NORMAL, MODE_STRAFE, MODE_TILT, MODE_CONFIG, NUM_MODES};


void pushState(int currentState);
struct StateStack popState();
void backPage();
void homePage();
void mainMenu();
void configPage();
void modePage();
void gaitPage();
void animationPage();
void legPage();
void jointPage();

#ifdef __cplusplus
}
#endif

#endif // MENU_H