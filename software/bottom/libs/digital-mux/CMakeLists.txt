add_library(MCP23S17)
target_sources(MCP23S17
  PRIVATE
    MCP23S17.cpp
  PUBLIC
    MCP23S17.hpp
)

add_library(dmux)
target_sources(dmux
  PRIVATE
    dmux.cpp
  PUBLIC
    dmux.hpp
)
target_link_libraries(dmux MCP23S17)
