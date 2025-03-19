add_executable(main main.cpp)
target_sources(main
  PUBLIC
  actions/LEDs.cpp
  sensors/IMUs.cpp
  main.cpp
)

target_link_libraries(main ICM20948 WS2812)
pico_add_extra_outputs(main)
