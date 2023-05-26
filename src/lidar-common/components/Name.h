#pragma once

#include <components/BaseComponent.h>

#include <string>

struct Name : public BaseComponent
{

	Name() = delete;
	Name(Entity* entity) : BaseComponent(entity)
	{

	}

	std::string name;
};