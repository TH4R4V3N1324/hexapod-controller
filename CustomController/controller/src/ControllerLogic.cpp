#include "ControllerLogic.h"

ControllerLogic controllerLogic;

/*
@brief Pushes the current state and cursor position onto the state stack
@param currentState The current state to be pushed onto the stack
@param selectedItem The current cursor position to be pushed onto the stack
*/
void ControllerLogic::pushState(int currentState, int selectedItem) {
  if (stackIndex < STATE_STACK_MAX) {stateStack[stackIndex++] = { currentState, selectedItem };}
}

/*
@brief Pops the state and cursor position from the state stack
@return The state and cursor position from the previous page
*/
StateStack ControllerLogic::popState() {
  if (stackIndex > 0) {return stateStack[--stackIndex];}
  return { 0, 0 };  // Default fallback
}

/*
@brief Navigates back to the previous page by popping the state stack
*/
void ControllerLogic::backPage() {
	StateStack restored = popState();
	display.item_selected = restored.item_selected;
	pages.state = restored.state;
}

/*
@brief Handles the scroll and selection logic for a given page
@param MenuPage The page structure containing menu items and their destinations
@param itemCount The number of items in the menu
@param destination If true, allows navigation to the destination state on selection
@note If destination is false, selection will not change the state
*/
void ControllerLogic::handleScrollAndSelect(page* MenuPage, int itemCount, bool destination) {
	if ((input.button1Z1 != input.button1Z0) && (!input.button1Z0)) {backPage(); return;}

	int delta = input.encoderCount - input.lastEncoderCount;

	if (abs(delta) >= input.encoderCountPerIndent) {
		input.lastEncoderCount = input.encoderCount;
		if (delta > 0) {display.item_selected = (display.item_selected + itemCount - 1) % itemCount;} 
		else {display.item_selected = (display.item_selected + 1) % itemCount;}
	}

	if (!destination) return;

	if ((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {
		pushState(pages.state, display.item_selected);
		pages.state = MenuPage[display.item_selected].destination;
		display.visualScrollIndex = 0.00f;
		display.item_selected = 0; 
	}
}

/*
@brief Main finite state machine for controlling the robot
@note This function handles state transitions and actions based on user input
*/
void ControllerLogic::mainFSM() {
	switch (pages.state) {
		case STATE_HOME:
			if (input.encoderDelta >= input.encoderCountPerIndent) {controlPacket.currentHeight --; input.lastEncoderCount += input.encoderCountPerIndent;}
		if (input.encoderDelta <= -input.encoderCountPerIndent) {controlPacket.currentHeight ++; input.lastEncoderCount -= input.encoderCountPerIndent;}
			if ((input.button2Z1 != input.button2Z0) && (!input.button2Z0)) {pages.activeGait = static_cast<Gaits>((pages.activeGait + 1) % pages.GAIT_ITEMS); controlPacket.command = CMD_SET_GAIT; controlPacket.commandArgs[0] = pages.activeGait;}
			if ((input.button3Z1 != input.button3Z0) && (!input.button3Z0)) {pages.activeMode = static_cast<Modes>((pages.activeMode + 1) % pages.MODE_ITEMS); controlPacket.command = CMD_SET_MODE; controlPacket.commandArgs[0] = pages.activeMode;}
			if ((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {pushState(pages.state, display.item_selected); pages.state = STATE_MENU; input.lastEncoderCount = input.encoderCount;}
			break;
		case STATE_MENU:
			if (configStance) {controlPacket.command = CMD_HOME_STANCE; configStance = false;}
			handleScrollAndSelect(pages.MENU, pages.MENU_ITEMS);
			break;
		case STATE_CONFIG:
			if (!configStance) {controlPacket.command = CMD_SET_MODE ; controlPacket.commandArgs[0] = MODE_CONFIG ; configStance = true;}
			if ((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {pages.leg_selected = display.item_selected;}
			handleScrollAndSelect(pages.CONFIG, pages.CONFIG_ITEMS);
			break;
		case STATE_LEG:
			if ((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {
				pages.joint_selected = display.item_selected; 
				pages.jointOffset = pages.LEG_OFFSET[pages.leg_selected][pages.joint_selected];
			}
			handleScrollAndSelect(pages.LEG, pages.LEG_ITEMS);
			break;
		case STATE_GAIT:
			handleScrollAndSelect(pages.GAIT, pages.GAIT_ITEMS, false);
			if((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {pages.activeGait = static_cast<Gaits>(display.item_selected); controlPacket.command = CMD_SET_GAIT; controlPacket.commandArgs[0] = pages.activeGait;}
			break;
		case STATE_MODE:
			handleScrollAndSelect(pages.MODE, pages.MODE_ITEMS, false);
			if((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {pages.activeMode = static_cast<Modes>(display.item_selected); controlPacket.command = CMD_SET_MODE; controlPacket.commandArgs[0] = pages.activeMode;}
			break;
		case STATE_JOINT:
			if ((input.button1Z1 != input.button1Z0) && (!input.button1Z0)) {backPage();}
			if((input.encoderButtonZ1 != input.encoderButtonZ0) && (!input.encoderButtonZ0)) {
				pages.LEG_OFFSET[pages.leg_selected][pages.joint_selected] = pages.jointOffset;
                controlPacket.command = CMD_SET_CONFIG;
                controlPacket.commandArgs[0] = pages.leg_selected;
                controlPacket.commandArgs[1] = pages.joint_selected;
                controlPacket.commandArgs[2] = pages.jointOffset;
				pages.jointOffset = 0;
				backPage();
			}
			if(input.encoderDelta >= input.encoderCountPerIndent) {pages.jointOffset --; input.lastEncoderCount += input.encoderCountPerIndent;}
		    if(input.encoderDelta <= -input.encoderCountPerIndent) {pages.jointOffset ++; input.lastEncoderCount -= input.encoderCountPerIndent;}
			if (pages.jointOffset > 60) pages.jointOffset = 60;
			if (pages.jointOffset < -60) pages.jointOffset = -60;
	        break;
        case STATE_ANIMATION:
            handleScrollAndSelect(pages.ANIMATION, pages.ANIMATION_ITEMS, false);
            break;
        default:
            break;
	}
}