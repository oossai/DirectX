#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "../Headers.h"
#include "../Macros.h"
#include "../Utility/Utility.h"
#include "../Models/Model.h"
#include "CommandQueue.h"
#include "SwapChain.h"

class Graphics
{
protected:
	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
	
	Microsoft::WRL::ComPtr<ID3D12Device3> m_device;

	CommandQueue m_commandQueue{ D3D12_COMMAND_LIST_TYPE_DIRECT };

	SwapChain m_swapChain;

	unsigned m_currentBackBufferIndex;
	
	uint64_t m_fenceValuesPerFrame[SwapChain::GetBackBufferCount()]{};
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap;

	bool m_initialized = false;
public:
	Graphics() = default;
	virtual ~Graphics();
	operator bool() { return m_initialized; }
	void OnInitialize(HWND hWnd);
	virtual void OnMouseEvent();
	virtual void OnKeyBoardEvent();
	virtual void OnUpdate(float time);
	virtual void OnRender(bool vSync);
	void OnResize(unsigned width, unsigned height, bool minimized);
private:
	NO_COPY_OR_MOVE(Graphics);
protected:
	void OnDestroy();
};

#endif