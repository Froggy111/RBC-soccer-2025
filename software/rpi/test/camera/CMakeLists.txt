add_executable(camera_test
    main.cpp
)
target_sources(camera_test 
  PUBLIC
    main.cpp
)

# Enable USB output for debugging
pico_enable_stdio_usb(camera_test 1)
pico_enable_stdio_uart(camera_test 0)

# Link against the camera library
target_link_libraries(camera_test camera)

# Create map/bin/hex file etc.
pico_add_extra_outputs(camera_test)