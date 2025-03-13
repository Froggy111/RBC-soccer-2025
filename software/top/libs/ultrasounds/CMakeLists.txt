# Define paths to SonicLib sources
set(SONICLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/ultrasonic.soniclib)
set(SRC_PATH ${SONICLIB_ROOT}/invn/soniclib)
set(SENS_FW_PATH ${SONICLIB_ROOT}/invn/soniclib/sensor_fw)
set(ICU_INTERFACE_PATH ${SONICLIB_ROOT}/invn/icu_interface)

# Configure sources
set(SONICLIB_SOURCES
    ${SRC_PATH}/ch_api.c
    ${SRC_PATH}/ch_common.c
    ${SRC_PATH}/ch_driver.c
    ${SRC_PATH}/ch_math_utils.c
    ${SRC_PATH}/ch_log.c
    ${SRC_PATH}/extra/ch_extra_display_utils.c
    ${SRC_PATH}/ch_rangefinder.c
)

set(SONICLIB_SOURCES ${SONICLIB_SOURCES}
    ${SENS_FW_PATH}/ch201/ch201_gprmt.c
    ${SENS_FW_PATH}/ch201/ch201_gprmt_fw.c
)

# Create a unified library with both SonicLib and CH201 sources
add_library(CH201)
target_sources(CH201
  PRIVATE
    CH201.cpp
    chbsp.cpp
    pin_selector.cpp
    ${SONICLIB_SOURCES}
  PUBLIC
    include/CH201.hpp
    include/chbsp.hpp
    include/pin_selector.hpp
)

# Include directories for your code and SonicLib
target_include_directories(CH201 PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${SONICLIB_ROOT}
)

target_link_libraries(CH201 MCP23S17 hardware_i2c)

# TODO: REMOVE THE TOP CMAKELISTS ONE

target_compile_definitions(CH201
    PUBLIC -DINVN_SONICLIB_INTERNAL_BOARD_CONFIG
           -DCHIRP_MAX_NUM_SENSORS=16
           -DCHIRP_NUM_BUSES=1
           -DCH_LOG_MODULE_LEVEL=2
           -DINCLUDE_WHITNEY_SUPPORT
)

target_link_globals(CH201)