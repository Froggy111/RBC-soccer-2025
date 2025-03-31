add_library(IR)
target_sources(IR
  PRIVATE
    IR.cpp
  PUBLIC
    include/IR.hpp
)
target_include_directories(IR PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(IR hardware_timer hardware_gpio)
target_link_globals(IR)
