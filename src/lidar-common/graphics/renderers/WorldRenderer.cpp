#include "WorldRenderer.h"
#include <data/Archive/ArchiveMgr.h>
#include "views/WorldView.h"

#include <glm/gtx/matrix_major_storage.hpp>

void WorldRenderer::Upload(WorldView* view)
{
	// Model Pipeline
	{
		Diligent::GraphicsPipelineStateCreateInfo pipelineStateCI;
		pipelineStateCI.PSODesc.Name = "SimpleModel Pipeline";
		pipelineStateCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

		{
			uint8_t* shaderData; uint32_t shaderSize;
			if (!SArchiveMgr->OpenFile("shaders/SimpleModel.hlsl", &shaderData, &shaderSize))
				assert(false && "Failed to open SimpleModel shader.");

			Diligent::ShaderCreateInfo shaderCI;
			shaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
			shaderCI.Source = (Diligent::Char*)shaderData;
			shaderCI.SourceLength = shaderSize;

			shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			shaderCI.Desc.Name = "SimpleModel Vertex";
			shaderCI.EntryPoint = "VSMain";

			SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pVS);
			if (!pipelineStateCI.pVS)
				assert(false && "Failed to create SimpleModel Vertex shader.");

			shaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			shaderCI.Desc.Name = "SimpleModel Pixel";
			shaderCI.EntryPoint = "PSMain";

			SRendererDevice->CreateShader(shaderCI, &pipelineStateCI.pPS);
			if (!pipelineStateCI.pPS)
				assert(false && "Failed to create SimpleModel Pixel shader.");

			free(shaderData);
		}

		// TODO: Get format from swapchain!
		pipelineStateCI.GraphicsPipeline.NumRenderTargets = 1;
		pipelineStateCI.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB;
		pipelineStateCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_D32_FLOAT;
		pipelineStateCI.GraphicsPipeline.RasterizerDesc.DepthClipEnable = false;

		// Pipeline inputs layout
		Diligent::LayoutElement layoutElements[7];
		layoutElements[0].BufferSlot = 0;
		layoutElements[0].InputIndex = 0;
		layoutElements[0].NumComponents = 3;
		layoutElements[0].ValueType = Diligent::VT_FLOAT32;

		layoutElements[1].BufferSlot = 1;
		layoutElements[1].InputIndex = 1;
		layoutElements[1].NumComponents = 3;
		layoutElements[1].ValueType = Diligent::VT_FLOAT32;

		layoutElements[2].BufferSlot = 2;
		layoutElements[2].InputIndex = 2;
		layoutElements[2].NumComponents = 3;
		layoutElements[2].ValueType = Diligent::VT_FLOAT32;
		layoutElements[2].Frequency = Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE;

		// 3 - 6
		for (int i = 0; i < 4; i++)
		{
			layoutElements[i + 3].BufferSlot = 2;
			layoutElements[i + 3].InputIndex = i + 3;
			layoutElements[i + 3].NumComponents = 4;
			layoutElements[i + 3].ValueType = Diligent::VT_FLOAT32;
			layoutElements[i + 3].Frequency = Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE;
		}

		pipelineStateCI.GraphicsPipeline.InputLayout.NumElements = 7;
		pipelineStateCI.GraphicsPipeline.InputLayout.LayoutElements = layoutElements;

		SRendererDevice->CreateGraphicsPipelineState(pipelineStateCI, &m_modelPipeline);
		pipelineStateCI.pVS->Release();
		pipelineStateCI.pPS->Release();

		if (!m_modelPipeline)
			assert(false && "Failed to create Model pipeline.");

		m_modelPipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "FrameConstants")->Set(view->CameraVPMatrix);
	}
}

void WorldRenderer::Unload()
{

}

void WorldRenderer::Render(WorldView* view)
{
	auto modelView = view->GetRegistry().view<Model>();

	SRendererContext->SetPipelineState(m_modelPipeline);

	IndirectIndexedDrawArgs args;
	m_indirectDrawBuffer.MapBuffer();
	m_indirectDrawBuffer.m_bufferWritePoint = 0;
	m_instanceConstants.MapBuffer();
	m_instanceConstants.m_bufferWritePoint = 0;
	uint32_t drawCount = 0;

	InstanceConstants instanceConstants;

	Diligent::float4x4 modelMatrix = Diligent::float4x4::Scale(1.0f) * Diligent::float4x4::Translation(0.0f, 1000.0f, 0.0f);

	for (const auto& [entity, model] : modelView.each())
	{
		if (!model.uploaded || !model.show)
			continue;

		for (const Model::Mesh& mesh : model.meshes)
		{
			args.NumIndices = mesh.indexCount;
			args.NumInstances = 1;
			args.FirstIndexLocation = mesh.indexOffset;
			args.BaseVertex = mesh.vertexOffset;
			args.FirstInstanceLocation = 0;
			m_indirectDrawBuffer.Insert(&args, 1);

			instanceConstants.modelColour = glm::vec3(1.0f, 0.0f, 0.0f);
			instanceConstants.modelMatrix = modelMatrix.Transpose();

			m_instanceConstants.Insert(&instanceConstants, 1);
			drawCount++;
		}
	}
	m_indirectDrawBuffer.UnmapBuffer();
	m_instanceConstants.UnmapBuffer();

	std::lock_guard _(view->VertexBuffer.m_mutex);
	std::lock_guard _1(view->NormalBuffer.m_mutex);
	std::lock_guard _2(view->IndexBuffer.m_mutex);

	Diligent::IBuffer* vertexBuffers[] = { view->VertexBuffer.m_buffer, view->NormalBuffer.m_buffer, m_instanceConstants.m_buffer };
	uint64_t vertexOffsets[] = { 0, 0, 0 };
	SRendererContext->SetVertexBuffers(0, 3, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	SRendererContext->SetIndexBuffer(view->IndexBuffer.m_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	SRendererContext->DrawIndexedIndirect(Diligent::DrawIndexedIndirectAttribs{ Diligent::VT_UINT32, m_indirectDrawBuffer.m_buffer, Diligent::DRAW_FLAG_VERIFY_ALL, drawCount });
}
