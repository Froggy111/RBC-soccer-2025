set(PICO_BOARD pico CACHE STRING "Board type")
cmake_minimum_required(VERSION 3.23)

include(pico_sdk_import.cmake)

project(pico_custom_cdc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(main
    main.c
    usb_descriptors.c
    usb_callbacks.c
)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(main
    pico_stdlib
    pico_unique_id
    pico_usb_reset_interface
    tinyusb_device
)

pico_add_extra_outputs(main)
