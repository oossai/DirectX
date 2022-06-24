/**
 * Wrapper class for D3D12 commandQueue
 */

#ifndef __COMMANDQUEUE_H__
#define __COMMANDQUEUE_H__

#include "../Headers.h"
#include "../Error.h"
#include "../Macros.h"

class CommandQueue
{
private:
	struct CommandAllocator
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		uint64_t fenceValue = 0;
	};

	D3D12_COMMAND_LIST_TYPE m_type;

	ID3D12Device* m_device = nullptr;
	
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	
	Microsoft::WRL::ComPtr<ID3D12Fence1> m_fence;
	
	uint64_t m_lastFenceValue = 0;
	
	HANDLE m_fenceEventHandle = nullptr;
	
	std::queue<CommandAllocator> m_commandAllocatorQueue;
	
	std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> m_commandListQueue;

	bool m_initialized = false;
public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE type) : m_type{type}{}
	~CommandQueue();
	operator bool() { return m_initialized; }
	ID3D12CommandQueue* Get() { return m_commandQueue.Get(); }
	uint64_t GetLastFenceValue() { return m_lastFenceValue; }
	void OnInitialize(ID3D12Device* device);
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	uint64_t ExecuteCommandList(ID3D12GraphicsCommandList* commandList);
	void MoveToNextFrame(uint64_t fenceValueToWaitOn);
	void WaitForGPU();
private:
	NO_COPY_OR_MOVE(CommandQueue);
	bool IsFenceValueReached(uint64_t fencevalue);
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12CommandAllocator* commandAllocator);
};

#endif