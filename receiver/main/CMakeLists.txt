idf_component_register(
    SRCS "main.cpp" "Link.cpp" "EspNowLink.cpp" "I2CLink.cpp" "Gamepad.cpp"
    INCLUDE_DIRS "."
                 # From the gait-core submodule: hexapod_protocol/packets.h (the packets every robot
                 # uses) and hexapod_teleop/gamepad_teleop.hpp (the gamepad layout ROS 2 uses too)
                 "../../external/hexapod-gait-core/include"
    REQUIRES bluepad32 btstack esp_wifi esp_driver_i2c esp_driver_gpio esp_timer nvs_flash hal)
