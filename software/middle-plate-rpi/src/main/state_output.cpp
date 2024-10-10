#include "state_output.hpp"
#include <mutex>
#include "spdlog/spdlog.h"

std::mutex m_out;

OutputStateManager::OutputStateManager() : state(new OutputState()) {}

OutputState OutputStateManager::read_state()
{
	m_out.lock();
	OutputState copy = *state;
	m_out.unlock();
	return copy;
}

void OutputStateManager::update_state()
{
	m_out.lock();
	state->counter++;
	OutputState copy = *state;
	m_out.unlock();

	for (std::pair<std::function<void(OutputState)>, int> funcInfo : functions)
	{
		if (!funcInfo.second) continue;
		std::function<void(OutputState)> func = funcInfo.first;
		func(copy);
	}
}

int OutputStateManager::add_function(std::function<void(OutputState)> func)
{
	int pos = functions.size();
	functions.push_back({
		func, true
	});

	// returns function position
	return pos;
}

void OutputStateManager::deactivate_fuction(int pos)
{
	functions[pos].second = false;
}

OutputStateManager input_state_manager = OutputStateManager();