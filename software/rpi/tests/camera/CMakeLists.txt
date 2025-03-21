find_package(Threads REQUIRED)

add_executable(camera_test main.cpp)

target_link_libraries(camera_test 
    bbw_camera
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_features(camera_test PUBLIC cxx_std_17)