#pragma once
#include "utils/vector.hpp"

class Kinematics {
	private:
		Vector2D *position;
		Vector2D *velocity;
		Vector2D *acceleration;

	public:
		Kinematics();
		void update_position(Vector2D *position);
};