# hexapod-controller receiver: handheld controller (ESP-NOW) and Bluetooth gamepads (Bluepad32) in,
# Servo 2040 (I2C) and/or robot ESP32 (ESP-NOW relay) out.
cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(BLUEPAD32_ROOT "${CMAKE_CURRENT_LIST_DIR}/../external/bluepad32")
set(BTSTACK_ROOT "${BLUEPAD32_ROOT}/external/btstack")
if(NOT EXISTS "${BTSTACK_ROOT}/src/btstack.h")
    message(FATAL_ERROR "Bluepad32 or BTstack is missing: run `git submodule update --init --recursive`")
endif()

# BTstack becomes an ESP-IDF component by copying parts of its tree together. Bluepad32's
# integrate_btstack.py does this inside the submodule; doing it here keeps the submodule clean.
# The copy goes in the build folder and is redone when the BTstack commit changes.
set(BTSTACK_COMPONENT "${CMAKE_BINARY_DIR}/btstack_component/btstack")
execute_process(COMMAND git -C "${BTSTACK_ROOT}" rev-parse HEAD
                OUTPUT_VARIABLE BTSTACK_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
if(NOT EXISTS "${BTSTACK_COMPONENT}/.commit")
    set(_btstack_stale TRUE)
else()
    file(READ "${BTSTACK_COMPONENT}/.commit" _btstack_done)
    if(NOT _btstack_done STREQUAL BTSTACK_COMMIT)
        set(_btstack_stale TRUE)
    endif()
    if(EXISTS "${BTSTACK_COMPONENT}/port/esp32/components/btstack/CMakeLists.txt"
       AND EXISTS "${BTSTACK_COMPONENT}/port/esp32/components/btstack_config/CMakeLists.txt")
        file(READ "${BTSTACK_ROOT}/port/esp32/components/btstack/CMakeLists.txt"
             _btstack_source_cmake)
        file(READ "${BTSTACK_COMPONENT}/port/esp32/components/btstack/CMakeLists.txt"
             _btstack_copied_cmake)
        if(NOT _btstack_source_cmake STREQUAL _btstack_copied_cmake)
            set(_btstack_stale TRUE)
        endif()
    else()
        set(_btstack_stale TRUE)
    endif()
endif()
if(_btstack_stale)
    message(STATUS "Creating the BTstack component (${BTSTACK_COMMIT})")
    file(REMOVE_RECURSE "${BTSTACK_COMPONENT}")
    file(COPY "${BTSTACK_ROOT}/port/esp32/components/btstack/" DESTINATION "${BTSTACK_COMPONENT}")
    foreach(dir src 3rd-party/bluedroid 3rd-party/hxcmod-player 3rd-party/lwip/dhcp-server
                3rd-party/lc3-google 3rd-party/md5 3rd-party/micro-ecc 3rd-party/qr-code-generator
                3rd-party/yxml chipset/zephyr
                platform/freertos platform/lwip tool)
        get_filename_component(_parent "${dir}" DIRECTORY)
        file(COPY "${BTSTACK_ROOT}/${dir}" DESTINATION "${BTSTACK_COMPONENT}/${_parent}")
    endforeach()
    file(COPY "${BTSTACK_ROOT}/port/esp32/components/btstack_config/"
         DESTINATION "${BTSTACK_COMPONENT}/port/esp32/components/btstack_config")
    foreach(f hal_time_ms.h hal_uart_dma.h hci_dump_embedded_stdout.h hci_dump_embedded_stdout.c)
        file(COPY "${BTSTACK_ROOT}/platform/embedded/${f}" DESTINATION "${BTSTACK_COMPONENT}/platform/embedded")
    endforeach()
    file(COPY "${BTSTACK_ROOT}/port/esp32/components/btstack/"
         DESTINATION "${BTSTACK_COMPONENT}/port/esp32/components/btstack")
    file(WRITE "${BTSTACK_COMPONENT}/.commit" "${BTSTACK_COMMIT}")
endif()

set(EXTRA_COMPONENT_DIRS
    "${BLUEPAD32_ROOT}/src/components"   # bluepad32, and the cmd_nvs/cmd_system it uses
    "${BTSTACK_COMPONENT}/port/esp32/components")

set(ENV{BTSTACK_ROOT} "${BTSTACK_COMPONENT}")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include("${BTSTACK_COMPONENT}/cmake/BTstack_project.cmake")
project(receiver)
