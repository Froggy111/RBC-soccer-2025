add_library(camera)
target_sources(camera
  PRIVATE
    cam.cpp
  PUBLIC
    include/cam.hpp
)
pico_enable_stdio_usb(camera 1)
pico_enable_stdio_uart(camera 0)
target_include_directories(camera PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(camera hardware_spi hardware_pwm MCP23S17)
target_link_globals(camera)
