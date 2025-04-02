add_library(IMU)
target_sources(IMU
PRIVATE
  IMU.cpp
PUBLIC
  include/IMU.hpp
)

target_include_directories(IMU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_globals(IMU comms types)
