#include "SwapChain.h"

using Microsoft::WRL::ComPtr;

void SwapChain::OnInitialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND window)
{
	assert(device && commandQueue);

	m_device = device;

	unsigned dxgiFactoryFlags = 0;
#ifdef _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	ComPtr<IDXGIFactory6> dxgiFactory;
	THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, 
		IID_PPV_ARGS(&dxgiFactory)));

	BOOL allowTearing{};
	THROW_IF_FAILED(dxgiFactory->CheckFeatureSupport(
		DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, 
		static_cast<unsigned>(sizeof(BOOL))));
	m_allowTearing = allowTearing;

	RECT windowRect{};
	GetClientRect(window, &windowRect);
	m_width = windowRect.right - windowRect.left;
	m_height = windowRect.bottom - windowRect.top;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = {1, 0};
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_backBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain;
	THROW_IF_FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue, 
	window, &swapChainDesc, nullptr, nullptr, swapChain.ReleaseAndGetAddressOf()));
	THROW_IF_FAILED(swapChain.As(&m_swapChain));

	// Disable dxgi automatic fullscreen switch on ALT + ENTER 
	THROW_IF_FAILED(dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	CreateDescriptorHeap(m_device, m_backBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_rtvHeap.ReleaseAndGetAddressOf());
	
	CreateDescriptorHeap(m_device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_dsvHeap.ReleaseAndGetAddressOf());

	m_rtvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_dsvSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	LoadSizeDependentResources();

	m_initialized = true;
}

void SwapChain::LoadSizeDependentResources()
{
	// depthbuffer is a resource used by the swapchain
	// it's not created automatically
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, 
			m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	auto optClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0F, 0U);

	THROW_IF_FAILED(m_device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optClearValue,
        IID_PPV_ARGS(m_depthBuffer.ReleaseAndGetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	
	m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, dsvHandle);

	// create rtv
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (unsigned i = 0; i < m_backBufferCount; i++)
	{
		THROW_IF_FAILED(m_swapChain->GetBuffer(i, 
			IID_PPV_ARGS(m_backBuffers[i].ReleaseAndGetAddressOf())));

		m_device->CreateRenderTargetView(m_backBuffers[i].Get(),
			&rtvDesc, rtvHandle);

		rtvHandle.Offset(1, m_rtvSize);
	}

	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width = (float)m_width;
	m_viewPort.Height = (float)m_height;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = m_width;
	m_scissorRect.bottom = m_height;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::CurrentDepthStencilView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::CurrentRenderTargetView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIndex, m_rtvSize);
}

void SwapChain::OnResize(unsigned width, unsigned height)
{	
	m_width = std::max(1u, width);
	m_height = std::max(1u, height);

	for (size_t i = 0; i < m_backBufferCount; i++)
	{
		m_backBuffers[i].Reset();
	}
			
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	THROW_IF_FAILED(m_swapChain->GetDesc1(&swapChainDesc));

	THROW_IF_FAILED(m_swapChain->ResizeBuffers(m_backBufferCount, m_width, 
		m_height, swapChainDesc.Format, swapChainDesc.Flags));
		
	// swapchain backbuffer indexing maybe arbitrary
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		
	// reload any resource that depend on the swapchain backbuffer dimensions
	LoadSizeDependentResources();
}

unsigned SwapChain::Present(bool vSync)
{
	unsigned syncInterval = vSync ? 1 : 0; 
	LONG PresentFlags = m_allowTearing && !vSync ? 
		DXGI_PRESENT_ALLOW_TEARING : 0;

	THROW_IF_FAILED(m_swapChain->Present(syncInterval, PresentFlags));
	
	// flip
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	return m_currentBackBufferIndex;
}