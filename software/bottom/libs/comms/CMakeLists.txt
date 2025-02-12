add_library(identifiers)
target_sources(identifiers
  INTERFACE
    identifiers.hpp
)

add_library(usb)
target_sources(usb
  PRIVATE
    usb.cpp
  PUBLIC
    usb.hpp
)
target_link_libraries(usb identifiers)

add_library(uart)
target_sources(uart
  PRIVATE
    uart.cpp
  PUBLIC
    uart.hpp
)

add_library(comms)
target_sources(comms
  PRIVATE
    comms.cpp
  PUBLIC
    comms.hpp
)
target_link_libraries(comms usb uart)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    debug.hpp
)
target_link_libraries(debug_ comms)
link_libraries(debug_)
