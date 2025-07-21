#ifndef CONTROLLER_LOGIC_H
#define CONTROLLER_LOGIC_H
#define STATE_STACK_MAX 10
#include "Pages.h"
#include "Display.h"

// Definition of the stack used for navigation
struct StateStack {
	States state;
	int item_selected;
	StateStack(int s = 0, int i = 0) : state(static_cast<States>(s)), item_selected(i) {}
};

class ControllerLogic {
private:
    StateStack stateStack[STATE_STACK_MAX];
    int stackTop = -1;
    int stackIndex = 0;
    States state;
    bool configStance = false;
public:
    void pushState(int currentState, int selectedItem);
    StateStack popState();
    void backPage();
    void handleScrollAndSelect(page* pages, int itemCount, bool destination = true);
    void mainFSM();
};
extern ControllerLogic controllerLogic;

#endif