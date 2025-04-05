add_executable(ball-detection-test main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(ball-detection-test
    PUBLIC
    bbw_camera
    ${OpenCV_LIBS}
)

target_compile_features(ball-detection-test PUBLIC cxx_std_17)
