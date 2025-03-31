add_library(PMW3360)
target_sources(PMW3360
  PRIVATE
    PMW3360.cpp
    dbg_pins.cpp
    pin_selector.cpp
    include/pin_selector.hpp
    include/dbg_pins.hpp
    include/srom_firmware.hpp
    
  PUBLIC
    include/PMW3360.hpp
)
target_include_directories(PMW3360 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(PMW3360 hardware_spi hardware_pwm hardware_timer comms)
target_link_globals(PMW3360)
