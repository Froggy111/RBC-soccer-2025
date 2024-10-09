#include "state_output.hpp"
#include <mutex>

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
	m_out.unlock();
}

OutputStateManager output_state_manager = OutputStateManager();