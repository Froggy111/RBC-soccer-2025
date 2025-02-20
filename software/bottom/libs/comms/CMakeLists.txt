include_directories(include/tusb)

add_library(comms)
target_sources(comms
  PRIVATE
    comms.cpp
    usb.cpp
    uart.cpp
    usb_descriptors.c
    usb_callbacks.cpp
  PUBLIC
    include/comms/usb.hpp
    include/comms/uart.hpp
    include/comms/default_usb_config.h
    include/comms.hpp
    include/comms/identifiers.hpp
    include/comms/errors.hpp
)
target_include_directories(comms PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(comms pico_unique_id pico_usb_reset_interface tinyusb_device)
target_link_globals(comms)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    include/debug.hpp
)
target_include_directories(debug_ PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(debug_ comms)
target_link_globals(debug_)
add_global_library(debug_)
