add_executable(ultrasounds_test main.cpp)
target_sources(ultrasounds_test
  PUBLIC
    main.cpp
)

target_link_globals(ultrasounds_test)
target_link_libraries(ultrasounds_test CH201 hardware_i2c hardware_spi)
pico_add_extra_outputs(ultrasounds_test)