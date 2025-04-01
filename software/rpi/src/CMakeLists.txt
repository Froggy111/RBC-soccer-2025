add_executable(main main.cpp)
target_sources(main
  PUBLIC
  mode_controller.cpp
  mode_controller.hpp
  sensors/IR.cpp
  sensors/IR.hpp
)

find_package(Threads REQUIRED)

target_link_libraries(main
  bbw_camera
  ${CMAKE_THREAD_LIBS_INIT}
)


target_link_libraries(main bbw_camera motion-control wiringPi)
