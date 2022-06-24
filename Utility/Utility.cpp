#include "Utility.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

constexpr auto& Keep(auto&& x) noexcept 
{
    return x;
}

size_t CalcConstantBufferByteSize(size_t byteSize)
{
	return (byteSize + 255) & ~255;
}

std::string WideToNarrowString(std::wstring_view wideString)
{
	size_t wideStringSize = wideString.length() + 1;
	const size_t sizeOfNewString = wideStringSize * 2;
	char* str = new char[sizeOfNewString];
	size_t convertedWideChars = 0;
	if (wcstombs_s(&convertedWideChars, str, sizeOfNewString, wideString.data(), _TRUNCATE) != 0)
	{
		throw std::runtime_error("WideToNarrowString conversion failed!");
	}
	std::string narrowString{str};
	delete[] str;
	return narrowString;
}

std::wstring NarrowToWideString(std::string_view narrowString)
{
	size_t narrowStringSize = narrowString.length() + 1;
	const size_t sizeOfNewString = narrowStringSize * 2;
	wchar_t* wstr = new wchar_t[sizeOfNewString];
	size_t convertedChars = 0;
    if (mbstowcs_s(&convertedChars, wstr, sizeOfNewString, narrowString.data(), _TRUNCATE) != 0)
	{
		throw std::runtime_error("NarrowToWideString conversion failed!");
	}
	std::wstring wideString{wstr};
	delete[] wstr;
	return wideString;
}

void CreateDescriptorHeap(ID3D12Device* pDevice, 
	uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, 
	D3D12_DESCRIPTOR_HEAP_FLAGS flags, 
	ID3D12DescriptorHeap** ppDescriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = type;
	heapDesc.NumDescriptors = count;
	heapDesc.Flags = flags;

	THROW_IF_FAILED(pDevice->CreateDescriptorHeap(&heapDesc, 
		IID_PPV_ARGS(ppDescriptorHeap)));
}

void CreateDefaultBuffer(ID3D12Device* pDevice,
    ID3D12GraphicsCommandList* commandList,
    ID3D12Resource** ppDestinationResource, 
    ID3D12Resource** ppIntermediateResource,
    size_t sizeInBytes, const void* bufferData)
{
	assert(bufferData && "Buffer data is null");

	THROW_IF_FAILED(pDevice->CreateCommittedResource(
        &Keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &Keep(CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes)),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(ppDestinationResource)));
		
	THROW_IF_FAILED(pDevice->CreateCommittedResource(
        &Keep(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
        D3D12_HEAP_FLAG_NONE,
        &Keep(CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData{};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = sizeInBytes;
        subresourceData.SlicePitch = subresourceData.RowPitch;
 
        UpdateSubresources<1>(commandList, 
            *ppDestinationResource, *ppIntermediateResource,
            0, 0, 1, &subresourceData);
}

void ParseCommandLineArgs(bool& useWrap)
{
	int argc{};
	useWrap = false;
	wchar_t** argv = CommandLineToArgvW(GetCommandLine(), &argc); // Get commandLine args array of pointers
	
	for (int i = 0; i < argc; i++)
	{
		if (wcscmp( argv[i], L"-useWarp") == 0)
		{
			useWrap = true;
		}
	}
}

void RuntimeShaderCompile(const wchar_t* shaderFilePath, 
	const char* target, ID3DBlob** shaderBlob)
{
	ComPtr<ID3DBlob> errorBlob;
#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	HRESULT hr = D3DCompileFromFile(shaderFilePath, nullptr, nullptr, 
	"main", target, compileFlags, 0, shaderBlob, &errorBlob);
	
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			throw std::runtime_error((char*)errorBlob->GetBufferPointer());
		}
		else
		{
			throw Error{L"CreateShaderBlob", L"CreateShader.cpp", 6, hr};
		}
	}
}

void CalculateVertexNormals(FXMVECTOR vertexPositions[], size_t vs, 
	std::vector<uint16_t>& triangleIndices, XMFLOAT3 vertexNormals[], size_t ns)
{

    // // Zero-out our normal buffer to start from a clean slate.
    // for(int vertex = 0; vertex < vertexPositions.Length; vertex++)
    //     vertexNormals[vertex] = Vector3.zero;

    // For each face, compute the face normal, and accumulate it into each vertex.
    for(size_t index = 0; index < triangleIndices.size(); index += 3) {
        uint16_t vertexA = triangleIndices[index];
        uint16_t vertexB = triangleIndices[index + 1];
        uint16_t vertexC = triangleIndices[index + 2];    

        auto edgeAB = DirectX::XMVectorSubtract(vertexPositions[vertexB], vertexPositions[vertexA]);
        auto edgeAC = DirectX::XMVectorSubtract(vertexPositions[vertexC], vertexPositions[vertexA]);

        // The cross product is perpendicular to both input vectors (normal to the plane).
        // Flip the argument order if you need the opposite winding.    
        auto areaWeightedNormal = DirectX::XMVector3Cross(edgeAB, edgeAC);

        // Don't normalize this vector just yet. Its magnitude is proportional to the
        // area of the triangle (times 2), so this helps ensure tiny/skinny triangles
        // don't have an outsized impact on the final normal per vertex.

        // Accumulate this cross product into each vertex normal slot.
		XMVECTOR normalA = DirectX::XMLoadFloat3(&vertexNormals[vertexA]);
		normalA = DirectX::XMVectorAdd(normalA, areaWeightedNormal);
		DirectX::XMStoreFloat3(&vertexNormals[vertexA], normalA);
		
		XMVECTOR normalB = DirectX::XMLoadFloat3(&vertexNormals[vertexB]);
		normalB = DirectX::XMVectorAdd(normalB, areaWeightedNormal);
		DirectX::XMStoreFloat3(&vertexNormals[vertexB], normalB);

		XMVECTOR normalC = DirectX::XMLoadFloat3(&vertexNormals[vertexC]);
		normalC = DirectX::XMVectorAdd(normalC, areaWeightedNormal);
		DirectX::XMStoreFloat3(&vertexNormals[vertexC], normalC);

    }       

    // Finally, normalize all the sums to get a unit-length, area-weighted average.
    for(size_t vertex = 0; vertex < vs; vertex++)
	{
		XMVECTOR normal = DirectX::XMLoadFloat3(&vertexNormals[vertex]);
        DirectX::XMStoreFloat3(&vertexNormals[vertex], DirectX::XMVector3Normalize(normal));
	}
}