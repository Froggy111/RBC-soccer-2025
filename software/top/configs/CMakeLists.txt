# header-only lib
add_library(config)
target_sources(config
  PUBLIC
    include/config.cpp)
target_include_directories(config PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_globals(config)
add_global_library(config)
