idf_component_register(
    SRCS "main.cpp" "Link.cpp" "EspNowLink.cpp" "I2CLink.cpp" "Gamepad.cpp"
    INCLUDE_DIRS "."
                 # HexapodPackets.h: the packet layout shared with the robot firmware
                 "../../common"
    REQUIRES bluepad32 btstack esp_wifi esp_driver_i2c esp_driver_gpio esp_timer nvs_flash hal)
