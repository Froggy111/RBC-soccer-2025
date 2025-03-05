add_executable(camera_test
    main.cpp
)
target_sources(camera_test 
  PUBLIC
    main.cpp
)

# Link against the camera library
target_link_libraries(camera_test camera)