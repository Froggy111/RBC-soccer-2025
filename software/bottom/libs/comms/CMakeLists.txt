add_library(usb)
target_sources(usb
  PRIVATE
    usb.cpp
  PUBLIC
    usb.hpp
)
# if (NOT TARGET pico_usb_reset_interface)
#   message(FATAL_ERROR "pico_usb_reset_interface not found")
# endif()
target_link_libraries(usb PRIVATE tinyusb_device pico_usb_reset_interface)

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
target_link_libraries(comms PRIVATE usb uart)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    debug.hpp
)
target_link_libraries(debug_ PRIVATE comms)
link_libraries(debug_)
