#include "pico/stdlib.h"
#include "pinmap.hpp"
#include <map>
#include <iostream>

// pins responsible for providing input to DRV8244
class PinInputControl
{
	std::map<PinMap, std::pair<bool, int>> cache;

	void init_digital(PinMap pin, bool value)
	{
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
		write_digital(pin, value);
	}

	void init_analog(PinMap pin, int value)
	{
		// help
	}

	void init(PinMap pin)
	{
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_IN);
	}

	void write_digital(PinMap pin, bool value)
	{
		if (this->cache.find(pin) == this->cache.end())
		{
			throw std::runtime_error("Pin not initialized");
		}
		gpio_put(pin, value);
		this->cache[pin] = {0, value};
	}

	void write_analog(PinMap pin, int value)
	{
		// help
		this->cache[pin] = {1, value};
	}

	bool get_last_value(PinMap pin)
	{
		if (this->cache.find(pin) == this->cache.end())
		{
			throw std::runtime_error("Pin not initialized");
		}
		return this->cache[pin].second;
	}
};

// pins responsible for providing output to DRV8244
class PinOutputControl
{
	void init_digital(PinMap pin)
	{
		gpio_init(pin);
		gpio_set_dir(pin, GPIO_OUT);
		this->read_digital(pin);
	}

	void init_analog(PinMap pin)
	{
		// help
	}

	bool read_digital(PinMap pin)
	{
		return gpio_get(pin);
	}

	int read_analog(PinMap pin)
	{
		// help
	}
};