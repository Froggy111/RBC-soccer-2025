add_executable(ultrasounds main.cpp)
target_sources(ultrasounds
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(ultrasounds 1)
target_link_libraries(ultrasounds CH201 hardware_i2c hardware_spi)
pico_add_extra_outputs(ultrasounds)