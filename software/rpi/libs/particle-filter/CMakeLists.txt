add_library(particle_filter)

target_sources(particle_filter
    PUBLIC
    include/pf.hpp
    PRIVATE
    pf.cpp
)

target_include_directories(particle_filter
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)


target_compile_features(particle_filter PUBLIC cxx_std_17)
target_link_globals(particle_filter)
add_global_library(particle_filter)