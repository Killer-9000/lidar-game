#include "Entity.h"

#include <components/Hierarchy.h>
#include <components/Name.h>
#include <components/Transform.h>

Entity::Entity(const entt::entity& entity, entt::registry* registry)
	: _entity(entity), _registry(registry)
{
	_hierarchyComponent = &GetOrAddComponent<Hierarchy>();
	_nameComponent = &GetOrAddComponent<Name>();
	_transformComponent = &GetOrAddComponent<Transform>();
}

const std::string& Entity::GetName() const
{
	return _nameComponent->name;
}

void Entity::SetName(const std::string& name)
{
	_nameComponent->name = name;
}
