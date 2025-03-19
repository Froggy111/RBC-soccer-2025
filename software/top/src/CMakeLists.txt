add_executable(main main.cpp)
target_sources(main
  PRIVATE
  actions/LEDs.cpp
  sensors/IMUs.cpp
)

target_link_libraries(main ICM20948 WS2812 hardware_i2c hardware_spi)
pico_add_extra_outputs(main)
