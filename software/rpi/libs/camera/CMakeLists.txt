add_library(bbw_camera)

target_sources(bbw_camera
    PRIVATE
    camera.cpp
    processor.cpp
    PUBLIC
    include/camera.hpp
    include/config.hpp
    include/processor.hpp
    include/field.hpp
    include/field_chunked.hpp
)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

target_include_directories(bbw_camera
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${LIBCAMERA_INCLUDE_DIRS}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link against OpenCV libraries
target_link_libraries(bbw_camera PUBLIC ${OpenCV_LIBS} ${LIBCAMERA_LIBRARIES} utils IMU debug_)

target_compile_features(bbw_camera PUBLIC cxx_std_17)
