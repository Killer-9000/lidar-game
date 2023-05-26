#pragma once

#include "graphics/IWindow.h"
#include <graphics/renderers/WorldRenderer.h>
#include "loaders/AsyncLoader.h"
#include "views/WorldView.h"

#include <Texture.h>
#include <ShaderResourceBinding.h>

class MainWindow : public IWindow
{
public:
	MainWindow()
		: IWindow("Lidar Editor", 1600, 1200, SDL_WINDOW_RESIZABLE)
	{
		m_worldView = std::make_unique<WorldView>();
	}

	virtual bool Init() override
	{
		if (!IWindow::Init())
			return false;

		OnWindowResize();

		SAsyncLoader->AddWork([&]() {
			CreateWorldGridShader();
			m_worldRenderer.Upload(m_worldView.get());
			m_shadersCreated = true;
			});

		return true;
	}

	virtual void Deinit()
	{
		m_worldRenderer.Unload();
	}

	virtual bool ProcessSDLEvent(const SDL_Event & event) override;
	virtual bool Render() override;
	virtual bool Update() override;
	void RenderWorld();

	void OnWindowResize();

	void UpdateCameraVPMatrix();

private:
	void CreateWorldGridShader();

	Diligent::RefCntAutoPtr<Diligent::ITexture> m_renderTexture, m_depthTexture;
	glm::vec2 m_renderViewportPos, m_renderViewportSize;
	bool m_renderViewportFocused = false;
	std::unique_ptr<WorldView> m_worldView;
	float m_worldClearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	struct
	{
		Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shaderResources;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline;
	} m_worldGrid;

	bool m_shadersCreated = false;

	glm::vec2 m_mousePoint, m_mouseDiff = { 0.0f, 0.0f };
	bool m_mouseDown = false;
private:
	Diligent::RefCntAutoPtr<Diligent::ITexture> m_viewportRender, m_viewportDepth;
	glm::vec2 m_viewportPos, m_viewportSize;
	bool m_viewportFocused = false;
	float m_viewportClearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	WorldRenderer m_worldRenderer;

	const Entity* m_selectedEntity = nullptr;
};
