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

# Toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/PI.cmake CACHE FILEPATH "Toolchain file")

cmake_minimum_required(VERSION 3.23)

# Metadata
project(bottom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# # ===================================================================================
# # ======================== CAMERA STREAMING DEPENDENCIES ============================
# # Find required packages
# find_package(OpenCV REQUIRED)
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LIBCAMERA REQUIRED libcamera)

# pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)
# message(STATUS "Using libwebsockets for web server")
# set(WEB_SERVER_LIBS ${LIBWEBSOCKETS_LIBRARIES})
# set(WEB_SERVER_INCLUDE_DIRS ${LIBWEBSOCKETS_INCLUDE_DIRS})
# set(WEB_SERVER_CFLAGS ${LIBWEBSOCKETS_CFLAGS})
# add_definitions(-DUSE_LIBWEBSOCKETS)

# # Find threading library
# find_package(Threads REQUIRED)

# # Add dependencies to include path
# include_directories(
#   ${OpenCV_INCLUDE_DIRS}
#   ${LIBCAMERA_INCLUDE_DIRS}
#   ${WEB_SERVER_INCLUDE_DIRS}
# )

# # Setup global compiler flags
# set(CMAKE_CXX_FLAGS "${LIBCAMERA_CFLAGS} ${WEB_SERVER_CFLAGS}")

# Libraries
# add_subdirectory(libs) # global libs need to be explicitly linked to with target_link_globals(TARGET)

# Main
add_subdirectory(src)

# Tests
add_subdirectory(test)