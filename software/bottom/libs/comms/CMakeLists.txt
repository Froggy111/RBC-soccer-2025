include_directories(include)

add_library(comms)
target_sources(comms
  PRIVATE
    comms.cpp
    usb.cpp
    uart.cpp
  INTERFACE # this can prob be privated, but idk
    usb.hpp
    uart.hpp
  PUBLIC
    comms.hpp
    identifiers.hpp
    errors.hpp
)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    debug.hpp
)
target_link_libraries(debug_ comms)
link_libraries(debug_)
