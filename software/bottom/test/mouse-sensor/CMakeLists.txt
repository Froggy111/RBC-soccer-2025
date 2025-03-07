add_executable(mouse_sensor_test)
target_sources(mouse_sensor_test 
  PRIVATE
    main.cpp
)
target_link_libraries(mouse_sensor_test comms)
target_link_globals(mouse_sensor_test)
pico_add_extra_outputs(mouse_sensor_test)