add_executable(motion_test main.cpp)

target_link_libraries(motion_test
PUBLIC
    motion-control
)

target_compile_features(motion_test PUBLIC cxx_std_17)
