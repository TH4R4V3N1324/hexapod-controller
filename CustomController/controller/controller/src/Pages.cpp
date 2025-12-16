#include "Pages.h"
#include "Display.h"

Pages pages;

Pages::Pages():
    MENU{
        {"Config", epd_bitmap_cog_icon, STATE_CONFIG},
        {"Mode", epd_bitmap_controller_icon, STATE_MODE},
        {"Gait", epd_bitmap_paw_icon, STATE_GAIT},
        {"Animation", epd_bitmap_film_icon, STATE_ANIMATION}
    },
    CONFIG{
        {"Leg1", epd_bitmap_leg1_icon, STATE_LEG},
        {"Leg2", epd_bitmap_leg2_icon, STATE_LEG},
        {"Leg3", epd_bitmap_leg3_icon, STATE_LEG},
        {"Leg4", epd_bitmap_leg4_icon, STATE_LEG},
        {"Leg5", epd_bitmap_leg5_icon, STATE_LEG},
        {"Leg6", epd_bitmap_leg6_icon, STATE_LEG}
    },
    LEG{
        {"Coxa", epd_bitmap_leg_icon, STATE_JOINT},
        {"Femur", epd_bitmap_leg_icon, STATE_JOINT},
        {"Tibia", epd_bitmap_leg_icon, STATE_JOINT}
    },
    GAIT{
        {"Tripod", nullptr, STATE_NONE},
        {"Ripple", nullptr, STATE_NONE},
        {"Wave", nullptr, STATE_NONE}
    },
    MODE{
        {"Normal", epd_bitmap_normal_mode_icon, STATE_NONE},
        {"Strafe", epd_bitmap_strafe_mode_icon, STATE_NONE},
        {"Tilt", epd_bitmap_tilt_mode_icon, STATE_NONE}
    },
    ANIMATION{
        {"Animation 1", nullptr, STATE_NONE},
        {"Animation 2", nullptr, STATE_NONE},
        {"Animation 3", nullptr, STATE_NONE},
        {"Animation 4", nullptr, STATE_NONE},
        {"Animation 5", nullptr, STATE_NONE}
    }{}

/*
@brief Renders the home page with switch states, gait, mode, phase, height, and menu button
@note Uses icons to represent switch states and mode
*/
void Pages::homePage() {
	// Switch 1
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(2, 1, 5, 7, (digitalRead(switch1) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
	u8g2.drawStr(9, 7, "SW1");

	// Switch 2
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(35, 1, 5, 7, (digitalRead(switch2) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
	u8g2.drawStr(42, 7, "SW2");

	// Switch 3
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(67, 1, 5, 7, (digitalRead(switch3) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
	u8g2.drawStr(74, 7, "SW3");

	// Switch 4
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(98, 1, 5, 7, (digitalRead(switch4) ? epd_bitmap_switch_up_icon : epd_bitmap_switch_down_icon));
	u8g2.drawStr(105, 7, "SW4");

	// Gait Button
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(1, 11, 26, 10, epd_bitmap_button_boarder);
	u8g2.drawStr(5, 18, "Gait");
	u8g2.drawStr(36, 18, GAIT[static_cast<int>(activeGait)].item);

	// Mode Button
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(1, 22, 26, 10, epd_bitmap_button_boarder);
	u8g2.drawStr(5, 29, "Mode");
	u8g2.drawStr(36, 29, MODE[static_cast<int>(activeMode)].item);

	// Phase
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawStr(3, 40, "Phase");
	char currentPhaseStr[10];
	sprintf(currentPhaseStr, "%d", hexPacket.currentPhase);
	u8g2.drawStr(36, 40, currentPhaseStr);
	
	// Height
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawStr(3, 49, "Height");
	char currentHeightStr[10];
	sprintf(currentHeightStr, "%d", controlPacket.currentHeight);
	u8g2.drawStr(36, 49, currentHeightStr);

	// Menu Button
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(1, 53, 26, 10, epd_bitmap_button_boarder);
	u8g2.drawStr(5, 60, "Menu");

	// Hexapod
	u8g2.drawXBMP(72, 12, 48, 48, epd_bitmap_hex_boarder);
	u8g2.drawXBMP(75, 15, 42, 42, MODE[activeMode].icon);
}

/*
@brief Renders the menu page with available menu items
@note Displays item icons
*/
void Pages::menuPage() {
	display.setupNav("Menu");
	display.drawPageItems(MENU, MENU_ITEMS, true);
}

/*
@brief Renders the configuration page with available configuration items
@note Displays leg items with hex icons
*/
void Pages::configPage() {
	display.setupNav("Menu>Config");
	display.drawPageItems(CONFIG, CONFIG_ITEMS, false, true);
}

/*
@brief Renders the leg configuration page with available leg items
@note Displays current offset for selected joint and leg
*/
void Pages::legPage() {
	char navHeading[64];
	snprintf(navHeading, sizeof(navHeading), "Menu>Config>%s", CONFIG[leg_selected].item);
	display.setupNav(navHeading);
	display.drawPageItems(LEG, LEG_ITEMS, false, true);

	u8g2.setFont(u8g_font_7x14);
	char currentOffsetStr[3];
	sprintf(currentOffsetStr, "%d", abs(LEG_OFFSET[leg_selected][display.item_selected]));
	u8g2.drawStr(94, 56, (LEG_OFFSET[leg_selected][display.item_selected] < 0) ? "-" : "+");
	u8g2.drawStr(102, 56, currentOffsetStr);

    if (display.item_selected == coxa) u8g2.drawXBMP(86, 37, 3, 3, epd_bitmap_joint_selected_icon);
    if (display.item_selected == femur) u8g2.drawXBMP(97, 37, 3, 3, epd_bitmap_joint_selected_icon);
    if (display.item_selected == tibia) u8g2.drawXBMP(108, 26, 3, 3, epd_bitmap_joint_selected_icon);
}

/*
@brief Renders the gait configuration page with available gait items
@note Highlights the currently active gait
*/
void Pages::gaitPage() {
	display.setupNav("Menu>Gait");
	display.drawPageItems(GAIT, GAIT_ITEMS, false, false);
	display.drawActiveItem(GAIT_ITEMS, activeGait);
}

/*
@brief Renders the mode configuration page with available mode items
@note Highlights the currently active mode
*/
void Pages::modePage() {
	display.setupNav("Menu>Mode");
	display.drawPageItems(MODE, MODE_ITEMS, false, true);
	display.drawActiveItem(MODE_ITEMS, activeMode);
}

/*
@brief Renders the joint configuration page with available joint items
@note Displays current offset for selected joint and leg
*/
void Pages::jointPage() {
	char navHeading[64];  // Make sure buffer is big enough
	snprintf(navHeading, sizeof(navHeading), "Menu>Config>%s>%s", CONFIG[leg_selected].item, LEG[joint_selected].item);
	display.setupNav(navHeading);

	u8g2.drawXBMP(1, 22, 126, 20, epd_bitmap_selection_boarder);
	u8g2.drawBox(64 + ((jointOffset < 0) ? jointOffset : 0), 24, abs(jointOffset), 15);

	u8g2.setFont(u8g_font_7x14);
	char jointOffsetStr[4];
	sprintf(jointOffsetStr, "%d", abs(jointOffset));
	u8g2.drawStr(53, 56, (jointOffset < 0) ? "-" : "+");
	u8g2.drawStr(61, 56, jointOffsetStr);

	u8g2.setFont(u8g2_font_4x6_mf);
    u8g2.drawXBMP(0, 53, 26, 10, epd_bitmap_button_boarder);
    u8g2.drawStr(5, 60, "Save");
}

/*
@brief Renders the animation configuration page with available animation items
@note Currently does not display icons
*/
void Pages::animationPage() {
	display.setupNav("Menu>Animation");
	display.drawPageItems(ANIMATION, ANIMATION_ITEMS, false, false);
}

/*
@brief Main display function to render the current page based on state
@note Calls specific page rendering functions based on the current state
*/
void Pages::displayPages() {
    u8g2.firstPage();
    do {
		if (state == STATE_HOME) homePage();
		if (state == STATE_MENU) menuPage();
		if (state == STATE_CONFIG) configPage();
		if (state == STATE_LEG) legPage();
		if (state == STATE_GAIT) gaitPage();
		if (state == STATE_MODE) modePage();
		if (state == STATE_ANIMATION) animationPage();
		if (state == STATE_JOINT) jointPage();
    } while ( u8g2.nextPage() );
}