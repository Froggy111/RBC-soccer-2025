add_executable(usb_test)
target_sources(usb_test
  PRIVATE
    main.cpp
)
target_link_libraries(usb_test comms)
target_link_globals(usb_test)
pico_add_extra_outputs(usb_test)
