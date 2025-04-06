#include "attack.hpp"
#include "debug.hpp"
#include "motors.hpp"
#include "types.hpp"

using namespace types;

namespace strategy {
static AttackMode attack_mode = AttackMode::orbit;
void attack(Vec2f32 ball_pos, f32 goal_heading, bool ball_captured,
            const Vec2f32 &line_evade) {
    evaluate_attack_mode(ball_pos, goal_heading, ball_captured);
    switch (attack_mode) {
        case AttackMode::orbit:
            orbit(ball_pos, goal_heading, line_evade);
            break;
        case AttackMode::scoring:
            motors::translate_with_target_heading(
                attack_speed_multi, goal_heading, goal_heading, line_evade);
            break;
    }
}

void evaluate_attack_mode(Vec2f32 ball_pos, f32 goal_heading,
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

void orbit(types::Vec2f32 ball_pos_rel, types::f32 goal_heading,
           const types::Vec2f32 &line_evade) {
    using namespace OrbitConstants;

    types::f32 ball_rel_x = ball_pos_rel.x;
    types::f32 ball_rel_y = ball_pos_rel.y;

    // Target position relative to the ball.
    // We want to end up at (0, -TARGET_DISTANCE_BEHIND) relative to the ball.
    types::Vec2f32 target_pos_rel_ball(0, 0);

    // --- Decide on the intermediate target based on current relative position ---
    // This logic determines where around the ball we should aim right now.

    // Condition check: Are we too close laterally or not far enough behind?
    // This logic is inspired by the Rust code's "moving_back" conditions.
    // Simplified: If we are in front of the desired 'behind' position, or
    // if we are behind but too close laterally, perform a wider maneuver.
    bool needs_maneuver =
        (ball_rel_y > -APPROACH_THRESHOLD_Y) ||
        ((ball_rel_y <= -APPROACH_THRESHOLD_Y &&
          std::fabs(ball_rel_x) < LATERAL_CLEARANCE - APPROACH_THRESHOLD_X));

    if (needs_maneuver) {
        // --- Wide Maneuver ---
        // Aim for a point laterally offset and further behind the ball.
        // Determine which side to orbit towards based on the ball's relative x.
        // Use copysignf to get the sign of ball_rel_x or default to positive if x is near zero.
        types::f32 target_x_offset =
            std::copysignf(LATERAL_CLEARANCE, ball_rel_x);
        // Handle case where ball is directly in front or behind
        if (std::fabs(ball_rel_x) < EPSILON) {
            target_x_offset = LATERAL_CLEARANCE; // Default to orbiting right
        }

        target_pos_rel_ball.x = target_x_offset;
        target_pos_rel_ball.y =
            -MANEUVER_DISTANCE_BEHIND; // Aim further back during maneuver
    } else {
        // --- Approach / Fine Tune ---
        // We are sufficiently behind and laterally clear. Aim closer to the
        // ideal 'behind' position.
        target_pos_rel_ball.x = 0.0f; // Aim directly behind
        target_pos_rel_ball.y = -TARGET_DISTANCE_BEHIND;
    }

    // --- Calculate the vector from the robot to the target point ---
    // Target point relative to robot = ball pos relative to robot + target pos relative to ball
    types::Vec2f32 target_pos_rel_robot(0, 0);
    target_pos_rel_robot.x = ball_pos_rel.x + target_pos_rel_ball.x;
    target_pos_rel_robot.y = ball_pos_rel.y + target_pos_rel_ball.y;

    // --- Calculate desired movement vector (speed and direction) ---
    types::f32 target_dist_sq =
        target_pos_rel_robot.x * target_pos_rel_robot.x +
        target_pos_rel_robot.y * target_pos_rel_robot.y;
    types::f32 target_dist = sqrtf(target_dist_sq);

    types::f32 speed = MAX_ORBIT_SPEED; // Start with max speed
    // Optional: Scale speed based on distance (e.g., slow down when close)
    // speed = std::min(MAX_ORBIT_SPEED, MIN_ORBIT_SPEED + target_dist * 0.05f); // Example scaling

    // Calculate the heading for translation relative to the robot's current forward direction
    // atan2f(y, x) gives angle from positive X-axis.
    // We want angle from positive Y-axis (forward). atan2f(x, y) does this.
    types::f32 translate_heading =
        atan2f(target_pos_rel_robot.x, target_pos_rel_robot.y);

    // Ensure we move if the target is not exactly the current position
    if (target_dist < EPSILON) {
        speed             = 0.0f; // Don't move if already at the target
        translate_heading = 0.0f; // Or maintain current heading
    }

    // --- Call the low-level translation function ---
    // Use the calculated speed, relative translation heading, and the desired world orientation.
    motors::translate_with_target_heading(
        speed, translate_heading, goal_heading,
        line_evade); // Pass line_evade through
}
} // namespace strategy
