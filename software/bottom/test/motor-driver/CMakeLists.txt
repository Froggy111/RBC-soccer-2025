add_executable(motor_driver_test main.cpp)
target_sources(motor_driver_test 
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(motor_driver_test 1)
target_link_libraries(motor_driver_test DRV8244)
pico_add_extra_outputs(motor_driver_test)