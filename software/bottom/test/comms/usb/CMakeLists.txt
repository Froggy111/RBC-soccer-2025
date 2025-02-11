add_executable(usb_test)
target_sources(usb_test
  PRIVATE
    usbtest.cpp
)
target_link_libraries(usb_test usb)
