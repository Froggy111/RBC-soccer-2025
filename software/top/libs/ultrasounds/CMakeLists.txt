add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../external/ultrasonic.soniclib/ ${CMAKE_BINARY_DIR}/ultrasonic.soniclib)

add_library(CH201)
target_sources(CH201
  PRIVATE
    CH201.cpp
    chbsp.cpp
  PUBLIC
    include/CH201.hpp
    include/chbsp.hpp
)

target_include_directories(CH201 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(CH201 soniclib MCP23S17 hardware_i2c)
target_link_globals(CH201)
