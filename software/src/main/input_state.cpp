#include "input_state.hpp"
#include <mutex>

std::mutex m;

InputStateManager::InputStateManager() : state(new InputState()) {}

InputState InputStateManager::read_state()
{
	m.lock();
	InputState copy = *state;
	m.unlock();
	return copy;
}

void InputStateManager::update_state()
{
	m.lock();
	state->counter++;
	m.unlock();
}

InputStateManager input_state_manager = InputStateManager();