add_library(DRV8244)
target_sources(DRV8244
  PRIVATE
    DRV8244.cpp
    registers.cpp
    pin_selector.cpp
  PUBLIC
    include/DRV8244.hpp
    include/pin_selector.hpp
    include/registers.hpp
)

target_include_directories(DRV8244 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(DRV8244 pins hardware_gpio hardware_spi hardware_pwm ADS1115 MCP23S17)
target_link_globals(DRV8244)
