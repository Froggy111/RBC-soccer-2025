add_executable(camera_processor_standalone main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(camera_processor_standalone
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(camera_processor_standalone PUBLIC cxx_std_17)
