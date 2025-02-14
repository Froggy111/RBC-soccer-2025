add_library(hardware-descriptors)
target_sources(hardware-descriptors
  INTERFACE
    include/lib/pinmap.hpp
)
target_include_directories(hardware-descriptors PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
link_libraries(hardware-descriptors) # universally used
