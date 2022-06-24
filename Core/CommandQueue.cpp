#include "CommandQueue.h"

using Microsoft::WRL::ComPtr;

CommandQueue::~CommandQueue()
{
	if (m_fenceEventHandle)
	{
		CloseHandle(m_fenceEventHandle);
	}
}

void CommandQueue::OnInitialize(ID3D12Device* device)
{
	assert(device);

	m_device = device;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = m_type;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	THROW_IF_FAILED(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));

	THROW_IF_FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));

	m_fenceEventHandle = CreateEvent(nullptr, false, false, nullptr);

	assert(m_fence != 0 && "Failed to create fence event handle");

	m_initialized = true;
}

ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	
	THROW_IF_FAILED(m_device->CreateCommandAllocator(m_type, IID_PPV_ARGS(commandAllocator.ReleaseAndGetAddressOf())));

	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> CommandQueue::CreateCommandList(ID3D12CommandAllocator* commandAllocator)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	
	THROW_IF_FAILED(m_device->CreateCommandList(0, m_type, commandAllocator, 
		nullptr, IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf())));
	
	return commandList;
}

ComPtr<ID3D12GraphicsCommandList> CommandQueue::GetCommandList()
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	
	if (!m_commandAllocatorQueue.empty() && IsFenceValueReached(m_commandAllocatorQueue.front().fenceValue))
	{
		commandAllocator = std::move(m_commandAllocatorQueue.front().commandAllocator);
		THROW_IF_FAILED(commandAllocator->Reset());
		m_commandAllocatorQueue.pop();
	}
	else
	{
		commandAllocator = CreateCommandAllocator();
	}
	
	if (!m_commandListQueue.empty())
	{
		commandList = std::move(m_commandListQueue.front());
		m_commandListQueue.pop();

		THROW_IF_FAILED(commandList->Reset(commandAllocator.Get(), nullptr));
	}
	else
	{
		commandList = CreateCommandList(commandAllocator.Get());
	}
	
	commandList->SetPrivateDataInterface( __uuidof(ID3D12CommandAllocator), commandAllocator.Get());
	
	return commandList;
}

bool CommandQueue::IsFenceValueReached(uint64_t fencevalue)
{
	return m_fence->GetCompletedValue() >= fencevalue;
}

void CommandQueue::MoveToNextFrame(uint64_t fenceValueToWaitOn)
{
	if (!IsFenceValueReached(fenceValueToWaitOn))
	{
		m_fence->SetEventOnCompletion(fenceValueToWaitOn, m_fenceEventHandle);
		WaitForSingleObject(m_fenceEventHandle, INFINITE);
	}
}

void CommandQueue::WaitForGPU()
{
	m_commandQueue->Signal(m_fence.Get(), ++m_lastFenceValue);
	MoveToNextFrame(m_lastFenceValue);
}

uint64_t CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList* commandList)
{
	THROW_IF_FAILED(commandList->Close());
	
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	
	UINT dataSize = (UINT)sizeof(ID3D12CommandAllocator);
 	
	THROW_IF_FAILED(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, commandAllocator.ReleaseAndGetAddressOf()));

	ID3D12CommandList *const commandLists[] { commandList };
	
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	
	m_commandQueue->Signal(m_fence.Get(), ++m_lastFenceValue);
	
	m_commandAllocatorQueue.emplace(CommandAllocator{std::move(commandAllocator), m_lastFenceValue});

	m_commandListQueue.emplace(std::move(commandList));
	
	return m_lastFenceValue;
}