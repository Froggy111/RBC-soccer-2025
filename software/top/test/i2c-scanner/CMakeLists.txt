add_executable(i2c_scanner_test main.cpp)
target_sources(i2c_scanner_test
  PUBLIC
    main.cpp
)

target_link_globals(i2c_scanner_test)
target_link_libraries(i2c_scanner_test hardware_i2c hardware_gpio MCP23S17 comms)
pico_add_extra_outputs(i2c_scanner_test)