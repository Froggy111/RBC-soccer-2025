#pragma once
#include <ctime>
#include "types/loop_types.hpp"

class FPS {
	private:
		clock_t input_loop_previous;
		clock_t input_loop_current;
		clock_t strategy_loop_previous;
		clock_t strategy_loop_current;
		clock_t output_loop_previous;
		clock_t output_loop_current;
	public:
		FPS();
		void update_fps(LoopType loop);
		int get_specific_fps(LoopType loop);
		int display_fps();
};
