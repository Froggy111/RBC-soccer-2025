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

	for (std::pair<std::function<void(InputState)>, int> funcInfo : functions)
	{
		if (!funcInfo.second) continue;
		std::function<void(InputState)> func = funcInfo.first;
		func(copy);
	}
}

int InputStateManager::add_function(std::function<void(InputState)> func)
{
	int pos = functions.size();
	functions.push_back({
		func, true
	});

	// returns function position
	return pos;
}

void InputStateManager::deactivate_fuction(int pos)
{
	functions[pos].second = false;
}

InputStateManager input_state_manager = InputStateManager();