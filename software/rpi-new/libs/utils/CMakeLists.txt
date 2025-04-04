add_library(utils)

target_sources(utils
    PUBLIC
    include/position.hpp
    include/types.hpp
    include/timer.hpp
    PRIVATE
    position.cpp
)

target_include_directories(utils
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_features(utils PUBLIC cxx_std_17)
target_link_globals(utils)
add_global_library(utils)
