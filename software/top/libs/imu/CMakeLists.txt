add_library(ICM20948)
target_sources(ICM20948
  PRIVATE
    ICM20948.cpp
  PUBLIC
    include/ICM20948.hpp
    include/registers.hpp
)
target_include_directories(ICM20948 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ICM20948 hardware_spi hardware_gpio comms)
target_link_globals(ICM20948)