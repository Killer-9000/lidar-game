#pragma once

#include <components/BaseComponent.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>

struct Name;
struct Transform;
struct Hierarchy;

struct Entity
{
public:
	Entity() { }

	Entity(const entt::entity& entity, entt::registry* registry);

	template <class T, class... Args>
	T& AddComponent(Args&&... args)
	{
		assert(!HasComponent<T>() && "Entity already has this component.");
		return _registry->emplace<T>(_entity, this, std::forward<Args>(args)...);
	}

	template <class T>
	T& GetComponent() const
	{
		assert(HasComponent<T>() && "Entity doesn't have component.");
		return _registry->get<T>(_entity);
	}

	template <class T>
	bool HasComponent() const
	{
		assert(_entity != entt::null && "Entity is null.");
		return _registry->any_of<T>(_entity);
	}

	template <class T>
	void RemoveComponent()
	{
		assert(HasComponent<T>() && "Entity doesn't have component.");
		_registry->remove<T>(_entity);
	}

	template <class T, class... Args>
	T& GetOrAddComponent(Args&&... args)
	{
		return HasComponent<T>() ? GetComponent<T>() : AddComponent<T>(std::forward<Args>(args)...);
	}

	operator entt::entity() const { return _entity; }
	operator bool() const { return _entity != entt::null; }

	// Cached components
	Transform* GetTransform() const { return _transformComponent; }
	Hierarchy* GetHierarchy() const { return _hierarchyComponent; }
	const std::string& GetName() const;
	void SetName(const std::string& name);

	entt::entity InternalId() const { return _entity; }

private:
	entt::entity _entity = entt::null;
	entt::registry* _registry = nullptr;

	// Cached components
	Hierarchy* _hierarchyComponent = nullptr;
	Name* _nameComponent = nullptr;
	Transform* _transformComponent = nullptr;
};