#include "Display.h"

// Constructor for OLED screen, esp32s2 SPI default is 36(SCK) and 35(MOSI)
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 34, /* dc=*/ 33, /* reset=*/ 21);

Display display;

/*
@brief Calculates the circular delta between two values
@param from The starting value
@param to The target value
@param size The range of values
@return The circular delta
*/
float Display::circularDelta(float from, float to, int size) {
	float delta = fmodf((to - from + size), size);
	if (delta > size / 2.0f) delta -= size;
	return delta;
}

/*
@brief Sets up the navigation bar with a heading
@param heading The heading text to display
@note Draws a "Back" button and the heading on the navigation bar
*/
void Display::setupNav(const char* heading) {
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.drawXBMP(0, 0, 26, 10, epd_bitmap_button_boarder);
	u8g2.drawStr(5, 7, "Back");
	u8g2.drawStr(29, 7, heading);
}

/*
@brief Draws the "Selected" icon on the active item
@param NUM_ITEMS The total number of items
@param activeItem The index of the currently active item
@note The active item is highlighted with a special icon
*/
void Display::drawActiveItem(int NUM_ITEMS, int activeItem) {
	// Number of items
	const int itemCount = NUM_ITEMS;

	// Calculate offset from currently selected item to active item, handling wrap-around
	int delta = activeItem - item_selected;
	if (delta > itemCount / 2) delta -= itemCount;
	if (delta < -itemCount / 2) delta += itemCount;

	// Define Y positions for items relative to selected item at 33
	// If your item_selected is at Y=33, and items are spaced by 18px:
	int yBase = 33;        // Y pos of selected item
	int ySpacing = 18;     // vertical spacing between items
	int yPos = yBase + delta * ySpacing;

	// Only draw if visible within your scrolling window (e.g., y between 15 and 51)
	if (yPos >= 15 && yPos <= 51) {u8g2.drawXBMP(5, yPos, 7, 7, epd_bitmap_selected_icon);}
}

/*
@brief Draws the page items and their icons if available
@param pages The array of page items
@param NUM_ITEMS The total number of items
@param itemIcons Whether to display item icons
@param hexIcon Whether to display the hex icon
*/
void Display::drawPageItems(page* pages, int NUM_ITEMS, bool itemIcons, bool hexIcon) {
	float delta = circularDelta(visualScrollIndex, (float)item_selected, NUM_ITEMS);
	visualScrollIndex += 0.2f * delta;

	// Clamp within [0, MENU_ITEMS)
	if (visualScrollIndex < 0) visualScrollIndex += NUM_ITEMS;
	if (visualScrollIndex >= NUM_ITEMS) visualScrollIndex -= NUM_ITEMS;

	// Wrap scroll index to keep in [0, NUM_ITEMS)
	float wrappedScroll = fmodf(visualScrollIndex + NUM_ITEMS, NUM_ITEMS);
	int centerIndex = (int)wrappedScroll;
	float fractionalOffset = wrappedScroll - (float)centerIndex;

	// Draw the fixed selection border at CENTER_Y
	if (hexIcon) {u8g2.drawXBMP(1, (int)(CENTER_Y - 14), 77, 20, epd_bitmap_selection_boarder_hex);} 
	else {u8g2.drawXBMP(1, (int)(CENTER_Y - 14), 126, 20, epd_bitmap_selection_boarder);}
  
	// Draw visible items
	for (int i = -2; i <= 2; i++) {
		int index = (centerIndex + i + NUM_ITEMS) % NUM_ITEMS;
		float y = CENTER_Y + ySpacing * (i - fractionalOffset);

		if (y < NAVBAR_HEIGHT + 1 || y > SCREEN_HEIGHT - 1) continue;

		if (index == item_selected) {
			// This is the selected item — draw bold
			u8g2.setFont(u8g_font_7x14B);
			u8g2.drawStr(26, (int)roundf(y), pages[index].item);
			if (itemIcons && !hexIcon) {u8g2.drawXBMP(4, (int)roundf(y - 13), 16, 16, pages[index].icon);}
			if (hexIcon) {
				u8g2.drawXBMP(79, 12, 48, 48, epd_bitmap_hex_boarder);
				u8g2.drawXBMP(82, 15, 42, 42, pages[index].icon);
			}
		} else {
			// Non-selected
			u8g2.setFont(u8g_font_7x14);
			u8g2.drawStr(26, (int)roundf(y), pages[index].item);
			if (itemIcons) {u8g2.drawXBMP(4, (int)roundf(y - 13), 16, 16, pages[index].icon);}
		}
	}
}

/*
@brief Initializes the display
*/
void Display::initializeDisplay() {
    // Initialize the display
    u8g2.begin();
    u8g2.setFont(u8g2_font_5x8_mn);
    u8g2.setColorIndex(1);
}