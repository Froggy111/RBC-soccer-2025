#include "pinmap.hpp"
#include <map>
#include <stdexcept>

class PinInputControl
{
public:
    void init_digital(PinMap pin, bool value);
    void init_analog(PinMap pin, int value);
    void init(PinMap pin);
    void write_digital(PinMap pin, bool value);
    void write_analog(PinMap pin, int value);
    bool get_last_value(PinMap pin);

private:
    std::map<PinMap, std::pair<bool, int>> cache;
};

class PinOutputControl
{
public:
    void init_digital(PinMap pin);
    bool read_digital(PinMap pin);
};