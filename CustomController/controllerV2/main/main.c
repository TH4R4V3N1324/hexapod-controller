#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "GT911.h"
#include "SD_MMC.h"
#include "LVGL_Example.h"
#include "Wireless.h"
#include "BAT_Driver.h"
#include "normal_mode_icon.c"
#include "Menu.h"

void Driver_Loop(void *parameter)
{
    while(1)
    {
        QMI8658_Loop();
        RTC_Loop();
        BAT_Get_Volts();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}
void Driver_Init(void)
{
    Flash_Searching();
    BAT_Init();
    I2C_Init();
    PCF85063_Init();
    QMI8658_Init();
    EXIO_Init();                    // Example Initialize EXIO
    xTaskCreatePinnedToCore(
        Driver_Loop, 
        "Other Driver task",
        4096, 
        NULL, 
        3, 
        NULL, 
        0);
}

void show_bitmap_example(lv_obj_t *parent)
{
    lv_obj_t *img = lv_img_create(parent);
    lv_img_set_src(img, &normal_mode_icon);
    lv_obj_center(img);
}

void test(){
    // Create a new screen
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_scr_load(scr);

    // "Back" button
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 90, 40);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_16, 0);

    // "Menu" label
    lv_obj_t *menu_label = lv_label_create(scr);
    lv_label_set_text(menu_label, "Menu");
    lv_obj_set_style_text_color(menu_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(menu_label, &lv_font_montserrat_16, 0);
    lv_obj_align(menu_label, LV_ALIGN_TOP_LEFT, 120, 20);

    // Menu items
    const char *items[] = {"Config", "Gait", "Mode"};
    lv_obj_t *icons[3]; // Replace with your icon objects
    lv_obj_t *labels[3];
    for (int i = 0; i < 3; i++) {
        // Icon (replace with your own icon drawing)
        icons[i] = lv_label_create(scr);
        lv_label_set_text(icons[i], LV_SYMBOL_SETTINGS); // Example icon
        lv_obj_set_style_text_color(icons[i], lv_color_white(), 0);
        lv_obj_align(icons[i], LV_ALIGN_TOP_LEFT, 30, 80 + i * 80);
        
        // Label
        labels[i] = lv_label_create(scr);
        lv_label_set_text(labels[i], items[i]);
        lv_obj_set_style_text_color(labels[i], lv_color_white(), 0);
        lv_obj_set_style_text_font(labels[i], &lv_font_montserrat_40, 0);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, 120, 80 + i * 80);
    }

    // Highlight "Gait"
    lv_obj_t *highlight = lv_obj_create(scr);
    lv_obj_set_size(highlight, 420, 90);
    lv_obj_set_style_bg_opa(highlight, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(highlight, lv_color_white(), 0);
    lv_obj_set_style_border_width(highlight, 5, 0);
    lv_obj_align(highlight, LV_ALIGN_TOP_LEFT, 10, 60 + 1 * 80); // 1 = "Gait"
    lv_obj_move_background(highlight); // Put highlight behind text/icons
}

void app_main(void)
{   
    Wireless_Init();
    Driver_Init();
    LCD_Init();
    Touch_Init();
    SD_Init();
    LVGL_Init();
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
    homePage();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
