include_directories(include)
# header-only lib
add_library(types)
target_sources(types
  INTERFACE
    include/lib/types.hpp)
link_libraries(types) # universally used
