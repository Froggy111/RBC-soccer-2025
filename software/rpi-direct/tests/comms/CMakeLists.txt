add_executable(comms_test main.cpp)

target_link_libraries(comms_test
    PUBLIC
    comms
    debug_
)

target_compile_features(comms_test PUBLIC cxx_std_17)
