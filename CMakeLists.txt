# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "Drv" "Hal" "Screens")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_INCLUDE_SIMPLE=1" APPEND)
# idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_PATH= ${CMAKE_SOURCE_DIR}/Drv/inc/lv_conf.h" APPEND)
project(ESP32-S3_Test1)
