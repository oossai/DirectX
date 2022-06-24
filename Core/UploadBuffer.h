#pragma once
#include "../Headers.h"

template<typename T>
class UploadBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	size_t m_elementSize = sizeof(T);
	BYTE* m_mappedData = nullptr;
	bool m_isConstantBuffer = false;
public:
	UploadBuffer() = default;
	void Initialize(ID3D12Device3* device, size_t elementCount, size_t isConstantBuffer);
	~UploadBuffer();
	ID3D12Resource* GetResource() { return m_resource.Get(); }
	void CopyData(size_t elementIndex, const T* data);
};

template<typename T>
void UploadBuffer<T>::Initialize(ID3D12Device3* device, size_t elementCount, size_t isConstantBuffer)
{
	m_isConstantBuffer = isConstantBuffer;
	if (isConstantBuffer)
	{
		m_elementSize = CalcConstantBufferByteSize(sizeof(T));
	}
	
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_elementSize * elementCount);
	
	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_resource.ReleaseAndGetAddressOf())));

	ThrowIfFailed(m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
}

template<typename T>
UploadBuffer<T>::~UploadBuffer()
{
	if (m_resource)
	{
		m_resource->Unmap(0, nullptr);
	}
	m_mappedData = nullptr;
}

template<typename T>
void UploadBuffer<T>::CopyData(size_t elementIndex, const T* data)
{
	memcpy(&m_mappedData[elementIndex * m_elementSize], data, sizeof(T));
}
 