#include "state_input.hpp"
#include <mutex>
#include "spdlog/spdlog.h"

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
	InputState copy = *state;
	m_in.unlock();

	for (std::function<void(InputState)> func : functions)
	{
		func(copy);
	}
}

void InputStateManager::add_function(std::function<void(InputState)> func)
{
	functions.push_back(func);
}

InputStateManager input_state_manager = InputStateManager();