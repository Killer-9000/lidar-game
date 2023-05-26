#include "MainWindow.h"

#include <components/Model.h>
#include <components/Name.h>
#include <components/Hierarchy.h>
#include <data/Archive/ArchiveMgr.h>
#include "graphics/renderers/WorldRenderer.h"
#include "graphics/Renderer.h"
#include <graphics/imgui_extensions/ImFileDialog.h>
#include <loaders/ModelLoader.h>

#include <fmt/printf.h>
#include <graphics/WindowMgr.h>
#include <glm/gtx/matrix_major_storage.hpp>

bool MainWindow::ProcessSDLEvent(const SDL_Event& event)
{
	if (!IWindow::ProcessSDLEvent(event))
		return false;

	switch (event.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		m_mouseDown = true;
		m_mousePoint = { event.button.x, event.button.y };
		m_mouseDiff = { 0.0f, 0.0f };
		break;
	case SDL_MOUSEBUTTONUP:
		m_mouseDiff = { 0.0f, 0.0f };
		m_mouseDown = false;
		break;
	case SDL_MOUSEMOTION:
	{
		// If mouseDown and Window selected, and mouse in window.
		bool inBounds = true;
		inBounds &= ImGui::GetMousePos().x > m_renderViewportPos.x + 4 && ImGui::GetMousePos().y > m_renderViewportPos.y + 4;
		inBounds &= ImGui::GetMousePos().x < m_renderViewportPos.x + 4 + m_renderViewportSize.x - 4 && ImGui::GetMousePos().y < m_renderViewportPos.y + 4 + m_renderViewportSize.y - 4;
		inBounds &= m_renderViewportFocused;
		if (m_mouseDown && inBounds)
		{
			m_mouseDiff.x = event.button.x - m_mousePoint.x;
			m_mouseDiff.y = m_mousePoint.y - event.button.y;
			m_mousePoint = { event.button.x, event.button.y };
			m_worldView->CameraRot.x += m_mouseDiff.y / m_frameDelta;
			m_worldView->CameraRot.y += m_mouseDiff.x / m_frameDelta;
			m_worldView->CameraRot.x = std::clamp(m_worldView->CameraRot.x, -89.9f, 89.9f);
			if (m_worldView->CameraRot.y > 180.5f) m_worldView->CameraRot.y = -179.5f;
			else if (m_worldView->CameraRot.y < -180.5f) m_worldView->CameraRot.y = 179.5f;

			if (isinf(m_worldView->CameraRot.x) || isnan(m_worldView->CameraRot.x))
				m_worldView->CameraRot.x = 0;
			if (isinf(m_worldView->CameraRot.y) || isnan(m_worldView->CameraRot.y))
				m_worldView->CameraRot.y = 0;
		}
	} break;
	case SDL_MOUSEWHEEL:
	{
		if (m_renderViewportFocused)
		{
			m_worldView->CameraSpeed += (float)event.wheel.y / m_frameDelta;
			m_worldView->CameraSpeed = std::clamp(m_worldView->CameraSpeed, 1.0f, 1000.0f);
		}
	} break;
	case SDL_WINDOWEVENT:
	{
		if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			OnWindowResize();
	} break;
	}
	return true;
}

bool MainWindow::Render()
{
	static bool openWindow = false;
	if (!StartRender())
		return true;

	ImGui::DockSpaceOverViewport();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Load Scene"))
				SFILE_DIALOG->Open("mainwindow_load_scene", "Open Scene file", "*.scene");
			else if (ImGui::MenuItem("Save Scene"))
			{
				if (m_worldView->IsSceneFromFile())
					m_worldView->SaveScene();
				else
					SFILE_DIALOG->Save("mainwindow_save_scene", "Save Scene file", "*.scene");
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		m_renderViewportFocused = m_imguiContext->NavWindow == ImGui::GetCurrentWindowRead();
		ImVec2 viewport = ImGui::GetContentRegionAvail();
		ImVec2 padding = ImGui::GetCurrentWindow()->WindowPadding;
		float x = ImGui::GetCursorPosX() - padding.x;
		float y = ImGui::GetCursorPosY() - padding.y;
		ImGui::SetCursorPos({ x, y });
		m_renderViewportPos = (glm::vec2)ImGui::GetCurrentWindow()->Pos + glm::vec2{ x, y };
		m_renderViewportSize = { viewport.x + (padding.x * 2), viewport.y + (padding.y * 2) };
		ImGui::Image(m_renderTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE), { m_renderViewportSize.x, m_renderViewportSize.y });

		if (y == 0.0f)
		{
			x += ImGui::GetWindowPos().x;
			y += ImGui::GetWindowPos().y;
			ImU32 col;
			if (ImGui::GetFocusID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
			else if (ImGui::GetHoveredID() == ImGui::GetID("#UNHIDE"))
				col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			else
				col = ImGui::GetColorU32(ImGuiCol_Button);
			ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled({ x, y }, { x + 12, y }, { x, y + 12 }, col);
		}

		ImGui::End();
	}

	if (ImGui::Begin("Debug"))
	{
		ImGui::Text("FPS: %f", 1000.0f / m_frameDelta);
		ImGui::Text("Mouse Diff:   %.1f %.1f", m_mouseDiff.x, m_mouseDiff.y);
		ImGui::Text("Camera pos:   %.3f %.3f %.3f", m_worldView->CameraPos.x, m_worldView->CameraPos.y, m_worldView->CameraPos.z);
		ImGui::Text("Camera rot:   %.3f %.3f %.3f", m_worldView->CameraRot.x, m_worldView->CameraRot.y, m_worldView->CameraRot.z);
		ImGui::Text("Camera speed: %.3f", m_worldView->CameraSpeed);

		if (ImGui::Button("Open test window"))
			openWindow = true;

		ImGui::End();
	}

	if (ImGui::Begin("Item Draw"))
	{
		if (ImGui::Button("Load Model"))
			SFILE_DIALOG->Open("mainwindow_load_model", "Load model file", "*.blend,*.3ds,*.obj,*.ply,*.dxf,*.mdl,*.q3o,*.q3s,*.stl,*.dxf,*.mesh.xml,*.glTF");

		if (ImGui::Button("Clear models"))
			m_worldView->RemoveAllEntities();

		ImGui::End();
	}

	if (ImGui::Begin("Scene Hierarchy"))
	{
		auto hierarchyView = m_worldView->GetRegistry().view<Hierarchy>();
		for (const auto& [entity, hierarchy] : hierarchyView.each())
		{
			if (hierarchy.parent == nullptr)
			{
				Hierarchy::RecurseChildren(hierarchy._entity,
					[this](const Entity* entity) -> bool {
						ImGuiTreeNodeFlags selected = m_selectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0;
						if (ImGui::TreeNodeEx(fmt::sprintf("##scene_hierarchy%i", (int)entity->InternalId()).c_str(), selected, "%s", entity->GetName().c_str()))
						{
							if (ImGui::IsItemClicked())
								m_selectedEntity = entity;
							return true;
						}
						return false;
					},
					[](const Entity* entity) {
						ImGui::TreePop();
				});
			}
		}
		ImGui::End();
	}

	if (SFILE_DIALOG->IsDone("mainwindow_load_scene"))
	{
		if (SFILE_DIALOG->HasResult())
		{
			const std::filesystem::path& path = SFILE_DIALOG->GetResult();
			if (std::filesystem::exists(path))
			{
				if (!m_worldView->LoadScene(path.string()))
					printf("Failed to load scene.\n");
			}
			else
			{
				printf("Unable to load scene, filepath doesn't exists.\n");
			}
		}
		SFILE_DIALOG->Close();
	}
	else if (SFILE_DIALOG->IsDone("mainwindow_save_scene"))
	{
		if (SFILE_DIALOG->HasResult())
		{
			const std::filesystem::path& path = SFILE_DIALOG->GetResult();
			m_worldView->SaveScene(path.string());
		}
		SFILE_DIALOG->Close();
	}
	else if (SFILE_DIALOG->IsDone("mainwindow_load_model"))
	{
		if (SFILE_DIALOG->HasResult())
		{
			const std::filesystem::path& path = SFILE_DIALOG->GetResult();
			if (std::filesystem::exists(path))
			{
				SAsyncLoader->AddWork([&]() {
					Entity entity = m_worldView->CreateEntity();
					Model& model = entity.AddComponent<Model>();
					if (!ModelLoader::Load(m_worldView.get(), &model, path.string()))
						printf("Failed to load scene.\n");
					});
			}
			else
			{
				printf("Unable to load model, filepath doesn't exists.\n");
			}
		}
		SFILE_DIALOG->Close();
	}

	EndRender();

	RenderWorld();

	if (openWindow)
	{
		openWindow = false;

		class TestWindow : public IWindow
		{
		public:
			TestWindow() : IWindow("Test Window", 720, 720)
			{ }

			virtual bool Render() override
			{
				if (!StartRender())
					return false;

				ImGui::DockSpaceOverViewport();

				ImGui::Begin("Test window");
				ImGui::End();

				EndRender();

				return true;
			}
		};

		SWindowMgr->AddWindow(new TestWindow());
	}

	return true;
}

bool MainWindow::Update()
{

	const bool* keyState = GetKeyboardState();

	// Viewport movement
	if (m_renderViewportFocused)
	{
		if (keyState[SDL_SCANCODE_W] && keyState[SDL_SCANCODE_S]) {}
		else if (keyState[SDL_SCANCODE_W])
		{
			glm::vec3 direction = m_worldView->GetCameraForward();
			m_worldView->CameraPos += direction * m_worldView->CameraSpeed / m_frameDelta;
		}
		else if (keyState[SDL_SCANCODE_S])
		{
			glm::vec3 direction = m_worldView->GetCameraForward();
			m_worldView->CameraPos -= direction * m_worldView->CameraSpeed / m_frameDelta;
		}

		if (keyState[SDL_SCANCODE_A] && keyState[SDL_SCANCODE_D]) {}
		else if (keyState[SDL_SCANCODE_A])
		{
			glm::vec3 direction = m_worldView->GetCameraRight();
			m_worldView->CameraPos -= direction * m_worldView->CameraSpeed / m_frameDelta;
		}
		else if (keyState[SDL_SCANCODE_D])
		{
			glm::vec3 direction = m_worldView->GetCameraRight();
			m_worldView->CameraPos += direction * m_worldView->CameraSpeed / m_frameDelta;
		}

		if (keyState[SDL_SCANCODE_Q] && keyState[SDL_SCANCODE_E]) {}
		else if (keyState[SDL_SCANCODE_Q])
		{
			m_worldView->CameraRot.y -= 0.1f * m_worldView->CameraSpeed / m_frameDelta;
		}
		else if (keyState[SDL_SCANCODE_E])
		{
			m_worldView->CameraRot.y += 0.1f * m_worldView->CameraSpeed / m_frameDelta;
		}

		if (keyState[SDL_SCANCODE_LCTRL] && keyState[SDL_SCANCODE_SPACE]) {}
		else if (keyState[SDL_SCANCODE_LCTRL])
		{
			m_worldView->CameraPos.y -= m_worldView->CameraSpeed / m_frameDelta;
		}
		else if (keyState[SDL_SCANCODE_SPACE])
		{
			m_worldView->CameraPos.y += m_worldView->CameraSpeed / m_frameDelta;
		}

		bool moving = false;
		moving |= keyState[SDL_SCANCODE_W] | keyState[SDL_SCANCODE_S];
		moving |= keyState[SDL_SCANCODE_A] | keyState[SDL_SCANCODE_D];
		moving |= keyState[SDL_SCANCODE_Q] | keyState[SDL_SCANCODE_E];
		moving |= keyState[SDL_SCANCODE_LCTRL] | keyState[SDL_SCANCODE_SPACE];
		if (moving)
		{
			if (isinf(m_worldView->CameraPos.x) || isnan(m_worldView->CameraPos.x))
				m_worldView->CameraPos.x = 0;
			if (isinf(m_worldView->CameraPos.y) || isnan(m_worldView->CameraPos.y))
				m_worldView->CameraPos.y = 0;
			if (isinf(m_worldView->CameraPos.z) || isnan(m_worldView->CameraPos.z))
				m_worldView->CameraPos.z = 0;
		}
	}

	return true;
}

void MainWindow::RenderWorld()
{
	Diligent::ITextureView* pRTV = m_renderTexture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
	Diligent::ITextureView* pDSV = m_depthTexture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
	SRendererContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	SRendererContext->ClearRenderTarget(pRTV, m_worldClearColour, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	SRendererContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	
	if (m_shadersCreated)
	{
		Diligent::Viewport vp;
		vp.Width = m_windowWidth;
		vp.Height = m_windowHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = vp.TopLeftY = 0.0f;
		SRendererContext->SetViewports(1, &vp, m_windowWidth, m_windowHeight);

		Diligent::Rect sr;
		sr.left = 0;
		sr.top = 0;
		sr.right = m_windowWidth;
		sr.bottom = m_windowHeight;
		SRendererContext->SetScissorRects(1, &sr, m_windowWidth, m_windowHeight);

		UpdateCameraVPMatrix();

		//// Draw world grid.
		if (m_worldGrid.pipeline)
		{
			SRendererContext->SetPipelineState(m_worldGrid.pipeline);

			Diligent::IBuffer* vertexBuffers[] = { m_worldGrid.vertexBuffer };
			uint64_t vertexOffsets[] = { 0 };
			SRendererContext->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			SRendererContext->SetIndexBuffer(m_worldGrid.indexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			SRendererContext->CommitShaderResources(m_worldGrid.shaderResources, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			SRendererContext->DrawIndexed(Diligent::DrawIndexedAttribs{ 6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL });
		}
		
		m_worldRenderer.Render(m_worldView.get());
	}
}

void MainWindow::OnWindowResize()
{
	// Create render image
	m_renderTexture.Release();
	m_depthTexture.Release();

	Diligent::TextureDesc textureDesc;
	textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
	textureDesc.Width = m_windowWidth;
	textureDesc.Height = m_windowHeight;
	textureDesc.Format = m_swapchain->GetDesc().ColorBufferFormat;
	textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
	textureDesc.Usage = Diligent::USAGE_DEFAULT;

	SRenderer->GetRenderDevice()->CreateTexture(textureDesc, nullptr, &m_renderTexture);

	textureDesc.Format = m_swapchain->GetDesc().DepthBufferFormat;
	textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
	SRenderer->GetRenderDevice()->CreateTexture(textureDesc, nullptr, &m_depthTexture);
	IM_ASSERT((m_renderTexture && m_depthTexture) && "Failed to create render or depth textures.");
}

void MainWindow::UpdateCameraVPMatrix()
{
	m_worldView->CameraMatrix = glm::lookAt(
		m_worldView->CameraPos,
		m_worldView->CameraPos + m_worldView->GetCameraForward(),
		glm::vec3(0, 1, 0)
	);

	m_worldView->ProjectionMatrix = glm::perspective(glm::radians(65.0f), m_renderViewportSize.x / m_renderViewportSize.y, 0.25f, 10000.0f);

	struct
	{
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::vec3 cameraPos;
	} data;

	data.projMatrix = glm::transpose(m_worldView->ProjectionMatrix);
	data.viewMatrix = glm::transpose(m_worldView->CameraMatrix);
	data.cameraPos = m_worldView->CameraPos;
	SRenderer->UpdateBuffer(m_worldView->CameraVPMatrix, &data, sizeof(data));
}

void MainWindow::CreateWorldGridShader()
{
	Diligent::GraphicsPipelineStateCreateInfo pipelineStateCI;
	pipelineStateCI.PSODesc.Name = "World Grid Pipeline";
	pipelineStateCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

	{
		uint8_t* shaderData; uint32_t shaderSize;
		if (!SArchiveMgr->OpenFile("shaders/WorldGrid.hlsl", &shaderData, &shaderSize))
			assert(false && "Failed to open WorldGrid shader.");

		Diligent::ShaderCreateInfo shaderCI;
		shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		shaderCI.Source = (Diligent::Char*)shaderData;
		shaderCI.SourceLength = shaderSize;

		shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
		shaderCI.Desc.Name = "World Grid Vertex";
		shaderCI.EntryPoint = "VSMain";

		SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pVS);
		if (!pipelineStateCI.pVS)
			assert(false && "Failed to create WorldGrid Vertex shader.");

		shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
		shaderCI.Desc.Name = "World Grid Pixel";
		shaderCI.EntryPoint = "PSMain";

		SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pPS);
		if (!pipelineStateCI.pPS)
			assert(false && "Failed to create WorldGrid Pixel shader.");

		free(shaderData);
	}

	pipelineStateCI.GraphicsPipeline.NumRenderTargets = 1;
	pipelineStateCI.GraphicsPipeline.RTVFormats[0] = m_swapchain->GetDesc().ColorBufferFormat;
	pipelineStateCI.GraphicsPipeline.DSVFormat = m_swapchain->GetDesc().DepthBufferFormat;
	pipelineStateCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Blend description
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = Diligent::BLEND_OPERATION_ADD;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
	pipelineStateCI.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

	// Rasterizer description
	pipelineStateCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;

	// Depth-Stencil description
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.DepthFunc = Diligent::COMPARISON_FUNC_ALWAYS;
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFailOp = Diligent::STENCIL_OP_KEEP;
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = Diligent::STENCIL_OP_KEEP;
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
	pipelineStateCI.GraphicsPipeline.DepthStencilDesc.BackFace = pipelineStateCI.GraphicsPipeline.DepthStencilDesc.FrontFace;

	// Pipeline inputs layout
	Diligent::LayoutElement layoutElements[2];

	pipelineStateCI.GraphicsPipeline.InputLayout.NumElements = 2;
	pipelineStateCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;
	layoutElements[0].InputIndex = 0;
	layoutElements[0].NumComponents = 3;
	layoutElements[0].ValueType = Diligent::VT_FLOAT32;
	layoutElements[1].InputIndex = 1;
	layoutElements[1].NumComponents = 2;
	layoutElements[1].ValueType = Diligent::VT_FLOAT32;

	pipelineStateCI.PSODesc.ResourceLayout.NumVariables = 0;

	SRendererDevice->CreateGraphicsPipelineState(pipelineStateCI, &m_worldGrid.pipeline);
	pipelineStateCI.pVS->Release();
	pipelineStateCI.pPS->Release();

	if (!m_worldGrid.pipeline)
		assert(false && "Failed to create WorldGrid pipeline.");

	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		bufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
		bufferDesc.MiscFlags = Diligent::MISC_BUFFER_FLAG_NONE;

		{
			struct VertexData
			{
				glm::vec3 pos;
				glm::vec2 uv;
			};
			VertexData vertexData[] =
			{
				{ {-1, 0,-1 }, { 0, 0 } },
				{ { 1, 0,-1 }, { 1, 0 } },
				{ {-1, 0, 1 }, { 0, 1 } },
				{ { 1, 0, 1 }, { 1, 1 } }
			};
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.Size = 4 * sizeof(VertexData);
			bufferDesc.ElementByteStride = sizeof(VertexData);

			Diligent::BufferData bufferData;
			bufferData.pContext = SRendererContext;
			bufferData.DataSize = 4 * sizeof(VertexData);
			bufferData.pData = vertexData;

			SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &m_worldGrid.vertexBuffer);

			if (!m_worldGrid.vertexBuffer)
				assert(false && "Failed to create WorldGrid vertex buffer.");
		}

		{
			uint32_t indexData[] =
			{
				0, 1, 2,
				2, 3, 1
			};
			bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			bufferDesc.Size = 6 * sizeof(uint32_t);
			bufferDesc.ElementByteStride = sizeof(uint32_t);

			Diligent::BufferData bufferData;
			bufferData.pContext = SRendererContext;
			bufferData.DataSize = 6 * sizeof(uint32_t);
			bufferData.pData = indexData;

			SRendererDevice->CreateBuffer(bufferDesc, &bufferData, &m_worldGrid.indexBuffer);
			if (!m_worldGrid.indexBuffer)
				assert(false && "Failed to create WorldGrid index buffer.");
		}

	}

	// Frame Constant Buffer
	{
		Diligent::BufferDesc mvpConstantDesc;
		mvpConstantDesc.Name = "Frame Constant Buffer";
		mvpConstantDesc.Size = sizeof(glm::mat4) * 2 + sizeof(glm::vec3);
		mvpConstantDesc.Usage = Diligent::USAGE_DYNAMIC;
		mvpConstantDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		mvpConstantDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		SRendererDevice->CreateBuffer(mvpConstantDesc, nullptr, &m_worldView->CameraVPMatrix);
		UpdateCameraVPMatrix();

		m_worldGrid.pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "FrameConstants")->Set(m_worldView->CameraVPMatrix);
	}

	// Create shader resource.
	m_worldGrid.pipeline->CreateShaderResourceBinding(&m_worldGrid.shaderResources, true);
	if (!m_worldGrid.shaderResources)
		assert(false && "Failed to create WorldGrid shader resource.");
}
