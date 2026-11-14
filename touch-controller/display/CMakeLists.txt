cmake_minimum_required(VERSION 3.16)

# LVGL is the external/lvgl submodule (git submodule update --init)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../external/lvgl")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(touch-controller)
