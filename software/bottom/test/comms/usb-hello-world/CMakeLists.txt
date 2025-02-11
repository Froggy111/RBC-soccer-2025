add_executable(usb_test)
target_sources(usb_test
  PRIVATE
    main.cpp
)
target_link_libraries(usb_test usb)
# pico_enable_stdio_usb(usb_test 1)
# pico_enable_stdio_uart(usb_test 0)
pico_add_extra_outputs(usb_test)
