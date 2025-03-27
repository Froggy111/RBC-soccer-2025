add_executable(comms_test main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(comms_test
    PUBLIC
    comms
    debug_
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_features(comms_test PUBLIC cxx_std_17)
