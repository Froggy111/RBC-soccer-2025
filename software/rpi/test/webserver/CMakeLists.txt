add_executable(webserver_test
    main.cpp
)
target_sources(webserver_test 
  PUBLIC
    main.cpp
)

# Link against the camera library
target_link_libraries(webserver_test webserver)