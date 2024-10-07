#include <mutex>

extern std::mutex m_out;

struct OutputState
{
    int counter;
};

class OutputStateManager
{
private:
    OutputState *state;

public:
    OutputStateManager();
    OutputState read_state();
    void update_state();
};

extern OutputStateManager output_state_manager;