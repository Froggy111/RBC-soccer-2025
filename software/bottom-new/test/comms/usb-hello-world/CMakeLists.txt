add_executable(usb_hello_world)
target_sources(usb_hello_world
  PRIVATE
    main.cpp
)
target_link_libraries(usb_hello_world comms debug_)
target_link_globals(usb_hello_world)
pico_add_extra_outputs(usb_hello_world)
