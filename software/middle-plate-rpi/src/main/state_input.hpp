#pragma once
#include <mutex>
#include <vector>
#include <functional>


extern std::mutex m_in;

struct InputState
{
	int counter;
};

class InputStateManager
{
private:
	InputState *state;
	std::vector<std::function<void(InputState)>> functions;

public:
	InputStateManager();
	InputState read_state();
	void update_state();
	void add_function(std::function<void(InputState)> func);
};