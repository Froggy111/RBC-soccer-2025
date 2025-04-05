add_executable(main main.cpp)
target_sources(main
PRIVATE
  mode_controller.cpp
  sensors/IR.cpp
  sensors/line_sensors.cpp
  PUBLIC
  mode_controller.hpp
  sensors/IR.hpp
  actions/LEDs.hpp
  actions/kicker.hpp
  sensors/line_sensors.hpp
)

find_package(Threads REQUIRED)

target_link_libraries(main
  bbw_camera
  attack
  ${CMAKE_THREAD_LIBS_INIT}
)


target_link_libraries(main bbw_camera motion-control wiringPi motors IMU)
