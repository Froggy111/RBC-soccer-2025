add_executable(motor_driver_test_on_board main.cpp)
target_sources(motor_driver_test_on_board 
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(motor_driver_test_on_board 1)
target_link_libraries(motor_driver_test_on_board DRV8244 hardware_spi)
pico_add_extra_outputs(motor_driver_test_on_board)