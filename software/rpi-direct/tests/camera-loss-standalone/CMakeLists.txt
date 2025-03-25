add_executable(camera_loss_standalone main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(camera_loss_standalone
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(camera_loss_standalone PUBLIC cxx_std_17)
