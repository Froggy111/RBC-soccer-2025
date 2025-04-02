add_executable(ball-detect-test main.cpp)
target_link_globals(ball-detect-test)
target_link_libraries(ball-detect-test comms IR WS2812 types ball-detect)
