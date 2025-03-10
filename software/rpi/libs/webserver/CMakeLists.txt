add_library(webserver)
target_sources(webserver
  PRIVATE
    webserver.cpp
  PUBLIC
    include/webserver.hpp
)

target_include_directories(webserver PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(webserver PRIVATE ${LIBWEBSOCKETS_LIBRARIES})
target_compile_definitions(webserver PRIVATE USE_LIBWEBSOCKETS)

# Link with OpenCV for image encoding
target_link_libraries(webserver PRIVATE ${OpenCV_LIBS})

# Link with threading library
target_link_libraries(webserver PRIVATE Threads::Threads)

target_link_globals(webserver)