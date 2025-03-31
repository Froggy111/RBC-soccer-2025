add_library(motion-control)

target_sources(motion-control
    PUBLIC
    include/motion.hpp
    PRIVATE
    motion.cpp
)

target_include_directories(motion-control
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(motion-control bbw_camera)
target_compile_features(motion-control PUBLIC cxx_std_17)
target_link_globals(motion-control)
add_global_library(motion-control)