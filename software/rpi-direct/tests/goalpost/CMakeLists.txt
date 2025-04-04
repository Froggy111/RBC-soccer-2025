add_executable(goalpost main.cpp)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(goalpost
PUBLIC
    bbw_camera 
    ${OpenCV_LIBS}
)

target_compile_features(goalpost PUBLIC cxx_std_17)
