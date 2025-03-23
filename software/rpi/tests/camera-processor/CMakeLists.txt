add_executable(camera_processor main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(camera_processor
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(camera_processor PUBLIC cxx_std_17)
