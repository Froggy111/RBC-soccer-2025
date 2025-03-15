add_library(TSSP4038)
target_sources(TSSP4038
  PRIVATE
    TSSP4038.cpp
  PUBLIC
    include/TSSP4038.hpp
)

pico_enable_stdio_usb(TSSP4038 1)
pico_enable_stdio_uart(TSSP4038 0)
target_include_directories(TSSP4038 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(TSSP4038 hardware/gpio.h hardware-descriptors)
target_link_globals(TSSP4038)