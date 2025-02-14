include_directories(include)
add_library(PMW3360)
target_sources(PMW3360
  PRIVATE
    PMW3360.cpp
  PUBLIC
    include/lib/PMW3360.hpp
)
target_link_libraries(PMW3360 hardware_spi)
