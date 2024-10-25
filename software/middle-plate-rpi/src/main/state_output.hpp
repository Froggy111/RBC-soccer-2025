#pragma once
#include <mutex>
#include <vector>
#include <functional>

extern std::mutex m_out;

struct OutputState
{
	int counter;
};

class OutputStateManager
{
	private:
		OutputState *state;
		std::vector<std::pair<std::function<void(OutputState)>, int>> functions;

	public:
		OutputStateManager();
		OutputState read_state();
		void update_state();

		int add_function(std::function<void(OutputState)> func);
		void deactivate_fuction(int pos);
		
};