add_executable(ir main.cpp)
target_sources(ir
  PUBLIC
    main.cpp
)
pico_enable_stdio_usb(ir 1)
target_link_libraries(ir TSSP4038 hardware_gpio comms)
pico_add_extra_outputs(ir)