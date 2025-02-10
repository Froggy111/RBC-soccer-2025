add_library(DRV8244)
target_sources(DRV8244
  PRIVATE
    DRV8244.cpp
  PUBLIC
    DRV8244.hpp
)
target_link_libraries(DRV8244 PRIVATE hardware_spi)
