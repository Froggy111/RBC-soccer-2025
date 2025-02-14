add_library(DRV8244)
target_sources(DRV8244
  PRIVATE
    DRV8244.cpp
  PUBLIC
    DRV8244.hpp
)
target_include_directories(DRV8244 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(DRV8244 hardware_spi)
target_link_globals(DRV8244)
