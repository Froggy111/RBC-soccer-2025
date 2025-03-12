add_library(CH201)
target_sources(CH201
  PRIVATE
    CH201.cpp
  PUBLIC
    include/CH201.hpp
)

target_include_directories(CH201 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(CH201 soniclib hardware_i2c MCP23S17)
target_link_globals(CH201)
