add_executable(pf_test main.cpp)

target_link_libraries(pf_test
PUBLIC
    particle_filter
)

target_compile_features(pf_test PUBLIC cxx_std_17)
