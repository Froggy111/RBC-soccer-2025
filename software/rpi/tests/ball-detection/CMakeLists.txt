add_executable(ball-detection-test main.cpp)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(goalpost
PUBLIC
    bbw_camera 
    ball-detection
    ${OpenCV_LIBS}
    IR
)

target_compile_features(ball-detection-test PUBLIC cxx_std_17)
