add_executable(webserver_test
    main.cpp
)
target_sources(webserver_test 
  PUBLIC
    main.cpp
)

target_compile_options(webserver_test PRIVATE -fno-builtin-math)

# Link against the camera library
target_link_libraries(webserver_test webserver)