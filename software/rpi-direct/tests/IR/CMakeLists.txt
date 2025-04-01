add_executable(IR_test main.cpp)
target_link_globals(IR_test)
target_link_libraries(IR_test comms IR)
