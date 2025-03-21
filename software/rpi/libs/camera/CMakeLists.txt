add_library(bbw_camera)

target_sources(bbw_camera
    PRIVATE
    camera.cpp
    processor.cpp
    PUBLIC
    include/camera.hpp
    include/config.hpp
    include/processor.hpp)

target_include_directories(bbw_camera
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${LIBCAMERA_INCLUDE_DIRS}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link against OpenCV libraries
target_link_libraries(bbw_camera PUBLIC ${OpenCV_LIBS} ${LIBCAMERA_LIBRARIES})

target_compile_features(bbw_camera PUBLIC cxx_std_17)
