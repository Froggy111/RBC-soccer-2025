add_library(MCP23S17)
target_sources(MCP23S17
  PRIVATE
    MCP23S17.cpp
  PUBLIC
    MCP23S17.hpp
)
target_link_libraries(MCP23S17 hardware_spi)

add_library(pins)
target_sources(pins
  PRIVATE
    pins.cpp
  PUBLIC
    pins.hpp
)
target_link_libraries(pins MCP23S17)
