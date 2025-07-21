#ifndef DISPLAY_H
#define DISPLAY_H
#include <U8g2lib.h>
#include "bitmaps.h"
#include "Pages.h"

// Constructor for OLED screen, esp32s2 SPI default is 36(SCK) and 35(MOSI)
extern U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2;

class Display{
private:
    const int SCREEN_HEIGHT = 64;
    const int NAVBAR_HEIGHT = 10;
    const int ITEM_HEIGHT = 18;
    const float CENTER_Y = 42.0f;
    const float ySpacing = 19.0f;
    const float SCROLL_THRESHOLD = 1.0f; // Change item when this is exceeded
    
public:
    Display() {initializeDisplay();}
    int item_selected;
    int item_previous;
    int item_next;
    float visualScrollIndex; // For smooth scrolling
    float scrollPosition; // Accumulates encoderDelta
    float circularDelta(float from, float to, int size);
    void setupNav(const char* heading);
    void drawActiveItem(int NUM_ITEMS, int activeItem);
    void drawPageItems(page* pages, int NUM_ITEMS, bool itemIcons = false, bool hexIcon = false);
    void initializeDisplay();
};

extern Display display;

#endif