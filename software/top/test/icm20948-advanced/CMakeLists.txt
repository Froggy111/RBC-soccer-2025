add_executable(icm20948_final_test main.cpp)
target_sources(icm20948_final_test
  PUBLIC
    main.cpp
)

target_link_globals(icm20948_final_test)
target_link_libraries(icm20948_final_test ICM20948 comms)
pico_add_extra_outputs(icm20948_final_test)