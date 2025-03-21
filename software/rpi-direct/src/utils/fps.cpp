#include "fps.hpp"
#include <iostream>
#include <ctime>
#include "spdlog/spdlog.h"

// Constructor to initialize member variables
FPS::FPS() 
    : input_loop_previous(clock()), input_loop_current(clock()),
      strategy_loop_previous(clock()), strategy_loop_current(clock()),
      output_loop_previous(clock()), output_loop_current(clock()) {}

// Method to update the FPS values
void FPS::update_fps(LoopType loop) {
	clock_t new_fps = clock();
	switch (loop) {
		case INPUT:
			input_loop_previous = input_loop_current;
			input_loop_current = new_fps;
			break;
		case STRATEGY:
			strategy_loop_previous = strategy_loop_current;
			strategy_loop_current = new_fps;
			break;
		case OUTPUT:
			output_loop_previous = output_loop_current;
			output_loop_current = new_fps;
			break;
	}
}

// Method to return a specific FPS value
int FPS::get_specific_fps(LoopType loop) {
	switch (loop) {
		case INPUT:
			return CLOCKS_PER_SEC / (input_loop_current - input_loop_previous);
		case STRATEGY:
			return CLOCKS_PER_SEC / (input_loop_current - input_loop_previous);
		case OUTPUT:
			return CLOCKS_PER_SEC / (input_loop_current - input_loop_previous);
	}
	return 0;
}

// Method to display the FPS values
int FPS::display_fps() {
	spdlog::info("Input Loop FPS: {}, Strategy Loop FPS: {}, Output Loop FPS: {}", get_specific_fps(INPUT), get_specific_fps(STRATEGY), get_specific_fps(OUTPUT));
    return 0;
}