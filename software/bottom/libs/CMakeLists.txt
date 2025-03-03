# then utils
add_subdirectory(utils) # this links common util libs

# then hardware descriptors
add_subdirectory(hardware-descriptors) # this links hardware-descriptors

# then communication interface
#add_subdirectory(comms) # this links debug

# then pins
add_subdirectory(pins)

# then these which need to link to utils
add_subdirectory(motors)
add_subdirectory(mouse-sensor)
add_subdirectory(line-sensor)