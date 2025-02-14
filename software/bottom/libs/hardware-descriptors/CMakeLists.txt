add_library(hardware-descriptors)
target_sources(hardware-descriptors
  INTERFACE
    include/pinmap.hpp
)
target_include_directories(hardware-descriptors PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_globals(hardware-descriptors)
add_global_library(hardware-descriptors)
