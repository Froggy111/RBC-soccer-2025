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
		std::vector<std::pair<std::function<void(InputState)>, int>> functions;

	public:
		InputStateManager();
		InputState read_state();
		void update_state();

		int add_function(std::function<void(InputState)> func);
		void deactivate_fuction(int pos);
		
};