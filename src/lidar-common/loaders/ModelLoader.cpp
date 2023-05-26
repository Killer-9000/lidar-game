#include "ModelLoader.h"

#include <components/Hierarchy.h>
#include <components/Model.h>
#include <data/Archive/ArchiveMgr.h>
#include <util/AssimpArchiveIOHandler.h>
#include <views/WorldView.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/Logger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

struct UploadedMeshData
{
	aiAABB boundingBox;
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;
};

void RecursiveNodeChildren(WorldView* worldView, aiScene* scene, UploadedMeshData* meshData, Entity parent, aiNode** nodes, uint32_t nodeCount)
{
	for (uint32_t i = 0; i < nodeCount; i++)
	{
		Entity entity = worldView->CreateEntity();
		entity.GetHierarchy()->UpdateParent(&parent);
		entity.GetHierarchy()->parentLocked = true;
		entity.SetName(nodes[i]->mName.C_Str());

		if (nodes[i]->mNumMeshes)
		{
			Model& model = entity.AddComponent<Model>();

			// Load node
			model.meshes.resize(nodes[i]->mNumMeshes);
			for (uint32_t j = 0; j < nodes[i]->mNumMeshes; j++)
			{
				model.meshes[j].boundingBox = meshData[j].boundingBox;
				model.meshes[j].vertexOffset = meshData[j].vertexOffset;
				model.meshes[j].indexOffset = meshData[j].indexOffset;
				model.meshes[j].indexCount = meshData[j].indexCount;
			}

			model.modelMatrix = nodes[i]->mTransformation;
		}

		RecursiveNodeChildren(worldView, scene, meshData, entity, nodes[i]->mChildren, nodes[i]->mNumChildren);
	}
}

bool ModelLoader::Load(WorldView* worldView, Model* model, const std::string& filename)
{
	// TODO: Check if file is already loaded, if so just use that offset.
	Assimp::DefaultLogger::create("Assimp.log", Assimp::Logger::VERBOSE, aiDefaultLogStream_FILE | aiDefaultLogStream_STDOUT);

	Assimp::Importer importer;
	importer.SetIOHandler(new AssimpArchiveIOHandler(std::filesystem::path(filename).parent_path().string()));
	const aiScene* scene = importer.ReadFile(std::filesystem::path(filename).filename().string(),
		aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality | aiProcess_SplitLargeMeshes | aiProcess_FindInstances |
		aiProcess_GenBoundingBoxes | aiProcess_MakeLeftHanded);
	if (!scene)
	{
		printf("Failed to read model file '%s'.\n", filename.c_str());
		Assimp::DefaultLogger::kill();
		return false;
	}

	{
		model->_entity->SetName(std::filesystem::path(filename).filename().replace_extension("").string());

		uint32_t meshCount = scene->mNumMeshes;
		aiMesh** meshes = scene->mMeshes;

		std::lock_guard _(worldView->VertexBuffer.m_mutex);
		std::lock_guard _1(worldView->NormalBuffer.m_mutex);
		std::lock_guard _2(worldView->IndexBuffer.m_mutex);

		worldView->VertexBuffer.MapBuffer();
		worldView->NormalBuffer.MapBuffer();
		worldView->IndexBuffer.MapBuffer();

		model->boundingBox.mMin = aiVector3D(FLT_MAX);
		model->boundingBox.mMax = aiVector3D(-FLT_MAX);

		UploadedMeshData* meshData = new UploadedMeshData[meshCount];

		// Load in meshes
		for (uint32_t i = 0; i < meshCount; i++)
		{
			meshData[i].vertexOffset = worldView->VertexBuffer.m_bufferWritePoint / sizeof(glm::vec3);
			meshData[i].indexOffset = worldView->IndexBuffer.m_bufferWritePoint / sizeof(uint32_t);

			aiAABB boundBox = meshes[i]->mAABB;
			meshData[i].boundingBox = boundBox;

			if (boundBox.mMin.x < model->boundingBox.mMin.x)
				model->boundingBox.mMin.x = boundBox.mMin.x;
			if (boundBox.mMin.y < model->boundingBox.mMin.y)
				model->boundingBox.mMin.y = boundBox.mMin.y;
			if (boundBox.mMin.z < model->boundingBox.mMin.z)
				model->boundingBox.mMin.z = boundBox.mMin.z;

			if (boundBox.mMax.x > model->boundingBox.mMax.x)
				model->boundingBox.mMax.x = boundBox.mMax.x;
			if (boundBox.mMax.y > model->boundingBox.mMax.y)
				model->boundingBox.mMax.y = boundBox.mMax.y;
			if (boundBox.mMax.z > model->boundingBox.mMax.z)
				model->boundingBox.mMax.z = boundBox.mMax.z;

			worldView->VertexBuffer.Insert((glm::vec3*)meshes[i]->mVertices, meshes[i]->mNumVertices);
			worldView->NormalBuffer.Insert((glm::vec3*)meshes[i]->mNormals, meshes[i]->mNumVertices);

			meshData[i].indexCount = 0;
			for (uint32_t j = 0; j < meshes[i]->mNumFaces; j++)
			{
				aiFace* face = &meshes[i]->mFaces[j];
				worldView->IndexBuffer.Insert(face->mIndices, face->mNumIndices);
				meshData[i].indexCount += face->mNumIndices;
			}
		}

		// Load in nodes.
		RecursiveNodeChildren(worldView, (aiScene*)scene, meshData, *model->_entity, (aiNode**)&scene->mRootNode, 1);

		delete[] meshData;

		worldView->IndexBuffer.UnmapBuffer();
		worldView->NormalBuffer.UnmapBuffer();
		worldView->VertexBuffer.UnmapBuffer();
	}

	Hierarchy::RecurseChildren(model->_entity, [](const Entity* entity) -> bool {
		if (entity->HasComponent<Model>())
			entity->GetComponent<Model>().uploaded = true;
		return true;
	});

	Assimp::DefaultLogger::kill();

	return true;
}
