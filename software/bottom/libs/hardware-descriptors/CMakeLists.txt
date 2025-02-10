add_library(hardware-descriptors)
target_sources(hardware-descriptors
  INTERFACE
    pinmap.hpp
)
link_libraries(hardware-descriptors) # universally used
