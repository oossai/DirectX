#include "Graphics.h"
#include "../Error.h"

using Microsoft::WRL::ComPtr;

Graphics::~Graphics()
{
	OnDestroy();
}

void Graphics::OnInitialize(HWND hWnd)
{
	// enable debug layer
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif
	
	// get suitable adapter
	unsigned dxgiFactoryFlags = 0;
#ifdef _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	ComPtr<IDXGIFactory6> dxgiFactory;
	THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, 
		IID_PPV_ARGS(&dxgiFactory)));
	
	DXGI_ADAPTER_DESC1 adapterDesc{};

	if (false)
	{
		dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&m_adapter));
		m_adapter->GetDesc1(&adapterDesc);
	}
	else
	{
		for (unsigned i = 0;
			SUCCEEDED(dxgiFactory->EnumAdapterByGpuPreference(i, 
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));
			i++)
		{
			m_adapter->GetDesc1(&adapterDesc);
			if (adapterDesc.Flags == DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), 
				D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), NULL)))
			{
				break;
			}
		}
	}

	assert(m_adapter);
	OutputDebugStringW(adapterDesc.Description);

	// create device
	THROW_IF_FAILED(D3D12CreateDevice(m_adapter.Get(), 
		D3D_FEATURE_LEVEL_11_0, 
		IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())));
	
	// setup debug info queue
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(m_device.As(&infoQueue)))
	{
		THROW_IF_FAILED(infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_CORRUPTION, true));

		THROW_IF_FAILED(infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_WARNING, true));

		THROW_IF_FAILED(infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_INFO, true));
	}
#endif

	// create command and sync objects
	m_commandQueue.OnInitialize(m_device.Get());
	
	// create swapchain
	m_swapChain.OnInitialize(m_device.Get(), m_commandQueue.Get(), hWnd);

	// create constant buffer descriptor heaps
	CreateDescriptorHeap(m_device.Get(), 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_cbvHeap.ReleaseAndGetAddressOf());

	m_initialized = true;
}

void Graphics::OnResize(unsigned width, unsigned height, bool minimized)
{
	if (m_swapChain.DimensionsChanged(width, height) && !minimized)
	{
		m_commandQueue.WaitForGPU();
		m_swapChain.OnResize(width, height);
		for (size_t i = 0; i < SwapChain::GetBackBufferCount(); i++)
		{
			m_fenceValuesPerFrame[i] = m_commandQueue.GetLastFenceValue();
		}
	}
}

void Graphics::OnMouseEvent()
{}

void Graphics::OnKeyBoardEvent()
{}

void Graphics::OnUpdate(float time)
{}

void Graphics::OnRender(bool vSync)
{
	// get object
	auto commandList = m_commandQueue.GetCommandList();

	auto currentBackBuffer = m_swapChain.GetCurrentBackBuffer();

	m_currentBackBufferIndex = m_swapChain.GetCurrentBackBufferIndex();

	auto currentRTVHandle = m_swapChain.CurrentRenderTargetView();

	auto currentDSVHandle = m_swapChain.CurrentDepthStencilView();
	
	// barrier
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, 
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	
	commandList->ResourceBarrier(1, &barrier);
	
	//clear
	float clearColour[] {0.5f, 0.4f, 1.0f, 1.0f};

	commandList->ClearRenderTargetView(currentRTVHandle, clearColour, 0, nullptr);

	commandList->ClearDepthStencilView(currentDSVHandle, D3D12_CLEAR_FLAG_DEPTH | 
		D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	
	// barrier
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, 
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	commandList->ResourceBarrier(1, &barrier);

	// execute
	m_fenceValuesPerFrame[m_currentBackBufferIndex] = 
		m_commandQueue.ExecuteCommandList(commandList.Get());

	// present
	m_currentBackBufferIndex = m_swapChain.Present(vSync);

	// move to next frame
	m_commandQueue.MoveToNextFrame(m_fenceValuesPerFrame[m_currentBackBufferIndex]);
}

void Graphics::OnDestroy()
{
	if (m_commandQueue)
	{
		m_commandQueue.WaitForGPU();
	}
}