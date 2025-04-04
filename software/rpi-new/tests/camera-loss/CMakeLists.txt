add_executable(camera_loss main.cpp)

find_package(OpenCV REQUIRED)

target_link_libraries(camera_loss
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(camera_loss PUBLIC cxx_std_17)
