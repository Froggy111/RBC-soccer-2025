add_library(comms)
target_sources(comms
  PRIVATE
    uart.cpp
    usb.cpp
  PUBLIC
    comms.hpp
)

add_library(debug)
target_sources(debug
  PRIVATE
    debug.cpp
  PUBLIC
    debug.hpp
)
target_link_libraries(debug comms)
