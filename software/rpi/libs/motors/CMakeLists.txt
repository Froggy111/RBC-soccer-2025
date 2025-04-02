add_library(motors)

target_sources(motors
    PUBLIC
    include/motors.hpp
    include/config.hpp
    PRIVATE
    motors.cpp
)

target_include_directories(motors
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)


target_compile_features(motors PUBLIC cxx_std_17)
target_link_libraries(motors comms)