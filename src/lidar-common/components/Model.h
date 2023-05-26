#pragma once

#include <components/BaseComponent.h>

#include <assimp/aabb.h>
#include <assimp/matrix4x4.h>

#include <vector>

struct Model : public BaseComponent
{
	struct Mesh
	{
		uint64_t vertexOffset = 0;
		uint64_t indexOffset = 0;
		uint64_t indexCount = 0;
		aiAABB boundingBox;
	};

	Model() = delete;
	Model(Entity* entity) : BaseComponent(entity)
	{

	}

	aiAABB boundingBox;
	aiMatrix4x4 modelMatrix;
	std::vector<Mesh> meshes;

	bool uploaded = false;
	bool show = true;
};