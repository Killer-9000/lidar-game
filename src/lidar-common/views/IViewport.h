#pragma once

#include "data/Entity.h"

// A viewport of a scene.
class IViewport
{
public:
	Entity CreateEntity()
	{
		return Entity{ _registry.create(), &_registry };
	};

	void RemoveEntity(Entity entity)
	{
		_registry.destroy(entity);
	};

	void RemoveAllEntities()
	{
		_registry = entt::registry();
	};

	const entt::registry& GetRegistry() const { return _registry; }

protected:
	entt::registry _registry;
};
