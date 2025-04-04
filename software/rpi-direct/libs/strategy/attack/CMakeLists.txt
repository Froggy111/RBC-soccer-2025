add_library(attack)
target_sources(attack
PUBLIC
    attack.hpp
    ball_capture.hpp
PRIVATE
    attack.cpp
    )
