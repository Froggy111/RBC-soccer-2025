add_executable(comms_hello_world main.cpp)

target_link_libraries(comms_hello_world
    PUBLIC
    comms
    debug_
)

target_compile_features(comms_hello_world PUBLIC cxx_std_17)
