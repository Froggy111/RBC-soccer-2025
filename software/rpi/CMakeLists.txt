cmake_minimum_required(VERSION 3.20)
project(greeter VERSION 1.0.0 LANGUAGES CXX)
enable_testing()

add_subdirectory("libs")
add_subdirectory("src")
add_subdirectory("tests")

# Packaging
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
include(CPack)