add_executable(benchmark main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(benchmark 
    bbw_camera
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_features(benchmark PUBLIC cxx_std_17)