# header-only lib
add_library(types)
target_sources(types
  PUBLIC
    include/types.hpp
    include/delay.hpp
)
target_include_directories(types PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_globals(types)
add_global_library(types)
