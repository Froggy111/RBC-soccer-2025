add_library(ball_detect)
target_sources(ball_detect
PRIVATE
  ball_detect.cpp
PUBLIC
  include/ball_detect.hpp
)

target_include_directories(ball_detect PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include comms types IR)
target_link_globals(ball_detect)
