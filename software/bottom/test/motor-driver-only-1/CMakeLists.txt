add_executable(motor_driver_test_only_1 main.cpp)
target_sources(motor_driver_test_only_1 
  PUBLIC
    main.cpp
)

target_link_libraries(motor_driver_test_only_1 DRV8244 hardware_spi hardware_i2c)
pico_add_extra_outputs(motor_driver_test_only_1)