add_executable(main main.cpp)
target_sources(main
  PUBLIC
  line_sensor.cpp
  motors.cpp
  mouse_sensor.cpp
  main.cpp
)

target_link_libraries(main DRV8244 ALSPT19 MCP23S17 PMW3360 comms hardware_spi hardware_i2c hardware_gpio)
pico_add_extra_outputs(main)
