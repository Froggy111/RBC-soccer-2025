include_directories(include)
add_library(MCP23S17)
target_sources(MCP23S17
  PRIVATE
    MCP23S17.cpp
  PUBLIC
    include/MCP23S17.hpp
)
target_include_directories(MCP23S17 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(MCP23S17 hardware_spi)
target_link_globals(MCP23S17)

add_library(pins)
target_sources(pins
  PRIVATE
    pins.cpp
  PUBLIC
    include/pins.hpp
)
target_include_directories(pins PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pins MCP23S17)
target_link_globals(pins)
