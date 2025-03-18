add_library(DRV8244)
target_sources(DRV8244
  PRIVATE
    DRV8244.cpp
    pin_manager.cpp
    faults.cpp
    status.cpp
    pin_selector.cpp
    include/pin_selector.hpp
    include/pin_manager.hpp
    include/faults.hpp
    include/status.hpp
  PUBLIC
    include/DRV8244.hpp
)

target_include_directories(DRV8244 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(DRV8244 hardware_gpio hardware_spi hardware_pwm MCP23S17)
target_link_globals(DRV8244)
