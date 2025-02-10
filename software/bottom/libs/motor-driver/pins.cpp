#include "pico/stdlib.h"
#include "pinmap.hpp"
#include <map>
#include <iostream>

// pins responsible for providing input to DRV8244
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
		if (this->cache.find(pin) == this->cache.end()) {
			throw std::runtime_error("Pin not initialized");
		}
		gpio_put(pin, value);
		this->cache[pin] = value;
	}

	bool get_last_value(PinMap pin) {
		if (this->cache.find(pin) == this->cache.end()) {
			throw std::runtime_error("Pin not initialized");
		}
		return this->cache[pin];
	}
};


// pins responsible for providing output to DRV8244
class PinOutputControl {
	void init_with_value(PinMap pin) {
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_OUT);
		this->read(pin);
	}

	bool read(PinMap pin) {
		return gpio_get(pin);
	}
};