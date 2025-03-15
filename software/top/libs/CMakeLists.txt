# then utils
add_subdirectory(utils) # this links common util libs

# then hardware descriptors
add_subdirectory(hardware-descriptors) # this links hardware-descriptors

# then communication interface
add_subdirectory(comms) # this links debug

# then pins
add_subdirectory(pins)

add_subdirectory(imu)

# add_subdirectory(ultrasounds)