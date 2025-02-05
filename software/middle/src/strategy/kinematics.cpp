#include "strategy/kinematics.hpp"
#include "utils/vector.hpp"

Kinematics::Kinematics() : position(new Vector2D(0, 0)), velocity(new Vector2D(0, 0)), acceleration(new Vector2D(0, 0)) {}

void Kinematics::update_position(Vector2D *position) {
}