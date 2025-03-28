add_executable(usb_hello_world main.cpp)

target_link_libraries(usb_hello_world
    PUBLIC
    comms
    debug_
)

target_compile_features(usb_hello_world PUBLIC cxx_std_17)
