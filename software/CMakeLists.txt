cmake_minimum_required(VERSION 3.30.2)
project(
  RBC-2025
  VERSION 1.0
  DESCRIPTION
  "Robocup Soccer 2025"
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Include directories
include_directories("src" "lib" "utils")

# Collect all source files
file(GLOB_RECURSE SRC_SOURCES "src/*.cpp")
file(GLOB_RECURSE SRC_SOURCES "src/*.cpp")
file(GLOB_RECURSE SRC_MAIN_SOURCES "src/main/*.cpp")
file(GLOB_RECURSE SRC_MAIN_HEADERS "src/main/*.hpp")
add_library(lib ${SRC_SOURCES} ${SRC_HEADERS} ${SRC_MAIN_SOURCES} ${SRC_MAIN_HEADERS})

file(GLOB_RECURSE LIB_SORUCES "lib/*.cpp")
file(GLOB_RECURSE LIB_HEADERS "lib/*.hpp")
foreach(libsourcefile ${LIB_SOURCES})
  get_filename_component(libname ${libsourcefile} NAME_WE)
  add_library(${libname}.o ${libsourcefile})
  get_filename_component(libdir ${libsourcefile} DIRECTORY)
  set_target_properties(${libname}.o PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${libdir})
  target_link_libraries(${libname}.o lib)
endforeach(libsourcefile ${LIB_SOURCES})

file(GLOB_RECURSE UTILS_SOURCES "utils/*.cpp")
file(GLOB_RECURSE UTILS_HEADERS "utils/*.hpp")
foreach(utilsourcefile ${UTILS_SOURCES})
  get_filename_component(utilname ${utilsourcefile} NAME_WE)
  add_library(${utilname}.o ${utilsourcefile})
  get_filename_component(utildir ${utilsourcefile} DIRECTORY)
  set_target_properties(${utilname}.o PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${utildir})
  target_link_libraries(${utilname}.o lib)
endforeach(utilsourcefile ${UTILS_SOURCES})