#include "pico/stdlib.h"
#include "pinmap.hpp"
#include <map>

class PinInputControl {
	std::map<PinMap, bool> cache;

	void init(PinMap pin, bool value) {
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		write(pin, value);
	}
	
	void init(PinMap pin) {
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
	}

	bool write(PinMap pin, bool value) {
		gpio_put(pin, value);
		this->cache[pin] = value;
	}

	bool get_last_value(PinMap pin) {
		return this->cache[pin];
	}
};

class PinOutputControl {
	void init_with_value(PinMap pin) {
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		this->read(pin);
	}

	bool read(PinMap pin) {
		return gpio_get(pin);
	}
};