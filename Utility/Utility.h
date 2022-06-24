#ifndef __UTILITY_H__
#define  __UTILITY_H__

#include "../Headers.h"
#include "../Error.h"
#include "../Macros.h"

std::string WideToNarrowString(std::wstring_view wideString);

std::wstring NarrowToWideString(std::string_view narrowString);

void ParseCommandLineArgs(bool& useWrap);

size_t CalcConstantBufferByteSize(size_t byteSize);

void RuntimeShaderCompile(const wchar_t* shaderFilePath, 
	const char* target, ID3DBlob** shaderBlob);

struct GRAPHICS_PIPELINE
{
	static D3D12_GRAPHICS_PIPELINE_STATE_DESC DefaultPipelineDesc()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};
		graphicsPipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		graphicsPipelineDesc.SampleMask = UINT_MAX;
		graphicsPipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		graphicsPipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		graphicsPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		graphicsPipelineDesc.NumRenderTargets = 1;
		graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		graphicsPipelineDesc.SampleDesc.Count = 1;
		graphicsPipelineDesc.SampleDesc.Quality = 0;
		return graphicsPipelineDesc;
	}
};

void CreateDescriptorHeap(ID3D12Device* pDevice, 
	uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, 
	D3D12_DESCRIPTOR_HEAP_FLAGS flags, 
	ID3D12DescriptorHeap** descriptorHeap);

void CreateDefaultBuffer(ID3D12Device* pDevice,
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource** ppDestinationResource, 
    ID3D12Resource** ppIntermediateResource,
    size_t sizeInBytes, const void* bufferData);

void CalculateVertexNormals(DirectX::FXMVECTOR vertexPositions[], size_t vs, 
	std::vector<uint16_t>& triangleIndices, DirectX::XMFLOAT3 vertexNormals[], size_t ns);

#endif