#include "debug.hpp"
#include "motion.hpp"
#include "types.hpp"

namespace strategy {
enum AttackMode {
    orbit,
    scoring,
};

enum OrbitMode {
    ball_behind,
    ball_in_front,
};
const types::f32 attack_speed_multi = 0.5;

// add line sensor later...
void attack(types::f32 ball_heading, types::f32 goal_heading,
            bool ball_captured, const types::Vec2f32 &line_evade);

void evaluate_attack_mode(types::f32 ball_heading, types::f32 goal_heading,
                          bool ball_captured);
} // namespace strategy
