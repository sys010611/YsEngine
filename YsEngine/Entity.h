#pragma once

#include <string>

class Entity
{
public:
	std::string GetName() { return name; }

protected:
	std::string name;
};

