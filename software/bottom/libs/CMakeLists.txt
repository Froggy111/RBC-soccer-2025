# Initialize the global libraries list
set(GLOBAL_LIBS "" CACHE INTERNAL "List of global libraries")

# Function to add a library to the global list
function(add_global_library lib)
  # Ensure the library exists
  if(NOT TARGET ${lib})
    message(FATAL_ERROR "add_global_library: Target '${lib}' does not exist")
  endif()

  # Add to global list if not already there
  set(GLOBAL_LIBS ${GLOBAL_LIBS} CACHE INTERNAL "List of global libraries")
  list(APPEND GLOBAL_LIBS ${lib})
  list(REMOVE_DUPLICATES GLOBAL_LIBS)
  set(GLOBAL_LIBS ${GLOBAL_LIBS} CACHE INTERNAL "List of global libraries")
endfunction()

# Function to link a target with all global libraries
function(target_link_globals target_name)
  # Ensure the target exists
  if(NOT TARGET ${target_name})
    message(FATAL_ERROR "target_link_globals: Target '${target_name}' does not exist")
  endif()

  # Link all global libraries to the target
  foreach(lib ${GLOBAL_LIBS})
    target_link_libraries(${target_name} ${lib})
  endforeach()
endfunction()

add_global_library(pico_stdlib)
add_global_library(pico_stdio_usb)

# macro configs should be first
add_subdirectory(config) # this links config

# then utils
add_subdirectory(utils) # this links common util libs

# then hardware descriptors
add_subdirectory(hardware-descriptors) # this links hardware-descriptors

# then communication interface
# add_subdirectory(comms) # this links debug

# then pins
add_subdirectory(pins)

# then these which need to link to utils
add_subdirectory(motors)
add_subdirectory(mouse-sensor)