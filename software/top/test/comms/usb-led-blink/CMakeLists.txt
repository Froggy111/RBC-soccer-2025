add_executable(usb_led_blink)
target_sources(usb_led_blink
  PRIVATE
    main.cpp
)
target_link_libraries(usb_led_blink comms)
target_link_globals(usb_led_blink)
pico_add_extra_outputs(usb_led_blink)
