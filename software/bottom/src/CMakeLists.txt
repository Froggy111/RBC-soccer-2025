add_executable(main main.cpp)
target_sources(main 
  PUBLIC
    main.cpp
)

target_link_libraries(main DRV8244 ALSPT19 MCP23S17 PMW3360 comms hardware_spi hardware_gpio)
pico_add_extra_outputs(main)