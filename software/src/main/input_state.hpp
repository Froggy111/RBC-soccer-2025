#include <mutex>

extern std::mutex m_in;

struct InputState
{
	int counter;
};

class InputStateManager
{
private:
	InputState *state;

public:
	InputStateManager();
	InputState read_state();
	void update_state();
};

extern InputStateManager input_state_manager;