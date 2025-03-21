add_executable(main main.cpp)
target_sources(main
  PRIVATE
  actions/LEDs.hpp
  sensors/IRs.hpp
)

target_link_libraries(main WS2812 comms)
pico_add_extra_outputs(main)
