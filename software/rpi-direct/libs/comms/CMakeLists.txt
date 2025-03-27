add_library(comms)
target_sources(comms
  PRIVATE
    comms.cpp
    usb.cpp
  PUBLIC
    include/comms/usb.hpp
    include/comms/default_usb_config.h
    include/comms.hpp
    include/comms/identifiers.hpp
    include/comms/errors.hpp
)

# Find libusb
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

target_include_directories(comms PUBLIC   ${CMAKE_CURRENT_SOURCE_DIR}/include
${LIBUSB_INCLUDE_DIRS})
target_link_libraries(comms   ${LIBUSB_LIBRARIES}
)
target_link_globals(comms)

add_library(debug_) # named debug_ to avoid clashing with cmake option debug
target_sources(debug_
  PRIVATE
    debug.cpp
  PUBLIC
    include/debug.hpp
)
target_include_directories(debug_ PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(debug_ comms)
target_link_globals(debug_)
add_global_library(debug_)
