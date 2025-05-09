#include "attack.hpp"
#include "debug.hpp"
#include "motors.hpp"
#include "types.hpp"

using namespace types;

namespace strategy {
static AttackMode attack_mode = AttackMode::orbit;
void attack(f32 ball_heading, f32 goal_heading, bool ball_captured,
            const Vec2f32 &line_evade) {
    evaluate_attack_mode(ball_heading, goal_heading, ball_captured);
    switch (attack_mode) {
        case AttackMode::orbit: break;
        case AttackMode::scoring:
            motors::translate_with_target_heading(
                attack_speed_multi, goal_heading, goal_heading, line_evade);
            break;
    }
}

void evaluate_attack_mode(f32 ball_heading, f32 goal_heading,
                          bool ball_captured) {
    if (ball_captured) {
        if (attack_mode != AttackMode::scoring) {
            debug::info("Attack: ball captured, switched to scoring mode\n");
        }
        attack_mode = AttackMode::scoring;
    } else {
        if (attack_mode != AttackMode::orbit) {
            debug::info("Attack: ball not captured, switched to orbit mode\n");
        }
        attack_mode = AttackMode::orbit;
    }
}
} // namespace strategy
