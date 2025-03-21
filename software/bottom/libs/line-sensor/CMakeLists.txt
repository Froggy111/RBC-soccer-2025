add_library(ALSPT19)
target_sources(ALSPT19
  PRIVATE
    ALSPT19.cpp
  PUBLIC
    include/ALSPT19.hpp
)

target_include_directories(ALSPT19 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ALSPT19 hardware_adc MCP23S17 comms)
target_link_globals(ALSPT19) 
