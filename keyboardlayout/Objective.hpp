#pragma once

template<size_t KeyboardSize>
class Objective
{
	virtual float evaluate(const Keyboard<KeyboardSize>& keyboard) const = 0;
};
