add_library(bbw_camera
    camera.cpp
    include/camera.hpp
    include/config.hpp
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
target_link_libraries(bbw_camera PUBLIC ${OpenCV_LIBS} ${LIBCAMERA_LIBRARIES})

target_compile_features(bbw_camera PUBLIC cxx_std_17)