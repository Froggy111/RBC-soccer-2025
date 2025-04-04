#include "debug.hpp"
#include "motion.hpp"
#include "types.hpp"

namespace strategy {
enum class AttackMode {
    orbit,
    scoring,
};

struct OrbitState {
    types::Vec2f32 ball_pos = types::Vec2f32(0, 0);
};

namespace orbit_constants {}

const types::f32 attack_speed_multi = 0.5;

// add line sensor later...
void attack(types::Vec2f32 ball_pos, types::f32 goal_heading,
            bool ball_captured, const types::Vec2f32 &line_evade);

void evaluate_attack_mode(types::Vec2f32 ball_pos, types::f32 goal_heading,
                          bool ball_captured);
void orbit(types::Vec2f32 ball_pos, types::f32 goal_heading);
} // namespace strategy
