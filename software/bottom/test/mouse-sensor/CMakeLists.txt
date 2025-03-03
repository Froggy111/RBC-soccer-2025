add_executable(mouse_sensor_test main.cpp)
target_sources(mouse_sensor_test 
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(mouse_sensor_test 1)
target_link_libraries(mouse_sensor_test PMW3360)
pico_add_extra_outputs(mouse_sensor_test)