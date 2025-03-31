add_executable(IR_test main.cpp)
target_sources(IR
  PUBLIC
    main.cpp
)
target_link_libraries(IR_test IR)
target_link_globals(IR_test)
pico_add_extra_outputs(IR_test)
