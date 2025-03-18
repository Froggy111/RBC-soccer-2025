add_library(WS2812)
target_sources(WS2812
  PRIVATE
    WS2812.cpp
  PUBLIC
    include/WS2812.hpp
)

pico_generate_pio_header(WS2812 ${CMAKE_CURRENT_LIST_DIR}/WS2812.pio)
target_include_directories(WS2812 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(WS2812 hardware-descriptors hardware_pio comms)
target_link_globals(WS2812)