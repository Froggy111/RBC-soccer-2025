#include "debug.hpp"
#include "motion.hpp"
#include "types.hpp"

namespace strategy {

namespace OrbitConstants {
// Desired distance 'behind' the ball relative to goal_heading
constexpr types::f32 TARGET_DISTANCE_BEHIND =
    30.0f; // Similar to BALLCAP_DISTANCE

// Desired lateral clearance from the ball during the main orbit maneuver
constexpr types::f32 LATERAL_CLEARANCE =
    25.0f; // Similar to CLEARANCE_X / 2 + buffer

// How far 'behind' the ball to target when needing significant repositioning
// Can be different from TARGET_DISTANCE_BEHIND to create smoother paths
constexpr types::f32 MANEUVER_DISTANCE_BEHIND = 35.0f;

// Thresholds to decide if we are 'behind' enough or laterally aligned enough
// If robot_y < -APPROACH_THRESHOLD_Y, we consider it behind.
constexpr types::f32 APPROACH_THRESHOLD_Y = TARGET_DISTANCE_BEHIND * 0.8f;
// If abs(robot_x) < APPROACH_THRESHOLD_X, we consider it roughly aligned laterally.
constexpr types::f32 APPROACH_THRESHOLD_X =
    5.0f; // Similar to BALLCAP_WIDTH / 2

// Speed control
constexpr types::f32 MAX_ORBIT_SPEED = 0.8f; // Max speed (0 to 1) during orbit
constexpr types::f32 MIN_ORBIT_SPEED = 0.2f; // Min speed to ensure movement

// Small value to avoid division by zero or issues with zero vectors
constexpr types::f32 EPSILON = 1e-4f;
} // namespace OrbitConstants
//
enum class AttackMode {
    orbit,
    scoring,
};

enum class OrbitMode {
    ball_behind,
    ball_in_front,
};
const types::f32 attack_speed_multi = 0.5;

// add line sensor later...
void attack(types::Vec2f32 ball_pos, types::f32 goal_heading,
            bool ball_captured, const types::Vec2f32 &line_evade);

void evaluate_attack_mode(types::Vec2f32 ball_pos, types::f32 goal_heading,
                          bool ball_captured);

void orbit(types::Vec2f32 ball_pos, types::f32 goal_heading,
           const types::Vec2f32 &line_evade);
} // namespace strategy
