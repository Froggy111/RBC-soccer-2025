# ====================================================================================

# ================================= HELPER FUNCTIONS =================================
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

cmake_minimum_required(VERSION 3.23)

# Metadata
project(bottom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ====================================================================================

# ===================================== RASPICAM =====================================

# Define the path to raspicam
set(RASPICAM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/raspicam")

# Set C++ standard to C++11 before building raspicam
set(CMAKE_CXX_STANDARD_BACKUP ${CMAKE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD 11)

# First build the raspicam library
add_subdirectory(${RASPICAM_SOURCE_DIR} ${CMAKE_BINARY_DIR}/raspicam)

# Restore original C++ standard
set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD_BACKUP})

# Make raspicam targets available directly
if(TARGET raspicam)
  add_global_library(raspicam)
  message(STATUS "Found raspicam target")
endif()

if(TARGET raspicam_cv)
  add_global_library(raspicam_cv)
  message(STATUS "Found raspicam_cv target")
endif()

# Add include directories
include_directories(${RASPICAM_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/raspicam) # For generated files

# Libraries
add_subdirectory(libs) # global libs need to be explicitly linked to with target_link_globals(TARGET)

# Main
add_subdirectory(src)

# Tests
add_subdirectory(test)