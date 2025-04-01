add_library(IR)
target_sources(IR
PRIVATE
  IR.cpp
PUBLIC
  include/IR.hpp
)

target_include_directories(IR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_globals(IR)
