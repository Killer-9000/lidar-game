#pragma once

#include <components/Model.h>
#include <graphics/UnifiedBuffer.h>

#include <PipelineState.h>
#include <RefCntAutoPtr.hpp>
#include <BasicMath.hpp>

#include <glm/glm.hpp>

class WorldView;

struct IndirectDrawArgs
{
	uint32_t NumVertices;
	uint32_t NumInstances;
	uint32_t StartVertexLocation;
	uint32_t FirstInstanceLocation;
};

struct IndirectIndexedDrawArgs
{
	uint32_t NumIndices;
	uint32_t NumInstances;
	uint32_t FirstIndexLocation;
	uint32_t BaseVertex;
	uint32_t FirstInstanceLocation;
};

struct InstanceConstants
{
	Diligent::float4x4 modelMatrix;
	glm::vec3 modelColour;
};

class WorldRenderer
{
public:
	void Upload(WorldView* view);
	void Unload();
	void Render(WorldView* view);

private:
	Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_modelPipeline;
	UnifiedBuffer<IndirectIndexedDrawArgs> m_indirectDrawBuffer = UnifiedBuffer<IndirectIndexedDrawArgs>(1024, Diligent::BIND_INDIRECT_DRAW_ARGS);
	UnifiedBuffer<InstanceConstants> m_instanceConstants = UnifiedBuffer<InstanceConstants>(1024, Diligent::BIND_VERTEX_BUFFER);
};
