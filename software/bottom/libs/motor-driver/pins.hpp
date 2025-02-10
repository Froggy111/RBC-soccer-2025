#pragma once

#include "pinmap.hpp"
#include <map>

class PinInputControl {
public:
    void init(PinMap pin, bool value);
    void init(PinMap pin);
    bool write(PinMap pin, bool value);
    bool get_last_value(PinMap pin);

private:
    std::map<PinMap, bool> cache;
};

class PinOutputControl {
public:
    void init_with_value(PinMap pin);
    bool read(PinMap pin);
};