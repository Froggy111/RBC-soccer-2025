add_library(CH201)
target_sources(CH201
  PRIVATE
    CH201.cpp
  PUBLIC
    include/CH201.hpp
)

pico_enable_stdio_usb(CH201 1)
pico_enable_stdio_uart(CH201 0)
target_include_directories(CH201 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(CH201 soniclib hardware_i2c MCP23S17)
target_link_globals(CH201)
