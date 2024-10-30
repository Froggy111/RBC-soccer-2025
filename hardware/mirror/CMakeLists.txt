cmake_minimum_required(VERSION 3.22)

project(mirror-generator
VERSION 0.0
LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories("src")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
link_libraries(SDL2::SDL2)

set(CMAKE_CXX_FLAGS "-O3")

file(GLOB_RECURSE LIB_SOURCES "src/lib/*.cpp")
file(GLOB_RECURSE LIB_HEADERS "src/lib/*.hpp")
add_library(lib ${LIB_SOURCES} ${LIB_HEADERS})

file(GLOB_RECURSE TEST_SOURCES "src/tests/*.cpp")
foreach(testsourcefile ${TEST_SOURCES})
  get_filename_component(testname ${testsourcefile} NAME_WE)
  add_executable(${testname}.o ${testsourcefile})
  get_filename_component(testdir ${testsourcefile} DIRECTORY)
  set_target_properties(${testname}.o PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${testdir})
  target_link_libraries(${testname}.o lib)
endforeach(testsourcefile ${TEST_SOURCES})

file(GLOB APP_SOURCES "src/*.cpp")
foreach(appsourcefile ${APP_SOURCES})
  get_filename_component(appname ${appsourcefile} NAME_WE)
  add_executable(${appname}.o ${appsourcefile})
  get_filename_component(appdir ${appsourcefile} DIRECTORY)
  set_target_properties(${appname}.o PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${appdir})
  target_link_libraries(${appname}.o lib)
endforeach(appsourcefile ${APP_SOURCES})