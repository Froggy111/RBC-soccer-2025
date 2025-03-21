add_executable(led-test main.cpp)
target_sources(led-test
  PUBLIC
    main.cpp
)

target_link_libraries(led-test comms WS2812)
pico_add_extra_outputs(led-test)