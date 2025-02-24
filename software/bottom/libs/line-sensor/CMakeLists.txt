add_library(ALSPT19)
target_sources(ALSPT19
  PRIVATE
    ALSPT19.cpp
  PUBLIC
    include/ALSPT19.hpp
)

pico_enable_stdio_usb(ALSPT19 1)
pico_enable_stdio_uart(ALSPT19 0)
target_include_directories(ALSPT19 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ALSPT19 hardware_adc)
target_link_globals(ALSPT19) 
