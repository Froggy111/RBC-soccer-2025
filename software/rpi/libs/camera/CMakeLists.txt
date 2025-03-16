add_library(camera
    camera.cpp
    include/camera.hpp
)

# Find OpenCV 
find_package(OpenCV REQUIRED)

target_include_directories(camera
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link against OpenCV libraries
target_link_libraries(camera PUBLIC ${OpenCV_LIBS})

target_compile_features(camera PUBLIC cxx_std_17)