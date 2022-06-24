#ifndef __SWAPCHAIN_H__
#define __SWAPCHAIN_H__

#include "../Headers.h"
#include "../Macros.h"
#include "../Error.h"
#include "../Utility/Utility.h"

class SwapChain
{
private:
	bool m_allowTearing = false;
	static inline const unsigned m_backBufferCount = 2;
	
	unsigned m_width = 0;
	unsigned m_height = 0;
	unsigned m_currentBackBufferIndex = 0;
	unsigned m_rtvSize = 0;
	unsigned m_dsvSize = 0;
	D3D12_VIEWPORT m_viewPort{};
	RECT m_scissorRect{};

	ID3D12Device* m_device = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[m_backBufferCount]{};

	bool m_initialized = false;
public:
	SwapChain() = default;
	
	~SwapChain() = default;

	operator bool() { return m_initialized; }
	
	void OnInitialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND window);
	
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentDepthStencilView();
	
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentRenderTargetView();
	
	static constexpr unsigned GetBackBufferCount() { return m_backBufferCount; }

	float GetAspectRatio() { return float(m_width) /  m_height; }

	D3D12_VIEWPORT GetViewPort() { return m_viewPort; }

	RECT GetScissorRect() { return m_scissorRect; }
	
	unsigned GetCurrentBackBufferIndex() { return m_swapChain->GetCurrentBackBufferIndex(); }
	
	ID3D12Resource* GetCurrentBackBuffer() { return m_backBuffers[m_currentBackBufferIndex].Get(); }
	
	bool DimensionsChanged(unsigned width, unsigned height){ return (m_width != width || m_height != height); }
	
	void OnResize(unsigned width, unsigned height);
	
	[[nodiscard("flip back buffer index")]] unsigned Present(bool vSync);
private:
	NO_COPY_OR_MOVE(SwapChain)
	void LoadSizeDependentResources();
};

#endif