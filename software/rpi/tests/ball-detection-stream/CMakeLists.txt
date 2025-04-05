add_executable(ball-detection-stream main.cpp)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(ball-detection-stream
    PUBLIC
    bbw_camera
    ${OpenCV_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_features(ball-detection-stream PUBLIC cxx_std_17)
