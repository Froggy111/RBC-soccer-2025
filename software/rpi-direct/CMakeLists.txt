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
# ====================================================================================

cmake_minimum_required(VERSION 3.20)
project(middle C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(calibration)
