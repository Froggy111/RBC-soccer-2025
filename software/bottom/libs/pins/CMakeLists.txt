add_library(MCP23S17)
target_sources(MCP23S17
  PRIVATE
    MCP23S17.cpp
  PUBLIC
    include/pins/MCP23S17.hpp
)
target_include_directories(MCP23S17 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(MCP23S17 hardware_spi)
target_link_globals(MCP23S17)

add_library(digital_pins)
target_sources(digital_pins
  PRIVATE
    digital_pins.cpp
  PUBLIC
    include/pins/digital_pins.hpp
)
target_include_directories(digital_pins PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(digital_pins MCP23S17)
target_link_globals(digital_pins)

add_library(ADS1115)
target_sources(ADS1115
  PRIVATE
    ADS1115.cpp
  PUBLIC
    include/pins/ADS1115.hpp
)
target_include_directories(ADS1115 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ADS1115 hardware_i2c digital_pins)
target_link_globals(ADS1115)

add_library(analog_pins)
target_sources(analog_pins
  PRIVATE
    analog_pins.cpp
  PUBLIC
    include/pins/analog_pins.hpp
)
target_include_directories(analog_pins PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(analog_pins ADS1115 digital_pins)
target_link_globals(analog_pins)

add_library(pins)
target_sources(pins
  PUBLIC
    include/pins.hpp
)
target_include_directories(pins PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pins digital_pins analog_pins)
target_link_globals(pins)
