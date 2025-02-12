# utils should always be first
add_subdirectory(utils) # this also links common util libs

# then hardware descriptors
add_subdirectory(hardware-descriptors) # this also links hardware-descriptors

# then communication interface
add_subdirectory(comms) # this also links debug

# then pins
add_subdirectory(pins)

# then these which need to link to utils
add_subdirectory(motors)
add_subdirectory(mouse-sensor)
