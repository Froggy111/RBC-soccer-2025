add_executable(led-test main.cpp)
target_sources(led-test
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(led-test 1)
target_link_libraries(led-test comms)
pico_add_extra_outputs(led-test)