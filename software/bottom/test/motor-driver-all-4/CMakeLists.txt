add_executable(motor_driver_test_all_4 main.cpp)
target_sources(motor_driver_test_all_4 
  PUBLIC
    main.cpp
)

target_link_libraries(motor_driver_test_all_4 DRV8244 hardware_spi comms)
pico_add_extra_outputs(motor_driver_test_all_4)