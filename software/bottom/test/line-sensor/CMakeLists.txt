add_executable(line-sensor main.cpp)
target_sources(line-sensor
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(line-sensor 1)
target_link_libraries(line-sensor ALSPT19 hardware_spi)
pico_add_extra_outputs(line-sensor)