#pragma once

class Objective
{
	virtual float evaluate(const Keyboard& keyboard) const = 0;
};
