#pragma once

#include <components/BaseComponent.h>

#include <glm/glm.hpp>

struct Transform : public BaseComponent
{
	Transform() = delete;
	Transform(Entity* entity) : BaseComponent(entity)
	{

	}

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
};