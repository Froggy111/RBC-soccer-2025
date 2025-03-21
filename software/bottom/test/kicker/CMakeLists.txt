add_executable(kicker main.cpp)
target_sources(kicker
  PUBLIC
  main.cpp
)

target_link_libraries(kicker hardware_gpio comms)
pico_add_extra_outputs(kicker)
