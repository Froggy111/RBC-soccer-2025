add_executable(usb-led-blink main.cpp)

target_link_libraries(usb-led-blink
    PUBLIC
    comms
    debug_
)

target_compile_features(usb-led-blink PUBLIC cxx_std_17)
