#include "input_state.hpp"
#include <mutex>

std::mutex m_in;

InputStateManager::InputStateManager() : state(new InputState()) {}

InputState InputStateManager::read_state()
{
	m_in.lock();
	InputState copy = *state;
	m_in.unlock();
	return copy;
}

void InputStateManager::update_state()
{
	m_in.lock();
	state->counter++;
	m_in.unlock();
}

InputStateManager input_state_manager = InputStateManager();