add_executable(motor_driver_test_dbg main.cpp)
target_sources(motor_driver_test_dbg 
  PUBLIC
    main.cpp
)

target_link_libraries(motor_driver_test_dbg DRV8244 hardware_spi comms)
pico_add_extra_outputs(motor_driver_test_dbg)