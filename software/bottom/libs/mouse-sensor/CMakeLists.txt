add_library(PMW3360)
target_sources(PMW3360
  PRIVATE
    PMW3360.cpp
  PUBLIC
    include/lib/PMW3360.hpp
)
target_include_directories(PMW3360 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(PMW3360 hardware_spi)
