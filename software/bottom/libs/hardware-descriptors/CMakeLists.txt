include_directories(include)
add_library(hardware-descriptors)
target_sources(hardware-descriptors
  INTERFACE
    include/lib/pinmap.hpp
)
link_libraries(hardware-descriptors) # universally used
