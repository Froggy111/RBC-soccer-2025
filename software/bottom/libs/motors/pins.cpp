#include "pico/stdlib.h"
#include "pinmap.hpp"
#include "pins.hpp"
#include <map>
#include <iostream>

// pins responsible for providing input to DRV8244
void PinInputControl::init_digital(PinMap pin, bool value)
{
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
	write_digital(pin, value);
}

void PinInputControl::init_analog(PinMap pin, int value)
{
	// help
}

void PinInputControl::init(PinMap pin)
{
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
}

void PinInputControl::write_digital(PinMap pin, bool value)
{
	if (cache.find(pin) == cache.end())
	{
		throw std::runtime_error("Pin not initialized");
	}
	gpio_put(pin, value);
	cache[pin] = {0, value};
}

void PinInputControl::write_analog(PinMap pin, int value)
{
	// help
	cache[pin] = {1, value};
}

bool PinInputControl::get_last_value(PinMap pin)
{
	if (this->cache.find(pin) == this->cache.end())
	{
		throw std::runtime_error("Pin not initialized");
	}
	return this->cache[pin].second;
}

// pins responsible for providing output to DRV8244
void PinOutputControl::init_digital(PinMap pin)
{
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_OUT);
	read_digital(pin);
}

void PinOutputControl::init_analog(PinMap pin)
{
	// help
}

bool PinOutputControl::read_digital(PinMap pin)
{
	return gpio_get(pin);
}

int PinOutputControl::read_analog(PinMap pin)
{
	// help
}