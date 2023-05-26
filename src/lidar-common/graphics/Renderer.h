#pragma once

#include <EngineFactoryVk.h>
#include <RefCntAutoPtr.hpp>

class Renderer
{
	Renderer() { }
	~Renderer() { }

public:
	static Renderer* Instance()
	{
		static Renderer instance;
		return &instance;
	}

	bool Initialize()
	{
		Diligent::EngineVkCreateInfo engineCI;
		m_factory = Diligent::GetEngineFactoryVk();
		m_factory->CreateDeviceAndContextsVk(engineCI, &m_renderDevice, &m_deviceContext);
		return true;
	}

	void DeInitialize()
	{
		m_renderDevice.Release();
		m_deviceContext.Release();
		m_factory.Release();
	}

	const auto& GetFactory() const { return m_factory; }
	auto& GetFactory() { return m_factory; }

	const auto& GetRenderDevice() const { return m_renderDevice; }
	auto& GetRenderDevice() { return m_renderDevice; }

	const auto& GetDeviceContext() const { return m_deviceContext; }
	auto& GetDeviceContext() { return m_deviceContext; }

	void UpdateBuffer(Diligent::IBuffer* buffer, void* data, size_t dataSize, Diligent::MAP_FLAGS flags = Diligent::MAP_FLAG_DISCARD)
	{
		void* mapped;
		m_deviceContext->MapBuffer(buffer, Diligent::MAP_WRITE, flags, mapped);
		memcpy(mapped, data, dataSize);
		m_deviceContext->UnmapBuffer(buffer, Diligent::MAP_WRITE);
	}

	void CreateSwapchain(const Diligent::SwapChainDesc& swapChainDesc, const Diligent::NativeWindow& window, Diligent::ISwapChain** swapchain)
	{
		m_factory->CreateSwapChainVk(m_renderDevice, m_deviceContext, swapChainDesc, window, swapchain);
	}

private:
	Diligent::RefCntAutoPtr<Diligent::IEngineFactoryVk> m_factory;
	Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_renderDevice;
	Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_deviceContext;
};

#define SRenderer Renderer::Instance()
#define SRendererContext SRenderer->GetDeviceContext()
#define SRendererDevice SRenderer->GetRenderDevice()
