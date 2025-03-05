add_library(camera)
target_sources(camera
  PRIVATE
    camera.cpp
  PUBLIC
    include/camera.hpp
)

target_include_directories(camera PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(camera)
target_link_globals(camera)
