add_executable(hello_world
    main.cpp
)
target_sources(hello_world 
  PUBLIC
    main.cpp
)

# Link against the camera library
target_link_libraries(hello_world)