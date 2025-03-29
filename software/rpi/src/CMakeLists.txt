add_executable(main main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(main 
    bbw_camera
    ${CMAKE_THREAD_LIBS_INIT}
)


target_link_libraries(main bbw_camera motion-control)
