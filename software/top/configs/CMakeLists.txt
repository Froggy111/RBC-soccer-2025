add_library(config)
target_sources(config
  PRIVATE
  config.cpp
  PUBLIC
  include/config.hpp
)
target_include_directories(config PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_globals(config)
add_global_library(config)
