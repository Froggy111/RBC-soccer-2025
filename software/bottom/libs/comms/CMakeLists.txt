add_library(comms)
target_sources(comms
  PRIVATE
    uart.cpp
    usb.cpp
  PUBLIC
    comms.hpp
)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    debug.hpp
)
target_link_libraries(debug_ PRIVATE comms)
link_libraries(debug_)
