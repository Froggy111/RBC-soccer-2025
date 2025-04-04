add_executable(usb_led_blink main.cpp)

target_link_libraries(usb_led_blink
    PUBLIC
    comms
    debug_
)

target_compile_features(usb_led_blink PUBLIC cxx_std_17)
