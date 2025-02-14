# header-only lib
add_library(types)
target_sources(types
  PUBLIC
    include/lib/types.hpp)
target_include_directories(types PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
link_libraries(types) # universally used
