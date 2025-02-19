add_library(DRV8244)
target_sources(DRV8244
  PRIVATE
    DRV8244.cpp
    dbg_pins.cpp
    faults.cpp
    status.cpp
    pin_selector.cpp
  PUBLIC
    include/DRV8244.hpp
    include/dbg_pins.hpp
    include/faults.hpp
    include/status.hpp
    include/pin_selector.hpp
)
pico_enable_stdio_usb(DRV8244 1)
pico_enable_stdio_uart(DRV8244 0)
target_include_directories(DRV8244 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(DRV8244 hardware_spi hardware_pwm)
target_link_globals(DRV8244)
