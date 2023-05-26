#pragma once

#include <graphics/Renderer.h>

#include <Buffer.h>
#include <RefCntAutoPtr.hpp>

#include <assert.h>
#include <mutex>
#include <vector>

template <typename Type>
class UnifiedBuffer
{
public:
	UnifiedBuffer() { }

	UnifiedBuffer(size_t startSize, Diligent::BIND_FLAGS bindFlags)
	{
		startSize *= sizeof(Type);
		assert(startSize % 16 == 0 && "Starting size of buffer is not divisible by 16!");
		m_bufferSize = startSize;

		Diligent::BufferDesc desc;
		desc.Usage = Diligent::USAGE_UNIFIED;
		desc.Size = startSize;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
		desc.ElementByteStride = sizeof(Type);

		SRendererDevice->CreateBuffer(desc, nullptr, &m_buffer);
	}

	~UnifiedBuffer()
	{
		UnmapBuffer();
	}

	void MapBuffer()
	{
		if (m_mappedBuffer)
			return;
		SRendererContext->MapBuffer(m_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NONE, m_mappedBuffer);
	}

	void UnmapBuffer()
	{
		if (!m_mappedBuffer)
			return;
		SRendererContext->UnmapBuffer(m_buffer, Diligent::MAP_WRITE);
		m_mappedBuffer = nullptr;
	}

	bool Insert(Type* data, size_t count)
	{
		assert(m_mappedBuffer && "Buffer isn't mapped");

		if (!m_buffer)
			return false;

		// Realloc if not enough space.
		if (m_bufferWritePoint + (count * sizeof(Type)) > m_bufferSize)
		{
			Diligent::BufferDesc desc;
			desc.Usage = Diligent::USAGE_UNIFIED;
			desc.Size = m_bufferSize + m_bufferSize / 2;
			desc.BindFlags = m_buffer->GetDesc().BindFlags;
			desc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			desc.ElementByteStride = sizeof(Type);

			// Create new buffer
			Diligent::RefCntAutoPtr<Diligent::IBuffer> temp;
			SRendererDevice->CreateBuffer(desc, nullptr, &temp);

			SRendererContext->CopyBuffer(m_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, temp, 0, m_bufferSize, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			SRendererContext->UnmapBuffer(m_buffer, Diligent::MAP_WRITE);

			m_buffer.Release();
			m_buffer = temp;

			SRendererContext->MapBuffer(m_buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NONE, m_mappedBuffer);

			// Increase size variable
			m_bufferSize += m_bufferSize / 2;
		}

		Type* section = (Type*)((uint8_t*)m_mappedBuffer + m_bufferWritePoint);

		memcpy(section, data, count * sizeof(Type));

		m_bufferWritePoint += count * sizeof(Type);

		return true;
	}

	void* m_mappedBuffer = nullptr;
	size_t m_bufferSize = 0;
	size_t m_bufferWritePoint = 0;
	std::mutex m_mutex;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> m_buffer;
};