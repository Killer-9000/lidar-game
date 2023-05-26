#pragma once

#include "IViewport.h"
#include <graphics/UnifiedBuffer.h>
#include "loaders/AsyncLoader.h"
#include <loaders/SceneLoader.h>

#include <Buffer.h>
#include <fmt/printf.h>
#include <RefCntAutoPtr.hpp>

// Handles view of an open world within a map.
class WorldView : public IViewport
{
public:
	WorldView() { }
	~WorldView() { }

	void UnloadScene()
	{
		// Reset camera
		CameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
		CameraRot = glm::vec3(0.0f, 0.0f, 0.0f);
		CameraSpeed = 1.0f;
	}

	bool LoadScene(std::string filepath)
	{
		UnloadScene();
		return SceneLoader::Load(this, filepath);
	}

	bool SaveScene(std::string filepath = "")
	{
		if (filepath == "") filepath = m_sceneFilename;

		return SceneLoader::Save(this, filepath);
	}

	bool IsSceneFromFile() { return m_sceneFilename != ""; }

	glm::vec3 GetCameraForward()
	{
		glm::vec3 direction;
		direction.x = cos(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		direction.y = sin(glm::radians(CameraRot.x));
		direction.z = sin(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		return glm::normalize(direction);
	}
	glm::vec3 GetCameraRight()
	{
		return glm::cross(GetCameraForward(), glm::vec3(0, 1, 0));
	}

	glm::vec3 CameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 CameraRot = glm::vec3(0.0f, 0.0f, 0.0f);
	float CameraSpeed = 1.0f;
	glm::mat4 CameraMatrix = glm::mat4(1.0f);
	glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraVPMatrix;

	UnifiedBuffer<glm::vec3> VertexBuffer = UnifiedBuffer<glm::vec3>(128 * 1024 * 1024, Diligent::BIND_VERTEX_BUFFER);
	UnifiedBuffer<glm::vec3> NormalBuffer = UnifiedBuffer<glm::vec3>(128 * 1024 * 1024, Diligent::BIND_VERTEX_BUFFER);
	UnifiedBuffer<uint32_t> IndexBuffer = UnifiedBuffer<uint32_t>(128 * 1024 * 1024, Diligent::BIND_INDEX_BUFFER);

private:
	std::string m_sceneFilename = "";
};
