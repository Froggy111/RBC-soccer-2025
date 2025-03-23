add_executable(processor_test main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(processor_test
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(processor_test PUBLIC cxx_std_17)
