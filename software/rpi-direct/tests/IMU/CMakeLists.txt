add_executable(IMU_test main.cpp)
target_link_globals(IMU_test)
target_link_libraries(IMU_test IMU)
