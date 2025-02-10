add_library(PMW3360)
target_sources(PMW3360
  PRIVATE
    PMW3360.cpp
  PUBLIC
    PMW3360.hpp
)
target_link_libraries(PMW3360 PRIVATE hardware_spi)
