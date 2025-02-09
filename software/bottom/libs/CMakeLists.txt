# utils should always be first
add_subdirectory(utils)

# then communication interface
add_subdirectory(comms)

# then these which need to link to utils
add_subdirectory(motor-driver)
add_subdirectory(digital-mux)
add_subdirectory(mouse-sensor)
