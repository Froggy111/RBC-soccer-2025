include_directories(include)

add_library(comms)
target_sources(comms
  PRIVATE
    comms.cpp
    usb.cpp
    uart.cpp
    usb_descriptors.c
  INTERFACE # this can prob be privated, but idk
    include/comms/usb.hpp
    include/comms/uart.hpp
    include/comms/default_usb_config.h
  PUBLIC
    include/comms.hpp
    include/identifiers.hpp
    include/errors.hpp
)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    include/debug.hpp
)
target_link_libraries(debug_ comms)
link_libraries(debug_)
