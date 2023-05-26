#pragma once

#include <components/BaseComponent.h>

#include <data/Entity.h>

struct Hierarchy : public BaseComponent
{
	template <typename FUNC>
	static void RecurseChildren(const Entity* entity, FUNC&& func)
	{
		if (!func(entity))
			return;

		for (const Entity& child : entity->GetHierarchy()->children)
		{
			if (child)
				RecurseChildren(&child, func);
		}
	}

	template <typename FUNC1, typename FUNC2>
	static void RecurseChildren(const Entity* entity, FUNC1&& beginFunc, FUNC2&& endFunc)
	{
		if (!beginFunc(entity))
			return;

		for (const Entity& child : entity->GetHierarchy()->children)
		{
			if (child)
				RecurseChildren(&child, beginFunc, endFunc);
		}

		endFunc(entity);
	}

	Hierarchy() = delete;
	Hierarchy(Entity* entity) : BaseComponent(entity)
	{

	}

	static constexpr size_t MAX_CHILDREN = 256;

	// If parentLocked, the parent can't be changed.
	bool parentLocked = false;
	Entity* parent = nullptr;
	std::array<Entity, MAX_CHILDREN> children;
	uint32_t childrenCount = 0;

	bool UpdateParent(Entity* newParent)
	{
		assert(!parentLocked && "Trying to update a parent is not allowed on this object.");
		if (newParent->GetHierarchy()->childrenCount >= MAX_CHILDREN)
		{
			printf("Parent object already has max amount of children.");
			return false;
		}

		if (parent != nullptr)
		{
			for (int i = 0; i < MAX_CHILDREN; i++)
			{
				if ((entt::entity)parent->GetHierarchy()->children[i] == (entt::entity)*_entity)
				{
					parent->GetHierarchy()->children[i] = Entity();
					break;
				}
			}
			parent->GetHierarchy()->childrenCount--;

		}

		parent = newParent;
		if (parent)
		{
			for (int i = 0; i < MAX_CHILDREN; i++)
			{
				if (!parent->GetHierarchy()->children[i])
				{
					parent->GetHierarchy()->children[i] = *_entity;
					break;
				}
			}
			parent->GetHierarchy()->childrenCount++;
		}

		return true;
	}

	bool AddChild(Entity* newChild)
	{
		if (childrenCount >= MAX_CHILDREN)
		{
			printf("Parent object already has max amount of children.");
			return false;
		}

		if (!newChild->GetHierarchy()->UpdateParent(_entity))
			return false;

		for (int i = 0; i < MAX_CHILDREN; i++)
			if (!children[i])
				children[i] = *_entity;
		childrenCount++;

		return true;
	}

	// For removing child, just update its parent
};
